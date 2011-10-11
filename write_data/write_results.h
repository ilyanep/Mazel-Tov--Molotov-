#include<vector>
using namespace std;

#define RESULT_FILE "result.dta"

// Library for writing output to file in the same format as example.dta. Rating should be a
// vector of predicted ratings sorted in the same order as either mu or um (keep track of this).
void output_results(vector<double> rating); 
