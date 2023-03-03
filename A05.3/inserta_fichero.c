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
    int fd_mytar, fd_dat, file_number;
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

    file_number = seek_end_of_files(fd_mytar);
    if (file_number < 0) return file_number; // Error (E_TARFORM)

    // Write the header and the data
    if (write(fd_mytar, &tar_header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
        return E_DESCO;
    if (WriteFileDataBlocks(fd_dat, fd_mytar) < 0)
        return E_DESCO;

    // Write the end of the tar file
    WriteEndTarArchive(fd_mytar);
    fstat(fd_mytar, &stat_mytar); // Equivalent to stat(f_mytar, &stat_mytar)
    WriteCompleteTarSize(stat_mytar.st_size, fd_mytar);

    // Close the files
    close(fd_dat);
    close(fd_mytar);

    return file_number;
}


/**
 * @brief Seek the end of the files in the tar file
 * @param fd_mytar The file descriptor of the tar file
 * @return the number of the file to be inserted if everything went well,
            E_TARFORM if the tar file is not well formed
*/
int seek_end_of_files(int fd_mytar) {
    int file_number = 0;
    char header_buffer[FILE_HEADER_SIZE];
    memset(header_buffer, 0, DATAFILE_BLOCK_SIZE);
    while (1)
    {
        // Read the header
        int read_size = read(fd_mytar, header_buffer, DATAFILE_BLOCK_SIZE);
        if (read_size == 0) break; // In case the file is empty
        else if (read_size != DATAFILE_BLOCK_SIZE) return E_TARFORM;

        // Read the size from the header
        struct c_header_gnu_tar * header = (struct c_header_gnu_tar *) header_buffer;
        int file_size = strtol(header->size, NULL, 8);

        // If the size is 0, this is probably where the new header should go
        if (file_size == 0) {
            lseek(fd_mytar, -DATAFILE_BLOCK_SIZE, SEEK_CUR);
            break;
        }
        file_number++;

        // Advance to the next header
        int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
        lseek(fd_mytar, offset, SEEK_CUR);
    }

    return file_number;
}