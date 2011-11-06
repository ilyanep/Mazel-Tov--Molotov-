#include "rbm.h"
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

const int RBM_ALL_POINTS_SIZE = 102416306;

RestrictedBoltzmannMachine::RestrictedBoltzmannMachine() {
    // Initialize weights (probably to 0)

    // Initialize ratings vector
}

// Learn the weights and write them to a file
void RestrictedBoltzmannMachine::learn(int partition) {

}

// Read saved weights from a file
void RestrictedBoltzmannMachine::remember(int partition) {

}

// Return a predictian
double RestrictedBoltzmannMachine::predict(int user, int movie, int date) {

}

// Has this user rated this movie?
bool user_has_rated_movie(int user, int movie) {

}

// For the following, ratings are in vectors of pair<int, int>. The first value in the pair
// is the movie number and the second value is a 5-bit number that shows whether the current
// rating value is "turned on" (since each rating value is a separate output node).
// Return a sample for h_j (a hidden node) given the distribution based on the current given
// ratings (for gibbs sampling).
bool sample_hj_given_user_ratings(int j, vector<pair<int, int> > current_ratings) {

}

// Reconstruct the rating for a partiular movie given the current values of h_i (for gibbs sampling).
bool reconstruct_rating_given_hidden(int movie, int rating, bool* values) {

}

// The gibbs sampler for movie ratings and h_i. Runs for num_steps full steps.
void gibbs_sampler(bool* h, vector<pair<int, int> >* rating_vector, int num_steps) {

}
