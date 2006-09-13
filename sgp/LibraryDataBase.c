#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	#include "FileMan.h"
	#include "LibraryDataBase.h"
	#include "MemMan.h"
	#include "WCheck.h"
	#include "Debug.h"
	#include <string.h>

	#if defined(JA2) || defined( UTIL )
		#include "Video.h"
	#else
		#include "video2.h"
	#endif
#endif

//NUMBER_OF_LIBRARIES
#ifdef JA2
	#include	"JA2_Libs.c"
	#include	"GameSettings.h"
#elif defined(UTIL)
	LibraryInitHeader gGameLibaries[ ] = { 0 };
#else
// We link it as an .obj file
//	#include "WizLibs.c"
#endif




//used when doing the binary search of the libraries
INT16	gsCurrentLibrary = -1;


//The location of the cdrom drive
CHAR8	gzCdDirectory[ SGPFILENAME_LEN ];


INT			CompareFileNames( CHAR8 **arg1, FileHeaderStruct **arg2 );
BOOLEAN	GetFileHeaderFromLibrary( INT16 sLibraryID, STR pstrFileName, FileHeaderStruct **pFileHeader );
void		AddSlashToPath( STR pName );
HWFILE	CreateLibraryFileHandle( INT16 sLibraryID, UINT32 uiFileNum );
BOOLEAN CheckIfFileIsAlreadyOpen( STR pFileName, INT16 sLibraryID );

INT32 CompareDirEntryFileNames( CHAR8 *arg1[], DIRENTRY **arg2 );


//************************************************************************
//
//	 InitializeFileDatabase():  Call this function to initialize the file
//	database.  It will use the gGameLibaries[] array for the list of libraries
//	and the define NUMBER_OF_LIBRARIES for the number of libraries.  The gGameLibaries
//	array is an array of structure, one of the fields determines if the library
//	will be initialized and game start.
//
//************************************************************************
BOOLEAN InitializeFileDatabase( )
{
	INT16			i;
	UINT32		uiSize;
	BOOLEAN		fLibraryInited = FALSE;

#ifdef JA2
	GetCDLocation( );
#else
	gzCdDirectory[ 0 ] = '.';
#endif

	//if all the libraries exist, set them up
	gFileDataBase.usNumberOfLibraries = NUMBER_OF_LIBRARIES;

	//allocate memory for the each of the library headers
	uiSize = NUMBER_OF_LIBRARIES * sizeof( LibraryHeaderStruct );
	if( uiSize )
	{
		gFileDataBase.pLibraries = MemAlloc( uiSize );
		CHECKF( gFileDataBase.pLibraries );

		//set all the memrory to 0
		memset( gFileDataBase.pLibraries, 0, uiSize );


		//Load up each library
		for( i=0; i< NUMBER_OF_LIBRARIES; i++ )
		{
			//if you want to init the library at the begining of the game
			if( gGameLibaries[i].fInitOnStart )
			{
				//if the library exists
				if( OpenLibrary( i ) )
					fLibraryInited = TRUE;

				//else the library doesnt exist
				else
				{
					FastDebugMsg( String("Warning in InitializeFileDatabase( ): Library Id #%d (%s) is to be loaded but cannot be found.\n", i, gGameLibaries[i].sLibraryName ));
					gFileDataBase.pLibraries[ i ].fLibraryOpen = FALSE;
				}
			}
		}
		//signify that the database has been initialized ( only if there was a library loaded )
		gFileDataBase.fInitialized = fLibraryInited;
	}

	//allocate memory for the handles of the 'real files' that will be open
	//This is needed because the the code wouldnt be able to tell the difference between a 'real' handle and a made up one
	uiSize = INITIAL_NUM_HANDLES * sizeof( RealFileOpenStruct );
	gFileDataBase.RealFiles.pRealFilesOpen = MemAlloc( uiSize );
	CHECKF( gFileDataBase.RealFiles.pRealFilesOpen );

	//clear the memory
	memset( gFileDataBase.RealFiles.pRealFilesOpen, 0, uiSize);

	//set the initial number how many files can be opened at the one time
	gFileDataBase.RealFiles.iSizeOfOpenFileArray = INITIAL_NUM_HANDLES;


	return(TRUE);
}


