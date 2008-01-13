#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "Config.h"
#include "Debug.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "WCheck.h"

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
#	include "Stubs.h" // XXX
#endif


//The FileDatabaseHeader
DatabaseManagerHeaderStruct gFileDataBase;


#ifdef _WIN32

static HANDLE hFindInfoHandle[20];

#else

struct Glob
{
	glob_t Glob;
	UINT32 Index;
};
typedef struct Glob Glob;

static Glob Win32FindInfo[20];

#endif


static BOOLEAN fFindInfoInUse[20];


static char LocalPath[512];
static const config_entry* BinDataDir;

static void TellAboutDataDir(const char* ConfigFile)
{
	FILE* const IniFile = fopen(ConfigFile, "a");
	if (IniFile != NULL)
	{
		fprintf(IniFile, "#Tells ja2-stracciatella where the binary datafiles are located\ndata_dir = /some/place/where/the/data/is");
		fclose(IniFile);
		fprintf(stderr, "Please edit \"%s\" to point to the binary data.\n", ConfigFile);
	}
}


BOOLEAN InitializeFileManager(void)
{
#ifdef _WIN32
	for (UINT i = 0; i < lengthof(hFindInfoHandle); ++i)
	{
		hFindInfoHandle[i] = INVALID_HANDLE_VALUE;
	}

	char home[MAX_PATH];
	if (FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, home)))
	{
		fprintf(stderr, "Unable to locate home directory\n");
		return FALSE;
	}

#	define LOCALDIR "JA2"

#else
	const char* home = getenv("HOME");
	if (home == NULL)
	{
		const struct passwd* const passwd = getpwuid(getuid());
		if (passwd == NULL || passwd->pw_dir == NULL)
		{
			fprintf(stderr, "Unable to locate home directory\n");
			return FALSE;
		}

		home = passwd->pw_dir;
	}

#	define LOCALDIR ".ja2"

#endif

	snprintf(LocalPath, lengthof(LocalPath), "%s/" LOCALDIR, home);
	if (mkdir(LocalPath, 0700) != 0 && errno != EEXIST)
	{
		fprintf(stderr, "Unable to create directory \"%s\"\n", LocalPath);
		return FALSE;
	}

	char DataPath[512];
	snprintf(DataPath, lengthof(DataPath), "%s/Data", LocalPath);
	if (mkdir(DataPath, 0700) != 0 && errno != EEXIST)
	{
		fprintf(stderr, "Unable to create directory \"%s\"\n", DataPath);
		return FALSE;
	}
	BinDataDir = ConfigRegisterKey("data_dir");

	char ConfigFile[512];
	snprintf(ConfigFile, lengthof(ConfigFile), "%s/ja2.ini", LocalPath);
	if (ConfigParseFile(ConfigFile))
	{
		fprintf(stderr, "Couldn't open configfile (\"%s\").\n", ConfigFile);
		TellAboutDataDir(ConfigFile);
		return FALSE;
	}

	if (GetBinDataPath() == NULL)
	{
		fprintf(stderr, "Path to binary data is not set.\n");
		TellAboutDataDir(ConfigFile);
		return FALSE;
	}
	return TRUE;
}


BOOLEAN FileExists(const char* const filename)
{
	BOOLEAN fExists = FileExistsNoDB(filename);
	if (!fExists)
	{
		fExists = CheckIfFileExistInLibrary(filename);
	}
	return fExists;
}


BOOLEAN FileExistsNoDB(const char* const filename)
{
	FILE* file = fopen(filename, "rb");
	if (file == NULL)
	{
		char Path[512];
		snprintf(Path, lengthof(Path), "%s/Data/%s", GetBinDataPath(), filename);
		file = fopen(Path, "rb");
		if (file == NULL) return FALSE;
	}

	fclose(file);
	return TRUE;
}


BOOLEAN FileDelete(const char* const path)
{
	return unlink(path) == 0 || errno == ENOENT;
}


