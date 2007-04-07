//**************************************************************************
//
// Filename :	FileMan.c
//
//	Purpose :	function definitions for the memory manager
//
// Modification history :
//
//		24sep96:HJH		-> creation
//    08Apr97:ARM   -> Assign return value from Push() calls back to HStack
//                     handle, because it may possibly do a MemRealloc()
//		29Dec97:Kris Morness
//									-> Added functionality for setting file attributes which
//									   allows for read-only attribute overriding
//									-> Also added a simple function that clears all file attributes
//										 to normal.
//
//		5 Feb 98:Dave French -> extensive modification to support libraries
//
//**************************************************************************

#include "Debug.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "Types.h"
#include "WCheck.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "Stubs.h" // XXX

#ifdef _WIN32
#	include <direct.h>
#	include <shlobj.h>
#	include <winerror.h>

#	define mkdir(path, mode) _mkdir(path)

#else
#	include <glob.h>
#	include <pwd.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#endif


//The FileDatabaseHeader
DatabaseManagerHeaderStruct gFileDataBase;


struct Glob
{
	glob_t Glob;
	UINT32 Index;
};
typedef struct Glob Glob;

static Glob Win32FindInfo[20];
BOOLEAN fFindInfoInUse[20] = {FALSE,FALSE,FALSE,FALSE,FALSE,
															FALSE,FALSE,FALSE,FALSE,FALSE,
															FALSE,FALSE,FALSE,FALSE,FALSE,
															FALSE,FALSE,FALSE,FALSE,FALSE };


static char LocalPath[512];


//**************************************************************************
//
// FileSystemInit
//
//		Starts up the file system.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN InitializeFileManager(void)
{
	char DataPath[512];

#ifdef _WIN32
	char Home[MAX_PATH];
	if (FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, Home)))
	{
		fprintf(stderr, "Unable to locate home directory\n");
		return FALSE;
	}

#	define LOCALDIR "JA2"

#else
	const char* Home = getenv("HOME");
	if (Home == NULL)
	{
		const struct passwd* passwd = getpwuid(getuid());

		if (passwd == NULL || passwd->pw_dir == NULL)
		{
			fprintf(stderr, "Unable to locate home directory\n");
			return FALSE;
		}

		Home = passwd->pw_dir;
	}

#	define LOCALDIR ".ja2"

#endif

	snprintf(LocalPath, lengthof(LocalPath), "%s/" LOCALDIR, Home);
	if (mkdir(LocalPath, 0700) != 0 && errno != EEXIST)
	{
		fprintf(stderr, "Unable to create directory \"%s\"\n", LocalPath);
		return FALSE;
	}

	snprintf(DataPath, lengthof(DataPath), "%s/Data", LocalPath);
	if (mkdir(DataPath, 0700) != 0 && errno != EEXIST)
	{
		fprintf(stderr, "Unable to create directory \"%s\"\n", DataPath);
		return FALSE;
	}

	return( TRUE );
}


//**************************************************************************
//
// FileExists
//
//		Checks if a file exists.
//
// Parameter List :
//
//		STR	-> name of file to check existence of
//
// Return Value :
//
//		BOOLEAN	-> TRUE if it exists
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

BOOLEAN FileExists(const char *strFilename)
{
	BOOLEAN fExists = FileExistsNoDB(strFilename);
	if (!fExists && gFileDataBase.fInitialized)
	{
		fExists = CheckIfFileExistInLibrary(strFilename);
	}
	return fExists;
}

//**************************************************************************
//
// FileExistsNoDB
//
//		Checks if a file exists, but doesn't check the database files.
//
// Parameter List :
//
//		STR	-> name of file to check existence of
//
// Return Value :
//
//		BOOLEAN	-> TRUE if it exists
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN FileExistsNoDB(const char *strFilename)
{
	BOOLEAN	fExists = FALSE;
	FILE		*file;
	//HANDLE	hRealFile;

	//open up the file to see if it exists on the disk
	file = fopen( strFilename, "r" );
	//hRealFile = CreateFile( strFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING,
	//								FILE_FLAG_RANDOM_ACCESS, NULL );
	if ( file )
	//if ( hRealFile != INVALID_HANDLE_VALUE )
	{
		fExists = TRUE;
		fclose( file );
		//CloseHandle( hRealFile );
	}
	else
	{
		char Path[512];

		snprintf(Path, lengthof(Path), SGPDATADIR "/Data/%s", strFilename);
		file = fopen(Path, "r");
		if (file != NULL)
		{
			fclose(file);
			return TRUE;
		}
	}

	return( fExists );

}

