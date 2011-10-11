#include "write_results.h"
#include <string>
#include <fstream>
#include <iostream>

void output_results(vector<double> rating) {
    ofstream outfile;
    outfile.open(RESULT_FILE); // Defined in this file's header file.
    for(int i=0; i < rating.size(); ++i) {
        outfile << rating[i] << "\n"; 
    }
    outfile.close();
    return;
}
