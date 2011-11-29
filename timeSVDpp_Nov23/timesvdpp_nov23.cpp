#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include "timesvdpp_nov23.h"
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

timeSVDpp_Nov23::timeSVDpp_Nov23(){
    data_loaded = false;
    learn_rate = LEARN_RATE;
    load_data();
    learned_dim = 0;
    //srand(time(NULL));
}

void timeSVDpp_Nov23::free_mem(){
    gsl_matrix_free(ratings);
}

void timeSVDpp_Nov23::learn(int partition){
    assert(data_loaded);
    rmsein.reserve(40 * 100);
    rmseout.reserve(40 * 100);
    if(learned_dim == 0){
        freqNum = vector <vector <int> >(); //[user][freq]
        freqDates = vector <vector <int> >(); //[user][date]
        avgRatingDates = gsl_matrix_calloc(USER_COUNT, 1);
        userFreqSpikes = gsl_matrix_calloc(USER_COUNT, NUM_USER_TIME_FACTORS);

        printf("Initializing temporal dynamics...\n");
        generate_frequency_table(partition);
        generate_freq_spikes();
        generate_avg_dates(partition);
    }

    printf("Learning SVD...\n");
    if(learned_dim == 0){
        ratings = gsl_matrix_calloc(DATA_COUNT, 3);
    }
        
    printf("Initializing bias cache...\n");
    for(int point = 0; point < DATA_COUNT; point++){
                
        double rating = AVG_RATING;
        rating += INIT_SVD_VAL * INIT_SVD_VAL * (double)SVD_DIM;
         
        //Check if user has a spike at that moment
        int freqdate = 0;
        int datePoint = 0;
        double spikeFactor = 0.0;
        bool found = false;
        int spikeDate;
        int user = get_um_all_usernumber(point)-1;
        int date = get_um_all_datenumber(point);
        while(!found && datePoint < NUM_USER_TIME_FACTORS){
            freqdate = gsl_matrix_get(userFreqSpikes, user, datePoint);
            if(freqdate == date)
                found = true;
            else
                datePoint++;
        }
        if(found){
            spikeDate = datePoint;
        }else{
            spikeDate = -1;
        }
        int dateIndex = find_element_vect(freqDates[user], date);
        double rateFreq;
        if(dateIndex == -1)
            rateFreq = 0.0;
        else
            rateFreq = log((double)freqNum[user][dateIndex]) / LN_LOG_BASE;
        if (rateFreq > 3.0)
            rateFreq = 3.0;
        if(learned_dim == 0){
            gsl_matrix_set(ratings, point, 0, rating);
        }
        gsl_matrix_set(ratings, point, 1, spikeDate);
        gsl_matrix_set(ratings, point, 2, (int)(20 * rateFreq));
    }
    
    gsl_matrix_free(userFreqSpikes);
    freqNum.clear();
    freqDates.clear();

    //Initializing baseline stuff
    userBias = gsl_matrix_calloc(USER_COUNT, 3 + NUM_USER_TIME_FACTORS * 2);
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, 1 + NUM_MOVIE_BINS + FREQ_LOG_MAX);
    //Set multiplicative factor to 1.0
    for (int i = 0; i < USER_COUNT; i++)
        gsl_matrix_set(userBias, i, 2, 1.0);

    
    //Initializing SVD++
    double init_value =  INIT_SVD_VAL;

    userSVD = gsl_matrix_calloc(USER_COUNT, 2 + NUM_USER_TIME_FACTORS);
    movieSVD = gsl_matrix_calloc(MOVIE_COUNT, 2 + NUM_MOVIE_BINS + FREQ_LOG_MAX);
    userMovies = vector < vector <int> > ();
    
    printf("Generating list of movies rated by user...\n");
    userMovies.reserve(USER_COUNT);
    for(int user = 0; user < USER_COUNT; user++){
        userMovies.push_back(vector <int> ());
        userMovies[user].reserve(100);
    }
    for(int point = 0; point < DATA_COUNT; point++){
        userMovies[get_um_all_usernumber(point)-1].push_back(get_um_all_movienumber(point)-1);
    }

    printf("Learning by SGD...\n");
    int param_start = learned_dim;
    int min_epochs = LEARN_EPOCHS_MIN;
    
    /* Cycle through dataset */
    for(int p = param_start; p < SVD_DIM; p++){
        printf("\tParameter %u/%u\n", p+1, SVD_DIM);
    
        for(int i = 0; i < USER_COUNT; i++){
            gsl_matrix_set(userSVD, i, 0, init_value);
            gsl_matrix_set(userSVD, i, 1, 0.0 );
            for(int s = 0; s < NUM_USER_TIME_FACTORS; s++){
                gsl_matrix_set(userSVD, i, 2 + s, 0.0);
            }
        }
	    for(int i = 0; i < MOVIE_COUNT; i++){
            gsl_matrix_set(movieSVD, i, 0, 0.0);
            gsl_matrix_set(movieSVD, i, 1, init_value );
            for(int s = 0; s < NUM_MOVIE_BINS + FREQ_LOG_MAX; s++){
                gsl_matrix_set(movieSVD, i, 2 + s, 0.0);
            }
        }

        int k = 0;
        int point_count;
        double err;
        double errsq;
        double rmse = 10.0;
        double rmseo = 10.0;
        double oldrmse = 100.0;
        learn_rate = LEARN_RATE*pow(0.9,(double)p);
        //if(p == SVD_DIM/2)
        //    min_epochs = min_epochs / 2;
        //while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT || k < min_epochs){
        while(k < min_epochs){
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
                                         (int)get_um_all_datenumber(i),
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
            rmseout.push_back(rmseo);
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
            int date = (int)get_um_all_datenumber(point);
            if(c_userNNum != user){
                //Gather information for new user
                c_userNNum = user;
                c_userNSum = 0.0;
                
                for(int y = 0; y < userMovies[user].size(); y++){
                    c_userNSum += gsl_matrix_get(movieSVD, userMovies[user][y], 0);
                }
                c_userNSum = c_userNSum / sqrt((double)userMovies[user].size());
            }
                                
            //Calculate SVD++ addition
            double rating = gsl_matrix_get(ratings, point, 0);
            rating = rating - INIT_SVD_VAL * INIT_SVD_VAL;

            int movieBin = (date - 1) / MOVIE_BIN_SIZE;
            int spikeDate = gsl_matrix_get(ratings, point, 1);
            int rateFreq = gsl_matrix_get(ratings, point, 2);
            double user_avgdate = gsl_matrix_get(avgRatingDates, user, 0);
            double dateFactor;
            if((double)date >= user_avgdate)
                dateFactor = pow((double)date - user_avgdate, USER_DATE_EXP);
            else
                dateFactor = -1.0*pow(user_avgdate - (double)date, USER_DATE_EXP);

            double svd_user_intercept = gsl_matrix_get(userSVD, user, 0);
            double svd_user_slope = gsl_matrix_get(userSVD, user, 1);
            double svd_spikeFactor = 0.0;
            if(spikeDate != -1){
                svd_spikeFactor = gsl_matrix_get(userSVD, user, 2 + spikeDate);
            }
            double svd_userFactor = svd_user_intercept + svd_user_slope * dateFactor + svd_spikeFactor + c_userNSum;

            double svd_movieFactor = gsl_matrix_get(movieSVD, movie, 1) + 
                                     gsl_matrix_get(movieSVD, movie, 2 + movieBin) +
                                     gsl_matrix_get(movieSVD, movie, 2 + NUM_MOVIE_BINS + rateFreq);               
            rating += svd_userFactor * svd_movieFactor;            

            gsl_matrix_set(ratings, point, 0, rating);
        }

        rmseo = rmse_probe();
        rmseout.push_back(rmseo);
        printf("\t\t\tRMSE(out): %lf\n",rmseo);

        save(p);
    }
    gsl_matrix_free(userSVD);
    gsl_matrix_free(movieSVD);
    gsl_matrix_free(userBias);
    gsl_matrix_free(movieBias);
    gsl_matrix_free(avgRatingDates);
    userMovies.clear();
    printf("Done! :)\n");
}

