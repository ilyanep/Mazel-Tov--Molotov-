#include "rbm.h"
#include <ctime>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
    if(argc != 1) {
        cout << "rbm_executable2" << endl;
        return 0;
    }
    time_t init;
    time_t end;
    cout << "Creating predictor and remembering" << endl;
    time(&init);
    RestrictedBoltzmannMachine predictor;
    predictor.remember(3);
    time(&end);
    cout << "Remembering took " << difftime(init, end) << " seconds" << endl;

    time(&init);
    // cout << "Starting RMSE on probe calculation." << endl;
    // cout << predictor.rmse_probe() << endl;
    cout << "Starting write to file" << endl;
    predictor.write_predictions_to_file();
    time(&end);
    cout << "RMSE took " << difftime(init, end) << " seconds" << endl;
    return 0;
}
