#ifndef MOVIE_KNN_PEARSON_CLASS_H
#define MOVIE_KNN_PEARSON_CLASS_H

#include "movie_knn.h"



// A Movie_Knn predictor class with the Pearson Correlation Coefficient in its Rhos. 
class Movie_Knn_Pearson: public Movie_Knn
{
protected:
    static double *rhos[5]; // This shall be the set of all calculated rho values, 
                            // which will be stored on the heap, since it's huge.
                            // Note that there is a set of rhos for each trainable partition.
    
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
    int force_generate_rhos(int partition);
    
    /*
     * If there are no rhos stored yet for this partition (in memory), this will load and/or generate those rhos. 
     *
     * partition : the partition to learn these rhos with (1-4)
     *
     * return : 0 on success, -1 otherwise.
     */
    int generate_rhos(int partition);
    
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
    double rho(int movie_i, int movie_j, int partition);
public:
    //  an unnecessary but usefull function if something is changed and you 
    //  want to make sure the rho values stored are still correct:
    //virtual void probabalistic_check(int partition);
    
    /*
     * Movie_Knn_Pearson Constructor. Takes no arguments, just calls initiate(), to initiate 
     * the defaults and whatnot. 
     */
    Movie_Knn_Pearson();
    
    /*
     * sets the learned partition (the partition this object is expected to have "learned.")
     * 
     * clears any previously generated rhos (Pearson movie/movie correlation values) for 
     * this partition from memory, and loads the saved values in (or creates them if they don't exist). 
     *
     * partition: the partition to learn
     */
    virtual void learn(int partition);
    
    /*
     * sets the learned partition (the partition this object is expected to have "learned.")
     *
     * loads the rhos (Pearson movie/movie correlation values) for this partition into memory. 
     * (and generates and saves them if they don't yet exist)
     *
     */
    virtual void remember(int partition);
};


#endif
