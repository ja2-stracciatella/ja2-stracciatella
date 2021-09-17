#include "Debug.h"
#include "FileMan.h"
#include "Logger.h"
#include "RustInterface.h"
#include "Exceptions.h"

#include <string_theory/string>

#include <stdexcept>

void FileMan::deleteFile(const ST::string& path)
{
	if (Fs_exists(path.c_str()) && !Fs_removeFile(path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::deleteFile('{}') failed: {}", path, err.get()));
	}
}

void FileMan::createDir(const ST::string& path)
{
	if (!Fs_isDir(path.c_str()) && !Fs_createDir(path.c_str()))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::createDir('{}') failed: {}", path, err.get()));
	}
}


void FileMan::eraseDirectory(const ST::string& dirPath)
{
	std::vector<ST::string> paths = FileMan::findAllFilesInDir(dirPath);
	for (const ST::string& path : paths)
	{
		if (FileMan::isDir(path)) continue;
		try
		{
			FileMan::deleteFile(path);
		}
		catch (const IoException& ex)
		{
			throw IoException(ST::format("FileMan::eraseDirectory('{}') failed because {}", dirPath, ex.what()));;
		}
	}
}

uint64_t FileMan::getFreeSpaceOnHardDriveWhereGameIsRunningFrom(void)
{
	RustPointer<char> path(Env_currentDir());
	if (!path)
	{
		RustPointer<char> msg(getRustError());
		SLOGW("FileMan::getFreeSpaceOnHardDriveWhereGameIsRunningFrom() failed: %s", msg.get());
		return 0;
	}
	uint64_t bytes;
	if (!Fs_freeSpace(path.get(), &bytes))
	{
		RustPointer<char> msg(getRustError());
		SLOGW("FileMan::getFreeSpaceOnHardDriveWhereGameIsRunningFrom() failed: %s", msg.get());
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

ST::string FileMan::resolveExistingComponents(const ST::string& path)
{
	RustPointer<char> resolved{Fs_resolveExistingComponents(path.c_str(), NULL, true)};
	if (resolved.get() == NULL) {
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::resolveExistingComponents('{}') failed: {}", path, err.get())); 	
	}
	return resolved.get();
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
		throw IoException(ST::format("FileMan::openForWriting('{}') failed: {}", filename, err.get()));
	}
	return new SGPFile(file.release());
}


/** Open file for appending data.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForAppend(const ST::string& filename)
{
	RustPointer<File> file{File_open(filename.c_str(), FILE_OPEN_APPEND | FILE_OPEN_CREATE)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::openForAppend('{}') failed: {}", filename, err.get()));
	}
	return new SGPFile(file.release());
}


/** Open file for reading and writing.
 * If file doesn't exist, it will be created. */
SGPFile* FileMan::openForReadWrite(const ST::string& filename)
{
	RustPointer<File> file{File_open(filename.c_str(), FILE_OPEN_READ | FILE_OPEN_WRITE | FILE_OPEN_CREATE)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::openForReadWrite('{}') failed: {}", filename, err.get()));
	}
	return new SGPFile(file.release());
}

/** Open file for reading. */
SGPFile* FileMan::openForReading(const ST::string &filename)
{
	RustPointer<File> file{File_open(filename.c_str(), FILE_OPEN_READ)};
	if (!file)
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::openForReading('{}') failed: {}", filename, err.get()));
	}
	return new SGPFile(file.release());
}

std::vector<ST::string>
FileMan::findFilesInDir(const ST::string& dirPath,
		const ST::string& ext,
		bool caseInsensitive,
		bool returnOnlyNames,
		bool sortResults,
		bool recursive)
{
	std::vector<ST::string> results;
	std::vector<ST::string> paths = FileMan::findAllFilesInDir(dirPath, sortResults, recursive);
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
FileMan::findAllFilesInDir(const ST::string& dirPath, bool sortResults, bool recursive, bool returnOnlyNames)
{
	std::vector<ST::string> paths;
	RustPointer<VecCString> vec{Fs_findAllFilesInDir(dirPath.c_str(), sortResults, recursive)};
	if (!vec)
	{
		RustPointer<char> err{getRustError()};
		STLOGW("FileMan::findAllFilesInDir({}) failed: {}", dirPath, err.get());
		return paths;
	}
	size_t len = VecCString_len(vec.get());
	for (size_t i = 0; i < len; i++)
	{
		RustPointer<char> path{VecCString_get(vec.get(), i)};
		if (returnOnlyNames) {
			RustPointer<char> filename{Path_filename(path.get())};
			paths.emplace_back(filename.get());
		} else {
			paths.emplace_back(path.get());
		}
	}
	return paths;
}

std::vector<ST::string>
FileMan::findAllDirsInDir(const ST::string& dirPath, bool sortResults, bool recursive, bool returnOnlyNames)
{
	std::vector<ST::string> paths;
	RustPointer<VecCString> vec{Fs_findAllDirsInDir(dirPath.c_str(), sortResults, recursive)};
	if (!vec)
	{
		RustPointer<char> err{getRustError()};
		STLOGW("FileMan::findAllDirsInDir({}) failed: {}", dirPath, err.get());
		return paths;
	}
	size_t len = VecCString_len(vec.get());
	for (size_t i = 0; i < len; i++)
	{
		RustPointer<char> path{VecCString_get(vec.get(), i)};
		if (returnOnlyNames) {
			RustPointer<char> filename{Path_filename(path.get())};
			paths.emplace_back(filename.get());
		} else {
			paths.emplace_back(path.get());
		}
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
			RustPointer<char> err(getRustError());
			throw IoException(ST::format("FileMan::getParentPath('{}') failed: {}", path, err.get()));
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

bool FileMan::isFile(const ST::string& path) {
	return Fs_isFile(path.c_str());
}

bool FileMan::isDir(const ST::string& path) {
	return Fs_isDir(path.c_str());
}

bool FileMan::isAbsolute(const ST::string& path) {
	return Path_isAbsolute(path.c_str());
}

bool FileMan::isReadOnly(const ST::string& path) {
	bool readonly = false;
	bool success = Fs_getReadOnly(path.c_str(), &readonly);
	if (!success) {
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::isReadOnly('{}') failed: {}", path, err.get()));
	}
	return readonly;
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
		throw IoException(ST::format("FileMan::moveFile('{}', '{}') failed: {}", from, to, err.get()));
	}
}

double FileMan::getLastModifiedTime(const ST::string& path)
{
	double lastModified = 0;
	if (!Fs_modifiedSecs( path.c_str(), &lastModified ))
	{
		RustPointer<char> err{getRustError()};
		throw IoException(ST::format("FileMan::getLastModifiedTime('{}') failed: {}", path, err.get()));
	}
	return lastModified;
}
