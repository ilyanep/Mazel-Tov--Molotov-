#include <string.h>
#include "svd_svm_nov6.h"
#include <math.h>
#include <assert.h>
using namespace std;
#include "../binary_files/binary_files.h"
#include <gsl/gsl_matrix.h>
#include "../write_data/write_results.h"
int main(int argc, char* argv[]) {
    clock_t init, final;
    printf("Loading SVD data set...\n");
    SVD_SVM_Nov6 svd_predictor;
    printf("Loading SVD parameters...\n");
    svd_predictor.remember(3);
    printf("Outputting SVM parameters...\n");
    svd_predictor.output_svm_data_files(4);
}