//**************************************************************************
//
// FileDelete
//
//		Deletes a file.
//
// Parameter List :
//
//		STR	-> name of file to delete
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN FileDelete(const char *strFilename)
{
#if 1 // XXX TODO
	return unlink(strFilename) == 0;
#else
	return( DeleteFile( strFilename ) );
#endif
}

//**************************************************************************
//
// FileOpen
//
//		Opens a file.
//
// Parameter List :
//
//		STR	   -> filename
//		UIN32		-> access - read or write, or both
//
// Return Value :
//
//		HWFILE	-> handle of opened file
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

HWFILE FileOpen(const char* strFilename, UINT32 uiOptions)
{
	HWFILE	hFile;
	FILE*	hRealFile;
	const char* dwAccess;
	BOOLEAN	fExists;
	DWORD		dwCreationFlags;
	HWFILE hLibFile;

	hFile = 0;
	dwCreationFlags = 0;

	// check if the file exists - note that we use the function FileExistsNoDB
	// because it doesn't check the databases, and we don't want to do that here
	fExists = FileExistsNoDB( strFilename );

	dwAccess = 0;
	if (uiOptions & FILE_ACCESS_READ && uiOptions & FILE_ACCESS_WRITE) {
		dwAccess = "r+";
	} else if (uiOptions & FILE_ACCESS_READ) {
		dwAccess = "r";
	} else if (uiOptions & FILE_ACCESS_WRITE) {
		dwAccess = "r+"; // XXX HACK do not truncate the file
	} else {
		dwAccess = "";
		abort(); // XXX something is fishy
	}

	//if the file is on the disk
	if ( fExists )
	{
		hRealFile = fopen(strFilename, dwAccess);
		if (hRealFile == NULL)
		{
			char Path[512];

			snprintf(Path, lengthof(Path), SGPDATADIR "/Data/%s", strFilename);
			hRealFile = fopen(Path, dwAccess);

			if (hRealFile == NULL)
			{
				return 0;
			}
		}

		//create a file handle for the 'real file'
		hFile = CreateRealFileHandle( hRealFile );
	}

	// if the file did not exist, try to open it from the database
	else if ( gFileDataBase.fInitialized )
	{
		//if the file doesnt exist on the harddrive, but it is to be created, dont try to load it from the file database
		if( uiOptions & FILE_ACCESS_WRITE )
		{
			//if the files is to be written to
			if( ( uiOptions & FILE_CREATE_NEW ) || ( uiOptions & FILE_OPEN_ALWAYS ) || ( uiOptions & FILE_CREATE_ALWAYS ) || ( uiOptions & FILE_TRUNCATE_EXISTING ) )
			{
				hFile = 0;
			}
		}
		//else if the file is to be opened using FILE_OPEN_EXISTING, and the file doesnt exists, fail out of the function)
//		else if( uiOptions & FILE_OPEN_EXISTING )
//		{
			//fail out of the function
//			return( 0 );
//		}
		else
		{
			//If the file is in the library, get a handle to it.
			hLibFile = OpenFileFromLibrary( strFilename );

			//tried to open a file that wasnt in the database
			if( !hLibFile )
				return( 0 );
			else
				return( hLibFile );		//return the file handle
		}
	}

	if ( !hFile )
	{
#if 1
		FIXME
		hRealFile = fopen(strFilename, "w");
		if (hRealFile == NULL) return 0;
#else
		if ( uiOptions & FILE_CREATE_NEW )
		{
			dwCreationFlags = CREATE_NEW;
		}
		else if ( uiOptions & FILE_CREATE_ALWAYS )
		{
			dwCreationFlags = CREATE_ALWAYS;
		}
		else if ( uiOptions & FILE_OPEN_EXISTING || uiOptions & FILE_ACCESS_READ )
		{
			dwCreationFlags = OPEN_EXISTING;
		}
		else if ( uiOptions & FILE_OPEN_ALWAYS )
		{
			dwCreationFlags = OPEN_ALWAYS;
		}
		else if ( uiOptions & FILE_TRUNCATE_EXISTING )
		{
			dwCreationFlags = TRUNCATE_EXISTING;
		}
		else
		{
			dwCreationFlags = OPEN_ALWAYS;
		}

		hRealFile = CreateFile( strFilename, dwAccess, 0, NULL, dwCreationFlags,
										dwFlagsAndAttributes, NULL );
		if ( hRealFile == INVALID_HANDLE_VALUE )
		{
				UINT32 uiLastError = GetLastError();
				char zString[1024];
				FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

			return(0);
		}
#endif

		hFile = CreateRealFileHandle( hRealFile );
	}

	if ( !hFile )
		return(0);

	return(hFile);
}



