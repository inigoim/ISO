/* *
 * * @file create_mytar.c
 * * @author G.A.
 * * @date 10/02/2023
 * * @brief First version of mytar
 * * @details  Create a tar file with only one "data file"
 * *
 * */
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


// #define ERROR_OPEN_DAT_FILE (2)
// #define ERROR_OPEN_TAR_FILE (3)
// #define ERROR_GENERATE_TAR_FILE (4)
// #define ERROR_GENERATE_TAR_FILE2 (5)


// #define HEADER_OK (1)
// #define HEADER_ERR (2)

// #define FILE_HEADER_SIZE     512
// #define DATAFILE_BLOCK_SIZE  512
// #define END_TAR_ARCHIVE_ENTRY_SIZE  (512*2)
// #define TAR_FILE_BLOCK_SIZE  ((unsigned long) (DATAFILE_BLOCK_SIZE*20))

struct passwd *pws;
struct group  *grp;

char FileDataBlock[DATAFILE_BLOCK_SIZE];

//-----------------------------------------------------------------------------
// Return UserName (string) from uid (integer). See man 2 stat and man getpwuid 
char * getUserName(uid_t uid)
{
   pws = getpwuid(uid);
       return pws->pw_name;
}

//------------------------------------------------------------------------------
// Return GroupName (string) from gid (integer). See man 2 stat and man getgrgid   
char * getGroupName(gid_t gid)
{
   grp = getgrgid(gid);
       return grp->gr_name;
}

//----------------------------------------
// Return Mode type entry  (tar Mode type)
char mode_tar( mode_t Mode)
{
   if  (S_ISREG(Mode))  return  '0';
   if  (S_ISLNK(Mode))  return  '2';
   if  (S_ISCHR(Mode))  return  '3';
   if  (S_ISBLK(Mode))  return  '4';
   if  (S_ISDIR(Mode))  return  '5';
   if  (S_ISFIFO(Mode))  return  '6';
   if  (S_ISSOCK(Mode))  return  '7';
   return '0';
}


// ------------------------------------------------------------------------
// (1.0) Build my_tardat structure with FileName stat info (See man 2 stat)
int BuilTarHeader(char *FileName, struct c_header_gnu_tar *pTarHeader)
{
   struct stat  stat_file;

   ssize_t  Symlink_Size;
   int n, i;
   char  *pTarHeaderBytes;
   unsigned int  Checksum;

   bzero(pTarHeader, sizeof(struct c_header_gnu_tar));

   if (stat (FileName, &stat_file) == -1)  return HEADER_ERR;

   strcpy(pTarHeader->name, FileName);
   sprintf(pTarHeader->mode, "%07o", stat_file.st_mode & 07777);  // Only  the least significant 12 bits
   printf("st_mode del archivo %s %07o\n",FileName, stat_file.st_mode & 07777); // Only  the least significant 12 bits
   sprintf(pTarHeader->uid, "%07o", stat_file.st_uid);
   sprintf(pTarHeader->gid, "%07o", stat_file.st_gid);
   sprintf(pTarHeader->size, "%011lo", stat_file.st_size);
   sprintf(pTarHeader->mtime, "%011lo", stat_file.st_mtime);
   // checksum  the last     sprintf(pTarHeader->checksum, "%06o", Checksum);

   pTarHeader->typeflag[0] = mode_tar(stat_file.st_mode);

    //  linkname
   if  (S_ISLNK(stat_file.st_mode))
        Symlink_Size = readlink(FileName,pTarHeader->linkname,100);

   strncpy(pTarHeader->magic, "ustar ",6);  // "ustar" followed by a space (without null char)
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
   memset(pTarHeader->checksum, ' ', 8);   // Initialize to blanc spaces
   pTarHeaderBytes = (unsigned char *) pTarHeader;

   for (i=0,Checksum=0 ; i < sizeof(struct c_header_gnu_tar); i++)
          Checksum = Checksum + pTarHeaderBytes[i];

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
   printf("Datos Escritos:");   // Traza
   memset(FileDataBlock, 0, sizeof(FileDataBlock));
   while((n=read(fd_DataFile,  FileDataBlock, sizeof(FileDataBlock))) > 0)
   {
       NumWriteBytes += write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));  // ojo!!!, no se escriben n, ni sizeof(FileDataBlock)
       printf(" ---%d", n); // Traza
       memset(FileDataBlock, 0, sizeof(FileDataBlock)); 
   }
   
   printf("\nTotal: %ld bytes escritos\n", NumWriteBytes); // Traza
   return NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.1)Write end tar archive entry (2x512 bytes with zeros) 
unsigned long WriteEndTarArchive( int fd_TarFile)
{
   unsigned long NumWriteBytes;
   int n;
   char end_of_archive[END_TAR_ARCHIVE_ENTRY_SIZE];

   NumWriteBytes = 0;
   memset(end_of_archive, 0, sizeof(end_of_archive)); //podemos usar tambien END_TAR_ARCHIVE_ENTRY_SIZE
   NumWriteBytes = write(fd_TarFile, end_of_archive, sizeof(end_of_archive)); //podemos usar tambien END_TAR_ARCHIVE_ENTRY_SIZE

   printf(" Escritos (End block %d) total %ld\n", n, NumWriteBytes); // Traza

   return   NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.2) complete Tar file to  multiple of 10KB size block
unsigned long WriteCompleteTarSize( unsigned long TarActualSize,  int fd_TarFile)
{
   unsigned long NumWriteBytes;
   unsigned long Module, offset;
   int n;
   char * buffer;
   
   NumWriteBytes = TarActualSize;
   
   // complete to  multiple of 10KB size blocks
   printf("TAR_FILE_BLOCK_SIZE=%ld  TarFileSize=%ld\n", TAR_FILE_BLOCK_SIZE, NumWriteBytes); // Traza
   Module = NumWriteBytes % 10240;
   offset = 10240 - Module;

   if(Module!=0){
    char padding[offset];
    memset(padding, 0, sizeof(padding));
    NumWriteBytes = write(fd_TarFile, padding, sizeof(padding));
   }
  
   printf("OK: Generado el EndTarBlocks del archivo tar %ld bytes \n", NumWriteBytes); // Traza

   return offset;
}

// Verify Tar file zize to  multiple of 10KB size blocks
int  VerifyCompleteTarSize( unsigned long TarActualSize)
{

	 // Verify
	if ((TarActualSize % TAR_FILE_BLOCK_SIZE) != 0)
	{
		fprintf(stderr,"Error al generar el fichero tar. Tamanio erroneo %ld\n", TarActualSize);
		return ERROR_GENERATE_TAR_FILE2;
	}
	return 0;
		
}
