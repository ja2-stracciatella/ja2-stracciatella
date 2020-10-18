#ifndef FILEMAN_H
#define FILEMAN_H

#include "sgp/SGPFile.h"
#include "sgp/Types.h"

#include <string_theory/string>

#include <vector>

/* Delete the file at path. */
void FileDelete(const ST::string &path);

void FileRead( SGPFile*, void*       pDest, size_t uiBytesToRead);
size_t FileReadAtMost( SGPFile*, void*       pDest, size_t uiBytesToRead);
void FileWrite(SGPFile*, void const* pDest, size_t uiBytesToWrite);

/* Helper method to read a string from file */
ST::string FileReadString(SGPFile* const f, size_t const uiBytesToRead);

SDL_RWops* FileGetRWOps(SGPFile* const f);

template<typename T, typename U> static inline void FileWriteArray(SGPFile* const f, T const& n, U const* const data)
{
	FileWrite(f, &n, sizeof(n));
	if (n != 0) FileWrite(f, data, sizeof(*data) * n);
}

void  FileSeek(SGPFile*, INT32 distance, FileSeekMode);
INT32 FileGetPos(const SGPFile*);

UINT32 FileGetSize(const SGPFile*);

/* Removes ALL FILES in the specified directory, but leaves the directory alone.
 * Does not affect any subdirectories! */
void EraseDirectory(const ST::string& dirPath);

/* Pass in the Fileman file handle of an OPEN file and it will return..
 * - if its a Real File, the return will be the handle of the REAL file
 * - if its a LIBRARY file, the return will be null */
File* GetRealFileHandleFromFileManFileHandle(const SGPFile* hFile);

//Gets the amount of free space on the hard drive that the main executeablt is runnning from
uint64_t GetFreeSpaceOnHardDriveWhereGameIsRunningFrom(void);

/***
 * New file manager.
 *
 * This class provides clean interface for file operations. */
class FileMan
{
public:

	/** Find config folder and switch into it. */
	static void switchTmpFolder(const ST::string& homeDir);

	/** Open file for writing.
	 * If file is missing it will be created.
	 * If file exists, it's content will be removed. */
	static SGPFile* openForWriting(const ST::string& filename, bool truncate=true);

	/** Open file for appending data.
	 * If file doesn't exist, it will be created. */
	static SGPFile* openForAppend(const ST::string& filename);

	/** Open file for reading and writing.
	 * If file doesn't exist, it will be created. */
	static SGPFile* openForReadWrite(const ST::string& filename);

	/** Open file for reading. */
	static SGPFile* openForReading(const ST::string &filename);

	/** Read the whole file as text. */
	static ST::string fileReadText(SGPFile*);

	/** Open file in the 'Data' directory in case-insensitive manner. */
	static RustPointer<File> openForReadingCaseInsensitive(const ST::string& folderPath, const ST::string& filename);

	/* ------------------------------------------------------------
	 * Other operations
	 * ------------------------------------------------------------ */

	/** Create directory.
	 * If directory already exists, do nothing.
	 * If failed to create, raise an exception. */
	static void createDir(const ST::string& path);

	/** Join two path components. */
	static ST::string joinPaths(const ST::string& first, const ST::string& second);

	/** Join multiple path components. */
	static ST::string joinPaths(const std::vector<ST::string> parts);

	/** Replace extension of a file. */
	static ST::string replaceExtension(const ST::string& path, const ST::string& newExtension);

	/** Get parent path (e.g. directory path from the full path). */
	static ST::string getParentPath(const ST::string &path, bool absolute);

	/** Get filename from the path. */
	static ST::string getFileName(const ST::string &path);

	/** Get filename from the path without extension. */
	static ST::string getFileNameWithoutExt(const ST::string& path);

	static RustPointer<File> openFileForReading(const ST::string& path);

	/** Open file in the given folder in case-insensitive manner.
	 * @return file descriptor or null if file is not found. */
	static RustPointer<File> openFileCaseInsensitive(const ST::string& folderPath, const ST::string& filename, uint8_t open_options);

	/** Convert File to HWFile. */
	static SGPFile* getSGPFileFromFile(File* f);

	/** Replace all \ with / */
	static void slashifyPath(ST::string &path);

	/** Check file existance. */
	static bool checkFileExistance(const ST::string& folder, const ST::string& fileName);

	/** Returns if the given path (dir or file) exists */
	static bool checkPathExistance(const ST::string& path);

	/** Move a file */
	static void moveFile(const ST::string& from, const ST::string& to);

private:
	/** Private constructor to avoid instantiation. */
	FileMan() {};
};

/**
 * Find all files with the given extension in the given directory.
 * @param dirPath Path to the directory
 * @param extension Extension (e.g. "txt")
 * @param caseIncensitive When True, do case-insensitive search even of case-sensitive file-systems.
 * @param returnOnlyNames When True, return only names (without the directory path) except when resursive is True
 * @param sortResults When True, sort found paths.
 * @param recursive When True, recurse into subs. Function returns full path regardless of returnOnlyNames
 * @return List of paths (dir + filename) or filenames. */
std::vector<ST::string>
FindFilesInDir(const ST::string& dirPath,
		const ST::string& ext,
		bool caseInsensitive,
		bool returnOnlyNames,
		bool sortResults = false,
		bool recursive = false);

/**
 * Find all files in a directory.
 * @param dirPath Path to the directory
 * @param sortResults When True, sort found paths.
 * @param recursive When True, recurse into subs.
 * @return List of paths (dir + filename). */
std::vector<ST::string>
FindAllFilesInDir(const ST::string& dirPath, bool sortResults = false, bool recursive = false);

#endif
