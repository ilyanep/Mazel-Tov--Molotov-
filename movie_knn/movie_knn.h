#ifndef MOVIE_KNN_MOVIE_KNN_H
#define MOVIE_KNN_MOVIE_KNN_H
#include "../learning_method.h"

class Movie_Knn: public IPredictor
{
protected:
    static int num_movies;                  // The number of movies involved
    static int num_users;                   // The number of users involved
    static int num_ratings;                 // The total number of ratings ever
    
    static int movie_start_indexes[17772];  // This will become the array: [movie_id] -> start index in mu_all
                                            //    note that here we use movie_start_indexes[0] as an indicator. -1 indicates 
                                            //    the array has not yet been set, while 0 indicates it has. Movie indicees 
                                            //    start at 1, so this is not actually connected to actual movie indicees.
                                            //    note also that in order to make edge cases easier, 
                                            //        movie_start_indexes[1771]=num_ratings
    
    static int user_start_indexes[458295];  // This will become the array: [ user_id] -> start index in um_all
                                            //    note that here we use movie_start_indexes[0] as an indicator. -1 indicates 
                                            //    the array has not yet been set, while 0 indicates it has. User indicees 
                                            //    start at 1, so this is not actually connected to actual user indicees. 
                                            //    note also that in order to make edge cases easier, 
                                            //        user_start_indexes[458294]=num_ratings

    int learned_partition;                  // This is the partition this object is meant to have "learned" from
    
    /*
     * Initiate an instance of this class. 
     * Sets the learned_partition to default (1)
     */
    virtual void initiate();
    
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
    virtual void initialize_movie_start_indexes();
    
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
    virtual void initialize_user_start_indexes();
    
    /*
     * Determines the number of users in this partition or less that have rated this movie
     *
     * movie:       the movie involved
     * partition:   an integer 1-5, we're only talking this partition or less
     *
     * return:      The number of users that have rated this movie in this partition or less
     */
    virtual int number_users_rated(int movie, int partition);
    
    /*
     * Determines the number of users in this partition or less that have rated both of these movies
     *
     * movie_i:     one movie involved
     * movie_j:     the other movie involved
     * partition:   an integer 1-5, we're only talking this partition or less
     *
     * return:      The number of users that have rated both movie_i and movie_j in this partition or less
     */
    virtual int number_users_rated(int movie_i, int movie_j, int partition);
    
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
    virtual int rating_pairs(double *pairs, int movie_i, int movie_j, int partition);
    
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
    virtual double rho(int movie_i, int movie_j, int partition);
    
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
    virtual double c(int movie_i, int movie_j, int partition);
public:
    
    /*
     * Movie_Knn Constructor. Takes no arguments, just calls initiate(), to initiate 
     * the defaults and whatnot. 
     */
    Movie_Knn();
    
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
    virtual void learn(int partition);
    
    /* This method should read any residuals that are already written onto
     * disk to avoid re-learning. If the residuals cannot be found, it
     * will call Learn().
     *
     * In this case, we're just going to set the stored learned_partition, which
     * other methods may use (it's meant to represent which partion this object
     * is expected to have "learned"). 
     */
    virtual void remember(int partition);
    
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
    virtual double predict(int user, int movie, int time, int placeholder);

    virtual void free_mem();
};

#endif
