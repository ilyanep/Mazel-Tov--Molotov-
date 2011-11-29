#ifndef BASELINE_BASELINE_PREDICTOR_H
#define BASELINE_BASELINE_PREDICTOR_H
#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>

#define USER_COUNT 458293
#define MOVIE_COUNT 17770
#define DATA_COUNT 102416306
#define DATE_COUNT 2243
#define AVG_RATING 3.6095 //Computed over the first 3 partitions

//Gradient descent
#define GRAD_STEP 0.001
#define DERIV_STEP 0.1
#define GRAD_MIN_IMPROVEMENT 0.0001

//Global averages
#define REGUL_BIAS_MOVIE 25 //from BelKor
#define REGUL_BIAS_USER 10 //from BelKor

//Time effects
#define MOVIE_BIN_SIZE 90 //from BelKor
#define NUM_MOVIE_BINS 25 //from BelKor
#define USER_DATE_EXP 0.01 //Optimized by hand 0.01
#define NUM_USER_TIME_FACTORS 40  //from BelKor
#define USER_FREQ_REGUL 3
#define USER_DATE_REGUL 50 //Optimized by hand 50

#define REGUL_BIAS_MOVIE 25
#define REGUL_BIAS_USER 10

#define BASELINE_FILE "../Baseline/mu_baseline.dta"

#define SUBMIT_NUM_POINTS 2749898

class Baseline: public IPredictor{
    private:
        bool data_loaded;
        gsl_matrix *userBias;
        gsl_matrix *movieBias;
        void load_data();
        void calculate_average_biases(int partition);
        void calculate_movie_time_effects(int partition);
        void calculate_user_time_gradient(int partition);
        void calculate_user_time_spikes(int partition);
        void find_user_rating_days(int partition);
        int findMinIndex(gsl_matrix *mat, int numPts);
        /*
        double rmse_probe_part(int partition);
        double rmse_probe_user(int partition, int user, int startIndex, int endIndex);
        double rmse_probe_movie(int partition, int movie);
        void refine_by_gradient_descent(int partition);
        */
    public:
        Baseline();
        Baseline(bool loadedData);
        double rmse_probe();
        void save_baseline(int partition);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time, int index);
        virtual void remember(int partition);
      
    
};

#endif
