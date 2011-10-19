#ifndef BINARY_FILES_BINARY_FILES_H
#define BINARY_FILES_BINARY_FILES_H

#define PROJECT_PARENT_DIRECTORY (string("Mazel-Tov--Molotov-")) // the project folder name
#define BINARY_FILES_LIBRARY_DIRECTORY (string("binary_files")) // the directory containing this library, relative to the project folder
#define DATA_FILES_DIRECTORY (string("data_files")) //the directory in which the binary data files live, relative to this library
#define MU_DIRECTORY (string("../../mu"))   // the directory where the mu data is, relative to the data_files directory.
#define UM_DIRECTORY (string("../../um"))   // the directory where the um data is, relative to the data_files directory.

/*
 * returns the current directory path, or the empty string if something went wrong.
 */
string get_current_path();

/*
 * changes the current directory to the data files directory. It will create the directory, if necessary. This should work as long
 * as it is running out of some subdirectory within the project directory, and the project directory contains the binary_files
 * directory, as well as the mu and um directories. 
 *
 * returns: the string representing the full path of the directory you used to be in
 * It will return the empty string if it encounters a problem, like not being in the project directory anywhere. 
 */
string change_to_data_files_directory ();





/*
 * writes byte array to a binary file in the data files directory. 
 * bytes:  the char * to be written
 * length: the number of bytes to write
 * ouFile: the name of the file to be written
 *
 * Returns 0 if all went well, -1 otherwise. 
 */
int array_to_file(char *bytes, int length, string outFile);




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
int dta_to_bins(string inFile, string *outFiles, int *bytes_per_column, int num_columns);

/*
 * writes an all.dta file (4 columns) to 4 .bin files. 
 * inFile:         the location of the all.dta file (including filename, relative to data_files directory)
 * outfile_prefix: the prefix (first set of characters) of the output files. ex: "mu_all"
 *
 * returns 0 if successful, -1 otherwise. 
 */
int write_all_dta_to_bin(string inFile, string outfile_prefix);

/*
 * writes a qual.dta file (3 columns) to 3 .bin files. 
 * inFile:         the location of the all.dta file (including filename, relative to data_files directory)
 * outfile_prefix: the prefix (first set of characters) of the output files. ex: "mu_qual"
 *
 * returns 0 if successful, -1 otherwise. 
 */
int write_qual_dta_to_bin(string inFile, string outfile_prefix);

/*
 * writes a .idx file (1 column) to 1 .bin file. 
 * inFile:         the location of the all.dta file (including filename, relative to data_files directory)
 * outfile_prefix: the prefix (first set of characters) of the output file. ex: "mu_idx"
 *
 * returns 0 if successful, -1 otherwise. 
 */
int write_idx_to_bin(string inFile, string outfile_prefix);

/*
 * writes all the data files in the "mu" direcory to .bin files. 
 * returns -(number of files from "mu" that failed to read), so 0 on full success. 
 */
int write_mu_files_to_bin();

/*
 * writes all the data files in the "um" direcory to .bin files. 
 * returns -(number of files from "um" that failed to read), so 0 on full success. 
 */
int write_um_files_to_bin();


/*
 * writes all the data files in the "um" and the "mu" direcories to .bin files. 
 * returns -(number of files that failed to read), so 0 on full success. 
 */
int write_data_files_to_bin();

/*
 * returns whether or not a file of name filename exists in the DATA_FILES directory. 
 */
bool data_file_exists(string filename);

/*
 * returns the size of the file with the filename (relative to data_files directory) input (in bytes)
 * filename: the name of the file the size of which you want
 *
 * returns the size of the file, or 0 if there is an error in reading.
 */
int file_size(string filename);

/*
 * get the bytes of a file 
 * filename: the name of the file (relative to data_files directory) you want.
 *
 * returns a pointer (from malloc) to the bytes in memory. This is on the heap. Returns NULL if something goes wrong. 
 */
char *file_bytes(string filename);



// free any memory occupied by mu_all_usernumber, which stores the usernumbers from the mu/all.dta file, and set mu_all_usernumber to NULL.
void free_mu_all_usernumber();

/* 
 * free any memory mu_all_usernumber points to, 
 * malloc up some new memory  space, and fill it with mu_all_usernumber_4bytes.bin.
 * set mu_all_usernumber to point to that memory. Note that this is on the heap.
 * If mu_all_usernumber_4bytes.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_usernumber winds up being NULL.
 */
int force_load_mu_all_usernumber();

