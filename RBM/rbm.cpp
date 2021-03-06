#include "rbm.h"
#include "../binary_files/binary_files.h"
#include <assert.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <time.h>

using namespace std;

// Hacky
double** allocate_double_matrix(int a, int b) {
    double** matrix = (double**)malloc(sizeof(double*) * a);
    assert(matrix != NULL);

    for(int i=0; i < a; ++i) {
        matrix[i] = (double*)malloc(sizeof(double) * b);
        assert(matrix[i] != NULL);
        for(int j = 0; j < b; ++j) {
            matrix[i][j] = 0;
        }
    }
    return matrix;
}

void free_double_matrix(double** matrix, int a, int b) {
    for(int i=0; i < a; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

double*** allocate_triple_matrix(int a, int b, int c) {
    double*** matrix = (double***)malloc(sizeof(double**) * a);
    assert(matrix != NULL);

    for(int i=0; i < a; ++i) {
        matrix[i] = (double**)malloc(sizeof(double*) * b);
        assert(matrix[i] != NULL);
        for(int j = 0; j < b; ++j) {
            matrix[i][j] = (double*)malloc(sizeof(double) * c);
            assert(matrix[i][j] != NULL);
            for(int k = 0; k < c; ++k) {
                matrix[i][j][k] = 0;
            }
        }
    }
    return matrix;
}

void free_triple_matrix(double*** matrix, int a, int b, int c) {
    for(int i=0; i < a; ++i) {
        for(int j = 0; j < b; ++j) {
            free(matrix[i][j]);
        }
        free(matrix[i]);
    }
    free(matrix);
}

RestrictedBoltzmannMachine::RestrictedBoltzmannMachine() {
    initialized_ = false;

    weights_ = allocate_triple_matrix(RBM_MOVIE_COUNT, RBM_NUM_HIDDEN_UNITS, RBM_HIGHEST_RATING + 1);
    delta_weights_ = allocate_triple_matrix(RBM_MOVIE_COUNT, RBM_NUM_HIDDEN_UNITS, RBM_HIGHEST_RATING + 1);
}

RestrictedBoltzmannMachine::~RestrictedBoltzmannMachine() {
    free_triple_matrix(weights_, RBM_MOVIE_COUNT, RBM_NUM_HIDDEN_UNITS, RBM_HIGHEST_RATING + 1);
    free_triple_matrix(delta_weights_, RBM_MOVIE_COUNT, RBM_NUM_HIDDEN_UNITS, RBM_HIGHEST_RATING + 1);
}

void RestrictedBoltzmannMachine::free_mem() {
    initialized_ = false;
    free_triple_matrix(weights_, RBM_MOVIE_COUNT, RBM_NUM_HIDDEN_UNITS, RBM_HIGHEST_RATING + 1);
    free_triple_matrix(delta_weights_, RBM_MOVIE_COUNT, RBM_NUM_HIDDEN_UNITS, RBM_HIGHEST_RATING + 1);
}

// Learn the weights and write them to a file
void RestrictedBoltzmannMachine::learn(int partition) {
    srand(time(NULL));
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

    // Half the memory consumption
    free_um_all_usernumber();
    free_um_all_movienumber();
    free_um_all_rating();
    free_um_idx_ratingset();

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

        initialized_ = true; // Hacky
        cout << "RMSE at current epoch: " << rmse_probe() << endl;
        initialized_ = false; // Prevent from making predictions before done.

        for(int u = 0; u < RBM_USER_COUNT; ++u) {
            //cout << "Gradient descent for user" << u << " of " << RBM_USER_COUNT << endl;
            if(u % 500 == 0) { cerr << "Working on user " << u << endl; }

            for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                h[j] = false;
            }

            // Gibbs sample the current step
            vector<pair<int, int> > working_ratings;
            for(int r = 0; r < user_ratings_[u].size(); ++r) {
                working_ratings.push_back(make_pair(user_ratings_[u][r].first, user_ratings_[u][r].second));
            } 

            //cout << "Beginning gibbs sampler" << endl;
            gibbs_sampler(h, &working_ratings, (e / RBM_EPOCHS_PER_T_INCREASE) + RBM_STARTING_T); 
            //cout << "Gibbs sampler complete" << endl;
            assert(working_ratings.size() == user_ratings_[u].size());

            // Calculate data ev <v_i^kh_j> and hidden unit ev <h_j>, as these can be reused in the next big loop
            //cout << "Calculating h evs and data evs" << endl;
            double hev[RBM_NUM_HIDDEN_UNITS]; // Sample EV of h_j
            double*** data_ev = allocate_triple_matrix(user_ratings_[u].size(), RBM_HIGHEST_RATING + 1, RBM_NUM_HIDDEN_UNITS);
            double** sample_rating_ev = allocate_double_matrix(user_ratings_[u].size(), RBM_HIGHEST_RATING + 1); // Sample EV of V_ik
            //cout << "Allocated array alright" << endl;
            double temp_sum = 0; // For the Data EV
            double temp_sum_sample = 0; // For the Sample EV
            int current_movie_rating;
            // Data EV sum thing
            for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                temp_sum = 0;
                for(int r = 0; r < user_ratings_[u].size(); ++r) {
                    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                        temp_sum += ((user_ratings_[u][r].second >> k) & 1) * weights_[user_ratings_[u][r].first][j][k];  
                    }
                }
                temp_sum_sample = 0;
                for(int r = 0; r < user_ratings_[u].size(); ++r) {
                    current_movie_rating = (log(working_ratings[r].second) / log(2));
                    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                       temp_sum_sample += ((working_ratings[r].second >> k) & 1) * weights_[working_ratings[r].first][j][k];

                       if(current_movie_rating == k) { data_ev[r][k][j] = 1 / (1 + exp(-1 * (temp_sum))); }
                       else { data_ev[r][k][j] = 0; }
                    }
                }
                hev[j] = 1 / (1 + exp(-1 * (temp_sum_sample))); 
            }
            // V_ik EV from sample
            double current_sum;
            double normalize_sum;
            double per_rating_sum[RBM_HIGHEST_RATING + 1];
            for(int r = 0; r < user_ratings_[u].size(); ++r) {
                normalize_sum = 0;
                for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                    current_sum = 0;
                    for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                        current_sum += h[j] * weights_[user_ratings_[u][r].first][j][k];
                    }
                    normalize_sum += exp(current_sum);
                    per_rating_sum[k] = exp(current_sum);
                }
                for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                    per_rating_sum[k] /= normalize_sum;
                    sample_rating_ev[r][k] = per_rating_sum[k];
                }
            }

            // Calculate deltas for every weight
            bool user_rated_movie = false;
            double cur_data_ev;
            double sampled_ev;
            for(int r = 0; r < user_ratings_[u].size(); ++r) {
                // In order to avoid re-writing my code
                int i = user_ratings_[u][r].first;
                //cout << "Starting movie " << i << endl;
                current_movie_rating = (log(working_ratings[r].second) / log(2));
                for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                   //cout << "Starting hidden unit " << j << endl; 
                    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                        //cout << "(Epoch " << e << ", user " << u << "  movie " << i << ", unit " << j << " and rating " << k << ")" << 
                        //    " Starting machine with rating " << k << endl;

                        cur_data_ev = data_ev[r][k][j]; 

                        // Calculate Gibbs Sampled EV = P(v_{ik} = 1 | H) * P(h_j = 1 | V)
                        //cout << "Calculating Sample EV" << endl;
                        sampled_ev = hev[j] * sample_rating_ev[r][k]; 

                        // Use these to calculate deltas
                        delta_weights_[i][j][k] += RBM_LEARNING_RATE * (cur_data_ev - sampled_ev); 
                        //cout << "Contributed " << delta_weights_[i][j][k] << " to " << i << " " << j << " " << k << endl; 
                    }
                }
            }

            free_triple_matrix(data_ev, user_ratings_[u].size(), RBM_HIGHEST_RATING + 1, RBM_NUM_HIDDEN_UNITS);
            free_double_matrix(sample_rating_ev, user_ratings_[u].size(), RBM_HIGHEST_RATING + 1); // Sample EV of V_ik
        }
        // Change weights
        cout << "Applying weight deltas" << endl;
        for(int i = 1; i < RBM_MOVIE_COUNT; ++i) {
            for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
                for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
                    weights_[i][j][k] += (1/(double)RBM_USER_COUNT) * delta_weights_[i][j][k]; // Averaging over users
                }
            }
        }
    }

    //cout << "Saving weights to file." << endl;
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
    //cout << "Loading ratings and initializing ratings vector" << endl;
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

    // Half the memory consumption
    /*free_um_all_usernumber();
    free_um_all_movienumber();
    free_um_all_rating();
    free_um_idx_ratingset(); */

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
    int lookup[33];
    lookup[2] = 1;
    lookup[4] = 2;
    lookup[8] = 3;
    lookup[16] = 4;
    lookup[32] = 5; 

    assert(initialized_);
    double total_product[6];
    double normalize_sum;
    double temp_sum;
    int current_rating;
    int current_encoded_rating;
    int current_size = user_ratings_[user].size();
    pair<int,int> current_pair;
    normalize_sum = 0;
    for(int k = 1; k <= RBM_HIGHEST_RATING; ++k) {
        total_product[k] = 1;
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            temp_sum = 0;
            for(int r = 0; r < current_size; ++r) {
                current_pair = user_ratings_[user][r];
                current_rating = lookup[current_pair.second];
                temp_sum += (weights_[current_pair.first][j][current_rating] + 
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
        //cout << "Gibbs step " << num_steps << endl;
        // Sample each hidden unit and save into temp array
        for(int j = 0; j < RBM_NUM_HIDDEN_UNITS; ++j) {
            h[j] = sample_hj_given_user_ratings(j, *rating_vector);
        }

        //cout << "Reconstructing user data" << endl;
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

        //cout << "Sampling h_j again" << endl;
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
            count = count + 1;
        }
    }
    cout << "Count was " << count << endl;
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}