HWFILE FileOpen(const char* const filename, const UINT32 uiOptions)
{
	const char* dwAccess = 0;
	if (uiOptions & FILE_ACCESS_READ && uiOptions & FILE_ACCESS_WRITE) {
		dwAccess = "r+b";
	} else if (uiOptions & FILE_ACCESS_READ) {
		dwAccess = "rb";
	} else if (uiOptions & FILE_ACCESS_WRITE) {
		dwAccess = "r+b"; // XXX HACK do not truncate the file
	} else {
		dwAccess = "";
		abort(); // XXX something is fishy
	}

	HWFILE hFile = 0;
	/* check if the file exists - note that we use the function FileExistsNoDB
	 * because it doesn't check the databases, and we don't want to do that here
	 */
	if (FileExistsNoDB(filename))
	{
		FILE* hRealFile = fopen(filename, dwAccess);
		if (hRealFile == NULL)
		{
			char Path[512];
			snprintf(Path, lengthof(Path), "%s/Data/%s", GetBinDataPath(), filename);
			hRealFile = fopen(Path, dwAccess);
			if (hRealFile == NULL) return 0;
		}

		//create a file handle for the 'real file'
		hFile = CreateRealFileHandle(hRealFile);
	}
	else if (gFileDataBase.fInitialized) // if the file did not exist, try to open it from the database
	{
		//if the file doesnt exist on the harddrive, but it is to be created, dont try to load it from the file database
		if (!(uiOptions & FILE_ACCESS_WRITE))
		{
			//If the file is in the library, get a handle to it.
			return OpenFileFromLibrary(filename);
		}
	}

	if (hFile) return hFile;

#if 1
	FIXME
	FILE* const hRealFile = fopen(filename, "wb");
	if (hRealFile == NULL) return 0;
#else
	DWORD dwCreationFlags;
	if (uiOptions & FILE_CREATE_ALWAYS)
	{
		dwCreationFlags = CREATE_ALWAYS;
	}
	else if (uiOptions & FILE_OPEN_ALWAYS)
	{
		dwCreationFlags = OPEN_ALWAYS;
	}
	else
	{
		dwCreationFlags = OPEN_EXISTING;
	}

	hRealFile = CreateFile(filename, dwAccess, 0, NULL, dwCreationFlags, dwFlagsAndAttributes, NULL);
	if (hRealFile == INVALID_HANDLE_VALUE)
	{
		UINT32 uiLastError = GetLastError();
		char zString[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);
		return 0;
	}
#endif

	return CreateRealFileHandle(hRealFile);
}


void FileClose(const HWFILE hFile)
{
	INT16 sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		RealFileHeaderStruct* const rfh = &gFileDataBase.RealFiles;
		// if its not already closed
		if (rfh->pRealFilesOpen[uiFileNum] != NULL)
		{
			fclose(rfh->pRealFilesOpen[uiFileNum]);
			rfh->pRealFilesOpen[uiFileNum] = NULL;
			Assert(gFileDataBase.RealFiles.iNumFilesOpen > 0);
			--gFileDataBase.RealFiles.iNumFilesOpen;
		}
	}
	else
	{
		CloseLibraryFile(hFile);
	}
}


#ifdef JA2TESTVERSION
#	include "Timer_Control.h"
extern UINT32 uiTotalFileReadTime;
extern UINT32 uiTotalFileReadCalls;
#endif

BOOLEAN FileRead(const HWFILE hFile, void* const pDest, const UINT32 uiBytesToRead)
{
#ifdef JA2TESTVERSION
	UINT32 uiStartTime = GetJA2Clock();
#endif

	INT16 sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	BOOLEAN	fRet = FALSE;
	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		if (uiFileNum != 0) // if the file is opened
		{
			FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
			fRet = (fread(pDest, uiBytesToRead, 1, hRealFile) == 1);
		}
	}
	else
	{
		fRet = LoadDataFromLibrary(sLibraryID, uiFileNum, pDest, uiBytesToRead);
	}

#ifdef JA2TESTVERSION
	//Add the time that we spent in this function to the total.
	uiTotalFileReadTime += GetJA2Clock() - uiStartTime;
	uiTotalFileReadCalls++;
