#pragma once

#include "RustInterface.h"
#include "SGPFile.h"

#include <memory>
#include <string_theory/string>


/** Provides oprations for files within a subdirectory.
 *  Should be kept in sync with FileMan namespace to provide the same interface.
 *
 *  All paths are relative to the root of the DirFs and are resolved case insensitively, because
 *  the class is a thin wrapper around a writable VFS layer.
 */
class DirFs
{
private:
	ST::string m_location;
	RustPointer<VfsLayer> m_layer;

	/** Create a DirFs for a layer that is already open. */
	DirFs(RustPointer<VfsLayer> layer, ST::string location)
		: m_location(std::move(location)), m_layer(std::move(layer)) {};

	/** The underlying VFS layer, opened on first use. Throws when it cannot be opened.
	 *  Opening is deferred so that creating a DirFs for a directory that does not exist yet only
	 *  fails once something uses it, which is how this behaved when it was a plain path. */
	VfsLayer *layer();

	/** The underlying VFS layer, or null when it cannot be opened. */
	VfsLayer *layerOrNull() noexcept;

	/** Open file with the given options. Throws when it fails. */
	SGPFile *open(const ST::string &path, uint8_t options, const char *operation);

public:
	/** Create a DirFs for a filesystem directory. */
	explicit DirFs(ST::string location) : m_location(std::move(location)) {};

	/** Create a DirFs for a fresh temporary directory.
	 * The directory and everything in it is deleted when the DirFs is destroyed. */
	static std::unique_ptr<DirFs> createTemp();

	/** Return where the files are, for diagnostics. */
	ST::string basePath();

	/** Move a file from one DirFs to another.
	 * Falls back to copying when the two are not both backed by a filesystem directory. */
	static void moveFileBetween(DirFs &fromFs, const ST::string &from, DirFs &toFs, const ST::string &to);

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

	/** Create directory, including missing parents.
	 * If directory already exists, do nothing.
	 * If failed to create, raise an exception. */
	void createDir(const ST::string &path);

	/* Removes ALL FILES in the specified directory, but leaves the directory alone.
 	 * Does not affect any subdirectories! */
	void eraseDir(const ST::string &path);

	/**
	 * Find all files with the given extension in the given directory.
	 * @param path Path to the directory
	 * @param ext Extension (e.g. "txt")
	 * @param caseInsensitive Ignored, the lookup is always case-insensitive.
	 * @param returnOnlyNames When True, return only names (without the directory path) except when recursive is True
	 * @param sortResults Ignored, the results are always sorted.
	 * @param recursive When True, recurse into subs.
	 * @return List of paths relative to the DirFs, or filenames. */
	std::vector<ST::string>
	findFilesInDir(const ST::string &path,
				   const ST::string &ext,
				   bool caseInsensitive,
				   bool returnOnlyNames,
				   bool sortResults = false,
				   bool recursive = false);

	/**
	 * Find all files in a directory.
	 * @param path Path to the directory
	 * @param sortResults Ignored, the results are always sorted.
	 * @param recursive When True, recurse into subs.
	 * @return List of paths relative to the DirFs. */
	std::vector<ST::string>
	findAllFilesInDir(const ST::string &path, bool sortResults = false, bool recursive = false, bool returnOnlyNames = false);

	/**
	 * Find all directories in directory
	 * @param path Path to the directory
	 * @param sortResults Ignored, the results are always sorted.
	 * @param recursive When True, recurse into subs.
	 * @return List of paths relative to the DirFs. */
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

	/** Move a file within this DirFs */
	void moveFile(const ST::string &from, const ST::string &to);

	/** Get last modified time in seconds since UNIX epoch */
	double getLastModifiedTime(const ST::string &path);

	/** Gets the amount of free space on the harddrive in a directory */
	uint64_t getFreeSpace(const ST::string& path);
};
