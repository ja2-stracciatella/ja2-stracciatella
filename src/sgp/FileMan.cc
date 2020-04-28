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
void FileMan::switchTmpFolder(const ST::string& home)
{
	// Create another directory and set is as the current directory for the process
	// Temporary files will be created in this directory.
	// ----------------------------------------------------------------------------

	RustPointer<char> tmpPath{Path_push(home.c_str(), LOCAL_CURRENT_DIR)};
	if (!Fs_isDir(tmpPath.get()) && !Fs_createDir(tmpPath.get()))
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("Unable to create tmp directory '{}': {}", tmpPath.get(), err.get()));
		throw std::runtime_error("Unable to create tmp directory");
	}
	if (!Env_setCurrentDir(tmpPath.get()))
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("Unable to switch to tmp directory '{}': {}", tmpPath.get(), err.get()));
		throw std::runtime_error("Unable to switch to tmp directory");
	}
}


RustPointer<File> FileMan::openFileCaseInsensitive(const ST::string& folderPath, const char* filename, uint8_t open_options)
{
	RustPointer<char> path{Fs_resolveExistingComponents(filename, folderPath.c_str(), true)};
	return RustPointer<File>{File_open(path.get(), open_options)};
}

void FileDelete(const ST::string& path)
{
	if (Fs_exists(path.c_str()) && !Fs_removeFile(path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("Deleting file '{}' failed: {}", path, err.get()));
		throw std::runtime_error("Deleting file failed");
	}
}


void FileClose(SGPFile* f)
{
	if (f->flags & SGPFILE_REAL)
	{
		File_close(f->u.file);
	}
	else
	{
		LibraryFile_close(f->u.lib);
	}
	delete f;
}

void FileRead(SGPFile* const f, void* const pDest, size_t const uiBytesToRead)
{
	BOOLEAN ret;
	if (f->flags & SGPFILE_REAL)
	{
		ret = File_readExact(f->u.file, reinterpret_cast<uint8_t*>(pDest), uiBytesToRead);
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
	if (!File_writeAll(f->u.file, reinterpret_cast<const uint8_t*>(pDest), uiBytesToWrite)) throw std::runtime_error("Writing to file failed");
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
		switch (how)
		{
			case FILE_SEEK_FROM_START: success = distance >= 0 && File_seekFromStart(f->u.file, static_cast<uint64_t>(distance)) != UINT64_MAX; break;
			case FILE_SEEK_FROM_END:   success = File_seekFromEnd(f->u.file, distance) != UINT64_MAX; break;
			default:                   success = File_seekFromCurrent(f->u.file, distance) != UINT64_MAX; break;
		}
	}
	else
	{
		success = LibraryFile_seek(f->u.lib, distance, how);
	}
	if (!success) throw std::runtime_error("Seek in file failed");
}


INT32 FileGetPos(const SGPFile* f)
{
	return f->flags & SGPFILE_REAL ? (INT32)File_seekFromCurrent(f->u.file, 0) : (INT32)LibraryFile_getPosition(f->u.lib);
}


UINT32 FileGetSize(const SGPFile* f)
{
	if (f->flags & SGPFILE_REAL)
	{
		uint64_t len = File_len(f->u.file);
		if (len == UINT64_MAX)
		{
			throw std::runtime_error("Getting file size failed");
		}
		return (UINT32)len;
	}
	else
	{
		return (UINT32)LibraryFile_getSize(f->u.lib);
	}
}


void FileMan::createDir(const ST::string& path)
{
	if (!Fs_isDir(path.c_str()) && !Fs_createDir(path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("Failed to created directory '{}': {}", path, err.get()));
		throw std::runtime_error("Failed to create directory");
	}
}


void EraseDirectory(char const* const dirPath)
{
	std::vector<ST::string> paths = FindAllFilesInDir(dirPath);
	for (std::vector<ST::string>::const_iterator it(paths.begin()); it != paths.end(); ++it)
	{
		try
		{
			FileDelete(it->c_str());
		}
		catch (...)
		{
			if (Fs_isDir(it->c_str())) continue;
			throw;
		}
	}
}


File* GetRealFileHandleFromFileManFileHandle(const SGPFile* f)
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
ST::string FileMan::joinPaths(const ST::string &first, const char *second)
{
	ST::string result = first;
	if((result.size() == 0) || (result[result.size()-1] != PATH_SEPARATOR))
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
ST::string FileMan::joinPaths(const ST::string &first, const ST::string &second)
{
	return joinPaths(first, second.c_str());
}

/** Join two path components. */
ST::string FileMan::joinPaths(const char *first, const char *second)
{
	return joinPaths(ST::string(first), second);
}


SGPFile* FileMan::getSGPFileFromFile(File* f)
{
	Assert(f);
	SGPFile *sgp_file = new SGPFile{};
	sgp_file->flags  = SGPFILE_REAL;
	sgp_file->u.file = f;
	return sgp_file;
}


/** Open file for writing.
 * If file is missing it will be created.
 * If file exists, it's content will be removed. */
SGPFile* FileMan::openForWriting(const char *filename, bool truncate)
{
	uint8_t open_options = FILE_OPEN_WRITE | FILE_OPEN_CREATE;
	if (truncate)
	{
		open_options |= FILE_OPEN_TRUNCATE;
	}

	RustPointer<File> file(File_open(filename, open_options));
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "FileMan::openForWriting: %s", err.get());
		throw std::runtime_error(buf);
	}
	return getSGPFileFromFile(file.release());
}


