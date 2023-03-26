/**
 * @file mytar_utils.c
 * @authors G.A., XG.C., E.F., I.I.
 * @brief Functions used by mytar programs
 * @details Contains many functions used for tar archive operations
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
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "s_mytarheader.h"


// Index of the last processed file in the tar (read or written)
int file_number = 0;


// Return UserName (string) from uid (integer). See man 2 stat and man getpwuid
char * getUserName(uid_t uid) {
   return getpwuid(uid)->pw_name;
}

// Return GroupName (string) from gid (integer). See man 2 stat and man getgrgid   
char * getGroupName(gid_t gid) {
   return getgrgid(gid)->gr_name;
}

// Return Mode type entry  (tar Mode type)
char mode_tar(mode_t Mode) {
   if  (S_ISREG(Mode))   return  '0';
   if  (S_ISLNK(Mode))   return  '2';
   if  (S_ISCHR(Mode))   return  '3';
   if  (S_ISBLK(Mode))   return  '4';
   if  (S_ISDIR(Mode))   return  '5';
   if  (S_ISFIFO(Mode))  return  '6';
   if  (S_ISSOCK(Mode))  return  '7';
   return '0';
}

// ------------------------------------------------------------------------- //

/**
* Builds GNU Tar header structure with FileName stat info (See man 2 stat)
* @param FileName File from which to build the header
* @param pTarHeader Pointer to the header to be filled
* @return HEADER_OK if successful, HEADER_ERR otherwise
*/
int BuilTarHeader(const char *FileName, struct c_header_gnu_tar *pTarHeader) {
   struct stat  stat_file;
   int i;
   char  *pTarHeaderBytes;
   unsigned int Checksum;

   memset(pTarHeader, 0, sizeof(struct c_header_gnu_tar));

   if (lstat(FileName, &stat_file) == -1)  return HEADER_ERR;

   strcpy(pTarHeader->name, FileName);
   sprintf(pTarHeader->mode, "%07o", stat_file.st_mode & 07777);  // Only  the least significant 12 bits
   printf("st_mode del archivo %s %07o\n",FileName, stat_file.st_mode & 07777); // Only  the least significant 12 bits
   sprintf(pTarHeader->uid, "%07o", stat_file.st_uid);
   sprintf(pTarHeader->gid, "%07o", stat_file.st_gid);
   sprintf(pTarHeader->mtime, "%011lo", stat_file.st_mtime);

   pTarHeader->typeflag[0] = mode_tar(stat_file.st_mode);
   if (pTarHeader->typeflag[0] == '5')
      sprintf(pTarHeader->size, "%011lo", 0);
   else
      sprintf(pTarHeader->size, "%011lo", stat_file.st_size);

    //  linkname
   if (S_ISLNK(stat_file.st_mode))
      readlink(FileName, pTarHeader->linkname, 100);

   memcpy(pTarHeader->magic, "ustar ", 6);  // "ustar" followed by a space (without null char)
   strcpy(pTarHeader->version, " ");  //   space character followed by a null char.
   strcpy(pTarHeader->uname, getUserName(stat_file.st_uid));
   strcpy(pTarHeader->gname, getGroupName(stat_file.st_gid));
     //  devmayor (not used)
     //  devminor (not used)
   sprintf(pTarHeader->atime, "%011lo", stat_file.st_atime);
   sprintf(pTarHeader->ctime, "%011lo", stat_file.st_ctime);
     //  offset (not used)
     //  longnames (not used)
     //  unused (not used)
     //  sparse (not used)
     //  isextended (not used)
     //  realsize (not used)
     //  pad (not used)

   // compute checksum (the last)
   memset(pTarHeader->checksum, ' ', sizeof(pTarHeader->checksum));   // Initialize to blanc spaces
   pTarHeaderBytes = (unsigned char *) pTarHeader;

   for (i=0, Checksum=0 ; i < sizeof(struct c_header_gnu_tar); i++) {
      Checksum += pTarHeaderBytes[i];
   }

   sprintf(pTarHeader->checksum, "%06o", Checksum);    // six octal digits followed by a null and a space character

   return HEADER_OK;
}

