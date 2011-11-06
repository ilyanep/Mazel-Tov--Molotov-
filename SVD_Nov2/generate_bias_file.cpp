#include <string.h>
#include "svd_nov2.h"
#include <math.h>
#include <assert.h>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>

int main(int argc, char* argv[]) {
    printf("Loading data...\n");
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);
    
    printf("Calculating user/movie biases...\n");
    gsl_matrix *userBias = gsl_matrix_calloc(SVD_Nov2::USER_COUNT, 2);
    gsl_matrix *movieBias = gsl_matrix_calloc(SVD_Nov2::MOVIE_COUNT, 2);
    int user;
    int movie;
    for(int point = 0; point < SVD_Nov2::DATA_COUNT; point++){
        if(get_mu_idx_ratingset(point) < 5){
            user = get_mu_all_usernumber(point);
            movie = get_mu_all_movienumber(point);
            gsl_matrix_set(userBias, user-1, 0,
                gsl_matrix_get(userBias, user-1, 0) +
                get_mu_all_rating(point));

            gsl_matrix_set(movieBias, movie-1, 0,
                gsl_matrix_get(movieBias, movie-1, 0) +
                get_mu_all_rating(point));

            gsl_matrix_set(userBias, user-1, 1,
                gsl_matrix_get(userBias, user-1, 1) + 1);

            gsl_matrix_set(movieBias, movie-1, 1,
                gsl_matrix_get(movieBias, movie-1, 1) + 1);
        }
    }
    
    printf("Saving user/movie biases...\n");
    FILE *outFile;
    outFile = fopen(SVD_NOV2_BIAS_FILE, "w");
    double bias;
    for(int user = 0; user < SVD_Nov2::USER_COUNT; user++){
        bias = (((double)SVD_Nov2::AVG_RATING * SVD_Nov2::REGUL_BIAS_PARAM) + gsl_matrix_get(userBias, user, 0)) / 
               (((double)SVD_Nov2::REGUL_BIAS_PARAM) + gsl_matrix_get(userBias, user, 1)) - SVD_Nov2::AVG_RATING;
        fprintf(outFile,"%lf\n",bias);
    }
    for(int movie = 0; movie < SVD_Nov2::MOVIE_COUNT; movie++){
        bias = (((double)SVD_Nov2::AVG_RATING * SVD_Nov2::REGUL_BIAS_PARAM) + gsl_matrix_get(movieBias, movie, 0)) /
               (((double)SVD_Nov2::REGUL_BIAS_PARAM) + gsl_matrix_get(movieBias, movie, 1)) - SVD_Nov2::AVG_RATING;
        fprintf(outFile,"%lf\n",bias);
    }
    printf("Done!");
}
    
