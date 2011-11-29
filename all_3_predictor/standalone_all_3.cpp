#include "all_3_predictor.h"
#include "../write_data/write_results.h"

int main() {
    All3Predictor predictor;
    predictor.learn(5);
    vector<double> res;
    for(int i=0; i < 2749898; ++i) {
        res.push_back(predictor.predict(i,i,i,0));
    }
    output_results(res);
}
