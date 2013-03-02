#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "Config.h"
#include "Directories.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "PODObj.h"
#include "Logger.h"

#if _WIN32
#include <shlobj.h>
#else
#include <pwd.h>
#endif

#include "PlatformIO.h"

#if CASE_SENSITIVE_FS
#include <dirent.h>
#endif

#define BASEDATADIR    "data"
#define LOCAL_CURRENT_DIR "tmp"

enum SGPFileFlags
{
	SGPFILE_NONE = 0U,
	SGPFILE_REAL = 1U << 0
};

struct SGPFile
{
	SGPFileFlags flags;
	union
	{
		FILE*       file;
		LibraryFile lib;
	} u;
};


static const char* GetBinDataPath(void);
static void SetFileManCurrentDirectory(char const* const pcDirectory);

#if CASE_SENSITIVE_FS
/**
 * Find an object (file or subdirectory) in the given directory in case-independent manner.
 * @return true when found, copy found name into fileNameBuf. */
static bool findObjectCaseInsensitive(const char *directory, const char *name, bool lookForFiles, bool lookForSubdirs, char *nameBuf, int nameBufSize);
#endif

/** Get file open modes from our enumeration.
 * Abort program if conversion is not found.
 * @return file mode for fopen call and posix mode using parameter 'posixMode' */
static const char* GetFileOpenModes(FileOpenFlags flags, int *posixMode);

SGP::FindFiles::FindFiles(char const* const pattern) :
#ifdef _WIN32
	first_done_()
#else
	index_()
#endif
{
#ifdef _WIN32
	HANDLE const h = FindFirstFile(pattern, &find_data_);
	if (h != INVALID_HANDLE_VALUE)
	{
		find_handle_ = h;
		return;
	}
	else if (GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		find_handle_ = 0;
		return;
	}
#else
	glob_t* const g = &glob_data_;
	switch (glob(pattern, GLOB_NOSORT, 0, g))
	{
		case 0:
		case GLOB_NOMATCH:
			return;

		default:
			globfree(g);
			break;
	}
#endif
	throw std::runtime_error("Failed to start file search");
}


SGP::FindFiles::~FindFiles()
{
#ifdef _WIN32
	if (find_handle_) FindClose(find_handle_);
#else
	globfree(&glob_data_);
#endif
}


char const* SGP::FindFiles::Next()
{
#ifdef _WIN32
	if (!first_done_)
	{
		first_done_ = true;
		// No match?
		if (!find_handle_) return 0;
	}
	else if (!FindNextFile(find_handle_, &find_data_))
	{
		if (GetLastError() == ERROR_NO_MORE_FILES) return 0;
		throw std::runtime_error("Failed to get next file in file search");
	}
	return find_data_.cFileName;
#else
	if (index_ >= glob_data_.gl_pathc) return 0;
	char const* const path  = glob_data_.gl_pathv[index_++];
	char const* const start = strrchr(path, '/');
	return start ? start + 1 : path;
#endif
}


static char         LocalPath[512];
static ConfigEntry* BinDataDir;

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


void InitializeFileManager(void)
{
#ifdef _WIN32
	char home[MAX_PATH];
	if (FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, home)))
	{
		throw std::runtime_error("Unable to locate home directory\n");
	}
#else
	const char* home = getenv("HOME");
	if (home == NULL)
	{
		const struct passwd* const passwd = getpwuid(getuid());
		if (passwd == NULL || passwd->pw_dir == NULL)
		{
			throw std::runtime_error("Unable to locate home directory");
		}

		home = passwd->pw_dir;
	}
#endif

#ifdef _WIN32
	snprintf(LocalPath, lengthof(LocalPath), "%s/JA2", home);
#else
	snprintf(LocalPath, lengthof(LocalPath), "%s/.ja2", home);
