#include <string.h>
#include "baseline_oct25.h"
#include <math.h>
#include <assert.h>
#include <algorithm>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>
#include <vector>

int find_element_vect(vector<int> vect, int element);
Baseline_Oct25::Baseline_Oct25(){
    data_loaded = false;
    load_data();
}

Baseline_Oct25::Baseline_Oct25(bool loadedData){
    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 3 + NUM_USER_TIME_FACTORS * 2);
    //userBias = gsl_matrix_calloc(USER_COUNT, 2); 
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, NUM_MOVIE_BINS); 
    data_loaded = loadedData;
    if(!data_loaded)
        load_data();
}

void Baseline_Oct25::free_mem(){
    gsl_matrix_free(userBias);
    gsl_matrix_free(movieBias);
}

void Baseline_Oct25::learn(int partition){
    assert(data_loaded);
    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 3 + NUM_USER_TIME_FACTORS * 2);
    //userBias = gsl_matrix_calloc(USER_COUNT, 2); 
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, NUM_MOVIE_BINS); 

    printf("Finding global biases...\n");
    calculate_average_biases(partition);
    printf("--> RMSE = %lf\n", rmse_probe());
    //printf("Refining by gradient descent...\n");
    //refine_by_gradient_descent(partition);
    //printf("--> RMSE = %lf\n", rmse_probe());
    printf("Finding user time gradient...\n");
    calculate_user_time_gradient(partition);
    printf("--> RMSE = %lf\n", rmse_probe());
    printf("Finding movie time effects...\n");
    calculate_movie_time_effects(partition);
    printf("--> RMSE = %lf\n", rmse_probe());
    //printf("Refining by gradient descent...\n");
    //refine_by_gradient_descent(partition);
    //printf("--> RMSE = %lf\n", rmse_probe());
    printf("Finding user time spikes...\n");
    calculate_user_time_spikes(partition);
    printf("--> RMSE = %lf\n", rmse_probe());
}

void Baseline_Oct25::calculate_user_time_spikes(int partition){
    printf("\tFinding locations of time spikes...\n");
    find_user_rating_days(partition);

    printf("\tCalculating biases for time spikes...\n");
    gsl_matrix *user_days = gsl_matrix_alloc(USER_COUNT, NUM_USER_TIME_FACTORS * 2);
    int user;
    int date;
    int movie;
    double ratingErr;
    int datePoint;
    int freqdate;
    for(int point = 0; point < DATA_COUNT; point++){
        //if(point % 10000000 == 0)
        //    printf("\t\t\t%i percent\n", (int)((double)point*100.0/(double)DATA_COUNT));
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            date = get_mu_all_datenumber(point);
            
            //Check if this is one of the users frequent dates
            bool foundDate = false;
            datePoint = 0;
            while(!foundDate && datePoint < NUM_USER_TIME_FACTORS){
                freqdate = gsl_matrix_get(userBias, user-1, 3+datePoint);
                if(freqdate == date)
                    foundDate = true;
                else
                    datePoint++;
            }
            //If date is found, modify appropriate counters
            if(foundDate){
                movie = get_mu_all_movienumber(point);
                ratingErr = (double)get_mu_all_rating(point) - 
                            predict(user, movie, date,0);
                gsl_matrix_set(user_days, user-1, datePoint,
                    gsl_matrix_get(user_days, user-1, datePoint)+
                    ratingErr);
                gsl_matrix_set(user_days, user-1, NUM_USER_TIME_FACTORS + datePoint,
                    gsl_matrix_get(user_days, user-1, NUM_USER_TIME_FACTORS + datePoint) + 1);
            }
        }
    }
    printf("\tSaving time spikes...\n");
    double spikeavg;
    double rateCount;
    for(int user = 0; user < USER_COUNT; user++){
        for(int dateIndex = 0; dateIndex < NUM_USER_TIME_FACTORS; dateIndex++){
            rateCount = gsl_matrix_get(user_days, user, NUM_USER_TIME_FACTORS + dateIndex);
            if(rateCount != 0)
                spikeavg = gsl_matrix_get(user_days, user, dateIndex)/
                      ((double)USER_FREQ_REGUL + rateCount);
            else
                spikeavg = 0.0;
            gsl_matrix_set(userBias, user, 3+NUM_USER_TIME_FACTORS + dateIndex, spikeavg);
        }
    }
}
/* Compute the days with the highest frequency of rating for each user
 *
 * First fill rating_frequency with number of ratings on each day for each user
 * Then pick up days with the most ratings and store them in user_days
 */
