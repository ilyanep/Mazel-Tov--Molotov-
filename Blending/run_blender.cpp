#include "blender.h"
#include <assert.h>
#include <vector>
#include <iostream>
#include "../binary_files/binary_files.h"
#include "../learning_method.h"
#include "../write_data/write_results.h"
#include "../SVD/learn_svd.h"
#include "../Baseline/baseline_predictor.h"

using namespace std;

#define BLENDER_LEARNING_PARTITION 3
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
    SVD svd_predictor; // Oh god that's a terrible class name :P
    predictor_vector.push_back(&svd_predictor);
    Baseline baseline_predictor; // Wow.
    predictor_vector.push_back(&baseline_predictor);

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