#endif

	return fRet;
}


BOOLEAN FileWrite(HWFILE hFile, const void* pDest, UINT32 uiBytesToWrite)
{
	INT16 sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	// we cannot write to a library file
	if (sLibraryID != REAL_FILE_LIBRARY_ID) return FALSE;

	FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
	return fwrite(pDest, uiBytesToWrite, 1, hRealFile) == 1;
}


BOOLEAN FileSeek(const HWFILE hFile, INT32 distance, const INT how)
{
	INT16 sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

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

		FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		return fseek(hRealFile, distance, whence) == 0;
	}
	else
	{
		LibraryFileSeek(sLibraryID, uiFileNum, distance, how);
	}

	return TRUE;
}


INT32 FileGetPos(const HWFILE hFile)
{
	INT16 sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		return ftell(hRealFile);
	}
	else
	{
		//if the library is open
		if (IsLibraryOpened(sLibraryID))
		{
			const FileOpenStruct* const fo = &gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum];
			if (fo->pFileHeader != NULL) // if the file is open
			{
				const UINT32 uiPositionInFile = fo->uiFilePosInFile;
				return uiPositionInFile;
			}
		}
	}

	return BAD_INDEX;
}


UINT32 FileGetSize(const HWFILE hFile)
{
	INT16 sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	UINT32 uiFileSize = 0xFFFFFFFF;
	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		FILE* const hRealHandle = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		const long here = ftell(hRealHandle);
		fseek(hRealHandle, 0, SEEK_END);
		uiFileSize = ftell(hRealHandle);
		fseek(hRealHandle, here, SEEK_SET);
	}
	else
	{
		if (IsLibraryOpened(sLibraryID))
		{
			uiFileSize = gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileLength;
		}
	}

	return uiFileSize == 0xFFFFFFFF ? 0 : uiFileSize;
}


BOOLEAN SetFileManCurrentDirectory(const char* const pcDirectory)
{
#if 1 // XXX TODO
	return chdir(pcDirectory) == 0;
#else
	return SetCurrentDirectory(pcDirectory);
#endif
}


BOOLEAN GetFileManCurrentDirectory(STRING512 pcDirectory)
{
#if 1 // XXX TODO
	return getcwd(pcDirectory, sizeof(STRING512)) != NULL;
#else
	return GetCurrentDirectory(512, pcDirectory) != 0;
#endif
}


BOOLEAN MakeFileManDirectory(const char* const pcDirectory)
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
#	if defined WITH_FIXMES
	fprintf(stderr, "===> %s:%d: IGNORING %s(\"%s\", %s)\n", __FILE__, __LINE__, __func__, pcDirectory, fRecursive ? "TRUE" : "FALSE");
#	endif
	return FALSE;
	UNIMPLEMENTED
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
	UNIMPLEMENTED
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


#ifdef _WIN32
static void W32toSGPFileFind(GETFILESTRUCT* pGFStruct, WIN32_FIND_DATA* pW32Struct);
#else
static void W32toSGPFileFind(GETFILESTRUCT* pGFStruct, Glob* pW32Struct);
#endif


BOOLEAN GetFileFirst(const char* const pSpec, GETFILESTRUCT* const pGFStruct)
{
	CHECKF(pSpec     != NULL);
	CHECKF(pGFStruct != NULL);

	INT32   iWhich = 0;
	BOOLEAN fFound = FALSE;
	for (INT32 x = 0; x < 20; ++x)
	{
		if (!fFindInfoInUse[x])
		{
			iWhich = x;
			fFound = TRUE;
			break;
		}
	}

	if (!fFound) return FALSE;

	pGFStruct->iFindHandle = iWhich;

#ifdef _WIN32
	WIN32_FIND_DATA Win32FindInfo;
	hFindInfoHandle[iWhich] = FindFirstFile(pSpec, &Win32FindInfo);
	if (hFindInfoHandle[iWhich] == INVALID_HANDLE_VALUE) return FALSE;
	W32toSGPFileFind(pGFStruct, &Win32FindInfo);
#else
	if (glob(pSpec, 0, NULL, &Win32FindInfo[iWhich].Glob) != 0)
	{
		globfree(&Win32FindInfo[iWhich].Glob);
		return FALSE;
	}
	Win32FindInfo[iWhich].Index = 0;
	W32toSGPFileFind(pGFStruct, &Win32FindInfo[iWhich]);
#endif

	fFindInfoInUse[iWhich] = TRUE;
	return TRUE;
}


