#include "blender.h"
#include <string>
using namespace std;
#include "../binary_files/binary_files.h"
#include "../movie_knn/movie_knn.h"
#include <assert.h>
#include <vector>
#include <iostream>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>

#define ALL_POINTS_SIZE 102416306
#define LEARNED_PARTITION 3 // The partition that all the predictors included learned on.

LinearBlender::LinearBlender(vector<IPredictor*> predictors) {
    predictors_ = predictors;
    initialized_ = false;
}

void LinearBlender::free_mem() {}

// Assuming that all the predictors involved have already learned.
void LinearBlender::learn(int partition) {
    cout << "Learning the linear blender" << endl;
    cout << "Loading all data and remembering the predictors" << endl;
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_datenumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

    //for (int i=0; i < predictors_.size(); ++i) {
    //    predictors_[i]->remember(LEARNED_PARTITION); 
    //}

    cout << "Gathering relevant points" << endl;
    // Count how many points we are using to train
    vector<int64> relevant_points_list;
    for (int64 i=0; i < ALL_POINTS_SIZE; ++i) {
        if (get_mu_idx_ratingset(i) == partition) {
            relevant_points_list.push_back(i);
        }   
    }

    // Fill predictions matrix
    cout << "Filling predictions matrix and ratings vector" << endl;
    gsl_matrix * predictions_matrix = gsl_matrix_alloc(relevant_points_list.size(), predictors_.size());
    for(int j = 0; j < predictors_.size(); ++j) {
        printf("predictor number %i\n", j);
        predictors_[j]->remember(LEARNED_PARTITION); 
        for(int64 i = 0; i < relevant_points_list.size(); ++i) {
            gsl_matrix_set(predictions_matrix, i, j, 
                           predictors_[j]->predict(get_mu_all_usernumber(relevant_points_list[i]),
                                                  get_mu_all_movienumber(relevant_points_list[i]),
                                                  get_mu_all_datenumber(relevant_points_list[i]),
                                                  relevant_points_list[i]));
        }
        predictors_[j]->free_mem(); 
    }

    // Fill ratings vector
    gsl_vector * ratings_vector = gsl_vector_alloc(relevant_points_list.size());
    for(int64 i = 0; i < relevant_points_list.size(); ++i) {
        gsl_vector_set(ratings_vector, i, get_mu_all_rating(relevant_points_list[i]));
    }

    aggregator_weights_ = aggregator_solution(predictions_matrix, ratings_vector, relevant_points_list.size(),
                                              predictors_.size());

    initialized_ = true;
}

gsl_vector * LinearBlender::aggregator_solution(gsl_matrix* predictions_matrix, gsl_vector* ratings_vector, 
                                                int64 pred_points, int64 pred_num) {
    cout << "Calculating aggregator weights" << endl;
    
    // Solution is (G^T G)^-1 G^T r where G is predictions_matrix and r is ratings_vector

    cout << "G^T G" << endl;
    // Calculate G^T G
    gsl_matrix * gtrans_g = gsl_matrix_alloc(pred_num, pred_num);
    gsl_matrix_set_zero(gtrans_g);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, predictions_matrix, predictions_matrix,
                   1.0, gtrans_g);

    cout << "(G^T G)^{-1}" << endl;
    // Inverse of G^T G
    gsl_matrix * gtrans_g_inverse = gsl_matrix_alloc(pred_num, pred_num);
    gsl_permutation * p = gsl_permutation_alloc(pred_num);
    int signum = 1; // Does anyone know what the hell this is?
    gsl_linalg_LU_decomp(gtrans_g, p, &signum);
    gsl_linalg_LU_invert(gtrans_g, p, gtrans_g_inverse);
    gsl_matrix_free(gtrans_g);

    // Manual multiplication to save memory
    cout << "G^T r" << endl;
    gsl_vector * gtrans_r_vector = gsl_vector_alloc(pred_num);
    gsl_vector_set_zero(gtrans_r_vector); // Just in case
    double current_sum = 0;
    for(int i=0; i< pred_num; ++i) {
        for(int64 j=0; j<pred_points; ++j) {
            current_sum += gsl_vector_get(ratings_vector, j) * gsl_matrix_get(predictions_matrix, j, i);
        }
        gsl_vector_set(gtrans_r_vector, i, current_sum);
        current_sum = 0;
    }

    // This old code to get G^T took up too much memory
    /*gsl_matrix * gtrans = gsl_matrix_alloc(pred_num, pred_points);
    gsl_matrix * identity = gsl_matrix_alloc(pred_points, pred_points); 
    gsl_matrix_set_zero(gtrans);
    gsl_matrix_set_identity(identity);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, predictions_matrix, identity, 1.0, gtrans); */

    /*cout << "(G^T G)^-1 G^T" << endl;
    // Multiply (G^T G)^-1 G^T
    gsl_matrix * gtrans_g_inverse_gtrans = gsl_matrix_alloc(pred_num, pred_points);
    gsl_matrix_set_zero(gtrans_g_inverse_gtrans);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, gtrans_g_inverse, gtrans, 1.0, gtrans_g_inverse_gtrans);
    gsl_matrix_free(gtrans_g_inverse);
    gsl_matrix_free(gtrans); */

    cout << "(G^T G)^-1 times that last thing" << endl;
    // Do the vector multpilication 
    gsl_vector * final_vector = gsl_vector_alloc(pred_num);
    gsl_vector_set_zero(final_vector);
    gsl_blas_dgemv(CblasNoTrans, 1.0, gtrans_g_inverse, gtrans_r_vector, 1.0, final_vector);
    gsl_matrix_free(gtrans_g_inverse);
    gsl_vector_free(gtrans_r_vector);

    cout << "Final vector" << endl;
    for(int i=0; i< pred_num; ++i) {
        cout << gsl_vector_get(final_vector, i) << " ";
    }
    cout << endl;

    return final_vector;
}

// Saving results is for pussies (but mostly, I don't feel like implementing a check for each of the
// other predictors not having changed).
void LinearBlender::remember(int partition) {}

double LinearBlender::predict(int user, int movie, int date, int index) {
    assert(initialized_ == true);
    // Use solved aggregator matrix to predict
    double current_prediction = 0.0;
    for (int i = 0; i < predictors_.size(); ++i) {
        current_prediction += (gsl_vector_get(aggregator_weights_, i) * predictors_[i]->predict(user, movie, date, index));    
    }

    return current_prediction;
}
