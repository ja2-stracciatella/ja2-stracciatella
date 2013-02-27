#ifndef FILEMAN_H
#define FILEMAN_H

#include "AutoObj.h"
#include "Types.h"

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <glob.h>
#endif


enum FileOpenFlags
{
	FILE_ACCESS_READ      = 1U << 0,
	FILE_ACCESS_WRITE     = 1U << 1,
	FILE_ACCESS_READWRITE = FILE_ACCESS_READ | FILE_ACCESS_WRITE,
	FILE_ACCESS_APPEND    = 1U << 2,
	FILE_CREATE_ALWAYS    = 1U << 3, // create new file. overwrite existing
	FILE_OPEN_ALWAYS      = 1U << 4  // open a file, create if doesn't exist
};
ENUM_BITSET(FileOpenFlags)

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

/* Delete the file at path. Returns true iff deleting the file succeeded or
 * the file did not exist in the first place. */
void FileDelete(char const* path);

HWFILE FileOpen(const char* filename, FileOpenFlags);
void   FileClose(HWFILE);

void FileRead( HWFILE, void*       pDest, UINT32 uiBytesToRead);
void FileWrite(HWFILE, void const* pDest, UINT32 uiBytesToWrite);

template<typename T, typename U> static inline void FileWriteArray(HWFILE const f, T const& n, U const* const data)
{
	FileWrite(f, &n, sizeof(n));
	if (n != 0) FileWrite(f, data, sizeof(*data) * n);
}

void  FileSeek(HWFILE, INT32 distance, FileSeekMode);
INT32 FileGetPos(HWFILE);

UINT32 FileGetSize(HWFILE);

const char* GetExecutableDirectory(void);

/* Create the directory at path.  Returns true, iff the creation succeeded or
 * the directory exists already. */
void MakeFileManDirectory(char const* path);

/* Removes ALL FILES in the specified directory, but leaves the directory alone.
 * Does not affect any subdirectories! */
void EraseDirectory(char const* pcDirectory);


namespace SGP
{
	class FindFiles
	{
		public:
			/* Search for files designated by pattern.  No match causes Next() to return
			 * 0 the first time it is called. */
			FindFiles(char const* pattern);
			~FindFiles();

			/* Get the next filename.  Returns 0, iff there are no more files. */
			char const* Next();

		private:
#ifdef _WIN32
			HANDLE          find_handle_;
			WIN32_FIND_DATA find_data_;
			bool            first_done_;
#else
			size_t index_;
			glob_t glob_data_;
#endif
	};
}


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

/** Get path to the 'Data' directory of the game. */
const char* GetDataDirPath();

/** Get path to the 'Data/Tilecache' directory of the game. */
const char* GetTilecacheDirPath();

/** Open file in the 'Data' directory in case-insensitive manner. */
FILE* OpenFileInDataDir(const char *filename, FileOpenFlags flags);

#endif
