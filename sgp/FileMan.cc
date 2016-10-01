#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "PODObj.h"

#include "boost/filesystem.hpp"

#include "slog/slog.h"

#if _WIN32
#include <shlobj.h>
#else
#include <pwd.h>
#endif

#include "PlatformIO.h"

#if MACOS_USE_RESOURCES_FROM_BUNDLE && defined __APPLE__  && defined __MACH__
#include <CoreFoundation/CFBundle.h>
#endif

#if CASE_SENSITIVE_FS
#include <dirent.h>
#endif

// XXX: remove FileMan class and make it into a namespace

#define LOCAL_CURRENT_DIR "tmp"

enum FileOpenFlags
{
	FILE_ACCESS_READ      = 1U << 0,
	FILE_ACCESS_WRITE     = 1U << 1,
	FILE_ACCESS_READWRITE = FILE_ACCESS_READ | FILE_ACCESS_WRITE,
	FILE_ACCESS_APPEND    = 1U << 2
};
ENUM_BITSET(FileOpenFlags)


static void SetFileManCurrentDirectory(char const* const pcDirectory);

/** Get file open modes from our enumeration.
 * Abort program if conversion is not found.
 * @return file mode for fopen call and posix mode using parameter 'posixMode' */
static const char* GetFileOpenModes(FileOpenFlags flags, int *posixMode);

#if MACOS_USE_RESOURCES_FROM_BUNDLE && defined __APPLE__  && defined __MACH__

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

/** Find config folder and switch into it. */
std::string FileMan::findConfigFolderAndSwitchIntoIt()
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
  std::string configFolderPath = FileMan::joinPaths(home, "JA2");
#else
  std::string configFolderPath = FileMan::joinPaths(home, ".ja2");
#endif

	if (mkdir(configFolderPath.c_str(), 0700) != 0 && errno != EEXIST)
	{
	  SLOGE(DEBUG_TAG_FILEMAN, "Unable to create directory '%s'", configFolderPath.c_str());
	  throw std::runtime_error("Unable to create local directory");
	}

  // Create another directory and set is as the current directory for the process
  // Temporary files will be created in this directory.
  // ----------------------------------------------------------------------------

  std::string tmpPath = FileMan::joinPaths(configFolderPath, LOCAL_CURRENT_DIR);
	if (mkdir(tmpPath.c_str(), 0700) != 0 && errno != EEXIST)
	{
	  SLOGE(DEBUG_TAG_FILEMAN, "Unable to create tmp directory '%s'", tmpPath.c_str());
	  throw std::runtime_error("Unable to create tmp directory");
	}
  else
  {
    SetFileManCurrentDirectory(tmpPath.c_str());
  }

  return configFolderPath;
}


/** Open file in the given folder in case-insensitive manner.
 * @return file descriptor or -1 if file is not found. */
int FileMan::openFileCaseInsensitive(const std::string &folderPath, const char *filename, int mode)
{
  std::string path = FileMan::joinPaths(folderPath, filename);
  int d = open(path.c_str(), mode);
  if (d < 0)
  {
#if CASE_SENSITIVE_FS
    // on case-sensitive file system need to try to find another name
    std::string newFileName;
    if(findObjectCaseInsensitive(folderPath.c_str(), filename, true, false, newFileName))
    {
      path = FileMan::joinPaths(folderPath, newFileName);
      d = open(path.c_str(), mode);
    }
#endif
  }
  return d;
}

void FileDelete(const std::string &path)
{
  FileDelete(path.c_str());
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


/** Get file open modes from reading. */
const char* GetFileOpenModeForReading(int *posixMode)
{
  return GetFileOpenModes(FILE_ACCESS_READ, posixMode);
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

void FileClose(SGPFile* f)
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

void FileRead(SGPFile* const f, void* const pDest, size_t const uiBytesToRead)
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
		ret = LoadDataFromLibrary(&f->u.lib, pDest, (UINT32)uiBytesToRead);
	}

#ifdef JA2TESTVERSION
	//Add the time that we spent in this function to the total.
	uiTotalFileReadTime += GetJA2Clock() - uiStartTime;
	uiTotalFileReadCalls++;
#endif

	if (!ret) throw std::runtime_error("Reading from file failed");
}