//**************************************************************************
//
// FileClose
//
//
// Parameter List :
//
//		HWFILE hFile	-> handle to file to close
//
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

void FileClose( HWFILE hFile )
{
	INT16 sLibraryID;
	UINT32 uiFileNum;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its the 'real file' library
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//if its not already closed
		if( gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].uiFileID != 0 )
		{
			fclose(gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle);
			gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].uiFileID = 0;
			gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle= 0;
			gFileDataBase.RealFiles.iNumFilesOpen--;
			if( gFileDataBase.RealFiles.iNumFilesOpen < 0 )
			{
				//if for some reason we are below 0, report an error ( should never be )
				Assert( 0 );
			}
		}
	}
	else
	{
		//if the database is initialized
		if( gFileDataBase.fInitialized )
			CloseLibraryFile( sLibraryID, uiFileNum );
	}
}

//**************************************************************************
//
// FileRead
//
//		To read a file.
//
// Parameter List :
//
//		HWFILE		-> handle to file to read from
//		void	*	-> source buffer
//		UINT32	-> num bytes to read
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//		08Dec97:ARM		-> return FALSE if bytes to read != bytes read
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

#ifdef JA2TESTVERSION
	extern UINT32 uiTotalFileReadTime;
	extern UINT32 uiTotalFileReadCalls;
	#include "Timer_Control.h"
#endif

BOOLEAN FileRead(HWFILE hFile, PTR pDest, UINT32 uiBytesToRead)
{
	FILE* hRealFile;
	UINT32 dwNumBytesToRead, dwNumBytesRead;
	BOOLEAN	fRet = FALSE;
	INT16 sLibraryID;
	UINT32 uiFileNum;

#ifdef JA2TESTVERSION
	UINT32 uiStartTime = GetJA2Clock();
#endif

	//init the variables
	dwNumBytesToRead = dwNumBytesRead = 0;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	dwNumBytesToRead	= (DWORD)uiBytesToRead;

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//if the file is opened
		if( uiFileNum != 0 )
		{
			hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

			fRet = (fread(pDest, dwNumBytesToRead, 1, hRealFile) == 1);
		}
	}
	else
	{
		//if the database is initialized
		if( gFileDataBase.fInitialized )
		{
			//if the library is open
			if( IsLibraryOpened( sLibraryID ) )
			{
				//if the file is opened
				if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFileID != 0 )
				{
					//read the data from the library
					fRet = LoadDataFromLibrary( sLibraryID, uiFileNum, pDest, dwNumBytesToRead, &dwNumBytesRead );
				}
			}
		}
	}
	#ifdef JA2TESTVERSION
		//Add the time that we spent in this function to the total.
		uiTotalFileReadTime += GetJA2Clock() - uiStartTime;
		uiTotalFileReadCalls++;
	#endif

	return(fRet);
}

//**************************************************************************
//
// FileWrite
//
//		To write a file.
//
// Parameter List :
//
//		HWFILE		-> handle to file to write to
//		void	*	-> destination buffer
//		UINT32	-> num bytes to write
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//		08Dec97:ARM		-> return FALSE if dwNumBytesToWrite != dwNumBytesWritten
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

BOOLEAN FileWrite(HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite)
{
	FILE* hRealFile;
	DWORD		dwNumBytesToWrite;
	BOOLEAN	fRet;
	INT16 sLibraryID;
	UINT32 uiFileNum;


	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		dwNumBytesToWrite = (DWORD)uiBytesToWrite;

		//get the real file handle to the file
		hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

		fRet = (fwrite(pDest, dwNumBytesToWrite, 1, hRealFile) == 1);
	}
	else
	{
		//we cannot write to a library file
		return(FALSE);
	}

	return(fRet);
}


//**************************************************************************
//
// FileSeek
//
//		To seek to a position in a file.
//
// Parameter List :
//
//		HWFILE	-> handle to file to seek in
//		UINT32	-> distance to seek
//		UINT8		-> how to seek
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

