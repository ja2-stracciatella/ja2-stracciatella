#include "Debug.h"
#include "FileMan.h"
#include "Logger.h"
#include "MemMan.h"
#include "RustInterface.h"

#include <SDL_rwops.h>
#include <string_theory/string>

#include <stdexcept>

// XXX: remove FileMan class and make it into a namespace

#define LOCAL_CURRENT_DIR "tmp"
#define SDL_RWOPS_SGP 222


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


RustPointer<File> FileMan::openFileCaseInsensitive(const ST::string& folderPath, const ST::string& filename, uint8_t open_options)
{
	RustPointer<char> path{Fs_resolveExistingComponents(filename.c_str(), folderPath.c_str(), true)};
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
		VfsFile_close(f->u.vfile);
	}
	delete f;
}

void FileRead(SGPFile* const f, void* const pDest, size_t const uiBytesToRead)
{
	bool success;
	if (f->flags & SGPFILE_REAL)
	{
		success = File_readExact(f->u.file, reinterpret_cast<uint8_t*>(pDest), uiBytesToRead);
	}
	else
	{
		success = VfsFile_readExact(f->u.vfile, static_cast<uint8_t *>(pDest), uiBytesToRead);
	}

	if (!success)
	{
		RustPointer<char> err{getRustError()};
		SLOGE("FileRead: %s", err.get());
		throw std::runtime_error("Reading from file failed");
	}
}


void FileWrite(SGPFile* const f, void const* const pDest, size_t const uiBytesToWrite)
{
	bool success;
	if (f->flags & SGPFILE_REAL)
	{
		success = File_writeAll(f->u.file, reinterpret_cast<const uint8_t*>(pDest), uiBytesToWrite);
	}
	else
	{
		success = VfsFile_writeAll(f->u.vfile, reinterpret_cast<const uint8_t *>(pDest), uiBytesToWrite);
	}

	if (!success)
	{
		RustPointer<char> err{getRustError()};
		SLOGE("FileWrite: %s", err.get());
		throw std::runtime_error("Writing to file failed");
	}
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
		switch (how)
		{
			case FILE_SEEK_FROM_START: success = distance >= 0 && VfsFile_seekFromStart(f->u.vfile, static_cast<uint64_t>(distance), nullptr); break;
			case FILE_SEEK_FROM_END:   success = VfsFile_seekFromEnd(f->u.vfile, distance, nullptr); break;
			default:                   success = VfsFile_seekFromCurrent(f->u.vfile, distance, nullptr); break;
		}
	}
	if (!success) throw std::runtime_error("Seek in file failed");
}


INT32 FileGetPos(const SGPFile* f)
{
	bool success;
	uint64_t position = 0;
	if (f->flags & SGPFILE_REAL)
	{
		position = File_seekFromCurrent(f->u.file, 0);
		success = position != UINT64_MAX;
	}
	else
	{
		success = VfsFile_seekFromCurrent(f->u.vfile, 0, &position);
	}

	if (!success)
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileGetPos: {}", err.get()));
		throw std::runtime_error("Getting file position failed");
	}
	if (position > INT32_MAX)
	{
		SLOGW(ST::format("FileGetPos: truncating from {} to {}", position, INT32_MAX));
		position = INT32_MAX;
	}
	return static_cast<INT32>(position);
}


UINT32 FileGetSize(const SGPFile* f)
{
	bool success;
	uint64_t len = 0;
	if (f->flags & SGPFILE_REAL)
	{
		len = File_len(f->u.file);
		success = len != UINT64_MAX;
	}
	else
	{
		success = VfsFile_len(f->u.vfile, &len);
	}

	if (!success)
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileGetSize: {}", err.get()));
		throw std::runtime_error("Getting file size failed");
	}
	if (len > UINT32_MAX)
	{
		SLOGW(ST::format("FileGetSize: truncating from {} to {}", len, UINT32_MAX));
		len = UINT32_MAX;
	}
	return static_cast<UINT32>(len);
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