//*****************************************************************************************
// ReopenCDLibraries
//
// Closes all CD libraries, then reopens them. This function needs to be called when CDs
// are changed.
//
// Returns BOOLEAN            - TRUE, always
//
// Created:  3/21/00 Derek Beland
//*****************************************************************************************
BOOLEAN ReopenCDLibraries(void)
{
INT16 i;

	//Load up each library
	for(i=0; i < NUMBER_OF_LIBRARIES; i++ )
	{
		if(gFileDataBase.pLibraries[ i ].fLibraryOpen && gGameLibaries[i].fOnCDrom)
			CloseLibrary(i);

		if(gGameLibaries[i].fOnCDrom)
			OpenLibrary( i );
	}

	return(TRUE);
}

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


	//loop through all the 'opened files' ( there should be no files open )
	for( sLoop1=0; sLoop1< gFileDataBase.RealFiles.iNumFilesOpen; sLoop1++)
	{
		FastDebugMsg( String("ShutDownFileDatabase( ):  ERROR:  real file id still exists, wasnt closed") );
		CloseHandle( gFileDataBase.RealFiles.pRealFilesOpen[ sLoop1 ].hRealFileHandle );
	}

	//Free up the memory used for the real files array for the opened files
	if( gFileDataBase.RealFiles.pRealFilesOpen )
	{
		MemFree( gFileDataBase.RealFiles.pRealFilesOpen );
		gFileDataBase.RealFiles.pRealFilesOpen = NULL;
	}

	return( TRUE );
}




BOOLEAN CheckForLibraryExistence( STR pLibraryName )
{
	BOOLEAN fRetVal = FALSE;
	HANDLE	hFile;

	//try to opent the file, if we canm the library exists
	hFile = CreateFile( pLibraryName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL );

	//if the file was not opened
	if( hFile == INVALID_HANDLE_VALUE )
	{

		//the file wasnt opened
		fRetVal = FALSE;
	}
	else
	{
		CloseHandle( hFile );
		fRetVal = TRUE;
	}

	return( fRetVal );
}




