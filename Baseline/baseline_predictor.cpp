#include <string.h>
#include "baseline_predictor.h"
#include <math.h>
#include <assert.h>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>

Baseline::Baseline(){
    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 2); 
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, NUM_MOVIE_BINS); 
    data_loaded = false;
    load_data();
}

Baseline::Baseline(bool loadedData){
    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 2); 
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, NUM_MOVIE_BINS); 
    data_loaded = loadedData;
    if(!data_loaded)
        load_data();
}

void Baseline::learn(int partition){
    assert(data_loaded);
    printf("Finding global biases...\n");
    calculate_average_biases(partition);
    printf("Finding movie time effects...\n");
    calculate_movie_time_effects(partition);
    printf("Finding user time effects...\n");
    calculate_user_time_effects(partition);
}

void Baseline::calculate_average_biases(int partition){
    int user;
    int movie;
    float bias;
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
        bias = ((float)gsl_matrix_get(movieBias_t, movie, 0)) /
               ((float)REGUL_BIAS_MOVIE + gsl_matrix_get(movieBias_t, movie, 1));
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
        bias = ((float)gsl_matrix_get(userBias_t, user, 0)) / 
               (((float)REGUL_BIAS_USER) + gsl_matrix_get(userBias_t, user, 1));
        gsl_matrix_set(userBias, user, 0, bias);
    }
}

void Baseline::calculate_movie_time_effects(int partition){
    int user;
    int movie;
    int date;
    int binNumber;
    float bias;
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
            numRatings = ((float)gsl_matrix_get(movieBias_t, movie, NUM_MOVIE_BINS + bin));
            if(numRatings != 0){
                bias = ((float)gsl_matrix_get(movieBias_t, movie, bin)) /
                       ((float)numRatings);
                gsl_matrix_set(movieBias, movie, bin, gsl_matrix_get (movieBias, movie, bin) + bias);
            }
        }
    }
}

void Baseline::calculate_user_time_effects(int partition){
    int user;
    int movie;
    int binNumber;
    double date;
    double avgdate;
    double dateFactor;
    double bias;
    printf("\tCalculating average rating dates...\n");
    //for user, matrix will have (avgdate, covxy, varx)
    gsl_matrix *userBias_t = gsl_matrix_calloc(USER_COUNT, 3);
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            date = (double)get_mu_all_datenumber(point)/(double)DATE_COUNT;

            dateFactor = pow((double)date, USER_DATE_EXP);

            gsl_matrix_set(userBias_t, user-1, 0,
                gsl_matrix_get(userBias_t, user-1, 0) +
                date);

            gsl_matrix_set(userBias_t, user-1, 1,
                gsl_matrix_get(userBias_t, user-1, 1) +
                date);
        }
    }
    //Find average over dates
    for(user = 0; user < USER_COUNT; user++){
        avgdate = ((double)gsl_matrix_get(userBias_t, user, 0))/
               ((double)gsl_matrix_get(userBias_t, user, 1));
        gsl_matrix_set(userBias_t, user, 0, date);
        gsl_matrix_set(userBias_t, user, 1, 0.0);
    }
    printf("\tCalculating regression for ratings vs dates...\n");
    double covxy;
    double varx;
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);
            date = (double)get_mu_all_datenumber(point)/(double)DATE_COUNT;
            binNumber = (date - 1) / MOVIE_BIN_SIZE;
            avgdate = gsl_matrix_get(userBias_t, user-1, 0);
        
            dateFactor = pow((double)date, USER_DATE_EXP);
            
            covxy = gsl_matrix_get(userBias_t, user-1, 1) +
                (get_mu_all_rating(point) - (double)AVG_RATING -
                 gsl_matrix_get(movieBias, movie-1, binNumber) -
                 gsl_matrix_get(userBias, user-1, 0)) *
                (dateFactor - avgdate);
            

            varx = gsl_matrix_get(userBias_t, user-1, 2) + pow(dateFactor - avgdate, 2.0);
            gsl_matrix_set(userBias_t, user-1, 1, covxy);

            gsl_matrix_set(userBias_t, user-1, 2, varx);
        }
    }
    printf("\tSaving regression for ratings vs dates...\n");
    double slope;
    for(user = 0; user < USER_COUNT; user++){
        covxy = ((double)gsl_matrix_get(userBias_t, user, 1));
        varx = ((double)gsl_matrix_get(userBias_t, user, 2));
        if (varx != 0){
            slope = covxy/(varx + (double)USER_DATE_REGUL);
        }else{
            slope = 0.0;
        }        
        gsl_matrix_set(userBias_t, user, 1, slope);
        gsl_matrix_set(userBias_t, user, 2, 0.0);
    
    }

    double intercept;
    double avgrating;
    for(user = 0; user < USER_COUNT; user++){
        avgdate = gsl_matrix_get(userBias_t, user, 0);
        dateFactor = pow(avgdate, USER_DATE_EXP);
        slope = gsl_matrix_get(userBias_t, user, 1);
        if(slope != 0.0){
            avgrating = gsl_matrix_get(userBias, user, 0);
            intercept = avgrating - slope * dateFactor;
        
            gsl_matrix_set(userBias, user, 0, intercept);
        }
        gsl_matrix_set(userBias, user, 1, slope);
    }
}

