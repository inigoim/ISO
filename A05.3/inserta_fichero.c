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
unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_TarFile);
unsigned long WriteEndTarArchive( int fd_TarFile);
unsigned long WriteCompleteTarSize( unsigned long TarActualSize,  int fd_TarFile);

int seek_last_header(int fd_mytar, char * header_buffer);

int inserta_fichero(char * f_mytar, char * f_dat)
{
    int fd_mytar, fd_dat;
    struct c_header_gnu_tar tar_header;
    struct stat stat_mytar;
    
    memset(&tar_header, 0, sizeof(tar_header));
    memset(&stat_mytar, 0, sizeof(stat_mytar));


    // Open the files
    if ((fd_dat = open(f_dat, O_RDONLY)) == -1)
        return E_OPEN1;
    if ((fd_mytar = open(f_mytar, (O_RDWR | O_CREAT), 0600)) == -1)
        return E_OPEN2;

    // Build the data_file header
    if (BuilTarHeader(f_dat, &tar_header) != HEADER_OK) return E_OPEN1;

    //TODO: What if the file is empty?


    // Write the header and data
    seek_last_header(fd_mytar);
    if (write(fd_mytar, &tar_header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
        return E_DESCO;
    if (WriteFileDataBlocks(fd_dat, fd_mytar) < 0)
        return E_DESCO;

    // Write the end of the tar file
    WriteEndTarArchive(fd_mytar);
    fstat(fd_mytar, &stat_mytar);
    WriteCompleteTarSize(stat_mytar.st_size, fd_mytar);

    // Close the files
    close(fd_dat);
    close(fd_mytar);

    return 0;
}


// Go to the last header of the tar file
int seek_last_header(int fd_mytar) {
    char header_buffer[FILE_HEADER_SIZE];
    memset(header_buffer, 0, DATAFILE_BLOCK_SIZE);
    while (1)
    {
        // Read the header
        if (read(fd_mytar, header_buffer, DATAFILE_BLOCK_SIZE)
            != DATAFILE_BLOCK_SIZE)
            return E_TARFORM;

        // Check the size field of the header
        struct c_header_gnu_tar * header = (struct c_header_gnu_tar *) header_buffer;

        // TODO: Check if the header is valid
        int file_size = strtol(header->size, NULL, 8);
        if (file_size == 0) break;

        // Advance to the next header
        int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
        lseek(fd_mytar, offset, SEEK_CUR);
    }
    lseek(fd_mytar, -DATAFILE_BLOCK_SIZE, SEEK_CUR);

    return 0;
}