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


// ------------------------------------------------------------------------
// (1.0) Build my_tardat structure with FileName stat info (See man 2 stat)
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
   if  (S_ISLNK(stat_file.st_mode))
      readlink(FileName, pTarHeader->linkname, 100);

   strcpy(pTarHeader->magic, "ustar ");  // "ustar" followed by a space (without null char)
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
   while((n=read(fd_DataFile, FileDataBlock, sizeof(FileDataBlock))) > 0)
   {
       NumWriteBytes += write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));  // ojo!!!, no se escriben n, ni sizeof(FileDataBlock)
       memset(FileDataBlock, 0, sizeof(FileDataBlock)); 
   }
   
   printf("Total: %ld bytes escritos\n", NumWriteBytes); // Traza
   return NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.1)Write end tar archive entry (2x512 bytes with zeros) 
unsigned long WriteEndTarArchive( int fd_TarFile)
{
   unsigned long NumWriteBytes;

   memset(FileDataBlock, 0, sizeof(FileDataBlock));

   NumWriteBytes = 0;
   while (NumWriteBytes < END_TAR_ARCHIVE_ENTRY_SIZE)
      NumWriteBytes +=  write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));

   return NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.2) complete Tar file to  multiple of 10KB size block
unsigned long WriteCompleteTarSize( unsigned long TarCurrentSize,  int fd_TarFile)
{
   unsigned long NumWriteBytes;
   unsigned long module, offset;
   
   // complete to  multiple of 10KB size blocks
   NumWriteBytes = 0;
   module = TarCurrentSize % 10240;
   offset = 10240 - module;
   if(module != 0){
    char padding[offset];
    memset(padding, 0, sizeof(padding));
    NumWriteBytes = write(fd_TarFile, padding, sizeof(padding));
   }

   return NumWriteBytes;
}


int tar_insert_file (int fd_mytar, char *f_dat) {
   struct c_header_gnu_tar tar_header;
   if (BuilTarHeader(f_dat, &tar_header) != HEADER_OK) return -1;
   
   // Write the header and the data
   if (write(fd_mytar, &tar_header, FILE_HEADER_SIZE) != FILE_HEADER_SIZE)
      return -99;

   int fd_dat = open(f_dat, O_RDONLY);
   if (WriteFileDataBlocks(fd_dat, fd_mytar) < 0)
      return -99;
}

int tar_complete_archive (int fd_mytar) {
   struct stat stat_mytar;
   WriteEndTarArchive(fd_mytar);
   fstat(fd_mytar, &stat_mytar);
   WriteCompleteTarSize(stat_mytar.st_size, fd_mytar);
}
