#include <stdlib.h>
#include "Types.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "WCheck.h"
#include "Debug.h"


#ifdef JA2
#	include "GameSettings.h"
#endif


typedef struct
{
	CHAR8        sFileName[FILENAME_SIZE];
	UINT32       uiOffset;
	UINT32       uiLength;
	UINT8        ubState;
	UINT8        ubReserved;
	SGP_FILETIME sFileTime;
	UINT16       usReserved2;
} DIRENTRY;
CASSERT(sizeof(DIRENTRY) == 280)


//used when doing the binary search of the libraries
INT16	gsCurrentLibrary = -1;


//The location of the cdrom drive
CHAR8	gzCdDirectory[ SGPFILENAME_LEN ];


static HWFILE	CreateLibraryFileHandle(INT16 sLibraryID, UINT32 uiFileNum);
static BOOLEAN CheckIfFileIsAlreadyOpen(const char *pFileName, INT16 sLibraryID);


static BOOLEAN OpenLibrary(INT16 sLibraryID, const char* LibFilename);


BOOLEAN InitializeFileDatabase(const char* LibFilenames[], UINT LibCount)
{
	INT16			i;
	BOOLEAN		fLibraryInited = FALSE;

#ifdef JA2
	GetCDLocation( );
#else
	gzCdDirectory[ 0 ] = '.';
#endif

	//if all the libraries exist, set them up
	gFileDataBase.usNumberOfLibraries = LibCount;

	//allocate memory for the each of the library headers
	UINT32 uiSize = LibCount * sizeof(LibraryHeaderStruct);
	if( uiSize )
	{
		LibraryHeaderStruct* const libs = MemAlloc(uiSize);
		gFileDataBase.pLibraries = libs;
		CHECKF(libs);

		//set all the memrory to 0
		memset(libs, 0, uiSize);

		//Load up each library
		for (i = 0; i < LibCount; i++)
		{
			//if the library exists
			if (OpenLibrary(i, LibFilenames[i]))
				fLibraryInited = TRUE;

			//else the library doesnt exist
			else
			{
				FastDebugMsg(String("Warning in InitializeFileDatabase(): Library Id #%d (%s) is to be loaded but cannot be found.\n", i, LibFilenames[i]));
				libs[i].fLibraryOpen = FALSE;
			}
		}
		//signify that the database has been initialized ( only if there was a library loaded )
		gFileDataBase.fInitialized = fLibraryInited;
	}

	//allocate memory for the handles of the 'real files' that will be open
	//This is needed because the the code wouldnt be able to tell the difference between a 'real' handle and a made up one
	RealFileHeaderStruct* const rfh = &gFileDataBase.RealFiles;
	uiSize = INITIAL_NUM_HANDLES * sizeof(*rfh->pRealFilesOpen);
	rfh->pRealFilesOpen = MemAlloc(uiSize);
	CHECKF(rfh->pRealFilesOpen);

	//clear the memory
	memset(rfh->pRealFilesOpen, 0, uiSize);

	//set the initial number how many files can be opened at the one time
	rfh->iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;


	return(TRUE);
}


static BOOLEAN CloseLibrary(INT16 sLibraryID);


//************************************************************************
//
//	 ShutDownFileDatabase():  Call this function to close down the file
//	database.
//
//************************************************************************

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
static void Slashify(char* s)
{
	for (; *s != '\0'; s++) if (*s == '\\') *s = '/';
}


