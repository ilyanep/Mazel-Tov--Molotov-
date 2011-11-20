#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include "../Baseline_Nov19/baseline_nov19.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include "svdk_nov17.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <math.h>

/* Matrices:
 * userSVD: [bias factor0 factor1 ... factorN]
 * movieSVD: [bias factor0 factor1 ... factorN nfactor0 nfactor1 ... nfactorN]
 *
 */

SVDK_Nov17::SVDK_Nov17(){
    data_loaded = false;
    baseLoaded = false;
    userMoviesGenerated = false;
    learn_rate = LEARN_RATE;
    load_data();
    base_predict = Baseline_Nov19(data_loaded);
    //srand(time(NULL));
}

void SVDK_Nov17::free_mem(){
    userSVD.clear();
    movieSVD.clear();
    base_predict.free_mem();
    baseLoaded = false;
}

void SVDK_Nov17::learn(int partition){
    learn(partition, false);
}

void SVDK_Nov17::learn(int partition, bool refining){
    assert(data_loaded);
    double init_value =  INIT_SVD_VAL ;

     //Initially all matrix factor elements are set to 0.1
    userSVD.reserve(USER_COUNT);
    movieSVD.reserve(MOVIE_COUNT);
    for(int i = 0; i < USER_COUNT; i++){
        userSVD.reserve(SVD_DIM+1);
        userSVD[i].push_back(0.0);
        for(int p = 1; p < SVD_DIM+1; p++){
            userSVD[i].push_back( init_value );
        }
    }
	for(int i = 0; i < MOVIE_COUNT; i++){
        movieSVD.reserve(SVD_DIM*2+1);
        movieSVD[i].push_back(0.0);
        for(int p = 1; p < SVD_DIM*2+1; p++){
            movieSVD[i].push_back( init_value );
        }
    }

    if(!baseLoaded){
        printf("Loading baseline predictor...\n");
        base_predict.remember(partition);
        baseLoaded = true;
    }
    
    printf("Learning SVD...\n");

    if(!userMoviesGenerated){
        printf("Generating list of movies rated by user...\n");
        userMovies.reserve(USER_COUNT);
        for(int user = 0; user < USER_COUNT; user++){
            userMovies.push_back(vector <int> ());
            userMovies[user].reserve(100);
        }
        for(int point = 0; point < DATA_COUNT; point++){
            userMovies[get_um_all_usernumber(point)-1].push_back(get_um_all_movienumber(point)-1);
        }
        userMoviesGenerated = true;
    }    
    //printf("Sample user rating size: %i\n", (int)userMovies[437].size() );

    printf("Initializing ratings cache...\n");
    ratings.reserve(DATA_COUNT);
    for(int point = 0; point < DATA_COUNT; point++){
        ratings.push_back(base_predict.predict((int)get_um_all_usernumber(point),
                                               (int)get_um_all_movienumber(point),
                                               (int)get_um_all_datenumber(point)));
        ratings[point] += init_value + init_value*pow(userMovies[(int)get_um_all_usernumber(point)-1].size(), 0.5);
        //if(point % 10000000 == 0)     
        //   printf("Sample rating: %f\n", ratings[point]);
    }
    base_predict.free_mem();
    baseLoaded = false;
    
    printf("Learning by SGD...\n");
    int param_start = 0;
    int min_epochs = LEARN_EPOCHS_MIN;
    
    if(refining){
		printf("Refining...\n");
        int svd_pt = 0;
        while(svd_pt < SVD_DIM && userSVD[0][svd_pt] != init_value){
            svd_pt++;
        }
        if(svd_pt != SVD_DIM)
            param_start = svd_pt;
        min_epochs = REFINE_EPOCHS_MIN;
    }
    /* Cycle through dataset */
    for(int p = param_start; p < SVD_DIM; p++){
        printf("\tParameter %u/%u\n", p+1, SVD_DIM);
        int k = 0;
        int point_count;
        double err;
        double errsq;
        double rmse = 10.0;
        double oldrmse = 100.0;
        learn_rate = LEARN_RATE;
        userNSum = 0.0;
        userNNum = -1;
        userNChange = 0.0;
        //if(p == SVD_DIM/2)
        //    min_epochs = min_epochs / 2;
        while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT || k < min_epochs){
            oldrmse = rmse;
            k++;
            errsq = 0.0;
            point_count = 0;
            for(int i = 0; i < DATA_COUNT; i++){
                if(i%10000000 == 0)
                    printf("\t\tEpoch %u progress: %i\n", k, (int)(((double)i/(double)DATA_COUNT)*100));
                if(get_um_idx_ratingset(i) <= partition){
                    err = learn_point(p, get_um_all_usernumber(i)-1,
                                         (int)get_um_all_movienumber(i)-1,
                                         (double)get_um_all_rating(i), i, refining);
                    //if(err != -999){
                        errsq = errsq + err * err;
                        point_count++;
                    //}
                }
            }
            rmse = errsq / ((double)point_count);
            printf("\t\tEpoch %u: RMSE(in): %lf\n", k, sqrt(rmse));
            //printf("\tRMSE(out): %lf\n\n",rmse_probe());
            learn_rate = learn_rate * 0.9;
        }
        printf("\t\tRMSE(out): %lf\n\n",rmse_probe());
        //base_predict.free_mem();
        //baseLoaded = false;
        save_svd(3);
    }

    ratings.clear();
}

