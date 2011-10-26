#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include "../Baseline/baseline_predictor.h"
#include <string>

#define SVD_DIM_BASE 16 //Seems that only first 14 actually matter
#define USER_COUNT 458293
#define MOVIE_COUNT 17770
#define DATA_COUNT 102416306
#define LEARN_EPOCHS_MIN 60 //Papers suggest a big dip in RMSE if >50?
#define REFINE_EPOCHS_MIN 2 //Feasible time-wise
#define MIN_RMSE_IMPROVEMENT 0.0001 //Arbitrary
#define LEARN_RATE 0.001 //Suggested optimal by a lot of papers
#define REGUL_PARAM 0.015 //Suggested optimal by a lot of papers
//#define REGUL_BIAS_PARAM 25 //Suggested optimal by a lot of papers
#define AVG_RATING 3.6095 //Computed over the first 3 partitions
#define INIT_SVD_VAL 0.1 //Suggested optimal by a lot of papers

#define SVD_BASE_PARAM_FILE "../SVD_Baseline/svd_base_params.dta"

#define SUBMIT_NUM_POINTS 2749898

class SVD_Base: public IPredictor{
    private:
        Baseline base_predict;
        double learn_rate;
        double svd_regul;
        bool data_loaded;
        gsl_matrix *userSVD;
        gsl_matrix *movieSVD;
        double learn_point(int svd_pt, int user, int movie, int time, double rating, bool refining);
        double predict_point(int user, int movie, int time);
        double predict_point_train(int user, int movie, int time, int svd_pt);
        void load_data();
    public:
        SVD_Base();
        void save_svd(int partition);
        double rmse_probe();
        void learn(int partition, bool refine);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
      
    
};