void Baseline_Oct25::find_user_rating_days(int partition){
    vector< vector<int> > freqDates = vector <vector <int> >(); //[user][date]
    freqDates.reserve(USER_COUNT);
    vector< vector<int> > freqNum = vector <vector <int> >(); //[user][freq]
    freqNum.reserve(USER_COUNT);
    for(int user = 0; user < USER_COUNT; user++){
        freqDates.push_back(vector <int> () );
        freqNum.push_back(vector <int> () );
        freqDates[user].reserve(100);
        freqNum[user].reserve(100);
    }
    int user;
    int date;
    printf("\t\tGenerating frequency table...\n");
    for(int point = 0; point < DATA_COUNT; point++){
        //if(point % 10000000 == 0)
        //    printf("\t\t\t%i percent\n", (int)((double)point*100.0/(double)DATA_COUNT));
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            date = get_mu_all_datenumber(point);
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
    printf("\t\tPicking highest frequency dates...\n");
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
            gsl_matrix_set(userBias, user, 3 + freqdate,
                gsl_matrix_get(user_days, freqdate, 0));
        }
        gsl_matrix_free(user_days);
    }
}

int find_element_vect(vector <int> vect, int element){
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

int Baseline_Oct25::findMinIndex(gsl_matrix *mat, int numPts){
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

/* Model is rating = AVG_RATING + user_bias + movie_bias
 * user_bias = user_avg + b * (t-tavg)^c + spikeFactor 
 * movie_bias = movie_bin_avg
 *
 * userBias: [user][intercept slope avgrating spikedate0 spikedate1... spikefact0 spikefact1 ...]
 * movieBias: [movie][binavg1 binavg2 binavg3 ...]
 *
 */

/*
void Baseline_Oct25::refine_by_gradient_descent(int partition){

    double rmse = 0;
    double oldrmse_step = 999;
    vector<double> deriv = vector<double>();
    deriv.reserve(3 + NUM_MOVIE_BINS);
    for(int param = 0; param < (3 + NUM_MOVIE_BINS); param++){
        deriv.push_back(0.0);
    }
    printf("\tRefining user parameters...\n");
    for(int user = 0; user < USER_COUNT; user++){
        int startIndex = 0;
        int endIndex = 0;
        bool found = false;
        for(int point = 0; point < DATA_COUNT; point++){
            if(get_mu_all_usernumber(point) - 1 == user && !found){
                startIndex = point;
                found = true;
            }
            if(get_mu_all_usernumber(point) - 1 != user && found){
                endIndex = point;
                found = false;
                break;
            }
        }

        //if(user % 100000 == 0){
            printf("\t\t%i percent", (int)((double)user/(double)USER_COUNT * 100.0));
            printf("\t\tTrue RMSE: %lf\n", rmse_probe());
        //}
        rmse = 0;
        oldrmse_step = 999;
        while((oldrmse_step - rmse) > GRAD_MIN_IMPROVEMENT){
        
            oldrmse_step = rmse_probe_user(3,user+1,startIndex,endIndex);
            for(int param = 0; param < 3; param++){
                //Undo previous deriv
                if(param > 0){
                    gsl_matrix_set(userBias, user, param-1,
                        gsl_matrix_get(userBias, user, param-1) - DERIV_STEP);
                }
                //Calc current deriv
                gsl_matrix_set(userBias, user, param,
                        gsl_matrix_get(userBias, user, param) + DERIV_STEP);
                deriv[param] = (rmse_probe_user(3, user+1, startIndex, endIndex) - oldrmse_step) /
                               (double)DERIV_STEP;
                //printf("deriv: %lf\n",deriv[param]);
            }
            //Undo last deriv
            gsl_matrix_set(userBias, user, 2, 
                gsl_matrix_get(userBias, user, 2) - DERIV_STEP);

            //Update parameters
            for(int param = 0; param < 3; param++){
                gsl_matrix_set(userBias, user, param,
                        gsl_matrix_get(userBias, user, param) + deriv[param] * (double) GRAD_STEP);
            }
            rmse = rmse_probe_user(3,user+1,startIndex,endIndex);
        }
    }
    printf("\t\tTrue RMSE: %lf\n", rmse_probe());
}
*/

/*
double Baseline_Oct25::rmse_probe_part(int partition){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
        if(get_mu_idx_ratingset(i) == partition){
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
*/
/*
double Baseline_Oct25::rmse_probe_user(int partition, int user, int startIndex, int endIndex){
    double RMSE = 0.0;
    int count = 0;
    int ptUser;
    for(int i = startIndex; i <= endIndex; i++) {
        ptUser = (int)get_mu_all_usernumber(i);
        if(get_mu_idx_ratingset(i) == partition && ptUser == user){
            double prediction = predict(ptUser,
                                        get_mu_all_movienumber(i),
                                        (int)get_mu_all_datenumber(i));
            double error = (prediction - (double)get_mu_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
} 
*/
/*
double Baseline_Oct25::rmse_probe_movie(int partition, int movie){
    double RMSE = 0.0;
    int count = 0;
    int ptMovie;
    for(int i = 0; i < DATA_COUNT; i++) {
        ptMovie = (int)get_mu_all_movienumber(i);
        if(get_mu_idx_ratingset(i) == partition && ptMovie == movie){
            double prediction = predict(get_mu_all_usernumber(i),
                                                  ptMovie,
                                                  (int)get_mu_all_datenumber(i));
            double error = (prediction - (double)get_mu_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
} 
*/

void Baseline_Oct25::calculate_average_biases(int partition){
    int user;
    int movie;
    double bias;
    printf("\tCalculating movie biases...\n");
    gsl_matrix *userBias_t = gsl_matrix_calloc(USER_COUNT, 2);
    gsl_matrix *movieBias_t = gsl_matrix_calloc(MOVIE_COUNT, 2);
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            movie = get_mu_all_movienumber(point);

            gsl_matrix_set(movieBias_t, movie-1, 0,
                gsl_matrix_get(movieBias_t, movie-1, 0) +
                get_mu_all_rating(point) - (double)AVG_RATING);

            gsl_matrix_set(movieBias_t, movie-1, 1,
                gsl_matrix_get(movieBias_t, movie-1, 1) + 1);
        }
    }
    printf("\tSaving movie biases...\n");
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        bias = ((double)gsl_matrix_get(movieBias_t, movie, 0)) /
               ((double)REGUL_BIAS_MOVIE + gsl_matrix_get(movieBias_t, movie, 1));
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            gsl_matrix_set(movieBias, movie, bin, bias);
        }
    }
    printf("\tCalculating user biases...\n");
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);

            gsl_matrix_set(userBias_t, user-1, 0,
                gsl_matrix_get(userBias_t, user-1, 0) +
                get_mu_all_rating(point) - (double)AVG_RATING -
                gsl_matrix_get(movieBias, movie-1, 0));

            gsl_matrix_set(userBias_t, user-1, 1,
                gsl_matrix_get(userBias_t, user-1, 1) + 1);
        }
    }
    printf("\tSaving user biases...\n");
    for(int user = 0; user < USER_COUNT; user++){
        bias = ((double)gsl_matrix_get(userBias_t, user, 0)) / 
               (((double)REGUL_BIAS_USER) + gsl_matrix_get(userBias_t, user, 1));
        gsl_matrix_set(userBias, user, 0, bias);
    }
    gsl_matrix_free(userBias_t);
    gsl_matrix_free(movieBias_t);
}

