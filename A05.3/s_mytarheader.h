/**
* @file s_mytarheader.h
* @author   G.A.
* @date     10/02/2023
* @brief    Include file with struct c_header_gnu_tar
* @details  A header file with the definition of c_header_gnu_tar of
*           gnu tar file format 
*           (1) source: https://manpages.ubuntu.com/manpages/bionic/en/man5/tar.5.html:
*           "... A tar archive consists of a series of 512-byte records.  Each file system object requires a
*           header record which stores basic metadata (pathname, owner, permissions, etc.) and zero or
*           more records containing any file data.  The end of the archive is indicated by two records
*           consisting entirely of zero bytes.
*           ..."
*           The last block of data is completed with zero bytes.
*
*           Size of a .tar file
*           The size of a .tar file has to be a multiple of 10K bytes (20 x 512 blocks).(https://www.gnu.org/software/tar/manual/html_node/Blocking-Factor.html)
*
*           File Format
*           +++++++++++++++++++++++
*           + Header Record   0   +
*           +---------------------+
*           +  Data File 0        +
*           + 0... N blocks of    +
*           + of 512 bytes        +
*           +++++++++++++++++++++++
*           + Header Record   1   +
*           +---------------------+
*           +  Data File 1        +
*           + 0... N blocks of    +
*           + of 512 bytes        +
*           +++++++++++++++++++++++
*           +        ...          +
*           +++++++++++++++++++++++
*           + Header Record  N-1  +
*           +---------------------+
*           +  Data File N-1      +
*           + 0... N blocks of    +
*           + of 512 bytes        +
*           +++++++++++++++++++++++
*           +    End of archive   +
*           +    2 blocks of      +
*           +    of 512 bytes     +
*           +++++++++++++++++++++++
*           +    Padding Data     +
*           +  to  tar file size  +
*           +  equal to N * 10K   +
*           +  block size (zeros) +
*           +++++++++++++++++++++++
*
*/

#define ERROR_OPEN_DAT_FILE (2)
#define ERROR_OPEN_TAR_FILE (3)
#define ERROR_GENERATE_TAR_FILE (4)
#define ERROR_GENERATE_TAR_FILE2 (5)

#define FILE_HEADER_SIZE     512
#define DATAFILE_BLOCK_SIZE  512
#define END_TAR_ARCHIVE_ENTRY_SIZE  (512*2)
#define TAR_FILE_BLOCK_SIZE  ((unsigned long) (DATAFILE_BLOCK_SIZE*20))

#define HEADER_OK (1)
#define HEADER_ERR (2)

#define E_OPEN1 -1
#define E_OPEN2 -2
#define E_OPEN -1
#define E_NOEXIST -2
#define E_TARFORM -3
#define E_CREATDEST -4
#define E_DESCO -99


struct c_header_gnu_tar {
        char name[100];             // file name
        char mode[8];               // stored as an octal number in ASCII.
        char uid[8];                // (idem)
        char gid[8];                // (idem)
        char size[12];              // (idem)
        char mtime[12];             // (idem)
        char checksum[8];           // see (1).
        char typeflag[1];           // see (1).
        char linkname[100];         // see (1).
        char magic[6];              // see (1).
        char version[2];            // see (1).
        char uname[32];             // user name 
        char gname[32];             // group name
        char devmajor[8];           // not used (zeros)
        char devminor[8];           // not used (zeros)
        char atime[12];             // stored as an octal number in ASCII.
        char ctime[12];             // stored as an octal number in ASCII.
        char offset[12];            // not used (zeros)
        char longnames[4];          // not used (zeros)
        char unused[1];             // not used (zeros)
        struct {
                char offset[12];
                char numbytes[12];
        } sparse[4];                // not used (zeros)
        char isextended[1];         // not used (zeros)
        char realsize[12];          // not used (zeros)
        char pad[17];               // zeros
};