void FileWrite(SGPFile* const f, void const* const pDest, size_t const uiBytesToWrite)
{
	if (!(f->flags & SGPFILE_REAL)) throw std::logic_error("Tried to write to library file");
	if (fwrite(pDest, uiBytesToWrite, 1, f->u.file) != 1) throw std::runtime_error("Writing to file failed");
}


void FileSeek(SGPFile* const f, INT32 distance, FileSeekMode const how)
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


INT32 FileGetPos(const SGPFile* f)
{
	return f->flags & SGPFILE_REAL ? (INT32)ftell(f->u.file) : f->u.lib.uiFilePosInFile;
}


UINT32 FileGetSize(const SGPFile* f)
{
	if (f->flags & SGPFILE_REAL)
	{
		struct stat sb;
		if (fstat(fileno(f->u.file), &sb) != 0)
		{
			throw std::runtime_error("Getting file size failed");
		}
		return (UINT32)sb.st_size;
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


void FileMan::createDir(char const* const path)
{
	if (mkdir(path, 0755) == 0) return;

	if (errno == EEXIST)
	{
		FileAttributes const attr = FileGetAttributes(path);
		if (attr != FILE_ATTR_ERROR && attr & FILE_ATTR_DIRECTORY) return;
	}

	throw std::runtime_error("Failed to create directory");
}


void EraseDirectory(char const* const dirPath)
{
  std::vector<std::string> paths = FindAllFilesInDir(dirPath);
  for (std::vector<std::string>::const_iterator it(paths.begin()); it != paths.end(); ++it)
  {
		try
		{
			FileDelete(it->c_str());
		}
		catch (...)
		{
			const FileAttributes attr = FileGetAttributes(it->c_str());
			if (attr != FILE_ATTR_ERROR && attr & FILE_ATTR_DIRECTORY) continue;
			throw;
		}
	}
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


BOOLEAN FileClearAttributes(const std::string &filename)
{
  return FileClearAttributes(filename.c_str());
}

BOOLEAN FileClearAttributes(const char* const filename)
{
#if 1 // XXX TODO
  SLOGW(DEBUG_TAG_FILEMAN, "ignoring %s(\"%s\")", __func__, filename);
	return FALSE;
	// UNIMPLEMENTED
#else
	return SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);
#endif
}


BOOLEAN GetFileManFileTime(const SGPFile* f, SGP_FILETIME* const pCreationTime, SGP_FILETIME* const pLastAccessedTime, SGP_FILETIME* const pLastWriteTime)
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


FILE* GetRealFileHandleFromFileManFileHandle(const SGPFile* f)
{
	return f->flags & SGPFILE_REAL ? f->u.file : f->u.lib.lib->hLibraryHandle;
}

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

/** Join two path components. */
std::string FileMan::joinPaths(const std::string &first, const char *second)
{
  std::string result = first;
  if((result.length() == 0) || (result[result.length()-1] != PATH_SEPARATOR))
  {
    if(second[0] != PATH_SEPARATOR)
    {
      result += PATH_SEPARATOR;
    }
  }
  result += second;
  return result;
}

/** Join two path components. */
std::string FileMan::joinPaths(const std::string &first, const std::string &second)
{
  return joinPaths(first, second.c_str());
}

/** Join two path components. */
std::string FileMan::joinPaths(const char *first, const char *second)
{
  return joinPaths(std::string(first), second);
}

#if CASE_SENSITIVE_FS

/**
 * Find an object (file or subdirectory) in the given directory in case-independent manner.
 * @return true when found, return the found name using foundName. */
bool FileMan::findObjectCaseInsensitive(const char *directory, const char *name, bool lookForFiles, bool lookForSubdirs, std::string &foundName)
{
  bool result = false;

  // if name contains directories, than we have to find actual case-sensitive name of the directory
  // and only then look for a file
  const char *splitter = strstr(name, "/");
  int dirNameLen = (int)(splitter - name);
  if(splitter && (dirNameLen > 0) && splitter[1] != 0)
  {
    // we have directory in the name
    // let's find its correct name first
    char newDirectory[128];
    std::string actualSubdirName;
    strncpy(newDirectory, name, sizeof(newDirectory));
    newDirectory[dirNameLen] = 0;

    if(findObjectCaseInsensitive(directory, newDirectory, false, true, actualSubdirName))
    {
      // found subdirectory; let's continue the full search
      std::string pathInSubdir;
      std::string newDirectory = FileMan::joinPaths(directory, actualSubdirName.c_str());
      if(findObjectCaseInsensitive(newDirectory.c_str(), splitter + 1, lookForFiles, lookForSubdirs, pathInSubdir))
      {
        // found name in subdir
        foundName = FileMan::joinPaths(actualSubdirName, pathInSubdir);
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
          foundName = entry->d_name;
          result = true;
        }
      }
      closedir(d);
    }
  }

  // SLOGI(DEBUG_TAG_FILEMAN,"Looking for %s/[ %s ] : %s", directory, name, result ? "success" : "failure");
  return result;
}
#endif


/** Convert file descriptor to HWFile.
 * Raise runtime_error if not possible. */
SGPFile* FileMan::getSGPFileFromFD(int fd, const char *filename, const char *fmode)
{
	if (fd < 0)
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "Opening file '%s' failed", filename);
    throw std::runtime_error(buf);
  }

	FILE* const f = fdopen(fd, fmode);
	if (!f)
	{
    char buf[128];
    snprintf(buf, sizeof(buf), "Opening file '%s' failed", filename);
    throw std::runtime_error(buf);
	}

  SGPFile *file = MALLOCZ(SGPFile);
	file->flags  = SGPFILE_REAL;
	file->u.file = f;
	return file;
}


