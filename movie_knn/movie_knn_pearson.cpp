#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <assert.h>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
using namespace std;
#include "../binary_files/binary_files.h"
#include "movie_knn_pearson.h"



//Class Movie_Knn_Pearson
//  protected:

double *Movie_Knn_Pearson::rhos[5] = {NULL,NULL,NULL,NULL,NULL};// This shall be the set of all calculated rho values, 
                                                                // which will be stored on the heap, since it's huge.
                                                                // Note that there is a set of rhos for each trainable partition. 


/*
 * all inheritors of Movie_Knn must define a movie-movie correlation function, rho,
 * which must be trained on a partition. For the simplest version, let all movies be
 * equally correlated, rho = 1
 *
 * This is where most KNN algorithms differ. This is the big one.
 *
 * This class uses the Pearson Coefficient for rho. It is calculated and stored in
 * force_generate_rhos, and retrieved (from the rhos array) here. 
 *
 * movie_i:     one of the movies we're correlating
 * movie_j:     the other movie we're correlating
 * partiton:    trained on this partition or less
 *
 * return:      The Pearson Coefficient representing how correlated these movies are
 */
double Movie_Knn_Pearson::rho(int movie_i, int movie_j, int partition)
{
    // a movie is always correlated at 1.0 with itself. 
    if (movie_i == movie_j)
    {
        return (1.0);
    }
    
    // make sure to actually generate the rhos.
    if (generate_rhos(partition)!=0)
    {
        cerr << "error when generating rhos\n";
        return (0.0);
    }
    
    double answer;
    
    //Note that since rho(i,j,p) == rho(j,i,p), we can save space in the stored array like this:
    if (movie_i > movie_j)
    {
        answer =  rhos[partition][(((movie_i*movie_i)-(3*movie_i))/2) + movie_j];
    } else {
        answer =  rhos[partition][(((movie_j*movie_j)-(3*movie_j))/2) + movie_i];
    }
    
    if (isnan(answer))
    {
        // rho comes back NaN when the set of users who have rated both movies 
        // has 0 variance amongst the rankings for one or both movies. In these
        // cases, Pearson says nothing about their correlation. 
        //
        // For the moment, I allow these cases to be stored as NaN, and not 0, 
        // because it may be significant at some future date that they do not
        // display 0 calculated correlation, but rather no correlation can be 
        // calculated.
        return 0.0;
    }
    return answer;
}


string Movie_Knn_Pearson::filename(int partition)
{
    stringstream ss;
    ss << partition;
    return string("Movie_KNN_rhos_pearson_partition_")+ ss.str()+string(".bin");
    
}


/*
 * OK boys and girls, this is the most computationally intense part of Movie_Knn_Pearson.
 * we're going to calculate correlation coefficients for all of the pairs of movies. 
 *
 * In order to store all pairs (note that it doesn't matter which order you pair the movies, 
 * e.g. rho(i,j,p) == rho(j,i,p)), we are going to initiate (on the heap) the rhos[partition]
 * array, and store all the rhos in it (for this partition), with each rho(i,j,p), where i>j, 
 * being stored at rhos[partition][(i^2 - 3i)/2 + j], and we don't store rho(i,i,p), since that's
 * just 1. 
 *
 * partition : the partition to learn these rhos with (1-4)
 *
 * return : 0 on success, -1 otherwise.
 */
