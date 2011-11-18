#ifndef RBM_RBM_H
#define RBM_RBM_H

// Restricted Boltzmann Machine to learn stuff
#include "../learning_method.h"
#include <string>
#include <utility>
#include <vector>


using namespace std;

typedef long long int64; // Oh god so janky.
const int RBM_NUM_HIDDEN_UNITS = 100;
const int RBM_USER_COUNT = 458293+1;
const int RBM_MOVIE_COUNT = 17770+1;
const int RBM_TOTAL_NUM_POINTS = 102416306;
const int RBM_HIGHEST_RATING = 5;
const double RBM_LEARNING_RATE = .01;
const int RBM_EPOCHS_PER_T_INCREASE = 10;
const int RBM_STARTING_T = 1;
const int RBM_NUM_EPOCHS = 1;
const string RBM_PARAM_FILE = "../RBM/rbm_params.dta";

class RestrictedBoltzmannMachine : public IPredictor {
    private:
        // user_ratings[i][j] is user i's jth rating -- first is movie, second is rating
        vector<vector<pair<int, int> > > user_ratings_;
        double*** weights_;
        double*** delta_weights_;
        bool initialized_;

    public:
        RestrictedBoltzmannMachine();
        ~RestrictedBoltzmannMachine();
        // The following three are defined as part of IPredictor
        void learn(int partition);
        void remember(int partition);
        double predict(int user, int movie, int date);
        // RBM-specific functions
        bool user_has_rated_movie(int user, int movie);
        // Ratings vectors in the following have a 5-bit int with bit i set if rating i is set
        // They are not ints like you'd expect
        bool sample_hj_given_user_ratings(int j, vector<pair<int, int> > current_ratings);
        bool reconstruct_rating_given_hidden(int movie, int rating, bool* values);
        void gibbs_sampler(bool* h, vector<pair<int, int> >* rating_vector, int num_steps);
        double rmse_probe();
};

#endif
