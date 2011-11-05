#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include "svd_oct18.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <math.h>


Oct18_SVD::Oct18_SVD(){
    //Initially all matrix elements are set to 0.1
    userSVD = gsl_matrix_calloc(OCT18_USER_COUNT, OCT18_SVD_DIM+1);
    movieSVD = gsl_matrix_calloc(OCT18_SVD_DIM+1, OCT18_MOVIE_COUNT);
    for(int i = 0; i < OCT18_USER_COUNT; i++){
        for(int p = 0; p < OCT18_SVD_DIM+1; p++){
            gsl_matrix_set(userSVD, i, p, OCT18_INIT_SVD_VAL);
        }
    }
	for(int i = 0; i < OCT18_MOVIE_COUNT; i++){
        for(int p = 0; p < OCT18_SVD_DIM+1; p++){
            gsl_matrix_set(movieSVD, p, i, OCT18_INIT_SVD_VAL);
        }
    }
    data_loaded = false;
    learn_rate = OCT18_LEARN_RATE;
    svd_regul = OCT18_REGUL_PARAM;
    load_data();
    srand(time(NULL));
}

void Oct18_SVD::learn(int partition){
    learn(partition, false);
}

void Oct18_SVD::learn(int partition, bool refining){
    assert(data_loaded);

    /* Load bias parameters */
    FILE *inFile;
    inFile = fopen(OCT18_SVD_BIAS_FILE, "r");
    float bias;
    for(int i = 0; i < OCT18_USER_COUNT; i++){
        fscanf(inFile, "%g", &bias);
        gsl_matrix_set(userSVD, i, OCT18_SVD_DIM, bias);
    }
	for(int i = 0; i < OCT18_MOVIE_COUNT; i++){
        fscanf(inFile, "%g", &bias);
        gsl_matrix_set(movieSVD, OCT18_SVD_DIM, i, bias);
    }
    fclose(inFile);

    /* Choose points randomly */
   // for(int k = 0; k < OCT18_DATA_COUNT * LEARN_EPOCHS; k++){
   //     if(k % OCT18_DATA_COUNT == 0){
   //         printf("\tLearning ... %u percent \n", (int)((float)(k+1)*100.0/((float)(OCT18_DATA_COUNT * LEARN_EPOCHS))));
   //     }
   //     int i = rand() % OCT18_DATA_COUNT;

    int param_start = 0;
    int min_epochs = OCT18_LEARN_EPOCHS_MIN;
    float init_value = OCT18_INIT_SVD_VAL;
    if(refining){
        int svd_pt = 0;
        while(svd_pt < OCT18_SVD_DIM && gsl_matrix_get(userSVD, 0, svd_pt) != init_value){
            svd_pt++;
        }
        if(svd_pt != OCT18_SVD_DIM)
            param_start = svd_pt;
        min_epochs = OCT18_REFINE_EPOCHS_MIN;
    }
    /* Cycle through dataset */
    for(int p = param_start; p < OCT18_SVD_DIM; p++){
        printf("\tParameter %u: Learning... %u percent.\n", p+1, (int)((float)(p+1)*100.0/(float)OCT18_SVD_DIM));
        int k = 0;
        int point_count;
        float err;
        float errsq;
        float rmse = 10.0;
        float oldrmse = 100.0;
        while(fabs(oldrmse - rmse) > OCT18_MIN_RMSE_IMPROVEMENT || k < min_epochs){
        //while(oldrmse - rmse > OCT18_MIN_RMSE_IMPROVEMENT){
            oldrmse = rmse;
            k++;
            errsq = 0.0;
            point_count = 0;
            for(int i = 0; i < OCT18_DATA_COUNT; i++){
                if(get_mu_idx_ratingset(i) <= partition){
                    err = learn_point(p, get_mu_all_usernumber(i)-1,
                                         get_mu_all_movienumber(i)-1,
                                         get_mu_all_rating(i)-OCT18_AVG_RATING, refining);
                    if(err != -999){
                        errsq = errsq + err * err;
                        point_count++;
                    }
                }
            }
            rmse = errsq / ((float)point_count);
            printf("\t\tEpoch %u: RMSE(in): %f; RMSE(out): %f\n", k, sqrt(rmse),rmse_probe());
        }
        save_svd(3);
    }
}

