#include "rbm.h"
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

using namespace std;

RestrictedBoltzmannMachine::RestrictedBoltzmannMachine() {
    initialized_ = false;

    weights_ = (double***)malloc(sizeof(double**) * RBM_MOVIE_COUNT);
    assert(weights_ != NULL);
    delta_weights_ = (double***)malloc(sizeof(double**) * RBM_MOVIE_COUNT);
    assert(delta_weights_ != NULL);

    // Initialize weights (probably to 0)
    cout << "Initializing Weights vector" << endl;
    for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
        weights_[i] = (double**)malloc(sizeof(double*) * RBM_NUM_HIDDEN_UNITS);
        assert(weights_[i] != NULL);
        delta_weights_[i] = (double**)malloc(sizeof(double*) * RBM_NUM_HIDDEN_UNITS);
        assert(delta_weights_[i] != NULL);
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            weights_[i][j] = (double*)malloc(sizeof(double) * (RBM_HIGHEST_RATING + 1));
            assert(weights_[i][j] != NULL);
            delta_weights_[i][j] = (double*)malloc(sizeof(double) * (RBM_HIGHEST_RATING + 1));
            assert(delta_weights_[i][j] != NULL);
            for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                weights_[i][j][k] = 0;
                delta_weights_[i][j][k] = 0;
            }
        }   
    }
}

RestrictedBoltzmannMachine::~RestrictedBoltzmannMachine() {
    for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            free(weights_[i][j]);
            free(delta_weights_[i][j]);
        }
        free(weights_[i]);
        free(delta_weights_[i]);
    }

    free(weights_);
    free(delta_weights_);
}

