// Build my_tardat structure with FileName stat info (See man 2 stat)
int BuilTarHeader(char *FileName, struct c_header_gnu_tar *pTarHeader);

// Write the data file (blocks of 512 bytes)
unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_TarFile);

// Write end tar archive entry (2x512 bytes with zeros) 
unsigned long WriteEndTarArchive( int fd_TarFile);

// Complete Tar file to  multiple of 10KB size block
unsigned long WriteCompleteTarSize( unsigned long TarActualSize,  int fd_TarFile);

// Verify Tar file zize to  multiple of 10KB size blocks
int VerifyCompleteTarSize( unsigned long TarActualSize);