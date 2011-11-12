#include "blender.h"
#include <assert.h>
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#include "../all_3_predictor/all_3_predictor.h"
#include "../binary_files/binary_files.h"
#include "../learning_method.h"
#include "../write_data/write_results.h"
#include "../SVD_Oct18/svd_oct18.h"
#include "../SVD_Oct25/svd_oct25.h"
#include "../SVD_Nov2/svd_nov2.h"
#include "../SVDK_Nov9/svdk_nov9.h"
#include "../Baseline_Oct25/baseline_oct25.h"
#include "../movie_knn/movie_knn_pearson.h"

#define BLENDER_LEARNING_PARTITION 4
#ifndef SUBMIT_NUM_POINTS
#define SUBMIT_NUM_POINTS 2749898
#endif

int main() {
    // Pull in the datapoints on which we want predictions
    cout << "Loading and/or checking loaded data" << endl;
    assert(load_mu_qual_usernumber() == 0);
    assert(load_mu_qual_movienumber() == 0);
    assert(load_mu_qual_datenumber() == 0);

    // Initialize all the predictors
    cout << "Initializing predictor vector" << endl;
    vector<IPredictor*> predictor_vector;
    
    Movie_Knn mknn;
    Movie_Knn_Pearson mknn_pearson;
    All3Predictor all_3s; 
    SVD_Oct18 svd_oct18;
    SVD_Oct25 svd_oct25;
	Baseline_Oct25 baseline_oct25;
    SVD_Nov2 svd_nov2;
    SVDK_Nov9 svd_nov9;
    predictor_vector.push_back(&all_3s);
    predictor_vector.push_back(&svd_oct18);
    predictor_vector.push_back(&svd_oct25);
    predictor_vector.push_back(&baseline_oct25);
    predictor_vector.push_back(&svd_nov2);
    predictor_vector.push_back(&mknn);
    predictor_vector.push_back(&mknn_pearson);
    predictor_vector.push_back(&svdk_nov9);

    // Initialize your mom
    cout << "Loarning linear blender" << endl;
    LinearBlender linear_blend_predictor(predictor_vector);
    linear_blend_predictor.learn(BLENDER_LEARNING_PARTITION);

    // Get predictions
    cout << "Obtaining predictions" << endl;
    vector<double> res;
    for(int i=0; i < SUBMIT_NUM_POINTS; ++i) {
        res.push_back(linear_blend_predictor.predict(get_mu_qual_usernumber(i),
                                                     get_mu_qual_movienumber(i),
                                                     get_mu_qual_datenumber(i)));
    }

    // Write predictions
    cout << "Outputting results" << endl;
    output_results(res);
    return 0; // Success!
}