static BOOLEAN InitializeLibrary(const char* pLibraryName, LibraryHeaderStruct* pLibHeader)
{
	FILE*	hFile;
	UINT16	usNumEntries=0;
	UINT32	uiLoop;
	DIRENTRY DirEntry;
	LIBHEADER	LibFileHeader;
	UINT32	uiCount=0;
	CHAR8		zTempPath[ SGPFILENAME_LEN ];

	//open the library for reading ( if it exists )
	hFile = fopen(pLibraryName, "rb");
	if (hFile == NULL)
	{
		// Add the path of the cdrom to the path of the library file
		sprintf(zTempPath, "%s%s", gzCdDirectory, pLibraryName);

		//look on the cdrom
		hFile = fopen(zTempPath, "rb");
		if (hFile == NULL)
		{
			fprintf(stderr, "ERROR: Failed to open \"%s\"\n", zTempPath);
			abort();
		}
		FastDebugMsg(String("CD Library %s opened.", zTempPath));
	}

	// Read in the library header ( at the begining of the library )
	if (fread(&LibFileHeader, sizeof(LibFileHeader), 1, hFile) != 1)
		return( FALSE );

	//place the file pointer at the begining of the file headers ( they are at the end of the file )
	fseek(hFile, -(LibFileHeader.iEntries * (ssize_t)sizeof(DIRENTRY)), SEEK_END);

	//loop through the library and determine the number of files that are FILE_OK
	//ie.  so we dont load the old or deleted files
	usNumEntries = 0;
	for( uiLoop=0; uiLoop<(UINT32)LibFileHeader.iEntries; uiLoop++ )
	{
		//read in the file header
		if (fread(&DirEntry, sizeof(DirEntry), 1, hFile) != 1)
			return( FALSE );

		if( DirEntry.ubState == FILE_OK )
			usNumEntries++;
	}


	//Allocate enough memory for the library header
	pLibHeader->pFileHeader = MemAlloc( sizeof( FileHeaderStruct ) * usNumEntries );

	#ifdef JA2TESTVERSION
		pLibHeader->uiTotalMemoryAllocatedForLibrary = sizeof( FileHeaderStruct ) * usNumEntries;
	#endif


	//place the file pointer at the begining of the file headers ( they are at the end of the file )
	fseek(hFile, -(LibFileHeader.iEntries * (ssize_t)sizeof(DIRENTRY)), SEEK_END);

	//loop through all the entries
	uiCount=0;
	for( uiLoop=0; uiLoop<(UINT32)LibFileHeader.iEntries; uiLoop++ )
	{
		//read in the file header
		if (fread(&DirEntry, sizeof(DirEntry), 1, hFile) != 1)
			return( FALSE );


		if( DirEntry.ubState == FILE_OK )
		{
			//Check to see if the file is not longer then it should be
			if( ( strlen( DirEntry.sFileName ) + 1 ) >= FILENAME_SIZE )
				FastDebugMsg(String("\n*******InitializeLibrary():  Warning!:  '%s' from the library '%s' has name whose size (%d) is bigger then it should be (%s)", DirEntry.sFileName, pLibHeader->sLibraryPath, ( strlen( DirEntry.sFileName ) + 1 ), FILENAME_SIZE ) );


			//allocate memory for the files name
			pLibHeader->pFileHeader[ uiCount ].pFileName = MemAlloc( strlen( DirEntry.sFileName ) + 1 );

			//if we couldnt allocate memory
			if( !pLibHeader->pFileHeader[ uiCount ].pFileName )
			{
				//report an error
				return(FALSE);
			}


			#ifdef JA2TESTVERSION
				pLibHeader->uiTotalMemoryAllocatedForLibrary += strlen( DirEntry.sFileName ) + 1;
			#endif


			//copy the file name, offset and length into the header
			strcpy( pLibHeader->pFileHeader[ uiCount ].pFileName, DirEntry.sFileName);
			Slashify(pLibHeader->pFileHeader[uiCount].pFileName);
			pLibHeader->pFileHeader[ uiCount ].uiFileOffset = DirEntry.uiOffset;
			pLibHeader->pFileHeader[ uiCount ].uiFileLength = DirEntry.uiLength;

			uiCount++;
		}
	}

	qsort(pLibHeader->pFileHeader, usNumEntries, sizeof(*pLibHeader->pFileHeader), CompareFileHeader);

	pLibHeader->usNumberOfEntries = usNumEntries;

	//allocate memory for the library path
//	if( strlen( LibFileHeader.sPathToLibrary ) == 0 )
	{
//		FastDebugMsg( String("The %s library file does not contain a path.  Use 'n' argument to name the library when you create it\n", LibFileHeader.sLibName ) );
//		Assert( 0 );
	}

	pLibHeader->sLibraryPath = MemAlloc(strlen(LibFileHeader.sPathToLibrary) + 1);
	strcpy(pLibHeader->sLibraryPath, LibFileHeader.sPathToLibrary);
	Slashify(pLibHeader->sLibraryPath);

	#ifdef JA2TESTVERSION
		pLibHeader->uiTotalMemoryAllocatedForLibrary += strlen( LibFileHeader.sPathToLibrary ) + 1;
	#endif


	//allocate space for the open files array
	pLibHeader->pOpenFiles = MemAlloc( INITIAL_NUM_HANDLES * sizeof( FileOpenStruct ) );
	if( !pLibHeader->pOpenFiles )
	{
			//report an error
			return(FALSE);
	}

	memset( pLibHeader->pOpenFiles, 0, INITIAL_NUM_HANDLES * sizeof( FileOpenStruct ) );

	#ifdef JA2TESTVERSION
		pLibHeader->uiTotalMemoryAllocatedForLibrary += INITIAL_NUM_HANDLES * sizeof( FileOpenStruct );
	#endif



	pLibHeader->hLibraryHandle = hFile;
	pLibHeader->usNumberOfEntries = usNumEntries;
	pLibHeader->fLibraryOpen = TRUE;
	pLibHeader->iNumFilesOpen = 0;
	pLibHeader->iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;

	return( TRUE );
}


