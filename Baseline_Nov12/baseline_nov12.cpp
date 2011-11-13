#include <string.h>
#include "baseline_nov12.h"
#include <math.h>
#include <assert.h>
#include <algorithm>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>
#include <vector>

int find_element_vect(vector<int> vect, int element);
Baseline_Nov12::Baseline_Nov12(){
    //userBias: [user-1][intercept slope avgRatingDate multIntercept freqDate0 freqDate1 ... spikeAvg0 spikeAvg1 ... spikeMult0 spikeMult1 ...]
    //movieBias [movie-1][globalBias movieBin0_avg movieBin1_avg ... freqFact0 ... freqFact3]

    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 4 + NUM_USER_TIME_FACTORS * 3);
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, 1 + NUM_MOVIE_BINS + FREQ_LOG_MAX);
    //Set multiplicative factor to 1.0
    for (int i = 0; i < USER_COUNT; i++)
        gsl_matrix_set(userBias, i, 3, 1.0);

    freqNum = vector <vector <int> >(); //[user][freq]
    freqDates = vector <vector <int> >(); //[user][date]

    data_loaded = false;
    load_data();
}

Baseline_Nov12::Baseline_Nov12(bool loadedData){
    //userBias: [user-1][intercept slope avgRatingDate multIntercept freqDate0 freqDate1 ... spikeAvg0 spikeAvg1 ... spikeMult0 spikeMult1 ...]
    //movieBias [movie-1][globalBias movieBin0_avg movieBin1_avg ... freqFact0 ... freqFact3]

    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 4 + NUM_USER_TIME_FACTORS * 3);
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, 1 + NUM_MOVIE_BINS + 4);
    //Set multiplicative factor to 1.0
    for (int i = 0; i < USER_COUNT; i++)
        gsl_matrix_set(userBias, i, 3, 1.0);

    freqNum = vector <vector <int> >(); //[user][freq]
    freqDates = vector <vector <int> >(); //[user][date]

    data_loaded = loadedData;
    if(!data_loaded)
        load_data();
}


void Baseline_Nov12::learn(int partition){
    printf("Initializing temporal dynamics...\n");
    generate_frequency_table(partition);
    generate_freq_spikes();
    generate_avg_dates(partition);
    printf("RMSE before learning: %lf\n", rmse_probe());
    printf("Learning parameters by gradient descent\n");
    learn_by_gradient_descent(partition);

    
}