#endif

	if (mkdir(LocalPath, 0700) != 0 && errno != EEXIST)
	{
    LOG_ERROR("Unable to create directory '%s'\n", LocalPath);
		throw std::runtime_error("Unable to local directory");
	}

  // Create another directory and set is as the current directory for the process
  // Temporary files will be created in this directory.
  // ----------------------------------------------------------------------------

	char TmpPath[512];
	snprintf(TmpPath, lengthof(TmpPath), "%s/" LOCAL_CURRENT_DIR, LocalPath);
	if (mkdir(TmpPath, 0700) != 0 && errno != EEXIST)
	{
    LOG_ERROR("Unable to create tmp directory '%s'\n", TmpPath);
		throw std::runtime_error("Unable to create tmp directory");
	}
  else
  {
    SetFileManCurrentDirectory(TmpPath);
  }

  // Get directory with JA2 resources
  // --------------------------------------------

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

  LOG_INFO("Configuration file:  '%s'\n", ConfigFile);

	if (GetBinDataPath() == NULL)
	{
		TellAboutDataDir(ConfigFile);
		throw std::runtime_error("Path to binary data is not set.");
	}

  // Check that 'Data' directory exists inside the folder with JA2 resources
  // -----------------------------------------------------------------------

  if (GetDataDirPath() == NULL)
  {
    LOG_ERROR("ERROR: 'Data' directory is not found in '%s'\n", GetBinDataPath());
		throw std::runtime_error("'Data' directory is not found.");
  }

  LOG_INFO("Data directory:      '%s'\n", GetDataDirPath());
  LOG_INFO("Tilecache directory: '%s'\n", GetTilecacheDirPath());
}


bool FileExists(char const* const filename)
{
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		char path[512];
		snprintf(path, lengthof(path), "%s/%s", GetDataDirPath(), filename);
		file = fopen(path, "rb");
		if (!file) return CheckIfFileExistInLibrary(filename);
	}

	fclose(file);
	return true;
}

/**
 * Open file in the Data directory.
 *
 * Return file descriptor or -1 if file is not found. */
static int OpenFileInDataDirFD(const char *filename, int mode)
{
  char path[512];
  snprintf(path, lengthof(path), "%s/%s", GetDataDirPath(), filename);
  int d = open(path, mode);
  if (d < 0)
  {
#if CASE_SENSITIVE_FS
    // on case-sensitive file system need to try to find another name
    char newFileName[128];
    if(findObjectCaseInsensitive(GetDataDirPath(), filename, true, false, newFileName, sizeof(newFileName)))
    {
      snprintf(path, lengthof(path), "%s/%s", GetDataDirPath(), newFileName);
      d = open(path, mode);
    }
#endif
  }
  return d;
}

/** Open file in the 'Data' directory in case-insensitive manner. */
FILE* OpenFileInDataDir(const char *filename, FileOpenFlags flags)
{
	int mode;
	const char* fmode = GetFileOpenModes(flags, &mode);

  int d = OpenFileInDataDirFD(filename, mode);
  if(d >= 0)
  {
    FILE* hFile = fdopen(d, fmode);
    if (hFile == NULL)
    {
      close(d);
    }
    else
    {
      return hFile;
    }
  }
  return NULL;
}

void FileDelete(char const* const path)
{
	if (unlink(path) == 0) return;

	switch (errno)
	{
		case ENOENT: return;

#ifdef _WIN32
		/* On WIN32 read-only files cannot be deleted, so try to make the file
		 * writable and unlink() again */
		case EACCES:
			if ((chmod(path, S_IREAD | S_IWRITE) == 0 && unlink(path) == 0) ||
					errno == ENOENT)
			{
				return;
			}
			break;
#endif

		default: break;
	}

	throw std::runtime_error("Deleting file failed");
}


/** Get file open modes from our enumeration.
 * Abort program if conversion is not found.
 * @return file mode for fopen call and posix mode using parameter 'posixMode' */
static const char* GetFileOpenModes(FileOpenFlags flags, int *posixMode)
{
  const char *cMode = NULL;

#ifndef _WIN32
  *posixMode = 0;
#else
	*posixMode = O_BINARY;
#endif

	switch (flags & (FILE_ACCESS_READWRITE | FILE_ACCESS_APPEND))
	{
		case FILE_ACCESS_READ:      cMode = "rb";  *posixMode |= O_RDONLY;            break;
		case FILE_ACCESS_WRITE:     cMode = "wb";  *posixMode |= O_WRONLY;            break;
		case FILE_ACCESS_READWRITE: cMode = "r+b"; *posixMode |= O_RDWR;              break;
		case FILE_ACCESS_APPEND:    cMode = "ab";  *posixMode |= O_WRONLY | O_APPEND; break;

		default: abort();
	}
  return cMode;
}


