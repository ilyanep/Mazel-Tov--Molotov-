#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
#include"movie_knn_pearson.h"
#include "../binary_files/binary_files.h"

//This doesn't do much that's useful. It just tests very basic "this doesn't crash" functionality. 
int main (int argc, char **argv)
{
    Movie_Knn_Pearson m;
    m.remember(3);
    double rating_sum = 0.0;
    double error;
    int size_of_partition = 0;
    int i=1;
    printf("now calculated rmse on partition 4\n");
    for (int i=0; i < 102416306; i++)
    {
        if (get_mu_idx_ratingset(i) == char( 4))
        {
            error = m.predict(get_mu_all_usernumber(i),get_mu_all_movienumber(i),get_mu_all_datenumber(i),0)-float(get_mu_all_rating(i));
            if (error < 0.0)
            {
                error = -error;
            }
            rating_sum += pow(error,2.0);
            size_of_partition++;
        }
    }
    printf("rmse on partition 4 is %f\n",sqrt(rating_sum/float(size_of_partition)));
    return 0;
}
