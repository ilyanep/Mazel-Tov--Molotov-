#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include "../Baseline_Nov12/baseline_nov12.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include "svdk_nov13.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <math.h>


SVDK_Nov13::SVDK_Nov13(){
    //Initially all matrix elements are set to 0.1
    userSVD = gsl_matrix_calloc(USER_COUNT, SVD_DIM+1);
    movieSVD = gsl_matrix_calloc(SVD_DIM+1, MOVIE_COUNT);
    assert(userSVD != NULL && movieSVD != NULL);
    for(int i = 0; i < USER_COUNT; i++){
        for(int p = 0; p < SVD_DIM; p++){
            gsl_matrix_set(userSVD, i, p, INIT_SVD_VAL);
        }
    }
	for(int i = 0; i < MOVIE_COUNT; i++){
        for(int p = 0; p < SVD_DIM; p++){
            gsl_matrix_set(movieSVD, p, i, INIT_SVD_VAL);
        }
    }
    data_loaded = false;
    baseLoaded = false;
    learn_rate = LEARN_RATE;
    load_data();
    base_predict = Baseline_Nov12(data_loaded);
    //srand(time(NULL));
}

void SVDK_Nov13::learn(int partition){
    learn(partition, false);
}

void SVDK_Nov13::learn(int partition, bool refining){
    assert(data_loaded);

    if(!baseLoaded){
        printf("Loading baseline predictor...\n");
        base_predict.remember(partition);
        baseLoaded = true;
    }
    
    printf("Learning SVD...\n");
    printf("Generating unbiased data set...\n");
    unbiased_ratings = new double[DATA_COUNT];
    for(int point = 0; point < DATA_COUNT; point++){
        unbiased_ratings[point] = (double)get_mu_all_rating(point) - 
                                  base_predict.predict((int)get_mu_all_usernumber(point),
                                                       (int)get_mu_all_movienumber(point),
                                                       (int)get_mu_all_datenumber(point));
        if(point % 1000000 == 0)
            printf("Original rating: %lf, unbiased rating: %lf \n", (double)get_mu_all_rating(point), unbiased_ratings[point]);
    }

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
        if(p == SVD_DIM/3)
            min_epochs = min_epochs / 3;
        while(fabs(oldrmse - rmse) > MIN_RMSE_IMPROVEMENT || k < min_epochs){
        //while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT){
            oldrmse = rmse;
            k++;
            errsq = 0.0;
            point_count = 0;
            for(int i = 0; i < DATA_COUNT; i++){
                if(get_mu_idx_ratingset(i) <= partition){
                    err = learn_point(p, get_mu_all_usernumber(i)-1,
                                         (int)get_mu_all_movienumber(i)-1,
                                         unbiased_ratings[i], refining);
                    //if(err != -999){
                        errsq = errsq + err * err;
                        point_count++;
                    //}
                }
            }
            rmse = errsq / ((double)point_count);
            printf("\t\tEpoch %u: RMSE(in): %lf; RMSE(out): %lf\n", k, sqrt(rmse),rmse_probe());
        }
        save_svd(3);
    }
}

double SVDK_Nov13::learn_point(int svd_pt, int user, int movie, double rating, bool refining){
    //if(rating == 0)
    //    return -999;
    //Figure out current error on this point and modify feature parameters
    double err;
    //if(refining)
    //    err = rating - predict_point(user, movie, date);
    //else
	    err = rating - predict_point_train(user, movie, rating, svd_pt);
    double svd_user_old = gsl_matrix_get(userSVD, user, svd_pt); 
    double svd_movie_old = gsl_matrix_get(movieSVD, svd_pt, movie);

	gsl_matrix_set(userSVD, user, svd_pt, svd_user_old + 
                  (learn_rate * (err * svd_movie_old -
                  FEATURE_REGUL_PARAM * svd_user_old)));
	gsl_matrix_set(movieSVD, svd_pt, movie, svd_movie_old +
                  (learn_rate * (err * svd_user_old -
                  FEATURE_REGUL_PARAM * svd_movie_old)));

    //Update user and movie biases for this point
    double bias_user_old = gsl_matrix_get(userSVD, user, SVD_DIM); 
    double bias_movie_old = gsl_matrix_get(movieSVD, SVD_DIM, movie);
                 
    gsl_matrix_set(userSVD, user, SVD_DIM, bias_user_old + learn_rate * (err - BIAS_REGUL_PARAM * bias_user_old) );
	gsl_matrix_set(movieSVD, SVD_DIM, movie, bias_movie_old + learn_rate * (err - BIAS_REGUL_PARAM * bias_movie_old) );
    return err;
}

void SVDK_Nov13::save_svd(int partition){
    FILE *outFile;
    outFile = fopen(NOV13_SVDK_PARAM_FILE, "w");
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

void SVDK_Nov13::remember(int partition){
    if(!baseLoaded){
        base_predict.remember(partition);
        baseLoaded = true;
    }
    FILE *inFile;
    inFile = fopen(NOV13_SVDK_PARAM_FILE, "r");
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

void SVDK_Nov13::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_datenumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

    data_loaded = true;
}

double SVDK_Nov13::predict(int user, int movie, int time){
    double rating = predict_point(user-1, movie-1, time);
    return rating;
}

double SVDK_Nov13::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
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

double SVDK_Nov13::predict_point(int user, int movie, int date){
    double rating = base_predict.predict(user+1, movie+1, date) +
                    gsl_matrix_get(userSVD, user, SVD_DIM) +
                    gsl_matrix_get(movieSVD, SVD_DIM, movie);
    for (int i = 0; i < SVD_DIM; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}

double SVDK_Nov13::predict_point_train(int user, int movie, double base, int svd_pt){
    double rating = gsl_matrix_get(userSVD, user, SVD_DIM) +
                    gsl_matrix_get(movieSVD, SVD_DIM, movie);
    for (int i = 0; i <= svd_pt; i++){
        rating = rating + gsl_matrix_get(userSVD, user, i) * 
                  gsl_matrix_get(movieSVD, i, movie);
    }
    rating = rating + INIT_SVD_VAL * INIT_SVD_VAL * (SVD_DIM - svd_pt -1);
    if(rating < (1.0 - base))
        return (1.0 - base);
    else if(rating > (5.0 - base))
        return (5.0 - base);
    return rating;
}
            
        