BOOLEAN GetFileNext(GETFILESTRUCT* const pGFStruct)
{
	CHECKF(pGFStruct != NULL);

#ifdef _WIN32
	WIN32_FIND_DATA Win32FindInfo;
	if (!FindNextFile(hFindInfoHandle[pGFStruct->iFindHandle], &Win32FindInfo))
	{
		return FALSE;
	}
	W32toSGPFileFind(pGFStruct, &Win32FindInfo);
#else
	Glob* const g = &Win32FindInfo[pGFStruct->iFindHandle];
	if (g->Index >= g->Glob.gl_pathc) return FALSE;
	W32toSGPFileFind(pGFStruct, g);
#endif
	return TRUE;
}


void GetFileClose(GETFILESTRUCT* const pGFStruct)
{
	if (pGFStruct == NULL) return;

#ifdef _WIN32
	FindClose(hFindInfoHandle[pGFStruct->iFindHandle]);
	hFindInfoHandle[pGFStruct->iFindHandle] = INVALID_HANDLE_VALUE;
#else
	globfree(&Win32FindInfo[pGFStruct->iFindHandle].Glob);
#endif
	fFindInfoInUse[pGFStruct->iFindHandle] = FALSE;
}


#ifdef _WIN32
static void W32toSGPFileFind(GETFILESTRUCT* pGFStruct, WIN32_FIND_DATA* pW32Struct)
#else
static void W32toSGPFileFind(GETFILESTRUCT* pGFStruct, Glob* pW32Struct)
#endif
{
	UINT32 uiAttribMask;

	// Copy the filename
#ifdef _WIN32
	strcpy(pGFStruct->zFileName, pW32Struct->cFileName);
#else
	const char* start = strrchr(pW32Struct->Glob.gl_pathv[pW32Struct->Index], '/');
	start = (start != NULL ? start + 1 : pW32Struct->Glob.gl_pathv[pW32Struct->Index]);
	strcpy(pGFStruct->zFileName, start);
#endif

	// Copy the file attributes
#ifdef _WIN32
	pGFStruct->uiFileAttribs = 0;

	for (uiAttribMask = 0x80000000; uiAttribMask > 0; uiAttribMask >>= 1)
	{
		switch (pW32Struct->dwFileAttributes & uiAttribMask)
		{
			case FILE_ATTRIBUTE_ARCHIVE:    pGFStruct->uiFileAttribs |= FILE_IS_ARCHIVE;    break;
			case FILE_ATTRIBUTE_DIRECTORY:  pGFStruct->uiFileAttribs |= FILE_IS_DIRECTORY;  break;
			case FILE_ATTRIBUTE_HIDDEN:     pGFStruct->uiFileAttribs |= FILE_IS_HIDDEN;     break;
			case FILE_ATTRIBUTE_NORMAL:     pGFStruct->uiFileAttribs |= FILE_IS_NORMAL;     break;
			case FILE_ATTRIBUTE_READONLY:   pGFStruct->uiFileAttribs |= FILE_IS_READONLY;   break;
			case FILE_ATTRIBUTE_SYSTEM:     pGFStruct->uiFileAttribs |= FILE_IS_SYSTEM;     break;
			case FILE_ATTRIBUTE_TEMPORARY:  pGFStruct->uiFileAttribs |= FILE_IS_TEMPORARY;  break;
			case FILE_ATTRIBUTE_COMPRESSED: pGFStruct->uiFileAttribs |= FILE_IS_COMPRESSED; break;
			case FILE_ATTRIBUTE_OFFLINE:    pGFStruct->uiFileAttribs |= FILE_IS_OFFLINE;    break;
		}
	}
#else
	pGFStruct->uiFileAttribs = FILE_IS_NORMAL; // XXX TODO
	++pW32Struct->Index;
#endif
}


