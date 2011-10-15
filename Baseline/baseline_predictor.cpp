#include <string.h>
#include "baseline_predictor.h"
#include <math.h>
#include <assert.h>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>

Baseline::Baseline(){
    //Initially all matrix elements are set to 0.0
    userBias = gsl_matrix_calloc(USER_COUNT, 1); //Extra dimension for
    movieBias = gsl_matrix_calloc(MOVIE_COUNT, 1); //computational stuff
    data_loaded = false;
    load_data();
}

void Baseline::learn(int partition){
    assert(data_loaded);
    calculate_average_biases(partition);

}

void Baseline::calculate_average_biases(int partition){
    int user;
    int movie;
    float bias;
    gsl_matrix *userBias_t = gsl_matrix_calloc(USER_COUNT, 2);
    gsl_matrix *movieBias_t = gsl_matrix_calloc(MOVIE_COUNT, 2);
    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            movie = get_mu_all_movienumber(point);

            gsl_matrix_set(movieBias_t, movie-1, 0,
                gsl_matrix_get(movieBias_t, movie-1, 0) +
                get_mu_all_rating(point) - AVG_RATING);

            gsl_matrix_set(movieBias_t, movie-1, 1,
                gsl_matrix_get(movieBias_t, movie-1, 1) + 1);
        }
    }

    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        bias = ((float)gsl_matrix_get(movieBias_t, movie, 0)) /
               ((float)REGUL_BIAS_MOVIE + gsl_matrix_get(movieBias_t, movie, 1));
        gsl_matrix_set(movieBias, movie, 0, bias);
    }

    for(int point = 0; point < DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < partition){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);

            gsl_matrix_set(userBias_t, user-1, 0,
                gsl_matrix_get(userBias_t, user-1, 0) +
                get_mu_all_rating(point) - AVG_RATING -
                gsl_matrix_get(movieBias, movie-1, 0));

            gsl_matrix_set(userBias_t, user-1, 1,
                gsl_matrix_get(userBias_t, user-1, 1) + 1);
        }
    }
    
    for(int user = 0; user < USER_COUNT; user++){
        bias = ((float)gsl_matrix_get(userBias_t, user, 0)) / 
               (((float)REGUL_BIAS_USER) + gsl_matrix_get(userBias_t, user, 1));
        gsl_matrix_set(userBias, user, 0, bias);
    }
}

double Baseline::predict(int user, int movie, int time){
    double rating = AVG_RATING + gsl_matrix_get(userBias, user-1,0) +
                                 gsl_matrix_get(movieBias, movie-1,0);
    return rating;
}

void Baseline::remember(int partition){
    FILE *outFile;
    outFile = fopen(BASELINE_FILE, "w");
    float bias;
    for(int user = 0; user < USER_COUNT; user++){
        bias = gsl_matrix_get(userBias, user, 0);
        fprintf(outFile,"%f\n",bias);
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        bias = gsl_matrix_get(movieBias, movie, 0);	
        fprintf(outFile,"%f\n",bias);
    }
    fclose(outFile);
    return;

}

float Baseline::rmse_probe(){
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

void Baseline::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

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
    