/*
 * If mu_all_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_usernumber_4bytes.bin into that memory. If mu_all_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_usernumber is not NULL, this trusts that mu_all_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_usernumber winds up being NULL.
 */
int load_mu_all_usernumber();


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
int get_mu_all_usernumber(int index);



// free any memory occupied by mu_all_movienumber, which stores the movienumbers from the mu/all.dta file, and set mu_all_movienumber to NULL.
void free_mu_all_movienumber();

/* 
 * free any memory mu_all_movienumber points to, 
 * malloc up some new memory  space, and fill it with mu_all_movienumber_2bytes.bin.
 * set mu_all_movienumber to point to that memory. Note that this is on the heap.
 * If mu_all_movienumber_2bytes.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_movienumber winds up being NULL.
 */
int force_load_mu_all_movienumber();

/*
 * If mu_all_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_movienumber_2bytes.bin into that memory. If mu_all_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_movienumber is not NULL, this trusts that mu_all_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_movienumber winds up being NULL.
 */
int load_mu_all_movienumber();

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
short get_mu_all_movienumber(int index);


// free any memory occupied by mu_all_datenumber, which stores the datenumbers from the mu/all.dta file, and set mu_all_datenumber to NULL.
void free_mu_all_datenumber();

/* 
 * free any memory mu_all_datenumber points to, 
 * malloc up some new memory  space, and fill it with mu_all_datenumber_2bytes.bin.
 * set mu_all_datenumber to point to that memory. Note that this is on the heap.
 * If mu_all_datenumber_2bytes.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_datenumber winds up being NULL.
 */
int force_load_mu_all_datenumber();

/*
 * If mu_all_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_datenumber_2bytes.bin into that memory. If mu_all_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_datenumber is not NULL, this trusts that mu_all_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_datenumber winds up being NULL.
 */
int load_mu_all_datenumber();

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
short get_mu_all_datenumber(int index);


// free any memory occupied by mu_all_rating, which stores the datenumbers from the mu/all.dta file, and set mu_all_rating to NULL.
void free_mu_all_rating(); 

/* 
 * free any memory mu_all_rating points to, 
 * malloc up some new memory  space, and fill it with mu_all_rating_1byte.bin.
 * set mu_all_rating to point to that memory. Note that this is on the heap.
 * If mu_all_rating_1byte.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_rating winds up being NULL.
 */
int force_load_mu_all_rating();

/*
 * If mu_all_rating is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_all_rating_1byte.bin into that memory. If mu_all_rating_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_all_rating is not NULL, this trusts that mu_all_rating is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_all_rating winds up being NULL.
 */
int load_mu_all_rating();

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
char get_mu_all_rating(int index);


// free any memory occupied by mu_qual_usernumber, which stores the usernumbers from the mu/qual.dta file, and set mu_qual_usernumber to NULL.
void free_mu_qual_usernumber();

/* 
 * free any memory mu_qual_usernumber points to, 
 * malloc up some new memory  space, and fill it with mu_qual_usernumber_4bytes.bin.
 * set mu_qual_usernumber to point to that memory. Note that this is on the heap.
 * If mu_qual_usernumber_4bytes.bin does not exist, this generates it from mu/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_usernumber winds up being NULL.
 */
int force_load_mu_qual_usernumber();

/*
 * If mu_qual_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_usernumber_4bytes.bin into that memory. If mu_qual_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_usernumber is not NULL, this trusts that mu_qual_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_usernumber winds up being NULL.
 */
int load_mu_qual_usernumber();

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
int get_mu_qual_usernumber(int index);

// free any memory occupied by mu_qual_movienumber, which stores the movienumbers from the mu/qual.dta file, and set mu_qual_movienumber to NULL.
void free_mu_qual_movienumber();

/* 
 * free any memory mu_qual_movienumber points to, 
 * malloc up some new memory  space, and fill it with mu_qual_movienumber_2bytes.bin.
 * set mu_qual_movienumber to point to that memory. Note that this is on the heap.
 * If mu_qual_movienumber_2bytes.bin does not exist, this generates it from mu/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_movienumber winds up being NULL.
 */
int force_load_mu_qual_movienumber();

/*
 * If mu_qual_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_movienumber_2bytes.bin into that memory. If mu_qual_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_movienumber is not NULL, this trusts that mu_qual_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_movienumber winds up being NULL.
 */
int load_mu_qual_movienumber();

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
short get_mu_qual_movienumber(int index);