UINT32 FileGetAttributes(const char* const strFilename)
{
#ifndef _WIN32 // XXX TODO
	FIXME
	struct stat sb;
	if (stat(strFilename, &sb) != 0) return 0xFFFFFFFF;

	UINT32 uiFileAttrib = 0;
	if (S_ISDIR(sb.st_mode)) uiFileAttrib |= FILE_ATTRIBUTES_DIRECTORY;
	return uiFileAttrib;
#else
	const UINT32 uiAttribs = GetFileAttributes(strFilename);

	if (uiAttribs == 0xFFFFFFFF) return uiAttribs;

	UINT32 uiFileAttrib = 0;
	if (uiAttribs & FILE_ATTRIBUTE_ARCHIVE)   uiFileAttrib |= FILE_ATTRIBUTES_ARCHIVE;
	if (uiAttribs & FILE_ATTRIBUTE_HIDDEN)    uiFileAttrib |= FILE_ATTRIBUTES_HIDDEN;
	if (uiAttribs & FILE_ATTRIBUTE_NORMAL)    uiFileAttrib |= FILE_ATTRIBUTES_NORMAL;
	if (uiAttribs & FILE_ATTRIBUTE_OFFLINE)   uiFileAttrib |= FILE_ATTRIBUTES_OFFLINE;
	if (uiAttribs & FILE_ATTRIBUTE_READONLY)  uiFileAttrib |= FILE_ATTRIBUTES_READONLY;
	if (uiAttribs & FILE_ATTRIBUTE_SYSTEM)    uiFileAttrib |= FILE_ATTRIBUTES_SYSTEM;
	if (uiAttribs & FILE_ATTRIBUTE_TEMPORARY) uiFileAttrib |= FILE_ATTRIBUTES_TEMPORARY;
	if (uiAttribs & FILE_ATTRIBUTE_DIRECTORY) uiFileAttrib |= FILE_ATTRIBUTES_DIRECTORY;
	return uiFileAttrib;
#endif
}


BOOLEAN FileClearAttributes(const char* const filename)
{
#if 1 // XXX TODO
#	if defined WITH_FIXMES
	fprintf(stderr, "===> %s:%d: IGNORING %s(\"%s\")\n", __FILE__, __LINE__, __func__, filename);
#	endif
	return FALSE;
	UNIMPLEMENTED
#else
	return SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);
#endif
}


//returns true if at end of file, else false
BOOLEAN	FileCheckEndOfFile(const HWFILE hFile)
{
	INT16  sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		FILE* const hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];

		//Get the current position of the file pointer
		const UINT32 uiOldFilePtrLoc = ftell(hRealFile);

		//Get the end of file ptr location
		fseek(hRealFile, 0, SEEK_END);
		UINT32 uiEndOfFilePtrLoc = ftell(hRealFile);

		//reset back to the original location
		fseek(hRealFile, uiOldFilePtrLoc, SEEK_SET);

		//if the 2 pointers are the same, we are at the end of a file
		return uiOldFilePtrLoc >= uiEndOfFilePtrLoc;
	}
	else
	{
		if (IsLibraryOpened(sLibraryID))
		{
			const FileOpenStruct* const fo = &gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum];
			if (fo->pFileHeader != NULL) // if the file is opened
			{
				const UINT32 uiLength = fo->pFileHeader->uiFileLength;
				const UINT32 uiCurPos = fo->uiFilePosInFile;
				return uiCurPos >= uiLength;
			}
		}
	}

	return FALSE;
}


