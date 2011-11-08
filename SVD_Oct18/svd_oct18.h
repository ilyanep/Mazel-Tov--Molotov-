#ifndef SVD_SVD_OCT18_H
#define SVD_SVD_OCT18_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>

#define OCT18_SVD_DIM 64 //Seems that only first 14 actually matter
#define OCT18_USER_COUNT 458293
#define OCT18_MOVIE_COUNT 17770
#define OCT18_DATA_COUNT 102416306
#define OCT18_LEARN_EPOCHS_MIN 60 //Papers suggest a big dip in RMSE if >50?
#define OCT18_REFINE_EPOCHS_MIN 2 //Feasible time-wise
#define OCT18_MIN_RMSE_IMPROVEMENT 0.0001 //Arbitrary
#define OCT18_LEARN_RATE 0.001 //Suggested optimal by a lot of papers
#define OCT18_REGUL_PARAM 0.015 //Suggested optimal by a lot of papers
#define OCT18_REGUL_BIAS_PARAM 25 //Suggested optimal by a lot of papers
#define OCT18_AVG_RATING 3.6095 //Computed over the first 3 partitions
#define OCT18_INIT_SVD_VAL 0.1 //Suggested optimal by a lot of papers

#define OCT18_SVD_PARAM_FILE "../SVD_Oct18/svd_params.dta"
#define OCT18_SVD_BIAS_FILE "../SVD_Oct18/svd_bias.dta"

#define OCT18_SUBMIT_NUM_POINTS 2749898

class Oct18_SVD: public IPredictor{
    private:
        float learn_rate;
        float svd_regul;
        bool data_loaded;
        gsl_matrix *userSVD;
        gsl_matrix *movieSVD;
        float learn_point(int svd_pt, int user, int movie, float rating, bool refining);
        float predict_point(int user, int movie);
        float predict_point_train(int user, int movie, int svd_pt);
        void load_data();
    public:
        Oct18_SVD();
        void save_svd(int partition);
        float rmse_probe();
        void learn(int partition, bool refine);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
      
    
};
#endif
