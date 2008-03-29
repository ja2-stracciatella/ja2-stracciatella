#include <stdlib.h>
#include "Types.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "WCheck.h"
#include "Debug.h"


#define NUM_FILES_TO_ADD_AT_A_TIME 20
#define INITIAL_NUM_HANDLES        20


#define	FILENAME_SIZE 256


#define FILE_OK           0x00
#define FILE_DELETED      0xFF
#define FILE_OLD          0x01
#define FILE_DOESNT_EXIST 0xFE


// NOTE:  The following structs are also used by the datalib98 utility
typedef struct LIBHEADER
{
	char    sLibName[FILENAME_SIZE];
	char    sPathToLibrary[FILENAME_SIZE];
	INT32   iEntries;
	INT32   iUsed;
	UINT16  iSort;
	UINT16  iVersion;
	BOOLEAN	fContainsSubDirectories;
	INT32   iReserved;
} LIBHEADER;
CASSERT(sizeof(LIBHEADER) == 532)


typedef struct
{
	char         sFileName[FILENAME_SIZE];
	UINT32       uiOffset;
	UINT32       uiLength;
	UINT8        ubState;
	UINT8        ubReserved;
	SGP_FILETIME sFileTime;
	UINT16       usReserved2;
} DIRENTRY;
CASSERT(sizeof(DIRENTRY) == 280)


static BOOLEAN InitializeLibrary(const char* pLibraryName, LibraryHeaderStruct* pLibHeader);


BOOLEAN InitializeFileDatabase(const char* LibFilenames[], UINT LibCount)
{
	INT16			i;

	//if all the libraries exist, set them up
	gFileDataBase.usNumberOfLibraries = LibCount;

	//allocate memory for the each of the library headers
	if (LibCount > 0)
	{
		LibraryHeaderStruct* const libs = MALLOCN(LibraryHeaderStruct, LibCount);
		gFileDataBase.pLibraries = libs;
		CHECKF(libs);

		//set all the memrory to 0
		memset(libs, 0, sizeof(*libs) * LibCount);

		//Load up each library
		for (i = 0; i < LibCount; i++)
		{
			if (!InitializeLibrary(LibFilenames[i], &libs[i]))
			{
				FastDebugMsg(String("Warning in InitializeFileDatabase(): Library Id #%d (%s) is to be loaded but cannot be found.\n", i, LibFilenames[i]));
				return FALSE;
			}
		}
	}

	//allocate memory for the handles of the 'real files' that will be open
	//This is needed because the the code wouldnt be able to tell the difference between a 'real' handle and a made up one
	RealFileHeaderStruct* const rfh = &gFileDataBase.RealFiles;
	rfh->pRealFilesOpen = MALLOCN(FILE*, INITIAL_NUM_HANDLES);
	CHECKF(rfh->pRealFilesOpen);

	//clear the memory
	memset(rfh->pRealFilesOpen, 0, sizeof(*rfh->pRealFilesOpen) * INITIAL_NUM_HANDLES);

	//set the initial number how many files can be opened at the one time
	rfh->iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;


	return(TRUE);
}


static BOOLEAN CloseLibrary(INT16 sLibraryID);


BOOLEAN ShutDownFileDatabase( )
{
	UINT16 sLoop1;

	// Free up the memory used for each library
	for(sLoop1=0; sLoop1 < gFileDataBase.usNumberOfLibraries; sLoop1++)
		CloseLibrary( sLoop1 );

	//Free up the memory used for all the library headers
	if( gFileDataBase.pLibraries )
	{
		MemFree( gFileDataBase.pLibraries );
		gFileDataBase.pLibraries = NULL;
	}

	RealFileHeaderStruct* const rfh = &gFileDataBase.RealFiles;

	//loop through all the 'opened files' ( there should be no files open )
	for (sLoop1 = 0; sLoop1 < rfh->iNumFilesOpen; sLoop1++)
	{
		FastDebugMsg("ShutDownFileDatabase():  ERROR:  real file id still exists, wasnt closed");
		fclose(rfh->pRealFilesOpen[sLoop1]);
	}

	//Free up the memory used for the real files array for the opened files
	if (rfh->pRealFilesOpen)
	{
		MemFree(rfh->pRealFilesOpen);
		rfh->pRealFilesOpen = NULL;
	}

	return( TRUE );
}


static int CompareFileHeader(const void* a, const void* b)
{
	const FileHeaderStruct* fhsa = a;
	const FileHeaderStruct* fhsb = b;

	return strcasecmp(fhsa->pFileName, fhsb->pFileName);
}