// Learn the weights and write them to a file
void RestrictedBoltzmannMachine::learn(int partition) {

    // Initialize ratings vector
    cout << "Loading ratings and initializing ratings vector" << endl;
    assert(load_um_all_usernumber() == 0);
    assert(load_um_all_movienumber() == 0);
    assert(load_um_all_rating() == 0);
    assert(load_um_idx_ratingset() == 0);
    int current_user = -1;
    user_ratings_.resize(RBM_USER_COUNT);
    for(int p = 0; p < RBM_TOTAL_NUM_POINTS; ++p) {
        if (get_um_idx_ratingset(p) <= partition) {
            user_ratings_[get_um_all_usernumber(p)].push_back(
                          make_pair(get_um_all_movienumber(p), pow(2, get_um_all_rating(p)))); 
        }
    }

    // Hidden units array.
    bool h[RBM_NUM_HIDDEN_UNITS];

    for(int e = 0; e < RBM_NUM_EPOCHS; ++e) {
        cout << "Starting epoch: " << e << " of " << RBM_NUM_EPOCHS << endl;
        for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
            for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                    delta_weights_[i][j][k] = 0;
                }
            }   
        }

        // Batches of 1000 users
        for(int u = 0; u < RBM_USER_COUNT; ++u) {
            cout << "Gradient descent for user" << u << " of " << RBM_USER_COUNT << endl;

            for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                h[j] = false;
            }

            // Gibbs sample the current step
            vector<pair<int, int> > working_ratings;
            for(int r = 0; r < user_ratings_[u].size(); ++r) {
                working_ratings.push_back(make_pair(user_ratings_[u][r].first, user_ratings_[u][r].second));
            } 

            cout << "Beginning gibbs sampler" << endl;
            gibbs_sampler(h, &working_ratings, (e / RBM_EPOCHS_PER_T_INCREASE) + RBM_STARTING_T); 

            // Calculate deltas for every weight
            bool user_rated_movie = false;
            int current_movie_rating;
            double temp_sum;
            double data_ev;
            double sampled_ev;
            for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
                cout << "Starting movie " << i << endl;
                user_rated_movie = false;
                for(int r = 0; r < user_ratings_[u].size(); ++r) {
                    if(user_ratings_[u][r].first == i) { 
                        user_rated_movie = true;
                        current_movie_rating = (log(working_ratings[r].second) / log(2));
                        break; 
                    }
                }
                if (!user_rated_movie) { continue; }

                for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                    cout << "Starting hidden unit " << j << endl; 
                    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                        cout << "Starting machine with rating " << k << endl;
                        // Calculate Data EV
                        temp_sum = 0;
                        if(current_movie_rating == k) {
                            for(int m = 0; m < user_ratings_[u].size(); ++m) {
                                for(int n = 1; n <= RBM_HIGHEST_RATING; ++n) {
                                   temp_sum += ((user_ratings_[u][m].second >> n) & 1) * weights_[user_ratings_[u][m].first][j][n];  
                                }
                            }
                            data_ev = 1 / (1 + exp(-1 * (temp_sum)));
                        } 
                        else {
                            data_ev = 0;
                        }
                         
                        // Calculate Gibbs Sampled EV = P(v_{ik} = 1 | H) * P(h_j = 1 | V)
                        temp_sum = 0;
                        for(int m = 0; m < working_ratings.size(); ++m) {
                            for(int n = 1; n <= RBM_HIGHEST_RATING; ++n) {
                               temp_sum += ((working_ratings[m].second >> n) & 1) * weights_[working_ratings[m].first][j][n]; 
                            }
                        }
                        sampled_ev = 1 / (1 + exp(-1 * (temp_sum))); 

                        double normalize_sum = 0;
                        double correct_rating = 0;
                        double current_sum = 0;
                        for(int n = 1; n <= RBM_HIGHEST_RATING; ++k) {
                            current_sum = 0;
                            for(int m = 0; m < RBM_NUM_HIDDEN_UNITS; ++j) {
                                current_sum += h[m] * weights_[i][m][n];    
                            }
                            normalize_sum += exp(current_sum);
                            if(k == n) { correct_rating += exp(current_sum); }
                        }
                        sampled_ev *= ((correct_rating)/(normalize_sum)); 

                        // Use these to calculate deltas
                        delta_weights_[i][j][k] += (1/(double)RBM_USER_COUNT) * RBM_LEARNING_RATE * (data_ev - sampled_ev);  
                        cout << "Contributed " << delta_weights_[i][j][k] << " to " << i << " " << j << " " << k << endl; 
                    }
                }
            }

            // Change weights
            cout << "Applying weight deltas" << endl;
            for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
                for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                        weights_[i][j][k] += delta_weights_[i][j][k];
                    }
                }
            } 
        }
    }

    cout << "Saving weights to file." << endl;
    // Save weights to a file
    ofstream outfile;
    outfile.open(RBM_PARAM_FILE.c_str()); // Defined in the header file
    outfile << partition << "\n";
    for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                outfile << weights_[i][j][k] << endl;
            }
        }
    }

    initialized_ = true;
} // It's over!

// Read saved weights from a file
void RestrictedBoltzmannMachine::remember(int partition) {
    // Initialize ratings vector
    cout << "Loading ratings and initializing ratings vector" << endl;
    assert(load_um_all_usernumber() == 0);
    assert(load_um_all_movienumber() == 0);
    assert(load_um_all_rating() == 0);
    assert(load_um_idx_ratingset() == 0);
    int current_user = -1;
    user_ratings_.resize(RBM_USER_COUNT);
    for(int p = 0; p < RBM_TOTAL_NUM_POINTS; ++p) {
        if (get_um_idx_ratingset(p) <= partition) {
            user_ratings_[get_um_all_usernumber(p)].push_back(
                          make_pair(get_um_all_movienumber(p), pow(2, get_um_all_rating(p)))); 
        }
    }

    ifstream infile(RBM_PARAM_FILE.c_str());
    string line;
    if(infile.is_open()) {
        getline(infile, line);
        assert(atoi(line.c_str()) == partition);
        for(int i = 0; i < RBM_MOVIE_COUNT; ++i) {
            for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) { 
                for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                    assert(infile.good());
                    getline(infile, line);
                    weights_[i][j][k] = strtod(line.c_str(), NULL);
                } 
            }
        }
        
    }

    initialized_ = true;
}

