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
int Movie_Knn::num_movies;
int Movie_Knn::num_users;
int Movie_Knn::num_ratings;
int *Movie_Knn::movie_start_indexes;
int *Movie_Knn::user_start_indexes;

void Movie_Knn::initiate()
{
    num_movies = 17770;
    num_users = 458293;
    num_ratings = 102416306;
    user_start_indexes = NULL;
    movie_start_indexes = NULL;
    learned_partition   = 1;
}

void Movie_Knn::initialize_movie_start_indexes()
{
    if (movie_start_indexes == NULL)
    {
        movie_start_indexes = (int *) malloc(sizeof(int)*(num_movies + 2));
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

void Movie_Knn::initialize_user_start_indexes()
{
    int i; //useful control variable
    if (user_start_indexes == NULL)
    {
        user_start_indexes = (int *) malloc(sizeof(int)*(num_users + 2));
        i=0;
        int j=0;
        while ( j < num_ratings)
        {
            if ( i < get_um_all_usernumber(j))
            {
                i++;
                user_start_indexes[i] = j;
                //printf("user %d starts at index %d\n", i, j);
            } else {
                j++;
            }
        }
        user_start_indexes[num_users + 1] = num_ratings; // easier for edge cases
    }
}

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
                
                /*
                if (get_mu_all_usernumber(i) != get_mu_all_usernumber(j))
                {
                    cerr << "rating_pairs is just failing all over the place\n";
                }
                 */
                
                pairs[2*l] = get_mu_all_rating(i);
                pairs[2*l + 1] = get_mu_all_rating(j);
                l++;
            }
            j++;
        }
    }
    return l;
}


double Movie_Knn::rho(int movie_i, int movie_j, int partition)
{
    return 1.0F;
}

double Movie_Knn::c(int movie_i, int movie_j, int partition)
{
    return double(rho(movie_i, movie_j, partition));
}


//  public:


Movie_Knn::Movie_Knn()
{
    initiate();
}

void  Movie_Knn::learn(int partition)
{
    learned_partition = partition;
}


void  Movie_Knn::remember(int partition)
{
    learned_partition = partition;
}


double Movie_Knn::predict(int user, int movie, int time)
{
    initialize_user_start_indexes();
    double numerator = 0.0;
    double denominator = 0.0;
    int j;
    char r_uj;
    double c_ij;
    double prediction;
    for (int i = user_start_indexes[user]; i < user_start_indexes[user+1]; i++)
    {
        if (learned_partition >= get_um_idx_ratingset(i))
        {
            j = int(get_um_all_movienumber(i));
            r_uj = get_um_all_rating(i);
            c_ij = c(movie, j, learned_partition);
            
            //testing out the idea of reversing ratings for negative correlations . . . 
            if (c_ij < 0.0)
            {
                c_ij = -c_ij;
                r_uj = 6-r_uj;
            }
            //printf("user %d rated movie %d a %d.\n", user, j, r_uj);
            numerator += (c_ij * r_uj);
            denominator += c_ij;
        }
    }
    if (denominator < 0.000001 && denominator > -0.000001)
    {
        return 3.0;
    }
    prediction =  numerator / denominator;
    
    /*
    if ((prediction < 1.0 ) || (prediction > 5.0))
    {
        printf("user: %d\t movie: %d\t time: %d\t prediction: %f \t = %f \t / %f\n",
               user,
               movie, 
               time, 
               prediction,
               numerator,
               denominator);
        for (int i = user_start_indexes[user]; i < user_start_indexes[user+1]; i++)
        {
            if (learned_partition >= get_um_idx_ratingset(i))
            {
                j = int(get_um_all_movienumber(i));
                r_uj = get_um_all_rating(i);
                c_ij = c(movie, j, learned_partition);
                printf("\tc: %f\t r: %d\n", c_ij,r_uj);
            }
        }
    }
     
    if (prediction < 1.0)
    {
        return 1.0;
    }
    if (prediction > 5.0)
    {
        return 5.0;
    }*/
    return prediction;
}











//Class Movie_Knn_Pearson
//  protected:

double *Movie_Knn_Pearson::rhos;

