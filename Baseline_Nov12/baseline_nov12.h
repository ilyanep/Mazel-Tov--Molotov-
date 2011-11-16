#ifndef BASELINE_NOV12_H
#define BASELINE_NOV12_H
using namespace std;
#include <gsl/gsl_matrix.h>
#include "../learning_method.h"
#include <string>
#include <vector>

#define NOV12_BASELINE_FILE "../Baseline_Nov12/mu_baseline.dta"

class Baseline_Nov12: public IPredictor{
    private:
        bool data_loaded;
        gsl_matrix *userBias;
        gsl_matrix *movieBias;
        vector< vector<int> > freqDates; //[user][date]
        vector< vector<int> > freqNum;  //[user][freq]
        void load_data();

        void learn_by_gradient_descent(int partition);
        void generate_frequency_table(int partition);
        void generate_avg_dates(int partition);
        void generate_freq_spikes();
        double predictPt(int user, int movie, int time, int *userFreq, double *rateFreqRet);
        int findMinIndex(gsl_matrix *mat, int numPts);
        int find_element_vect(vector <int> vect, int element);
    public:
        static const int USER_COUNT = 458293;
        static const int MOVIE_COUNT = 17770;
        static const int DATA_COUNT = 102416306;
        static const int DATE_COUNT = 2243;
        static const int SUBMIT_NUM_POINTS = 2749898;
        static const double AVG_RATING = 3.6095; //Computed over the first 3 partitions

        //Time effects
        static const int MOVIE_BIN_SIZE = 90; //from BelKor
        static const int NUM_MOVIE_BINS = 25; //from BelKor
        static const double USER_DATE_EXP = 0.4; //Optimized by hand 0.01
        static const int NUM_USER_TIME_FACTORS = 40;  //from BelKor

        //Frequencies
        static const double LOG_BASE = 6.76;
        static const double LN_LOG_BASE = 1.91102;
        static const int FREQ_LOG_MAX = 61;

        //Gradient descent
        static const int LEARN_EPOCHS = 40;
        static const double MIN_RMSE_IMPROVEMENT = 0.00001;
        static const double LEARN_RATE_BU = 0.00267;
        static const double LEARN_RATE_BUT = 0.00255;
        static const double LEARN_RATE_AU = 3.11e-06;
        static const double LEARN_RATE_BI = 0.00048;
        static const double LEARN_RATE_BIT = 0.000115;
        static const double LEARN_RATE_CU = 0.00564;
        static const double LEARN_RATE_CUT = 0.00103;
        static const double LEARN_RATE_BIF = 0.00263;
        static const double REGUL_BU = 0.0255;
        static const double REGUL_BUT = 0.00231;
        static const double REGUL_AU = 3.95;
        static const double REGUL_BI = 0.0255;
        static const double REGUL_BIT = 0.0929;
        static const double REGUL_CU = 0.0496;
        static const double REGUL_CUT = 1.90;
        static const double REGUL_BIF = 1.10e-08;

        Baseline_Nov12();
        Baseline_Nov12(bool loadedData);
        double rmse_probe();
        void save_baseline(int partition);
        virtual void learn(int partition);
        virtual double predict(int user, int movie, int time);
        virtual void remember(int partition);
        virtual void free_mem();
      
    
};

#endif