void RestrictedBoltzmannMachine::write_predictions_to_file() {
    free_um_all_usernumber(); free_um_all_movienumber(); free_um_all_rating(); free_um_idx_ratingset();
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);
    assert(load_mu_qual_usernumber() == 0);
    assert(load_mu_qual_movienumber() == 0);
    assert(load_mu_qual_datenumber() == 0);
    // Save weights to a file
    ofstream outfile_probe;
    ofstream outfile_qual;
    outfile_probe.open(RBM_PREDICTIONS_FILE1.c_str()); // Defined in the header file
    outfile_qual.open(RBM_PREDICTIONS_FILE2.c_str()); // Defined in the header file
    double prediction = 0;
    for(int i=0; i < RBM_TOTAL_NUM_POINTS; ++i) {
        if(i % 1000000 == 0) { cout << "Predicting on all point #" << i << endl; }
        if(get_mu_idx_ratingset(i) == 4) {
            prediction = predict(get_mu_all_usernumber(i), get_mu_all_movienumber(i), get_mu_all_datenumber(i));
            if(isnan(prediction)) { prediction = 3; cout << "NaN prediction found" << endl; } 
            outfile_probe << prediction << endl;
        }
    }
    for(int i =0; i < RBM_QUAL_POINTS; ++i) {
        if(i % 1000000 == 0) { cout << "Predicting on qual point #" << i << endl; }
        prediction = predict(get_mu_qual_usernumber(i), get_mu_qual_movienumber(i), get_mu_qual_movienumber(i));
        if(isnan(prediction)) { prediction = 3; cout << "NaN prediction found" << endl; }
        outfile_qual << prediction << endl;
    }
}
