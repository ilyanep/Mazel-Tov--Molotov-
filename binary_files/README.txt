This is a library to create, load, and use binary data files. 

It is important to set the definitions in binary_files.h to be the directories in which you actually want things to be. Currently, it expects this library to be in the binary_files directory, which contains the data_files directory (if it doesn't yet, this library makes one) and is a subdirectory in the same place as the mu and um directories. 

When reading or writing data files, this will change to the data file directory, but then it will change back. it will get lost if it is not in some subdirectory within the project directory folder. 

binary_files.cpp contains a variety of useful functions, and sets several useful global variables. most notably:

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

Each of these global variables can be freed (set to NULL, and the memory the array had occupied released) with the free_<variable>() function.
Each of these global variables can be loaded with the load_<variable>() function, which will create a binary data file from the ascii files if one does not yet exist. 
Each of these global variables can be force loaded (reloaded even if it's already non-null) with the force_load_<variable>() function.
You can access any entry of these arrays with the get_<variable>(int index) function, which will load the array if it's not yet loaded, and will create a binary data file from the ascii files if one does not yet exist. 

Note that all of these functions (except free_…) return -1 if something goes wrong, and the load functions return 0 if things go well. 

This library writes data files, in the form of binary arrays as outlined above, to the data_files directory, which git does not back up. To create these, it reads from the mu and um directory files. 

To generate all of the binary files, and see the first 20 lines of all of them, run the generate_data_files program. 

for more details on the data_files functions, see the comments in data_files.h


-Isaac Sheff

P.S. yes, I realize that the way it changes directories all the time may be kind of janky, but I'm new to this much system io crap in low-level languages. 
