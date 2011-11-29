#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include "svdk_nov21.h"
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

SVDK_Nov21::SVDK_Nov21(){
    data_loaded = false;
    userMoviesGenerated = false;
    learn_rate = LEARN_RATE;
    load_data();
    learned_dim = 0;
    //srand(time(NULL));
}

void SVDK_Nov21::free_mem(){
    gsl_matrix_free(ratings);
}

void SVDK_Nov21::learn(int partition){
    assert(data_loaded);
    double init_value =  INIT_SVD_VAL ;

    rmsein.reserve(40 * 100);
    rmseout.reserve(40 * 100);

    userSVD = gsl_matrix_calloc(USER_COUNT, 2);
    movieSVD = gsl_matrix_calloc(MOVIE_COUNT, 3);
    userMovies = vector < vector <int> > ();
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
    printf("Learning SVD...\n");
    if(learned_dim == 0){
        ratings = gsl_matrix_calloc(DATA_COUNT, 1);
        
        printf("Initializing bias cache...\n");
        for(int point = 0; point < DATA_COUNT; point++){
                    
            double rating = AVG_RATING;
            rating += INIT_SVD_VAL * INIT_SVD_VAL * (double)SVD_DIM;

            gsl_matrix_set(ratings, point, 0, rating);
        }
    }
    
    printf("Learning by SGD...\n");
    int param_start = learned_dim;
    int min_epochs = LEARN_EPOCHS_MIN;
    
    /* Cycle through dataset */
    for(int p = param_start; p < SVD_DIM; p++){
        printf("\tParameter %u/%u\n", p+1, SVD_DIM);
    
        for(int i = 0; i < USER_COUNT; i++){
            gsl_matrix_set(userSVD, i, 0, 0.0);
            gsl_matrix_set(userSVD, i, 1, init_value );
        }
	    for(int i = 0; i < MOVIE_COUNT; i++){
            gsl_matrix_set(movieSVD, i, 0, 0.0);
            gsl_matrix_set(movieSVD, i, 1, init_value );
            gsl_matrix_set(movieSVD, i, 2, 0.0 );
        }

        int k = 0;
        int point_count;
        double err;
        double errsq;
        double rmse = 10.0;
        double rmseo = 10.0;
        double oldrmse = 100.0;
        learn_rate = LEARN_RATE*pow(0.95,(double)p);
        //if(p == SVD_DIM/2)
        //    min_epochs = min_epochs / 2;
        while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT || k < min_epochs){
        //while(k < min_epochs){
            oldrmse = rmse;
            k++;
            errsq = 0.0;
            point_count = 0;
            userNSum = 0.0;
            userNNum = -1;
            userNChange = 0.0;
            printf("\t\tEpoch %u: ", k);
            for(int i = 0; i < DATA_COUNT; i++){
                //if(i%25000000 == 0)
                //    printf("\t\t\tProgress: %i percent\n", (int)(((double)i/(double)DATA_COUNT)*100));
                if(get_um_idx_ratingset(i) <= partition){
                    err = learn_point(p, get_um_all_usernumber(i)-1,
                                         (int)get_um_all_movienumber(i)-1,
                                         (double)get_um_all_rating(i), i);
                    //if(err != -999){
                        errsq = errsq + err * err;
                        point_count++;
                    //}
                }
            }
            if(userNNum != -1){
                //Update last user
                double norm = sqrt((double)userMovies[userNNum].size());
                for(int y = 0; y < userMovies[userNNum].size(); y++){
                    gsl_matrix_set(movieSVD, userMovies[userNNum][y], 2,
                               gsl_matrix_get(movieSVD, userMovies[userNNum][y], 2) +
                               userNChange / norm);
                }
            }   
            
            assert(point_count != 0);
            rmse = errsq / ((double)point_count);
            rmsein.push_back(sqrt(rmse));
            printf("\tRMSE(in): %lf\n", sqrt(rmse));
            learn_rate = learn_rate * 0.95;
            //rmseo = rmse;            

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
                    c_userNSum += gsl_matrix_get(movieSVD, userMovies[user][y], 2);
                }
                c_userNSum = c_userNSum / sqrt((double)userMovies[user].size());
            }
                                
            double rating = gsl_matrix_get(ratings, point, 0);
            /*
            rating -= INIT_SVD_VAL * INIT_SVD_VAL;
            rating += gsl_matrix_get(userSVD, user, 0) +
                      gsl_matrix_get(movieSVD, movie, 0);
            rating += ( gsl_matrix_get(userSVD, user, 1) + c_userNSum) *
                      ( gsl_matrix_get(movieSVD, movie, 1) );
            if(rating < 1.0)
                rating = 1.0;
            else if(rating > 5.0)
                rating = 5.0;
            */
            gsl_matrix_set(ratings, point, 0, predict_train(user, movie, rating, c_userNSum));
        }

        rmseo = rmse_probe();
        rmseout.push_back(rmseo);
        printf("\t\t\tRMSE(out): %lf\n",rmseo);

        save(p);
    }
    printf("Done! :)\n");
}

