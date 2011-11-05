#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
#include"movie_knn.h"
#include "../binary_files/binary_files.h"

//This doesn't do much that's useful. It just tests very basic "this doesn't crash" functionality. 
int main (int argc, char **argv)
{
    Movie_Knn_Pearson m;
    Movie_Knn_Pearson n;
    m.learn(1);
    n.learn(1);
    double rating_sum = 0.0;
    double error;
    int size_of_partition_two = 0;
    int i=1;
    printf("m: %f, n: %f, rating = %d\n", m.predict(get_mu_all_usernumber(i),get_mu_all_movienumber(i),get_mu_all_datenumber(i)), n.predict(get_mu_all_usernumber(i),get_mu_all_movienumber(i),get_mu_all_datenumber(i)),get_mu_all_rating(i));
    
    // This is just the code to calculate the RMSE on partition 2. 
    /*for (int i=0; i < 102416306; i++)
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
    printf("rmse on partition 2 is %f\n",sqrt(rating_sum/float(size_of_partition_two)));*/
    return 0;
}
