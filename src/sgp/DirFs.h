#pragma once

#include "SGPFile.h"

#include <string_theory/string>


/** Provides oprations for files within a subdirectory.
 *  Should be kept in sync with FileMan namespace to provide the same interface.
 */
class DirFs
{
private:
	ST::string m_basePath;
public:
	/** Create a DirFs with base path */
	DirFs(const ST::string& path) : m_basePath(path) {};

	/** Return absolute path for file within DirFs */
	ST::string basePath();

	/** Return absolute path for file within DirFs */
	ST::string absolutePath(const ST::string &path);

	/** Open file for writing.
	 * If file is missing it will be created.
	 * If file exists, it's content will be removed. */
	SGPFile *openForWriting(const ST::string &path, bool truncate = true);

	/** Open file for appending data.
	 * If file doesn't exist, it will be created. */
	SGPFile *openForAppend(const ST::string &path);

	/** Open file for reading and writing.
	 * If file doesn't exist, it will be created. */
	SGPFile *openForReadWrite(const ST::string &path);

	/** Open file for reading. */
	SGPFile *openForReading(const ST::string &path);

	/* Delete the file at path. */
	void deleteFile(const ST::string &path);

	/* ------------------------------------------------------------
	 * Other operations
	 * ------------------------------------------------------------ */

	/** Create directory.
	 * If directory already exists, do nothing.
	 * If failed to create, raise an exception. */
	void createDir(const ST::string &path);

	/* Removes ALL FILES in the specified directory, but leaves the directory alone.
 	 * Does not affect any subdirectories! */
	void eraseDir(const ST::string &path);

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
	findFilesInDir(const ST::string &path,
				   const ST::string &ext,
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
	findAllFilesInDir(const ST::string &path, bool sortResults = false, bool recursive = false, bool returnOnlyNames = false);

	/**
	 * Find all directories in directory
	 * @param dirPath Path to the directory
	 * @param sortResults When True, sort found paths.
	 * @param recursive When True, recurse into subs.
	 * @return List of paths (dir + filename). */
	std::vector<ST::string>
	findAllDirsInDir(const ST::string &path, bool sortResults = false, bool recursive = false, bool returnOnlyNames = false);

	/* Resolve existing components of a path in a case insensitive manner */
	ST::string resolveExistingComponents(const ST::string &path);

	/** Check if path exists and is a file */
	bool isFile(const ST::string &path);

	/** Check if path exists and is a directory */
	bool isDir(const ST::string &path);

	/** Check if path is read only. Throws when path does not exist */
	bool isReadOnly(const ST::string &path);

	/** Returns if the given path (dir or file) exists */
	bool exists(const ST::string &path);

	/** Move a file */
	void moveFile(const ST::string &from, const ST::string &to);

	/** Get last modified time in seconds since UNIX epoch */
	double getLastModifiedTime(const ST::string &path);
};
