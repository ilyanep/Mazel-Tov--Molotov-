// This file contains the interface to use when you implement a class that 
// does a learning method on the data. The use of this interface will help
// standardize the learning classes so that our aggregator methods can work
// more easily.

// Note that it is safe to use the global variables that contain the data
// files, as whatever class calls your learning class should have run the
// program to generate the data arrays, and I'm pretty sure that will make
// them globally available to any class

// It is suggested that you have a static variable that keeps the location
// of your residual files so that you know where to find them at all times.
// In addition, some sort of internal state that notes whether Learn()
// or Remember() have been called yet.

#ifndef LEARNING_METHODS_H
#define LEARNING_METHODS_H

class IPredictor {
  public:
    virtual ~IPredictor() {}
    // This method should use the data to learn whatever technique this class
    // is using and then write whatever residuals it calculates to a file
    // so that they can be re-used. Partition will be an integer from 1-5.
    // Learning should be done on partitions 1 through partition, inclusive.
    // Make sure that whatever file is written has some sort of metadata
    // that says what partition was learned on.
    virtual void learn(int partition) = 0;

    // This method should read any residuals that are already written onto
    // disk to avoid re-learning. If the residuals cannot be found, it
    // will call Learn().
    virtual void remember(int partition) = 0;

    // This method should return a predicted rating based on existing
    // residuals. If the internal state indicates that the learning has not
    // occurred yet, Remember() should be called.
    virtual double predict(int user, int movie, int time) = 0;

    //This method should free up whatever memory the predictor is using
    //internally so that the blender doesn't use up infinite memory when loading
    //all the predictors. It is assumed that if the predictor is needed again
    //either learn or remember will be called.
    virtual void free_mem() = 0;
};

#endif