// Return a predictian
double RestrictedBoltzmannMachine::predict(int user, int movie, int date) {
    assert(initialized_);
    double total_product[6];
    double normalize_sum;
    double temp_sum;
    int current_rating;
    normalize_sum = 0;
    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
        total_product[k] = 1;
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            temp_sum = 0;
            for(int r = 0; r < user_ratings_[user].size(); ++r) {
                current_rating = ((log(user_ratings_[user][r].second))/log(2));
                temp_sum += (weights_[user_ratings_[user][r].first][j][current_rating] + 
                             weights_[movie][j][k]); 
            }            
            total_product[k] *= (1+ exp(temp_sum));
        }
        normalize_sum += total_product[k];
    }

    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
        total_product[k] /= normalize_sum;
    }

    double expected_rating = 0;
    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
        expected_rating += k * (total_product[k]);
    }

    return expected_rating;
}

// For the following, ratings are in vectors of pair<int, int>. The first value in the pair
// is the movie number and the second value is a 5-bit number that shows whether the current
// rating value is "turned on" (since each rating value is a separate output node).
// Return a sample for h_j (a hidden node) given the distribution based on the current given
// ratings (for gibbs sampling).
bool RestrictedBoltzmannMachine::sample_hj_given_user_ratings(int j, vector<pair<int, int> > current_ratings) {
    double temp_sum = 0;
    for(int r = 0; r < current_ratings.size(); ++r) {
        for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
            temp_sum += ((current_ratings[r].second >> k) & 1) * weights_[current_ratings[r].first][j][k]; 
        }
    }
    double prob = 1 / (1 + exp(-1 * temp_sum));
    return (((double)rand()/(double)RAND_MAX) <= prob); 
}

// Reconstruct the rating for a partiular movie given the current values of h_i (for gibbs sampling).
bool RestrictedBoltzmannMachine::reconstruct_rating_given_hidden(int movie, int rating, bool* values) {
    double normalize_sum = 0;
    double correct_rating = 0;
    double current_sum = 0;
    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
        current_sum = 0;
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            current_sum += values[j] * weights_[movie][j][k];    
        }
        normalize_sum += exp(current_sum);
        if(k == rating) { correct_rating += exp(current_sum); }
    }
    return (((double)rand()/(double)RAND_MAX) <= ((correct_rating)/(normalize_sum)));
}

// The gibbs sampler for movie ratings and h_i. Runs for num_steps full steps.
void RestrictedBoltzmannMachine::gibbs_sampler(bool* h, vector<pair<int, int> >* rating_vector, int num_steps) {
    // Initialize temp hidden units and user ratings arrays.
    int rating_int = 0;
    for(int i=0; i< num_steps; ++i) {
        cout << "Gibbs step " << num_steps << endl;
        // Sample each hidden unit and save into temp array
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            h[j] = sample_hj_given_user_ratings(j, *rating_vector);
        }

        // Reconstruct data for each user and save into temp array
        rating_int = 0;
        for(int r = 0; r < rating_vector->size(); ++r) {
            for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                if ( reconstruct_rating_given_hidden(rating_vector->at(r).first, k, h)) {
                    rating_int += pow(2, k);
                }
            }
            rating_vector->at(r).second = rating_int;
        }

        // Sample each hidden unit again and save once more
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            h[j] = sample_hj_given_user_ratings(j, *rating_vector);
        }
    }
}

double RestrictedBoltzmannMachine::rmse_probe() {
    double RMSE = 0;
    int count = 0;
    for(int i = 0; i < RBM_TOTAL_NUM_POINTS; ++i) {
        if(get_um_idx_ratingset(i) == 4) {
            double prediction = predict(get_um_all_usernumber(i), (int)get_um_all_movienumber(i), (int)get_um_all_datenumber(i));
            double error = (prediction - (double)get_um_all_rating(i));
            RMSE += (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}
