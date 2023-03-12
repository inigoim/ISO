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
    int fd_mytar;
    struct stat stat_mytar;

    
    char header_buffer[FILE_HEADER_SIZE];

    fd_mytar = open(f_mytar, O_RDONLY);
    
    //The tar file doesn't exist
    if (fd_mytar==-1) return E_OPEN;

    //The tar file doesn't have the correct format
    fstat(fd_mytar, &stat_mytar);
    if(stat_mytar.st_size % 10240 != 0)
    {
        close(fd_mytar);
        return E_TARFORM;
    }

    //f_dat doesn't exist in the tar file
    if (search_file(fd_mytar, f_dat) != 0){
        close(fd_mytar);
        return E_NOEXIST;
    }

    //f_dat exists in the tar file
    if (extract_file(fd_mytar,  (struct c_header_gnu_tar *) header_buffer) == 0){
        // header_buffer doesnt contain anything???
        close (fd_mytar);
        return 0;
    }

    //another type of errors
    else {
        close(fd_mytar);
        return E_DESCO;
    }
   
}
