#ifndef FILEMAN_H
#define FILEMAN_H

#include "Types.h"


enum
{
	FILE_ACCESS_READ      = 1U << 0,
	FILE_ACCESS_WRITE     = 1U << 1,
	FILE_ACCESS_READWRITE = FILE_ACCESS_READ | FILE_ACCESS_WRITE,
	FILE_ACCESS_APPEND    = 1U << 2,
	FILE_CREATE_ALWAYS    = 1U << 3, // create new file. overwrite existing
	FILE_OPEN_ALWAYS      = 1U << 4  // open a file, create if doesn't exist
};

enum
{
	FILE_SEEK_FROM_START,
	FILE_SEEK_FROM_END,
	FILE_SEEK_FROM_CURRENT
};

typedef struct SGP_FILETIME
{
	UINT32 Lo;
	UINT32 Hi;
} SGP_FILETIME;


#ifdef __cplusplus
extern "C" {
#endif


BOOLEAN InitializeFileManager(void);

/* Checks if a file exists. */
BOOLEAN FileExists(const char* filename);
/* Checks if a file exists, but doesn't check the database files. */
BOOLEAN FileExistsNoDB(const char* filename);

/* Delete the file at path. Returns true iff deleting the file succeeded or
 * the file did not exist in the first place. */
BOOLEAN FileDelete(const char* path);

HWFILE FileOpen(const char* filename, UINT32 uiOptions);
void   FileClose(HWFILE);

BOOLEAN FileRead(HWFILE hFile, void* pDest, UINT32 uiBytesToRead);
BOOLEAN FileWrite(HWFILE hFile, const void* pDest, UINT32 uiBytesToWrite);

BOOLEAN FileSeek(HWFILE, INT32 distance, INT how);
INT32   FileGetPos(HWFILE);

UINT32 FileGetSize(HWFILE);

BOOLEAN     SetFileManCurrentDirectory(const char* pcDirectory);
const char* GetExecutableDirectory(void);

BOOLEAN MakeFileManDirectory(const char* pcDirectory);

// WARNING: THESE DELETE ALL FILES IN THE DIRECTORY
BOOLEAN EraseDirectory(const char* pcDirectory);


typedef struct FindFileInfo FindFileInfo;

/* Search for files designated by pattern.  Returns NULL on error.  No match
 * does not qualify as error. */
FindFileInfo* FindFiles(const char* pattern);

/* Get the next filename of a search started by FindFiles().  Returns NULL, iff
 * there are no more files or an error occured. */
const char* FindFilesNext(FindFileInfo*);

/* Free the data returned by a successful call to FindFiles() */
void FindFilesFree(FindFileInfo*);


typedef enum FileAttributes
{
	FILE_ATTR_NONE      = 0,
	FILE_ATTR_READONLY  = 1U << 0,
	FILE_ATTR_DIRECTORY = 1U << 1,
	FILE_ATTR_ERROR     = 0xFFFFFFFFU
} FileAttributes;

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

const char* GetBinDataPath(void);

#ifdef __cplusplus
}
#endif

#endif
