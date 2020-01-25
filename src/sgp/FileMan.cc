#include <stdexcept>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "FileMan.h"
#include "RustInterface.h"
#include "MemMan.h"
#include "PODObj.h"

#include "boost/filesystem.hpp"

#include "Logger.h"

#if _WIN32
#include <shlobj.h>
#else
#include <pwd.h>
#endif

#include "PlatformIO.h"
#include "Debug.h"

#if MACOS_USE_RESOURCES_FROM_BUNDLE && defined __APPLE__  && defined __MACH__
#include <CoreFoundation/CFBundle.h>
#endif

#if CASE_SENSITIVE_FS
#include <dirent.h>
#include <SDL_rwops.h>

#endif

// XXX: remove FileMan class and make it into a namespace

#define LOCAL_CURRENT_DIR "tmp"
#define SDL_RWOPS_SGP 222

enum FileOpenFlags
{
	FILE_ACCESS_READ      = 1U << 0,
	FILE_ACCESS_WRITE     = 1U << 1,
	FILE_ACCESS_READWRITE = FILE_ACCESS_READ | FILE_ACCESS_WRITE,
	FILE_ACCESS_APPEND    = 1U << 2
};


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
std::string FileMan::switchTmpFolder(std::string home)
{
	// Create another directory and set is as the current directory for the process
	// Temporary files will be created in this directory.
	// ----------------------------------------------------------------------------

	std::string tmpPath = FileMan::joinPaths(home, LOCAL_CURRENT_DIR);
	if (mkdir(tmpPath.c_str(), 0700) != 0 && errno != EEXIST)
	{
		SLOGE("Unable to create tmp directory '%s'", tmpPath.c_str());
		throw std::runtime_error("Unable to create tmp directory");
	}
	else
	{
		SetFileManCurrentDirectory(tmpPath.c_str());
	}

	return home;
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
		LibraryFile_close(f->u.lib);
	}
	MemFree(f);
}

void FileRead(SGPFile* const f, void* const pDest, size_t const uiBytesToRead)
{
	BOOLEAN ret;
	if (f->flags & SGPFILE_REAL)
	{
		ret = fread(pDest, uiBytesToRead, 1, f->u.file) == 1;
	}
	else
	{
		ret = LibraryFile_read(f->u.lib, static_cast<uint8_t *>(pDest), uiBytesToRead);
	}

	if (!ret) throw std::runtime_error("Reading from file failed");
}


void FileWrite(SGPFile* const f, void const* const pDest, size_t const uiBytesToWrite)
{
	if (!(f->flags & SGPFILE_REAL)) throw std::logic_error("Tried to write to library file");
	if (fwrite(pDest, uiBytesToWrite, 1, f->u.file) != 1) throw std::runtime_error("Writing to file failed");
}

static int64_t SGPSeekRW(SDL_RWops *context, int64_t offset, int whence)
{
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);
	FileSeekMode mode = FILE_SEEK_FROM_CURRENT;
	switch (whence) {
		case RW_SEEK_SET:
			mode = FILE_SEEK_FROM_START;
			break;
		case RW_SEEK_END:
			mode = FILE_SEEK_FROM_END;
			break;
		default:
			break;
	}

	FileSeek(sgpFile, offset, mode);

	return int64_t(FileGetPos(sgpFile));
}

static int64_t SGPSizeRW(SDL_RWops *context)
{
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);

	return FileGetSize(sgpFile);
}

static size_t SGPReadRW(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);
	UINT32 posBefore = UINT32(FileGetPos(sgpFile));

	FileRead(sgpFile, ptr, size * maxnum);

	UINT32 posAfter = UINT32(FileGetPos(sgpFile));

	return (posAfter - posBefore) / size;
}

static size_t SGPWriteRW(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
	AssertMsg(false, "SGPWriteRW not supported");
	return 0;
}

static int SGPCloseRW(SDL_RWops *context)
{
	if(context->type != SDL_RWOPS_SGP)
	{
		return SDL_SetError("Wrong kind of SDL_RWops for SGPCloseRW()");
	}
	SGPFile* sgpFile = (SGPFile*)(context->hidden.unknown.data1);

	FileClose(sgpFile);
	SDL_FreeRW(context);

	return 0;
}

