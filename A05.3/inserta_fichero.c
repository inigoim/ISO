/**
 * @file insertar_fichero.c
 * @author XG.C., E.F, .I.I.
 * @date 01/03/2023
 * @brief First version of inserta_fichero
 * @details  Insert a dat file in tar file
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


int inserta_fichero(char * f_mytar, char * f_dat)
{
    int fd_mytar, file_number, res;
    struct stat stat_mytar;

    // Open the tar file
    if ((fd_mytar = open(f_mytar, (O_RDWR | O_CREAT), 0600)) == -1) {
        close(fd_mytar);
        return E_OPEN2;
    }
        

    // Check that the size of the tar file is multiple of 10KB
    fstat(fd_mytar, &stat_mytar);
    if (stat_mytar.st_size % 10240 != 0) {
        close(fd_mytar);
        return E_TARFORM;
    } 

    // Seek the postion where the next file should be inserted
    if (stat_mytar.st_size != 0) {
        file_number = seek_end_of_files(fd_mytar);
        if (file_number < 0) {
            close(fd_mytar);
            return file_number;
        }
    }
    else
        file_number = 1;

    // Insert the file and complete the archive
    res = tar_insert_file(fd_mytar, f_dat);
    if (res < 0) {
        close(fd_mytar);
        return res;
    }
    res = tar_complete_archive(fd_mytar);
    if (res < 0) {
        close(fd_mytar);
        return res;
    }

    close(fd_mytar);
    return file_number;
}