float Oct18_SVD::learn_point(int svd_pt, int user, int movie, float rating, bool refining){
    if(rating == 0)
        return -999;
    float err;
    if(refining)
        err = rating - predict_point(user, movie);
    else
	    err = rating - predict_point_train(user, movie, svd_pt);
    float svd_user_old = gsl_matrix_get(userSVD, user, svd_pt); 
    float svd_movie_old = gsl_matrix_get(movieSVD, svd_pt, movie); 

	gsl_matrix_set(userSVD, user, svd_pt, svd_user_old + 
                  (OCT18_LEARN_RATE * (err * svd_movie_old -
                  svd_regul * svd_user_old)));
	gsl_matrix_set(movieSVD, svd_pt, movie, svd_movie_old +
                  (OCT18_LEARN_RATE * (err * svd_user_old -
                  svd_regul * svd_movie_old)));
    return err;
}

void Oct18_SVD::save_svd(int partition){
    FILE *outFile;
    outFile = fopen(OCT18_SVD_PARAM_FILE, "w");
    fprintf(outFile,"%u\n",partition);
    for(int user = 0; user < OCT18_USER_COUNT; user++){
        for(int i = 0; i < OCT18_SVD_DIM+1; i++) {
            fprintf(outFile,"%f ",gsl_matrix_get(userSVD, user, i)); 
        }
        fprintf(outFile,"\n");
    }
    for(int movie = 0; movie < OCT18_MOVIE_COUNT; movie++){
        for(int i = 0; i < OCT18_SVD_DIM+1; i++) {
            fprintf(outFile,"%f ",gsl_matrix_get(movieSVD, i, movie));
        }
    }
    fclose(outFile);
    return;
}

void Oct18_SVD::remember(int partition){
    FILE *inFile;
    inFile = fopen(OCT18_SVD_PARAM_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    //printf("File opened.\n");
    float g = 0.0;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    for(int user = 0; user < OCT18_USER_COUNT; user++){
        for(int i = 0; i < OCT18_SVD_DIM+1; i++) {
            fscanf(inFile, "%g", &g);
            gsl_matrix_set(userSVD, user, i, g);
	    }
    }
    for(int movie = 0; movie < OCT18_MOVIE_COUNT; movie++){
        for(int i = 0; i < OCT18_SVD_DIM+1; i++) {
            fscanf(inFile, "%g", &g);
            gsl_matrix_set(movieSVD, i, movie, g);
        }
    }
    fclose(inFile);
    return;

}

void Oct18_SVD::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

    data_loaded = true;
}

double Oct18_SVD::predict(int user, int movie, int time){
    double rating = OCT18_AVG_RATING + (double)predict_point(user-1, movie-1);
    return rating;
}

float Oct18_SVD::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < OCT18_DATA_COUNT; i++) {
        if(get_mu_idx_ratingset(i) == 4){
            double prediction = predict(get_mu_all_usernumber(i),
                                                  (int)get_mu_all_movienumber(i),0);
            double error = (prediction - (double)get_mu_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}   

float Oct18_SVD::predict_point(int user, int movie){
    float rating = gsl_matrix_get(userSVD, user, OCT18_SVD_DIM) +
                   gsl_matrix_get(movieSVD, OCT18_SVD_DIM, movie);
    for (int i = 0; i < OCT18_SVD_DIM; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    return rating;
}

float Oct18_SVD::predict_point_train(int user, int movie, int svd_pt){
    float rating = gsl_matrix_get(userSVD, user, OCT18_SVD_DIM) +
                   gsl_matrix_get(movieSVD, OCT18_SVD_DIM, movie);
    for (int i = 0; i <= svd_pt; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    rating = rating + OCT18_INIT_SVD_VAL * OCT18_INIT_SVD_VAL * (OCT18_SVD_DIM - svd_pt -1);
    return rating;
}
            
        