HWFILE FileOpen(const char* const filename, const FileOpenFlags flags)
{
	int         mode;
	const char* fmode = GetFileOpenModes(flags, &mode);

	SGP::PODObj<SGPFile> file;

	int d;
	if (flags & FILE_CREATE_ALWAYS)
	{
		d = open3(filename, mode | O_CREAT | O_TRUNC, 0600);
	}
	else if (flags & (FILE_ACCESS_WRITE | FILE_ACCESS_APPEND))
	{
		if (flags & FILE_OPEN_ALWAYS) mode |= O_CREAT;
		d = open3(filename, mode, 0600);
	}
	else
	{
		d = open(filename, mode);
		if (d < 0)
		{
      // failed to open file in the local directory
      // let's try Data
      d = OpenFileInDataDirFD(filename, mode);
			if (d < 0)
			{
        // failed to open in the data dir
        // let's try libraries
				if (OpenFileFromLibrary(filename, &file->u.lib))
        {
          LOG__FILE_OPEN("Opened file (from library ): %s\n", filename);
          return file.Release();
        }

				if (flags & FILE_OPEN_ALWAYS)
				{
					d = open3(filename, mode | O_CREAT, 0600);
				}
			}
      else
      {
        LOG__FILE_OPEN("Opened file (from data dir): %s\n", filename);
      }
		}
    else
    {
      LOG__FILE_OPEN("Opened file (current dir  ): %s\n", filename);
    }
	}

	if (d < 0)
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "Opening file '%s' failed", filename);
    throw std::runtime_error(buf);
  }

	FILE* const f = fdopen(d, fmode);
	if (!f)
	{
		close(d);
		throw std::runtime_error("Opening file failed");
	}

	file->flags  = SGPFILE_REAL;
	file->u.file = f;
	return file.Release();
}


void FileClose(const HWFILE f)
{
	if (f->flags & SGPFILE_REAL)
	{
		fclose(f->u.file);
	}
	else
	{
		CloseLibraryFile(&f->u.lib);
	}
	MemFree(f);
}


#ifdef JA2TESTVERSION
#	include "Timer_Control.h"
extern UINT32 uiTotalFileReadTime;
extern UINT32 uiTotalFileReadCalls;
#endif

void FileRead(HWFILE const f, void* const pDest, UINT32 const uiBytesToRead)
{
#ifdef JA2TESTVERSION
	const UINT32 uiStartTime = GetJA2Clock();
#endif

	BOOLEAN ret;
	if (f->flags & SGPFILE_REAL)
	{
		ret = fread(pDest, uiBytesToRead, 1, f->u.file) == 1;
	}
	else
	{
		ret = LoadDataFromLibrary(&f->u.lib, pDest, uiBytesToRead);
	}

#ifdef JA2TESTVERSION
	//Add the time that we spent in this function to the total.
	uiTotalFileReadTime += GetJA2Clock() - uiStartTime;
	uiTotalFileReadCalls++;
#endif

	if (!ret) throw std::runtime_error("Reading from file failed");
}


void FileWrite(HWFILE const f, void const* const pDest, UINT32 const uiBytesToWrite)
{
	if (!(f->flags & SGPFILE_REAL)) throw std::logic_error("Tried to write to library file");
	if (fwrite(pDest, uiBytesToWrite, 1, f->u.file) != 1) throw std::runtime_error("Writing to file failed");
}


void FileSeek(HWFILE const f, INT32 distance, FileSeekMode const how)
{
	bool success;
	if (f->flags & SGPFILE_REAL)
	{
		int whence;
		switch (how)
		{
			case FILE_SEEK_FROM_START: whence = SEEK_SET; break;
			case FILE_SEEK_FROM_END:   whence = SEEK_END; break;
			default:                   whence = SEEK_CUR; break;
		}

		success = fseek(f->u.file, distance, whence) == 0;
	}
	else
	{
		success = LibraryFileSeek(&f->u.lib, distance, how);
	}
	if (!success) throw std::runtime_error("Seek in file failed");
}


