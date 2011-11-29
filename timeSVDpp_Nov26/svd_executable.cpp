#include <string.h>
#include "timesvdpp_nov26.h"
#include <math.h>
#include <assert.h>
//#include <time.h>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>
#include "../write_data/write_results.h"
int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("Proper use is svd_executable [--load] [--refine] [--learn] [--output].\n");
        exit(1);
    }
    clock_t init, final;
    printf("Creating predictor and loading data...\n");
    timeSVDpp_Nov26 predictor;
    //init=clock();
    if(strcmp(argv[1], "--load") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(4);
    }else if(strcmp(argv[1], "--refine") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(4);
        printf("Refining SVD parameters...\n");
        predictor.learn(4);
    }else if(strcmp(argv[1], "--learn") == 0){
        printf("Learning dataset...\n");
        predictor.learn(4);
    }else if(strcmp(argv[1], "--output") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(4);
        printf("Saving test predictions...\n");
        vector<double> results = vector <double> ();
        results.reserve(timeSVDpp_Nov26::SUBMIT_NUM_POINTS);
        for(int i=0; i < timeSVDpp_Nov26::DATA_COUNT; i++) {
            if(get_um_idx_ratingset(i) == 5){
                int user = get_um_all_usernumber(i)-1;
                int movie = (int)get_um_all_movienumber(i)-1;
                int date = (int)get_um_all_datenumber(i);           
                
                results.push_back(predictor.predict_point(user, movie, date, i));
            }
        }
        output_results(results);
    }      
}

