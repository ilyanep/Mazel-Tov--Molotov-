#include "all_3_predictor.h"
#include "../learning_method.h"

// The all 3 predictor will always return 3. This is more of an exercise in writing a predictor
// than anything else. Also Yaser wants something by Tuesday. 
    void All3Predictor::learn(int partition) { }
    void All3Predictor::remember(int partition) { }

    double All3Predictor::predict(int user, int movie, int time) { return 3.0; }
    void All3Predictor::free_mem() { }