BOOLEAN FileSeek( HWFILE hFile, UINT32 uiDistance, UINT8 uiHow )
{
	FILE* hRealFile;
	LONG		lDistanceToMove;
	int dwMoveMethod;
	INT32		iDistance=0;

	INT16 sLibraryID;
	UINT32 uiFileNum;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//Get the handle to the real file
		hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

		iDistance = (INT32) uiDistance;

		if ( uiHow == FILE_SEEK_FROM_START )
			dwMoveMethod = SEEK_SET;
		else if ( uiHow == FILE_SEEK_FROM_END )
		{
			dwMoveMethod = SEEK_END;
			if( iDistance > 0 )
				iDistance = -(iDistance);
		}
		else
			dwMoveMethod = SEEK_CUR;

		lDistanceToMove = (LONG)uiDistance;

		if (fseek(hRealFile, iDistance, dwMoveMethod) != 0)
			return(FALSE);
	}
	else
	{
		//if the database is initialized
		if( gFileDataBase.fInitialized )
			LibraryFileSeek( sLibraryID, uiFileNum, uiDistance, uiHow );
	}

	return(TRUE);
}

//**************************************************************************
//
// FileGetPos
//
//		To get the current position in a file.
//
// Parameter List :
//
//		HWFILE	-> handle to file
//
// Return Value :
//
//		INT32		-> current offset in file if successful
//					-> -1 if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

INT32 FileGetPos( HWFILE hFile )
{
	FILE* hRealFile;
	UINT32	uiPositionInFile=0;

	INT16 sLibraryID;
	UINT32 uiFileNum;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//Get the handle to the real file
		hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

		return ftell(hRealFile);
	}
	else
	{
		//if the library is open
		if( IsLibraryOpened( sLibraryID ) )
		{
			//check if the file is open
			if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFileID != 0 )
			{
				uiPositionInFile = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile;
				return( uiPositionInFile );
			}
		}
	}

	return(BAD_INDEX);
}

//**************************************************************************
//
// FileGetSize
//
//		To get the current file size.
//
// Parameter List :
//
//		HWFILE	-> handle to file
//
// Return Value :
//
//		INT32		-> file size in file if successful
//					-> 0 if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//		9 Feb 98	DEF - modified to work with the library system
//
//**************************************************************************

UINT32 FileGetSize( HWFILE hFile )
{
	FILE* hRealHandle;
	UINT32	uiFileSize = 0xFFFFFFFF;

	INT16 sLibraryID;
	UINT32 uiFileNum;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		long here;

		//Get the handle to a real file
		hRealHandle = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

		here = ftell(hRealHandle);
		fseek(hRealHandle, 0, SEEK_END);
		uiFileSize = ftell(hRealHandle);
		fseek(hRealHandle, here, SEEK_SET);
	}
	else
	{
		//if the library is open
		if( IsLibraryOpened( sLibraryID ) )
			uiFileSize = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->uiFileLength;
	}


	if ( uiFileSize == 0xFFFFFFFF )
		return(0);
	else
		return( uiFileSize );
}


BOOLEAN SetFileManCurrentDirectory(const char *pcDirectory )
{
#if 1 // XXX TODO
	return chdir(pcDirectory) == 0;
#else
	 return( SetCurrentDirectory( pcDirectory ) );
#endif
}


BOOLEAN GetFileManCurrentDirectory( STRING512 pcDirectory )
{
#if 1 // XXX TODO
	return getcwd(pcDirectory, sizeof(STRING512)) != NULL;
#else
	if (GetCurrentDirectory( 512, pcDirectory ) == 0)
	{
		return( FALSE );
	}
	return( TRUE );
#endif
}