double Movie_Knn_Pearson::rho(int movie_i, int movie_j, int partition)
{
    if (movie_i == movie_j)
    {
        return (1.0);
    }
    
    if (generate_rhos(partition)!=0)
    {
        cerr << "error when generating rhos\n";
        return (0.0);
    }
    double answer;
    if (movie_i > movie_j)
    {
        answer =  rhos[(((movie_i*movie_i)-(3*movie_i))/2) + movie_j];
    } else {
        answer =  rhos[(((movie_j*movie_j)-(3*movie_j))/2) + movie_i];
    }
    
    if (isnan(answer))
    {
        // rho comes back NaN when the set of users who have rated both movies 
        // has 0 variance amongst the rankings for one or both movies. In these
        // cases, Pearson says nothing about their correlation. 
        return 0.0;
        
        /*
        cerr << "nan on rho with a pair of movies with " << (number_users_rated(movie_i, movie_j, learned_partition))<< " common ratings\n";
        int i;
        int j;
        if (movie_i >= movie_j)
        {
            i = movie_i;
            j = movie_j;
        } else {
            j = movie_i;
            i = movie_j;
        }
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
        ell = rating_pairs(x_ij, i, j, partition);
        printf ("L = %d\n",ell);
        sum_x_i = 0;
        sum_x_j = 0;
        average_x_i = 0.0;
        average_x_j = 0.0;
        for (l=0; l < ell; l++)
        {
            sum_x_i += x_ij[2*l  ];
            sum_x_j += x_ij[2*l+1];
        }
        printf("sum_x_i = %d, sum_x_j = %d\n", sum_x_i, sum_x_j);
        average_x_i = double(sum_x_i) /  ell;
        average_x_j = double(sum_x_j) /  ell;
        printf("average_x_i = %f, average_x_j = %f\n", average_x_i, average_x_j);
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
        printf("numerator = %f, denominator_i = %f, denominator_j = %f\n", numerator, denominator_i, denominator_j);
        printf("answer should be  %f\n", (numerator / sqrt(denominator_i * denominator_j)));
         */
    }
    
    return answer;
    
}

int Movie_Knn_Pearson::force_generate_rhos(int partition)
{
    if (!(rhos == NULL))
    {
        free(rhos);
    }
    stringstream ss;
    ss << partition;
    
    string filename = string("Movie_KNN_rhos_pearson_partition_")+ ss.str()+string(".bin");
    if (data_file_exists(filename) && file_size(filename) == sizeof(double) * (((num_movies - 1)*num_movies)/2))
    {
        rhos = (double *) file_bytes(filename);
    } else {
        int start_index = 0;
        double *old_rhos = NULL;
        if (data_file_exists(filename))
        {
            start_index = (file_size(filename))/sizeof(double);
            old_rhos = (double *) file_bytes(filename);
        }
        rhos = (double *) malloc(sizeof(double) * (((num_movies - 1)*num_movies)/2));
        if (rhos == NULL)
        {
            cerr << "could not allocate memory for the rhos\n";
            return -1;
        }
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
                if (index < start_index)
                {
                    rhos[index] = old_rhos[index];
                } else {
                    if (index == start_index)
                    {
                        free(old_rhos);
                        old_rhos = NULL;
                    }
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
                        rhos[index] = (numerator / sqrt(denominator_i * denominator_j));
                    } else {
                        rhos[index] = 0.0;
                    }
                }
                if (progress_index == 1579)
                {
                    printf("now %d / 100000 complete calculating the rhos.\n", progress_counter);
                    progress_index = 0;
                    progress_counter ++;
                    if ((progress_counter % 1000 == 0) && (index > start_index))
                    {
                        array_to_file((char *)rhos, sizeof(double) * index, filename); // every 1%, write to file
                    }
                }
                index ++;
                progress_index++;
                /*
                printf("%d users rated both movie %d and %d\n", ell, i, j);
                printf("they rated them {");
                for (l=0;l<ell;l++)
                {
                    printf("(%d,%d),",x_ij[2*l],x_ij[2*l + 1]);
                }
                printf("}\n");
                printf("the average of movie %d was %f, and of %d was %f\n", i, average_x_i, j, average_x_j);
                printf(" and so the rho is %f / sqrt(%f * %f)\n", numerator, denominator_i, denominator_j); 
                printf("and so, the rho between %d and %d is %f\n", i, j, numerator / sqrt(denominator_i * denominator_j));*/
            }
        }
        free(old_rhos);
        return array_to_file((char *)rhos, sizeof(double) * (((num_movies - 1)*num_movies)/2), filename);
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

int Movie_Knn_Pearson::generate_rhos(int partition)
{
    if (rhos == NULL)
    {
        return force_generate_rhos(partition);
    }
    return 0;
}



//  public:
Movie_Knn_Pearson::Movie_Knn_Pearson()
{
    rhos = NULL;
    initiate();
}

void  Movie_Knn_Pearson::learn(int partition)
{
    learned_partition = partition;
    if (force_generate_rhos(partition)!=0)
    {
        cerr << "error when generating rhos\n";
    }
}


void  Movie_Knn_Pearson::remember(int partition)
{
    learned_partition = partition;
    if (generate_rhos(partition)!=0)
    {
        cerr << "error when generating rhos\n";
    }
}