// Replace all \ in a string by /
static char* Slashify(const char* s)
{
	char* const res = MALLOCN(char, strlen(s) + 1);
	if (res == NULL) return NULL;
	char* d = res;
	do { *d++ = (*s == '\\' ? '/' : *s); } while (*s++ != '\0');
	return res;
}


static BOOLEAN InitializeLibrary(const char* const lib_name, LibraryHeaderStruct* const lib)
{
	FILE* hFile = fopen(lib_name, "rb");
	if (hFile == NULL)
	{
		char zTempPath[SGPFILENAME_LEN];
		snprintf(zTempPath, lengthof(zTempPath), "%s/Data/%s", GetBinDataPath(), lib_name);
		hFile = fopen(zTempPath, "rb");
		if (hFile == NULL)
		{
			fprintf(stderr, "ERROR: Failed to open \"%s\"\n", zTempPath);
			return FALSE;
		}
		FastDebugMsg(String("CD Library %s opened.", zTempPath));
	}

	// read in the library header (at the begining of the library)
	LIBHEADER	LibFileHeader;
	if (fread(&LibFileHeader, sizeof(LibFileHeader), 1, hFile) != 1) return FALSE;

	const UINT32 count_entries = LibFileHeader.iEntries;

	FileHeaderStruct* fhs = MALLOCN(FileHeaderStruct, count_entries);
#ifdef JA2TESTVERSION
	lib->uiTotalMemoryAllocatedForLibrary = sizeof(*fhs) * count_entries;
#endif

	/* place the file pointer at the begining of the file headers (they are at the
	 * end of the file) */
	fseek(hFile, -(ssize_t)(count_entries * sizeof(DIRENTRY)), SEEK_END);

	UINT32 used_entries = 0;
	for (UINT32 uiLoop = 0; uiLoop < count_entries; ++uiLoop)
	{
		DIRENTRY DirEntry;
		if (fread(&DirEntry, sizeof(DirEntry), 1, hFile) != 1) return FALSE;

		if (DirEntry.ubState != FILE_OK) continue;

		// check to see if the file is not longer than it should be
		if (strlen(DirEntry.sFileName) + 1 >= FILENAME_SIZE)
			FastDebugMsg(String("\n*******InitializeLibrary():  Warning!:  '%s' from the library '%s' has name whose size (%d) is bigger then it should be (%s)", DirEntry.sFileName, lib->sLibraryPath, strlen(DirEntry.sFileName) + 1, FILENAME_SIZE));

		FileHeaderStruct* const fh = &fhs[used_entries++];

		fh->pFileName = Slashify(DirEntry.sFileName);
		if (fh->pFileName == NULL) return FALSE;
#ifdef JA2TESTVERSION
		lib->uiTotalMemoryAllocatedForLibrary += strlen(fh->pFileName) + 1;
#endif

		fh->uiFileOffset = DirEntry.uiOffset;
		fh->uiFileLength = DirEntry.uiLength;
	}

	if (used_entries != count_entries)
	{
		fhs = MemRealloc(fhs, sizeof(*fhs) * used_entries);
	}

	qsort(fhs, used_entries, sizeof(*fhs), CompareFileHeader);

	lib->pFileHeader       = fhs;
	lib->usNumberOfEntries = used_entries;

	lib->sLibraryPath = Slashify(LibFileHeader.sPathToLibrary);
	if (lib->sLibraryPath == NULL) return FALSE;
#ifdef JA2TESTVERSION
	lib->uiTotalMemoryAllocatedForLibrary += strlen(lib->sLibraryPath) + 1;
#endif

	//allocate space for the open files array
	FileOpenStruct* const fo = MALLOCN(FileOpenStruct, INITIAL_NUM_HANDLES);
	lib->pOpenFiles = fo;
	if (!fo) return FALSE;
	memset(fo, 0, INITIAL_NUM_HANDLES * sizeof(*fo));
#ifdef JA2TESTVERSION
	lib->uiTotalMemoryAllocatedForLibrary += INITIAL_NUM_HANDLES * sizeof(*fo);
#endif

	lib->hLibraryHandle       = hFile;
	lib->iNumFilesOpen        = 0;
	lib->iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;
	return TRUE;
}