/** Open file for writing.
 * If file is missing it will be created.
 * If file exists, it's content will be removed. */
SGPFile* FileMan::openForWriting(const char *filename, bool truncate)
{
	int mode;
	const char* fmode = GetFileOpenModes(FILE_ACCESS_WRITE, &mode);

  if(truncate)
  {
    mode |= O_TRUNC;
  }

	int d = open3(filename, mode | O_CREAT, 0600);
  return getSGPFileFromFD(d, filename, fmode);
}


/** Open file for appending data.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForAppend(const char *filename)
{
	int         mode;
	const char* fmode = GetFileOpenModes(FILE_ACCESS_APPEND, &mode);

  int d = open3(filename, mode | O_CREAT, 0600);
  return getSGPFileFromFD(d, filename, fmode);
}


/** Open file for reading and writing.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForReadWrite(const char *filename)
{
	int         mode;
	const char* fmode = GetFileOpenModes(FILE_ACCESS_READWRITE, &mode);

  int d = open3(filename, mode | O_CREAT, 0600);
  return getSGPFileFromFD(d, filename, fmode);
}

/** Open file for reading. */
SGPFile* FileMan::openForReading(const char *filename)
{
	int         mode;
	const char* fmode = GetFileOpenModes(FILE_ACCESS_READ, &mode);
  int d = open3(filename, mode, 0600);
  return getSGPFileFromFD(d, filename, fmode);
}

/** Open file for reading. */
SGPFile* FileMan::openForReading(const std::string &filename)
{
  return openForReading(filename.c_str());
}

