#include <iostream>
#include <fstream>
using namespace std;
#include"binary_files.h"

int main (int argc, char **argv)
{
    cout << "Now generating all of the binary files . . . \n";
    write_data_files_to_bin();
    int i;
    printf("UM:\nuser\tmovie\tdate\trating\tset\tqualU\tqualM\tqualD\n");
    for (i=0;i<20;i++)
    {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", 
               get_um_all_usernumber(i), 
               get_um_all_movienumber(i), 
               get_um_all_datenumber(i), 
               get_um_all_rating(i), 
               get_um_idx_ratingset(i), 
               get_um_qual_usernumber(i), 
               get_um_qual_movienumber(i), 
               get_um_qual_datenumber(i));
    }
    printf("\n\n\nMU:\nuser\tmovie\tdate\trating\tset\tqualU\tqualM\tqualD\n");
    for (i=0;i<20;i++)
    {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", 
               get_mu_all_usernumber(i), 
               get_mu_all_movienumber(i), 
               get_mu_all_datenumber(i), 
               get_mu_all_rating(i), 
               get_mu_idx_ratingset(i), 
               get_mu_qual_usernumber(i), 
               get_mu_qual_movienumber(i), 
               get_mu_qual_datenumber(i));
    }
    printf("\n\n\n");
    return 0;
}