BOOLEAN InitializeLibrary( STR pLibraryName, LibraryHeaderStruct *pLibHeader, BOOLEAN fCanBeOnCDrom )
{
	HANDLE	hFile;
	UINT16	usNumEntries=0;
	UINT32	uiNumBytesRead;
	UINT32	uiLoop;
	DIRENTRY DirEntry;
	LIBHEADER	LibFileHeader;
	UINT32	uiCount=0;
	CHAR8		zTempPath[ SGPFILENAME_LEN ];

	//open the library for reading ( if it exists )
	hFile = CreateFile( pLibraryName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL );
	if( hFile == INVALID_HANDLE_VALUE )
	{
		//if it failed finding the file on the hard drive, and the file can be on the cdrom
		if( fCanBeOnCDrom )
		{
			// Add the path of the cdrom to the path of the library file
			sprintf( zTempPath, "%s%s", gzCdDirectory, pLibraryName );

			//look on the cdrom
			hFile = CreateFile( zTempPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL );
			if( hFile == INVALID_HANDLE_VALUE )
			{
				UINT32 uiLastError = GetLastError();
				char zString[1024];
				FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

				return( FALSE );
			}
			else
				FastDebugMsg( String("CD Library %s opened.", zTempPath));
		}
		else
		{
			//error opening the library
			return(FALSE);
		}
	}

	// Read in the library header ( at the begining of the library )
	if( !ReadFile( hFile, &LibFileHeader, sizeof( LIBHEADER ), &uiNumBytesRead, NULL ) )
		return( FALSE );

	if( uiNumBytesRead != sizeof( LIBHEADER ) )
	{
		//Error Reading the file database header.
		return( FALSE );
	}

	//place the file pointer at the begining of the file headers ( they are at the end of the file )
	SetFilePointer( hFile, -( LibFileHeader.iEntries * (INT32)sizeof(DIRENTRY) ), NULL, FILE_END );

	//loop through the library and determine the number of files that are FILE_OK
	//ie.  so we dont load the old or deleted files
	usNumEntries = 0;
	for( uiLoop=0; uiLoop<(UINT32)LibFileHeader.iEntries; uiLoop++ )
	{
		//read in the file header
		if( !ReadFile( hFile, &DirEntry, sizeof( DIRENTRY ), &uiNumBytesRead, NULL ) )
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
	SetFilePointer( hFile, -( LibFileHeader.iEntries * (INT32)sizeof(DIRENTRY) ), NULL, FILE_END );

	//loop through all the entries
	uiCount=0;
	for( uiLoop=0; uiLoop<(UINT32)LibFileHeader.iEntries; uiLoop++ )
	{
		//read in the file header
		if( !ReadFile( hFile, &DirEntry, sizeof( DIRENTRY ), &uiNumBytesRead, NULL ) )
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
			pLibHeader->pFileHeader[ uiCount ].uiFileOffset = DirEntry.uiOffset;
			pLibHeader->pFileHeader[ uiCount ].uiFileLength = DirEntry.uiLength;

			uiCount++;
		}
	}


	pLibHeader->usNumberOfEntries = usNumEntries;

	//allocate memory for the library path
//	if( strlen( LibFileHeader.sPathToLibrary ) == 0 )
	{
//		FastDebugMsg( String("The %s library file does not contain a path.  Use 'n' argument to name the library when you create it\n", LibFileHeader.sLibName ) );
//		Assert( 0 );
	}

	//if the library has a path
	if( strlen( LibFileHeader.sPathToLibrary ) != 0 )
	{
		pLibHeader->sLibraryPath = MemAlloc( strlen( LibFileHeader.sPathToLibrary ) + 1 );
		strcpy( pLibHeader->sLibraryPath, LibFileHeader.sPathToLibrary );
	}
	else
	{
		//else the library name does not contain a path ( most likely either an error or it is the default path )
		pLibHeader->sLibraryPath = MemAlloc( 1 );
		pLibHeader->sLibraryPath[0] = '\0';
	}


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




BOOLEAN LoadDataFromLibrary( INT16 sLibraryID, UINT32 uiFileNum, PTR pData, UINT32 uiBytesToRead, UINT32 *pBytesRead )
{
	UINT32	uiOffsetInLibrary, uiLength;
	HANDLE	hLibraryFile;
	UINT32	uiNumBytesRead;
	UINT32	uiCurPos;


	//get the offset into the library, the length and current position of the file pointer.
	uiOffsetInLibrary = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->uiFileOffset;
	uiLength = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->uiFileLength;
	hLibraryFile = gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle;
	uiCurPos = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile;


	//set the file pointer to the right location
	SetFilePointer( hLibraryFile, ( uiOffsetInLibrary + uiCurPos ), NULL, FILE_BEGIN );

	//if we are trying to read more data then the size of the file, return an error
	if( uiBytesToRead + uiCurPos > uiLength )
	{
		*pBytesRead = 0;
		return( FALSE );
	}

	//get the data
	if( !ReadFile( hLibraryFile, pData, uiBytesToRead, &uiNumBytesRead, NULL ) )
		return( FALSE );

	if( uiBytesToRead != uiNumBytesRead )
	{
//		Gets the reason why the function failed
//		UINT32 uiLastError = GetLastError();
//		char zString[1024];
//		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

		return( FALSE );
	}

	gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile += uiNumBytesRead;

	//	CloseHandle( hLibraryFile );

	*pBytesRead = uiNumBytesRead;

	return( TRUE );
}

//************************************************************************
//
// CheckIfFileExistInLibrary() determines if a file exists in a library.
//
//************************************************************************

BOOLEAN CheckIfFileExistInLibrary( STR pFileName )
{
	INT16 sLibraryID;
	FileHeaderStruct *pFileHeader;

	//get thelibrary that file is in
	sLibraryID = GetLibraryIDFromFileName( pFileName );
	if( sLibraryID == -1 )
	{
		//not in any library
		return( FALSE );
	}

	if( GetFileHeaderFromLibrary( sLibraryID, pFileName, &pFileHeader ) )
		return( TRUE );
	else
		return( FALSE );
}


//************************************************************************
//
//	This function finds out if the file CAN be in a library.  It determines
//	if the library that the file MAY be in is open.
//	( eg. File is  Laptop\Test.sti, if the Laptop\ library is open, it returns true
//
//************************************************************************
INT16 GetLibraryIDFromFileName( STR pFileName )
{
INT16 sLoop1, sBestMatch=-1;

	//loop through all the libraries to check which library the file is in
	for( sLoop1=0; sLoop1<gFileDataBase.usNumberOfLibraries; sLoop1++)
	{
		//if the library is not loaded, dont try to access the array
		if( IsLibraryOpened( sLoop1 ) )
		{
			//if the library path name is of size zero, ( the library is for the default path )
			if( strlen( gFileDataBase.pLibraries[ sLoop1 ].sLibraryPath ) == 0 )
			{
				//determine if there is a directory in the file name
				if( strchr( pFileName, '\\' ) == NULL && strchr( pFileName, '//' ) == NULL )
				{
					//There is no directory in the file name
					return( sLoop1 );
				}
			}

			//compare the library name to the file name that is passed in
			else
			{
				// if the directory paths are the same, to the length of the lib's path
				if( _strnicmp( gFileDataBase.pLibraries[ sLoop1 ].sLibraryPath, pFileName, strlen( gFileDataBase.pLibraries[ sLoop1 ].sLibraryPath ) ) == 0 )
				{
					// if we've never matched, or this match's path is longer than the previous match (meaning it's more exact)
					if((sBestMatch==(-1)) || (strlen(gFileDataBase.pLibraries[ sLoop1 ].sLibraryPath) > strlen(gFileDataBase.pLibraries[ sBestMatch ].sLibraryPath)))
						sBestMatch = sLoop1;
				}
			}
		}
	}

	//no library was found, return an error
	return(sBestMatch);
}


//************************************************************************
//
//	GetFileHeaderFromLibrary() performsperforms a binary search of the
//	library.  It adds the libraries path to the file in the
//	library and then string compared that to the name that we are
//	searching for.
//
//************************************************************************

BOOLEAN	GetFileHeaderFromLibrary( INT16 sLibraryID, STR pstrFileName, FileHeaderStruct **pFileHeader )
{
	FileHeaderStruct **ppFileHeader;
	CHAR8		sFileNameWithPath[ FILENAME_SIZE ];

	//combine the library path to the file name (need it for the search of the library )
	strcpy( sFileNameWithPath, pstrFileName);

	gsCurrentLibrary = sLibraryID;

	 /* try to find the filename using a binary search algorithm: */
	 ppFileHeader = (FileHeaderStruct **) bsearch( (char *) &sFileNameWithPath, (FileHeaderStruct *) gFileDataBase.pLibraries[ sLibraryID ].pFileHeader, gFileDataBase.pLibraries[ sLibraryID ].usNumberOfEntries,
															sizeof( FileHeaderStruct ), (int (*)(const void*, const void*))CompareFileNames );

	 if( ppFileHeader )
	 {
			*pFileHeader = ( FileHeaderStruct * ) ppFileHeader;
			return( TRUE );
	 }
	 else
	 {
			pFileHeader = NULL;
			return( FALSE );
	 }
}


//************************************************************************
//
//	CompareFileNames() gets called by the binary search function.
//
//************************************************************************

INT CompareFileNames( CHAR8 *arg1[], FileHeaderStruct **arg2 )
{
	CHAR8		sSearchKey[ FILENAME_SIZE ];
	CHAR8		sFileNameWithPath[ FILENAME_SIZE ];
	FileHeaderStruct *TempFileHeader;

	TempFileHeader = ( FileHeaderStruct * ) arg2;

	sprintf( sSearchKey, "%s", arg1);

	sprintf( sFileNameWithPath, "%s%s", gFileDataBase.pLibraries[ gsCurrentLibrary ].sLibraryPath, TempFileHeader->pFileName );

   /* Compare all of both strings: */
   return _stricmp( sSearchKey, sFileNameWithPath );
}




void AddSlashToPath( STR pName )
{
	UINT32	uiLoop, uiCounter;
	BOOLEAN	fDone = FALSE;
	BOOLEAN fFound = FALSE;
	CHAR8		sNewName[ FILENAME_SIZE ];

	//find out if there is a '\' in the file name

	uiCounter=0;
	for( uiLoop=0; uiLoop < strlen( pName ) && !fDone; uiLoop++)
	{
		if( pName[ uiLoop ] == '\\' )
		{
			sNewName[ uiCounter ] = pName[ uiLoop ];
			uiCounter++;
			sNewName[ uiCounter ] = '\\';
		}
		else
			sNewName[ uiCounter ] = pName[ uiLoop ];

		uiCounter++;
	}
	sNewName[ uiCounter ] = '\0';

	strcpy( pName, sNewName );
}



//************************************************************************
//
// This function will see if a file is in a library.  If it is, the file will be opened and a file
// handle will be created for it.
//
//************************************************************************

HWFILE OpenFileFromLibrary( STR pName )
{
	FileHeaderStruct *pFileHeader;
	HWFILE					hLibFile;
	INT16							sLibraryID;
	UINT16						uiLoop1;
	UINT32						uiFileNum=0;

	UINT32						uiNewFilePosition=0;


	//Check if the file can be contained from an open library ( the path to the file a library path )
	sLibraryID = GetLibraryIDFromFileName( pName );

	if( sLibraryID != -1 )
	{
		//Check if another file is already open in the library ( report a warning if so )

//		if( gFileDataBase.pLibraries[ sLibraryID ].fAnotherFileAlreadyOpenedLibrary )
		if( gFileDataBase.pLibraries[ sLibraryID ].uiIdOfOtherFileAlreadyOpenedLibrary != 0 )
		{
			// Temp removed
//			FastDebugMsg(String("\n*******\nOpenFileFromLibrary():  Warning!:  Trying to load file '%s' from the library '%s' which already has a file open\n", pName, gGameLibaries[ sLibraryID ].sLibraryName ) );
//			FastDebugMsg(String("\n*******\nOpenFileFromLibrary():  Warning!:  Trying to load file '%s' from the library '%s' which already has a file open ( file open is '%s')\n", pName, gGameLibaries[ sLibraryID ].sLibraryName, gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ gFileDataBase.pLibraries[ sLibraryID ].uiIdOfOtherFileAlreadyOpenedLibrary ].pFileHeader->pFileName ) );
		}

		//check if the file is already open
		if( CheckIfFileIsAlreadyOpen( pName, sLibraryID ) )
			return( 0 );

		//if the file is in a library, get the file
		if( GetFileHeaderFromLibrary( sLibraryID, pName, &pFileHeader ) )
		{
			//increment the number of open files
			gFileDataBase.pLibraries[ sLibraryID ].iNumFilesOpen ++;

			//if there isnt enough space to put the file, realloc more space
			if( gFileDataBase.pLibraries[ sLibraryID ].iNumFilesOpen >= gFileDataBase.pLibraries[ sLibraryID ].iSizeOfOpenFileArray )
			{
				FileOpenStruct	*pOpenFiles;

				//reallocate more space for the array
				pOpenFiles = MemRealloc( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles,
								 gFileDataBase.pLibraries[ sLibraryID ].iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME );

				if( !pOpenFiles )
					return( 0 );

				//increment the number of open files that we can have open
				gFileDataBase.pLibraries[ sLibraryID ].iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;


				gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles = pOpenFiles;
			}

			//loop through to find a new spot in the array
			uiFileNum = 0;
			for( uiLoop1=1; uiLoop1 < gFileDataBase.pLibraries[ sLibraryID ].iSizeOfOpenFileArray; uiLoop1++)
			{
				if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiLoop1 ].uiFileID == 0 )
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
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFileID = hLibFile;
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile = 0;
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader = pFileHeader;

			//Save the current file position in the library
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiActualPositionInLibrary = SetFilePointer( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, 0, NULL, FILE_CURRENT );

			//Set the file position in the library to the begining of the 'file' in the library
			uiNewFilePosition = SetFilePointer( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->uiFileOffset, NULL, FILE_BEGIN );

			uiNewFilePosition = GetFileSize( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, NULL );

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




HWFILE CreateLibraryFileHandle( INT16 sLibraryID, UINT32 uiFileNum )
{
	HWFILE hLibFile;


	hLibFile = uiFileNum;
	hLibFile |= DB_ADD_LIBRARY_ID( sLibraryID );

	return( hLibFile );
}


HWFILE CreateRealFileHandle( HANDLE hFile )
{
	HWFILE hLibFile;
	INT32	iLoop1;
	UINT32	uiFileNum=0;
	UINT32 uiSize;

	//if there isnt enough space to put the file, realloc more space
	if( gFileDataBase.RealFiles.iNumFilesOpen >= ( gFileDataBase.RealFiles.iSizeOfOpenFileArray -1 ) )
	{
		uiSize = ( gFileDataBase.RealFiles.iSizeOfOpenFileArray + NUM_FILES_TO_ADD_AT_A_TIME ) * sizeof( RealFileOpenStruct );

		gFileDataBase.RealFiles.pRealFilesOpen = MemRealloc( gFileDataBase.RealFiles.pRealFilesOpen, uiSize );
		CHECKF( gFileDataBase.RealFiles.pRealFilesOpen );

		//Clear out the new part of the array
		memset( &gFileDataBase.RealFiles.pRealFilesOpen[ gFileDataBase.RealFiles.iSizeOfOpenFileArray ], 0, ( NUM_FILES_TO_ADD_AT_A_TIME * sizeof( RealFileOpenStruct ) ) );

		gFileDataBase.RealFiles.iSizeOfOpenFileArray += NUM_FILES_TO_ADD_AT_A_TIME;
	}


	//loop through to find a new spot in the array
	uiFileNum = 0;
	for( iLoop1=1; iLoop1 < gFileDataBase.RealFiles.iSizeOfOpenFileArray; iLoop1++)
	{
		if( gFileDataBase.RealFiles.pRealFilesOpen[ iLoop1 ].uiFileID == 0 )
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

	gFileDataBase.RealFiles.pRealFilesOpen[ iLoop1 ].uiFileID = hLibFile;
	gFileDataBase.RealFiles.pRealFilesOpen[ iLoop1 ].hRealFileHandle = hFile;

	gFileDataBase.RealFiles.iNumFilesOpen++;

	return( hLibFile );
}




BOOLEAN GetLibraryAndFileIDFromLibraryFileHandle( HWFILE hlibFile, INT16 *pLibraryID, UINT32 *pFileNum )
{
	*pFileNum = DB_EXTRACT_FILE_ID( hlibFile );
	*pLibraryID = (UINT16)DB_EXTRACT_LIBRARY( hlibFile );

//TEST: qq
/*	if( *pLibraryID == LIBRARY_SOUNDS )
	{
		int q=5;
	}
*/
	return( TRUE );
}


//************************************************************************
//
//	Close an individual file that is contained in the library
//
//************************************************************************



BOOLEAN CloseLibraryFile( INT16 sLibraryID, UINT32 uiFileID )
{
	if( IsLibraryOpened( sLibraryID ) )
	{
		//if the uiFileID is invalid
		if( (uiFileID >= (UINT32)gFileDataBase.pLibraries[ sLibraryID ].iSizeOfOpenFileArray ) )
			return( FALSE );

		//if the file is not opened, dont close it
		if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileID ].uiFileID != 0 )
		{
			//reset the variables
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileID ].uiFileID = 0;
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileID ].uiFilePosInFile = 0;
			gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileID ].pFileHeader = NULL;

			//reset the libraries file pointer to the positon it was in prior to opening the current file
			SetFilePointer( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileID ].uiActualPositionInLibrary, NULL, FILE_CURRENT);

			//decrement the number of files that are open
			gFileDataBase.pLibraries[ sLibraryID ].iNumFilesOpen--;

			// Reset the fact that a file is accessing the library
//			gFileDataBase.pLibraries[ sLibraryID ].fAnotherFileAlreadyOpenedLibrary = FALSE;
			gFileDataBase.pLibraries[ sLibraryID ].uiIdOfOtherFileAlreadyOpenedLibrary = 0;
		}
	}

	return( TRUE );
}