BOOLEAN MakeFileManDirectory(const char *pcDirectory)
{
	return mkdir(pcDirectory, 0755) == 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Removes ALL FILES in the specified directory (and all subdirectories with their files if fRecursive is TRUE)
// Use EraseDirectory() to simply delete directory contents without deleting the directory itself
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOLEAN RemoveFileManDirectory(const char *pcDirectory, BOOLEAN fRecursive)
{
#if 1 // XXX TODO
	fprintf(stderr, "===> %s:%d: IGNORING %s(\"%s\", %s)\n", __FILE__, __LINE__, __func__, pcDirectory, fRecursive ? "TRUE" : "FALSE");
	return FALSE;
	UNIMPLEMENTED();
#else
	WIN32_FIND_DATA sFindData;
	HANDLE		SearchHandle;
	const CHAR8	*pFileSpec = "*.*";
	BOOLEAN	fDone = FALSE;
	BOOLEAN fRetval=FALSE;
	CHAR8		zOldDir[512];
	CHAR8		zSubdirectory[512];

	GetFileManCurrentDirectory( zOldDir );

	if( !SetFileManCurrentDirectory( pcDirectory ) )
	{
		FastDebugMsg(String("RemoveFileManDirectory: ERROR - SetFileManCurrentDirectory on %s failed, error %d", pcDirectory, GetLastError()));
		return( FALSE );		//Error going into directory
	}

	//If there are files in the directory, DELETE THEM
	SearchHandle = FindFirstFile( pFileSpec, &sFindData);
	if( SearchHandle !=  INVALID_HANDLE_VALUE )
	{

		fDone = FALSE;
		do
		{
			// if the object is a directory
			if( GetFileAttributes( sFindData.cFileName ) == FILE_ATTRIBUTE_DIRECTORY )
			{
				// only go in if the fRecursive flag is TRUE (like Deltree)
				if (fRecursive)
				{
					sprintf(zSubdirectory, "%s\\%s", pcDirectory, sFindData.cFileName);

					if ((strcmp(sFindData.cFileName, ".") != 0) && (strcmp(sFindData.cFileName, "..") != 0))
					{
						if (!RemoveFileManDirectory(zSubdirectory, TRUE))
						{
				   		FastDebugMsg(String("RemoveFileManDirectory: ERROR - Recursive call on %s failed", zSubdirectory));
							break;
						}
					}
				}
				// otherwise, all the individual files will be deleted, but the subdirectories remain, causing
				// RemoveDirectory() at the end to fail, thus this function will return FALSE in that event (failure)
			}
			else
			{
				FileDelete( sFindData.cFileName );
			}

			//find the next file in the directory
			fRetval = FindNextFile( SearchHandle, &sFindData );
			if( fRetval == 0 )
			{
				fDone = TRUE;
			}
		}	while(!fDone);

		// very important: close the find handle, or subsequent RemoveDirectory() calls will fail
		FindClose( SearchHandle );
	}

	if( !SetFileManCurrentDirectory( zOldDir ) )
	{
		FastDebugMsg(String("RemoveFileManDirectory: ERROR - SetFileManCurrentDirectory on %s failed, error %d", zOldDir, GetLastError()));
		return( FALSE );		//Error returning from subdirectory
	}


	// The directory MUST be empty
	fRetval = RemoveDirectory( pcDirectory );
	if (!fRetval)
	{
		FastDebugMsg(String("RemoveFileManDirectory: ERROR - RemoveDirectory on %s failed, error %d", pcDirectory, GetLastError()));
	}

	return fRetval;
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Removes ALL FILES in the specified directory but leaves the directory alone.  Does not affect any subdirectories!
// Use RemoveFilemanDirectory() to also delete the directory itself, or to recursively delete subdirectories.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOLEAN EraseDirectory(const char *pcDirectory)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	WIN32_FIND_DATA sFindData;
	HANDLE		SearchHandle;
	const CHAR8	*pFileSpec = "*.*";
	BOOLEAN	fDone = FALSE;
	CHAR8		zOldDir[512];

	GetFileManCurrentDirectory( zOldDir );

	if( !SetFileManCurrentDirectory( pcDirectory ) )
	{
		FastDebugMsg(String("EraseDirectory: ERROR - SetFileManCurrentDirectory on %s failed, error %d", pcDirectory, GetLastError()));
		return( FALSE );		//Error going into directory
	}

	//If there are files in the directory, DELETE THEM
	SearchHandle = FindFirstFile( pFileSpec, &sFindData);
	if( SearchHandle !=  INVALID_HANDLE_VALUE )
	{

		fDone = FALSE;
		do
		{
			// if it's a file, not a directory
			if( GetFileAttributes( sFindData.cFileName ) != FILE_ATTRIBUTES_DIRECTORY )
			{
				FileDelete( sFindData.cFileName );
			}

			//find the next file in the directory
			if ( !FindNextFile( SearchHandle, &sFindData ))
			{
				fDone = TRUE;
			}
		} while(!fDone);

		// very important: close the find handle, or subsequent RemoveDirectory() calls will fail
		FindClose( SearchHandle );
	}

	if( !SetFileManCurrentDirectory( zOldDir ) )
	{
		FastDebugMsg(String("EraseDirectory: ERROR - SetFileManCurrentDirectory on %s failed, error %d", zOldDir, GetLastError()));
		return( FALSE );		//Error returning from directory
	}

	return( TRUE );
#endif
}


const char* GetExecutableDirectory(void)
{
	return LocalPath;
}


static void W32toSGPFileFind(GETFILESTRUCT* pGFStruct, Glob* pW32Struct);


BOOLEAN GetFileFirst(const char *pSpec, GETFILESTRUCT *pGFStruct )
{
	INT32 x,iWhich=0;
	BOOLEAN fFound;

	CHECKF( pSpec != NULL );
	CHECKF( pGFStruct != NULL );

	fFound = FALSE;
	for( x = 0; x < 20 && !fFound; x++)
	{
		if( !fFindInfoInUse[x] )
		{
			iWhich = x;
			fFound = TRUE;
		}
	}

	if ( !fFound )
		return(FALSE);

	pGFStruct->iFindHandle = iWhich;

	if (glob(pSpec, 0, NULL, &Win32FindInfo[iWhich].Glob) != 0)
	{
		globfree(&Win32FindInfo[iWhich].Glob);
		return FALSE;
	}

	Win32FindInfo[iWhich].Index = 0;
	fFindInfoInUse[iWhich] = TRUE;

	W32toSGPFileFind( pGFStruct, &Win32FindInfo[iWhich] );

	return(TRUE);
}


BOOLEAN GetFileNext( GETFILESTRUCT *pGFStruct )
{
	CHECKF( pGFStruct != NULL );

	Glob* g = &Win32FindInfo[pGFStruct->iFindHandle];
	if (g->Index >= g->Glob.gl_pathc) return FALSE;
	W32toSGPFileFind(pGFStruct, g);
	return TRUE;
}

void GetFileClose( GETFILESTRUCT *pGFStruct )
{
	if ( pGFStruct == NULL )
		return;

	globfree(&Win32FindInfo[pGFStruct->iFindHandle].Glob);
	fFindInfoInUse[pGFStruct->iFindHandle] = FALSE;
}


static void W32toSGPFileFind(GETFILESTRUCT* pGFStruct, Glob* pW32Struct)
{
	UINT32 uiAttribMask;

	// Copy the filename
	const char* Start = strrchr(pW32Struct->Glob.gl_pathv[pW32Struct->Index], '/');
	Start = (Start != NULL ? Start + 1 : pW32Struct->Glob.gl_pathv[pW32Struct->Index]);
	strcpy(pGFStruct->zFileName, Start);

	// Copy the file attributes
#if 1 // XXX TODO
	pGFStruct->uiFileAttribs = FILE_IS_NORMAL;
#else
	pGFStruct->uiFileAttribs = 0;

	for( uiAttribMask = 0x80000000; uiAttribMask > 0; uiAttribMask >>= 1)
	{
		switch( pW32Struct->dwFileAttributes & uiAttribMask )
		{
			case FILE_ATTRIBUTE_ARCHIVE:
				pGFStruct->uiFileAttribs |= FILE_IS_ARCHIVE;
				break;

			case FILE_ATTRIBUTE_DIRECTORY:
				pGFStruct->uiFileAttribs |= FILE_IS_DIRECTORY;
				break;

			case FILE_ATTRIBUTE_HIDDEN:
				pGFStruct->uiFileAttribs |= FILE_IS_HIDDEN;
				break;

			case FILE_ATTRIBUTE_NORMAL:
				pGFStruct->uiFileAttribs |= FILE_IS_NORMAL;
				break;

			case FILE_ATTRIBUTE_READONLY:
				pGFStruct->uiFileAttribs |= FILE_IS_READONLY;
				break;

			case FILE_ATTRIBUTE_SYSTEM:
				pGFStruct->uiFileAttribs |= FILE_IS_SYSTEM;
				break;

			case FILE_ATTRIBUTE_TEMPORARY:
				pGFStruct->uiFileAttribs |= FILE_IS_TEMPORARY;
				break;

			case FILE_ATTRIBUTE_COMPRESSED:
				pGFStruct->uiFileAttribs |= FILE_IS_COMPRESSED;
				break;

			case FILE_ATTRIBUTE_OFFLINE:
				pGFStruct->uiFileAttribs |= FILE_IS_OFFLINE;
				break;
		}
	}
#endif
	++pW32Struct->Index;
}


UINT32 FileGetAttributes(const char *strFilename)
{
#if 1 // XXX TODO
	FIXME
	UINT32 uiFileAttrib = 0;
	struct stat sb;

	if (stat(strFilename, &sb) != 0) return 0xFFFFFFFF;

	if (S_ISDIR(sb.st_mode)) uiFileAttrib |= FILE_ATTRIBUTES_DIRECTORY;

	return uiFileAttrib;
#else
	UINT32	uiAttribs = 0;
	UINT32	uiFileAttrib = 0;

	uiAttribs = GetFileAttributes( strFilename );

	if( uiAttribs == 0xFFFFFFFF )
		return( uiAttribs );

	if( uiAttribs & FILE_ATTRIBUTE_ARCHIVE )
		uiFileAttrib |= FILE_ATTRIBUTES_ARCHIVE;

	if( uiAttribs & FILE_ATTRIBUTE_HIDDEN )
		uiFileAttrib |= FILE_ATTRIBUTES_HIDDEN;

	if( uiAttribs & FILE_ATTRIBUTE_NORMAL )
		uiFileAttrib |= FILE_ATTRIBUTES_NORMAL;

	if( uiAttribs & FILE_ATTRIBUTE_OFFLINE )
		uiFileAttrib |= FILE_ATTRIBUTES_OFFLINE;

	if( uiAttribs & FILE_ATTRIBUTE_READONLY )
		uiFileAttrib |= FILE_ATTRIBUTES_READONLY;

	if( uiAttribs & FILE_ATTRIBUTE_SYSTEM	)
		uiFileAttrib |= FILE_ATTRIBUTES_SYSTEM;

	if( uiAttribs & FILE_ATTRIBUTE_TEMPORARY )
		uiFileAttrib |= FILE_ATTRIBUTES_TEMPORARY;

	if( uiAttribs & FILE_ATTRIBUTE_DIRECTORY )
		uiFileAttrib |= FILE_ATTRIBUTES_DIRECTORY;



	return( uiFileAttrib );
#endif
}




BOOLEAN FileClearAttributes(const char *strFilename)
{
#if 1 // XXX TODO
	fprintf(stderr, "===> %s:%d: IGNORING %s(\"%s\")\n", __FILE__, __LINE__, __func__, strFilename);
	return FALSE;
	UNIMPLEMENTED();
#else
	return SetFileAttributes( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif
}


//returns true if at end of file, else false
BOOLEAN	FileCheckEndOfFile( HWFILE hFile )
{
	INT16 sLibraryID;
	UINT32 uiFileNum;
	FILE* hRealFile;
//	UINT8		Data;
	UINT32	uiOldFilePtrLoc=0;
	UINT32	uiEndOfFilePtrLoc=0;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//Get the handle to the real file
		hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

		//Get the current position of the file pointer
		uiOldFilePtrLoc = ftell(hRealFile);

		//Get the end of file ptr location
		fseek(hRealFile, 0, SEEK_END);
		uiEndOfFilePtrLoc = ftell(hRealFile);

		//reset back to the original location
		fseek(hRealFile, uiOldFilePtrLoc, SEEK_SET);

		//if the 2 pointers are the same, we are at the end of a file
		if( uiEndOfFilePtrLoc <= uiOldFilePtrLoc )
		{
			return( 1 );
		}
	}

	//else it is a library file
	else
	{
		//if the database is initialized
		if( gFileDataBase.fInitialized )
		{
			//if the library is open
			if( IsLibraryOpened( sLibraryID ) )
			{
				//if the file is opened
				if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFileID != 0 )
				{
					UINT32	uiLength;					//uiOffsetInLibrary
//					HANDLE	hLibraryFile;
//					UINT32	uiNumBytesRead;
					UINT32	uiCurPos;

					uiLength = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].pFileHeader->uiFileLength;
					uiCurPos = gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFilePosInFile;

					//if we are trying to read more data then the size of the file, return an error
					if( uiCurPos >= uiLength )
					{
						return( TRUE );
					}
				}
			}
		}
	}

	//we are not and the end of a file
	return( 0 );
}



