#ifndef SVD_SVM_NOV6_H
#define SVD_SVM_NOV6_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include "../Baseline_Oct25/baseline_oct25.h"
#include <string>

#define SVD_OCT25_PARAM_FILE "../SVM/svd_params.dta"
#define SVM_OUTPUT_FILE "../SVM/svm_output.dta"

class SVD_SVM_Nov6: public IPredictor{
    private:
        bool data_loaded;
        bool svd_loaded;
        gsl_matrix *userSVD;
        gsl_matrix *movieSVD;
        Baseline_Oct25 base_predict;
        struct svm_problem trainData;
        struct svm_parameter param;
        struct svm_model *model;
        struct svm_node *x_space;
        double svd_predict(int user, int movie, int time);
        void load_data();
        void load_svd_parameters();
        void generate_svm_input(int partition);
        void generate_svm_param();
    public:
        static const int SVD_DIM = 16; //Seems that only first 14 actually matter
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int BASE_PARTITION = 3;
        static const int SVD_PARTITION = 3;
        static const int LEARN_EPOCHS_MIN = 60; //Papers suggest a big dip in RMSE if >50?
        static const int REFINE_EPOCHS_MIN = 2; //Feasible time-wise
        static const double MIN_RMSE_IMPROVEMENT = 0.0001; //Arbitrary
        static const double LEARN_RATE = 0.001; //Suggested optimal by a lot of papers
        static const double REGUL_PARAM = 0.015; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.1; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;

        SVD_SVM_Nov6();
        void save_svm();
        double rmse_probe();
        void learn(int partition, bool refine);
        void free_svd();
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
        
      
    
};
#endif