// free any memory occupied by mu_qual_datenumber, which stores the datenumbers from the mu/qual.dta file, and set mu_qual_datenumber to NULL.
void free_mu_qual_datenumber();

/* 
 * free any memory mu_qual_datenumber points to, 
 * malloc up some new memory  space, and fill it with mu_qual_datenumber_2bytes.bin.
 * set mu_qual_datenumber to point to that memory. Note that this is on the heap.
 * If mu_qual_datenumber_2bytes.bin does not exist, this generates it from mu/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_datenumber winds up being NULL.
 */
int force_load_mu_qual_datenumber();

/*
 * If mu_qual_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_qual_datenumber_2bytes.bin into that memory. If mu_qual_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from mu/qual.dta.
 * If mu_qual_datenumber is not NULL, this trusts that mu_qual_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_qual_datenumber winds up being NULL.
 */
int load_mu_qual_datenumber();

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
short get_mu_qual_datenumber(int index);


// free any memory occupied by mu_idx_ratingset, which stores the datenumbers from the mu/all.dta file, and set mu_idx_ratingset to NULL.
void free_mu_idx_ratingset();

/* 
 * free any memory mu_idx_ratingset points to, 
 * malloc up some new memory  space, and fill it with mu_idx_ratingset_1byte.bin.
 * set mu_idx_ratingset to point to that memory. Note that this is on the heap.
 * If mu_idx_ratingset_1byte.bin does not exist, this generates it from mu/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_idx_ratingset winds up being NULL.
 */
int force_load_mu_idx_ratingset();

/*
 * If mu_idx_ratingset is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads mu_idx_ratingset_1byte.bin into that memory. If mu_idx_ratingset_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from mu/all.dta.
 * If mu_idx_ratingset is not NULL, this trusts that mu_idx_ratingset is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, mu_idx_ratingset winds up being NULL.
 */
int load_mu_idx_ratingset();

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
char get_mu_idx_ratingset(int index);


// free any memory occupied by um_all_usernumber, which stores the usernumbers from the um/all.dta file, and set um_all_usernumber to NULL.
void free_um_all_usernumber();

/* 
 * free any memory um_all_usernumber points to, 
 * malloc up some new memory  space, and fill it with um_all_usernumber_4bytes.bin.
 * set um_all_usernumber to point to that memory. Note that this is on the heap.
 * If um_all_usernumber_4bytes.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_usernumber winds up being NULL.
 */
int force_load_um_all_usernumber();

/*
 * If um_all_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_usernumber_4bytes.bin into that memory. If um_all_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_usernumber is not NULL, this trusts that um_all_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_usernumber winds up being NULL.
 */
int load_um_all_usernumber();

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
int get_um_all_usernumber(int index);


// free any memory occupied by um_all_movienumber, which stores the movienumbers from the um/all.dta file, and set um_all_movienumber to NULL.
void free_um_all_movienumber();

/* 
 * free any memory um_all_movienumber points to, 
 * malloc up some new memory  space, and fill it with um_all_movienumber_2bytes.bin.
 * set um_all_movienumber to point to that memory. Note that this is on the heap.
 * If um_all_movienumber_2bytes.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_movienumber winds up being NULL.
 */
int force_load_um_all_movienumber();

/*
 * If um_all_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_movienumber_2bytes.bin into that memory. If um_all_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_movienumber is not NULL, this trusts that um_all_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_movienumber winds up being NULL.
 */
int load_um_all_movienumber();

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
short get_um_all_movienumber(int index);

// free any memory occupied by um_all_datenumber, which stores the datenumbers from the um/all.dta file, and set um_all_datenumber to NULL.
void free_um_all_datenumber();

/* 
 * free any memory um_all_datenumber points to, 
 * malloc up some new memory  space, and fill it with um_all_datenumber_2bytes.bin.
 * set um_all_datenumber to point to that memory. Note that this is on the heap.
 * If um_all_datenumber_2bytes.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_datenumber winds up being NULL.
 */
int force_load_um_all_datenumber();

/*
 * If um_all_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_datenumber_2bytes.bin into that memory. If um_all_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_datenumber is not NULL, this trusts that um_all_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_datenumber winds up being NULL.
 */
int load_um_all_datenumber();

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
short get_um_all_datenumber(int index);

// free any memory occupied by um_all_rating, which stores the datenumbers from the um/all.dta file, and set um_all_rating to NULL.
void free_um_all_rating();