BOOLEAN LoadDataFromLibrary(const HWFILE file, void* const pData, const UINT32 uiBytesToRead)
{
	const INT16  sLibraryID = DB_EXTRACT_LIBRARY(file);
	const UINT32 uiFileNum  = DB_EXTRACT_FILE_ID(file);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		if (uiFileNum == 0) return FALSE;

		FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		return fread(pData, uiBytesToRead, 1, hRealFile) == 1;
	}
	else
	{
		if (!IsLibraryOpened(sLibraryID)) return FALSE;
		const LibraryHeaderStruct* const lh = &gFileDataBase.pLibraries[sLibraryID];
		FileOpenStruct*            const fo = &lh->pOpenFiles[uiFileNum];
		if (fo->pFileHeader == NULL) return FALSE;

		const UINT32 uiOffsetInLibrary = fo->pFileHeader->uiFileOffset;
		const UINT32 uiLength          = fo->pFileHeader->uiFileLength;
		FILE* const  hLibraryFile      = lh->hLibraryHandle;
		const UINT32 uiCurPos          = fo->uiFilePosInFile;

		fseek(hLibraryFile, uiOffsetInLibrary + uiCurPos, SEEK_SET);

		// if we are trying to read more data than the size of the file, return an error
		if (uiBytesToRead + uiCurPos > uiLength) return FALSE;

		if (fread(pData, uiBytesToRead, 1, hLibraryFile) != 1) return FALSE;

		fo->uiFilePosInFile += uiBytesToRead;
		return TRUE;
	}
}


static const FileHeaderStruct* GetFileHeaderFromLibrary(const LibraryHeaderStruct* lib, const char* filename);
static INT16 GetLibraryIDFromFileName(const char* pFileName);


BOOLEAN CheckIfFileExistInLibrary(const char *pFileName)
{
	INT16 sLibraryID;

	//get thelibrary that file is in
	sLibraryID = GetLibraryIDFromFileName( pFileName );
	if( sLibraryID == -1 )
	{
		//not in any library
		return( FALSE );
	}

	return GetFileHeaderFromLibrary(&gFileDataBase.pLibraries[sLibraryID], pFileName) != NULL;
}


/* This function finds out if the file CAN be in a library.  It determines if
 * the library that the file MAY be in is open.  Eg. file is  Laptop/Test.sti,
 * if the Laptop/ library is open, it returns true */
static INT16 GetLibraryIDFromFileName(const char* pFileName)
{
INT16 sLoop1, sBestMatch=-1;

	//loop through all the libraries to check which library the file is in
	for( sLoop1=0; sLoop1<gFileDataBase.usNumberOfLibraries; sLoop1++)
	{
		//if the library is not loaded, dont try to access the array
		if( IsLibraryOpened( sLoop1 ) )
		{
			const char* const lib_path = gFileDataBase.pLibraries[sLoop1].sLibraryPath;
			//if the library path name is of size zero, ( the library is for the default path )
			if (strlen(lib_path) == 0)
			{
				//determine if there is a directory in the file name
				if (strchr(pFileName, '/') == NULL)
				{
					//There is no directory in the file name
					return( sLoop1 );
				}
			}

			//compare the library name to the file name that is passed in
			else
			{
				// if the directory paths are the same, to the length of the lib's path
				if (strncasecmp(lib_path, pFileName, strlen(lib_path)) == 0)
				{
					// if we've never matched, or this match's path is longer than the previous match (meaning it's more exact)
					if (sBestMatch == -1 || strlen(lib_path) > strlen(gFileDataBase.pLibraries[sBestMatch].sLibraryPath))
						sBestMatch = sLoop1;
				}
			}
		}
	}

	//no library was found, return an error
	return(sBestMatch);
}


static int CompareFileNames(const void* key, const void* member);


static const char* g_current_lib_path;


/* Performsperforms a binary search of the library.  It adds the libraries path
 * to the file in the library and then string compared that to the name that we
 * are searching for. */
static const FileHeaderStruct* GetFileHeaderFromLibrary(const LibraryHeaderStruct* const lib, const char* const filename)
{
	g_current_lib_path = lib->sLibraryPath;
	return bsearch(filename, lib->pFileHeader, lib->usNumberOfEntries, sizeof(*lib->pFileHeader), CompareFileNames);
}


static int CompareFileNames(const void* key, const void* member)
{
	const char* sSearchKey = key;
	const FileHeaderStruct* TempFileHeader = member;
	char sFileNameWithPath[FILENAME_SIZE];

	sprintf(sFileNameWithPath, "%s%s", g_current_lib_path, TempFileHeader->pFileName);

	return strcasecmp(sSearchKey, sFileNameWithPath);
}


