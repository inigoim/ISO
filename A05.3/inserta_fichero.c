/**
 * @file insertar_fichero.c
 * @author I.I., XG.C., E.F.
 * @date 24/02/2023
 * @brief First version of inserta_fichero
 * @details  Insert a tar file in dat file
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

int BuilTarHeader(char *FileName, struct c_header_gnu_tar * pTarHeader);

int inserta_fichero(char * f_mytar, char * f_dat)
{
    int ret, fd_mytar, fd_dat;
    struct c_header_gnu_tar tar_header;
    struct stat stat_mytar;
    char header_buffer[FILE_HEADER_SIZE];


    // Open the files
    if ((fd_dat=open(f_dat, O_RDONLY)) == -1) return E_OPEN1;
    if ((fd_mytar=open(f_mytar, (O_RDWR | O_CREAT), 0600)) == -1) return E_OPEN2;

    // Build the data_file header
    memset(&tar_header, 0, sizeof(tar_header)); // Is this necessary?
    ret = BuilTarHeader(f_dat, &tar_header);
    if (ret != HEADER_OK) return E_OPEN1;

    // Get the size of the tar file
    memset(&stat_mytar, 0, sizeof(stat_mytar)); // Is this necessary?
    fstat(fd_mytar, &stat_mytar);

    //TODO: What if the file doesn't exist?

    memset(header_buffer, 0, DATAFILE_BLOCK_SIZE);
    int file_size;
    do
    {
        // Read the header
        read(fd_mytar, header_buffer, DATAFILE_BLOCK_SIZE);

        // Check the size field of the header
        struct c_header_gnu_tar * header = (struct c_header_gnu_tar *) header_buffer;
        file_size = strtol(header->size, NULL, 8);
        printf("size field: \"%s\"\n", header->size);
        printf("Actual size: \"%d\"\n", file_size);

        // Advance to the next header
        int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
        lseek(fd_mytar, offset, SEEK_CUR);

    } while (file_size != 0); //FIXME: This is not the best way to check if it's the last header

    lseek(fd_mytar, -DATAFILE_BLOCK_SIZE, SEEK_CUR);
}
