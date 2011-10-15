#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>

#define USER_COUNT 458293
#define MOVIE_COUNT 17770
#define DATA_COUNT 102416306
#define AVG_RATING 3.6095 //Computed over the first 3 partitions

#define REGUL_BIAS_MOVIE 25
#define REGUL_BIAS_USER 10

#define BASELINE_FILE "mu_baseline.dta"

#define SUBMIT_NUM_POINTS 2749898

class Baseline: public IPredictor{
    private:
        bool data_loaded;
        gsl_matrix *userBias;
        gsl_matrix *movieBias;
        void load_data();
        void calculate_average_biases(int partition);
    public:
        Baseline();
        float rmse_probe();
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
      
    
};
