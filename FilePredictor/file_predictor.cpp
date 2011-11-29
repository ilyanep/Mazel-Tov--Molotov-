#include "file_predictor.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

using namespace std;

FilePredictor::FilePredictor(string ratings_file_location_probe, string ratings_file_location_qual) {
    cout << "Creating FilePredictor on files " << ratings_file_location_probe << " and " << ratings_file_location_qual << endl;
    ratings_file1_ = new ifstream(ratings_file_location_probe.c_str());
    ratings_file2_ = new ifstream(ratings_file_location_qual.c_str());
    assert(ratings_file1_->is_open()); 
    assert(ratings_file2_->is_open());
    cout << "WARNING: THIS RATINGS FILE WAS PROBABLY WRITTEN IN MU ORDER AND DOES NOT CHECK THE USER,"
         << "MOVIE AND DATE INTS PASSED IN. THEREFORE, PLEASE MAKE SURE YOU ARE PREDICTING IN MU ORDER"
         << "AND ON THE RIGHT PARTITION BEFORE RUNNING THIS PREDICTOR" << endl;
}

void FilePredictor::learn(int partition) { }
void FilePredictor::remember(int partition) { }
void FilePredictor::free_mem() { }

double FilePredictor::predict(int user, int movie, int date, int index) {
    ifstream* current_file;
    string line;
    if(ratings_file1_->good()) {
        getline(*(ratings_file1_), line);
    }
    else if(ratings_file2_->good()) {
        getline(*(ratings_file2_), line);
    }
    else { assert(false); }
    return strtod(line.c_str(), NULL); 
}