double timeSVDpp_Nov23::learn_point(int svd_pt, int user, int movie, int date, double rating, int pt_num){
    assert(rating == 5 || rating == 4 || rating == 3 || rating == 2 || rating == 1);

    //Calculate neighborhood factors   
    if(userNNum != user || userNNum == -1){
        if(userNNum != -1){
            //Update old user
            double norm = sqrt((double)userMovies[userNNum].size());
            for(int y = 0; y < userMovies[userNNum].size(); y++){
                gsl_matrix_set(movieSVD, userMovies[userNNum][y], 0,
                               gsl_matrix_get(movieSVD, userMovies[userNNum][y], 0) +
                               userNChange / norm);
            }
        }
        //Gather information for new user
        userNNum = user ;
        userNSum = 0.0;
        userNChange = 0.0;
        
        for(int y = 0; y < userMovies[user].size(); y++){
            userNSum += gsl_matrix_get(movieSVD, userMovies[user][y], 0);
        }
        //printf("NSum:%f -> ", userNSum);
        userNSum = userNSum / sqrt((double)userMovies[user].size());
        //printf("%f\n", userNSum);
    }

    int movieBin = (date - 1) / MOVIE_BIN_SIZE;
    int spikeDate = gsl_matrix_get(ratings, pt_num, 1);
    int rateFreq = gsl_matrix_get(ratings, pt_num, 2);
    double user_avgdate = gsl_matrix_get(avgRatingDates, user, 0);
    double dateFactor;
    if((double)date >= user_avgdate)
        dateFactor = pow((double)date - user_avgdate, USER_DATE_EXP);
    else
        dateFactor = -1.0*pow(user_avgdate - (double)date, USER_DATE_EXP);

    double err = (double)rating - predict_train(user, movie, 
                                                dateFactor, movieBin, spikeDate, rateFreq, 
                                                gsl_matrix_get(ratings, pt_num, 0), userNSum + userNChange);

    //UPDATE BASELINE
    double bu = gsl_matrix_get(userBias, user, 0);
    double but;
    double au = gsl_matrix_get(userBias, user, 1);
    double bi = gsl_matrix_get(movieBias, movie, 0);
    double bit = gsl_matrix_get(movieBias, movie, 1 + movieBin);
    double bif = gsl_matrix_get(movieBias, movie, 1 + NUM_MOVIE_BINS + rateFreq);
    double cu = gsl_matrix_get(userBias, user, 2);
    double cut;
    if(spikeDate != -1){
        but = gsl_matrix_get(userBias, user, 3+spikeDate);
        cut = gsl_matrix_get(userBias, user, 3+NUM_USER_TIME_FACTORS+spikeDate);
    }else{
       but = 0.0;
       cut = 0.0;
    }

    double change_bu = LEARN_RATE_BU * (err - REGUL_BU * bu);
    gsl_matrix_set(userBias, user, 0, bu + change_bu);

    double change_au = LEARN_RATE_AU * (err * dateFactor - REGUL_AU * au);
    gsl_matrix_set(userBias, user, 1, au + change_au);
    
    double change_bi = LEARN_RATE_BI * (err * (cu + cut) - REGUL_BI * bi);
    gsl_matrix_set(movieBias, movie, 0, bi + change_bi);

    double change_bit = LEARN_RATE_BIT * (err * (cu + cut) - REGUL_BIT * bit);
    gsl_matrix_set(movieBias, movie, 1 + movieBin, bit + change_bit);
                
    double change_bif = LEARN_RATE_BIF * (err - REGUL_BIF * bif);
    gsl_matrix_set(movieBias, movie, 1 + NUM_MOVIE_BINS + rateFreq, bif + change_bif);

    double change_cu = LEARN_RATE_CU * (err * (bi + bit) - REGUL_CU * (cu - 1.0));
    gsl_matrix_set(userBias, user, 2, cu + change_cu);

    if(spikeDate != -1){
        double change_but = LEARN_RATE_BUT * (err - REGUL_BU * but);
        gsl_matrix_set(userBias, user, 3+spikeDate, but + change_but);

        double change_cut = LEARN_RATE_CUT * (err * (bi + bit) - REGUL_CUT * cut);
        gsl_matrix_set(userBias, user, 3+NUM_USER_TIME_FACTORS+spikeDate, cut + change_cut);
    }

    //UPDATE SVD++ FACTORS
    double su = gsl_matrix_get(userSVD, user, 0);
    double sut = gsl_matrix_get(userSVD, user, 1);
    double sutS;
    if(spikeDate != -1)
        sutS = gsl_matrix_get(userSVD, user, 2 + spikeDate);
    else
        sutS = 0.0;
    double suTOTAL = su + sut * dateFactor + sutS + userNSum + userNChange;

    double si = gsl_matrix_get(movieSVD, movie, 1);
    double sit = gsl_matrix_get(movieSVD, movie, 2 + movieBin);
    double sif = gsl_matrix_get(movieSVD, movie, 2 + NUM_MOVIE_BINS + rateFreq);
    double siTOTAL = si + sit + sif;

    double norm = sqrt((double)userMovies[user].size());
    double userNChange_change = learn_rate * (err * siTOTAL * norm - FEATURE_REGUL_PARAM * (userNSum + userNChange));
    userNChange += userNChange_change;
    
    double change_su = learn_rate * (err * siTOTAL - FEATURE_REGUL_PARAM * su);
	gsl_matrix_set(userSVD, user, 0, su + change_su);

    double change_sut = learn_rate * (err * siTOTAL * dateFactor - FEATURE_REGUL_PARAM * sut);
	gsl_matrix_set(userSVD, user, 1, sut + change_sut);

    if(spikeDate != -1){
        double change_sutS = learn_rate * (err * siTOTAL - FEATURE_REGUL_PARAM * sutS);
	    gsl_matrix_set(userSVD, user, 2 + spikeDate, sutS + change_sutS);
    }

    double change_si = learn_rate * (err * suTOTAL - FEATURE_REGUL_PARAM * si);
	gsl_matrix_set(movieSVD, movie, 1, si + change_si);

    double change_sit = learn_rate * (err * suTOTAL - FEATURE_REGUL_PARAM * si);
	gsl_matrix_set(movieSVD, movie, 2 + movieBin, sit + change_sit);

    double change_sif = learn_rate * (err * suTOTAL - FEATURE_REGUL_PARAM * sif);
	gsl_matrix_set(movieSVD, movie, 2 + NUM_MOVIE_BINS + rateFreq, sif + change_sif);

    //Some error checking
    if(err!=err){
        printf("Nan at user %i, point %i\n", user+1, pt_num);
        printf("Params: usvd: (%f,%f,%f), msvd: (%f,%f,%f), nsvd: %f, ubias: (%f,%f,%f), mbias: (%f,%f,%f), dateFactor: %f\n", su, sut, sutS, 
                                                                                                               si, sit, sif,
                                                                                                               userNSum + userNChange, 
                                                                                                               bu, au, but,
                                                                                                               bi, bit, bif,
                                                                                                               dateFactor);
        exit(1);
    }
    return err;
}