static BOOLEAN CheckIfFileIsAlreadyOpen(const char* pFileName, const LibraryHeaderStruct* lib);


/* This function will see if a file is in a library.  If it is, the file will be
 * opened and a file handle will be created for it. */
HWFILE OpenFileFromLibrary(const char* const pName)
{
	//Check if the file can be contained from an open library ( the path to the file a library path )
	const INT16 sLibraryID = GetLibraryIDFromFileName(pName);
	if (sLibraryID == -1) return 0;

	LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[sLibraryID];

	if (CheckIfFileIsAlreadyOpen(pName, lib)) return 0;

	//if the file is in a library, get the file
	const FileHeaderStruct* const pFileHeader = GetFileHeaderFromLibrary(lib, pName);
	if (pFileHeader == NULL) return 0;

	//increment the number of open files
	lib->iNumFilesOpen++;

	//if there isnt enough space to put the file, realloc more space
	if (lib->iNumFilesOpen >= lib->iSizeOfOpenFileArray)
	{
		//reallocate more space for the array
		FileOpenStruct* const pOpenFiles = MemRealloc(lib->pOpenFiles, sizeof(*pOpenFiles) * (lib->iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME));
		if (!pOpenFiles) return 0;

		//increment the number of open files that we can have open
		lib->iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;
		lib->pOpenFiles            = pOpenFiles;
	}

	UINT32 uiFileNum;
	for (uiFileNum = 1;; ++uiFileNum)
	{
		if (uiFileNum >= lib->iSizeOfOpenFileArray)         return 0;
		if (lib->pOpenFiles[uiFileNum].pFileHeader == NULL) break;
	}

	//Set the current file data into the array of open files
	lib->pOpenFiles[uiFileNum].uiFilePosInFile = 0;
	lib->pOpenFiles[uiFileNum].pFileHeader     = pFileHeader;

	return DB_ADD_LIBRARY_ID(sLibraryID) | uiFileNum;
}


HWFILE CreateRealFileHandle(FILE* hFile)
{
	RealFileHeaderStruct* const rfh = &gFileDataBase.RealFiles;
	HWFILE hLibFile;
	INT32	iLoop1;
	UINT32	uiFileNum=0;

	//if there isnt enough space to put the file, realloc more space
	if (rfh->iNumFilesOpen >= rfh->iSizeOfOpenFileArray - 1)
	{
		const UINT32 uiSize = (rfh->iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME) * sizeof(*rfh->pRealFilesOpen);
		rfh->pRealFilesOpen = MemRealloc(rfh->pRealFilesOpen, uiSize);
		CHECKF(rfh->pRealFilesOpen);

		//Clear out the new part of the array
		memset(&rfh->pRealFilesOpen[rfh->iSizeOfOpenFileArray], 0, NUM_FILES_TO_ADD_AT_A_TIME * sizeof(*rfh->pRealFilesOpen));

		rfh->iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;
	}


	//loop through to find a new spot in the array
	uiFileNum = 0;
	for (iLoop1 = 1; iLoop1 < rfh->iSizeOfOpenFileArray; iLoop1++)
	{
		if (rfh->pRealFilesOpen[iLoop1] == NULL)
		{
			uiFileNum = iLoop1;
			break;
		}
	}

	//if for some reason we couldnt find a spot, return an error
	if( uiFileNum == 0 )
		return( 0 );

	hLibFile = uiFileNum;
	hLibFile |= DB_ADD_LIBRARY_ID( REAL_FILE_LIBRARY_ID );

	rfh->pRealFilesOpen[iLoop1] = hFile;
	rfh->iNumFilesOpen++;

	return( hLibFile );
}




BOOLEAN GetLibraryAndFileIDFromLibraryFileHandle( HWFILE hlibFile, INT16 *pLibraryID, UINT32 *pFileNum )
{
	*pFileNum = DB_EXTRACT_FILE_ID( hlibFile );
	*pLibraryID = (UINT16)DB_EXTRACT_LIBRARY( hlibFile );

	return( TRUE );
}


