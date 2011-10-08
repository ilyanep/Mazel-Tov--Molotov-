// This file contains the interface to use when you implement a class that 
// does a learning method on the data. The use of this interface will help
// standardize the learning classes so that our aggregator methods can work
// more easily.

// Note that it is safe to use the global variables that contain the data
// files, as whatever class calls your learning class should have run the
// program to generate the data arrays, and I'm pretty sure that will make
// them globally available to any class

class IPredictor {
  public:
    virtual ~IPredictor() {}
    // This method should use the data to learn whatever technique this class
    // is using and then write whatever residuals it calculates to a file
    // so that they can be re-used. Partition will be an integer from 1-5.
    // Learning should be done on partitions 1 through partition, inclusive.
    virtual void Learn(int partition);
    // This method should return a predicted rating based on existing
    // residuals. If Learn() has not yet been called, it should return
    // -1. 
    virtual double Predict(int user, int movie, int time);
};
