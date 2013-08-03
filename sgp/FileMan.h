#ifndef FILEMAN_H
#define FILEMAN_H

#include <string>
#include <vector>

#include "AutoObj.h"
#include "Types.h"

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <glob.h>
#endif


enum FileSeekMode
{
	FILE_SEEK_FROM_START,
	FILE_SEEK_FROM_END,
	FILE_SEEK_FROM_CURRENT
};

struct SGP_FILETIME
{
	UINT32 Lo;
	UINT32 Hi;
};


void InitializeFileManager(void);

/* Checks if a file exists. */
bool FileExists(char const* filename);
bool FileExists(const std::string &filename);

/* Delete the file at path. Returns true iff deleting the file succeeded or
 * the file did not exist in the first place. */
void FileDelete(char const* path);

void   FileClose(HWFILE);

void FileRead( HWFILE, void*       pDest, size_t uiBytesToRead);
void FileWrite(HWFILE, void const* pDest, size_t uiBytesToWrite);

template<typename T, typename U> static inline void FileWriteArray(HWFILE const f, T const& n, U const* const data)
{
	FileWrite(f, &n, sizeof(n));
	if (n != 0) FileWrite(f, data, sizeof(*data) * n);
}

void  FileSeek(HWFILE, INT32 distance, FileSeekMode);
INT32 FileGetPos(HWFILE);

UINT32 FileGetSize(HWFILE);

/* Removes ALL FILES in the specified directory, but leaves the directory alone.
 * Does not affect any subdirectories! */
void EraseDirectory(char const* pcDirectory);

enum FileAttributes
{
	FILE_ATTR_NONE      = 0,
	FILE_ATTR_READONLY  = 1U << 0,
	FILE_ATTR_DIRECTORY = 1U << 1,
	FILE_ATTR_ERROR     = 0xFFFFFFFFU
};
ENUM_BITSET(FileAttributes)

FileAttributes FileGetAttributes(const char* filename);
BOOLEAN        FileClearAttributes(const char* filename);

BOOLEAN GetFileManFileTime(HWFILE hFile, SGP_FILETIME* pCreationTime, SGP_FILETIME* pLastAccessedTime, SGP_FILETIME* pLastWriteTime);

/* returns
 * - -1 if the First file time is less than second file time. (first file is older)
 * -  0 First file time is equal to second file time.
 * - +1 First file time is greater than second file time (first file is newer). */
INT32 CompareSGPFileTimes(const SGP_FILETIME* const pFirstFileTime, const SGP_FILETIME* const pSecondFileTime);

/* Pass in the Fileman file handle of an OPEN file and it will return..
 * - if its a Real File, the return will be the handle of the REAL file
 * - if its a LIBRARY file, the return will be the handle of the LIBRARY */
FILE* GetRealFileHandleFromFileManFileHandle(HWFILE hFile);

//Gets the amount of free space on the hard drive that the main executeablt is runnning from
UINT32 GetFreeSpaceOnHardDriveWhereGameIsRunningFrom(void);

typedef SGP::AutoObj<SGPFile, FileClose> AutoSGPFile;

/***
 * New file manager.
 *
 * This class provides clean interface for file operations. */
class FileMan
{
public:

  /** Open file for writing.
   * If file is missing it will be created.
   * If file exists, it's content will be removed. */
  static HWFILE openForWriting(const char *filename);

  /** Open file for appending data.
   * If file doesn't exist, it will be created. */
  static HWFILE openForAppend(const char *filename);

  /** Open file for reading and writing.
   * If file doesn't exist, it will be created. */
  static HWFILE openForReadWrite(const char *filename);

  /* ------------------------------------------------------------
   * File operations with game resources.
   * Game resources is what located in 'Data' directory and below.
   * ------------------------------------------------------------ */

  /** Get path to the configuration folder. */
  static const std::string& getConfigFolderPath();

  /** Get path to the configuration file. */
  static const std::string& getConfigPath();

  /** Get path to the root folder of game resources. */
  static const std::string& getGameResRootPath();

  /** Get path to the 'Data' directory of the game. */
  static const std::string& getDataDirPath();

  /** Get path to the 'Data/Tilecache' directory of the game. */
  static const std::string& getTilecacheDirPath();

  /** Get path to the 'Data/Maps' directory of the game. */
  static const std::string& getMapsDirPath();

  /** Open file in the 'Data' directory in case-insensitive manner. */
  static FILE* openForReadingInDataDir(const char *filename);

  /** Open file for reading only.
   * When using the smart lookup:
   *  - first try to open file normally.
   *    It will work if the path is absolute and the file is found or path is relative to the current directory
   *    and file is present;
   *  - if file is not found, try to find the file relatively to 'Data' directory;
   *  - if file is not found, try to find the file in libraries located in 'Data' directory; */
  static HWFILE openForReadingSmart(const char* filename, bool useSmartLookup);

  /* ------------------------------------------------------------
   * Other operations
   * ------------------------------------------------------------ */

  /** Create directory.
   * If directory already exists, do nothing.
   * If failed to create, raise an exception. */
  static void createDir(char const* path);

  /** Join two path components. */
  static std::string joinPaths(const char *first, const char *second);

  /** Join two path components. */
  static std::string joinPaths(const std::string &first, const char *second);

  /** Join two path components. */
  static std::string joinPaths(const std::string &first, const std::string &second);

  /** Replace extension of a file. */
  static std::string replaceExtension(const std::string &path, const char *newExtensionWithDot);

  /** Get filename from the path. */
  static std::string getFileName(const std::string &path);

  /** Get filename from the path without extension. */
  static std::string getFileNameWithoutExt(const char *path);
  static std::string getFileNameWithoutExt(const std::string &path);

private:
  /** Private constructor to avoid instantiation. */
  FileMan() {};
};

/**
 * Find all files with the given extension in the given directory.
 * @param dirPath Path to the directory
 * @param extension Extension with dot (e.g. ".txt")
 * @param caseIncensitive When True, do case-insensitive search even of case-sensitive file-systems.
 * @param returnOnlyNames When True, return only names (without the director path)
 * @param sortResults When True, sort found paths.
 * @return List of paths (dir + filename). */
std::vector<std::string>
FindFilesInDir(const std::string &dirPath,
               const std::string &ext,
               bool caseIncensitive,
               bool returnOnlyNames,
               bool sortResults = false);

/**
 * Find all files in a directory.
 * @param dirPath Path to the directory
 * @param sortResults When True, sort found paths.
 * @return List of paths (dir + filename). */
std::vector<std::string>
FindAllFilesInDir(const std::string &dirPath, bool sortResults = false);

#endif
