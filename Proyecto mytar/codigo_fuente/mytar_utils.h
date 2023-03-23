/**
 * @file mytar_utils.h
 * @brief Header file for mytar_utils.c
 * @details Contains the higher level functions of mytar_utils
 */


int seek_end_of_files(int fd_mytar);

int search_file (int fd_mytar, const char * f_dat);

int tar_complete_archive (int fd_mytar);

int tar_insert_file (int fd_mytar, const char *f_dat);

int tar_extract_file(int fd_mytar);