/**
 * Write the data file to the tar file
 * @param fd_DataFile File descriptor of the data file to read from
 * @param fd_TarFile File descriptor of the tar file to write to
 * @return Number of bytes written
*/
unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_TarFile) {
   char FileDataBlock[DATAFILE_BLOCK_SIZE];
   unsigned long NumWriteBytes;
   int n;

   // write the data file (blocks of 512 bytes)
   NumWriteBytes = 0;
   memset(FileDataBlock, 0, sizeof(FileDataBlock));
   printf("Datos Escritos: ");   // Traza
   while((n=read(fd_DataFile, FileDataBlock, sizeof(FileDataBlock))) > 0) {
       NumWriteBytes += write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));  // ojo!!!, no se escriben n, ni sizeof(FileDataBlock)
       memset(FileDataBlock, 0, sizeof(FileDataBlock));
       printf("---%d ", n);
   }
   
   printf("\nTotal: Escritos %ld bytes\n", NumWriteBytes); // Traza
   return NumWriteBytes;
}


/**
 * Write end tar archive entry (2x512 bytes with zeros)
 * @param fd_TarFile File descriptor of the tar file to write to
 * @return Number of bytes written
 */
unsigned long WriteEndTarArchive(int fd_TarFile) {
   char FileDataBlock[DATAFILE_BLOCK_SIZE];
   int n;
   unsigned long NumWriteBytes;

   memset(FileDataBlock, 0, sizeof(FileDataBlock));

   NumWriteBytes = 0;
   while (NumWriteBytes < END_TAR_ARCHIVE_ENTRY_SIZE) {
      n = write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));
      if (n < 0) return E_DESCO;
      NumWriteBytes += n;
   }

   printf("Escritos (End block) %ld bytes\n", NumWriteBytes); // Traza
   return NumWriteBytes;
}

/**
 * complete Tar file to  multiple of 10KB size block
 * @param TarCurrentSize Tar file size in bytes
 * @param fd_TarFile Tar file descriptor
 * @return Number of bytes written 
 */
unsigned long WriteCompleteTarSize(unsigned long TarCurrentSize,  int fd_TarFile) {
   unsigned long NumWriteBytes;
   unsigned long module, offset;
   
   printf("TAR_FILE_BLOCK_SIZE=%ld  TarFileSize=%ld\n", TAR_FILE_BLOCK_SIZE, TarCurrentSize); // Traza
   // complete to  multiple of 10KB size blocks
   NumWriteBytes = 0;
   module = TarCurrentSize % 10240;
   offset = 10240 - module;
   if(module != 0) {
    char padding[offset];
    memset(padding, 0, sizeof(padding));
    NumWriteBytes = write(fd_TarFile, padding, sizeof(padding));
   }
   printf("OK: Generado el EndTarBlocks del archivo tar %ld bytes\n", NumWriteBytes); // Traza

   return NumWriteBytes;
}


//===========================================================================//
//===========================================================================//


/**
* Checks if a header is empty (all bytes are 0)
* @param header pointer to the header to check
* @return true if the header is empty, false otherwise
*/
bool is_empty (struct c_header_gnu_tar header) {
   char *header_bytes = (char *) &header;
   int i;
   for (i = 0; i < sizeof(struct c_header_gnu_tar); i++) {
      if (header_bytes[i] != 0) return false;
   }
   return true;
}

/** 
 * Moves the file descriptor offset to the block
 * after the last file in the tar archive
 * @param fd_mytar File descriptor of the tar archive
 * @return The index of the last file in the archive if successful, an error code otherwise
 */
int seek_end_of_files(int fd_mytar) {
   struct c_header_gnu_tar header;


   while (1)
   {
      // Read the header
      int read_size = read(fd_mytar, &header, DATAFILE_BLOCK_SIZE);
      if (read_size == -1) return E_OPEN2;
      if (read_size != DATAFILE_BLOCK_SIZE) return E_TARFORM;
      
      // Check if the header is valid
      if (memcmp(header.magic, "ustar ", 6) != 0) {
         if (is_empty(header)) {
            lseek (fd_mytar, -DATAFILE_BLOCK_SIZE, SEEK_CUR);
            break; // This is where the new file should go
         }
         else return E_TARFORM;
      }

      // Get the size of the file and advance to the next header
      int file_size = strtol(header.size, NULL, 8);
      if (file_size != 0) {
      int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
      lseek(fd_mytar, offset, SEEK_CUR);
      }

      file_number++;
   }
   
   return file_number;
}

