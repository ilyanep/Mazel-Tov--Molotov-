#include "write_results.h"
#include <vector>

int main() {
    vector<double> rat;
    rat.push_back(3.101010);
    rat.push_back(3.03010);
    rat.push_back(2);
    output_results(rat);
    return 0;
}