void timeSVDpp_Nov23::save(int svd_pt){
    FILE *outFile;
    outFile = fopen(NOV23_RMSE_FILE, "a");
    for(int i = 0; i < rmsein.size(); i++){
        fprintf(outFile, "%lf\t", rmsein[i]);
        fprintf(outFile, "%lf\n", rmseout[i]);
    }
    fclose(outFile);

    outFile = fopen(NOV23_RATINGS_FILE, "w");
    fprintf(outFile,"%i\t%i\n", svd_pt, SVD_DIM);
    for(int i = 0; i < DATA_COUNT; i++){
        fprintf(outFile, "%lf\n", gsl_matrix_get(ratings, i, 0));
    }
    fclose(outFile);

    outFile = fopen(NOV23_BASELINE_FILE, "w");
    double intercept, slope, multfact, spike, multspike;
    for(int user = 0; user < USER_COUNT; user++){
        intercept = gsl_matrix_get(userBias, user, 0);
        slope = gsl_matrix_get(userBias, user, 1);
        multfact = gsl_matrix_get(userBias, user, 2);
        fprintf(outFile,"%lf\t",intercept);
        fprintf(outFile,"%lf\t",slope);
        fprintf(outFile,"%lf\t",multfact);
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            spike = gsl_matrix_get(userBias, user, 
                        3+spikedate);
            fprintf(outFile,"%lf\t",spike);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            multspike = gsl_matrix_get(userBias, user, 
                        3+NUM_USER_TIME_FACTORS +spikedate);
            fprintf(outFile,"%lf\t",multspike);
        }
        fprintf(outFile,"\n");
    }
    double bias, binbias, freqfact;
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        bias = gsl_matrix_get(movieBias, movie, 0);
        fprintf(outFile,"%lf\t",bias);
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            binbias = gsl_matrix_get(movieBias, movie, 1+bin);	
            fprintf(outFile,"%lf\t",binbias);
        }
        for(int freqIndex = 0; freqIndex < FREQ_LOG_MAX; freqIndex++){
            freqfact = gsl_matrix_get(movieBias, movie, 1 + NUM_MOVIE_BINS + freqIndex);	
            fprintf(outFile,"%lf\t",freqfact);
        }
        fprintf(outFile,"\n");
    }
    fclose(outFile);
    return;
}