double SVDK_Nov21::learn_point(int svd_pt, int user, int movie, double rating, int pt_num){
    assert(rating == 5 || rating == 4 || rating == 3 || rating == 2 || rating == 1);

    //Calculate neighborhood factors   
    if(userNNum != user || userNNum == -1){
        if(userNNum != -1){
            //Update old user
            double norm = sqrt((double)userMovies[userNNum].size());
            for(int y = 0; y < userMovies[userNNum].size(); y++){
                gsl_matrix_set(movieSVD, userMovies[userNNum][y], 2,
                               gsl_matrix_get(movieSVD, userMovies[userNNum][y], 2) +
                               userNChange / norm);
            }
        }
        //Gather information for new user
        userNNum = user ;
        userNSum = 0.0;
        userNChange = 0.0;
        
        for(int y = 0; y < userMovies[user].size(); y++){
            userNSum += gsl_matrix_get(movieSVD, userMovies[user][y], 2);
        }
        //printf("NSum:%f -> ", userNSum);
        userNSum = userNSum / sqrt((double)userMovies[user].size());
        //printf("%f\n", userNSum);
    }

    //double pred_rating = predict(user+1,movie+1,(int)get_um_all_datenumber(pt_num));
    //double old_pred_rating = pred_rating;

    double err = (double)rating - predict_train(user, movie, gsl_matrix_get(ratings, pt_num, 0), userNSum + userNChange);

    //Update user and movie biases for this point
    double bias_user_old = gsl_matrix_get(userSVD, user, 0);
    double bias_movie_old = gsl_matrix_get(movieSVD, movie, 0);

    double bias_user_old_change = learn_rate * (err - BIAS_REGUL_PARAM * bias_user_old);
    double bias_movie_old_change = learn_rate * (err - BIAS_REGUL_PARAM * bias_movie_old);
             
    gsl_matrix_set(userSVD,user,0, bias_user_old + bias_user_old_change);
	gsl_matrix_set(movieSVD,movie,0, bias_movie_old + bias_movie_old_change);

    

    //Get matrix factors
    double svd_user_old = gsl_matrix_get(userSVD, user, 1);
    double svd_movie_old = gsl_matrix_get(movieSVD, movie, 1);
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
	gsl_matrix_set(userSVD, user, 1, svd_user_old + svd_user_old_change);

    double svd_movie_old_change = learn_rate * (err * svd_user_old_total - FEATURE_REGUL_PARAM * svd_movie_old);
    if(svd_movie_old_change!=svd_movie_old_change)
        printf("svd_movie_old_change nan");
	gsl_matrix_set(movieSVD, movie, 1, svd_movie_old + svd_movie_old_change);

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

void SVDK_Nov21::save(int svd_pt){
    FILE *outFile;
    outFile = fopen(NOV21_RMSE_FILE, "w");
    for(int i = 0; i < rmsein.size(); i++){
        fprintf(outFile, "%lf\t", rmsein[i]);
        fprintf(outFile, "%lf\n", rmseout[i]);
    }
    fclose(outFile);
    outFile = fopen(NOV21_RATINGS_FILE, "w");
    fprintf(outFile,"%i\t%i\n", svd_pt, SVD_DIM);
    for(int i = 0; i < DATA_COUNT; i++){
        fprintf(outFile, "%lf\n", gsl_matrix_get(ratings, i, 0));
    }
    fclose(outFile);
    return;
}

void SVDK_Nov21::remember(int partition){
    assert(data_loaded);

    ratings = gsl_matrix_calloc(DATA_COUNT, 1);

    double init_value = INIT_SVD_VAL;
    
    FILE *inFile;
    printf("Loading ratings list...\n");
    double rating = 0.0;
    inFile = fopen(NOV21_RATINGS_FILE, "r");
    assert(inFile != NULL);
    int svd_pt = 1;
    int svd_dim = 2;
    fscanf(inFile,"%i", &svd_pt);
    fscanf(inFile,"%i", &svd_dim);
    assert(SVD_DIM == svd_dim);
    learned_dim = svd_pt;
    for(int i = 0; i < DATA_COUNT; i++){
        fscanf(inFile, "%lf", &rating);
        if(rating < 1.0)
            rating = 1.0;
        else if(rating > 5.0)
            rating = 5.0;
        gsl_matrix_set(ratings, i, 0, rating);
    }
    fclose(inFile);
    return;
}

void SVDK_Nov21::load_data(){
    assert(load_um_all_usernumber() == 0);
    assert(load_um_all_movienumber() == 0);
    assert(load_um_all_datenumber() == 0);
    assert(load_um_all_rating() == 0);
    assert(load_um_idx_ratingset() == 0);

    data_loaded = true;
}

double SVDK_Nov21::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
        if(get_um_idx_ratingset(i) == 4){
            double prediction = gsl_matrix_get(ratings, i, 0);
            double error = prediction - (double)get_um_all_rating(i);
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}   

double SVDK_Nov21::predict(int user, int movie, int time, int index){
    return gsl_matrix_get(ratings, index, 0);
}

double SVDK_Nov21::predict_point(int user, int movie, int date){
    assert(false); //should not be called
    double rating = AVG_RATING +
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


double SVDK_Nov21::predict_train(int user, int movie, double bias, double nSum){
    double rating = bias - INIT_SVD_VAL * INIT_SVD_VAL;
                    gsl_matrix_get(userSVD, user, 0) +
                    gsl_matrix_get(movieSVD, movie, 0);
                    
    rating += ( gsl_matrix_get(userSVD, user, 1) + nSum) *
              ( gsl_matrix_get(movieSVD, movie, 1) );
    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}
            
        
