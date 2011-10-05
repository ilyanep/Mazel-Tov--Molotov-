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
#define MU_DIRECTORY (string("../../mu"))   // the directory where the mu data is, relative to the data_files directory.
#define UM_DIRECTORY (string("../../um"))   // the directory where the um data is, relative to the data_files directory.

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
    
    change_to_data_files_directory (); // enter the data files directory, where the binary files will be written. 
    
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
        while ( inputFile.good() )
        {
            for (i=0; i < num_columns; i++)
            {
                inputFile >> line[0];
                outFileStreams[i].write(&(writable[0]), bytes_per_column[i]);
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
        return -1;
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

int main (int argc, char **argv)
{
    cout << "hello, world! \n";
    write_data_files_to_bin();
    
     //this stuff is for testing. It reads out all the data files as numbers to standard out. Use with caution. 
     int linelimit = 20;
     int i;
     char buffer[4];
     ifstream myFile;
     
     
     myFile.open("mu_all_usernumber_4bytes.bin", ios::in | ios::binary);
    i = 0;
     while (myFile.good() && i < linelimit)
     {
         i++;
         myFile.read (buffer, 4);
         printf("%d\n",((int *)(buffer))[0]);
     }
     printf("\n\n\n");
     myFile.close();
     
     buffer[0] = (char)0;
     buffer[1] = (char)0;
     buffer[2] = (char)0;
     buffer[3] = (char)0;
     
    myFile.open("mu_all_movienumber_2bytes.bin", ios::in | ios::binary);
    i = 0;
    while (myFile.good() && i < linelimit)
    {
        i++;
         myFile.read (buffer, 2);
         printf("%d\n",((int *)(buffer))[0]);
     }
     printf("\n\n\n");
     myFile.close();
     
     
    myFile.open("mu_all_datenumber_2bytes.bin", ios::in | ios::binary);
    i = 0;
    while (myFile.good() && i < linelimit)
    {
        i++;
         myFile.read (buffer, 2);
         printf("%d\n",((int *)(buffer))[0]);
     }
     printf("\n\n\n");
     myFile.close();
     
     buffer[0] = (char)0;
     buffer[1] = (char)0;
     buffer[2] = (char)0;
     buffer[3] = (char)0;
     
    myFile.open("mu_all_rating_1byte.bin", ios::in | ios::binary);
    i = 0;
    while (myFile.good() && i < linelimit)
    {
        i++;
         myFile.read (buffer, 1);
         printf("%d\n",((int *)(buffer))[0]);
     }
     printf("\n\n\n");
     myFile.close();
     
     
    return 0;
}
