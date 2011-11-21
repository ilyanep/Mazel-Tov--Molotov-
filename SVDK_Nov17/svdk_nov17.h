#ifndef SVDK_NOV17_H
#define SVDK_NOV17_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>
#include "../Baseline_Nov19/baseline_nov19.h"
#include <vector>
using namespace std;
#define NOV17_SVDK_PARAM_FILE "../SVDK_Nov17/svd_params.dta"
#define NOV17_RMSE_FILE "../SVDK_Nov17/rmse.dta"
#define NOV17_RATINGS_FILE "../SVDK_Nov17/ratings.dta"

class SVDK_Nov17: public IPredictor{
    private:
        Baseline_Nov19 base_predict;

        bool userMoviesGenerated;
        bool baseLoaded;
        bool data_loaded;

        double learn_rate;
        double userNSum;
        double userNChange;
        int userNNum;

        gsl_matrix *ratings;
        gsl_matrix *userSVD;
        gsl_matrix *movieSVD;
        vector < vector <int> > userMovies;
        vector <double> rmsein;
        vector <double> rmseout;    

        double learn_point(int svd_pt, int user, int movie, double rating, int pt_num, bool refining);
        double predict_point(int user, int movie, int date);
        double predict_train(int user, int movie, double bias, int svd_pt, double nSum);
        void load_data();
    public:
        static const int SVD_DIM = 48; //Seems that only first 14 actually matter
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int LEARN_EPOCHS_MIN = 40; //Papers suggest a big dip in RMSE if >50?
        static const int REFINE_EPOCHS_MIN = 40; //Feasible time-wise
        static const double MIN_RMSE_IMPROVEMENT = 0.00001; //Arbitrary
        static const double LEARN_RATE = 0.008; //Suggested optimal by a lot of papers
        static const double FEATURE_REGUL_PARAM = 0.015; //Suggested optimal by a lot of papers
        static const double BIAS_REGUL_PARAM = 0.05; //Patarek paper
        static const double REGUL_BIAS_PARAM = 25; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.1; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;

        SVDK_Nov17();
        void save_svd(int partition);
        double rmse_probe();
        void learn(int partition, bool refine);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
        virtual void free_mem();
      
    
};
#endif