SDL_RWops* FileGetRWOps(SGPFile* const f) {
	SDL_RWops* rwOps = SDL_AllocRW();
	if(rwOps == NULL) {
		return NULL;
	}
	rwOps->type = SDL_RWOPS_SGP;
	rwOps->size = SGPSizeRW;
	rwOps->seek = SGPSeekRW;
	rwOps->read = SGPReadRW;
	rwOps->write= SGPWriteRW;
	rwOps->close= SGPCloseRW;
	rwOps->hidden.unknown.data1 = f;

	return rwOps;
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
		success = LibraryFile_seek(f->u.lib, distance, how);
	}
	if (!success) throw std::runtime_error("Seek in file failed");
}


INT32 FileGetPos(const SGPFile* f)
{
	return f->flags & SGPFILE_REAL ? (INT32)ftell(f->u.file) : (INT32)LibraryFile_getPosition(f->u.lib);
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
		return (UINT32)LibraryFile_getSize(f->u.lib);
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


FILE* GetRealFileHandleFromFileManFileHandle(const SGPFile* f)
{
	return f->flags & SGPFILE_REAL ? f->u.file : nullptr;
}

uint64_t GetFreeSpaceOnHardDriveWhereGameIsRunningFrom(void)
{
	RustPointer<char> path(Env_currentDir());
	if (!path)
	{
		RustPointer<char> msg(getRustError());
		SLOGW("%s", msg.get());
		return 0;
	}
	uint64_t bytes;
	if (!Fs_freeSpace(path.get(), &bytes))
	{
		RustPointer<char> msg(getRustError());
		SLOGW("%s", msg.get());
		return 0;
	}
	return bytes;
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
			if(findObjectCaseInsensitive(newDirectory.c_str(), splitter + 1,
							lookForFiles, lookForSubdirs, pathInSubdir))
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

	// SLOGI("Looking for %s/[ %s ] : %s", directory, name, result ? "success" : "failure");
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

std::vector<std::string>
FindFilesInDir(const std::string &dirPath,
		const std::string &ext,
		bool caseIncensitive,
		bool returnOnlyNames,
		bool sortResults)
{
	std::vector<std::string> ret;
	std::vector<std::string> paths = FindAllFilesInDir(dirPath, sortResults);
	for (std::string& path : paths)
	{
		RustPointer<char> path_ext(Path_extension(path.c_str()));
		bool same_ext;
		if (!path_ext)
		{
			same_ext = ext.empty();
		}
		else if (caseIncensitive)
		{
			same_ext = std::equal(ext.begin(), ext.end(), path_ext.get(), [](unsigned char a, unsigned char b) {
				return ::tolower(a) == ::tolower(b);
			});
		}
		else
		{
			same_ext = (ext == path_ext.get());
		}
		if (!same_ext)
		{
			continue;
		}
		if (returnOnlyNames)
		{
			RustPointer<char> filename(Path_filename(path.c_str()));
			if (!filename)
			{
				throw new std::logic_error("expected a filename");
			}
			ret.emplace_back(filename.get());
		}
		else
		{
			ret.emplace_back(std::move(path));
		}
	}
	return ret;
}

std::vector<std::string>
FindAllFilesInDir(const std::string &dirPath, bool sortResults)
{
	std::vector<std::string> paths;
	RustPointer<VecCString> vec(Fs_readDirPaths(dirPath.c_str(), false));
	if (!vec)
	{
		RustPointer<char> msg(getRustError());
		SLOGW("%s", msg.get());
		return paths;
	}
	size_t len = VecCString_length(vec.get());
	for (size_t i = 0; i < len; i++)
	{
		RustPointer<char> path(VecCString_get(vec.get(), i));
		if (Fs_isFile(path.get()))
		{
			paths.emplace_back(path.get());
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
std::string FileMan::getFileName(const std::string &path)
{
	RustPointer<char> filename(Path_filename(path.c_str()));
	if (!filename)
	{
		return std::string();
	}
	return std::string(filename.get());
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
	size_t len = path.size();
	for(size_t i = 0; i < len; i++)
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
	std::string path = joinPaths(folder, fileName);
	return Fs_exists(path.c_str());
}

void FileMan::moveFile(const char *from, const char *to)
{
	if (!Fs_rename(from, to))
	{
		RustPointer<char> msg(getRustError());
		throw new std::runtime_error(msg.get());
	}
}
