#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
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

#	if defined __APPLE__ && defined __MACH__
#		include <CoreFoundation/CoreFoundation.h>
#		include <sys/param.h>
#	endif
#endif


//The FileDatabaseHeader
DatabaseManagerHeaderStruct gFileDataBase;


struct FindFileInfo
{
#ifdef _WIN32
	HANDLE          find_handle;
	WIN32_FIND_DATA find_data;
	BOOLEAN         first_done;
#else
	size_t index;
	glob_t glob_data;
#endif
};


static char LocalPath[512];
static config_entry* BinDataDir;

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


#if defined __APPLE__  && defined __MACH__

void SetBinDataDirFromBundle(void)
{
	CFBundleRef const app_bundle = CFBundleGetMainBundle();
	if (app_bundle == NULL)
	{
		fputs("WARNING: Failed to get main bundle.\n", stderr);
		return;
	}

	CFURLRef const app_url = CFBundleCopyBundleURL(app_bundle);
	if (app_url == NULL)
	{
		fputs("WARNING: Failed to get URL of bundle.\n", stderr);
		return;
	}

#define RESOURCE_PATH "/Contents/Resources/ja2"
	char app_path[PATH_MAX + lengthof(RESOURCE_PATH)];
	if (!CFURLGetFileSystemRepresentation(app_url, TRUE, (UInt8*)app_path, PATH_MAX))
	{
		fputs("WARNING: Failed to get application path.\n", stderr);
		return;
	}

	strcat(app_path, RESOURCE_PATH);
	ConfigSetValue(BinDataDir, app_path);
#undef RESOURCE_PATH
}

#endif


