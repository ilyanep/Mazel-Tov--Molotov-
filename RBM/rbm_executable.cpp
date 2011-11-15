#include "rbm.h"
#include <ctime>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if(argc != 2 && argc != 1) {
        cout << "rbm_executable [--rmse_probe]" << endl;
    }
    time_t init;
    time_t end;
    cout << "Creating predictor and learning" << endl;
    time(&init);
    RestrictedBoltzmannMachine predictor;
    predictor.learn(3);
    time(&end);
    cout << "Learning took " << difftime(init, end) << " seconds" << endl;

    if(argc == 2 && argv[1] == "--rmse_probe") {
        cout << "Starting RMSE on probe calculation." << endl;
        cout << predictor.rmse_probe() << endl;
    }
    else if(argc > 1) {
        cout << "Unknown argument " << argv[1] << endl;
        cout << "You dolt." << endl;
    }
    return 0;
}