double Baseline::predict(int user, int movie, int time){
    int movieBin = (time - 1) / MOVIE_BIN_SIZE;
    double movieFactor = gsl_matrix_get(movieBias, movie-1, movieBin);
    double date = (double)time/(double)DATE_COUNT;
    double userFactor = gsl_matrix_get(userBias, user-1, 0) +
                        gsl_matrix_get(userBias, user-1, 1) *
                        pow(date, USER_DATE_EXP);
    double rating = AVG_RATING + userFactor + movieFactor;
    return rating;
}

void Baseline::save_baseline(int partition){
    FILE *outFile;
    outFile = fopen(BASELINE_FILE, "w");
    float bias, intercept, slope;
    fprintf(outFile,"%u\n",partition); 
    for(int user = 0; user < USER_COUNT; user++){
        intercept = gsl_matrix_get(userBias, user, 0);
        slope = gsl_matrix_get(userBias, user, 1);
        fprintf(outFile,"%f\t",intercept);
        fprintf(outFile,"%f\n",slope);
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            bias = gsl_matrix_get(movieBias, movie, bin);	
            fprintf(outFile,"%f\t",bias);
        }
        fprintf(outFile,"\n");
    }
    fclose(outFile);
    return;

}


void Baseline::remember(int partition){
    FILE *inFile;
    inFile = fopen(BASELINE_FILE, "r");
    int load_partition;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == partition);
    float bias, intercept, slope;
    for(int user = 0; user < USER_COUNT; user++){
        fscanf(inFile,"%f",&intercept);
        fscanf(inFile,"%f",&slope);
        gsl_matrix_set(userBias, user, 0, intercept);
        gsl_matrix_set(userBias, user, 1, slope);
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int bin = 0; bin < NUM_MOVIE_BINS; bin++){
            fscanf(inFile,"%f",&bias);
            gsl_matrix_set(movieBias, movie, bin, bias);	
        }
    }
    fclose(inFile);
    return;

}

float Baseline::rmse_probe(){
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

void Baseline::load_data(){
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
        bias = ((float)gsl_matrix_get(movieBias, movie, 0)) /
              (((float)REGUL_BIAS_MOVIE) + gsl_matrix_get(movieBias, movie, 1));
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
    
    float bias;
    for(int user = 0; user < USER_COUNT; user++){
        bias = (((float)AVG_RATING * REGUL_BIAS_PARAM) + gsl_matrix_get(userBias, user, 0)) / 
               (((float)REGUL_BIAS_PARAM) + gsl_matrix_get(userBias, user, 1)) - AVG_RATING;
        fprintf(outFile,"%f\n",bias);
    }
} */
    