void timeSVDpp_Nov23::remember(int partition){
    assert(data_loaded);

    ratings = gsl_matrix_calloc(DATA_COUNT, 3);

    double init_value = INIT_SVD_VAL;
    
    FILE *inFile;
    printf("Loading ratings list...\n");
    double rating = 0.0;
    inFile = fopen(NOV23_RATINGS_FILE, "r");
    int svd_pt = 1;
    int svd_dim = 2;
    fscanf(inFile,"%i", &svd_pt);
    fscanf(inFile,"%i", &svd_dim);
    assert(SVD_DIM == svd_dim);
    learned_dim = svd_pt+1;
    for(int i = 0; i < DATA_COUNT; i++){
        fscanf(inFile, "%lf", &rating);
        if(rating < 1.0)
            rating = 1.0;
        else if(rating > 5.0)
            rating = 5.0;
        gsl_matrix_set(ratings, i, 0, rating);
    }
    fclose(inFile);

    printf("Loading baseline...\n");    
    //Initializing baseline stuff
    userBias = gsl_matrix_calloc(USER_COUNT, 3 + NUM_USER_TIME_FACTORS * 2);
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, 1 + NUM_MOVIE_BINS + FREQ_LOG_MAX);

    freqNum = vector <vector <int> >(); //[user][freq]
    freqDates = vector <vector <int> >(); //[user][date]
    avgRatingDates = gsl_matrix_calloc(USER_COUNT, 1);
    userFreqSpikes = gsl_matrix_calloc(USER_COUNT, NUM_USER_TIME_FACTORS);

    printf("Initializing temporal dynamics...\n");
    generate_frequency_table(partition);
    generate_freq_spikes();
    generate_avg_dates(partition);

    inFile = fopen(NOV23_BASELINE_FILE, "r");
    assert(inFile != NULL);
    double intercept, slope, multfact, spike, multspike;
    for(int user = 0; user < USER_COUNT; user++){
        fscanf(inFile,"%lf",&intercept);
        fscanf(inFile,"%lf",&slope);
        fscanf(inFile,"%lf",&multfact);
        gsl_matrix_set(userBias, user, 0, intercept);
        gsl_matrix_set(userBias, user, 1, slope);
        gsl_matrix_set(userBias, user, 2, multfact);
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&spike);
            gsl_matrix_set(userBias, user, 
                        3 + spikedate, spike);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&multspike);
            gsl_matrix_set(userBias, user, 
                        3 + NUM_USER_TIME_FACTORS + spikedate, multspike);
        }
    }
    double bias, binbias, freqfact;
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        fscanf(inFile,"%lf",&bias);
        gsl_matrix_set(movieBias, movie, 0, bias);
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            fscanf(inFile,"%lf",&binbias);
            gsl_matrix_set(movieBias, movie, 1+bin, binbias);	
        }
        for(int freqIndex = 0; freqIndex < FREQ_LOG_MAX; freqIndex++){
            fscanf(inFile,"%lf",&freqfact);
            gsl_matrix_set(movieBias, movie, 1 + NUM_MOVIE_BINS + freqIndex, freqfact);	
        }
    }
    fclose(inFile);
    return;
}

