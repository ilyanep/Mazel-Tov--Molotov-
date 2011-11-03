#include "../learning_method.h"

class Movie_Knn: public IPredictor
{
protected:
    static int num_movies;
    static int num_users;
    static int num_ratings;
    static int *movie_start_indexes;
    static int *user_start_indexes;
    int learned_partition;
    
    void initiate();
    void initialize_movie_start_indexes();
    void initialize_user_start_indexes();
    int number_users_rated(int movie, int partition);
    int number_users_rated(int movie_i, int movie_j, int partition);
    int rating_pairs(char *pairs, int movie_i, int movie_j, int partition);
    virtual double rho(int movie_i, int movie_j, int partition);
    virtual double c(int movie_i, int movie_j, int partition);
public:
    Movie_Knn();
    // This method should use the data to learn whatever technique this class
    // is using and then write whatever residuals it calculates to a file
    // so that they can be re-used. Partition will be an integer from 1-5.
    // Learning should be done on partitions 1 through partition, inclusive.
    // Make sure that whatever file is written has some sort of metadata
    // that says what partition was learned on.
    virtual void learn(int partition);
    
    // This method should read any residuals that are already written onto
    // disk to avoid re-learning. If the residuals cannot be found, it
    // will call Learn().
    virtual void remember(int partition);
    
    // This method should return a predicted rating based on existing
    // residuals. If the internal state indicates that the learning has not
    // occurred yet, Remember() should be called.
    virtual double predict(int user, int movie, int time);
};



class Movie_Knn_Pearson: public Movie_Knn
{
protected:
    static double *rhos;
    int force_generate_rhos(int partition);
    int generate_rhos(int partition);
    double rho(int movie_i, int movie_j, int partition);
public:
    //  an undecessary but usefull function if something is changed and you 
    //  want to make sure the rho values stored are still correct:
    //virtual void probabalistic_check(int partition);
    
    Movie_Knn_Pearson();
    // This method should use the data to learn whatever technique this class
    // is using and then write whatever residuals it calculates to a file
    // so that they can be re-used. Partition will be an integer from 1-5.
    // Learning should be done on partitions 1 through partition, inclusive.
    // Make sure that whatever file is written has some sort of metadata
    // that says what partition was learned on.
    virtual void learn(int partition);
    
    // This method should read any residuals that are already written onto
    // disk to avoid re-learning. If the residuals cannot be found, it
    // will call Learn().
    virtual void remember(int partition);
};

