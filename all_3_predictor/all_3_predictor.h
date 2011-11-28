#ifndef ALL_3_PREDICTOR_ALL_3_PREDICTOR_H
#define ALL_3_PREDICTOR_ALL_3_PREDICTOR_H

#include "../learning_method.h"

class All3Predictor : public IPredictor {
  public:
    // Learn(), Remember(), and Predict() are described in learning_method.h
    void learn(int partition);
    void remember(int partition);
    double predict(int user, int movie, int date, int index);
    void free_mem();
};

#endif
