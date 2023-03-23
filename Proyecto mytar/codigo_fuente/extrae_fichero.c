/**
 * @file extrae_fichero.c
 * @authors XG.C., E.F, I.I.
 * @brief Extracts the specified file from a tar file
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include "s_mytarheader.h"
#include "mytar_utils.h"


/**
* Extracts the specified file from a tar file
* @param fd_mytar File descriptor of the tar file
* @param f_dat Name of the file to extract
* @return 0 if the file was extracted successfully, an error code otherwise  
*/
int extrae_fichero(const char * f_mytar, const char * f_dat) {
    int fd_mytar, res;
    struct stat stat_mytar;
    char header_buffer[FILE_HEADER_SIZE];


    fd_mytar = open(f_mytar, O_RDONLY);
    
    // If there is an error opening the tar file, throw an error
    if (fd_mytar == -1) return E_OPEN;

    // Check the tar file format
    fstat(fd_mytar, &stat_mytar);
    if(stat_mytar.st_size % 10240 != 0) {
        close(fd_mytar);
        return E_TARFORM;
    }

    // Search f_dat in the tar file
    res = search_file(fd_mytar, f_dat);
    if (res < 0) {
        close(fd_mytar);
        return res;
    }

    // Extract the file from the tar
    res = tar_extract_file(fd_mytar);
    if (res < 0) {
        close (fd_mytar);
        return res;
    }

    close(fd_mytar);
    return 0;

}
