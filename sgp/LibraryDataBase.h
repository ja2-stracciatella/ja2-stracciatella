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


typedef struct
{
	char*  pFileName;
	UINT32	uiFileLength;
	UINT32	uiFileOffset;
} FileHeaderStruct;


typedef struct
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

} LibraryHeaderStruct;


typedef struct LibraryFile
{
	UINT32                  uiFilePosInFile; // current position in the file
	LibraryHeaderStruct*    lib;
	const FileHeaderStruct* pFileHeader;
} LibraryFile;


#ifdef __cplusplus
extern "C" {
#endif


BOOLEAN InitializeFileDatabase(const char* LibFilenames[], UINT LibCount);
BOOLEAN ShutDownFileDatabase(void);
BOOLEAN CheckIfFileExistInLibrary(const char *pFileName);

BOOLEAN OpenFileFromLibrary(const char* filename, LibraryFile*);
/* Close an individual file that is contained in the library */
void    CloseLibraryFile(LibraryFile*);
BOOLEAN LoadDataFromLibrary(LibraryFile*, void* pData, UINT32 uiBytesToRead);
BOOLEAN LibraryFileSeek(LibraryFile*, INT32 distance, FileSeekMode);

BOOLEAN IsLibraryOpened( INT16 sLibraryID );

BOOLEAN GetLibraryFileTime( INT16 sLibraryID, UINT32 uiFileNum, SGP_FILETIME	*pLastWriteTime );


#ifdef __cplusplus
}
#endif



#endif
