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
#include "movie_knn.h"

// Class Movie_Knn
//   protected:
int Movie_Knn::num_movies = 17770;                // The number of movies involved
int Movie_Knn::num_users = 458293;                // The number of users involved
int Movie_Knn::num_ratings = 102416306;           // The total number of ratings ever

int Movie_Knn::movie_start_indexes[17772] = {-1}; // This will become the array: [movie_id] -> start index in mu_all
                                                  //    note that here we use movie_start_indexes[0] as an indicator. -1 indicates 
                                                  //    the array has not yet been set, while 0 indicates it has. Movie indicees 
                                                  //    start at 1, so this is not actually connected to actual movie indicees.
                                                  //    note also that in order to make edge cases easier, 
                                                  //        movie_start_indexes[1771]=num_ratings

int Movie_Knn::user_start_indexes[458295] = {-1}; // This will become the array: [ user_id] -> start index in um_all
                                                  //    note that here we use movie_start_indexes[0] as an indicator. -1 indicates 
                                                  //    the array has not yet been set, while 0 indicates it has. User indicees 
                                                  //    start at 1, so this is not actually connected to actual user indicees. 
                                                  //    note also that in order to make edge cases easier, 
                                                  //        user_start_indexes[458294]=num_ratings

/*
 * Initiate an instance of this class. 
 * Sets the learned_partition to default (1)
 */
void Movie_Knn::initiate()
{
    learned_partition   = 1;
}

/*
 * This will set the movie_start_indexes to be an array of the form:
 * [movie_id] -> start index in mu_all
 *
 * note that here we use movie_start_indexes[0] as an indicator. -1 indicates 
 * the array has not yet been set, while 0 indicates it has. Movie indicees 
 * start at 1, so this is not actually connected to actual movie indicees. 
 * note also that in order to make edge cases easier, 
 *     movie_start_indexes[1771]=num_ratings
 */
void Movie_Knn::initialize_movie_start_indexes()
{
    if (movie_start_indexes[0] == -1)
    {
        movie_start_indexes[0] = 0;
        int i=0;
        int j=0;
        while ( j < num_ratings)
        {
            if ( i < get_mu_all_movienumber(j))
            {
                i++;
                movie_start_indexes[i] = j;
            } else {
                j++;
            }
        }
        movie_start_indexes[num_movies + 1] = num_ratings; // easier for edge cases
    }
}

/*
 * This will set the user_start_indexes to be an array of the form:
 * [user_id] -> start index in um_all
 *
 * note that here we use movie_start_indexes[0] as an indicator. -1 indicates 
 * the array has not yet been set, while 0 indicates it has. User indicees 
 * start at 1, so this is not actually connected to actual user indicees. 
 * note also that in order to make edge cases easier, 
 *     user_start_indexes[458294]=num_ratings
 */
void Movie_Knn::initialize_user_start_indexes()
{
    if (user_start_indexes[0] == -1)
    {
        user_start_indexes[0] = 0;
        int i=0;
        int j=0;
        while ( j < num_ratings)
        {
            if ( i < get_um_all_usernumber(j))
            {
                i++;
                user_start_indexes[i] = j;
            } else {
                j++;
            }
        }
        user_start_indexes[num_users + 1] = num_ratings; // easier for edge cases
    }
}

/*
 * Determines the number of users in this partition or less that have rated this movie
 *
 * movie:       the movie involved
 * partition:   an integer 1-5, we're only talking this partition or less
 *
 * return:      The number of users that have rated this movie in this partition or less
 */
int Movie_Knn::number_users_rated(int movie, int partition)
{
    int i; //useful control variable
    initialize_movie_start_indexes();
    int answer = 0;
    for (i = movie_start_indexes[movie]; i<movie_start_indexes[movie+1];i++)
    {
        if (partition >= get_mu_idx_ratingset(i))
        {
            answer++;
        }
    }
    return answer;    
}


/*
 * Determines the number of users in this partition or less that have rated both of these movies
 *
 * movie_i:     one movie involved
 * movie_j:     the other movie involved
 * partition:   an integer 1-5, we're only talking this partition or less
 *
 * return:      The number of users that have rated both movie_i and movie_j in this partition or less
 */
