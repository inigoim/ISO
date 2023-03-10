/**
 * @file insertar_fichero.c
 * @author XG.C., E.F, .I.I.
 * @date 01/03/2023
 * @brief First version of inserta_fichero
 * @details  Insert a dat file in tar file
*/

#define E_OPEN1 -1
#define E_OPEN2 -2
#define E_TARFORM -3
#define E_DESCO -99

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


int seek_end_of_files(int fd_mytar);

int inserta_fichero(char * f_mytar, char * f_dat)
{
    int fd_mytar, file_number;

    // Open the tar file
    if ((fd_mytar = open(f_mytar, (O_RDWR | O_CREAT), 0600)) == -1)
        return E_OPEN2;

    file_number = seek_end_of_files(fd_mytar);
    if (file_number < 0) return file_number; // Error (E_TARFORM)

    tar_insert_file(fd_mytar, f_dat);

    tar_complete_archive(fd_mytar);

    // Close the file
    close(fd_mytar);

    return file_number;
}
