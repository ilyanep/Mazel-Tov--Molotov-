#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <cstdlib>
#include <unistd.h>
using namespace std;
#include"binary_files.h"

int   * mu_all_usernumber   = NULL; // array of the user id numbers in the same order as the mu/all.dta file. It's NULL if it hasn't been loaded.
short * mu_all_movienumber  = NULL; // array of the movie id numbers in the same order as the mu/all.dta file. It's NULL if it hasn't been loaded.
short * mu_all_datenumber   = NULL; // array of the date numbers in the same order as the mu/all.dta file. It's NULL if it hasn't been loaded.
char  * mu_all_rating       = NULL; // array of the rating numbers (1-5) in the same order as the mu/all.dta file. It's NULL if it hasn't been loaded.
char  * mu_idx_ratingset    = NULL; // array of the data set index numbers in the same order as the mu/all.idx file. It's NULL if it hasn't been loaded.
int   * mu_qual_usernumber  = NULL; // array of the user id numbers in the same order as the mu/qual.dta file. It's NULL if it hasn't been loaded.
short * mu_qual_movienumber = NULL; // array of the movie id numbers in the same order as the mu/qual.dta file. It's NULL if it hasn't been loaded.
short * mu_qual_datenumber  = NULL; // array of the date numbers in the same order as the mu/qual.dta file. It's NULL if it hasn't been loaded.
int   * um_all_usernumber   = NULL; // array of the user id numbers in the same order as the um/all.dta file. It's NULL if it hasn't been loaded.
short * um_all_movienumber  = NULL; // array of the movie id numbers in the same order as the um/all.dta file. It's NULL if it hasn't been loaded.
short * um_all_datenumber   = NULL; // array of the date numbers in the same order as the um/all.dta file. It's NULL if it hasn't been loaded.
char  * um_all_rating       = NULL; // array of the rating numbers (1-5) in the same order as the um/all.dta file. It's NULL if it hasn't been loaded.
char  * um_idx_ratingset    = NULL; // array of the data set index numbers in the same order as the um/all.idx file. It's NULL if it hasn't been loaded.
int   * um_qual_usernumber  = NULL; // array of the user id numbers in the same order as the um/qual.dta file. It's NULL if it hasn't been loaded.
short * um_qual_movienumber = NULL; // array of the movie id numbers in the same order as the um/qual.dta file. It's NULL if it hasn't been loaded.
short * um_qual_datenumber  = NULL; // array of the date numbers in the same order as the um/qual.dta file. It's NULL if it hasn't been loaded.


/*
 * returns the current directory path, or the empty string if something went wrong.
 */
string get_current_path()
{
    
    char cCurrentPath[FILENAME_MAX];
    
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath)))
    {
        cerr << "Could not determine the current path.\n";
        return string("");
    }
    return string(cCurrentPath);
}


/*
 * changes the current directory to the data files directory. It will create the directory, if necessary. This should work as long
 * as it is running out of some subdirectory within the project directory, and the project directory contains the binary_files
 * directory, as well as the mu and um directories. 
 *
 * returns: the string representing the full path of the directory you used to be in
 * It will return the empty string if it encounters a problem, like not being in the project directory anywhere. 
 */
string change_to_data_files_directory ()
{
    string start_directory = get_current_path();
    string current_directory = start_directory;
    size_t found = current_directory.find(PROJECT_PARENT_DIRECTORY);
    
    //prodede up the directory structure until we're in the project directory, or somethign has gone wrong
    while (current_directory.length() > 0 && found != string::npos && int(found) != (current_directory.length() - PROJECT_PARENT_DIRECTORY.length()))
    {
        if (chdir("../") != 0)
        {
            cerr << "could not find project parent directory while going up directory structure.\n";
            chdir(start_directory.c_str());
            return string("");
        }
        found = current_directory.find(PROJECT_PARENT_DIRECTORY);
        current_directory = get_current_path();
    }
    
    // return empty string if something has gone wrong
    if (current_directory.length()==0 || found == string::npos || int(found) != (current_directory.length() - PROJECT_PARENT_DIRECTORY.length()))
    {
        cerr << "I was unable to find the project parent directory.\n";
        chdir(start_directory.c_str());
        return string("");
    }
    
    // enter binary files library directory
    if (chdir(BINARY_FILES_LIBRARY_DIRECTORY.c_str()) != 0)
    {
        cerr << "could not find binary files project directory.\n";
        chdir(start_directory.c_str());
        return string("");
    }
    
    // enter the data files directory, or create one if necessary 
    while (chdir(DATA_FILES_DIRECTORY.c_str()) != 0)
    {
        if (mkdir(DATA_FILES_DIRECTORY.c_str(),0777) != 0)
        {
            cerr << "Could not create data files directory\n";
            chdir(start_directory.c_str());
            return string("");
        }
    }
    return start_directory;
}




/*
 * writes information from dta files to binary files. It assumes a dta file is composed of 
 * sets of integers in ascii seperated by whitespace. 
 * inFile:           string path of the dta file to be read, relative to data_files directory
 * outFiles:         array of strings of the bin files to be written (one for each column of numbers in the dta file)
 * bytes_per_column: how many bytes should it use to represent the numbers in each column? (max: 4).
 * num_columns:      how many columns of integers are in the dta file?
 *
 * Returns 0 if all went well, -1 otherwise. 
 */
int dta_to_bins(string inFile, string *outFiles, int *bytes_per_column, int num_columns)
{
    // make sure we're reading at least one column
    if (num_columns < 1)
    {
        cerr << "You need at least one column.\n";
        return -1;
    }
    
    int i; //useful loop control variable.
    string start_directory = change_to_data_files_directory (); // enter the data files directory, where the binary files will be written. 
    if (start_directory.length()==0)
    {
        return -1;
    }
    
    ifstream inputFile (inFile.c_str()); //initialize input stream
    
    //initialize output streams
    ofstream outFileStreams[num_columns];
    for (i=0; i < num_columns; i++)
    {
        outFileStreams[i].open(outFiles[i].c_str(), ios::out | ios::binary);
    }
    
    //an int will be read repeatedly, but must be written as an array of chars, hence this:
    int line[1];
    char * writable = (char *) line;
    
    if (inputFile.is_open())
    {
        // ok, now for each line, read each int, and write the relevent bytes from that int to the relevant file.
        inputFile >> line[0];
        while ( inputFile.good() )
        {
            for (i=0; i < num_columns; i++)
            {
                outFileStreams[i].write(&(writable[0]), bytes_per_column[i]);
                inputFile >> line[0];
            }
        }
        
        //now close all of those output streams.
        for (i=0; i < num_columns; i++)
        {
            outFileStreams[i].close();
        }
        inputFile.close(); // and close the input file stream.
        
    } else { // if the file did not open, say so.
        cerr << "could not open "+inFile+"\n";
        if (chdir(start_directory.c_str()) != 0)
        {
            cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
        }
        return -1;
    }
    if (chdir(start_directory.c_str()) != 0)
    {
        cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
    }
    return 0; // all went well, so return 0. 
}