void Baseline_Nov12::learn_by_gradient_descent(int partition){
    
    int k = 0;
    int point_count;
    double err;
    double errsq;
    double rmse = 10.0;
    double oldrmse = 100.0;
    int userFreq = -1;
    double rateFreq = 0.0;
    while(fabs(oldrmse - rmse) > MIN_RMSE_IMPROVEMENT || k < LEARN_EPOCHS){
    //while(oldrmse - rmse > MIN_RMSE_IMPROVEMENT){
        oldrmse = rmse;
        k++;
        errsq = 0.0;
        point_count = 0;
        for(int i = 0; i < DATA_COUNT; i++){
            if(get_mu_idx_ratingset(i) <= partition){
                int user = get_mu_all_usernumber(i);
                int movie = get_mu_all_movienumber(i);
                int date = get_mu_all_datenumber(i);
                int movieBin = (date - 1) / MOVIE_BIN_SIZE;
    
                err = (double)get_mu_all_rating(i) - predictPt(user, movie, date, &userFreq, &rateFreq);
                
                double bu = gsl_matrix_get(userBias, user-1, 0);
                double but;
                double au = gsl_matrix_get(userBias, user-1, 1);
                double user_avgdate = gsl_matrix_get(userBias, user-1, 2);
                double bi = gsl_matrix_get(movieBias, movie-1, 0);
                double bit = gsl_matrix_get(movieBias, movie-1, 1 + movieBin);
                double bif = gsl_matrix_get(movieBias, movie-1, 1 + NUM_MOVIE_BINS + (int) (10.0 * rateFreq));
                double cu = gsl_matrix_get(userBias, user-1, 3);
                double cut;
                if(userFreq != -1){
                    but = gsl_matrix_get(userBias, user-1, 4+NUM_USER_TIME_FACTORS+userFreq);
                    cut = gsl_matrix_get(userBias, user-1, 4+NUM_USER_TIME_FACTORS*2+userFreq);
                }else{
                    but = 0.0;
                    cut = 0.0;
                }
                
                double change_bu = LEARN_RATE_BU * (err - REGUL_BU * bu);
                gsl_matrix_set(userBias, user-1, 0, bu + change_bu);

                double dateFactor = pow(fabs((double)date - user_avgdate), USER_DATE_EXP - 1.0);
                double change_au = LEARN_RATE_AU * (err * (USER_DATE_EXP * dateFactor) - REGUL_AU * au);
                gsl_matrix_set(userBias, user-1, 1, au + change_au);
    
                double change_bi = LEARN_RATE_BI * (err * (cu + cut) - REGUL_BI * bi);
                gsl_matrix_set(movieBias, movie-1, 0, bi + change_bi);

                double change_bit = LEARN_RATE_BIT * (err * (cu + cut) - REGUL_BIT * bit);
                gsl_matrix_set(movieBias, movie-1, 1 + movieBin, bit + change_bit);
                
                double change_bif = LEARN_RATE_BIF * (err - REGUL_BIF * bif);
                gsl_matrix_set(movieBias, movie-1, 1 + NUM_MOVIE_BINS + (int) (10.0 * rateFreq), bif + change_bif);

                double change_cu = LEARN_RATE_CU * (err * (bi + bit) - REGUL_CU * (cu - 1.0));
                gsl_matrix_set(userBias, user-1, 3, cu + change_cu);

                if(userFreq != -1){
                    double change_but = LEARN_RATE_BUT * (err - REGUL_BU * but);
                    gsl_matrix_set(userBias, user-1, 4+NUM_USER_TIME_FACTORS+userFreq, but + change_but);

                    double change_cut = LEARN_RATE_CUT * (err * (bi + bit) - REGUL_CUT * cut);
                    gsl_matrix_set(userBias, user-1, 4+NUM_USER_TIME_FACTORS*2+userFreq, cut + change_cut);
                }
                
                errsq += err * err;
                point_count++;
            }
        }
        rmse = errsq / ((double)point_count);
        printf("\t\tEpoch %u: RMSE(in): %lf; RMSE(out): %lf\n", k, sqrt(rmse), rmse_probe());
    }

}

double Baseline_Nov12::predict(int user, int movie, int date){
    int placeholder = -1;
    int placeholder2 = -1;
    return predictPt(user, movie, date, &placeholder, &placeholder2);
}

double Baseline_Nov12::predictPt(int user, int movie, int date, int *userFreqRet, int *rateFreqRet){
    //userBias: [user-1][intercept slope avgRatingDate multIntercept freqDate0 freqDate1 ... spikeAvg0 spikeAvg1 ... spikeMult0 spikeMult1 ...]
    //movieBias [movie-1][globalBias movieBin0_avg movieBin1_avg ... freqFact0 ... freqFact3]

    //Calculate movie factor from bin
    int movieBin = (date - 1) / MOVIE_BIN_SIZE;
    double movieFactor = gsl_matrix_get(movieBias, movie-1, 0) +
                         gsl_matrix_get(movieBias, movie-1, 1 + movieBin);

    //Calculate movie frequency factor
    int dateIndex = find_element_vect(freqDates[user-1], date);
    double logRateFreq;
    if(dateIndex == -1)
        rateFreq = 0.0;
    else
        rateFreq = log((double)freqNum[user-1][dateIndex]) / LN_LOG_BASE;
    if (rateFreq > 5.0)
        rateFreq = 5.0;
    *rateFreqRet = rateFreq;    
    //0.0->5.0 :: 0 -> 50
    double freqFactor = gsl_matrix_get(movieBias, movie-1, 1 + NUM_MOVIE_BINS + (int) (10.0 * rateFreq));

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
    int freqdate = 0;
    int datePoint = 0;
    double spikeFactor = 0.0;
    bool found = false;
    while(!found && datePoint < NUM_USER_TIME_FACTORS){
        freqdate = gsl_matrix_get(userBias, user-1, 4+datePoint);
        if(freqdate == date)
            found = true;
        else
            datePoint++;
    }
    if(found){
        spikeFactor = gsl_matrix_get(userBias, user-1, 4+NUM_USER_TIME_FACTORS+datePoint);
        *userFreqRet = datePoint;
    }else{
        *userFreqRet = -1;
    }
        
    userFactor = userFactor + spikeFactor;
    
    double multFactor = gsl_matrix_get(userBias, user-1, 3);
    if(found)
        multFactor += gsl_matrix_get(userBias, user-1, 4+NUM_USER_TIME_FACTORS*2+datePoint);

    double rating = AVG_RATING + userFactor + movieFactor * multFactor + freqFactor;
    return rating;
}