INT32 FileGetPos(const HWFILE f)
{
	return f->flags & SGPFILE_REAL ? ftell(f->u.file) : f->u.lib.uiFilePosInFile;
}


UINT32 FileGetSize(const HWFILE f)
{
	if (f->flags & SGPFILE_REAL)
	{
		struct stat sb;
		if (fstat(fileno(f->u.file), &sb) != 0)
		{
			throw std::runtime_error("Getting file size failed");
		}
		return sb.st_size;
	}
	else
	{
		return f->u.lib.pFileHeader->uiFileLength;
	}
}


static void SetFileManCurrentDirectory(char const* const pcDirectory)
{
#if 1 // XXX TODO
	if (chdir(pcDirectory) != 0)
#else
	if (!SetCurrentDirectory(pcDirectory))
#endif
	{
		throw std::runtime_error("Changing directory failed");
	}
}


void MakeFileManDirectory(char const* const path)
{
	if (mkdir(path, 0755) == 0) return;

	if (errno == EEXIST)
	{
		FileAttributes const attr = FileGetAttributes(path);
		if (attr != FILE_ATTR_ERROR && attr & FILE_ATTR_DIRECTORY) return;
	}

	throw std::runtime_error("Failed to create directory");
}


void EraseDirectory(char const* const path)
{
	char pattern[512];
	snprintf(pattern, lengthof(pattern), "%s/*", path);

	SGP::FindFiles find(pattern);
	while (char const* const find_filename = find.Next())
	{
		char filename[512];
		snprintf(filename, lengthof(filename), "%s/%s", path, find_filename);

		try
		{
			FileDelete(filename);
		}
		catch (...)
		{
			const FileAttributes attr = FileGetAttributes(filename);
			if (attr != FILE_ATTR_ERROR && attr & FILE_ATTR_DIRECTORY) continue;
			throw;
		}
	}
}