void Baseline_Oct25::calculate_movie_time_effects(int partition){
    int user;
    int movie;
    int date;
    int binNumber;
    double bias;
    gsl_matrix *movieBias_t = gsl_matrix_calloc(MOVIE_COUNT, NUM_MOVIE_BINS * 2);
    printf("\tCalculating movie bins...\n");
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            movie = get_mu_all_movienumber(point);
            date = get_mu_all_datenumber(point);
            binNumber = (date - 1) / MOVIE_BIN_SIZE;

            gsl_matrix_set(movieBias_t, movie-1, binNumber,
                gsl_matrix_get(movieBias_t, movie-1, binNumber) +
                get_mu_all_rating(point) - (double)AVG_RATING -
                gsl_matrix_get(movieBias, movie-1, binNumber));

            gsl_matrix_set(movieBias_t, movie-1, NUM_MOVIE_BINS+binNumber,
                gsl_matrix_get(movieBias_t, movie-1, NUM_MOVIE_BINS+binNumber) + 1);
        }
    }
    printf("\tSaving movie bins...\n");
    int numRatings;
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            numRatings = ((double)gsl_matrix_get(movieBias_t, movie, NUM_MOVIE_BINS + bin));
            if(numRatings != 0){
                bias = ((double)gsl_matrix_get(movieBias_t, movie, bin)) /
                       ((double)numRatings);
                gsl_matrix_set(movieBias, movie, bin, gsl_matrix_get (movieBias, movie, bin) + bias);
            }
        }
    }
    gsl_matrix_free(movieBias_t);
}