BOOLEAN GetFileManFileTime(const HWFILE hFile, SGP_FILETIME* const pCreationTime, SGP_FILETIME* const pLastAccessedTime, SGP_FILETIME* const pLastWriteTime)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	//Initialize the passed in variables
	memset(pCreationTime,     0, sizeof(*pCreationTime));
	memset(pLastAccessedTime, 0, sizeof(*pLastAccessedTime));
	memset(pLastWriteTime,    0, sizeof(*pLastWriteTime));

	INT16  sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		const HANDLE hRealFile = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum].hRealFileHandle;

		//Gets the UTC file time for the 'real' file
		SGP_FILETIME sCreationUtcFileTime;
		SGP_FILETIME sLastAccessedUtcFileTime;
		SGP_FILETIME sLastWriteUtcFileTime;
		GetFileTime(hRealFile, &sCreationUtcFileTime, &sLastAccessedUtcFileTime, &sLastWriteUtcFileTime);

		//converts the creation UTC file time to the current time used for the file
		FileTimeToLocalFileTime(&sCreationUtcFileTime, pCreationTime);

		//converts the accessed UTC file time to the current time used for the file
		FileTimeToLocalFileTime(&sLastAccessedUtcFileTime, pLastAccessedTime);

		//converts the write UTC file time to the current time used for the file
		FileTimeToLocalFileTime(&sLastWriteUtcFileTime, pLastWriteTime);
		return TRUE;
	}
	else
	{
		return GetLibraryFileTime(sLibraryID, uiFileNum, pLastWriteTime);
	}
#endif
}


INT32	CompareSGPFileTimes(const SGP_FILETIME* const pFirstFileTime, const SGP_FILETIME* const pSecondFileTime)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	return CompareFileTime(pFirstFileTime, pSecondFileTime);
#endif
}


UINT32 FileSize(const char* const filename)
{
	const HWFILE hFile = FileOpen(filename, FILE_ACCESS_READ);
	if (hFile == 0) return 0;

	const UINT32 uiSize = FileGetSize(hFile);
	FileClose(hFile);

	return uiSize;
}


FILE* GetRealFileHandleFromFileManFileHandle(const HWFILE hFile)
{
	INT16  sLibraryID;
	UINT32 uiFileNum;
	GetLibraryAndFileIDFromLibraryFileHandle(hFile, &sLibraryID, &uiFileNum);

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		return gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
	}
	else
	{
		const LibraryHeaderStruct* const lh = &gFileDataBase.pLibraries[sLibraryID];
		if (lh->pOpenFiles[uiFileNum].pFileHeader != NULL) return lh->hLibraryHandle;
	}
	return NULL;
}


static UINT32 GetFreeSpaceOnHardDrive(const char* pzDriveLetter);


UINT32 GetFreeSpaceOnHardDriveWhereGameIsRunningFrom(void)
{
#if 1 // XXX TODO
	FIXME
	return 1024 * 1024 * 1024; // XXX TODO return an arbitrary number for now
#else
	//get the drive letter from the exec dir
  STRING512 zDrive;
	_splitpath(GetExecutableDirectory(), zDrive, NULL, NULL, NULL);

	sprintf(zDrive, "%s\\", zDrive);
	return GetFreeSpaceOnHardDrive(zDrive);
#endif
}


static UINT32 GetFreeSpaceOnHardDrive(const char* const pzDriveLetter)
{
#if 1 // XXX TODO
	UNIMPLEMENTED
#else
	UINT32 uiSectorsPerCluster     = 0;
	UINT32 uiBytesPerSector        = 0;
	UINT32 uiNumberOfFreeClusters  = 0;
	UINT32 uiTotalNumberOfClusters = 0;
	if (!GetDiskFreeSpace(pzDriveLetter, &uiSectorsPerCluster, &uiBytesPerSector, &uiNumberOfFreeClusters, &uiTotalNumberOfClusters))
	{
		const UINT32 uiLastError = GetLastError();
		char zString[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastError, 0, zString, 1024, NULL);
		return TRUE;
	}

	return uiBytesPerSector * uiNumberOfFreeClusters * uiSectorsPerCluster;
#endif
}


const char* GetBinDataPath(void)
{
	return ConfigGetValue(BinDataDir);
}
