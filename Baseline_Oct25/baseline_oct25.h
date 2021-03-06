#ifndef BASELINE_OCT25_H
#define BASELINE_OCT25_H
#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>

#define OCT25_BASELINE_FILE "../Baseline_Oct25/mu_baseline.dta"

#define OCT25_SUBMIT_NUM_POINTS 2749898

class Baseline_Oct25: public IPredictor{
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
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int DATE_COUNT = 2243;
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions

        //Global averages
        static const double REGUL_BIAS_MOVIE = 25; //from BelKor
        static const double REGUL_BIAS_USER = 10; //from BelKor

        //Time effects
        static const int MOVIE_BIN_SIZE = 90; //from BelKor
        static const int NUM_MOVIE_BINS = 25; //from BelKor
        static const double USER_DATE_EXP = 0.01; //Optimized by hand 0.01
        static const int NUM_USER_TIME_FACTORS = 40;  //from BelKor
        static const int USER_FREQ_REGUL = 3;
        static const int USER_DATE_REGUL = 50; //Optimized by hand 50

        Baseline_Oct25();
        Baseline_Oct25(bool loadedData);
        double rmse_probe();
        void save_baseline(int partition);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time, int index);
        virtual void remember(int partition);
        virtual void free_mem();
      
    
};

#endif