void timeSVDpp_Nov23::load_data(){
    assert(load_um_all_usernumber() == 0);
    assert(load_um_all_movienumber() == 0);
    assert(load_um_all_datenumber() == 0);
    assert(load_um_all_rating() == 0);
    assert(load_um_idx_ratingset() == 0);

    data_loaded = true;
}

double timeSVDpp_Nov23::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
        if(get_um_idx_ratingset(i) == 4){
            int user = get_um_all_usernumber(i)-1;
            int movie = (int)get_um_all_movienumber(i)-1;
            int date = (int)get_um_all_datenumber(i);           
            
            double err = (double)get_um_all_rating(i) - 
                                predict_point(user, movie, date, i);
            RMSE = RMSE + (err * err);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}   

double timeSVDpp_Nov23::predict(int user, int movie, int time, int index){
    double rating = predict_point(user-1, movie-1, time, index);
    return rating;
}

double timeSVDpp_Nov23::predict_point(int user, int movie, int date, int allIndex){
    int movieBin = (date - 1) / MOVIE_BIN_SIZE;
    int spikeDate = gsl_matrix_get(ratings, allIndex, 1);
    int rateFreq = gsl_matrix_get(ratings, allIndex, 2);
    double user_avgdate = gsl_matrix_get(avgRatingDates, user, 0);
    double dateFactor;
    if((double)date >= user_avgdate)
        dateFactor = pow((double)date - user_avgdate, USER_DATE_EXP);
    else
        dateFactor = -1.0*pow(user_avgdate - (double)date, USER_DATE_EXP);
    
    //Calculate baseline addition
    //Calculate movie factor from bin
    double movieFactor = gsl_matrix_get(movieBias, movie, 0) +
                         gsl_matrix_get(movieBias, movie, 1 + movieBin);

    //Calculate movie frequency factor
    double freqFactor = gsl_matrix_get(movieBias, movie, 1 + NUM_MOVIE_BINS + rateFreq);

    //Calculate user gradient
    double user_intercept = gsl_matrix_get(userBias, user, 0);
    double user_slope = gsl_matrix_get(userBias, user, 1);

    //Check if user has a spike at that moment
    double spikeFactor = 0.0;
    if(spikeDate != -1){
        spikeFactor = gsl_matrix_get(userBias, user, 3 + spikeDate);
    }
        
    double userFactor = user_intercept + user_slope * dateFactor + spikeFactor;
   
    double multFactor = gsl_matrix_get(userBias, user, 2);
    if(spikeDate != -1)
        multFactor += gsl_matrix_get(userBias, user, 3+NUM_USER_TIME_FACTORS+spikeDate);

    double rating = userFactor + movieFactor * multFactor + freqFactor;

    //Add SVD part
    rating += gsl_matrix_get(ratings, allIndex, 0);    

    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}