BOOLEAN GetFileManFileTime( HWFILE hFile, SGP_FILETIME	*pCreationTime, SGP_FILETIME *pLastAccessedTime, SGP_FILETIME *pLastWriteTime )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	HANDLE	hRealFile;
	INT16 sLibraryID;
	UINT32 uiFileNum;

	SGP_FILETIME sCreationUtcFileTime;
	SGP_FILETIME sLastAccessedUtcFileTime;
	SGP_FILETIME sLastWriteUtcFileTime;

	//Initialize the passed in variables
	memset( pCreationTime, 0, sizeof( SGP_FILETIME ) );
	memset( pLastAccessedTime, 0, sizeof( SGP_FILETIME ) );
	memset( pLastWriteTime, 0, sizeof( SGP_FILETIME ) );


	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its a real file, read the data from the file
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//get the real file handle to the file
		hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle;

		//Gets the UTC file time for the 'real' file
		GetFileTime( hRealFile, &sCreationUtcFileTime, &sLastAccessedUtcFileTime, &sLastWriteUtcFileTime );

		//converts the creation UTC file time to the current time used for the file
		FileTimeToLocalFileTime( &sCreationUtcFileTime, pCreationTime );

		//converts the accessed UTC file time to the current time used for the file
		FileTimeToLocalFileTime( &sLastAccessedUtcFileTime, pLastAccessedTime );

		//converts the write UTC file time to the current time used for the file
		FileTimeToLocalFileTime( &sLastWriteUtcFileTime, pLastWriteTime );
	}
	else
	{
		//if the database is initialized
		if( gFileDataBase.fInitialized )
		{
			//if the library is open
			if( IsLibraryOpened( sLibraryID ) )
			{
				//if the file is opened
				if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFileID != 0 )
				{
					if( !GetLibraryFileTime( sLibraryID, uiFileNum, pLastWriteTime ) )
					{
						return( FALSE );
					}
				}
			}
		}
	}

	return( TRUE );