/** Open file for appending data.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForAppend(const char *filename)
{
	RustPointer<File> file(File_open(filename, FILE_OPEN_APPEND | FILE_OPEN_CREATE));
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "FileMan::openForAppend: %s", err.get());
		throw std::runtime_error(buf);
	}
	return getSGPFileFromFile(file.release());
}


/** Open file for reading and writing.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForReadWrite(const char *filename)
{
	RustPointer<File> file(File_open(filename, FILE_OPEN_READ | FILE_OPEN_WRITE | FILE_OPEN_CREATE));
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "FileMan::openForReadWrite: %s", err.get());
		throw std::runtime_error(buf);
	}
	return getSGPFileFromFile(file.release());
}

/** Open file for reading. */
SGPFile* FileMan::openForReading(const char *filename)
{
	RustPointer<File> file(File_open(filename, FILE_OPEN_READ));
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "FileMan::openForReading: %s", err.get());
		throw std::runtime_error(buf);
	}
	return getSGPFileFromFile(file.release());
}

/** Open file for reading. */
SGPFile* FileMan::openForReading(const ST::string &filename)
{
	return openForReading(filename.c_str());
}

/** Open file for reading.  Look file in folderPath in case-insensitive manner. */
RustPointer<File> FileMan::openForReadingCaseInsensitive(const ST::string& folderPath, const char* filename)
{
	return openFileCaseInsensitive(folderPath, filename, FILE_OPEN_READ);
}

std::vector<ST::string>
FindFilesInDir(const ST::string &dirPath,
		const ST::string &ext,
		bool caseIncensitive,
		bool returnOnlyNames,
		bool sortResults)
{
	std::vector<ST::string> ret;
	std::vector<ST::string> paths = FindAllFilesInDir(dirPath, sortResults);
	for (ST::string& path : paths)
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

std::vector<ST::string>
FindAllFilesInDir(const ST::string &dirPath, bool sortResults)
{
	std::vector<ST::string> paths;
	RustPointer<VecCString> vec(Fs_readDirPaths(dirPath.c_str(), false));
	if (!vec)
	{
		RustPointer<char> msg(getRustError());
		SLOGW("%s", msg.get());
		return paths;
	}
	size_t len = VecCString_len(vec.get());
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

ST::string FileMan::replaceExtension(const ST::string &path, const char *newExtensionWithDot)
{
	// TODO switch to rust path extensions (treats the dot in a different way)
	ST::string filename = getFileName(path);
	size_t n = filename.size();

	if (filename != "." && filename != "..")
	{
		auto dot = filename.find_last('.');
		if (dot != -1)
		{
			filename = filename.substr(0, dot);
		}
	}
	if (newExtensionWithDot[0] != '\0' && newExtensionWithDot[0] != '.')
	{
		filename += '.';
	}
	filename += newExtensionWithDot;

	ST::string newPath = path.substr(0, path.size() - n);
	newPath += filename;
	return newPath;
}

ST::string FileMan::getParentPath(const ST::string &path, bool absolute)
{
	RustPointer<char> parent(Path_parent(path.c_str()));
	if (!parent)
	{
		return ST::string();
	}
	if (absolute && !Path_isAbsolute(path.c_str()))
	{
		RustPointer<char> dir(Env_currentDir());
		if (!dir)
		{
			RustPointer<char> msg(getRustError());
			SLOGW("%s", msg.get());
			throw new std::runtime_error("expected the current directory");
		}
		return joinPaths(dir.get(), parent.get());
	}
	return parent.get();
}

/** Get filename from the path. */
ST::string FileMan::getFileName(const ST::string &path)
{
	RustPointer<char> filename(Path_filename(path.c_str()));
	if (!filename)
	{
		return ST::string();
	}
	return ST::string(filename.get());
}

/** Get filename from the path without extension. */
ST::string FileMan::getFileNameWithoutExt(const char *path)
{
	return replaceExtension(getFileName(path), "");
}

ST::string FileMan::getFileNameWithoutExt(const ST::string &path)
{
	return getFileNameWithoutExt(path.c_str());
}

RustPointer<File> FileMan::openFileForReading(const char* filename)
{
	return RustPointer<File>(File_open(filename, FILE_OPEN_READ));
}

/** Replace all \ with / */
void FileMan::slashifyPath(ST::string &path)
{
	path = path.replace("\\", "/");
}

/** Read the whole file as text. */
ST::string FileMan::fileReadText(SGPFile* file)
{
	uint32_t size = FileGetSize(file);
	char *data = new char[size+1];
	FileRead(file, data, size);
	data[size] = 0;
	ST::string result(data);
	delete[] data;
	return result;
}

/** Check file existance. */
bool FileMan::checkFileExistance(const char *folder, const char *fileName)
{
	ST::string path = joinPaths(folder, fileName);
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
