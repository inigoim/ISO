/**
 * @file mytar_utils.c
 * @author G.A.
 * @date 10/02/2023
 * @brief Functions used by mytar programs
 * @details Contains many functions used for tar file generation
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

#include "s_mytarheader.h"



char FileDataBlock[DATAFILE_BLOCK_SIZE];

//-----------------------------------------------------------------------------
// Return UserName (string) from uid (integer). See man 2 stat and man getpwuid
char * getUserName(uid_t uid)
{
   return getpwuid(uid)->pw_name;
}

//------------------------------------------------------------------------------
// Return GroupName (string) from gid (integer). See man 2 stat and man getgrgid   
char * getGroupName(gid_t gid)
{
   return getgrgid(gid)->gr_name;
}

//----------------------------------------
// Return Mode type entry  (tar Mode type)
char mode_tar( mode_t Mode)
{
   if  (S_ISREG(Mode))   return  '0';
   if  (S_ISLNK(Mode))   return  '2';
   if  (S_ISCHR(Mode))   return  '3';
   if  (S_ISBLK(Mode))   return  '4';
   if  (S_ISDIR(Mode))   return  '5';
   if  (S_ISFIFO(Mode))  return  '6';
   if  (S_ISSOCK(Mode))  return  '7';
   return '0';
}

/**
* Build my_tardat structure with FileName stat info (See man 2 stat)
* @param FileName File name to be added to the tar file
* @param pTarHeader Pointer to the structure to be filled
* @return HEADER_OK if OK, HEADER_ERR if error
*/
int BuilTarHeader(char *FileName, struct c_header_gnu_tar *pTarHeader)
{
   struct stat  stat_file;

   int i;
   char  *pTarHeaderBytes;
   unsigned int  Checksum;

   memset(pTarHeader, 0, sizeof(struct c_header_gnu_tar));

   if (stat (FileName, &stat_file) == -1)  return HEADER_ERR;

   strcpy(pTarHeader->name, FileName);
   sprintf(pTarHeader->mode, "%07o", stat_file.st_mode & 07777);  // Only  the least significant 12 bits
   printf("st_mode del archivo %s %07o\n",FileName, stat_file.st_mode & 07777); // Only  the least significant 12 bits
   sprintf(pTarHeader->uid, "%07o", stat_file.st_uid);
   sprintf(pTarHeader->gid, "%07o", stat_file.st_gid);
   sprintf(pTarHeader->size, "%011lo", stat_file.st_size);
   sprintf(pTarHeader->mtime, "%011lo", stat_file.st_mtime);

   pTarHeader->typeflag[0] = mode_tar(stat_file.st_mode);

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

   for (i=0, Checksum=0 ; i < sizeof(struct c_header_gnu_tar); i++)
          Checksum += pTarHeaderBytes[i];

   sprintf(pTarHeader->checksum, "%06o", Checksum);    // six octal digits followed by a null and a space character

   return HEADER_OK;
}


// ----------------------------------------------------------------
// (1.2) write the data file (blocks of 512 bytes)
unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_TarFile)
{
   unsigned long NumWriteBytes;
   int n;

   // write the data file (blocks of 512 bytes)
   NumWriteBytes = 0;
   memset(FileDataBlock, 0, sizeof(FileDataBlock));
   printf("Datos Escritos: ");   // Traza
   while((n=read(fd_DataFile, FileDataBlock, sizeof(FileDataBlock))) > 0)
   {
       NumWriteBytes += write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));  // ojo!!!, no se escriben n, ni sizeof(FileDataBlock)
       memset(FileDataBlock, 0, sizeof(FileDataBlock));
       printf("---%d ", n);
   }
   
   printf("\nTotal: Escritos %ld bytes\n", NumWriteBytes); // Traza
   return NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.1)Write end tar archive entry (2x512 bytes with zeros) 

