#include <string.h>
#include "svd_svm_nov6.h"
#include <math.h>
#include <assert.h>
//#include <time.h>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>
#include "../write_data/write_results.h"
int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("Proper use is svm_executable [--load] [--learn] [--output].\n");
        exit(1);
    }
    clock_t init, final;
    printf("Creating predictor and loading data...\n");
    SVD_SVM_Nov6 predictor;
    //init=clock();
    if(strcmp(argv[1], "--load") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(3);
    }else if(strcmp(argv[1], "--learn") == 0){
        printf("Learning dataset...\n");
        predictor.learn(3);
    }else if(strcmp(argv[1], "--output") == 0){
        printf("Loading SVD parameters...\n");
        predictor.remember(3);
        printf("Saving test predictions...\n");
        vector<double> results;
        //load qual data files (user,movie numbers) in mu order
        assert(load_mu_qual_usernumber() == 0);
        assert(load_mu_qual_movienumber() == 0);
        for(int i=0; i < SVD_SVM_Nov6::SUBMIT_NUM_POINTS; i++) {
            results.push_back(predictor.predict(get_mu_qual_usernumber(i),
                                           (int)get_mu_qual_movienumber(i),0));
        }
        output_results(results);
    }else{
        printf("Proper use is svm_executable [--load] [--learn] [--output].\n");
        exit(1);
    }
    
    printf("Calculating RMSE on probe...\n");
    double RMSE = predictor.rmse_probe();
    printf("Probe RMSE: %f\n", RMSE);
    printf("Saving SVD parameters...\n");
    predictor.save_svm();
}

