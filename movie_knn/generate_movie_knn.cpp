#include <iostream>
#include <fstream>
using namespace std;
#include"movie_knn.h"

int main (int argc, char **argv)
{
    Movie_Knn_Pearson m;
    int i = 200;
    int j = 10877;
    printf("The correlation between movies %d and %d is %f.\n",i,j,m.rho(i,j,1));
}