/*
 * writes an all.dta file (4 columns) to 4 .bin files. 
 * inFile:         the location of the all.dta file (including filename, relative to data_files directory)
 * outfile_prefix: the prefix (first set of characters) of the output files. ex: "mu_all"
 *
 * returns 0 if successful, -1 otherwise. 
 */
int write_all_dta_to_bin(string inFile, string outfile_prefix)
{
    string outfiles[4];
    outfiles[0] = outfile_prefix+"_usernumber_4bytes.bin";
    outfiles[1] = outfile_prefix+"_movienumber_2bytes.bin";
    outfiles[2] = outfile_prefix+"_datenumber_2bytes.bin";
    outfiles[3] = outfile_prefix+"_rating_1byte.bin";
    int bytes_per_column[4];
    bytes_per_column[0] = 4;
    bytes_per_column[1] = 2;
    bytes_per_column[2] = 2;
    bytes_per_column[3] = 1;
    return dta_to_bins(inFile, outfiles, bytes_per_column, 4);
}

/*
 * writes a qual.dta file (3 columns) to 3 .bin files. 
 * inFile:         the location of the all.dta file (including filename, relative to data_files directory)
 * outfile_prefix: the prefix (first set of characters) of the output files. ex: "mu_qual"
 *
 * returns 0 if successful, -1 otherwise. 
 */
int write_qual_dta_to_bin(string inFile, string outfile_prefix)
{
    string outfiles[3];
    outfiles[0] = outfile_prefix+"_usernumber_4bytes.bin";
    outfiles[1] = outfile_prefix+"_movienumber_2bytes.bin";
    outfiles[2] = outfile_prefix+"_datenumber_2bytes.bin";
    int bytes_per_column[3];
    bytes_per_column[0] = 4;
    bytes_per_column[1] = 2;
    bytes_per_column[2] = 2;
    return dta_to_bins(inFile, outfiles, bytes_per_column, 3);
}

/*
 * writes a .idx file (1 column) to 1 .bin file. 
 * inFile:         the location of the all.dta file (including filename, relative to data_files directory)
 * outfile_prefix: the prefix (first set of characters) of the output file. ex: "mu_idx"
 *
 * returns 0 if successful, -1 otherwise. 
 */
int write_idx_to_bin(string inFile, string outfile_prefix)
{
    string outfiles[1];
    outfiles[0] = outfile_prefix+"_ratingset_1byte.bin";
    int bytes_per_column[1];
    bytes_per_column[0] = 1;
    return dta_to_bins(inFile, outfiles, bytes_per_column, 1);
}

/*
 * writes all the data files in the "mu" direcory to .bin files. 
 * returns -(number of files from "mu" that failed to read), so 0 on full success. 
 */
int write_mu_files_to_bin()
{
    return write_all_dta_to_bin(MU_DIRECTORY+"/all.dta", "mu_all")+
        write_qual_dta_to_bin(MU_DIRECTORY+"/qual.dta", "mu_qual")+
        write_idx_to_bin(MU_DIRECTORY+"/all.idx", "mu_idx");
}

/*
 * writes all the data files in the "um" direcory to .bin files. 
 * returns -(number of files from "um" that failed to read), so 0 on full success. 
 */
int write_um_files_to_bin()
{
    return write_all_dta_to_bin(UM_DIRECTORY+"/all.dta", "um_all")+
        write_qual_dta_to_bin(UM_DIRECTORY+"/qual.dta", "um_qual")+
        write_idx_to_bin(UM_DIRECTORY+"/all.idx", "um_idx");
}


/*
 * writes all the data files in the "um" and the "mu" direcories to .bin files. 
 * returns -(number of files that failed to read), so 0 on full success. 
 */
int write_data_files_to_bin()
{
    return write_mu_files_to_bin() + write_um_files_to_bin();
}


/*
 * returns the size of the file with the filename (relative to data_files directory) input (in bytes)
 * filename: the name of the file the size of which you want
 *
 * returns the size of the file, or 0 if there is an error in reading.
 */
int file_size(string filename)
{
    string start_directory = change_to_data_files_directory (); // enter the data files directory, where the binary files will be written
    if (start_directory.length() == 0)
    {
        return 0;
    }
    FILE * inFile;
    inFile = fopen(filename.c_str(), "r");
    if (inFile == NULL)
    {
        cerr << ("could not judge the size of "+filename+".\n").c_str();
        if (chdir(start_directory.c_str()) != 0)
        {
            cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
        }
        return 0;
    }
    fseek(inFile, 0, SEEK_END);
    int answer = ftell(inFile);
    fclose(inFile);
    if (chdir(start_directory.c_str()) != 0)
    {
        cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
    }
    return answer;
}

/*
 * get the bytes of a file 
 * filename: the name of the file (relative to data_files directory) you want.
 *
 * returns a pointer (from malloc) to the bytes in memory. This is on the heap. Returns NULL if something goes wrong. 
 */
char *file_bytes(string filename)
{
    string start_directory = change_to_data_files_directory (); // enter the data files directory, where the binary files will be written
    if (start_directory.length()==0)
    {
        return NULL;
    }
    int size = file_size(filename);
    if (size > 0)
    {
        char *byte_array = (char *) malloc(size); // note that the bytes will be stored on the heap.
                                                  // This means you have to manually deallocate them.
        if (byte_array == NULL)
        {
            cerr << "malloc failed while trying to allocate " << size << " bytes for " << filename.c_str() << ".\n";
            if (chdir(start_directory.c_str()) != 0)
            {
                cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
            }
            return NULL;
        }
        ifstream inFile;
        inFile.open(filename.c_str(), ios::in | ios::binary);
        if (inFile.good())
        {
            inFile.read (byte_array, size);
        } else {
            cerr << ("could not read bytes of "+filename+".\n" ).c_str();
            if (chdir(start_directory.c_str()) != 0)
            {
                cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
            }
            return  NULL;
        }
        inFile.close();
        if (chdir(start_directory.c_str()) != 0)
        {
            cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
        }
        return byte_array;
    }
    cerr << ("could not read bytes of "+filename+", because size !> 0.\n").c_str();
    if (chdir(start_directory.c_str()) != 0)
    {
        cerr << "could not return to start directory of " << start_directory.c_str() << "\n";
    }
    return NULL;
}