int Movie_Knn_Pearson::force_generate_rhos(int partition)
{
    // If you're asking to force generate rhos for a partition that cannot be trained, fail.
    if (partition < 1 || partition > 4)
    {
        cerr << "cannot train rhos on partition " << partition << "\n";
        return -1;
    }
    
    // If there are already entries at this rhos[partition] array, clear them.
    if (!(rhos[partition] == NULL))
    {
        free(rhos[partition]);
    }
    
    // Generate the name of the file in which these rhos ought to be stored. 
    
    // If this data file exists and is complete (all the rhos have been generated), load them up.
    if (data_file_exists(filename(partition)) && file_size(filename(partition)) == sizeof(double) * (((num_movies - 1)*num_movies)/2))
    {
        rhos[partition] = (double *) file_bytes(filename(partition));
        
    // If the data file does not exist or is incomplete, let's get ready to generate!
    } else {
        // If the data file exists, and so it's not done yet, load it up, and remember how big it is
        int start_index = 0;
        double *old_rhos = NULL;
        if (data_file_exists(filename(partition)))
        {
            start_index = (file_size(filename(partition)))/sizeof(double);
            old_rhos = (double *) file_bytes(filename(partition));
        }
        
        // Allocate some memory for the new, complete array of rhos
        rhos[partition] = (double *) malloc(sizeof(double) * (((num_movies - 1)*num_movies)/2));
        if (rhos[partition] == NULL)
        {
            cerr << "could not allocate memory for the rhos\n";
            return -1;
        }
        
        // For each pair of movies, fill in that the rho for that pair:
        // rhos[partition][(i^2 - 3i)/2 + j] = covariance(x_i,x_j),
        // where x_i and x_j are the sets of ratings of movies i and j, where 
        // for all l, x_i[l] and x_j[l] are by the same user.
        // see BigChaos for details, or wikipedia's Pearson article. 
        int ell;
        double x_ij[num_users * 2];
        double average_x_i;
        double average_x_j;
        int l;
        double numerator;
        double denominator_i;
        double denominator_j;
        int index = 0;
        int progress_counter = 0; // the progress_counter and _index stuff is just to print out progress.
        int progress_index = 0;
        double sum_x_i;
        double sum_x_j;
        double x_ij_i;
        double x_ij_j;
        for (int i=1; i <=num_movies; i++)
        {
            for (int j=1; j<i; j++)
            {
                if (index < start_index) // if this is still in the set of pre-stored rhos, use that.
                {
                    rhos[partition][index] = old_rhos[index];
                } else {
                    if (index == start_index) // if we've just finished the set of pre-stored rhos, clear that.
                    {
                        free(old_rhos);
                        old_rhos = NULL;
                    }
                    
                    // Otherwise, let the Pearson Correlation calculation begin!
                    ell = rating_pairs(x_ij, i, j, partition);
                    if (ell > 1)
                    {
                        sum_x_i = 0;
                        sum_x_j = 0;
                        average_x_i = 0.0;
                        average_x_j = 0.0;
                        for (l=0; l < ell; l++)
                        {
                            sum_x_i += x_ij[2*l  ];
                            sum_x_j += x_ij[2*l+1];
                        }
                        average_x_i = (sum_x_i) /  ell;
                        average_x_j = (sum_x_j) /  ell;
                        numerator = 0.0;
                        denominator_i=0.0;
                        denominator_j=0.0;
                        for (l=0; l< ell; l++)
                        {
                            x_ij_i = x_ij[2*l];
                            x_ij_j = x_ij[2*l + 1];
                            numerator += (x_ij_i- average_x_i)*(x_ij_j - average_x_j);
                            denominator_i += pow((x_ij_i- average_x_i),2);
                            denominator_j += pow((x_ij_j- average_x_j),2);
                        }
                        rhos[partition][index] = (numerator / sqrt(denominator_i * denominator_j));
                    } else {
                        rhos[partition][index] = 0.0;
                    }
                }
                
                // This stuff is just to print out progress, and store your progress every 1% of the way. 
                if (progress_index == 1579)
                {
                    printf("now %d / 100000 complete calculating the rhos.\n", progress_counter);
                    progress_index = 0;
                    progress_counter ++;
                    if ((progress_counter % 1000 == 0) && (index > start_index))
                    {
                        array_to_file((char *)rhos[partition], sizeof(double) * index, filename(partition)); // every 1%, write to file
                    }
                }
                index ++;
                progress_index++;
            }
        }
        free(old_rhos); // make sure to clear all the old rhos from the pre-stored file, if that exists. 
        
        // store the calculated rhos values in their file, return the success of that operation. 
        return array_to_file((char *)rhos[partition], sizeof(double) * (((num_movies - 1)*num_movies)/2), filename(partition));
    }
    return 0;
}

