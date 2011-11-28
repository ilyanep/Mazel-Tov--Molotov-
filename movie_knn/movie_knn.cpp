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

void Movie_Knn::free_mem(){};

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
 * pairs:       the double[] to which to write the ratings of these movies
 * movie_i:     one movie involved
 * movie_j:     the other movie involved
 * partition:   an integer 1-5, we're only talking this partition or less
 *
 * return:      The number of users that have rated both movie_i and movie_j in this partition or less
 */
int Movie_Knn::rating_pairs(double *pairs, int movie_i, int movie_j, int partition)
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
                pairs[2*l] = double(get_mu_all_rating(i));
                pairs[2*l + 1] = double(get_mu_all_rating(j));
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
