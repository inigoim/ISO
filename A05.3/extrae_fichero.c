/**
 * @file extrae_fichero.c
 * @author XG.C., E.F, .I.I.
 * @date 01/03/2023
 * @brief First version of extrae_fichero
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

int extrae_fichero(char * f_mytar, char * f_dat) {
    int fd_mytar, res;
    struct stat stat_mytar;
    char header_buffer[FILE_HEADER_SIZE];


    fd_mytar = open(f_mytar, O_RDONLY);
    
    // If there is an error opening the tar file, throw an error
    if (fd_mytar == -1) return E_OPEN;

    // Check the tar file format
    fstat(fd_mytar, &stat_mytar);
    if(stat_mytar.st_size % 10240 != 0)
    {
        close(fd_mytar);
        return E_TARFORM;
    }

    // Search f_dat in the tar file
    res = search_file(fd_mytar, f_dat);
    if (res < 0)
    {
        close(fd_mytar);
        return res;
    }

    // Extract the file from the tar
    res = extract_file(fd_mytar);
    if (res < 0)
    {
        close (fd_mytar);
        return res;
    }

    close(fd_mytar);
    return 0;

}
