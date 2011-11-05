#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include "svd_oct25.h"


Oct25_SVD::Oct25_SVD(){
    //Initially all matrix elements are set to 0.1
    userSVD = gsl_matrix_calloc(OCT25_USER_COUNT, OCT25_SVD_DIM);
    movieSVD = gsl_matrix_calloc(OCT25_SVD_DIM, OCT25_MOVIE_COUNT);
    for(int i = 0; i < OCT25_USER_COUNT; i++){
        for(int p = 0; p < OCT25_SVD_DIM; p++){
            gsl_matrix_set(userSVD, i, p, OCT25_INIT_SVD_VAL);
        }
    }
	for(int i = 0; i < OCT25_MOVIE_COUNT; i++){
        for(int p = 0; p < OCT25_SVD_DIM; p++){
            gsl_matrix_set(movieSVD, p, i, OCT25_INIT_SVD_VAL);
        }
    }
    data_loaded = false;
    learn_rate = OCT25_LEARN_RATE;
    svd_regul = OCT25_REGUL_PARAM;
    load_data();
    base_predict = Oct25_Baseline(data_loaded);
    srand(time(NULL));
}

void Oct25_SVD::learn(int partition){
    learn(partition, false);
}

void Oct25_SVD::learn(int partition, bool refining){
    assert(data_loaded);

    /* Load bias parameters */
    printf("Loading baseline...\n");
    base_predict.remember(partition);

    printf("Computing SVD...\n");
    int param_start = 0;
    int min_epochs = OCT25_LEARN_EPOCHS_MIN;
    double init_value = OCT25_INIT_SVD_VAL;
    if(refining){
        int svd_pt = 0;
        while(svd_pt < OCT25_SVD_DIM && gsl_matrix_get(userSVD, 0, svd_pt) != init_value){
            svd_pt++;
        }
        if(svd_pt != OCT25_SVD_DIM)
            param_start = svd_pt;
        min_epochs = OCT25_REFINE_EPOCHS_MIN;
    }
    /* Cycle through dataset */
    for(int p = param_start; p < OCT25_SVD_DIM; p++){
        printf("\nSVD Parameter %u: Learning... %u percent.\n", p+1, (int)((double)(p)*100.0/(double)OCT25_SVD_DIM));
        int k = 0;
        int point_count;
        double err;
        double errsq;
        double rmse = 10.0;
        double oldrmse = 100.0;
        while(fabs(oldrmse - rmse) > OCT25_MIN_RMSE_IMPROVEMENT || k < min_epochs){
        //while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT){
            oldrmse = rmse;
            k++;
            errsq = 0.0;
            point_count = 0;
            for(int i = 0; i < OCT25_DATA_COUNT; i++){
                if(get_mu_idx_ratingset(i) <= partition){
                    err = learn_point(p, get_mu_all_usernumber(i)-1,
                                         get_mu_all_movienumber(i)-1,
                                         get_mu_all_datenumber(i),
                                         get_mu_all_rating(i), refining);
                    if(err != -999){
                        errsq = errsq + err * err;
                        point_count++;
                    }
                }
            }
            rmse = errsq / ((double)point_count);
            printf("\tEpoch %u: RMSE(in): %f; RMSE(out): %f\n", k, sqrt(rmse),rmse_probe());
        }
        save_svd(3);
    }
}

double Oct25_SVD::learn_point(int svd_pt, int user, int movie, int time, double rating, bool refining){
    if(rating == 0)
        return -999;
    double err;
    if(refining)
        err = rating - predict_point(user, movie, time);
    else
	    err = rating - predict_point_train(user, movie, time, svd_pt);

    double svd_user_old = gsl_matrix_get(userSVD, user, svd_pt); 
    double svd_movie_old = gsl_matrix_get(movieSVD, svd_pt, movie); 
	gsl_matrix_set(userSVD, user, svd_pt, svd_user_old + 
                  (learn_rate * (err * svd_movie_old -
                  svd_regul * svd_user_old)));
	gsl_matrix_set(movieSVD, svd_pt, movie, svd_movie_old +
                  (learn_rate * (err * svd_user_old -
                  svd_regul * svd_movie_old)));
    return err;
}

void Oct25_SVD::save_svd(int partition){
    FILE *outFile;
    outFile = fopen(OCT25_SVD_PARAM_FILE, "w");
    fprintf(outFile,"%u\n",partition);
    for(int user = 0; user < OCT25_USER_COUNT; user++){
        for(int i = 0; i < OCT25_SVD_DIM; i++) {
            fprintf(outFile,"%lf ",gsl_matrix_get(userSVD, user, i)); 
        }
        fprintf(outFile,"\n");
    }
    for(int movie = 0; movie < OCT25_MOVIE_COUNT; movie++){
        for(int i = 0; i < OCT25_SVD_DIM; i++) {
            fprintf(outFile,"%lf ",gsl_matrix_get(movieSVD, i, movie));
        }
    }
    fclose(outFile);
    return;
}

void Oct25_SVD::remember(int partition){
    base_predict.remember(partition);
    FILE *inFile;
    inFile = fopen(OCT25_SVD_PARAM_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    //printf("File opened.\n");
    double g = 0.0;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    for(int user = 0; user < OCT25_USER_COUNT; user++){
        for(int i = 0; i < OCT25_SVD_DIM; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(userSVD, user, i, g);
	    }
    }
    for(int movie = 0; movie < OCT25_MOVIE_COUNT; movie++){
        for(int i = 0; i < OCT25_SVD_DIM; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(movieSVD, i, movie, g);
        }
    }
    fclose(inFile);
    return;

}

void Oct25_SVD::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_datenumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

    data_loaded = true;
}

double Oct25_SVD::predict(int user, int movie, int time){
    double rating = (double)predict_point(user-1, movie-1, time);
    return rating;
}

double Oct25_SVD::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < OCT25_DATA_COUNT; i++) {
        if(get_mu_idx_ratingset(i) == 4){
            double prediction = predict(get_mu_all_usernumber(i),
                                        (int)get_mu_all_movienumber(i),
                                        (int)get_mu_all_datenumber(i));
            double error = (prediction - (double)get_mu_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}   

double Oct25_SVD::predict_point(int user, int movie, int date){
    double rating = base_predict.predict(user+1, movie+1, date);
    for (int i = 0; i < OCT25_SVD_DIM; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);   
    }
    return rating;
}

double Oct25_SVD::predict_point_train(int user, int movie, int date, int svd_pt){
    double rating = base_predict.predict(user+1, movie+1, date);
    for (int i = 0; i <= svd_pt; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    rating = rating + OCT25_INIT_SVD_VAL * OCT25_INIT_SVD_VAL * (OCT25_SVD_DIM - svd_pt -1);
    return rating;
}
            
        