BOOLEAN LibraryFileSeek( INT16 sLibraryID, UINT32 uiFileNum, UINT32 uiDistance, UINT8 uiHowToSeek )
{
	UINT32	uiCurPos, uiSize;

	//if the library is not open, return an error
	if( !IsLibraryOpened( sLibraryID ) )
		return( FALSE );

	uiCurPos = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile;
	uiSize = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->uiFileLength;


	if ( uiHowToSeek == FILE_SEEK_FROM_START )
		uiCurPos = uiDistance;
	else if ( uiHowToSeek == FILE_SEEK_FROM_END )
		uiCurPos = uiSize - uiDistance;
	else if ( uiHowToSeek == FILE_SEEK_FROM_CURRENT )
		uiCurPos += uiDistance;
	else
		return(FALSE);

	gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile = uiCurPos;
	return( TRUE );
}




//************************************************************************
//
//	OpenLibrary() Opens a library from the 'array' of library names
//	that was passd in at game initialization.  Pass in an enum for the
//	library.
//
//************************************************************************

BOOLEAN OpenLibrary( INT16 sLibraryID )
{
	//if the library is already opened, report an error
	if( gFileDataBase.pLibraries[ sLibraryID ].fLibraryOpen )
		return( FALSE );

	//if we are trying to do something with an invalid library id
	if( sLibraryID >= gFileDataBase.usNumberOfLibraries )
		return( FALSE );


	//if we cant open the library
	if( !InitializeLibrary( gGameLibaries[ sLibraryID ].sLibraryName, &gFileDataBase.pLibraries[ sLibraryID ], gGameLibaries[ sLibraryID ].fOnCDrom ) )
		return( FALSE );

	return( TRUE );
}




