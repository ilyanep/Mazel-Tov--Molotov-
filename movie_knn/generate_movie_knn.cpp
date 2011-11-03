#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
#include"movie_knn.h"
#include "../binary_files/binary_files.h"

int main (int argc, char **argv)
{
    Movie_Knn_Pearson m;
    m.learn(1);
    double rating_sum = 0.0;
    double error;
    int size_of_partition_two = 0;
    for (int i=0; i < 102416306; i++)
    {
        if (get_mu_idx_ratingset(i) == char( 2))
        {
            error = m.predict(get_mu_all_usernumber(i),get_mu_all_movienumber(i),get_mu_all_datenumber(i))-float(get_mu_all_rating(i));
            if (error < 0.0)
            {
                error = -error;
            }
            rating_sum += pow(error,2.0);
            size_of_partition_two++;
        }
    }
    printf("rmse on partition 2 is %f\n",sqrt(rating_sum/float(size_of_partition_two)));
}
