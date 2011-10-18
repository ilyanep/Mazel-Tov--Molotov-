#include "blender.h"
#include <string>
using namespace std;
#include "../binary_files/binary_files.h"
#include "../learning_method.h"
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

// Assuming that all the predictors involved have already learned.
void LinearBlender::learn(int partition) {
    cout << "Learning the linear blender" << endl;
    cout << "Loading all data and remembering the predictors" << endl;
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_datenumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);
    for (int i=0; i < predictors_.size(); ++i) {
        predictors_[i]->remember(LEARNED_PARTITION); 
    }

    cout << "Gathering relevant points" << endl;
    // Count how many points we are using to train
    vector<int64> relevant_points_list;
    for (int64 i=0; i < ALL_POINTS_SIZE; ++i) {
        if (get_mu_idx_ratingset(i) == partition) {
            relevant_points_list.push_back(i);
        }   
    }

    // Fill predictions matrix
    cout << "1" << endl;
    gsl_matrix * predictions_matrix = gsl_matrix_alloc(relevant_points_list.size(), predictors_.size());
    for(int64 i = 0; i < relevant_points_list.size(); ++i) {
        for(int j = 0; j < predictors_.size(); ++j) {
            gsl_matrix_set(predictions_matrix, i, j, 
                           predictors_[j]->predict(get_mu_all_usernumber(relevant_points_list[i]),
                                                  get_mu_all_movienumber(relevant_points_list[i]),
                                                  get_mu_all_datenumber(relevant_points_list[i])));
        }
    }
cout << "2" << endl;
    // Fill ratings vector
    gsl_vector * ratings_vector = gsl_vector_alloc(relevant_points_list.size());
    for(int64 i = 0; i < relevant_points_list.size(); ++i) {
        gsl_vector_set(ratings_vector, i, get_mu_all_rating(relevant_points_list[i]));
    }
cout << "3" << endl;
    aggregator_weights_ = aggregator_solution(predictions_matrix, ratings_vector, relevant_points_list.size(),
                                              predictors_.size());

    initialized_ = true;
}

gsl_vector * LinearBlender::aggregator_solution(gsl_matrix* predictions_matrix, gsl_vector* ratings_vector, 
                                                int64 pred_points, int64 pred_num) {
    
    // Solution is (G^T G)^-1 G^T r where G is predictions_matrix and r is ratings_vector
    gsl_matrix * gtrans_g = gsl_matrix_alloc(pred_num, pred_num);
    gsl_matrix * gtrans_g_inverse = gsl_matrix_alloc(pred_num, pred_num);
    gsl_matrix * gtrans = gsl_matrix_alloc(pred_num, pred_points);
    gsl_matrix * identity = gsl_matrix_alloc(pred_points, pred_points); 
    gsl_matrix * gtrans_g_inverse_gtrans = gsl_matrix_alloc(pred_num, pred_points);
    gsl_vector * final_vector = gsl_vector_alloc(pred_points);

    // Calculate G^T G
    gsl_matrix_set_zero(gtrans_g);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, predictions_matrix, predictions_matrix,
                   1.0, gtrans_g);

    // Inverse of G^T G
    gsl_permutation * p = gsl_permutation_alloc(pred_points);
    int *signum;
    *signum = 1; // Does anyone know what the hell this is?
    gsl_linalg_LU_decomp(gtrans_g, p, signum);
    gsl_linalg_LU_invert(gtrans_g, p, gtrans_g_inverse);

    // Get G^T
    gsl_matrix_set_zero(gtrans);
    gsl_matrix_set_identity(identity);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, predictions_matrix, identity, 1.0, gtrans);

   // Multiply (G^T G)^-1 G^T
   gsl_matrix_set_zero(gtrans_g_inverse_gtrans);
   gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, gtrans_g_inverse, gtrans, 1.0, gtrans_g_inverse_gtrans);

   // Do the vector multpilication 
   gsl_vector_set_zero(final_vector);
   gsl_blas_dgemv(CblasNoTrans, 1.0, gtrans_g_inverse_gtrans, ratings_vector, 1.0, final_vector);

   return final_vector;
}

// Saving results is for pussies (but mostly, I don't feel like implementing a check for each of the
// other predictors not having changed).
void LinearBlender::remember(int partition) {}

double LinearBlender::predict(int user, int movie, int date) {
    assert(initialized_ == true);
    // Use solved aggregator matrix to predict
    double current_prediction = 0.0;
    for (int i = 0; i < predictors_.size(); ++i) {
        current_prediction += (gsl_vector_get(aggregator_weights_, i) * predictors_[i]->predict(user, movie, date));    
    }

    return current_prediction;
}
