#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include "learn_svd.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <math.h>


SVD::SVD(){
    //Initially all matrix elements are set to 0.1
    userSVD = gsl_matrix_calloc(USER_COUNT, SVD_DIM+1);
    movieSVD = gsl_matrix_calloc(SVD_DIM+1, MOVIE_COUNT);
    for(int i = 0; i < USER_COUNT; i++){
        for(int p = 0; p < SVD_DIM+1; p++){
            gsl_matrix_set(userSVD, i, p, INIT_SVD_VAL);
        }
    }
	for(int i = 0; i < MOVIE_COUNT; i++){
        for(int p = 0; p < SVD_DIM+1; p++){
            gsl_matrix_set(movieSVD, p, i, INIT_SVD_VAL);
        }
    }
    data_loaded = false;
    learn_rate = LEARN_RATE;
    svd_regul = REGUL_PARAM;
    load_data();
    srand(time(NULL));
}

void SVD::learn(int partition){
    learn(partition, false);
}

void SVD::learn(int partition, bool refining){
    assert(data_loaded);

    /* Load bias parameters */
    FILE *inFile;
    inFile = fopen(SVD_BIAS_FILE, "r");
    double bias;
    for(int i = 0; i < USER_COUNT; i++){
        fscanf(inFile, "%lf", &bias);
        gsl_matrix_set(userSVD, i, SVD_DIM, bias);
    }
	for(int i = 0; i < MOVIE_COUNT; i++){
        fscanf(inFile, "%lf", &bias);
        gsl_matrix_set(movieSVD, SVD_DIM, i, bias);
    }
    fclose(inFile);

    /* Choose points randomly */
   // for(int k = 0; k < DATA_COUNT * LEARN_EPOCHS; k++){
   //     if(k % DATA_COUNT == 0){
   //         printf("\tLearning ... %u percent \n", (int)((double)(k+1)*100.0/((double)(DATA_COUNT * LEARN_EPOCHS))));
   //     }
   //     int i = rand() % DATA_COUNT;

    int param_start = 0;
    int min_epochs = LEARN_EPOCHS_MIN;
    double init_value = INIT_SVD_VAL;
    if(refining){
		printf("Refining...\n");
        int svd_pt = 0;
        while(svd_pt < SVD_DIM && gsl_matrix_get(userSVD, 0, svd_pt) != init_value){
            svd_pt++;
        }
        if(svd_pt != SVD_DIM)
            param_start = svd_pt;
        min_epochs = REFINE_EPOCHS_MIN;
    }
    /* Cycle through dataset */
    for(int p = param_start; p < SVD_DIM; p++){
        printf("\tParameter %u: Learning... %u percent.\n", p+1, (int)((double)(p+1)*100.0/(double)SVD_DIM));
        int k = 0;
        int point_count;
        double err;
        double errsq;
        double rmse = 10.0;
        double oldrmse = 100.0;
        while(fabs(oldrmse - rmse) > MIN_RMSE_IMPROVEMENT || k < min_epochs){
        //while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT){
            oldrmse = rmse;
            k++;
            errsq = 0.0;
            point_count = 0;
            for(int i = 0; i < DATA_COUNT; i++){
                if(get_mu_idx_ratingset(i) <= partition){
                    err = learn_point(p, get_mu_all_usernumber(i)-1,
                                         get_mu_all_movienumber(i)-1,
                                         get_mu_all_rating(i)-AVG_RATING, refining);
                    if(err != -999){
                        errsq = errsq + err * err;
                        point_count++;
                    }
                }
            }
            rmse = errsq / ((double)point_count);
            printf("\t\tEpoch %u: RMSE(in): %lf; RMSE(out): %lf\n", k, sqrt(rmse),rmse_probe());
        }
        save_svd(3);
    }
}

double SVD::learn_point(int svd_pt, int user, int movie, double rating, bool refining){
    if(rating == 0)
        return -999;
    double err;
    if(refining)
        err = rating - predict_point(user, movie);
    else
	    err = rating - predict_point_train(user, movie, svd_pt);
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

void SVD::save_svd(int partition){
    FILE *outFile;
    outFile = fopen(SVD_PARAM_FILE, "w");
    fprintf(outFile,"%u\n",partition);
    for(int user = 0; user < USER_COUNT; user++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fprintf(outFile,"%lf ",gsl_matrix_get(userSVD, user, i)); 
        }
        fprintf(outFile,"\n");
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fprintf(outFile,"%lf ",gsl_matrix_get(movieSVD, i, movie));
        }
    }
    fclose(outFile);
    return;
}

void SVD::remember(int partition){
    FILE *inFile;
    inFile = fopen(SVD_PARAM_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    //printf("File opened.\n");
    double g = 0.0;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    for(int user = 0; user < USER_COUNT; user++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(userSVD, user, i, g);
	    }
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(movieSVD, i, movie, g);
        }
    }
    fclose(inFile);
    return;

}

void SVD::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

    data_loaded = true;
}

double SVD::predict(int user, int movie, int time){
    double rating = AVG_RATING + (double)predict_point(user-1, movie-1);
    return rating;
}

double SVD::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
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

double SVD::predict_point(int user, int movie){
    double rating = gsl_matrix_get(userSVD, user, SVD_DIM) +
                   gsl_matrix_get(movieSVD, SVD_DIM, movie);
    for (int i = 0; i < SVD_DIM; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    return rating;
}

double SVD::predict_point_train(int user, int movie, int svd_pt){
    double rating = gsl_matrix_get(userSVD, user, SVD_DIM) +
                   gsl_matrix_get(movieSVD, SVD_DIM, movie);
    for (int i = 0; i <= svd_pt; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    rating = rating + INIT_SVD_VAL * INIT_SVD_VAL * (SVD_DIM - svd_pt -1);
    return rating;
}
            
        
