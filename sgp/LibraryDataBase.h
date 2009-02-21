#ifndef _LIBRARY_DATABASE_H
#define _LIBRARY_DATABASE_H

#include "Types.h"
#include "FileMan.h"


#define	REAL_FILE_LIBRARY_ID					1022

#define	DB_BITS_FOR_LIBRARY							10
#define	DB_BITS_FOR_FILE_ID							22

#define	DB_EXTRACT_LIBRARY( exp )				( exp >> DB_BITS_FOR_FILE_ID )
#define	DB_EXTRACT_FILE_ID( exp )				( exp & 0x3FFFFF )

#define DB_ADD_LIBRARY_ID( exp )				( exp << DB_BITS_FOR_FILE_ID )
#define DB_ADD_FILE_ID( exp )						( exp & 0xC00000 )


struct FileHeaderStruct
{
	char*  pFileName;
	UINT32	uiFileLength;
	UINT32	uiFileOffset;
};


struct LibraryHeaderStruct
{
	char* sLibraryPath;
	FILE* hLibraryHandle;
	UINT16	usNumberOfEntries;
	INT32		iNumFilesOpen;
	FileHeaderStruct *pFileHeader;

//
//	Temp:	Total memory used for each library ( all memory allocated
//
	#ifdef JA2TESTVERSION
		UINT32	uiTotalMemoryAllocatedForLibrary;
	#endif
};


struct LibraryFile
{
	UINT32                  uiFilePosInFile; // current position in the file
	LibraryHeaderStruct*    lib;
	const FileHeaderStruct* pFileHeader;
};


void    InitializeFileDatabase(char const* LibFilenames[], UINT LibCount);
void    ShutDownFileDatabase(void);
BOOLEAN CheckIfFileExistInLibrary(const char *pFileName);

BOOLEAN OpenFileFromLibrary(const char* filename, LibraryFile*);
/* Close an individual file that is contained in the library */
void    CloseLibraryFile(LibraryFile*);
BOOLEAN LoadDataFromLibrary(LibraryFile*, void* pData, UINT32 uiBytesToRead);
BOOLEAN LibraryFileSeek(LibraryFile*, INT32 distance, FileSeekMode);

BOOLEAN GetLibraryFileTime( INT16 sLibraryID, UINT32 uiFileNum, SGP_FILETIME	*pLastWriteTime );

#endif