int Movie_Knn::number_users_rated(int movie_i, int movie_j, int partition)
{
    initialize_movie_start_indexes();
    int answer = 0;
    int i = movie_start_indexes[movie_i];
    int j = movie_start_indexes[movie_j];
    while(i<movie_start_indexes[movie_i+1] && j<movie_start_indexes[movie_j+1] )
    {
        if (get_mu_all_usernumber(i) < get_mu_all_usernumber(j))
        {
            i++;
        } else {
            if (get_mu_all_usernumber(i) == get_mu_all_usernumber(j) && 
                partition >= get_mu_idx_ratingset(i) && 
                partition >= get_mu_idx_ratingset(j))
            {
                answer++;
            }
            j++;
        }
    }
    return answer;    
}


/*
 * Determines the number of users in this partition or less that have rated both of these movies,
 * and writes into "pairs" the ratings from those users. Specifically, for index l:
 *    0 < l < (number of users who rated both movie_i and movie_j in partition or less)
 *    pairs[2*l]   = a rating of movie_i
 *    pairs[2*l+1] = a rating of movie_j by the same user
 * 
 * pairs:       the char[] to which to write the ratings of these movies
 * movie_i:     one movie involved
 * movie_j:     the other movie involved
 * partition:   an integer 1-5, we're only talking this partition or less
 *
 * return:      The number of users that have rated both movie_i and movie_j in this partition or less
 */
int Movie_Knn::rating_pairs(char *pairs, int movie_i, int movie_j, int partition)
{
    initialize_movie_start_indexes();
    int l = 0;
    int i = movie_start_indexes[movie_i];
    int j = movie_start_indexes[movie_j];
    while(i<movie_start_indexes[movie_i+1] && j<movie_start_indexes[movie_j+1] )
    {
        if (get_mu_all_usernumber(i) < get_mu_all_usernumber(j))
        {
            i++;
        } else {
            if (get_mu_all_usernumber(i) == get_mu_all_usernumber(j) && 
                partition >= get_mu_idx_ratingset(i) && 
                partition >= get_mu_idx_ratingset(j))
            {
                pairs[2*l] = get_mu_all_rating(i);
                pairs[2*l + 1] = get_mu_all_rating(j);
                l++;
            }
            j++;
        }
    }
    return l;
}

/*
 * all inheritors of Movie_Knn must define a movie-movie correlation function, rho,
 * which must be trained on a partition. For the simplest version, let all movies be
 * equally correlated, rho = 1
 *
 * This is where most KNN algorithms differ. This is the big one. 
 *
 * movie_i:     one of the movies we're correlating
 * movie_j:     the other movie we're correlating
 * partiton:    trained on this partition or less
 *
 * return:      some coefficient representing how correlated these movies are, in this
 *                  simplest of cases, it's just 1.
 */
double Movie_Knn::rho(int movie_i, int movie_j, int partition)
{
    return 1.0F;
}

/*
 * When we're actually taking our correlation-weighted averages, we may want to take
 * additional factors, such as how many users actually participated in a certain
 * correlation calculation into account. Often calculting c will involve metaparameters. 
 * In the simplest case, however, this modified correlation, c, is just the same as rho. 
 *
 * movie_i:     one of the movies we're correlating
 * movie_j:     the other movie we're correlating
 * partiton:    trained on this partition or less
 *
 * return:      some coefficient representing how correlated these movies are, in this
 *                  simplest of cases, it's just 1.
 */
double Movie_Knn::c(int movie_i, int movie_j, int partition)
{
    return double(rho(movie_i, movie_j, partition));
}






//  public:

/*
 * Movie_Knn Constructor. Takes no arguments, just calls initiate(), to initiate 
 * the defaults and whatnot. 
 */
Movie_Knn::Movie_Knn()
{
    initiate();
}

