#ifndef SVDK_NOV13_H
#define SVDK_NOV13_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>
#include "../Baseline_Nov12/baseline_nov12.h"

#define NOV13_SVDK_PARAM_FILE "../SVDK_Nov13/svd_params.dta"
#define NOV13_SVDK_BIAS_FILE "../SVDK_Nov13/svd_bias.dta"

class SVDK_Nov13: public IPredictor{
    private:
        Baseline_Nov12 base_predict;
        double learn_rate;
        bool data_loaded;
        gsl_matrix *userSVD;
        gsl_matrix *movieSVD;
        double learn_point(int svd_pt, int user, int movie, int date, double rating, bool refining);
        double predict_point(int user, int movie, int date);
        double predict_point_train(int user, int movie, int date, int svd_pt);
        void load_data();
    public:
        static const int SVD_DIM = 96; //Seems that only first 14 actually matter
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int LEARN_EPOCHS_MIN = 120; //Papers suggest a big dip in RMSE if >50?
        static const int REFINE_EPOCHS_MIN = 30; //Feasible time-wise
        static const double MIN_RMSE_IMPROVEMENT = 0.0001; //Arbitrary
        static const double LEARN_RATE = 0.001; //Suggested optimal by a lot of papers
        static const double FEATURE_REGUL_PARAM = 0.02; //Suggested optimal by a lot of papers
        static const double BIAS_REGUL_PARAM = 0.05; //Patarek paper
        static const double REGUL_BIAS_PARAM = 25; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.1; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;

        SVDK_Nov13();
        void save_svd(int partition);
        double rmse_probe();
        void learn(int partition, bool refine);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
      
    
};
#endif
