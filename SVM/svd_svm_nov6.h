#ifndef SVD_SVM_NOV6_H
#define SVD_SVM_NOV6_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include "../Baseline_Oct25/baseline_oct25.h"
#include <string>

#define SVD_OCT25_PARAM_FILE "../SVM/svd_params.dta"
#define SVM_TRAIN_FILE "../SVM/svm_train.dta"
#define SVM_TEST_FILE "../SVM/svm_test.dta"
#define SVM_OUTPUT_FILE "../SVM/svm_output.dta"

class SVD_SVM_Nov6: public IPredictor{
    private:
        Oct25_Baseline base_predict;
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
        static const int SVD_DIM = 16; //Seems that only first 14 actually matter
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int LEARN_EPOCHS_MIN = 60; //Papers suggest a big dip in RMSE if >50?
        static const int REFINE_EPOCHS_MIN = 2; //Feasible time-wise
        static const double MIN_RMSE_IMPROVEMENT = 0.0001; //Arbitrary
        static const double LEARN_RATE = 0.001; //Suggested optimal by a lot of papers
        static const double REGUL_PARAM = 0.015; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.1; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;
        SVD_SVM_Nov6();
        void save_svd(int partition);
        double rmse_probe();
        void learn(int partition, bool refine);
        void output_svm_data_files(int train_partition);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        //double svdpredict(int user, int movie, int time);
        virtual void remember(int partition);
        
      
    
};
#endif