void CloseLibraryFile(const HWFILE file)
{
	const INT16  lib_id  = DB_EXTRACT_LIBRARY(file);
	const UINT32 file_id = DB_EXTRACT_FILE_ID(file);

	if (lib_id == REAL_FILE_LIBRARY_ID)
	{
		RealFileHeaderStruct* const rfh = &gFileDataBase.RealFiles;
		// if its not already closed
		if (rfh->pRealFilesOpen[file_id] != NULL)
		{
			fclose(rfh->pRealFilesOpen[file_id]);
			rfh->pRealFilesOpen[file_id] = NULL;
			Assert(rfh->iNumFilesOpen > 0);
			--rfh->iNumFilesOpen;
		}
	}
	else
	{
		if (!IsLibraryOpened(lib_id)) return;
		LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[lib_id];

		// if the file_id is invalid
		if (file_id >= (UINT32)lib->iSizeOfOpenFileArray) return;

		// if the file is not opened, dont close it
		if (lib->pOpenFiles[file_id].pFileHeader == NULL) return;

		// reset the variables
		lib->pOpenFiles[file_id].uiFilePosInFile = 0;
		lib->pOpenFiles[file_id].pFileHeader     = NULL;

		// decrement the number of files that are open
		lib->iNumFilesOpen--;
	}
}


BOOLEAN LibraryFileSeek(const HWFILE file, INT32 distance, const INT how)
{
	const INT16  sLibraryID = DB_EXTRACT_LIBRARY(file);
	const UINT32 uiFileNum  = DB_EXTRACT_FILE_ID(file);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		int whence;
		switch (how)
		{
			case FILE_SEEK_FROM_START: whence = SEEK_SET; break;

			case FILE_SEEK_FROM_END:
				whence = SEEK_END;
				if (distance > 0) distance = -distance;
				break;

			default: whence = SEEK_CUR; break;
		}

		FILE* const f = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		return fseek(f, distance, whence) == 0;
	}
	else
	{
		if (!IsLibraryOpened(sLibraryID)) return FALSE;
		FileOpenStruct* const fo = &gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum];

		UINT32       uiCurPos = fo->uiFilePosInFile;
		const UINT32 uiSize   = fo->pFileHeader->uiFileLength;

		switch (how)
		{
			case FILE_SEEK_FROM_START:   uiCurPos  = distance;          break;
			case FILE_SEEK_FROM_END:     uiCurPos  = uiSize - distance; break;
			case FILE_SEEK_FROM_CURRENT: uiCurPos += distance;          break;
			default:                     return FALSE;
		}

		fo->uiFilePosInFile = uiCurPos;
		return TRUE;
	}
}


static BOOLEAN CloseLibrary(INT16 sLibraryID)
{
	UINT32	uiLoop1;

	//if the library isnt loaded, dont close it
	if( !IsLibraryOpened( sLibraryID ) )
		return( FALSE );
	LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[sLibraryID];

#ifdef JA2TESTVERSION
	FastDebugMsg(String("ShutDownFileDatabase( ): %d bytes of ram used for the Library #%3d, path '%s',  in the File Database System\n", lib->uiTotalMemoryAllocatedForLibrary, sLibraryID, lib->sLibraryPath));
	lib->uiTotalMemoryAllocatedForLibrary = 0;
#endif

	//if there are any open files, loop through the library and close down whatever file is still open
	if (lib->iNumFilesOpen)
	{
		//loop though the array of open files to see if any are still open
		for (uiLoop1 = 0; uiLoop1 < (UINT32)lib->usNumberOfEntries; uiLoop1++)
		{
			const char* const filename = lib->pFileHeader[uiLoop1].pFileName;
			if (CheckIfFileIsAlreadyOpen(filename, lib))
			{
				FastDebugMsg(String("CloseLibrary():  ERROR:  %s library file id still exists, wasnt closed, closing now.", filename));
				CloseLibraryFile(DB_ADD_LIBRARY_ID(sLibraryID) | uiLoop1);

				//	Removed because the memory gets freed in the next for loop.  Would only enter here if files were still open
				//	lib->pFileHeader[uiLoop1].pFileName = NULL;
			}
		}
	}

	//Free up the memory used for each file name
	for (uiLoop1 = 0; uiLoop1 < lib->usNumberOfEntries; uiLoop1++)
	{
		MemFree(lib->pFileHeader[uiLoop1].pFileName);
		lib->pFileHeader[uiLoop1].pFileName = NULL;
	}

	//Free up the memory needed for the Library File Headers
	if (lib->pFileHeader)
	{
		MemFree(lib->pFileHeader);
		lib->pFileHeader = NULL;
	}

	//Free up the memory used for the library name
	if (lib->sLibraryPath)
	{
		MemFree(lib->sLibraryPath);
		lib->sLibraryPath = NULL;
	}

	//Free up the space requiered for the open files array
	if (lib->pOpenFiles)
	{
		MemFree(lib->pOpenFiles);
		lib->pOpenFiles = NULL;
	}

	fclose(lib->hLibraryHandle);
	lib->hLibraryHandle = NULL;

	return( TRUE );
}


