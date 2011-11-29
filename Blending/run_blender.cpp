#include "blender.h"
#include <assert.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#include "../timeSVDpp_Nov23/timesvdpp_nov23.h"
#include "../SVDK_Nov21/svdk_nov21.h"
#include "../SVDK_Nov17/svdk_nov17.h"
#include "../Baseline_Nov19/baseline_nov19.h"
#include "../all_3_predictor/all_3_predictor.h"
#include "../binary_files/binary_files.h"
#include "../learning_method.h"
#include "../write_data/write_results.h"
#include "../SVD_Oct18/svd_oct18.h"
#include "../SVD_Oct25/svd_oct25.h"
#include "../SVD_Nov2/svd_nov2.h"
#include "../SVDK_Nov9/svdk_nov9.h"
#include "../SVDK_Nov13/svdk_nov13.h"
#include "../Baseline_Oct25/baseline_oct25.h"
#include "../Baseline_Nov12/baseline_nov12.h"
#include "../movie_knn/movie_knn_pearson.h"
#include "../movie_knn/movie_knn.h"

#define BLENDER_LEARNING_PARTITION 4
#ifndef SUBMIT_NUM_POINTS
#define SUBMIT_NUM_POINTS 2749898
#endif

int main() {
    // Pull in the datapoints on which we want predictions
    cout << "Loading and/or checking loaded data" << endl;
    assert(load_um_all_usernumber() == 0);
    assert(load_um_all_movienumber() == 0);
    assert(load_um_all_datenumber() == 0);
    assert(load_um_idx_ratingset() == 0);

    // Initialize all the predictors
    cout << "Initializing predictor vector" << endl;
    vector<IPredictor*> predictor_vector;
    
    Movie_Knn mknn;
    Movie_Knn_Pearson mknn_pearson;
    /*All3Predictor all_3s; 
    SVDK_Nov21 svdk_nov21;
    SVDK_Nov17 svdk_nov17;
    timeSVDpp_Nov23 timesvdpp_nov23;
    Baseline_Nov19 baseline_nov19;
    SVD_Oct18 svd_oct18;
    SVD_Oct25 svd_oct25;
	Baseline_Oct25 baseline_oct25;
    Baseline_Nov12 baseline_nov12;
    SVD_Nov2 svd_nov2;
    SVDK_Nov9 svdk_nov9;
    SVDK_Nov13 svdk_nov13;*/

    predictor_vector.push_back(&mknn);
    predictor_vector.push_back(&mknn_pearson);
    /*predictor_vector.push_back(&all_3s);
    predictor_vector.push_back(&svdk_nov21);
    predictor_vector.push_back(&svdk_nov17);
    predictor_vector.push_back(&timesvdpp_nov23);
    predictor_vector.push_back(&baseline_nov19);
    predictor_vector.push_back(&svd_oct18);
    predictor_vector.push_back(&svd_oct25);
    predictor_vector.push_back(&baseline_oct25);
    predictor_vector.push_back(&baseline_nov12);
    predictor_vector.push_back(&svd_nov2);
    predictor_vector.push_back(&svdk_nov9);
    predictor_vector.push_back(&svdk_nov13);*/

    // Initialize your mom
    cout << "Loarning linear blender" << endl;
    LinearBlender linear_blend_predictor(predictor_vector);
    linear_blend_predictor.learn(BLENDER_LEARNING_PARTITION);

    // Get predictions
    cout << "Obtaining predictions" << endl;
    vector<double> res;
    for(int i=0; i < SVDK_Nov13::DATA_COUNT; i++) {
        if(get_um_idx_ratingset(i) == 5){
            res.push_back(0);
        }
    }
    for (int j = 0; j < linear_blend_predictor.predictors_.size(); j++) {
        printf("Predictor number %i\n", j);
        linear_blend_predictor.predictors_[j]->remember(3); 
        printf("\tPredictor loaded. Obtaining predictions...\n");
        int ptcount = 0;
        for(int i=0; i < SVDK_Nov13::DATA_COUNT; i++) {
            if(get_um_idx_ratingset(i) == 5){
                res[ptcount] += (gsl_vector_get(linear_blend_predictor.aggregator_weights_, j) * 
                            linear_blend_predictor.predictors_[j]->predict( get_um_all_usernumber(i), get_um_all_movienumber(i), get_um_all_datenumber(i), i) );
                ptcount++;
            }
        }
        printf("\tFreeing predictor...\n");
        linear_blend_predictor.predictors_[j]->free_mem();
    }

    // Write predictions
    cout << "Outputting results" << endl;
    output_results(res);
    return 0; // Success!
}