BOOLEAN LoadDataFromLibrary(const HWFILE file, void* const pData, const UINT32 uiBytesToRead)
{
	const INT16  sLibraryID = DB_EXTRACT_LIBRARY(file);
	const UINT32 uiFileNum  = DB_EXTRACT_FILE_ID(file);

	BOOLEAN fRet = FALSE;
	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		if (uiFileNum == 0) return FALSE;

		FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		return (fread(pData, uiBytesToRead, 1, hRealFile) == 1);
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


static const FileHeaderStruct* GetFileHeaderFromLibrary(INT16 sLibraryID, const char* pstrFileName);
static INT16 GetLibraryIDFromFileName(const char* pFileName);


//************************************************************************
//
// CheckIfFileExistInLibrary() determines if a file exists in a library.
//
//************************************************************************

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

	return GetFileHeaderFromLibrary(sLibraryID, pFileName) != NULL;
}


//************************************************************************
//
//	This function finds out if the file CAN be in a library.  It determines
//	if the library that the file MAY be in is open.
//	( eg. File is  Laptop\Test.sti, if the Laptop\ library is open, it returns true
//
//************************************************************************
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


/* Performsperforms a binary search of the library.  It adds the libraries path
 * to the file in the library and then string compared that to the name that we
 * are searching for. */
static const FileHeaderStruct* GetFileHeaderFromLibrary(INT16 sLibraryID, const char* pstrFileName)
{
	gsCurrentLibrary = sLibraryID;

	const LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[sLibraryID];
	return bsearch(pstrFileName, lib->pFileHeader, lib->usNumberOfEntries, sizeof(*lib->pFileHeader), CompareFileNames);
}


//************************************************************************
//
//	CompareFileNames() gets called by the binary search function.
//
//************************************************************************
static int CompareFileNames(const void* key, const void* member)
{
	const char* sSearchKey = key;
	const FileHeaderStruct* TempFileHeader = member;
	char sFileNameWithPath[FILENAME_SIZE];

	sprintf(sFileNameWithPath, "%s%s", gFileDataBase.pLibraries[gsCurrentLibrary].sLibraryPath, TempFileHeader->pFileName);

	return strcasecmp(sSearchKey, sFileNameWithPath);
}


//************************************************************************
//
// This function will see if a file is in a library.  If it is, the file will be opened and a file
// handle will be created for it.
//
//************************************************************************

HWFILE OpenFileFromLibrary(const char *pName)
{
	HWFILE					hLibFile;
	INT16							sLibraryID;
	UINT16						uiLoop1;
	UINT32						uiFileNum=0;

	//Check if the file can be contained from an open library ( the path to the file a library path )
	sLibraryID = GetLibraryIDFromFileName( pName );

	if( sLibraryID != -1 )
	{
		LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[sLibraryID];
		//Check if another file is already open in the library ( report a warning if so )

//		if (lib->fAnotherFileAlreadyOpenedLibrary)
		if (lib->uiIdOfOtherFileAlreadyOpenedLibrary != 0)
		{
			// Temp removed
//			FastDebugMsg(String("\n*******\nOpenFileFromLibrary():  Warning!:  Trying to load file '%s' from the library '%s' which already has a file open\n", pName, gGameLibaries[ sLibraryID ].sLibraryName ) );
//			FastDebugMsg(String("\n*******\nOpenFileFromLibrary():  Warning!:  Trying to load file '%s' from the library '%s' which already has a file open ( file open is '%s')\n", pName, gGameLibaries[sLibraryID].sLibraryName, lib->pOpenFiles[lib->uiIdOfOtherFileAlreadyOpenedLibrary].pFileHeader->pFileName));
		}

		//check if the file is already open
		if (CheckIfFileIsAlreadyOpen(pName, sLibraryID))
			return( 0 );

		//if the file is in a library, get the file
		const FileHeaderStruct* pFileHeader = GetFileHeaderFromLibrary(sLibraryID, pName);
		if (pFileHeader != NULL)
		{
			//increment the number of open files
			lib->iNumFilesOpen++;

			//if there isnt enough space to put the file, realloc more space
			if (lib->iNumFilesOpen >= lib->iSizeOfOpenFileArray)
			{
				FileOpenStruct	*pOpenFiles;

				//reallocate more space for the array
				pOpenFiles = MemRealloc(lib->pOpenFiles, lib->iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME);

				if( !pOpenFiles )
					return( 0 );

				//increment the number of open files that we can have open
				lib->iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;


				lib->pOpenFiles = pOpenFiles;
			}

			//loop through to find a new spot in the array
			uiFileNum = 0;
			for (uiLoop1 = 1; uiLoop1 < lib->iSizeOfOpenFileArray; uiLoop1++)
			{
				if (lib->pOpenFiles[uiLoop1].pFileHeader == NULL)
				{
					uiFileNum = uiLoop1;
					break;
				}
			}

			//if for some reason we couldnt find a spot, return an error
			if( uiFileNum == 0 )
				return( 0 );

			//Create a library handle for the new file
			hLibFile = CreateLibraryFileHandle( sLibraryID, uiFileNum );

			//Set the current file data into the array of open files
			lib->pOpenFiles[uiFileNum].uiFilePosInFile = 0;
			lib->pOpenFiles[uiFileNum].pFileHeader = pFileHeader;

			//Set the file position in the library to the begining of the 'file' in the library
			fseek(lib->hLibraryHandle, lib->pOpenFiles[uiFileNum].pFileHeader->uiFileOffset, SEEK_SET);
		}
		else
		{
			// Failed to find the file in a library
			return( 0 );
		}
	}
	else
	{
		// Library is not open, or doesnt exist
		return( 0 );
	}

	//Set the fact the a file is currently open in the library
//	gFileDataBase.pLibraries[ sLibraryID ].fAnotherFileAlreadyOpenedLibrary = TRUE;
	gFileDataBase.pLibraries[ sLibraryID ].uiIdOfOtherFileAlreadyOpenedLibrary = uiFileNum;

	return( hLibFile );
}