double SVDK_Nov17::learn_point(int svd_pt, int user, int movie, double rating, int pt_num, bool refining){
    double err;
    err = rating - ratings[pt_num];

    //Update user and movie biases for this point
    double bias_user_old = userSVD[user][0]; 
    double bias_movie_old = movieSVD[movie][0];
    double bias_user_old_change = learn_rate * (err - BIAS_REGUL_PARAM * bias_user_old);
    double bias_movie_old_change = learn_rate * (err - BIAS_REGUL_PARAM * bias_movie_old);
                 
    userSVD[user][0] += bias_user_old_change;
	movieSVD[movie][0] += bias_movie_old_change;

    //Get matrix factors
    double svd_user_old = userSVD[user][svd_pt+1]; 
    double svd_movie_old = movieSVD[movie][svd_pt+1];

    //Calculate neighborhood factors   
    if(userNNum != user){
        //Update old user
        double norm = sqrt((double)userMovies[userNNum].size());
        for(int y = 0; y < userMovies[userNNum].size(); y++){
            movieSVD[userMovies[userNNum][y]][SVD_DIM+svd_pt+1] += userNChange / norm;
        }
        //Gather information for new user
        userNNum = user;
        userNSum = 0.0;
        userNChange = 0.0;
        for(int y = 0; y < userMovies[user].size(); y++){
            userNSum += movieSVD[userMovies[user][y]][SVD_DIM+svd_pt+1];
        }
        userNSum = userNSum / sqrt((double)userMovies[user].size());   
    }

    //Update matrix factors
    userNChange += learn_rate * (err * svd_movie_old - FEATURE_REGUL_PARAM * userNSum);
    
    double svd_user_old_change = learn_rate * (err * svd_movie_old - FEATURE_REGUL_PARAM * svd_user_old);
	userSVD[user][svd_pt+1] += svd_user_old_change;
    double svd_movie_old_change = learn_rate * (err * (svd_user_old + userNSum + userNChange) - FEATURE_REGUL_PARAM * svd_movie_old);
	movieSVD[movie][svd_pt+1] += svd_movie_old_change;

    ratings[pt_num] += svd_user_old * svd_movie_old_change +
                         svd_movie_old * (svd_user_old_change + userNChange) +
                         (svd_user_old_change + userNChange) * svd_movie_old_change +
                         bias_user_old_change +
                         bias_movie_old_change;
    if(ratings[pt_num] > 5.0)
        ratings[pt_num] = 5.0;
    else if(ratings[pt_num] < 1.0)
        ratings[pt_num] = 1.0;
    return err;
}

