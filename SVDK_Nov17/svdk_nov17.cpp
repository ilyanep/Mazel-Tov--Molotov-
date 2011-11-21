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
 * (*userSVD): [bias factor0 factor1 ... factorN]
 * (*movieSVD): [bias factor0 factor1 ... factorN nfactor0 nfactor1 ... nfactorN]
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
    gsl_matrix_free(userSVD);
    gsl_matrix_free(movieSVD);
    gsl_matrix_free(ratings);
    userMovies.clear();
    base_predict.free_mem();
    baseLoaded = false;
}

void SVDK_Nov17::learn(int partition){
    learn(partition, false);
}

void SVDK_Nov17::learn(int partition, bool refining){
    assert(data_loaded);
    double init_value =  INIT_SVD_VAL ;

    rmsein.reserve(40 * 64);
    rmseout.reserve(40 * 64);

    if(!refining){
         //Initially all matrix factor elements are set to 0.1
        userMovies = vector < vector <int> > ();

        userSVD = gsl_matrix_calloc(USER_COUNT, SVD_DIM+1);
        movieSVD = gsl_matrix_calloc(MOVIE_COUNT, SVD_DIM*2+1);
        ratings = gsl_matrix_calloc(DATA_COUNT, 1);

        for(int i = 0; i < USER_COUNT; i++){
            for(int p = 1; p < SVD_DIM+1; p++){
                gsl_matrix_set(userSVD, i, p, init_value );
            }
        }
	    for(int i = 0; i < MOVIE_COUNT; i++){
            for(int p = 1; p < SVD_DIM+1; p++){
                gsl_matrix_set(movieSVD, i, p, init_value );
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


        printf("Initializing bias cache...\n");
        for(int point = 0; point < DATA_COUNT; point++){
                    
            double rating = base_predict.predict((int)get_um_all_usernumber(point),
                                                 (int)get_um_all_movienumber(point),
                                                 (int)get_um_all_datenumber(point));

            //double rating = AVG_RATING;
            gsl_matrix_set(ratings, point, 0, rating);
        }
    }
   // for(int user = 1424; user < 1460; user++){
  //      printf("user %u: usvd1: %f, msvd1: %f\n", user, gsl_matrix_get(userSVD, user, 1), gsl_matrix_get(movieSVD, user, 1));
  //  }
    //base_predict.free_mem();
    //baseLoaded = false;
    
    printf("Learning by SGD...\n");
    int param_start = 0;
    int min_epochs = LEARN_EPOCHS_MIN;
    
    if(refining){
		printf("Refining...\n");
        int svd_pt = 0;
        while(svd_pt < SVD_DIM && gsl_matrix_get(userSVD, 0, svd_pt+1) != init_value){
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
        double rmseo = 10.0;
        double oldrmse = 100.0;
        userNSum = 0.0;
        userNNum = -1;
        userNChange = 0.0;
        learn_rate = LEARN_RATE;
        //if(p == SVD_DIM/2)
        //    min_epochs = min_epochs / 2;
        while(oldrmse - rmseo > MIN_RMSE_IMPROVEMENT || k < min_epochs){
        //while(k < min_epochs){
            oldrmse = rmseo;
            k++;
            errsq = 0.0;
            point_count = 0;
            printf("\t\tEpoch %u: ", k);
            for(int i = 0; i < DATA_COUNT; i++){
                //if(i%25000000 == 0)
                //    printf("\t\t\tProgress: %i percent\n", (int)(((double)i/(double)DATA_COUNT)*100));
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
            assert(point_count != 0);
            rmse = errsq / ((double)point_count);
            rmsein.push_back(sqrt(rmse));
            printf("\tRMSE(in): %lf", sqrt(rmse));
            //printf("\tRMSE(out): %lf\n\n",rmse_probe());
            learn_rate = learn_rate * 0.9;
            rmseo = rmse_probe();
            rmseout.push_back(rmseo);
            printf("\t RMSE(out): %lf\n",rmseo);
            }

        printf("\t\t\tCaching learned parameters...\n");
        int c_userNNum = -1;
        double c_userNSum = 0.0;
        for(int point = 0; point < DATA_COUNT; point++){
            int user = get_um_all_usernumber(point)-1;
            int movie = (int)get_um_all_movienumber(point)-1;
            if(c_userNNum != user){
                //Gather information for new user
                c_userNNum = user;
                c_userNSum = 0.0;
                
                for(int y = 0; y < userMovies[user].size(); y++){
                    c_userNSum += gsl_matrix_get(movieSVD, userMovies[user][y], SVD_DIM+p+1);
                }
                c_userNSum = c_userNSum / sqrt((double)userMovies[user].size());
            }
                                
            double rating = gsl_matrix_get(ratings, point, 0);
            rating += ( gsl_matrix_get(userSVD, user, p + 1) + c_userNSum) *
                      ( gsl_matrix_get(movieSVD, movie, p + 1) );
            gsl_matrix_set(ratings, point, 0, rating);
        }
        //printf("\t\tRMSE(out): %lf\n\n",rmse_probe());
        //base_predict.free_mem();
        //baseLoaded = false;
        save_svd(partition);
    }
    printf("Done! :)\n");
}

double SVDK_Nov17::learn_point(int svd_pt, int user, int movie, double rating, int pt_num, bool refining){
    assert(rating == 5 || rating == 4 || rating == 3 || rating == 2 || rating == 1);

    //Calculate neighborhood factors   
    if(userNNum != user || userNNum == -1){
        if(userNNum != -1){
            //Update old user
            double norm = sqrt((double)userMovies[userNNum].size());
            for(int y = 0; y < userMovies[userNNum].size(); y++){
                gsl_matrix_set(movieSVD, userMovies[userNNum][y], SVD_DIM+svd_pt+1,
                               gsl_matrix_get(movieSVD, userMovies[userNNum][y], SVD_DIM+svd_pt+1) +
                               userNChange / norm);
            }
        }
        //Gather information for new user
        userNNum = user ;
        userNSum = 0.0;
        userNChange = 0.0;
        
        for(int y = 0; y < userMovies[user].size(); y++){
            userNSum += gsl_matrix_get(movieSVD, userMovies[user][y], SVD_DIM+svd_pt+1);
        }
        //printf("NSum:%f -> ", userNSum);
        userNSum = userNSum / sqrt((double)userMovies[user].size());
        //printf("%f\n", userNSum);
    }

    //double pred_rating = predict(user+1,movie+1,(int)get_um_all_datenumber(pt_num));
    //double old_pred_rating = pred_rating;

    double err = (double)rating - predict_train(user, movie, gsl_matrix_get(ratings, pt_num, 0), svd_pt, userNSum + userNChange);

    //Update user and movie biases for this point
    double bias_user_old = gsl_matrix_get(userSVD, user , 0);
    double bias_movie_old = gsl_matrix_get(movieSVD, movie, 0);

    double bias_user_old_change = learn_rate * (err - BIAS_REGUL_PARAM * bias_user_old);
    double bias_movie_old_change = learn_rate * (err - BIAS_REGUL_PARAM * bias_movie_old);
             
    gsl_matrix_set(userSVD,user,0, bias_user_old + bias_user_old_change);
	gsl_matrix_set(movieSVD,movie,0, bias_movie_old + bias_movie_old_change);

    

    //Get matrix factors
    double svd_user_old = gsl_matrix_get(userSVD, user, svd_pt+1);
    double svd_movie_old = gsl_matrix_get(movieSVD, movie, svd_pt+1);
    double svd_user_old_total = svd_user_old + userNSum + userNChange; //Running total before this point
 
    //Update matrix factors
    double norm = sqrt((double)userMovies[user].size());
    //double userNChange_change = learn_rate * (err * svd_movie_old - FEATURE_REGUL_PARAM * (userNSum + userNChange));
    double userNChange_change = learn_rate * (err * svd_movie_old * norm - FEATURE_REGUL_PARAM * (userNSum + userNChange));
    if(userNChange_change!=userNChange_change)
        printf("userNChange nan");
    userNChange += userNChange_change;
    
    double svd_user_old_change = learn_rate * (err * svd_movie_old - FEATURE_REGUL_PARAM * svd_user_old);
    if(svd_user_old_change!=svd_user_old_change)
        printf("svd_user_old_change nan");
	gsl_matrix_set(userSVD, user, svd_pt+1, svd_user_old + svd_user_old_change);

    double svd_movie_old_change = learn_rate * (err * svd_user_old_total - FEATURE_REGUL_PARAM * svd_movie_old);
    if(svd_movie_old_change!=svd_movie_old_change)
        printf("svd_movie_old_change nan");
	gsl_matrix_set(movieSVD, movie, svd_pt+1, svd_movie_old + svd_movie_old_change);

    //pred_rating = predict(user+1,movie+1,(int)get_um_all_datenumber(pt_num));    
    
    /*if(pt_num == 316620){
        printf("Params: usvd: %f, msvd: %f, nsvd: %f, ubias: %f, mbias: %f\n", svd_user_old, svd_movie_old, userNSum + userNChange, bias_user_old, bias_movie_old);
        printf("AFTER: user %i, movie: %i, real rating %f, point %i\n", user+1, movie+1, rating, pt_num);
    }*/




    if(err!=err){
        printf("Nan at user %i, point %i\n", user+1, pt_num);
        printf("Params: usvd: %f, msvd: %f, nsvd: %f, ubias: %f, mbias: %f\n", svd_user_old, svd_movie_old, userNSum + userNChange, bias_user_old, bias_movie_old);
        exit(1);
    }
    /*
    double err2 = rating - pred_rating;
    if (pt_num == 316620){
        printf("BEFORE: user %i, movie: %i, real rating %f, current rating %f, point %i\n", user+1, movie+1, rating, old_pred_rating, pt_num);
        printf("Params: usvd: %f, msvd: %f, nsvd: %f, ubias: %f, mbias: %f\n", svd_user_old, svd_movie_old, userNSum + userNChange, bias_user_old, bias_movie_old);
        printf("AFTER: user %i, movie: %i, real rating %f, current rating %f, point %i\n\n", user+1, movie+1, rating, pred_rating, pt_num);
        //printf("fail\n");
        //exit(1);
    }*/
    return err;
}

void SVDK_Nov17::save_svd(int partition){
    FILE *outFile;
    outFile = fopen(NOV17_SVDK_PARAM_FILE, "w");
    fprintf(outFile,"%u\n",partition);
    for(int user = 0; user < USER_COUNT; user++){
        for(int i = 0; i < SVD_DIM+1; i++) {
            fprintf(outFile,"%lf ",gsl_matrix_get(userSVD, user, i)); 
        }
        fprintf(outFile,"\n");
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM*2+1; i++) {
            fprintf(outFile,"%lf ",gsl_matrix_get(movieSVD, movie, i));
        }
    }
    fclose(outFile);
    outFile = fopen(NOV17_RMSE_FILE, "w");
    for(int i = 0; i < rmsein.size(); i++){
        fprintf(outFile, "%lf\t", rmsein[i]);
        fprintf(outFile, "%lf\n", rmseout[i]);
    }
    fclose(outFile);
    outFile = fopen(NOV17_RATINGS_FILE, "w");
    for(int i = 0; i < DATA_COUNT; i++){
        fprintf(outFile, "%lf\n", gsl_matrix_get(ratings, i, 0));
    }
    fclose(outFile);
    return;
}

void SVDK_Nov17::remember(int partition){
    assert(data_loaded);
    if(!baseLoaded){
        printf("Loading baseline predictor...\n");
        base_predict.remember(partition);
        baseLoaded = true;
    }

    userMovies = vector < vector <int> > ();
    userSVD = gsl_matrix_calloc(USER_COUNT, SVD_DIM+1);
    movieSVD = gsl_matrix_calloc(MOVIE_COUNT, SVD_DIM*2+1);
    ratings = gsl_matrix_calloc(DATA_COUNT, 1);

    double init_value = INIT_SVD_VAL;
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
    printf("Loading SVD matrices...\n");
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
            gsl_matrix_set(userSVD, user, i, g);
	    }
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM*2+1; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(movieSVD, movie, i, g);
        }
    }
    fclose(inFile);
    printf("Loading ratings list...\n");
    double rating = 0.0;
    inFile = fopen(NOV17_RATINGS_FILE, "r");
    for(int i = 0; i < DATA_COUNT; i++){
        fscanf(inFile, "%lf", &rating);
        gsl_matrix_set(ratings, i, 0, rating);
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

double SVDK_Nov17::predict(int user, int movie, int time){
    double rating = predict_point(user-1, movie-1, time);
    return rating;
}

double SVDK_Nov17::predict_point(int user, int movie, int date){
    double rating = base_predict.predict(user+1, movie+1, date) +
                    gsl_matrix_get(userSVD, user, 0) +
                    gsl_matrix_get(movieSVD, movie, 0);
    double norm = sqrt((double)userMovies[user].size());
    for (int i = 1; i < SVD_DIM+1; i++){
        double nSum = 0.0;
        for(int y = 0; y < userMovies[user].size(); y++){
            nSum = nSum + gsl_matrix_get(movieSVD, userMovies[user][y], SVD_DIM+i);
        }
        rating = rating + ( gsl_matrix_get(userSVD, user, i) + (nSum / norm) ) *
                          ( gsl_matrix_get(movieSVD, movie, i) );
    }
    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}


double SVDK_Nov17::predict_train(int user, int movie, double bias, int svd_pt, double nSum){
    double rating = bias +
                    gsl_matrix_get(userSVD, user, 0) +
                    gsl_matrix_get(movieSVD, movie, 0);
    double norm = sqrt((double)userMovies[user].size());
/*   for (int i = 1; i <= svd_pt+1; i++){
        rating += ( gsl_matrix_get(userSVD, user, i) + nSum) *
                  ( gsl_matrix_get(movieSVD, movie, i) );
    }*/
    rating += ( gsl_matrix_get(userSVD, user, svd_pt + 1) + nSum) *
              ( gsl_matrix_get(movieSVD, movie, svd_pt + 1) );
    rating += INIT_SVD_VAL * INIT_SVD_VAL * (SVD_DIM - svd_pt -1);
    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}
            
        