#endif
}


INT32	CompareSGPFileTimes( SGP_FILETIME	*pFirstFileTime, SGP_FILETIME *pSecondFileTime )
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	return( CompareFileTime( pFirstFileTime, pSecondFileTime ) );
#endif
}


UINT32 FileSize(const char *strFilename)
{
HWFILE hFile;
UINT32 uiSize;

	hFile = FileOpen(strFilename, FILE_OPEN_EXISTING | FILE_ACCESS_READ);
	if (hFile == 0)
		return(0);

	uiSize=FileGetSize(hFile);
	FileClose(hFile);

	return(uiSize);
}


FILE* GetRealFileHandleFromFileManFileHandle( HWFILE hFile )
{
	INT16 sLibraryID;
	UINT32 uiFileNum;

	GetLibraryAndFileIDFromLibraryFileHandle( hFile, &sLibraryID, &uiFileNum );

	//if its the 'real file' library
	if( sLibraryID == REAL_FILE_LIBRARY_ID )
	{
		//if its not already closed
		if( gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].uiFileID != 0 )
		{
			return( gFileDataBase.RealFiles.pRealFilesOpen[ uiFileNum ].hRealFileHandle );
		}
	}
	else
	{
		//if the file is not opened, dont close it
		if( gFileDataBase.pLibraries[ sLibraryID ].pOpenFiles[ uiFileNum ].uiFileID != 0 )
		{
			return( gFileDataBase.pLibraries[ sLibraryID ].hLibraryHandle );
		}
	}
	return NULL;
}