/*
 * userFactor = a + b *(t-tavg)^c
 *
 * b = SUM{ [(ratingerr) - (avgratingerr)]*[ (t-tavg)^c - avg(t-tavg)^c] }/
 *     SUM{ [ (t-tavg)^c - avg(t-tavg)^c ]^2 }
 *
 * a = avgratingerr - b * avg(t-tavg)^c
 */
void Baseline_Oct25::calculate_user_time_gradient(int partition){
    int user;
    int movie;
    int binNumber;
    double date;
    double avgdate;
    double avgtransdate;
    double avgerr;
    double dateFactor;
    double bias;
    printf("\tCalculating average rating dates and errors...\n");
    gsl_matrix *userBias_t = gsl_matrix_calloc(USER_COUNT, 4);

    //Find average rating date and error
    //userBias_t: [user][date_sum, rating_count, 0, 0]
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);
            date = (double)get_mu_all_datenumber(point);

            gsl_matrix_set(userBias_t, user-1, 0,
                gsl_matrix_get(userBias_t, user-1, 0) +
                date);
            
            /*gsl_matrix_set(userBias_t, user-1, 1,
                gsl_matrix_get(userBias_t, user-1, 1) +
                (double)get_mu_all_rating(point) - predict(user, movie, date));
            */
            gsl_matrix_set(userBias_t, user-1, 1,
                gsl_matrix_get(userBias_t, user-1, 1) + 1);

        }
    }

    //userBias_t: [user][avgdate, rating_count, 0, 0]
    for(user = 0; user < USER_COUNT; user++){
        avgdate = ((double)gsl_matrix_get(userBias_t, user, 0))/
               ((double)gsl_matrix_get(userBias_t, user, 1));
        /*avgerr = ((double)gsl_matrix_get(userBias_t, user, 1))/
               ((double)gsl_matrix_get(userBias_t, user, 2));*/
        gsl_matrix_set(userBias_t, user, 0, avgdate);
        //gsl_matrix_set(userBias_t, user, 1, avgerr);
        //if(user == 1745)
        //    printf("avgerr: %lf\n", avgerr);
    }

    //Find average date with transformation
    //userBias_t: [user][avgdate, rating_count, sum_transformed_date, 0]
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);
            date = (double)get_mu_all_datenumber(point);
            avgdate = gsl_matrix_get(userBias_t, user-1, 0);

            if((double)date >= avgdate)
                dateFactor = pow((double)date - avgdate, USER_DATE_EXP);
            else
                dateFactor = -1.0*pow(avgdate - (double)date, USER_DATE_EXP);


            gsl_matrix_set(userBias_t, user-1, 2,
                gsl_matrix_get(userBias_t, user-1, 2) +
                dateFactor);
        }
    }

    //userBias_t: [user][avgdate, avg_trans_date, 0, 0]
    for(user = 0; user < USER_COUNT; user++){
        avgtransdate = ((double)gsl_matrix_get(userBias_t, user, 2))/
               ((double)gsl_matrix_get(userBias_t, user, 1));
        gsl_matrix_set(userBias_t, user, 1, avgtransdate);
        gsl_matrix_set(userBias_t, user, 2, 0.0);
    }

    printf("\tCalculating regression for ratings vs dates...\n");

    //Calculate avg rating error for each user
    //userBias_t: [user][avgdate, avg_trans_date, covxy, varx]
    double covxy;
    double varx;
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);
            date = (double)get_mu_all_datenumber(point);
            avgdate = gsl_matrix_get(userBias_t, user-1, 0);
            //avgerr = gsl_matrix_get(userBias_t, user-1, 1);
            avgerr = 0.0;
            avgtransdate = gsl_matrix_get(userBias_t, user-1, 1);
        
            if((double)date >= avgdate)
                dateFactor = pow((double)date - avgdate, USER_DATE_EXP);
            else
                dateFactor = -1.0*pow(avgdate - (double)date, USER_DATE_EXP);

            covxy = gsl_matrix_get(userBias_t, user-1, 2) +
                (((double)get_mu_all_rating(point) - predict(user, movie, date,0) - avgerr) *
                (dateFactor - avgtransdate));
            

            varx = gsl_matrix_get(userBias_t, user-1, 3) + pow(dateFactor-avgtransdate, 2.0);
            
            gsl_matrix_set(userBias_t, user-1, 2, covxy);
            gsl_matrix_set(userBias_t, user-1, 3, varx);
        }
    }
    printf("\tSaving regression for ratings vs dates...\n");
    double slope;
    //userBias_t: [user][avgdate, avg_trans_date, covxy, varx]
    for(user = 0; user < USER_COUNT; user++){
        covxy = ((double)gsl_matrix_get(userBias_t, user, 2));
        varx = ((double)gsl_matrix_get(userBias_t, user, 3));
        if (varx != 0){
            slope = covxy/(varx + (double)USER_DATE_REGUL);
        }else{
            slope = 0.0;
        }        
        gsl_matrix_set(userBias, user, 1, slope);
    
    }

    double intercept;
    //userBias_t: [user][avgdate, avg_trans_date, covxy, varx]
    for(user = 0; user < USER_COUNT; user++){
        avgdate = gsl_matrix_get(userBias_t, user, 0);
        //avgerr = gsl_matrix_get(userBias_t, user, 1);
        avgerr = 0.0;
        avgtransdate = gsl_matrix_get(userBias_t, user, 1);

        slope = gsl_matrix_get(userBias, user, 1);

        intercept = avgerr - slope * avgtransdate;
        
        gsl_matrix_set(userBias, user, 0, gsl_matrix_get(userBias, user, 0) + intercept);
        gsl_matrix_set(userBias, user, 2, avgdate);
    }
    gsl_matrix_free(userBias_t);
}