/*
 //This function exists only to check the existing rhos to make sure they're
 //what "would have been calculated" by the current code. 
 //It's useful only if the code changes or something like that. 
void Movie_Knn_Pearson::probabalistic_check(int partition)
{
    
    int ell;
    char x_ij[num_users * 2];
    double average_x_i;
    double average_x_j;
    int l;
    double numerator;
    double denominator_i;
    double denominator_j;
    int index = 0;
    int progress_counter = 0;
    int progress_index = 0;
    int sum_x_i;
    int sum_x_j;
    double x_ij_i;
    double x_ij_j;
    for (int i=1; i <=num_movies; i++)
    {
        for (int j=1; j<i; j++)
        {
            if (index % 100 == 1)
            {
                ell = rating_pairs(x_ij, i, j, partition);
                if (ell > 1)
                {
                    sum_x_i = 0;
                    sum_x_j = 0;
                    average_x_i = 0.0;
                    average_x_j = 0.0;
                    for (l=0; l < ell; l++)
                    {
                        sum_x_i += x_ij[2*l  ];
                        sum_x_j += x_ij[2*l+1];
                    }
                    average_x_i = double(sum_x_i) /  ell;
                    average_x_j = double(sum_x_j) /  ell;
                    numerator = 0.0;
                    denominator_i=0.0;
                    denominator_j=0.0;
                    for (l=0; l< ell; l++)
                    {
                        x_ij_i = double(x_ij[2*l]);
                        x_ij_j = double(x_ij[2*l + 1]);
                        numerator += (x_ij_i- average_x_i)*(x_ij_j - average_x_j);
                        denominator_i += pow((x_ij_i- average_x_i),2);
                        denominator_j += pow((x_ij_j- average_x_j),2);
                    }
                    if ((((numerator / sqrt(denominator_i * denominator_j)) - rho(i,j,partition)) > 0.0001) || (((numerator / sqrt(denominator_i * denominator_j)) - rho(i,j,partition)) < -0.0001) )
                    {
                        printf("For movies %d and %d I calculated %f, but the record has %f\n", i, j, (numerator / sqrt(denominator_i * denominator_j)),rho(i,j,partition));
                    }
                } else {
                    if (((rho(i,j,partition)) > 0.001) || ((rho(i,j,partition)) < -0.001))
                    {
                        
                        printf("For movies %d and %d I calculated %f, but the record has %f\n", i, j, 0.0,rho(i,j,partition));
                    }
                }
            }
            index ++;
        }
    }
}*/

/*
 * If there are no rhos stored yet for this partition (in memory), this will load and/or generate those rhos. 
 *
 * partition : the partition to learn these rhos with (1-4)
 *
 * return : 0 on success, -1 otherwise.
 */
int Movie_Knn_Pearson::generate_rhos(int partition)
{
    if (rhos[partition] == NULL)
    {
        return force_generate_rhos(partition);
    }
    return 0;
}



//  public:

/*
 * Movie_Knn_Pearson Constructor. Takes no arguments, just calls initiate(), to initiate 
 * the defaults and whatnot. 
 */
Movie_Knn_Pearson::Movie_Knn_Pearson()
{
    initiate();
}

/*
 * sets the learned partition (the partition this object is expected to have "learned.")
 * 
 * clears any previously generated rhos (Pearson movie/movie correlation values) for 
 * this partition from memory, and loads the saved values in (or creates them if they don't exist). 
 *
 * partition: the partition to learn
 */
void  Movie_Knn_Pearson::learn(int partition)
{
    learned_partition = partition;
    if (force_generate_rhos(partition)!=0)
    {
        cerr << "error when generating rhos\n";
    }
}

/*
 * sets the learned partition (the partition this object is expected to have "learned.")
 *
 * loads the rhos (Pearson movie/movie correlation values) for this partition into memory. 
 * (and generates and saves them if they don't yet exist)
 *
 */
void  Movie_Knn_Pearson::remember(int partition)
{
    learned_partition = partition;
    if (generate_rhos(partition)!=0)
    {
        cerr << "error when generating rhos\n";
    }
}