// free any memory occupied by mu_all_usernumber, which stores the usernumbers from the mu/all.dta file, and set mu_all_usernumber to NULL.
void free_mu_all_usernumber()
{
    if (mu_all_usernumber != NULL)
    {
        free(mu_all_usernumber);
        mu_all_usernumber = NULL;
    }
}

/* 
 * free any memory mu_all_usernumber points to, 
 * malloc up some new memory  space, and fill it with mu_all_usernumber_4bytes.bin.
 * set mu_all_usernumber to point to that memory. Note that this is on the heap.
 * If mu_all_usernumber_4bytes.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_usernumber winds up being NULL.
 */
int force_load_mu_all_usernumber()
{
    //clear up the memory we're writing to
    free_mu_all_usernumber();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_all_usernumber_4bytes.bin", "r"))
    {
        if (write_all_dta_to_bin(MU_DIRECTORY+"/all.dta", "mu_all") != 0)
        {
            cerr << "could neither find nor create mu_all_usernumber_4bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_all_usernumber = (int *) file_bytes("mu_all_usernumber_4bytes.bin");
    
    // make sure everything read ok
    if (mu_all_usernumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_all_usernumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_all_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_usernumber_4bytes.bin into that memory. If mu_all_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_usernumber is not NULL, this trusts that mu_all_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_usernumber winds up being NULL.
 */
int load_mu_all_usernumber()
{
    if (mu_all_usernumber == NULL)
    {
        return force_load_mu_all_usernumber();
    }
    return 0;
}
/*
 * returns the indexth user number, as ordered in mu/all.dta.
 *
 * This is retrieved from the mu_all_usernumber array. 
 *
 * If mu_all_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_usernumber_4bytes.bin into that memory. If mu_all_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_usernumber is not NULL, this trusts that mu_all_usernumber is already correct. 
 *
 * index: the index (starting at 0) of the user number desired, as ordered by all.dta
 *
 * returns the indexth user number, or -1 in the event that something has gone wrong in the process of loading mu_all_usernumber_4bytes.bin.
 */
int get_mu_all_usernumber(int index)
{
    if (load_mu_all_usernumber() != 0)
    {
        cerr << "cannot get mu_all_usernumber [" << index << "], because of errors loading mu_all_usernumber_4bytes.bin.\n";
        return -1;
    }
    return mu_all_usernumber[index];
}






// free any memory occupied by mu_all_movienumber, which stores the movienumbers from the mu/all.dta file, and set mu_all_movienumber to NULL.
void free_mu_all_movienumber()
{
    if (mu_all_movienumber != NULL)
    {
        free(mu_all_movienumber);
        mu_all_movienumber = NULL;
    }
}

/* 
 * free any memory mu_all_movienumber points to, 
 * malloc up some new memory  space, and fill it with mu_all_movienumber_2bytes.bin.
 * set mu_all_movienumber to point to that memory. Note that this is on the heap.
 * If mu_all_movienumber_2bytes.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_movienumber winds up being NULL.
 */
int force_load_mu_all_movienumber()
{
    //clear up the memory we're writing to
    free_mu_all_movienumber();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_all_movienumber_2bytes.bin", "r"))
    {
        if (write_all_dta_to_bin(MU_DIRECTORY+"/all.dta", "mu_all") != 0)
        {
            cerr << "could neither find nor create mu_all_movienumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_all_movienumber = (short *) file_bytes("mu_all_movienumber_2bytes.bin");
    
    // make sure everything read ok
    if (mu_all_movienumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_all_movienumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_all_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_movienumber_2bytes.bin into that memory. If mu_all_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_movienumber is not NULL, this trusts that mu_all_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_movienumber winds up being NULL.
 */
int load_mu_all_movienumber()
{
    if (mu_all_movienumber == NULL)
    {
        return force_load_mu_all_movienumber();
    }
    return 0;
}
/*
 * returns the indexth movie number, as ordered in mu/all.dta.
 *
 * This is retrieved from the mu_all_movienumber array. 
 *
 * If mu_all_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_movienumber_2bytes.bin into that memory. If mu_all_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_movienumber is not NULL, this trusts that mu_all_movienumber is already correct. 
 *
 * index: the index (starting at 0) of the movie number desired, as ordered by all.dta
 *
 * returns the indexth movie number, or -1 in the event that something has gone wrong in the process of loading mu_all_movienumber_2bytes.bin.
 */
short get_mu_all_movienumber(int index)
{
    if (load_mu_all_movienumber() != 0)
    {
        cerr << "cannot get mu_all_movienumber [" << index << "], because of errors loading mu_all_movienumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return mu_all_movienumber[index];
}








// free any memory occupied by mu_all_datenumber, which stores the datenumbers from the mu/all.dta file, and set mu_all_datenumber to NULL.
void free_mu_all_datenumber()
{
    if (mu_all_datenumber != NULL)
    {
        free(mu_all_datenumber);
        mu_all_datenumber = NULL;
    }
}

/* 
 * free any memory mu_all_datenumber points to, 
 * malloc up some new memory  space, and fill it with mu_all_datenumber_2bytes.bin.
 * set mu_all_datenumber to point to that memory. Note that this is on the heap.
 * If mu_all_datenumber_2bytes.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_datenumber winds up being NULL.
 */
int force_load_mu_all_datenumber()
{
    //clear up the memory we're writing to
    free_mu_all_datenumber();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_all_datenumber_2bytes.bin", "r"))
    {
        if (write_all_dta_to_bin(MU_DIRECTORY+"/all.dta", "mu_all") != 0)
        {
            cerr << "could neither find nor create mu_all_datenumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_all_datenumber = (short *) file_bytes("mu_all_datenumber_2bytes.bin");
    
    // make sure everything read ok
    if (mu_all_datenumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_all_datenumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_all_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_datenumber_2bytes.bin into that memory. If mu_all_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_datenumber is not NULL, this trusts that mu_all_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_datenumber winds up being NULL.
 */
int load_mu_all_datenumber()
{
    if (mu_all_datenumber == NULL)
    {
        return force_load_mu_all_datenumber();
    }
    return 0;
}
/*
 * returns the indexth date number, as ordered in mu/all.dta.
 *
 * This is retrieved from the mu_all_datenumber array. 
 *
 * If mu_all_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_datenumber_2bytes.bin into that memory. If mu_all_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_datenumber is not NULL, this trusts that mu_all_datenumber is already correct. 
 *
 * index: the index (starting at 0) of the date number desired, as ordered by all.dta
 *
 * returns the indexth date number, or -1 in the event that something has gone wrong in the process of loading mu_all_datenumber_2bytes.bin.
 */
short get_mu_all_datenumber(int index)
{
    if (load_mu_all_datenumber() != 0)
    {
        cerr << "cannot get mu_all_datenumber [" << index << "], because of errors loading mu_all_datenumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return mu_all_datenumber[index];
}







// free any memory occupied by mu_all_rating, which stores the datenumbers from the mu/all.dta file, and set mu_all_rating to NULL.
void free_mu_all_rating()
{
    if (mu_all_rating != NULL)
    {
        free(mu_all_rating);
        mu_all_rating = NULL;
    }
}

/* 
 * free any memory mu_all_rating points to, 
 * malloc up some new memory  space, and fill it with mu_all_rating_1byte.bin.
 * set mu_all_rating to point to that memory. Note that this is on the heap.
 * If mu_all_rating_1byte.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_rating winds up being NULL.
 */
int force_load_mu_all_rating()
{
    //clear up the memory we're writing to
    free_mu_all_rating();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_all_rating_1byte.bin", "r"))
    {
        if (write_all_dta_to_bin(MU_DIRECTORY+"/all.dta", "mu_all") != 0)
        {
            cerr << "could neither find nor create mu_all_rating_1byte.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_all_rating = file_bytes("mu_all_rating_1byte.bin");
    
    // make sure everything read ok
    if (mu_all_rating == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_all_rating.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_all_rating is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_rating_1byte.bin into that memory. If mu_all_rating_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_rating is not NULL, this trusts that mu_all_rating is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_rating winds up being NULL.
 */
int load_mu_all_rating()
{
    if (mu_all_rating == NULL)
    {
        return force_load_mu_all_rating();
    }
    return 0;
}
/*
 * returns the indexth rating (1-5), as ordered in mu/all.dta.
 *
 * This is retrieved from the mu_all_rating array. 
 *
 * If mu_all_rating is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_rating_1byte.bin into that memory. If mu_all_rating_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_rating is not NULL, this trusts that mu_all_rating is already correct. 
 *
 * index: the index (starting at 0) of the rating (1-5) desired, as ordered by all.dta
 *
 * returns the indexth rating (1-5), or -1 in the event that something has gone wrong in the process of loading mu_all_rating_1byte.bin.
 */
char get_mu_all_rating(int index)
{
    if (load_mu_all_rating() != 0)
    {
        cerr << "cannot get mu_all_rating [" << index << "], because of errors loading mu_all_rating_1byte.bin.\n";
        return (char) (-1);
    }
    return mu_all_rating[index];
}









// free any memory occupied by mu_qual_usernumber, which stores the usernumbers from the mu/qual.dta file, and set mu_qual_usernumber to NULL.
void free_mu_qual_usernumber()
{
    if (mu_qual_usernumber != NULL)
    {
        free(mu_qual_usernumber);
        mu_qual_usernumber = NULL;
    }
}

/* 
 * free any memory mu_qual_usernumber points to, 
 * malloc up some new memory  space, and fill it with mu_qual_usernumber_4bytes.bin.
 * set mu_qual_usernumber to point to that memory. Note that this is on the heap.
 * If mu_qual_usernumber_4bytes.bin does not exist, this generates it from mu/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_usernumber winds up being NULL.
 */
int force_load_mu_qual_usernumber()
{
    //clear up the memory we're writing to
    free_mu_qual_usernumber();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_qual_usernumber_4bytes.bin", "r"))
    {
        if (write_qual_dta_to_bin(MU_DIRECTORY+"/qual.dta", "mu_qual") != 0)
        {
            cerr << "could neither find nor create mu_qual_usernumber_4bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_qual_usernumber = (int *) file_bytes("mu_qual_usernumber_4bytes.bin");
    
    // make sure everything read ok
    if (mu_qual_usernumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_qual_usernumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_qual_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_usernumber_4bytes.bin into that memory. If mu_qual_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_usernumber is not NULL, this trusts that mu_qual_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_usernumber winds up being NULL.
 */
int load_mu_qual_usernumber()
{
    if (mu_qual_usernumber == NULL)
    {
        return force_load_mu_qual_usernumber();
    }
    return 0;
}
/*
 * returns the indexth user number, as ordered in mu/qual.dta.
 *
 * This is retrieved from the mu_qual_usernumber array. 
 *
 * If mu_qual_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_usernumber_4bytes.bin into that memory. If mu_qual_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_usernumber is not NULL, this trusts that mu_qual_usernumber is already correct. 
 *
 * index: the index (starting at 0) of the user number desired, as ordered by qual.dta
 *
 * returns the indexth user number, or -1 in the event that something has gone wrong in the process of loading mu_qual_usernumber_4bytes.bin.
 */
int get_mu_qual_usernumber(int index)
{
    if (load_mu_qual_usernumber() != 0)
    {
        cerr << "cannot get mu_qual_usernumber [" << index << "], because of errors loading mu_qual_usernumber_4bytes.bin.\n";
        return -1;
    }
    return mu_qual_usernumber[index];
}






// free any memory occupied by mu_qual_movienumber, which stores the movienumbers from the mu/qual.dta file, and set mu_qual_movienumber to NULL.
void free_mu_qual_movienumber()
{
    if (mu_qual_movienumber != NULL)
    {
        free(mu_qual_movienumber);
        mu_qual_movienumber = NULL;
    }
}

/* 
 * free any memory mu_qual_movienumber points to, 
 * malloc up some new memory  space, and fill it with mu_qual_movienumber_2bytes.bin.
 * set mu_qual_movienumber to point to that memory. Note that this is on the heap.
 * If mu_qual_movienumber_2bytes.bin does not exist, this generates it from mu/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_movienumber winds up being NULL.
 */
int force_load_mu_qual_movienumber()
{
    //clear up the memory we're writing to
    free_mu_qual_movienumber();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_qual_movienumber_2bytes.bin", "r"))
    {
        if (write_qual_dta_to_bin(MU_DIRECTORY+"/qual.dta", "mu_qual") != 0)
        {
            cerr << "could neither find nor create mu_qual_movienumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_qual_movienumber = (short *) file_bytes("mu_qual_movienumber_2bytes.bin");
    
    // make sure everything read ok
    if (mu_qual_movienumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_qual_movienumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_qual_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_movienumber_2bytes.bin into that memory. If mu_qual_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_movienumber is not NULL, this trusts that mu_qual_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_movienumber winds up being NULL.
 */
int load_mu_qual_movienumber()
{
    if (mu_qual_movienumber == NULL)
    {
        return force_load_mu_qual_movienumber();
    }
    return 0;
}
/*
 * returns the indexth movie number, as ordered in mu/qual.dta.
 *
 * This is retrieved from the mu_qual_movienumber array. 
 *
 * If mu_qual_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_movienumber_2bytes.bin into that memory. If mu_qual_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_movienumber is not NULL, this trusts that mu_qual_movienumber is already correct. 
 *
 * index: the index (starting at 0) of the movie number desired, as ordered by qual.dta
 *
 * returns the indexth movie number, or -1 in the event that something has gone wrong in the process of loading mu_qual_movienumber_2bytes.bin.
 */
short get_mu_qual_movienumber(int index)
{
    if (load_mu_qual_movienumber() != 0)
    {
        cerr << "cannot get mu_qual_movienumber [" << index << "], because of errors loading mu_qual_movienumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return mu_qual_movienumber[index];
}








// free any memory occupied by mu_qual_datenumber, which stores the datenumbers from the mu/qual.dta file, and set mu_qual_datenumber to NULL.
void free_mu_qual_datenumber()
{
    if (mu_qual_datenumber != NULL)
    {
        free(mu_qual_datenumber);
        mu_qual_datenumber = NULL;
    }
}

/* 
 * free any memory mu_qual_datenumber points to, 
 * malloc up some new memory  space, and fill it with mu_qual_datenumber_2bytes.bin.
 * set mu_qual_datenumber to point to that memory. Note that this is on the heap.
 * If mu_qual_datenumber_2bytes.bin does not exist, this generates it from mu/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_datenumber winds up being NULL.
 */
int force_load_mu_qual_datenumber()
{
    //clear up the memory we're writing to
    free_mu_qual_datenumber();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_qual_datenumber_2bytes.bin", "r"))
    {
        if (write_qual_dta_to_bin(MU_DIRECTORY+"/qual.dta", "mu_qual") != 0)
        {
            cerr << "could neither find nor create mu_qual_datenumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_qual_datenumber = (short *) file_bytes("mu_qual_datenumber_2bytes.bin");
    
    // make sure everything read ok
    if (mu_qual_datenumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_qual_datenumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_qual_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_datenumber_2bytes.bin into that memory. If mu_qual_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_datenumber is not NULL, this trusts that mu_qual_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_datenumber winds up being NULL.
 */
int load_mu_qual_datenumber()
{
    if (mu_qual_datenumber == NULL)
    {
        return force_load_mu_qual_datenumber();
    }
    return 0;
}
/*
 * returns the indexth date number, as ordered in mu/qual.dta.
 *
 * This is retrieved from the mu_qual_datenumber array. 
 *
 * If mu_qual_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_datenumber_2bytes.bin into that memory. If mu_qual_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_datenumber is not NULL, this trusts that mu_qual_datenumber is already correct. 
 *
 * index: the index (starting at 0) of the date number desired, as ordered by qual.dta
 *
 * returns the indexth date number, or -1 in the event that something has gone wrong in the process of loading mu_qual_datenumber_2bytes.bin.
 */
short get_mu_qual_datenumber(int index)
{
    if (load_mu_qual_datenumber() != 0)
    {
        cerr << "cannot get mu_qual_datenumber [" << index << "], because of errors loading mu_qual_datenumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return mu_qual_datenumber[index];
}















// free any memory occupied by mu_idx_ratingset, which stores the datenumbers from the mu/all.dta file, and set mu_idx_ratingset to NULL.
void free_mu_idx_ratingset()
{
    if (mu_idx_ratingset != NULL)
    {
        free(mu_idx_ratingset);
        mu_idx_ratingset = NULL;
    }
}

/* 
 * free any memory mu_idx_ratingset points to, 
 * malloc up some new memory  space, and fill it with mu_idx_ratingset_1byte.bin.
 * set mu_idx_ratingset to point to that memory. Note that this is on the heap.
 * If mu_idx_ratingset_1byte.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_idx_ratingset winds up being NULL.
 */
int force_load_mu_idx_ratingset()
{
    //clear up the memory we're writing to
    free_mu_idx_ratingset();
    
    // make sure we have a file to be reading from
    if (! fopen("mu_idx_ratingset_1byte.bin", "r"))
    {
        if (write_idx_to_bin(MU_DIRECTORY+"/all.idx", "mu_idx") != 0)
        {
            cerr << "could neither find nor create mu_idx_ratingset_1byte.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    mu_idx_ratingset = file_bytes("mu_idx_ratingset_1byte.bin");
    
    // make sure everything read ok
    if (mu_idx_ratingset == NULL)
    {
        cerr << "error whilst trying to read the bytes for mu_idx_ratingset.\n";
        return -1;
    }
    return 0;
}

/*
 * If mu_idx_ratingset is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_idx_ratingset_1byte.bin into that memory. If mu_idx_ratingset_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_idx_ratingset is not NULL, this trusts that mu_idx_ratingset is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_idx_ratingset winds up being NULL.
 */
int load_mu_idx_ratingset()
{
    if (mu_idx_ratingset == NULL)
    {
        return force_load_mu_idx_ratingset();
    }
    return 0;
}
/*
 * returns the indexth rating set (1-5), as ordered in mu/all.dta.
 *
 * This is retrieved from the mu_idx_ratingset array. 
 *
 * If mu_idx_ratingset is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_idx_ratingset_1byte.bin into that memory. If mu_idx_ratingset_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_idx_ratingset is not NULL, this trusts that mu_idx_ratingset is already correct. 
 *
 * index: the index (starting at 0) of the rating set (1-5) desired, as ordered by all.dta
 *
 * returns the indexth rating set (1-5), or -1 in the event that something has gone wrong in the process of loading mu_idx_ratingset_1byte.bin.
 */
char get_mu_idx_ratingset(int index)
{
    if (load_mu_idx_ratingset() != 0)
    {
        cerr << "cannot get mu_idx_ratingset [" << index << "], because of errors loading mu_idx_ratingset_1byte.bin.\n";
        return (char) (-1);
    }
    return mu_idx_ratingset[index];
}







// free any memory occupied by um_all_usernumber, which stores the usernumbers from the um/all.dta file, and set um_all_usernumber to NULL.
void free_um_all_usernumber()
{
    if (um_all_usernumber != NULL)
    {
        free(um_all_usernumber);
        um_all_usernumber = NULL;
    }
}

/* 
 * free any memory um_all_usernumber points to, 
 * malloc up some new memory  space, and fill it with um_all_usernumber_4bytes.bin.
 * set um_all_usernumber to point to that memory. Note that this is on the heap.
 * If um_all_usernumber_4bytes.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_usernumber winds up being NULL.
 */
int force_load_um_all_usernumber()
{
    //clear up the memory we're writing to
    free_um_all_usernumber();
    
    // make sure we have a file to be reading from
    if (! fopen("um_all_usernumber_4bytes.bin", "r"))
    {
        if (write_all_dta_to_bin(UM_DIRECTORY+"/all.dta", "um_all") != 0)
        {
            cerr << "could neither find nor create um_all_usernumber_4bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_all_usernumber = (int *) file_bytes("um_all_usernumber_4bytes.bin");
    
    // make sure everything read ok
    if (um_all_usernumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_all_usernumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_all_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_usernumber_4bytes.bin into that memory. If um_all_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_usernumber is not NULL, this trusts that um_all_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_usernumber winds up being NULL.
 */
int load_um_all_usernumber()
{
    if (um_all_usernumber == NULL)
    {
        return force_load_um_all_usernumber();
    }
    return 0;
}
/*
 * returns the indexth user number, as ordered in um/all.dta.
 *
 * This is retrieved from the um_all_usernumber array. 
 *
 * If um_all_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_usernumber_4bytes.bin into that memory. If um_all_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_usernumber is not NULL, this trusts that um_all_usernumber is already correct. 
 *
 * index: the index (starting at 0) of the user number desired, as ordered by all.dta
 *
 * returns the indexth user number, or -1 in the event that something has gone wrong in the process of loading um_all_usernumber_4bytes.bin.
 */
int get_um_all_usernumber(int index)
{
    if (load_um_all_usernumber() != 0)
    {
        cerr << "cannot get um_all_usernumber [" << index << "], because of errors loading um_all_usernumber_4bytes.bin.\n";
        return -1;
    }
    return um_all_usernumber[index];
}






// free any memory occupied by um_all_movienumber, which stores the movienumbers from the um/all.dta file, and set um_all_movienumber to NULL.
void free_um_all_movienumber()
{
    if (um_all_movienumber != NULL)
    {
        free(um_all_movienumber);
        um_all_movienumber = NULL;
    }
}

/* 
 * free any memory um_all_movienumber points to, 
 * malloc up some new memory  space, and fill it with um_all_movienumber_2bytes.bin.
 * set um_all_movienumber to point to that memory. Note that this is on the heap.
 * If um_all_movienumber_2bytes.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_movienumber winds up being NULL.
 */
int force_load_um_all_movienumber()
{
    //clear up the memory we're writing to
    free_um_all_movienumber();
    
    // make sure we have a file to be reading from
    if (! fopen("um_all_movienumber_2bytes.bin", "r"))
    {
        if (write_all_dta_to_bin(UM_DIRECTORY+"/all.dta", "um_all") != 0)
        {
            cerr << "could neither find nor create um_all_movienumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_all_movienumber = (short *) file_bytes("um_all_movienumber_2bytes.bin");
    
    // make sure everything read ok
    if (um_all_movienumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_all_movienumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_all_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_movienumber_2bytes.bin into that memory. If um_all_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_movienumber is not NULL, this trusts that um_all_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_movienumber winds up being NULL.
 */
int load_um_all_movienumber()
{
    if (um_all_movienumber == NULL)
    {
        return force_load_um_all_movienumber();
    }
    return 0;
}
/*
 * returns the indexth movie number, as ordered in um/all.dta.
 *
 * This is retrieved from the um_all_movienumber array. 
 *
 * If um_all_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_movienumber_2bytes.bin into that memory. If um_all_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_movienumber is not NULL, this trusts that um_all_movienumber is already correct. 
 *
 * index: the index (starting at 0) of the movie number desired, as ordered by all.dta
 *
 * returns the indexth movie number, or -1 in the event that something has gone wrong in the process of loading um_all_movienumber_2bytes.bin.
 */
short get_um_all_movienumber(int index)
{
    if (load_um_all_movienumber() != 0)
    {
        cerr << "cannot get um_all_movienumber [" << index << "], because of errors loading um_all_movienumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return um_all_movienumber[index];
}








// free any memory occupied by um_all_datenumber, which stores the datenumbers from the um/all.dta file, and set um_all_datenumber to NULL.
void free_um_all_datenumber()
{
    if (um_all_datenumber != NULL)
    {
        free(um_all_datenumber);
        um_all_datenumber = NULL;
    }
}

/* 
 * free any memory um_all_datenumber points to, 
 * malloc up some new memory  space, and fill it with um_all_datenumber_2bytes.bin.
 * set um_all_datenumber to point to that memory. Note that this is on the heap.
 * If um_all_datenumber_2bytes.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_datenumber winds up being NULL.
 */
int force_load_um_all_datenumber()
{
    //clear up the memory we're writing to
    free_um_all_datenumber();
    
    // make sure we have a file to be reading from
    if (! fopen("um_all_datenumber_2bytes.bin", "r"))
    {
        if (write_all_dta_to_bin(UM_DIRECTORY+"/all.dta", "um_all") != 0)
        {
            cerr << "could neither find nor create um_all_datenumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_all_datenumber = (short *) file_bytes("um_all_datenumber_2bytes.bin");
    
    // make sure everything read ok
    if (um_all_datenumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_all_datenumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_all_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_datenumber_2bytes.bin into that memory. If um_all_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_datenumber is not NULL, this trusts that um_all_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_datenumber winds up being NULL.
 */
int load_um_all_datenumber()
{
    if (um_all_datenumber == NULL)
    {
        return force_load_um_all_datenumber();
    }
    return 0;
}
/*
 * returns the indexth date number, as ordered in um/all.dta.
 *
 * This is retrieved from the um_all_datenumber array. 
 *
 * If um_all_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_datenumber_2bytes.bin into that memory. If um_all_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_datenumber is not NULL, this trusts that um_all_datenumber is already correct. 
 *
 * index: the index (starting at 0) of the date number desired, as ordered by all.dta
 *
 * returns the indexth date number, or -1 in the event that something has gone wrong in the process of loading um_all_datenumber_2bytes.bin.
 */
short get_um_all_datenumber(int index)
{
    if (load_um_all_datenumber() != 0)
    {
        cerr << "cannot get um_all_datenumber [" << index << "], because of errors loading um_all_datenumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return um_all_datenumber[index];
}







// free any memory occupied by um_all_rating, which stores the datenumbers from the um/all.dta file, and set um_all_rating to NULL.
void free_um_all_rating()
{
    if (um_all_rating != NULL)
    {
        free(um_all_rating);
        um_all_rating = NULL;
    }
}

/* 
 * free any memory um_all_rating points to, 
 * malloc up some new memory  space, and fill it with um_all_rating_1byte.bin.
 * set um_all_rating to point to that memory. Note that this is on the heap.
 * If um_all_rating_1byte.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_rating winds up being NULL.
 */
int force_load_um_all_rating()
{
    //clear up the memory we're writing to
    free_um_all_rating();
    
    // make sure we have a file to be reading from
    if (! fopen("um_all_rating_1byte.bin", "r"))
    {
        if (write_all_dta_to_bin(UM_DIRECTORY+"/all.dta", "um_all") != 0)
        {
            cerr << "could neither find nor create um_all_rating_1byte.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_all_rating = file_bytes("um_all_rating_1byte.bin");
    
    // make sure everything read ok
    if (um_all_rating == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_all_rating.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_all_rating is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_rating_1byte.bin into that memory. If um_all_rating_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_rating is not NULL, this trusts that um_all_rating is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_rating winds up being NULL.
 */
int load_um_all_rating()
{
    if (um_all_rating == NULL)
    {
        return force_load_um_all_rating();
    }
    return 0;
}
/*
 * returns the indexth rating (1-5), as ordered in um/all.dta.
 *
 * This is retrieved from the um_all_rating array. 
 *
 * If um_all_rating is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_rating_1byte.bin into that memory. If um_all_rating_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_rating is not NULL, this trusts that um_all_rating is already correct. 
 *
 * index: the index (starting at 0) of the rating (1-5) desired, as ordered by all.dta
 *
 * returns the indexth rating (1-5), or -1 in the event that something has gone wrong in the process of loading um_all_rating_1byte.bin.
 */
char get_um_all_rating(int index)
{
    if (load_um_all_rating() != 0)
    {
        cerr << "cannot get um_all_rating [" << index << "], because of errors loading um_all_rating_1byte.bin.\n";
        return (char) (-1);
    }
    return um_all_rating[index];
}









// free any memory occupied by um_qual_usernumber, which stores the usernumbers from the um/qual.dta file, and set um_qual_usernumber to NULL.
void free_um_qual_usernumber()
{
    if (um_qual_usernumber != NULL)
    {
        free(um_qual_usernumber);
        um_qual_usernumber = NULL;
    }
}

/* 
 * free any memory um_qual_usernumber points to, 
 * malloc up some new memory  space, and fill it with um_qual_usernumber_4bytes.bin.
 * set um_qual_usernumber to point to that memory. Note that this is on the heap.
 * If um_qual_usernumber_4bytes.bin does not exist, this generates it from um/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_usernumber winds up being NULL.
 */
int force_load_um_qual_usernumber()
{
    //clear up the memory we're writing to
    free_um_qual_usernumber();
    
    // make sure we have a file to be reading from
    if (! fopen("um_qual_usernumber_4bytes.bin", "r"))
    {
        if (write_qual_dta_to_bin(UM_DIRECTORY+"/qual.dta", "um_qual") != 0)
        {
            cerr << "could neither find nor create um_qual_usernumber_4bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_qual_usernumber = (int *) file_bytes("um_qual_usernumber_4bytes.bin");
    
    // make sure everything read ok
    if (um_qual_usernumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_qual_usernumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_qual_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_usernumber_4bytes.bin into that memory. If um_qual_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_usernumber is not NULL, this trusts that um_qual_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_usernumber winds up being NULL.
 */
int load_um_qual_usernumber()
{
    if (um_qual_usernumber == NULL)
    {
        return force_load_um_qual_usernumber();
    }
    return 0;
}
/*
 * returns the indexth user number, as ordered in um/qual.dta.
 *
 * This is retrieved from the um_qual_usernumber array. 
 *
 * If um_qual_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_usernumber_4bytes.bin into that memory. If um_qual_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_usernumber is not NULL, this trusts that um_qual_usernumber is already correct. 
 *
 * index: the index (starting at 0) of the user number desired, as ordered by qual.dta
 *
 * returns the indexth user number, or -1 in the event that something has gone wrong in the process of loading um_qual_usernumber_4bytes.bin.
 */
int get_um_qual_usernumber(int index)
{
    if (load_um_qual_usernumber() != 0)
    {
        cerr << "cannot get um_qual_usernumber [" << index << "], because of errors loading um_qual_usernumber_4bytes.bin.\n";
        return -1;
    }
    return um_qual_usernumber[index];
}






// free any memory occupied by um_qual_movienumber, which stores the movienumbers from the um/qual.dta file, and set um_qual_movienumber to NULL.
void free_um_qual_movienumber()
{
    if (um_qual_movienumber != NULL)
    {
        free(um_qual_movienumber);
        um_qual_movienumber = NULL;
    }
}

/* 
 * free any memory um_qual_movienumber points to, 
 * malloc up some new memory  space, and fill it with um_qual_movienumber_2bytes.bin.
 * set um_qual_movienumber to point to that memory. Note that this is on the heap.
 * If um_qual_movienumber_2bytes.bin does not exist, this generates it from um/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_movienumber winds up being NULL.
 */
int force_load_um_qual_movienumber()
{
    //clear up the memory we're writing to
    free_um_qual_movienumber();
    
    // make sure we have a file to be reading from
    if (! fopen("um_qual_movienumber_2bytes.bin", "r"))
    {
        if (write_qual_dta_to_bin(UM_DIRECTORY+"/qual.dta", "um_qual") != 0)
        {
            cerr << "could neither find nor create um_qual_movienumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_qual_movienumber = (short *) file_bytes("um_qual_movienumber_2bytes.bin");
    
    // make sure everything read ok
    if (um_qual_movienumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_qual_movienumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_qual_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_movienumber_2bytes.bin into that memory. If um_qual_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_movienumber is not NULL, this trusts that um_qual_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_movienumber winds up being NULL.
 */
int load_um_qual_movienumber()
{
    if (um_qual_movienumber == NULL)
    {
        return force_load_um_qual_movienumber();
    }
    return 0;
}
/*
 * returns the indexth movie number, as ordered in um/qual.dta.
 *
 * This is retrieved from the um_qual_movienumber array. 
 *
 * If um_qual_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_movienumber_2bytes.bin into that memory. If um_qual_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_movienumber is not NULL, this trusts that um_qual_movienumber is already correct. 
 *
 * index: the index (starting at 0) of the movie number desired, as ordered by qual.dta
 *
 * returns the indexth movie number, or -1 in the event that something has gone wrong in the process of loading um_qual_movienumber_2bytes.bin.
 */
short get_um_qual_movienumber(int index)
{
    if (load_um_qual_movienumber() != 0)
    {
        cerr << "cannot get um_qual_movienumber [" << index << "], because of errors loading um_qual_movienumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return um_qual_movienumber[index];
}








// free any memory occupied by um_qual_datenumber, which stores the datenumbers from the um/qual.dta file, and set um_qual_datenumber to NULL.
void free_um_qual_datenumber()
{
    if (um_qual_datenumber != NULL)
    {
        free(um_qual_datenumber);
        um_qual_datenumber = NULL;
    }
}

/* 
 * free any memory um_qual_datenumber points to, 
 * malloc up some new memory  space, and fill it with um_qual_datenumber_2bytes.bin.
 * set um_qual_datenumber to point to that memory. Note that this is on the heap.
 * If um_qual_datenumber_2bytes.bin does not exist, this generates it from um/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_datenumber winds up being NULL.
 */
int force_load_um_qual_datenumber()
{
    //clear up the memory we're writing to
    free_um_qual_datenumber();
    
    // make sure we have a file to be reading from
    if (! fopen("um_qual_datenumber_2bytes.bin", "r"))
    {
        if (write_qual_dta_to_bin(UM_DIRECTORY+"/qual.dta", "um_qual") != 0)
        {
            cerr << "could neither find nor create um_qual_datenumber_2bytes.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_qual_datenumber = (short *) file_bytes("um_qual_datenumber_2bytes.bin");
    
    // make sure everything read ok
    if (um_qual_datenumber == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_qual_datenumber.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_qual_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_datenumber_2bytes.bin into that memory. If um_qual_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_datenumber is not NULL, this trusts that um_qual_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_datenumber winds up being NULL.
 */
int load_um_qual_datenumber()
{
    if (um_qual_datenumber == NULL)
    {
        return force_load_um_qual_datenumber();
    }
    return 0;
}
/*
 * returns the indexth date number, as ordered in um/qual.dta.
 *
 * This is retrieved from the um_qual_datenumber array. 
 *
 * If um_qual_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_datenumber_2bytes.bin into that memory. If um_qual_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_datenumber is not NULL, this trusts that um_qual_datenumber is already correct. 
 *
 * index: the index (starting at 0) of the date number desired, as ordered by qual.dta
 *
 * returns the indexth date number, or -1 in the event that something has gone wrong in the process of loading um_qual_datenumber_2bytes.bin.
 */
short get_um_qual_datenumber(int index)
{
    if (load_um_qual_datenumber() != 0)
    {
        cerr << "cannot get um_qual_datenumber [" << index << "], because of errors loading um_qual_datenumber_2bytes.bin.\n";
        return (short) (-1);
    }
    return um_qual_datenumber[index];
}















// free any memory occupied by um_idx_ratingset, which stores the datenumbers from the um/all.dta file, and set um_idx_ratingset to NULL.
void free_um_idx_ratingset()
{
    if (um_idx_ratingset != NULL)
    {
        free(um_idx_ratingset);
        um_idx_ratingset = NULL;
    }
}

/* 
 * free any memory um_idx_ratingset points to, 
 * malloc up some new memory  space, and fill it with um_idx_ratingset_1byte.bin.
 * set um_idx_ratingset to point to that memory. Note that this is on the heap.
 * If um_idx_ratingset_1byte.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_idx_ratingset winds up being NULL.
 */
int force_load_um_idx_ratingset()
{
    //clear up the memory we're writing to
    free_um_idx_ratingset();
    
    // make sure we have a file to be reading from
    if (! fopen("um_idx_ratingset_1byte.bin", "r"))
    {
        if (write_idx_to_bin(UM_DIRECTORY+"/all.idx", "um_idx") != 0)
        {
            cerr << "could neither find nor create um_idx_ratingset_1byte.bin.\n";
            return -1;
        }
    }
    
    //read the file to the global variable
    um_idx_ratingset = file_bytes("um_idx_ratingset_1byte.bin");
    
    // make sure everything read ok
    if (um_idx_ratingset == NULL)
    {
        cerr << "error whilst trying to read the bytes for um_idx_ratingset.\n";
        return -1;
    }
    return 0;
}

/*
 * If um_idx_ratingset is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_idx_ratingset_1byte.bin into that memory. If um_idx_ratingset_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_idx_ratingset is not NULL, this trusts that um_idx_ratingset is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_idx_ratingset winds up being NULL.
 */
int load_um_idx_ratingset()
{
    if (um_idx_ratingset == NULL)
    {
        return force_load_um_idx_ratingset();
    }
    return 0;
}

/*
 * returns the indexth rating set (1-5), as ordered in um/all.dta.
 *
 * This is retrieved from the um_idx_ratingset array. 
 *
 * If um_idx_ratingset is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_idx_ratingset_1byte.bin into that memory. If um_idx_ratingset_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_idx_ratingset is not NULL, this trusts that um_idx_ratingset is already correct. 
 *
 * index: the index (starting at 0) of the rating set (1-5) desired, as ordered by all.dta
 *
 * returns the indexth rating set (1-5), or -1 in the event that something has gone wrong in the process of loading um_idx_ratingset_1byte.bin.
 */
char get_um_idx_ratingset(int index)
{
    if (load_um_idx_ratingset() != 0)
    {
        cerr << "cannot get um_idx_ratingset [" << index << "], because of errors loading um_idx_ratingset_1byte.bin.\n";
        return (char) (-1);
    }
    return um_idx_ratingset[index];
}