double timeSVDpp_Nov23::predict_train(int user, int movie, 
                                      double dateFactor, int movieBin, int spikeDate, int rateFreq, 
                                      double svd_prev, double nSum){
    //Calculate baseline addition
    //Calculate movie factor from bin
    double movieFactor = gsl_matrix_get(movieBias, movie, 0) +
                         gsl_matrix_get(movieBias, movie, 1 + movieBin);

    //Calculate movie frequency factor
    double freqFactor = gsl_matrix_get(movieBias, movie, 1 + NUM_MOVIE_BINS + rateFreq);

    //Calculate user gradient
    double user_intercept = gsl_matrix_get(userBias, user, 0);
    double user_slope = gsl_matrix_get(userBias, user, 1);

    //Check if user has a spike at that moment
    double spikeFactor = 0.0;
    if(spikeDate != -1){
        spikeFactor = gsl_matrix_get(userBias, user, 3 + spikeDate);
    }
        
    double userFactor = user_intercept + user_slope * dateFactor + spikeFactor;
   
    double multFactor = gsl_matrix_get(userBias, user, 2);
    if(spikeDate != -1)
        multFactor += gsl_matrix_get(userBias, user, 3+NUM_USER_TIME_FACTORS+spikeDate);

    double rating = userFactor + movieFactor * multFactor + freqFactor;    


    //Calculate SVD++ addition
    rating += svd_prev - INIT_SVD_VAL * INIT_SVD_VAL;
    double svd_user_intercept = gsl_matrix_get(userSVD, user, 0);
    double svd_user_slope = gsl_matrix_get(userSVD, user, 1);
    double svd_spikeFactor = 0.0;
    if(spikeDate != -1){
        svd_spikeFactor = gsl_matrix_get(userSVD, user, 2 + spikeDate);
    }
    double svd_userFactor = svd_user_intercept + svd_user_slope * dateFactor + svd_spikeFactor + nSum;

    double svd_movieFactor = gsl_matrix_get(movieSVD, movie, 1) + 
                             gsl_matrix_get(movieSVD, movie, 2 + movieBin) +
                             gsl_matrix_get(movieSVD, movie, 2 + NUM_MOVIE_BINS + rateFreq);               
    rating += svd_userFactor * svd_movieFactor;

    if(rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}

void timeSVDpp_Nov23::generate_frequency_table(int partition){
    printf("\tGenerating frequency table...\n");
    freqDates.reserve(USER_COUNT);
    freqNum.reserve(USER_COUNT);
    for(int user = 0; user < USER_COUNT; user++){
        freqDates.push_back(vector <int> () );
        freqNum.push_back(vector <int> () );
        freqDates[user].reserve(100);
        freqNum[user].reserve(100);
    }
    int user;
    int date;
    for(int point = 0; point < DATA_COUNT; point++){
        //if(point % 10000000 == 0)
        //    printf("\t\t\t%i percent\n", (int)((double)point*100.0/(double)DATA_COUNT));
        if(get_um_idx_ratingset(point) < partition){
            user = get_um_all_usernumber(point);
            date = get_um_all_datenumber(point);
            if(user < 1)
                printf("user... %i at point %i\n", user, point);
            if(date < 1)
                printf("date... %i at point %i\n", date, point);
            
            assert(user >= 1 && date >= 1);
            int dateIndex = find_element_vect(freqDates[user-1], date);
            if(dateIndex == -1){
                freqDates[user-1].push_back(date);
                freqNum[user-1].push_back(1);
                //if(user == 3)
                //    printf("date: %i -> record created", date);
            }else{
                freqNum[user-1][dateIndex] = 
                    freqNum[user-1][dateIndex] + 1;
                //if(user == 3)
                //    printf("date: %i -> freq", freqNum[user-1][dateIndex]);
            }
        }
    }
}

void timeSVDpp_Nov23::generate_freq_spikes(){
    printf("\tPicking highest frequency dates...\n");
    gsl_matrix *user_days;
    for(int user = 0; user < USER_COUNT; user++){
        //if(user % 100000 == 0)
        //    printf("\t\t\t%i percent\n", (int)((double)user*100.0/(double)USER_COUNT));
        user_days = gsl_matrix_calloc(NUM_USER_TIME_FACTORS, 1);
        int minIndex = 0;
        int fillCount = 0;
        int ratingFreq = 0;
        for(int dateIndex = 0; dateIndex < freqDates[user].size(); dateIndex++){
            if(fillCount < NUM_USER_TIME_FACTORS){
                gsl_matrix_set(user_days, fillCount, 0, freqDates[user][dateIndex]);
                fillCount++;
                minIndex = findMinIndex(user_days, fillCount);
            }else{
                ratingFreq = freqNum[user][dateIndex];
                if(gsl_matrix_get(user_days, minIndex, 0) < ratingFreq){
                    gsl_matrix_set(user_days, minIndex, 0, freqDates[user][dateIndex]);
                    minIndex = findMinIndex(user_days, NUM_USER_TIME_FACTORS);
                }
            }
        }
        for(int freqdate = 0; freqdate < NUM_USER_TIME_FACTORS; freqdate++){
            gsl_matrix_set(userFreqSpikes, user, freqdate,
                gsl_matrix_get(user_days, freqdate, 0));
        }
        gsl_matrix_free(user_days);
    }
}

void timeSVDpp_Nov23::generate_avg_dates(int partition){
    printf("\tCalculating average rating dates...\n");
    gsl_matrix *userBias_t;
    userBias_t = gsl_matrix_calloc(USER_COUNT, 2);

    //Find average rating date and error
    int user;
    int date;
    //userBias_t: [user][date_sum, rating_count]
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_um_idx_ratingset(point) < partition){
            user = get_um_all_usernumber(point);
            date = (double)get_um_all_datenumber(point);

            gsl_matrix_set(userBias_t, user-1, 0,
                gsl_matrix_get(userBias_t, user-1, 0) +
                date);
            
            gsl_matrix_set(userBias_t, user-1, 1,
                gsl_matrix_get(userBias_t, user-1, 1) + 1);

        }
    }
    int avgdate;
    int numR;
    for(user = 0; user < USER_COUNT; user++){
        numR = (double)gsl_matrix_get(userBias_t, user, 1);
        if(numR == 0){
            gsl_matrix_set(avgRatingDates, user, 0, 0.5);
        }else{
            avgdate = ((double)gsl_matrix_get(userBias_t, user, 0))/
                      ((double)numR);
            gsl_matrix_set(avgRatingDates, user, 0, avgdate + 0.5);
        }
    }
}

int timeSVDpp_Nov23::find_element_vect(vector <int> vect, int element){
    int index = 0;
    bool found = false;
    while(!found && index < vect.size()){
        if(vect[index] == element)
            found = true;
        else
            index++;
    }
    if(found)
        return index;
    else
        return -1;
}

int timeSVDpp_Nov23::findMinIndex(gsl_matrix *mat, int numPts){
    int minIndex = 0;
    int minValue = 99999;
    int value = 0;
    for(int i = 0; i < numPts; i++){
        value = gsl_matrix_get(mat, i, 0);
        if(value < minValue){
            minIndex = i;
            minValue = value;
        }
    }
    return minIndex;
}
            
        
