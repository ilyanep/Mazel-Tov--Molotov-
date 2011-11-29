#ifndef BLENDING_BLENDER_H
#define BLENDING_BLENDER_H

// Linear aggregator class to linearly blend existing predictions
#include "../learning_method.h"
#include <gsl/gsl_blas.h>
#include <gsl/gsl_matrix.h>
#include <vector>

using namespace std;

typedef long long int64;

class LinearBlender : public IPredictor {
    public:
        vector<IPredictor*> predictors_;
        gsl_vector* aggregator_weights_;
        bool initialized_;
        gsl_vector* aggregator_solution(gsl_matrix* predictions_matrix, gsl_vector* ratings_vector,
                                        int64 num_pred_points, int64 num_predictors);

        LinearBlender(vector<IPredictor*> predictors);
        //learn(), remember(), and predict() are defined in learning_methods.h
        void learn(int partition);
        void remember(int partition);
        double predict(int user, int movie, int date, int index);
        void free_mem();
};

#endif