void EraseDirectory(const ST::string& dirPath)
{
	std::vector<ST::string> paths = FindAllFilesInDir(dirPath);
	for (const ST::string& path : paths)
	{
		try
		{
			FileDelete(path);
		}
		catch (const std::runtime_error& ex)
		{
			if (Fs_isDir(path.c_str())) continue;
			SLOGE(ST::format("EraseDirectory '{}' '{}': {}", dirPath, path, ex.what()));
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


ST::string FileMan::joinPaths(const ST::string& first, const ST::string& second)
{
	RustPointer<char> path{Path_push(first.c_str(), second.c_str())};
	return path.get();
}

ST::string FileMan::joinPaths(const std::vector<ST::string> parts)
{
	if (parts.size() < 1) return ST::null;

	ST::string path = parts[0];
	for (size_t i = 1; i < parts.size(); i++)
	{
		path = joinPaths(path, parts[i]);
	}
	return path;
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
SGPFile* FileMan::openForWriting(const ST::string& filename, bool truncate)
{
	uint8_t open_options = FILE_OPEN_WRITE | FILE_OPEN_CREATE;
	if (truncate)
	{
		open_options |= FILE_OPEN_TRUNCATE;
	}

	RustPointer<File> file{File_open(filename.c_str(), open_options)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileMan::openForWriting '{}' {}: {}", filename, truncate, err.get()));
		throw std::runtime_error("FileMan::openForWriting failed");
	}
	return getSGPFileFromFile(file.release());
}


/** Open file for appending data.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForAppend(const ST::string& filename)
{
	RustPointer<File> file{File_open(filename.c_str(), FILE_OPEN_APPEND | FILE_OPEN_CREATE)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileMan::openForAppend '{}': {}", filename, err.get()));
		throw std::runtime_error("FileMan::openForAppend failed");
	}
	return getSGPFileFromFile(file.release());
}


/** Open file for reading and writing.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForReadWrite(const ST::string& filename)
{
	RustPointer<File> file{File_open(filename.c_str(), FILE_OPEN_READ | FILE_OPEN_WRITE | FILE_OPEN_CREATE)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileMan::openForReadWrite '{}': {}", filename, err.get()));
		throw std::runtime_error("FileMan::openForReadWrite failed");
	}
	return getSGPFileFromFile(file.release());
}

/** Open file for reading. */
SGPFile* FileMan::openForReading(const ST::string &filename)
{
	RustPointer<File> file{File_open(filename.c_str(), FILE_OPEN_READ)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileMan::openForReading '{}': {}", filename, err.get()));
		throw std::runtime_error("FileMan::openForReading failed");
	}
	return getSGPFileFromFile(file.release());
}

/** Open file for reading.  Look file in folderPath in case-insensitive manner. */
RustPointer<File> FileMan::openForReadingCaseInsensitive(const ST::string& folderPath, const ST::string& filename)
{
	return openFileCaseInsensitive(folderPath, filename, FILE_OPEN_READ);
}

std::vector<ST::string>
FindFilesInDir(const ST::string& dirPath,
		const ST::string& ext,
		bool caseInsensitive,
		bool returnOnlyNames,
		bool sortResults,
		bool recursive)
{
	std::vector<ST::string> results;
	std::vector<ST::string> paths = FindAllFilesInDir(dirPath, sortResults, recursive);
	for (ST::string& path : paths)
	{
		// the extension must match
		RustPointer<char> path_ext(Path_extension(path.c_str()));
		if (path_ext)
		{
			int cmp = caseInsensitive ? ext.compare_i(path_ext.get()) : ext.compare(path_ext.get());
			if (cmp != 0)
			{
				continue;
			}
		}
		else if (!ext.empty())
		{
			continue;
		}
		// keep filename or path
		if (returnOnlyNames && !recursive)
		{
			RustPointer<char> filename{Path_filename(path.c_str())};
			Assert(filename);
			results.emplace_back(filename.get());
		}
		else
		{
			results.emplace_back(std::move(path));
		}
	}
	if(sortResults)
	{
		std::sort(results.begin(), results.end());
	}
	return results;
}

std::vector<ST::string>
FindAllFilesInDir(const ST::string& dirPath, bool sortResults, bool recursive)
{
	std::vector<ST::string> paths;
	RustPointer<VecCString> vec{Fs_readDirPaths(dirPath.c_str(), false)};
	if (!vec)
	{
		RustPointer<char> err{getRustError()};
		SLOGW(ST::format("FindAllFilesInDir: {}", err.get()));
		return paths;
	}
	size_t len = VecCString_len(vec.get());
	for (size_t i = 0; i < len; i++)
	{
		RustPointer<char> path{VecCString_get(vec.get(), i)};
		if (Fs_isFile(path.get()))
		{
			paths.emplace_back(path.get());
		}
		else if (recursive && Fs_isDir(path.get()))
		{
			std::vector<ST::string> subDirFiles = FindAllFilesInDir(path.get(), false, true);
			paths.insert(paths.end(), subDirFiles.begin(), subDirFiles.end());
		}
	}
	if(sortResults)
	{
		std::sort(paths.begin(), paths.end());
	}
	return paths;
}

ST::string FileMan::replaceExtension(const ST::string& path, const ST::string& newExtension)
{
	RustPointer<char> newPath{Path_setExtension(path.c_str(), newExtension.c_str())};
	return newPath.get();
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

ST::string FileMan::getFileNameWithoutExt(const ST::string& path)
{
	RustPointer<char> filestem{Path_filestem(path.c_str())};
	return filestem ? ST::string(filestem.get()) : ST::null;
}

RustPointer<File> FileMan::openFileForReading(const ST::string& path)
{
	return RustPointer<File>{File_open(path.c_str(), FILE_OPEN_READ)};
}

/** Replace all \ with / */
void FileMan::slashifyPath(ST::string &path)
{
	path = path.replace("\\", "/");
}

ST::string FileMan::fileReadText(SGPFile* file)
{
	uint32_t size = FileGetSize(file);
	ST::char_buffer buf{size, '\0'};
	FileRead(file, buf.data(), size);
	return ST::string{buf};
}

/** Check file existance. */
bool FileMan::checkFileExistance(const ST::string& folder, const ST::string& fileName)
{
	return checkPathExistance(joinPaths(folder, fileName));
}

/**  Check path existence. */
bool FileMan::checkPathExistance(const ST::string& path)
{
	return Fs_exists(path.c_str());
}

void FileMan::moveFile(const ST::string& from, const ST::string& to)
{
	if (!Fs_rename(from.c_str(), to.c_str()))
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("FileMan::moveFile '{}' '{}': {}", from, to, err.get()));
		throw std::runtime_error("FileMan::moveFile failed");
	}
}