static UINT32 GetFreeSpaceOnHardDrive(const char* pzDriveLetter);


UINT32 GetFreeSpaceOnHardDriveWhereGameIsRunningFrom( )
{
#if 1 // XXX TODO
	FIXME
	return 1024 * 1024 * 1024; // XXX TODO return an arbitrary number for now
#else
  STRING512		zDrive;
	STRING512		zDir;
	STRING512		zFileName;
	STRING512		zExt;

	UINT32 uiFreeSpace = 0;

	//get the drive letter from the exec dir
	_splitpath(GetExecutableDirectory(), zDrive, zDir, zFileName, zExt);

	sprintf( zDrive, "%s\\", zDrive );

	uiFreeSpace = GetFreeSpaceOnHardDrive( zDrive );

	return( uiFreeSpace );
#endif
}


static UINT32 GetFreeSpaceOnHardDrive(const char* pzDriveLetter)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	UINT32			uiBytesFree=0;

	UINT32			uiSectorsPerCluster=0;
	UINT32			uiBytesPerSector=0;
	UINT32			uiNumberOfFreeClusters=0;
	UINT32			uiTotalNumberOfClusters=0;

	if( !GetDiskFreeSpace( pzDriveLetter, &uiSectorsPerCluster, &uiBytesPerSector, &uiNumberOfFreeClusters, &uiTotalNumberOfClusters ) )
	{
		UINT32 uiLastError = GetLastError();
		char zString[1024];
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);

		return( TRUE );
	}

	uiBytesFree = uiBytesPerSector * uiNumberOfFreeClusters * uiSectorsPerCluster;

	return( uiBytesFree );
#endif
}
