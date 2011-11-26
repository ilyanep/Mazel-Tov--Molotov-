#include <string.h>
#include "timesvdpp_nov23.h"
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
    timeSVDpp_Nov23 predictor;
    //init=clock();
    if(strcmp(argv[1], "--load") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(3);
    }else if(strcmp(argv[1], "--refine") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(3);
        printf("Refining SVD parameters...\n");
        predictor.learn(3);
    }else if(strcmp(argv[1], "--learn") == 0){
        printf("Learning dataset...\n");
        predictor.learn(3);
    }else if(strcmp(argv[1], "--output") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(3);
        printf("Saving test predictions...\n");
        vector<double> results;
        //load qual data files (user,movie numbers) in mu order
        assert(load_um_qual_usernumber() == 0);
        assert(load_um_qual_movienumber() == 0);
        for(int i=0; i < timeSVDpp_Nov23::SUBMIT_NUM_POINTS; i++) {
            results.push_back( predictor.predict(get_um_qual_usernumber(i),
                                           (int)get_um_qual_movienumber(i),
                                           (int)get_um_qual_datenumber(i)) );
        }
        output_results(results);
    }      
}