/** 
 * Searches for a file in the tar archive, and leaves the file offset at the beginning of the header
 * @param fd_mytar File descriptor of the tar archive
 * @param f_dat Name of the file to search
 * @return The index of the file before the searched file if the file was found, an error code otherwise
 */
int search_file (int fd_mytar, const char * f_dat) {

   struct c_header_gnu_tar header;

   while (1)
   {
      // Read the header
      int read_size = read(fd_mytar, &header, FILE_HEADER_SIZE);
      if (read_size == -1) return E_DESCO;
      if (read_size != DATAFILE_BLOCK_SIZE) return E_TARFORM;

      // If this is not a tar file header
      if (memcmp(header.magic, "ustar ", 6) != 0)
      {
         if (!is_empty(header)) return E_TARFORM; // Invalid tar file
         return E_NOEXIST; // End of the tar archive
      }

      // Check if the header has the name of the file to extract
      if (strcmp(header.name, f_dat) == 0)
      {
         lseek(fd_mytar, -FILE_HEADER_SIZE, SEEK_CUR);
         break;
      }

      // Get the size of the file and advance to the next header
      int file_size = strtol(header.size, NULL, 8);
      if (file_size != 0) {
      int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
      lseek(fd_mytar, offset, SEEK_CUR);
      }

      file_number++;
   }
   
   return file_number;
}

// ------------------------------------------------------------------------- //

/**
* Completes the tar archive by writing the end of archive entry and filling up to 10KB multiple
* @param fd_mytar File descriptor of the tar archive, offset must be at the end of the last file
* @return 0 if the archive was completed successfully, an error code otherwise
*/
int tar_complete_archive (int fd_mytar) {
   struct stat stat_mytar;
   if (WriteEndTarArchive(fd_mytar) < 0) return E_DESCO;
   if (fstat(fd_mytar, &stat_mytar) == -1) return E_DESCO;
   if (WriteCompleteTarSize(stat_mytar.st_size, fd_mytar) == -1) return E_DESCO;
   return 0;
}

int tar_insert_contents(int fd_mytar, const char *f_dir);