static HWFILE	CreateLibraryFileHandle(INT16 sLibraryID, UINT32 uiFileNum)
{
	HWFILE hLibFile;


	hLibFile = uiFileNum;
	hLibFile |= DB_ADD_LIBRARY_ID( sLibraryID );

	return( hLibFile );
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

		// Reset the fact that a file is accessing the library
		lib->uiIdOfOtherFileAlreadyOpenedLibrary = 0;
	}
}


BOOLEAN LibraryFileSeek(const HWFILE file, const UINT32 uiDistance, const UINT8 uiHowToSeek)
{
	const INT16  sLibraryID = DB_EXTRACT_LIBRARY(file);
	const UINT32 uiFileNum  = DB_EXTRACT_FILE_ID(file);

	//if the library is not open, return an error
	if( !IsLibraryOpened( sLibraryID ) )
		return( FALSE );
	FileOpenStruct* const fo = &gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum];

	UINT32       uiCurPos = fo->uiFilePosInFile;
	const UINT32 uiSize   = fo->pFileHeader->uiFileLength;


	if ( uiHowToSeek == FILE_SEEK_FROM_START )
		uiCurPos = uiDistance;
	else if ( uiHowToSeek == FILE_SEEK_FROM_END )
		uiCurPos = uiSize - uiDistance;
	else if ( uiHowToSeek == FILE_SEEK_FROM_CURRENT )
		uiCurPos += uiDistance;
	else
		return(FALSE);

	fo->uiFilePosInFile = uiCurPos;
	return( TRUE );
}


static BOOLEAN OpenLibrary(INT16 sLibraryID, const char* LibFilename)
{
	//if the library is already opened, report an error
	if( gFileDataBase.pLibraries[ sLibraryID ].fLibraryOpen )
		return( FALSE );

	//if we are trying to do something with an invalid library id
	if( sLibraryID >= gFileDataBase.usNumberOfLibraries )
		return( FALSE );


	//if we cant open the library
	return InitializeLibrary(LibFilename, &gFileDataBase.pLibraries[sLibraryID]);
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
			if (CheckIfFileIsAlreadyOpen(filename, sLibraryID))
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

	//set that the library isnt open
	lib->fLibraryOpen = FALSE;

	//close the file ( note libraries are to be closed by the Windows close function )
	fclose(lib->hLibraryHandle);

	return( TRUE );
}

BOOLEAN IsLibraryOpened( INT16 sLibraryID )
{
	//if the database is not initialized
	if( !gFileDataBase.fInitialized )
		return( FALSE );

	//if we are trying to do something with an invalid library id
	if( sLibraryID >= gFileDataBase.usNumberOfLibraries )
		return( FALSE );

	//if the library is opened
	if( gFileDataBase.pLibraries[ sLibraryID ].fLibraryOpen )
		return( TRUE );
	else
		return( FALSE );
}


static BOOLEAN CheckIfFileIsAlreadyOpen(const char *pFileName, INT16 sLibraryID)
{
	UINT16 usLoop1=0;

	const char* sTempName = strrchr(pFileName, '/');
	const char* sName = sTempName == NULL ? pFileName : sTempName + 1;

	const LibraryHeaderStruct* const lib = &gFileDataBase.pLibraries[sLibraryID];
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

	DIRENTRY *pAllEntries;


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

	pAllEntries = MemAlloc( sizeof( DIRENTRY ) * LibFileHeader.iEntries );
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