unsigned long WriteEndTarArchive( int fd_TarFile)
{
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
 * @return unsigned long Number of bytes written 
 */
unsigned long WriteCompleteTarSize( unsigned long TarCurrentSize,  int fd_TarFile)
{
   unsigned long NumWriteBytes;
   unsigned long module, offset;
   
   printf("TAR_FILE_BLOCK_SIZE=%ld  TarFileSize=%ld\n", TAR_FILE_BLOCK_SIZE, TarCurrentSize); // Traza
   // complete to  multiple of 10KB size blocks
   NumWriteBytes = 0;
   module = TarCurrentSize % 10240;
   offset = 10240 - module;
   if(module != 0){
    char padding[offset];
    memset(padding, 0, sizeof(padding));
    NumWriteBytes = write(fd_TarFile, padding, sizeof(padding));
   }
   printf("OK: Generado el EndTarBlocks del archivo tar %ld bytes\n", NumWriteBytes); // Traza

   return NumWriteBytes;
}


/**
* Inserts a file in the tar archive
* @param fd_mytar File descriptor of the tar archive
* @param f_dat File to insert in the tar archive
* @return 0 if the file was inserted successfully, an error code otherwise
*/
int tar_insert_file (int fd_mytar, char *f_dat) {
   struct c_header_gnu_tar tar_header;
   if (BuilTarHeader(f_dat, &tar_header) != HEADER_OK) return E_OPEN1;
   
   // Write the header and the data
   if (write(fd_mytar, &tar_header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
      return E_DESCO;
   int fd_dat = open(f_dat, O_RDONLY);
   if (WriteFileDataBlocks(fd_dat, fd_mytar) < 0)
      return E_DESCO;
   close(fd_dat);
}

/**
* Completes the tar archive by writing the end of archive entry and the size of the archive
* @param fd_mytar File descriptor of the tar archive
* @return 0 if the archive was completed successfully, an error code otherwise
*/
int tar_complete_archive (int fd_mytar) {
   struct stat stat_mytar;
   if (WriteEndTarArchive(fd_mytar) < 0) return E_DESCO;
   if (fstat(fd_mytar, &stat_mytar) == -1) return E_DESCO;
   if (WriteCompleteTarSize(stat_mytar.st_size, fd_mytar) == -1) return E_DESCO;
   return 0;
}

/**
* Checks if a header is empty (all bytes are 0)
* @param header Header to check
* @return true if the header is empty, false otherwise
*/
bool is_empty (struct c_header_gnu_tar * header) {
   char *header_bytes = (char *) header;
   int i;
   for (i = 0; i < sizeof(struct c_header_gnu_tar); i++) {
      if (header_bytes[i] != 0) return false;
   }
   return true;
}

/**
* Checks if the position is the end of the tar archive
* @param pos Position to check
* @param fd_mytar File descriptor of the tar archive
* @return true if the pos is inside the end of the tar archive, false otherwise
*/
bool correct_position(int pos, int fd_mytar) {
   struct stat stat_mytar;
   fstat(fd_mytar, &stat_mytar);
   int remaining = stat_mytar.st_size - pos;
   if (remaining >= END_TAR_ARCHIVE_ENTRY_SIZE && remaining <= (END_TAR_ARCHIVE_ENTRY_SIZE + 512 * 9)) {
      return true;
   }
   return false;
}

/** 
 * Seeks the end of the tar archive
 * @param fd_mytar File descriptor of the tar archive
 * @return 0 if the end of the archive was found, an error code otherwise
 */
int seek_end_of_files(int fd_mytar) {
   int file_number, current_offset;
   char header_buffer[FILE_HEADER_SIZE];

   file_number = 1;
   current_offset = 0;

   while (1)
   {
      // Read the header
      int read_size = read(fd_mytar, header_buffer, DATAFILE_BLOCK_SIZE);
      if (read_size == -1) return E_OPEN2;
      if (read_size != DATAFILE_BLOCK_SIZE) return E_TARFORM;
      current_offset += read_size;
      
      // Check if the header is valid
      struct c_header_gnu_tar * header = (struct c_header_gnu_tar *) header_buffer;
      if (memcmp(header->magic, "ustar ", 6) != 0) {
         if (correct_position(current_offset, fd_mytar) && is_empty(header)) break;
         else return E_TARFORM;
         }

      // Get the size of the file and advance to the next header
      int file_size = strtol(header->size, NULL, 8);
      int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
      current_offset = lseek(fd_mytar, offset, SEEK_CUR);

      file_number++;
   }
   lseek (fd_mytar, -DATAFILE_BLOCK_SIZE, SEEK_CUR);
   return file_number;
}

/**
* Extract the file (the file offset has to be at the beginning of the file header)
* @param fd_mytar file descriptor of the tar archive
* @return 0 if the file is correctly extracted, the respective error otherwise
*/
int extract_file(int fd_mytar) {
   struct c_header_gnu_tar header;


   if (read(fd_mytar, &header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
      return E_DESCO;

   int file_size = strtol(header.size, NULL, 8);
   char data_buff[file_size];

   int fd_output = creat(header.name, 0600);
   if(fd_output == -1)
   {
      close(fd_output);
      return E_CREATDEST;  
   } 
   
   int rd = read(fd_mytar, data_buff, file_size);
   if (rd == -1)
   {
      close(fd_output);
      return E_DESCO;
   }
   if (rd < file_size)
   {
      close(fd_output);
      return E_TARFORM;
   }
      
   if(write(fd_output, data_buff, file_size) < file_size) 
   { 
      close(fd_output);
      return E_DESCO;
   }

   close(fd_output);
   return 0;
}

/** 
 * Searches for a file in the tar archive, and leaves the file offset at the beginning of the header
 * @param fd_mytar File descriptor of the tar archive
 * @param f_dat Name of the file to search
 * @return 0 if the file is found. Otherwise it returns the error code.
 */
int search_file (int fd_mytar, char * f_dat) {

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
         if (!is_empty(&header)) return E_TARFORM; // Invalid tar file
         return E_NOEXIST; // End of the tar archive
      }

      // Check if the header has the name of the file to extract
      if (strcmp(header.name, f_dat) == 0)
      {
         lseek(fd_mytar, -FILE_HEADER_SIZE, SEEK_CUR);
         return 0;
      }

      // Get the size of the file and advance to the next header
      int file_size = strtol(header.size, NULL, 8);
      int offset = file_size + (DATAFILE_BLOCK_SIZE - (file_size % DATAFILE_BLOCK_SIZE));
      lseek(fd_mytar, offset, SEEK_CUR);
   }
}
