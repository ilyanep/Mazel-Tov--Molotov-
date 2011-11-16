#ifndef SVD_NOV2_H
#define SVD_NOV2_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>

#define SVD_NOV2_PARAM_FILE "../SVD_Nov2/svd_params.dta"
#define SVD_NOV2_BIAS_FILE "../SVD_Nov2/svd_bias.dta"

class SVD_Nov2: public IPredictor{
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
        static const int SVD_DIM = 64; //Seems that only first 14 actually matter
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int LEARN_EPOCHS_MIN = 120; //Papers suggest a big dip in RMSE if >50?
        static const int REFINE_EPOCHS_MIN = 30; //Feasible time-wise
        static const double MIN_RMSE_IMPROVEMENT = 0.0001; //Arbitrary
        static const double LEARN_RATE = 0.001; //Suggested optimal by a lot of papers
        static const double REGUL_PARAM = 0.015; //Suggested optimal by a lot of papers
        static const double REGUL_BIAS_PARAM = 25; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.1; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;
        SVD_Nov2();
        void save_svd(int partition);
        float rmse_probe();
        void learn(int partition, bool refine);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
        virtual void free_mem();
      
    
};
#endif
