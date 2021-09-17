#pragma once

#include "sgp/SGPFile.h"
#include "sgp/Types.h"

#include <string_theory/string>

#include <vector>

/***
 * The file manager namespace provides functions to interact with files
 */
namespace FileMan
{
	/** Open file for writing.
	 * If file is missing it will be created.
	 * If file exists, it's content will be removed. */
	SGPFile* openForWriting(const ST::string& filename, bool truncate=true);

	/** Open file for appending data.
	 * If file doesn't exist, it will be created. */
	SGPFile* openForAppend(const ST::string& filename);

	/** Open file for reading and writing.
	 * If file doesn't exist, it will be created. */
	SGPFile* openForReadWrite(const ST::string& filename);

	/** Open file for reading. */
	SGPFile* openForReading(const ST::string &filename);

	/* Delete the file at path. */
	void deleteFile(const ST::string &path);

	/* ------------------------------------------------------------
	 * Other operations
	 * ------------------------------------------------------------ */

	/** Create directory.
	 * If directory already exists, do nothing.
	 * If failed to create, raise an exception. */
	void createDir(const ST::string& path);

	/* Removes ALL FILES in the specified directory, but leaves the directory alone.
 	 * Does not affect any subdirectories! */
	void eraseDirectory(const ST::string& dirPath);

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
	findFilesInDir(const ST::string& dirPath,
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
	findAllFilesInDir(const ST::string& dirPath, bool sortResults = false, bool recursive = false, bool returnOnlyNames = false);

	/**
	 * Find all directories in directory
	 * @param dirPath Path to the directory
	 * @param sortResults When True, sort found paths.
	 * @param recursive When True, recurse into subs.
	 * @return List of paths (dir + filename). */
	std::vector<ST::string>
	findAllDirsInDir(const ST::string& dirPath, bool sortResults = false, bool recursive = false, bool returnOnlyNames = false);

	/** Join two path components. */
	ST::string joinPaths(const ST::string& first, const ST::string& second);

	/** Join multiple path components. */
	ST::string joinPaths(const std::vector<ST::string> parts);

	/** Replace extension of a file. */
	ST::string replaceExtension(const ST::string& path, const ST::string& newExtension);

	/** Get parent path (e.g. directory path from the full path). */
	ST::string getParentPath(const ST::string &path, bool absolute);

	/** Get filename from the path. */
	ST::string getFileName(const ST::string &path);

	/** Get filename from the path without extension. */
	ST::string getFileNameWithoutExt(const ST::string& path);

	/* Resolve existing components of a path in a case insensitive manner */
	ST::string resolveExistingComponents(const ST::string& path);

	/** Check if path exists and is a file */
	bool isFile(const ST::string& path);

	/** Check if path exists and is a directory */
	bool isDir(const ST::string& path);

	/** Check if path is absolute */
	bool isAbsolute(const ST::string& path);

	/** Check if path is read only. Throws when path does not exist */
	bool isReadOnly(const ST::string& path);

	/** Check file existance. */
	bool checkFileExistance(const ST::string& folder, const ST::string& fileName);

	/** Returns if the given path (dir or file) exists */
	bool checkPathExistance(const ST::string& path);

	/** Move a file */
	void moveFile(const ST::string& from, const ST::string& to);

	/** Get last modified time in seconds since UNIX epoch */
	double getLastModifiedTime(const ST::string& path);

	//Gets the amount of free space on the hard drive that the main executeablt is runnning from
	uint64_t getFreeSpaceOnHardDriveWhereGameIsRunningFrom(void);
};