double Baseline_Oct25::predict(int user, int movie, int date, int index){
    //Calculate movie factor from bin
    int movieBin = (date - 1) / MOVIE_BIN_SIZE;
    double movieFactor = gsl_matrix_get(movieBias, movie-1, movieBin);
    
    //Calculate user gradient
    double user_avgdate = gsl_matrix_get(userBias, user-1, 2);
    double user_intercept = gsl_matrix_get(userBias, user-1, 0);
    double user_slope = gsl_matrix_get(userBias, user-1, 1);
    double dateFactor;
    if((double)date >= user_avgdate)
        dateFactor = pow((double)date - user_avgdate, USER_DATE_EXP);
    else
        dateFactor = -1.0*pow(user_avgdate - (double)date, USER_DATE_EXP);
    
    double userFactor = user_intercept + user_slope * dateFactor;

    //Check if user has a spike at that moment
    bool found = false;
    int freqdate = 0;
    int datePoint = 0;
    double spikeFactor = 0.0;
    while(!found && datePoint < NUM_USER_TIME_FACTORS){
        freqdate = gsl_matrix_get(userBias, user-1, 3+datePoint);
        if(freqdate == date)
            found = true;
        else
            datePoint++;
    }
    if(found)
        spikeFactor = gsl_matrix_get(userBias, user-1, 3+NUM_USER_TIME_FACTORS+datePoint);
    userFactor = userFactor + spikeFactor;

    double rating = AVG_RATING + userFactor + movieFactor;
    return rating;
}