void Baseline_Nov12::save_baseline(int partition){
    FILE *outFile;
    outFile = fopen(NOV12_BASELINE_FILE, "w");
    double intercept, slope, avgdate, multfact, freqdate, spike, multspike;
    fprintf(outFile,"%u\n",partition); 
    for(int user = 0; user < USER_COUNT; user++){
        intercept = gsl_matrix_get(userBias, user, 0);
        slope = gsl_matrix_get(userBias, user, 1);
        avgdate = gsl_matrix_get(userBias, user, 2);
        multfact = gsl_matrix_get(userBias, user, 3);
        fprintf(outFile,"%lf\t",intercept);
        fprintf(outFile,"%lf\t",slope);
        fprintf(outFile,"%lf\t",avgdate);
        fprintf(outFile,"%lf\t",multfact);
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            freqdate = gsl_matrix_get(userBias, user, 4+spikedate);
            fprintf(outFile,"%lf\t",freqdate);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            spike = gsl_matrix_get(userBias, user, 
                        4+NUM_USER_TIME_FACTORS +spikedate);
            fprintf(outFile,"%lf\t",spike);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            multspike = gsl_matrix_get(userBias, user, 
                        4+NUM_USER_TIME_FACTORS*2 +spikedate);
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


void Baseline_Nov12::remember(int partition){
    FILE *inFile;
    inFile = fopen(NOV12_BASELINE_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    double intercept, slope, avgdate, multfact, freqdate, spike, multspike;
    for(int user = 0; user < USER_COUNT; user++){
        fscanf(inFile,"%lf",&intercept);
        fscanf(inFile,"%lf",&slope);
        fscanf(inFile,"%lf",&avgdate);
        fscanf(inFile,"%lf",&multfact);
        gsl_matrix_set(userBias, user, 0, intercept);
        gsl_matrix_set(userBias, user, 1, slope);
        gsl_matrix_set(userBias, user, 2, avgdate);
        gsl_matrix_set(userBias, user, 3, multfact);
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&freqdate);
            gsl_matrix_set(userBias, user, 4+spikedate,freqdate);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&spike);
            gsl_matrix_set(userBias, user, 
                        4+NUM_USER_TIME_FACTORS +spikedate, spike);
        }
        for(int spikedate = 0; spikedate < NUM_USER_TIME_FACTORS; spikedate++){
            fscanf(inFile,"%lf",&multspike);
            gsl_matrix_set(userBias, user, 
                        4+NUM_USER_TIME_FACTORS*2 +spikedate, multspike);
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

double Baseline_Nov12::rmse_probe(){
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

void Baseline_Nov12::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);
    assert(load_mu_all_datenumber() == 0);
    
    data_loaded = true;
}

void Baseline_Nov12::generate_frequency_table(int partition){
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
}

void Baseline_Nov12::generate_freq_spikes(){
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
            gsl_matrix_set(userBias, user, 4 + freqdate,
                gsl_matrix_get(user_days, freqdate, 0));
        }
        gsl_matrix_free(user_days);
    }
}

void Baseline_Nov12::generate_avg_dates(int partition){
    printf("\tCalculating average rating dates...\n");
    gsl_matrix *userBias_t;
    userBias_t = gsl_matrix_calloc(USER_COUNT, 2);

    //Find average rating date and error
    int user;
    int date;
    //userBias_t: [user][date_sum, rating_count]
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            date = (double)get_mu_all_datenumber(point);

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
            gsl_matrix_set(userBias, user, 2, 0.5);
        }else{
            avgdate = ((double)gsl_matrix_get(userBias_t, user, 0))/
                      ((double)numR);
            gsl_matrix_set(userBias, user, 2, avgdate + 0.5);
        }
    }
}

int Baseline_Nov12::find_element_vect(vector <int> vect, int element){
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

int Baseline_Nov12::findMinIndex(gsl_matrix *mat, int numPts){
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
    