/* 
 * free any memory um_all_rating points to, 
 * malloc up some new memory  space, and fill it with um_all_rating_1byte.bin.
 * set um_all_rating to point to that memory. Note that this is on the heap.
 * If um_all_rating_1byte.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_rating winds up being NULL.
 */
int force_load_um_all_rating();

/*
 * If um_all_rating is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_all_rating_1byte.bin into that memory. If um_all_rating_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_all_rating is not NULL, this trusts that um_all_rating is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_all_rating winds up being NULL.
 */
int load_um_all_rating();

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
char get_um_all_rating(int index);

// free any memory occupied by um_qual_usernumber, which stores the usernumbers from the um/qual.dta file, and set um_qual_usernumber to NULL.
void free_um_qual_usernumber();

/* 
 * free any memory um_qual_usernumber points to, 
 * malloc up some new memory  space, and fill it with um_qual_usernumber_4bytes.bin.
 * set um_qual_usernumber to point to that memory. Note that this is on the heap.
 * If um_qual_usernumber_4bytes.bin does not exist, this generates it from um/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_usernumber winds up being NULL.
 */
int force_load_um_qual_usernumber();

/*
 * If um_qual_usernumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_usernumber_4bytes.bin into that memory. If um_qual_usernumber_4bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_usernumber is not NULL, this trusts that um_qual_usernumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_usernumber winds up being NULL.
 */
int load_um_qual_usernumber();

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
int get_um_qual_usernumber(int index);

// free any memory occupied by um_qual_movienumber, which stores the movienumbers from the um/qual.dta file, and set um_qual_movienumber to NULL.
void free_um_qual_movienumber();

/* 
 * free any memory um_qual_movienumber points to, 
 * malloc up some new memory  space, and fill it with um_qual_movienumber_2bytes.bin.
 * set um_qual_movienumber to point to that memory. Note that this is on the heap.
 * If um_qual_movienumber_2bytes.bin does not exist, this generates it from um/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_movienumber winds up being NULL.
 */
int force_load_um_qual_movienumber();

/*
 * If um_qual_movienumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_movienumber_2bytes.bin into that memory. If um_qual_movienumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_movienumber is not NULL, this trusts that um_qual_movienumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_movienumber winds up being NULL.
 */
int load_um_qual_movienumber();

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
short get_um_qual_movienumber(int index);

// free any memory occupied by um_qual_datenumber, which stores the datenumbers from the um/qual.dta file, and set um_qual_datenumber to NULL.
void free_um_qual_datenumber();

/* 
 * free any memory um_qual_datenumber points to, 
 * malloc up some new memory  space, and fill it with um_qual_datenumber_2bytes.bin.
 * set um_qual_datenumber to point to that memory. Note that this is on the heap.
 * If um_qual_datenumber_2bytes.bin does not exist, this generates it from um/qual.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_datenumber winds up being NULL.
 */
int force_load_um_qual_datenumber();

/*
 * If um_qual_datenumber is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_qual_datenumber_2bytes.bin into that memory. If um_qual_datenumber_2bytes.bin does
 * not exist, and this function wants to read it, it will generate it from um/qual.dta.
 * If um_qual_datenumber is not NULL, this trusts that um_qual_datenumber is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_qual_datenumber winds up being NULL.
 */
int load_um_qual_datenumber();

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
short get_um_qual_datenumber(int index);


// free any memory occupied by um_idx_ratingset, which stores the datenumbers from the um/all.dta file, and set um_idx_ratingset to NULL.
void free_um_idx_ratingset();

/* 
 * free any memory um_idx_ratingset points to, 
 * malloc up some new memory  space, and fill it with um_idx_ratingset_1byte.bin.
 * set um_idx_ratingset to point to that memory. Note that this is on the heap.
 * If um_idx_ratingset_1byte.bin does not exist, this generates it from um/all.dta.
 * 
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_idx_ratingset winds up being NULL.
 */
int force_load_um_idx_ratingset();

/*
 * If um_idx_ratingset is NULL, this mallocs up some memory for it (note that this is on the heap),
 * and then loads um_idx_ratingset_1byte.bin into that memory. If um_idx_ratingset_1byte.bin does
 * not exist, and this function wants to read it, it will generate it from um/all.dta.
 * If um_idx_ratingset is not NULL, this trusts that um_idx_ratingset is already correct. 
 *
 * returns 0 on success, -1 on failure (malloc failed, file wouldn't read, etc.)
 * in case of failure, um_idx_ratingset winds up being NULL.
 */
int load_um_idx_ratingset();

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
char get_um_idx_ratingset(int index);

#endif