/*
 * This method should use the data to learn whatever technique this class
 * is using and then write whatever residuals it calculates to a file
 * so that they can be re-used. Partition will be an integer from 1-5.
 * Learning should be done on partitions 1 through partition, inclusive.
 * Make sure that whatever file is written has some sort of metadata
 * that says what partition was learned on.
 *
 * In this case, we're just going to set the stored learned_partition, which
 * other methods may use (it's meant to represent which partion this object
 * is expected to have "learned"). 
 */
void  Movie_Knn::learn(int partition)
{
    learned_partition = partition;
}

/* This method should read any residuals that are already written onto
 * disk to avoid re-learning. If the residuals cannot be found, it
 * will call Learn().
 *
 * In this case, we're just going to set the stored learned_partition, which
 * other methods may use (it's meant to represent which partion this object
 * is expected to have "learned"). 
 */
void  Movie_Knn::remember(int partition)
{
    learned_partition = partition;
}

/*
 * This method should return a predicted rating based on existing
 * residuals. If the internal state indicates that the learning has not
 * occurred yet, Remember() should be called.
 *
 * In this case, we're just going to do the simplest version of KNN predictions,
 * and return a prediction that is the weighted average of the ratings this 
 * user has made in the learned partition, with each rating weighted by the correlation
 * between its movie and this movie, as determined by the correlation function, c. 
 *
 * Note that for negative correlaitons, we reverse the rating (6-rating) when taking this
 * average, so that we can calculate with all positive weights. 
 */
double Movie_Knn::predict(int user, int movie, int time)
{
    initialize_user_start_indexes();
    double numerator = 0.0;
    double denominator = 0.0;
    int j;
    char r_uj;
    double c_ij;
    for (int i = user_start_indexes[user]; i < user_start_indexes[user+1]; i++)
    {
        if (learned_partition >= get_um_idx_ratingset(i))
        {
            j = int(get_um_all_movienumber(i));
            r_uj = get_um_all_rating(i);
            c_ij = c(movie, j, learned_partition);
            
            // testing out the idea of reversing ratings for negative correlations . . . 
            // well, it improves pure Pearson Predictions on set 2, so that's good. I'll roll with it. 
            //
            // More intuitively, we reverse ratings to represent reverse correlations. This is legit, guys . . .
            if (c_ij < 0.0)
            {
                c_ij = -c_ij;
                r_uj = 6-r_uj;
            }
            numerator += (c_ij * r_uj);
            denominator += c_ij;
        }
    }
    if (denominator < 0.00000001 && denominator > -0.00000001) // if there is no weighting to go on, return 3
                                                               // This should hopefully not happen, but who knows?
    {
        return 3.0;
    }
    return   numerator / denominator;
}















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
    stringstream ss;
    ss << partition;
    string filename = string("Movie_KNN_rhos_pearson_partition_")+ ss.str()+string(".bin");
    
    // If this data file exists and is complete (all the rhos have been generated), load them up.
    if (data_file_exists(filename) && file_size(filename) == sizeof(double) * (((num_movies - 1)*num_movies)/2))
    {
        rhos[partition] = (double *) file_bytes(filename);
        
    // If the data file does not exist or is incomplete, let's get ready to generate!
    } else {
        // If the data file exists, and so it's not done yet, load it up, and remember how big it is
        int start_index = 0;
        double *old_rhos = NULL;
        if (data_file_exists(filename))
        {
            start_index = (file_size(filename))/sizeof(double);
            old_rhos = (double *) file_bytes(filename);
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
        char x_ij[num_users * 2];
        double average_x_i;
        double average_x_j;
        int l;
        double numerator;
        double denominator_i;
        double denominator_j;
        int index = 0;
        int progress_counter = 0; // the progress_counter and _index stuff is just to print out progress.
        int progress_index = 0;
        int sum_x_i;
        int sum_x_j;
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
                        array_to_file((char *)rhos[partition], sizeof(double) * index, filename); // every 1%, write to file
                    }
                }
                index ++;
                progress_index++;
            }
        }
        free(old_rhos); // make sure to clear all the old rhos from the pre-stored file, if that exists. 
        
        // store the calculated rhos values in their file, return the success of that operation. 
        return array_to_file((char *)rhos[partition], sizeof(double) * (((num_movies - 1)*num_movies)/2), filename);
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