const char* GetExecutableDirectory(void)
{
	return LocalPath;
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


BOOLEAN GetFileManFileTime(const HWFILE f, SGP_FILETIME* const pCreationTime, SGP_FILETIME* const pLastAccessedTime, SGP_FILETIME* const pLastWriteTime)
{
#if 1 // XXX TODO
	UNIMPLEMENTED;
  return FALSE;
#else
	//Initialize the passed in variables
	memset(pCreationTime,     0, sizeof(*pCreationTime));
	memset(pLastAccessedTime, 0, sizeof(*pLastAccessedTime));
	memset(pLastWriteTime,    0, sizeof(*pLastWriteTime));

	if (f->flags & SGPFILE_REAL)
	{
		const HANDLE hRealFile = f->u.file;

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
		return GetLibraryFileTime(&f->u.lib, pLastWriteTime);
	}
#endif
}


INT32	CompareSGPFileTimes(const SGP_FILETIME* const pFirstFileTime, const SGP_FILETIME* const pSecondFileTime)
{
#if 1 // XXX TODO
	UNIMPLEMENTED;
  return 0;
#else
	return CompareFileTime(pFirstFileTime, pSecondFileTime);
#endif
}


FILE* GetRealFileHandleFromFileManFileHandle(const HWFILE f)
{
	return f->flags & SGPFILE_REAL ? f->u.file : f->u.lib.lib->hLibraryHandle;
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

#if CASE_SENSITIVE_FS

/** Join two path components. */
static void joinPaths(const char *first, const char *second, char *outputBuf, int outputBufSize)
{
  strncpy(outputBuf, first, outputBufSize);
  if(first[strlen(first) - 1] != '/')
  {
    strncat(outputBuf, "/", outputBufSize);
  }
  strncat(outputBuf, second, outputBufSize);
}

/**
 * Find an object (file or subdirectory) in the given directory in case-independent manner.
 * @return true when found, copy found name into fileNameBuf. */
static bool findObjectCaseInsensitive(const char *directory, const char *name, bool lookForFiles, bool lookForSubdirs, char *nameBuf, int nameBufSize)
{
  bool result = false;

  // if name contains directories, than we have to find actual case-sensitive name of the directory
  // and only then look for a file
  const char *splitter = strstr(name, "/");
  int dirNameLen = splitter - name;
  if(splitter && (dirNameLen > 0) && splitter[1] != 0)
  {
    // we have directory in the name
    // let's find its correct name first
    char newDirectory[128];
    char actualSubdirName[128];
    strncpy(newDirectory, name, sizeof(newDirectory));
    newDirectory[dirNameLen] = 0;

    if(findObjectCaseInsensitive(directory, newDirectory, false, true, actualSubdirName, sizeof(actualSubdirName)))
    {
      // found subdirectory; let's continue the full search
      char pathInSubdir[128];
      joinPaths(directory, actualSubdirName, newDirectory, sizeof(newDirectory));
      if(findObjectCaseInsensitive(newDirectory, splitter + 1, lookForFiles, lookForSubdirs, pathInSubdir, sizeof(pathInSubdir)))
      {
        // found name in subdir
        joinPaths(actualSubdirName, pathInSubdir, nameBuf, nameBufSize);
        result = true;
      }
    }
  }
  else
  {
    // name contains only file, no directories
    DIR *d;
    struct dirent *entry;
    uint8_t objectTypes = (lookForFiles ? DT_REG : 0) | (lookForSubdirs ? DT_DIR : 0);

    d = opendir(directory);
    if (d)
    {
      while ((entry = readdir(d)) != NULL)
      {
        if((entry->d_type & objectTypes)
           && !strcasecmp(name, entry->d_name))
        {
          // found
          strncpy(nameBuf, entry->d_name, nameBufSize);
          nameBuf[nameBufSize - 1] = 0;
          result = true;
        }
      }
      closedir(d);
    }
  }

  // LOG_INFO("XXXXX Looking for %s/[ %s ] : %s\n", directory, name, result ? "success" : "failure");
  return result;
}
#endif

static char *s_dataDir = NULL;
static char *s_tileDir = NULL;
static char s_dataDirBuf[256] = "";
static char s_tileDirBuf[256] = "";
static bool s_needToFindDataDirs = true;


/**
 * Find actual paths to directories 'Data' and 'Data/Tilecache'.
 * On case-sensitive filesystems that might be tricky.
 */
static void findDataDirs()
{
#if !CASE_SENSITIVE_FS
    snprintf(s_dataDirBuf, lengthof(s_dataDirBuf), "%s/%s",    GetBinDataPath(), BASEDATADIR);
    snprintf(s_tileDirBuf, lengthof(s_tileDirBuf), "%s/%s/%s", GetBinDataPath(), BASEDATADIR, TILECACHEDIR);
    s_dataDir = s_dataDirBuf;
    s_tileDir = s_tileDirBuf;
#else
    s_dataDir = NULL;
    s_tileDir = NULL;
    char name[128];
    if(findObjectCaseInsensitive(GetBinDataPath(), BASEDATADIR, false, true, name, sizeof(name)))
    {
      snprintf(s_dataDirBuf, lengthof(s_dataDirBuf), "%s/%s", GetBinDataPath(), name);
      s_dataDir = s_dataDirBuf;
      if(findObjectCaseInsensitive(s_dataDir, TILECACHEDIR, false, true, name, sizeof(name)))
      {
        snprintf(s_tileDirBuf, lengthof(s_tileDirBuf), "%s/%s", s_dataDir, name);
        s_tileDir = s_tileDirBuf;
      }
    }
#endif
}

/** Get path to the 'Data' directory of the game. */
const char* GetDataDirPath()
{
  if(s_needToFindDataDirs)
  {
    findDataDirs();
  }
  return s_dataDir;
}

/** Get path to the 'Data/Tilecache' directory of the game. */
const char* GetTilecacheDirPath()
{
  if(s_needToFindDataDirs)
  {
    findDataDirs();
  }
  return s_tileDir;
}

