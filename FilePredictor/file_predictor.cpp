#include "file_predictor.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

using namespace std;

FilePredictor::FilePredictor(string ratings_file_location) {
    cout << "Creating FilePredictor on file " << ratings_file_location << endl;
    ratings_file_ = new ifstream(ratings_file_location.c_str());
    assert(ratings_file_->is_open()); 
    cout << "WARNING: THIS RATINGS FILE WAS PROBABLY WRITTEN IN MU ORDER AND DOES NOT CHECK THE USER,"
         << "MOVIE AND DATE INTS PASSED IN. THEREFORE, PLEASE MAKE SURE YOU ARE PREDICTING IN MU ORDER"
         << "AND ON THE RIGHT PARTITION BEFORE RUNNING THIS PREDICTOR" << endl;
}

void FilePredictor::learn(int partition) { }
void FilePredictor::remember(int partition) { }
void FilePredictor::free_mem() { }

double FilePredictor::predict(int user, int movie, int date) {
    assert(ratings_file_->good()); 
    string line;
    getline(*(ratings_file_), line);
    return strtod(line.c_str(), NULL); 
}