void SVDK_Nov17::save_svd(int partition){
    FILE *outFile;
    outFile = fopen(NOV17_SVDK_PARAM_FILE, "w");
    fprintf(outFile,"%u\n",partition);
    for(int user = 0; user < USER_COUNT; user++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fprintf(outFile,"%lf ",userSVD[user][i]); 
        }
        fprintf(outFile,"\n");
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM*2+1; i++) {
            fprintf(outFile,"%lf ",movieSVD[movie][i]);
        }
    }
    fclose(outFile);
    return;
}

void SVDK_Nov17::remember(int partition){
   if(!baseLoaded){
        printf("Loading baseline predictor...\n");
        base_predict.remember(partition);
        baseLoaded = true;
    }

    if(!userMoviesGenerated){
        printf("Generating list of movies rated by user...\n");
        userMovies.reserve(USER_COUNT);
        for(int user = 0; user < USER_COUNT; user++){
            userMovies.push_back(vector <int> ());
            userMovies[user].reserve(100);
        }
        for(int point = 0; point < DATA_COUNT; point++){
            userMovies[get_um_all_usernumber(point)-1].push_back(get_um_all_movienumber(point)-1);
        }
        userMoviesGenerated = true;
    }    

    //Initially all matrix factor elements are set to 0.1
    userSVD.reserve(USER_COUNT);
    movieSVD.reserve(MOVIE_COUNT);
    for(int i = 0; i < USER_COUNT; i++){
        userSVD.reserve(SVD_DIM+1);
        userSVD[i].push_back(0.0);
        for(int p = 1; p < SVD_DIM+1; p++){
            userSVD[i].push_back(0.1);
        }
    }
	for(int i = 0; i < MOVIE_COUNT; i++){
        movieSVD.reserve(SVD_DIM*2+1);
        movieSVD[i].push_back(0.0);
        for(int p = 1; p < SVD_DIM*2+1; p++){
            movieSVD[i].push_back(0.1);
        }
    }

    FILE *inFile;
    inFile = fopen(NOV17_SVDK_PARAM_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    //printf("File opened.\n");
    double g = 0.0;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    for(int user = 0; user < USER_COUNT; user++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fscanf(inFile, "%lf", &g);
            userSVD[user][i] = g;
	    }
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM*2+1; i++) {
            fscanf(inFile, "%lf", &g);
            movieSVD[movie][i] = g;
        }
    }
    fclose(inFile);
    return;

}

void SVDK_Nov17::load_data(){
    assert(load_um_all_usernumber() == 0);
    assert(load_um_all_movienumber() == 0);
    assert(load_um_all_datenumber() == 0);
    assert(load_um_all_rating() == 0);
    assert(load_um_idx_ratingset() == 0);

    data_loaded = true;
}

double SVDK_Nov17::predict(int user, int movie, int time){
    double rating = predict_point(user-1, movie-1, time);
    return rating;
}

double SVDK_Nov17::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
        if(get_um_idx_ratingset(i) == 4 && count < 10000){
            double prediction = predict((int)get_um_all_usernumber(i),
                                        (int)get_um_all_movienumber(i),
                                        (int)get_um_all_datenumber(i));
            double error = (prediction - (double)get_um_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}   

double SVDK_Nov17::predict_point(int user, int movie, int date){
    double rating = base_predict.predict(user+1, movie+1, date) +
                    userSVD[user][0] +
                    movieSVD[movie][0];
    double norm = sqrt((double)userMovies[user].size());
    for (int i = 1; i < SVD_DIM+1; i++){
        double nSum = 0.0;
        for(int y = 0; y < userMovies[user].size(); y++){
            nSum = nSum + movieSVD[userMovies[user][y]][SVD_DIM+i];
        }
        rating = rating + ( userSVD[user][i] + (nSum / norm) ) *
                          ( movieSVD[movie][i]);
    }
    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}
            
        
