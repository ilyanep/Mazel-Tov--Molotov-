#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>

#define USER_COUNT 458293
#define MOVIE_COUNT 17770
#define DATA_COUNT 102416306
#define DATE_COUNT 2243
#define AVG_RATING 3.6095 //Computed over the first 3 partitions

//Global averages
<<<<<<< HEAD
#define REGUL_BIAS_MOVIE 25 //from BelKor
#define REGUL_BIAS_USER 10 //from BelKor

//Time effects
#define MOVIE_BIN_SIZE 90 //from BelKor
#define NUM_MOVIE_BINS 25 //from BelKor
#define USER_DATE_EXP 4.7 //Optimized by hand
#define USER_DATE_REGUL 2 //Optimized by hand
=======
#define REGUL_BIAS_MOVIE 25
#define REGUL_BIAS_USER 10
>>>>>>> 4313a1490d921d164629762aea4bd8454a48cae5

//Time effects
#define MOVIE_BIN_SIZE 90
#define NUM_MOVIE_BINS 25
#define USER_DATE_EXP 4 //5.1
#define USER_DATE_REGUL 2

#define BASELINE_FILE "mu_baseline.dta"

#define SUBMIT_NUM_POINTS 2749898

class Baseline: public IPredictor{
    private:
        bool data_loaded;
        gsl_matrix *userBias;
        gsl_matrix *movieBias;
        void load_data();
        void calculate_average_biases(int partition);
        void calculate_movie_time_effects(int partition);
        void calculate_user_time_effects(int partition);
    public:
        Baseline();
        float rmse_probe();
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
      
    
};