BOOLEAN CloseLibrary( INT16 sLibraryID )
{
	UINT32	uiLoop1;

	//if the library isnt loaded, dont close it
	if( !IsLibraryOpened( sLibraryID ) )
		return( FALSE );

	#ifdef JA2TESTVERSION
		FastDebugMsg( String("ShutDownFileDatabase( ): %d bytes of ram used for the Library #%3d, path '%s',  in the File Database System\n", gFileDataBase.pLibraries[ sLibraryID ].uiTotalMemoryAllocatedForLibrary, sLibraryID, gFileDataBase.pLibraries[ sLibraryID ].sLibraryPath ));
		gFileDataBase.pLibraries[ sLibraryID ].uiTotalMemoryAllocatedForLibrary = 0;
	#endif

	//if there are any open files, loop through the library and close down whatever file is still open
	if( gFileDataBase.pLibraries[ sLibraryID ].iNumFilesOpen )
	{
		//loop though the array of open files to see if any are still open
		for( uiLoop1=0; uiLoop1<( UINT32 )gFileDataBase.pLibraries[ sLibraryID ].usNumberOfEntries; uiLoop1++)
		{
			if( CheckIfFileIsAlreadyOpen( gFileDataBase.pLibraries[ sLibraryID ].pFileHeader[ uiLoop1 ].pFileName, sLibraryID ) )
			{
				FastDebugMsg( String("CloseLibrary():  ERROR:  %s library file id still exists, wasnt closed, closing now.", gFileDataBase.pLibraries[ sLibraryID ].pFileHeader[ uiLoop1 ].pFileName ) );
				CloseLibraryFile( sLibraryID, uiLoop1 );

				//	Removed because the memory gets freed in the next for loop.  Would only enter here if files were still open
				//	gFileDataBase.pLibraries[ sLibraryID ].pFileHeader[ uiLoop1 ].pFileName = NULL;
			}
		}
	}

	//Free up the memory used for each file name
	for( uiLoop1=0; uiLoop1<gFileDataBase.pLibraries[ sLibraryID ].usNumberOfEntries; uiLoop1++)
	{
		MemFree( gFileDataBase.pLibraries[ sLibraryID ].pFileHeader[ uiLoop1 ].pFileName );
		gFileDataBase.pLibraries[ sLibraryID ].pFileHeader[ uiLoop1 ].pFileName = NULL;
	}

	//Free up the memory needed for the Library File Headers
	if( gFileDataBase.pLibraries[ sLibraryID ].pFileHeader )
	{
		MemFree( gFileDataBase.pLibraries[ sLibraryID ].pFileHeader );
		gFileDataBase.pLibraries[ sLibraryID ].pFileHeader = NULL;
	}

	//Free up the memory used for the library name
	if( gFileDataBase.pLibraries[ sLibraryID ].sLibraryPath )
	{
		MemFree( gFileDataBase.pLibraries[ sLibraryID ].sLibraryPath );
		gFileDataBase.pLibraries[ sLibraryID ].sLibraryPath = NULL;
	}

	//Free up the space requiered for the open files array
	if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles )
	{
		MemFree( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles );
		gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles = NULL;
	}

	//set that the library isnt open
	gFileDataBase.pLibraries[ sLibraryID ].fLibraryOpen = FALSE;

	//close the file ( note libraries are to be closed by the Windows close function )
	CloseHandle( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle );



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




