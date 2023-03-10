// Writes the data file with its header to the tar file
int tar_insert_file (int fd_mytar, char *f_dat);

// Writes the end of the tar file
// (2x512 bytes with zeros and complete to  multiple of 10KB size blocks)
int tar_complete_archive(int fd_mytar);