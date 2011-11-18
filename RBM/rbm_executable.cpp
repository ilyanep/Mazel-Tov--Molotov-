#include "rbm.h"
#include <ctime>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if(argc != 1) {
        cout << "rbm_executable" << endl;
        return 0;
    }
    time_t init;
    time_t end;
    cout << "Creating predictor and learning" << endl;
    time(&init);
    RestrictedBoltzmannMachine predictor;
    predictor.learn(3);
    time(&end);
    cout << "Learning took " << difftime(init, end) << " seconds" << endl;

    cout << "Starting RMSE on probe calculation." << endl;
    cout << predictor.rmse_probe() << endl;
    return 0;
}
