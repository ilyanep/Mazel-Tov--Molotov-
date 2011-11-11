#include <string>
#include "../learning_method.h"
using namespace std;
#include "../binary_files/binary_files.h"
#include "../Baseline_Oct25/baseline_oct25.h"
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include "svd_svm_nov6.h"
#include "libsvm-3.1/svm.h"

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

SVD_SVM_Nov6::SVD_SVM_Nov6(){
    data_loaded = false;
    svd_loaded = false;
    load_data();

    base_predict = Baseline_Oct25(data_loaded);
    base_predict.remember(BASE_PARTITION);
}

void SVD_SVM_Nov6::learn(int partition){
    load_svd_parameters();

    generate_svm_input(partition);
    generate_svm_param();
    free_svd();

    model = svm_train(&prob,&param);

    svm_destroy_param(&param);
	free(trainData.y);
	free(trainData.x);
	free(x_space);
}

void SVD_SVM_Nov6::free_svd(){
    if(!svd_loaded)
        return;
    gsl_matrix_free(userSVD);
    gsl_matrix_free(movieSVD);
    svd_loaded = false;
}

void SVD_SVM_Nov6::load_svd_parameters(){
    userSVD = gsl_matrix_calloc(USER_COUNT, SVD_DIM);
    movieSVD = gsl_matrix_calloc(SVD_DIM, MOVIE_COUNT);
    assert(userSVD != NULL && movieSVD != NULL);

    FILE *inFile;
    inFile = fopen(SVD_OCT25_PARAM_FILE, "r");
    assert(inFile != NULL);
    int load_partition;
    //printf("File opened.\n");
    double g = 0.0;
    fscanf(inFile,"%u",&load_partition);
    assert(load_partition == SVD_PARTITION);
    for(int user = 0; user < USER_COUNT; user++){
        for(int i = 0; i < SVD_DIM; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(userSVD, user, i, g);
	    }
    }
    for(int movie = 0; movie < MOVIE_COUNT; movie++){
        for(int i = 0; i < SVD_DIM; i++) {
            fscanf(inFile, "%lf", &g);
            gsl_matrix_set(movieSVD, i, movie, g);
        }
    }
    fclose(inFile);
    svd_loaded = true;
    return;
}

void SVD_SVM_Nov6::generate_svm_input(int partition){
    int partition_size;
    switch(partition){
        case 1:
            partition_size = PART1_SIZE;
            break;
        case 2:
            partition_size = PART2_SIZE;
            break;
        case 3:
            partition_size = PART3_SIZE;
            break;
        case 4:
            partition_size = PART4_SIZE;
            break;
        default:
            printf("Incorrect partition given to svm learn. Cannot learn");
            return;
    }
    trainData.l = partition_size;
    trainData.y = Malloc(double, partition_size);
    trainData.x = Malloc(struct svm_node *, partition_size);
    x_space = Malloc(struct svm_node, partition_size * (SVD_DIM*2 + 2));
    assert(trainData.y != NULL && trainData.x != NULL && x_space != NULL);

    int partPoint = 0;
    int nodePoint = 0;
    for(int point = 0; point < DATA_COUNT; point++){
        if( point % 10000000 == 0)
            printf("\tGenerating... %i percent\n", (int)((float)(point*100.0/DATA_COUNT)) );
        if(get_mu_idx_ratingset(point) == partition){
            prob.x[partPoint] = &x_space[nodePoint];
            prob.y[partPoint] = get_mu_all_rating(point);
            x_space[nodePoint].index = 1;
            x_space[nodePoint].value = svd_predict(get_mu_all_usernumber(point),
                                                   get_mu_all_movienumber(point),
                                                   get_mu_all_datenumber(point))
            nodePoint++;
            for(int i = 0; i < SVD_DIM; i++){
                x_space[nodePoint].index = i+2;
                x_space[nodePoint].value = gsl_matrix_get(userSVD, get_mu_all_usernumber(point)-1, i);
                nodePoint++;
            }for(int i = 0; i < SVD_DIM; i++)
                x_space[nodePoint].index = i+2+SVD_DIM;
                x_space[nodePoint].value = gsl_matrix_get(movieSVD, i, get_mu_all_movienumber(point)-1);
                nodePoint++;
            x_space[nodePoint].index = -1;
            x_space[nodePoint].value = 0.0;
            nodePoint++;
            partPoint++;
        }
    }
}

void SVD_SVM_Nov6::generate_svm_param(){
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0.001;
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 6000;
	param.C = 1;
	param.eps = 0.9;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;
}

void SVD_SVM_Nov6::save_svm();
    svm_save_model(SVM_OUTPUT_FILE, model)
    return;
}

void SVD_SVM_Nov6::remember(int partition){
    model = svm_load_model(SVD_OUTPUT_FILE);
}

void SVD_SVM_Nov6::load_data(){
    assert(load_mu_all_usernumber() == 0);
    assert(load_mu_all_movienumber() == 0);
    assert(load_mu_all_datenumber() == 0);
    assert(load_mu_all_rating() == 0);
    assert(load_mu_idx_ratingset() == 0);

    data_loaded = true;
}

double SVD_SVM_Nov6::predict(int user, int movie, int time){
    if(!svd_loaded)
        load_svd_parameters();
    struct svm_node *x;
    x = Malloc(struct svm_node, SVD_DIM*2 + 2);
    assert(x != NULL);
    int nodePoint = 0;
    x[nodePoint].index = 1;
    x[nodePoint].value = svd_predict(user, movie, time);
    nodePoint++;
    for(int i = 0; i < SVD_DIM; i++){
        x[nodePoint].index = i+2;
        x[nodePoint].value = gsl_matrix_get(userSVD, get_mu_all_usernumber(point)-1, i);
        nodePoint++;
    }for(int i = 0; i < SVD_DIM; i++)
        x[nodePoint].index = i+2+SVD_DIM;
        x[nodePoint].value = gsl_matrix_get(movieSVD, i, get_mu_all_movienumber(point)-1);
        nodePoint++;
    x[nodePoint].index = -1;
    x[nodePoint].value = 0.0;

    return rating;
}

double SVD_SVM_Nov6::rmse_probe(){
    double RMSE = 0.0;
    int count = 0;
    for(int i = 0; i < DATA_COUNT; i++) {
        if(get_mu_idx_ratingset(i) == 4 && i % 10000 == 0){
            double prediction = predict(get_mu_all_usernumber(i),
                                        (int)get_mu_all_movienumber(i),
                                        (int)get_mu_all_datenumber(i));
            double error = (prediction - (double)get_mu_all_rating(i));
            RMSE = RMSE + (error * error);
            count++;
        }
    }
    RMSE = sqrt(RMSE / ((double)count));
    return RMSE;
}   

double SVD_SVM_Nov6::svd_predict(int user, int movie, int date){
    assert(svd_loaded);
    double rating = base_predict.predict(user, movie, date);
    for (int i = 0; i < SVD_DIM; i++){
        rating = rating + gsl_matrix_get(userSVD, user-1, i) * 
                  gsl_matrix_get(movieSVD, i, movie-1);   
    }
    if (rating < 1.0)
        return 1.0;
    else if(rating > 5.0)
        return 5.0;
    return rating;
}
            
        