BOOLEAN CheckIfFileIsAlreadyOpen( STR pFileName, INT16 sLibraryID )
{
	UINT16 usLoop1=0;

	CHAR8 sName[ 60 ];
	CHAR8 sPath[ 90 ];
	CHAR8 sDrive[ 60 ];
	CHAR8 sExt[ 6 ];

	CHAR8	sTempName[ 70 ];

	_splitpath( pFileName, sDrive, sPath, sName, sExt);

	strcpy( sTempName, sName );
	strcat( sTempName, sExt );

	//loop through all the open files to see if 'new' file to open is already open
	for( usLoop1=1; usLoop1 < gFileDataBase.pLibraries[ sLibraryID ].iSizeOfOpenFileArray ; usLoop1++ )
	{
		//check if the file is open
		if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ usLoop1].uiFileID != 0 )
		{
			//Check if the file already exists
			if( _stricmp( sTempName, gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ usLoop1].pFileHeader->pFileName ) == 0 )
				return( TRUE );
		}
	}
	return( FALSE );
}


BOOLEAN GetLibraryFileTime( INT16 sLibraryID, UINT32 uiFileNum, SGP_FILETIME	*pLastWriteTime )
{
	UINT16	usNumEntries=0;
	UINT32	uiNumBytesRead;
	DIRENTRY *pDirEntry;
	LIBHEADER	LibFileHeader;
	BOOLEAN fDone = FALSE;
//	UINT32	cnt;
	INT32	iFilePos=0;

	DIRENTRY **ppDirEntry;

	DIRENTRY *pAllEntries;


	memset( pLastWriteTime, 0, sizeof( SGP_FILETIME ) );

	SetFilePointer( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, 0, NULL, FILE_BEGIN );

	// Read in the library header ( at the begining of the library )
	if( !ReadFile( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, &LibFileHeader, sizeof( LIBHEADER ), &uiNumBytesRead, NULL ) )
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
	SetFilePointer( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, iFilePos, NULL, FILE_END );

	// Read in the library header ( at the begining of the library )
	if( !ReadFile( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle, pAllEntries, ( sizeof( DIRENTRY ) * LibFileHeader.iEntries ), &uiNumBytesRead, NULL ) )
		return( FALSE );
	if( uiNumBytesRead != ( sizeof( DIRENTRY ) * LibFileHeader.iEntries ) )
	{
		//Error Reading the file database header.
		return( FALSE );
	}



	 /* try to find the filename using a binary search algorithm: */
	 ppDirEntry = (DIRENTRY **) bsearch( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->pFileName,
																			(DIRENTRY *) pAllEntries,
																			LibFileHeader.iEntries,
																			sizeof( DIRENTRY ), (int (*)(const void*, const void*))CompareDirEntryFileNames );

	 if( ppDirEntry )
		pDirEntry = ( DIRENTRY * ) ppDirEntry;
	 else
		return( FALSE );

	//Copy the dir entry time over to the passed in time
	memcpy( pLastWriteTime, &pDirEntry->sFileTime, sizeof( SGP_FILETIME ) );


	MemFree( pAllEntries );
	pAllEntries = NULL;

	return( TRUE );
}



//************************************************************************
//
//	CompareFileNames() gets called by the binary search function.
//
//************************************************************************

INT32 CompareDirEntryFileNames( CHAR8 *arg1[], DIRENTRY **arg2 )
{
	CHAR8				sSearchKey[ FILENAME_SIZE ];
	CHAR8				sFileNameWithPath[ FILENAME_SIZE ];
	DIRENTRY		*TempDirEntry;

	TempDirEntry = ( DIRENTRY * ) arg2;

	sprintf( sSearchKey, "%s", arg1);

	sprintf( sFileNameWithPath, "%s", TempDirEntry->sFileName );

   /* Compare all of both strings: */
   return _stricmp( sSearchKey, sFileNameWithPath );
}