BOOLEAN InitializeFileManager(void)
{
#ifdef _WIN32
	_fmode = O_BINARY;

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

#if defined __APPLE__  && defined __MACH__
	SetBinDataDirFromBundle();
#endif

	char ConfigFile[512];
	snprintf(ConfigFile, lengthof(ConfigFile), "%s/ja2.ini", LocalPath);
	if (ConfigParseFile(ConfigFile))
	{
		fprintf(stderr, "WARNING: Could not open configuration file (\"%s\").\n", ConfigFile);
	}

	if (GetBinDataPath() == NULL)
	{
		fputs("ERROR: Path to binary data is not set.\n", stderr);
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
	if (unlink(path) == 0) return TRUE;

	switch (errno)
	{
		case ENOENT: return TRUE;

#ifdef _WIN32
		/* On WIN32 read-only files cannot be deleted, so try to make the file
		 * writable and unlink() again */
		case EACCES:
			return
				(chmod(path, S_IREAD | S_IWRITE) == 0 && unlink(path) == 0) ||
				errno == ENOENT;
#endif

		default: return FALSE;
	}
}


HWFILE FileOpen(const char* const filename, const UINT32 flags)
{
	const char* fmode;
	int         mode;
	switch (flags & (FILE_ACCESS_READWRITE | FILE_ACCESS_APPEND))
	{
		case FILE_ACCESS_READ:      fmode = "rb";  mode = O_RDONLY;            break;
		case FILE_ACCESS_WRITE:     fmode = "wb";  mode = O_WRONLY;            break;
		case FILE_ACCESS_READWRITE: fmode = "r+b"; mode = O_RDWR;              break;
		case FILE_ACCESS_APPEND:    fmode = "ab";  mode = O_WRONLY | O_APPEND; break;

		default: abort();
	}

	int d;
	if (flags & FILE_CREATE_ALWAYS)
	{
		d = open(filename, mode | O_CREAT | O_TRUNC, 0600);
		if (d < 0) return 0;
	}
	else if (flags & (FILE_ACCESS_WRITE | FILE_ACCESS_APPEND))
	{
		if (flags & FILE_OPEN_ALWAYS) mode |= O_CREAT;
		d = open(filename, mode, 0600);
		if (d < 0) return 0;
	}
	else
	{
		d = open(filename, mode);
		if (d < 0)
		{
			char path[512];
			snprintf(path, lengthof(path), "%s/Data/%s", GetBinDataPath(), filename);
			d = open(path, mode);
			if (d < 0)
			{
				const HWFILE h = OpenFileFromLibrary(filename);
				if (h != 0 || !(flags & FILE_OPEN_ALWAYS)) return h;

				d = open(filename, mode | O_CREAT, 0600);
				if (d < 0) return 0;
			}
		}
	}

	FILE* const f = fdopen(d, fmode);
	if (f == NULL)
	{
		close(d);
		return 0;
	}
	return CreateRealFileHandle(f);
}


void FileClose(const HWFILE hFile)
{
	CloseLibraryFile(hFile);
}


#ifdef JA2TESTVERSION
#	include "Timer_Control.h"
extern UINT32 uiTotalFileReadTime;
extern UINT32 uiTotalFileReadCalls;
#endif

BOOLEAN FileRead(const HWFILE hFile, void* const pDest, const UINT32 uiBytesToRead)
{
#ifdef JA2TESTVERSION
	const UINT32 uiStartTime = GetJA2Clock();
#endif
	const BOOLEAN ret = LoadDataFromLibrary(hFile, pDest, uiBytesToRead);
#ifdef JA2TESTVERSION
	//Add the time that we spent in this function to the total.
	uiTotalFileReadTime += GetJA2Clock() - uiStartTime;
	uiTotalFileReadCalls++;
#endif
	return ret;
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
	return LibraryFileSeek(hFile, distance, how);
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

	if (sLibraryID == REAL_FILE_LIBRARY_ID)
	{
		FILE* const f = gFileDataBase.RealFiles.pRealFilesOpen[uiFileNum];
		struct stat sb;
		if (fstat(fileno(f), &sb) != 0) return 0;
		return sb.st_size;
	}
	else
	{
		if (!IsLibraryOpened(sLibraryID)) return 0;
		return gFileDataBase.pLibraries[sLibraryID].pOpenFiles[uiFileNum].pFileHeader->uiFileLength;
	}
}


BOOLEAN SetFileManCurrentDirectory(const char* const pcDirectory)
{
#if 1 // XXX TODO
	return chdir(pcDirectory) == 0;
#else
	return SetCurrentDirectory(pcDirectory);
#endif
}


BOOLEAN MakeFileManDirectory(const char* const pcDirectory)
{
	return mkdir(pcDirectory, 0755) == 0;
}


BOOLEAN EraseDirectory(const char* const path)
{
	char pattern[512];
	snprintf(pattern, lengthof(pattern), "%s/*", path);

	FindFileInfo* const find_info = FindFiles(pattern);
	if (find_info == NULL) return FALSE;

	BOOLEAN success = FALSE;
	for (;;)
	{
		const char* const find_filename = FindFilesNext(find_info);
		if (find_filename == NULL)
		{
			success = TRUE;
			break;
		}

		char filename[512];
		snprintf(filename, lengthof(filename), "%s/%s", path, find_filename);

		if (!FileDelete(filename))
		{
			const FileAttributes attr = FileGetAttributes(filename);
			if (attr == FILE_ATTR_ERROR)     break;
			if (attr &  FILE_ATTR_DIRECTORY) continue;
			break;
		}
	}

	FindFilesFree(find_info);
	return success;
}


const char* GetExecutableDirectory(void)
{
	return LocalPath;
}


FindFileInfo* FindFiles(const char* const pattern)
{
	CHECKF(pattern != NULL);

	FindFileInfo* const gfi = MALLOCZ(FindFileInfo);
	if (gfi == NULL) return NULL;

#ifdef _WIN32
	const HANDLE h = FindFirstFile(pattern, &gfi->find_data);
	if (h != INVALID_HANDLE_VALUE)
	{
		gfi->find_handle = h;
	}
	else if (GetLastError() != ERROR_FILE_NOT_FOUND)
	{
		MemFree(gfi);
		return NULL;
	}
	return gfi;
#else
	glob_t* const g = &gfi->glob_data;
	switch (glob(pattern, GLOB_NOSORT, NULL, g))
	{
		case 0:
		case GLOB_NOMATCH:
			return gfi;

		default:
			globfree(g);
			MemFree(gfi);
			return NULL;
	}
#endif
}


const char* FindFilesNext(FindFileInfo* const gfi)
{
	CHECKN(gfi != NULL);

#ifdef _WIN32
	if (!gfi->first_done)
	{
		gfi->first_done = TRUE;
		// No match?
		if (gfi->find_handle == NULL) return NULL;
	}
	else
	{
		if (!FindNextFile(gfi->find_handle, &gfi->find_data)) return NULL;
	}
	return gfi->find_data.cFileName;
#else
	if (gfi->index >= gfi->glob_data.gl_pathc) return NULL;

	const char* const path  = gfi->glob_data.gl_pathv[gfi->index++];
	const char* const start = strrchr(path, '/');
	return start != NULL ? start + 1 : path;
#endif
}


void FindFilesFree(FindFileInfo* const gfi)
{
	CHECKV(gfi != NULL);

#ifdef _WIN32
	if (gfi->find_handle != NULL) FindClose(gfi->find_handle);
#else
	globfree(&gfi->glob_data);
#endif
	MemFree(gfi);
}


FileAttributes FileGetAttributes(const char* const filename)
{
	FileAttributes attr = FILE_ATTR_NONE;
#ifndef _WIN32 // XXX TODO
	struct stat sb;
	if (stat(filename, &sb) != 0) return FILE_ATTR_ERROR;

	if (S_ISDIR(sb.st_mode))     attr |= FILE_ATTR_DIRECTORY;
	if (!(sb.st_mode & S_IWUSR)) attr |= FILE_ATTR_READONLY;
#else
	const UINT32 w32attr = GetFileAttributes(filename);
	if (w32attr == INVALID_FILE_ATTRIBUTES) return FILE_ATTR_ERROR;

	if (w32attr & FILE_ATTRIBUTE_READONLY)  attr |= FILE_ATTR_READONLY;
	if (w32attr & FILE_ATTRIBUTE_DIRECTORY) attr |= FILE_ATTR_DIRECTORY;
#endif
	return attr;
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
