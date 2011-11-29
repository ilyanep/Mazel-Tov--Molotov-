#ifndef FilePredictor_file_predictor_H
#define FilePredictor_file_predictor_H

// Read predictions from file and defense them
#include "../learning_method.h"
#include <fstream>
#include <string> 

using namespace std;

typedef long long int64; // Cause I'm lazy and bad at computar

class FilePredictor : public IPredictor {
    private:
        ifstream* ratings_file1_;
        ifstream* ratings_file2_;
        int num_made_;

    public:
        FilePredictor(string ratings_file_location_probe, string ratings_file_location_qual);
        void learn(int partition);
        void remember(int partition);
        double predict(int user, int movie, int date, int index);
        void free_mem(); 

};

#endif