/**
* Inserts a file in the tar archive (current offset)
* @param fd_mytar File descriptor of the tar archive
* @param f_dat File to insert in the tar archive
* @return The index of the inserted file in the archive if successful, an error code otherwise
*/
int tar_insert_file (int fd_mytar, const char *f_dat) {
   struct c_header_gnu_tar tar_header;
   int res;
   if (BuilTarHeader(f_dat, &tar_header) != HEADER_OK) return E_OPEN1;
   
   // Write the header and the data
   if (write(fd_mytar, &tar_header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
      return E_DESCO;
   int fd_dat = open(f_dat, O_RDONLY);

   // Write the file data
   switch (tar_header.typeflag[0]) {
   case '0': // Regular file
      if (WriteFileDataBlocks(fd_dat, fd_mytar) < 0)
         return E_DESCO;
      break;
   
   case '2': // Symbolic link
      // Nothing to do
      break;

   case '5': // Directory
      res = tar_insert_contents(fd_mytar, f_dat);
      if (res < 0) return res;
      break;

   default:
      fprintf(stderr, "tar_insert_file error: Unhandled file type\n");
      break;
   }
   
   file_number++;
   
   close(fd_dat);
   return file_number;
}

/**
 * Inserts all the files found inside the directory into the tar archive
 * @param fd_mytar File descriptor of the tar archive, the files will be inserted after the current offset
 * @param f_dir Directory from which to read files
 * @return 0 if the files were inserted successfully, an error code otherwise
 */
int tar_insert_contents(int fd_mytar, const char *f_dir) {
   DIR *dirp;
   struct dirent *diren;
   char filepath[100];

   dirp = opendir(f_dir);
   while ((diren = readdir(dirp)) != NULL) {
      if (strcmp(diren->d_name, ".") == 0 || strcmp(diren->d_name, "..") == 0)
         continue;

      if (strlen(diren->d_name) + 1 + strlen(f_dir) >= sizeof(filepath))
         return E_DESCO;

      if (f_dir[strlen(f_dir) - 1] == '/')
         sprintf(filepath, "%s%s", f_dir, diren->d_name);
      else
         sprintf(filepath, "%s/%s", f_dir, diren->d_name);
      
      int res = tar_insert_file(fd_mytar, filepath);
      if (res < 0) return res;
   }

   closedir(dirp);
   return 0;
}


/**
 * Makes the required directories to extract a file
 * @param f_dat Path of the file that will be extracted
 * @return 0 if the directories were created successfully, an error code otherwise
*/
int create_required_path(const char *f_dat) {
   char *path, *sep;
   path = strdup(f_dat);
   sep = strchr(path, '/');

   while (sep != NULL) {
      int current_length = sep - path;
      char current[current_length + 1];
      memcpy(current, path, current_length);
      current[current_length] = '\0';

      if (access(current, (R_OK | W_OK | X_OK)) == -1) {
         switch (errno) {
         case ENOENT:
            if (mkdir(current, 0700) == -1) return E_CREATDEST;
            break;

         case EACCES:
            return E_CREATDEST;
            break;
         
         default:
            return E_DESCO;
            break;
         }
      }
      
      sep = strchr(sep + 1, '/');
   }


   free(path);
   return 0;
}
/**
 * Helper function for tar_extract_file, it handles regular files
 * @param fd_mytar File descriptor of the tar archive
 * @param pheader Pointer to the header of the file to extract
 * @return 0 if the file was extracted successfully, an error code otherwise
 */
int tar_extract_regular_file(int fd_mytar, struct c_header_gnu_tar *pheader) {
   int fd_output = creat(pheader->name, 0600);
   if (fd_output == -1) {
      close(fd_output);
      return E_CREATDEST;  
   } 
   
   int file_size = strtol(pheader->size, NULL, 8);
   char data_buff[file_size];
   int rd = read(fd_mytar, data_buff, file_size);
   if (rd == -1) {
      close(fd_output);
      return E_DESCO;
   }
   if (rd < file_size) {
      close(fd_output);
      return E_TARFORM;
   }
      
   if(write(fd_output, data_buff, file_size) < file_size) { 
      close(fd_output);
      return E_DESCO;
   }

   close(fd_output);
   return 0;
}

/**
 * Helper function for tar_extract_file, it handles directories
 * @param fd_mytar File descriptor of the tar archive
 * @param pheader Pointer to the header of the directory to extract
 * @return 0 if the directory was created successfully, an error code otherwise
 */
int tar_extract_directory(int fd_mytar, struct c_header_gnu_tar *pheader) {
   int res = mkdir(pheader->name, 0700);
   if (res == -1 && errno != EEXIST) return E_CREATDEST;

   return 0;
}

/**
* Extract the file from a tar archive
* (the file descriptor offset has to be at the beginning of the file header)
* @param fd_mytar file descriptor of the tar archive
* @return 0 if successful, an error code otherwise
*/
int tar_extract_file(int fd_mytar) {
   struct c_header_gnu_tar header;
   int res;

   
   if (read(fd_mytar, &header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
      return E_DESCO;
   
   res = create_required_path(header.name);
   if (res < 0) return res;
   
   switch (header.typeflag[0]) {
   case '0': // Regular file
      res = tar_extract_regular_file(fd_mytar, &header);
      if (res < 0) return res;
      break;

   case '2':
      if (symlink(header.linkname, header.name) == -1) return E_CREATDEST;
      break;

   case '5': // Directory
      res = tar_extract_directory(fd_mytar, &header);
      if (res < 0) return res;
      break;

   default:
      fprintf(stderr, "tar_extract_file Error: Unhandled file type\n");
      break;
   }

   file_number++;
   
   return 0;
}
