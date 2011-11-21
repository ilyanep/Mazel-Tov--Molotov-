#ifndef SVDK_NOV21_H
#define SVDK_NOV21_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>
#include <vector>
using namespace std;
#define NOV21_RMSE_FILE "../SVDK_Nov21/rmse.dta"
#define NOV21_RATINGS_FILE "../SVDK_Nov21/ratings.dta"

class SVDK_Nov21: public IPredictor{
    private:
        bool userMoviesGenerated;
        bool data_loaded;
        int learned_dim;

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

        double learn_point(int svd_pt, int user, int movie, double rating, int pt_num);
        double predict_point(int user, int movie, int date);
        double predict_train(int user, int movie, double bias, double nSum);
        void load_data();
    public:
        static const int SVD_DIM = 96;
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int LEARN_EPOCHS_MIN = 40;
        static const double MIN_RMSE_IMPROVEMENT = 0.00003; //Arbitrary
        static const double LEARN_RATE = 0.002; //Suggested optimal by a lot of papers
        static const double FEATURE_REGUL_PARAM = 0.015; //Suggested optimal by a lot of papers
        static const double BIAS_REGUL_PARAM = 0.05; //Patarek paper
        static const double REGUL_BIAS_PARAM = 25; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.01; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;

        SVDK_Nov21();
        void save(int svd_pt);
        double rmse_probe();
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
        virtual void free_mem();
      
    
};
#endif