/** Open file for reading.  Look file in folderPath in case-insensitive manner. */
FILE* FileMan::openForReadingCaseInsensitive(const std::string &folderPath, const char *filename)
{
	int mode;
	const char* fmode = GetFileOpenModes(FILE_ACCESS_READ, &mode);

  int d = openFileCaseInsensitive(folderPath, filename, mode);
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

/**
 * Find all files with the given extension in the given directory.
 * @param dirPath Path to the directory
 * @param extension Extension with dot (e.g. ".txt")
 * @param caseIncensitive When True, do case-insensitive search even of case-sensitive file-systems. * * @return List of paths (dir + filename). */
std::vector<std::string>
FindFilesInDir(const std::string &dirPath,
               const std::string &ext,
               bool caseIncensitive,
               bool returnOnlyNames,
               bool sortResults)
{
  std::string ext_copy = ext;
  if(caseIncensitive)
  {
    std::transform(ext_copy.begin(), ext_copy.end(), ext_copy.begin(), ::tolower);
  }

  std::vector<std::string> paths;
  boost::filesystem::path path(dirPath);
  boost::filesystem::directory_iterator end;
  for(boost::filesystem::directory_iterator it(path); it != end; it++)
  {
    if(boost::filesystem::is_regular_file(it->status()))
    {
      std::string file_ext = it->path().extension().string();
      if(caseIncensitive)
      {
        std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);
      }
      if(file_ext.compare(ext_copy) == 0)
      {
        if(returnOnlyNames)
        {
          paths.push_back(it->path().filename().string());
        }
        else
        {
          paths.push_back(it->path().string());
        }
      }
    }
  }
  if(sortResults)
  {
    std::sort(paths.begin(), paths.end());
  }
  return paths;
}

/**
 * Find all files in a directory.
 * @return List of paths (dir + filename). */
std::vector<std::string>
FindAllFilesInDir(const std::string &dirPath, bool sortResults)
{
  std::vector<std::string> paths;
  boost::filesystem::path path(dirPath);
  boost::filesystem::directory_iterator end;
  for(boost::filesystem::directory_iterator it(path); it != end; it++)
  {
    if(boost::filesystem::is_regular_file(it->status()))
    {
      paths.push_back(it->path().string());
    }
  }
  if(sortResults)
  {
    std::sort(paths.begin(), paths.end());
  }
  return paths;
}

/** Replace extension of a file. */
std::string FileMan::replaceExtension(const std::string &_path, const char *newExtensionWithDot)
{
  boost::filesystem::path path(_path);
  boost::filesystem::path foo = boost::filesystem::path(newExtensionWithDot);
  return path.replace_extension(newExtensionWithDot).string();
}

/** Get parent path (e.g. directory path from the full path). */
std::string FileMan::getParentPath(const std::string &_path, bool absolute)
{
  boost::filesystem::path path(_path);
  boost::filesystem::path parent = path.parent_path();
  if(absolute)
  {
    parent = boost::filesystem::absolute(parent);
  }
  return parent.string();
}

/** Get filename from the path. */
std::string FileMan::getFileName(const std::string &_path)
{
  boost::filesystem::path path(_path);
  return path.filename().string();
}

/** Get filename from the path without extension. */
std::string FileMan::getFileNameWithoutExt(const char *path)
{
  return replaceExtension(getFileName(path), "");
}

std::string FileMan::getFileNameWithoutExt(const std::string &path)
{
  return getFileNameWithoutExt(path.c_str());
}

int FileMan::openFileForReading(const char *filename, int mode)
{
  return open(filename, mode);
}

/** Replace all \ with / */
void FileMan::slashifyPath(std::string &path)
{
  int len = path.size();
  for(int i = 0; i < len; i++)
  {
    if(path[i] == '\\')
    {
      path[i] = '/';
    }
  }
}

/** Read the whole file as text. */
std::string FileMan::fileReadText(SGPFile* file)
{
  uint32_t size = FileGetSize(file);
  char *data = new char[size+1];
  FileRead(file, data, size);
  data[size] = 0;
  std::string result(data);
  delete[] data;
  return result;
}

/** Check file existance. */
bool FileMan::checkFileExistance(const char *folder, const char *fileName)
{
  boost::filesystem::path path(folder);
  path /= fileName;
  return boost::filesystem::exists(path);
}

void FileMan::moveFile(const char *from, const char *to)
{
  boost::filesystem::path fromPath(from);
  boost::filesystem::path toPath(to);
  boost::filesystem::rename(fromPath, toPath);
}
