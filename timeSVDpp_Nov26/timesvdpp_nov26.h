#ifndef TIMESVDPP_NOV26_H
#define TIMESVDPP_NOV26_H

#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>
#include <vector>
using namespace std;
#define NOV26_RMSE_FILE "../timeSVDpp_Nov26/rmse.dta"
#define NOV26_RATINGS_FILE "../timeSVDpp_Nov26/ratings.dta"
#define NOV26_BASELINE_FILE "../timeSVDpp_Nov26/um_baseline.dta"

class timeSVDpp_Nov26: public IPredictor{
    private:
        bool data_loaded;
        int learned_dim;

        double lrd;
        double lrdb;
        double userNSum;
        double userNChange;
        int userNNum;

        gsl_matrix *ratings;
        gsl_matrix *userFreqSpikes; //[user][freqDate0 freqDate1 ... ]
        gsl_matrix *avgRatingDates; //[user][avgRatingDate]
        gsl_matrix *userSVD; //[user][intercept slope spike0 spike1 ...]
        gsl_matrix *movieSVD; //[movie][nfactor globalBias movieBin0_avg movieBin1_avg... freqfact0 freqfact1 ...]

        gsl_matrix *userBias; //[user][intercept slope multIntercept spikeAvg0 spikeAvg1 ... spikeMult0 spikeMult1 ...
        gsl_matrix *movieBias; //[movie][globalBias movieBin0_avg movieBin1_avg ... freqFact0 freqFact1 ...]

        vector< vector<int> > freqDates; //[user][date]
        vector< vector<int> > freqNum;  //[user][freq]
        vector < vector <int> > userMovies; //[user][ratedMovie0 ratedMovie1 ...]
        vector <double> rmsein;
        vector <double> rmseout;    

        void generate_frequency_table(int partition);
        void generate_avg_dates(int partition);
        void generate_freq_spikes();
        int findMinIndex(gsl_matrix *mat, int numPts);
        int find_element_vect(vector <int> vect, int element);

        double learn_point(int svd_pt, int user, int movie, int date, double rating, int pt_num);
        double predict_train(int user, int movie, 
                                      double dateFactor, int movieBin, int spikeDate, int rateFreq, 
                                      double svd_prev, double nSum, int svd_pt);
        
        void load_data();
    public:
        static const int SVD_DIM = 256;
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int LEARN_EPOCHS_MIN = 10;
        static const double MIN_RMSE_IMPROVEMENT = 0.00003; //Arbitrary
        static const double LEARN_RATE = 0.008; //Suggested optimal by a lot of papers
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions
        static const double INIT_SVD_VAL = 0.1; //Suggested optimal by a lot of papers
        static const int SUBMIT_NUM_POINTS = 2749898;

        //Time effects
        static const int MOVIE_BIN_SIZE = 90; //from BelKor
        static const int NUM_MOVIE_BINS = 25; //from BelKor
        static const double USER_DATE_EXP = 0.4; //from BelKor
        static const int NUM_USER_TIME_FACTORS = 40;  //from BelKor

        //Frequencies
        static const double LOG_BASE = 6.76;
        static const double LN_LOG_BASE = 1.91102;
        static const int FREQ_LOG_MAX = 61;

        //Gradient descent
        static const double SVD_LR_DECAY = 0.997;
        static const double BASE_LR_DECAY = 0.997;
        static const double LEARN_RATE_BU = 0.00267;
        static const double LEARN_RATE_BUT = 0.00255;
        static const double LEARN_RATE_AU = 3.11e-06;
        static const double LEARN_RATE_BI = 0.00048;
        static const double LEARN_RATE_BIT = 0.000115;
        static const double LEARN_RATE_CU = 0.00564;
        static const double LEARN_RATE_CUT = 0.00103;
        static const double LEARN_RATE_BIF = 0.00263;
        static const double LEARN_RATE_SU = 0.008;
        static const double LEARN_RATE_SUT = 1e-5;
        static const double LEARN_RATE_SUN = 0.008;
        static const double LEARN_RATE_SUTS = 0.004;
        static const double LEARN_RATE_SI = 0.008;
        static const double LEARN_RATE_SIT = 0.000115;
        static const double LEARN_RATE_SIF = 2e-5;
        static const double REGUL_BU = 0.0255;
        static const double REGUL_BUT = 0.00231;
        static const double REGUL_AU = 3.95;
        static const double REGUL_BI = 0.0255;
        static const double REGUL_BIT = 0.0929;
        static const double REGUL_CU = 0.0496;
        static const double REGUL_CUT = 1.90;
        static const double REGUL_BIF = 1.10e-08;
        static const double REGUL_SU = 0.0015;
        static const double REGUL_SUT = 50.0;
        static const double REGUL_SUN = 0.0015;
        static const double REGUL_SUTS = 0.01;
        static const double REGUL_SI = 0.0015;
        static const double REGUL_SIT = 0.01;
        static const double REGUL_SIF = 0.02;

        timeSVDpp_Nov26();
        void save(int svd_pt);
        double rmse_probe();
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time, int index);
        double predict_point(int user, int movie, int date, int allIndex);
        virtual void remember(int partition);
        virtual void free_mem();
      
    
};
#endif
