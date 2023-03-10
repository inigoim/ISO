// Writes the data file with its header to the tar file
int tar_insert_file (int fd_mytar, char *f_dat);

// Writes the end of the tar file
// (2x512 bytes with zeros and complete to  multiple of 10KB size blocks)
int tar_complete_archive(int fd_mytar);

/**
 * @brief Seek the end of the files in the tar file
 * @param fd_mytar The file descriptor of the tar file
 * @return the number of the file to be inserted if everything went well,
            E_TARFORM if the tar file is not well formed
*/
int seek_end_of_files(int fd_mytar);