void Baseline_Oct25::save_baseline(int partition){
    FILE *outFile;
    outFile = fopen(OCT25_BASELINE_FILE, "w");
    double bias, intercept, slope, avgdate, freqdate, spike;
    fprintf(outFile,"%u\n",partition); 
    for(int user = 0; user < USER_COUNT; user++){
        intercept = gsl_matrix_get(userBias, user, 0);
        slope = gsl_matrix_get(userBias, user, 1);
        avgdate = gsl_matrix_get(userBias, user, 2);
        fprintf(outFile,"%lf\t",intercept);
        fprintf(outFile,"%lf\t",slope);
        fprintf(outFile,"%lf\t",avgdate);
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            freqdate = gsl_matrix_get(userBias, user, 3+spikedate);
            fprintf(outFile,"%lf\t",freqdate);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            spike = gsl_matrix_get(userBias, user, 
                        3+NUM_USER_TIME_FACTORS +spikedate);
            fprintf(outFile,"%lf\t",spike);
        }
        fprintf(outFile,"\n");
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            bias = gsl_matrix_get(movieBias, movie, bin);	
            fprintf(outFile,"%lf\t",bias);
        }
        fprintf(outFile,"\n");
    }
    fclose(outFile);
    return;

}


void Baseline_Oct25::remember(int partition){
    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 3 + NUM_USER_TIME_FACTORS * 2);
    //userBias = gsl_matrix_calloc(USER_COUNT, 2); 
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, NUM_MOVIE_BINS); 

    FILE *inFile;
    inFile = fopen(OCT25_BASELINE_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    double bias, intercept, slope, avgdate, freqdate, spike;
    for(int user = 0; user < USER_COUNT; user++){
        fscanf(inFile,"%lf",&intercept);
        fscanf(inFile,"%lf",&slope);
        fscanf(inFile,"%lf",&avgdate);
        gsl_matrix_set(userBias, user, 0, intercept);
        gsl_matrix_set(userBias, user, 1, slope);
        gsl_matrix_set(userBias, user, 2, avgdate);
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&freqdate);
            gsl_matrix_set(userBias, user, 3+spikedate,freqdate);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&spike);
            gsl_matrix_set(userBias, user, 
                        3+NUM_USER_TIME_FACTORS +spikedate, spike);
        }
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            fscanf(inFile,"%lf",&bias);
            gsl_matrix_set(movieBias, movie, bin, bias);	
        }
    }
    fclose(inFile);
    return;

}

double Baseline_Oct25::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
        if(get_mu_idx_ratingset(i) == 4){
            double prediction = predict(get_mu_all_usernumber(i),
                                                  (int)get_mu_all_movienumber(i),
                                                  (int)get_mu_all_datenumber(i),0);
            double error = (prediction - (double)get_mu_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}    

void Baseline_Oct25::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);
    assert(load_mu_all_datenumber() == 0);
    data_loaded = true;
}

/*
int main(int argc, char* argv[]) {
    printf("Loading data...\n");
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_datenumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);
    
    printf("Calculating user/movie biases...\n");
    gsl_matrix *userBias = gsl_matrix_calloc(USER_COUNT, 2);
    gsl_matrix *movieBias = gsl_matrix_calloc(MOVIE_COUNT, 2);
    int user;
    int movie;
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < 5){
            movie = get_mu_all_movienumber(point);

            gsl_matrix_set(movieBias, movie-1, 0,
                gsl_matrix_get(movieBias, movie-1, 0) +
                get_mu_all_rating(point) - AVG_RATING);

            gsl_matrix_set(movieBias, movie-1, 1,
                gsl_matrix_get(movieBias, movie-1, 1) + 1);
        }
    }

    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        bias = ((double)gsl_matrix_get(movieBias, movie, 0)) /
              (((double)REGUL_BIAS_MOVIE) + gsl_matrix_get(movieBias, movie, 1));
        gsl_matrix_set(
    }

    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < 5){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);
            gsl_matrix_set(userBias, user-1, 0,
                gsl_matrix_get(userBias, user-1, 0) +
                get_mu_all_rating(point) - AVG_RATING -
                gsl_matrix_get(movieBias, movie-1, 0));

            gsl_matrix_set(userBias, user-1, 1,
                gsl_matrix_get(userBias, user-1, 1) + 1);

        }
    }
    
    double bias;
    for(int user = 0; user < USER_COUNT; user++){
        bias = (((double)AVG_RATING * REGUL_BIAS_PARAM) + gsl_matrix_get(userBias, user, 0)) / 
               (((double)REGUL_BIAS_PARAM) + gsl_matrix_get(userBias, user, 1)) - AVG_RATING;
        fprintf(outFile,"%lf\n",bias);
    }
} */
    