BOOLEAN IsLibraryOpened(const INT16 sLibraryID)
{
	return
		sLibraryID < gFileDataBase.usNumberOfLibraries &&
		gFileDataBase.pLibraries[sLibraryID].hLibraryHandle != NULL;
}


static BOOLEAN CheckIfFileIsAlreadyOpen(const char* const pFileName, const LibraryHeaderStruct* const lib)
{
	UINT16 usLoop1=0;

	const char* sTempName = strrchr(pFileName, '/');
	const char* sName = sTempName == NULL ? pFileName : sTempName + 1;

	//loop through all the open files to see if 'new' file to open is already open
	for (usLoop1 = 1; usLoop1 < lib->iSizeOfOpenFileArray ; usLoop1++)
	{
		const FileHeaderStruct* const fh = lib->pOpenFiles[usLoop1].pFileHeader;
		//check if the file is open
		if (fh != NULL && strcasecmp(sName, fh->pFileName) == 0) return TRUE;
	}
	return( FALSE );
}


static int CompareDirEntryFileNames(const void* key, const void* member);


BOOLEAN GetLibraryFileTime( INT16 sLibraryID, UINT32 uiFileNum, SGP_FILETIME	*pLastWriteTime )
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	if (!IsLibraryOpened(sLibraryID)) return FALSE;
	const LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[sLibraryID];
	if (lib->pOpenFiles[uiFileNum].uiFileID == 0) return FALSE;

	UINT16	usNumEntries=0;
	UINT32	uiNumBytesRead;
	LIBHEADER	LibFileHeader;
	BOOLEAN fDone = FALSE;
//	UINT32	cnt;
	INT32	iFilePos=0;

	memset( pLastWriteTime, 0, sizeof( SGP_FILETIME ) );

	SetFilePointer(lib->hLibraryHandle, 0, NULL, FILE_BEGIN);

	// Read in the library header ( at the begining of the library )
	if (!ReadFile(lib->hLibraryHandle, &LibFileHeader, sizeof(LIBHEADER), &uiNumBytesRead, NULL))
		return( FALSE );
	if( uiNumBytesRead != sizeof( LIBHEADER ) )
	{
		//Error Reading the file database header.
		return( FALSE );
	}


	//If the file number is greater then the number in the lirary, return false
	if( uiFileNum >= (UINT32)LibFileHeader.iEntries )
		return( FALSE );

	DIRENTRY* const pAllEntries = MALLOCN(DIRENTRY, LibFileHeader.iEntries);
	if( pAllEntries == NULL )
		return( FALSE );
	memset( pAllEntries, 0, sizeof( DIRENTRY ) );



	iFilePos = -( LibFileHeader.iEntries * (INT32)sizeof(DIRENTRY) );

	//set the file pointer to the right location
	SetFilePointer(lib->hLibraryHandle, iFilePos, NULL, FILE_END);

	// Read in the library header ( at the begining of the library )
	if (!ReadFile(lib->hLibraryHandle, pAllEntries, sizeof(DIRENTRY) * LibFileHeader.iEntries, &uiNumBytesRead, NULL))
		return( FALSE );
	if( uiNumBytesRead != ( sizeof( DIRENTRY ) * LibFileHeader.iEntries ) )
	{
		//Error Reading the file database header.
		return( FALSE );
	}

	DIRENTRY* pDirEntry = bsearch(
		lib->pOpenFiles[uiFileNum].pFileHeader->pFileName,
		pAllEntries,
		LibFileHeader.iEntries,
		sizeof(*pAllEntries),
		CompareDirEntryFileNames
	);

	if (pDirEntry == NULL) return FALSE;

	//Copy the dir entry time over to the passed in time
	*pLastWriteTime = pDirEntry->sFileTime;

	MemFree( pAllEntries );
	pAllEntries = NULL;

	return( TRUE );
#endif
}


static int CompareDirEntryFileNames(const void* key, const void* member)
{
	const char* sSearchKey = key;
	const DIRENTRY* TempDirEntry = member;

	return strcasecmp(sSearchKey, TempDirEntry->sFileName);
}
