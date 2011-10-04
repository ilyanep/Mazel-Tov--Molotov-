#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <cstdlib>
using namespace std;

#define DATA_FILES_DIRECTORY ("data_files") //the directory in which the binary data files live

bool already_in_data_files_directory = false; //set to true when directory is changed to that of the data files


/*
 * changes the current directory to the data files directory. It will create the directory, if necessary.
 */
void change_to_data_files_directory ()
{
    if (! already_in_data_files_directory)
    {
        while (chdir(DATA_FILES_DIRECTORY) != 0)
        {
            if (mkdir(DATA_FILES_DIRECTORY,0777) != 0)
            {
                cerr << "Could not create data files directory\n";
                exit(EXIT_FAILURE);
            }
        }
    }
    already_in_data_files_directory = true;
}


/*
 * writes the ratings files (.dta) to binary files. These will be written in the data files directory. 
 * mu:  true to use data files from the mu directory. false to use um.
 * all: true to use the ``all" data file. false to use the ``qual" one.
 * user_number: do you want to create a file of all the user numbers?
 * movie_number: do you want to create a file of all the movie numbers?
 * date_number: do you want to create a file of all the date numbers?
 * rating_number: do you want to create a file of all the ratings (1-5) ?
 */
int write_binary_ratings_files(bool mu, bool all, bool user_number, bool movie_number, bool date_number, bool rating_number)
{
    change_to_data_files_directory ();
    
    if ((!all) && rating_number)
    {
        return -1; // you can't get rating number on qual
    }
    
    string mu_or_um;
    if (mu)
    {
        mu_or_um = "mu";
    } else {
        mu_or_um = "um";
    }
    
    string all_or_qual;
    if (all)
    {
        all_or_qual = "all";
    } else {
        all_or_qual = "qual";
    }
    
    ifstream inputFile (("../../" + mu_or_um + "/" + all_or_qual + ".dta").c_str());
    
    
    ofstream userOutputFile;
    ofstream movieOutputFile;
    ofstream dateOutputFile;
    ofstream ratingOutputFile;
    
    
    if (user_number)
    {
        userOutputFile.open((mu_or_um + "_usernumber_4bytes_" + all_or_qual + ".bin").c_str(), ios::out | ios::binary);
    }
    
    if (movie_number)
    {
        movieOutputFile.open((mu_or_um + "_movienumber_2bytes_" + all_or_qual + ".bin").c_str(), ios::out | ios::binary);
    }
    
    if (date_number)
    {
        dateOutputFile.open((mu_or_um + "_datenumber_2bytes_" + all_or_qual + ".bin").c_str(), ios::out | ios::binary);
    }
    
    if (rating_number)
    {
        ratingOutputFile.open((mu_or_um + "_rating_1byte_" + all_or_qual + ".bin").c_str(), ios::out | ios::binary);
    }
    
    int line[4];
    char * writable;
    
    if (inputFile.is_open())
    {
        while ( inputFile.good() )
        {
            if (all)
            {
                inputFile >> line[0] >> line[1] >> line[2] >> line[3];
            } else {
                inputFile >> line[0] >> line[1] >> line[2]; //quals have no rating entry.
            }
            writable = (char *) line;
            
            if (user_number)
            {
                userOutputFile.write(&(writable[0]), 4);
            }
            
            if (movie_number)
            {
                movieOutputFile.write(&(writable[4]), 2);
            }
            
            if (date_number)
            {
                dateOutputFile.write(&(writable[8]), 2);
            }
            
            if (rating_number)
            {
                ratingOutputFile.write(&(writable[12]), 1);
            }
        }
        inputFile.close();
        userOutputFile.close();
        movieOutputFile.close();
        dateOutputFile.close();
        ratingOutputFile.close();
    } else {
        cerr << "could not open mu/all.dta\n";
        return -1;
    }
    /**
    * this stuff is for testing. It reads out all the data files as numbers to standard out. Use with caution. 
     
    char buffer[4];
    ifstream myFile;
    
    
    myFile.open((string("mu_usernumber_4bytes_") + string(all_or_qual) + string(".bin")).c_str(), ios::in | ios::binary);
    while (myFile.good())
    {
        myFile.read (buffer, 4);
        printf("%d\n",((int *)(buffer))[0]);
    }
    printf("\n\n\n");
    myFile.close();
    
    buffer[0] = (char)0;
    buffer[1] = (char)0;
    buffer[2] = (char)0;
    buffer[3] = (char)0;
    
    myFile.open((string("mu_movienumber_2bytes_") + string(all_or_qual) + string(".bin")).c_str(), ios::in | ios::binary);
    while (myFile.good())
    {
        myFile.read (buffer, 2);
        printf("%d\n",((int *)(buffer))[0]);
    }
    printf("\n\n\n");
    myFile.close();
    
    
    myFile.open((string("mu_datenumber_2bytes_") + string(all_or_qual) + string(".bin")).c_str(), ios::in | ios::binary);
    while (myFile.good())
    {
        myFile.read (buffer, 2);
        printf("%d\n",((int *)(buffer))[0]);
    }
    printf("\n\n\n");
    myFile.close();
    
    buffer[0] = (char)0;
    buffer[1] = (char)0;
    buffer[2] = (char)0;
    buffer[3] = (char)0;
    
    myFile.open((string("mu_rating_1byte_") + string(all_or_qual) + string(".bin")).c_str(), ios::in | ios::binary);
    while (myFile.good())
    {
        myFile.read (buffer, 1);
        printf("%d\n",((int *)(buffer))[0]);
    }
    printf("\n\n\n");
    myFile.close();
    **/
    return 0;

}


int main (int argc, char **argv)
{
    cout << "hello, world! \n";
    write_binary_ratings_files(true, true, true, true, true, true);
    write_binary_ratings_files(true, false, true, true, true, false);
    write_binary_ratings_files(false, true, true, true, true, true);
    return write_binary_ratings_files(false, false, true, true, true, false);
}
