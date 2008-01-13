#ifndef FILEMAN_H
#define FILEMAN_H

#include "Types.h"


enum
{
	FILE_ACCESS_READ      = 1U << 0,
	FILE_ACCESS_WRITE     = 1U << 1,
	FILE_ACCESS_READWRITE = FILE_ACCESS_READ | FILE_ACCESS_WRITE,
	FILE_CREATE_ALWAYS    = 1U << 2, // create new file. overwrite existing
	FILE_OPEN_ALWAYS      = 1U << 3  // open a file, create if doesn't exist
};

enum
{
	FILE_SEEK_FROM_START,
	FILE_SEEK_FROM_END,
	FILE_SEEK_FROM_CURRENT
};

// GetFile file attributes
enum
{
	FILE_IS_READONLY   = 1U << 0,
	FILE_IS_DIRECTORY  = 1U << 1,
	FILE_IS_HIDDEN     = 1U << 2,
	FILE_IS_NORMAL     = 1U << 3,
	FILE_IS_ARCHIVE    = 1U << 4,
	FILE_IS_SYSTEM     = 1U << 5,
	FILE_IS_TEMPORARY  = 1U << 6,
	FILE_IS_COMPRESSED = 1U << 7,
	FILE_IS_OFFLINE    = 1U << 8,
};

//File Attributes settings
#define FILE_ATTRIBUTES_ARCHIVE   FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTES_HIDDEN    FILE_ATTRIBUTE_HIDDEN
#define FILE_ATTRIBUTES_NORMAL    FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTES_OFFLINE   FILE_ATTRIBUTE_OFFLINE
#define FILE_ATTRIBUTES_READONLY  FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTES_SYSTEM    FILE_ATTRIBUTE_SYSTEM
#define FILE_ATTRIBUTES_TEMPORARY FILE_ATTRIBUTE_TEMPORARY
#define FILE_ATTRIBUTES_DIRECTORY FILE_ATTRIBUTE_DIRECTORY

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
UINT32 FileSize(const char* filename);

BOOLEAN     SetFileManCurrentDirectory(const char* pcDirectory);
BOOLEAN     GetFileManCurrentDirectory(STRING512 pcDirectory);
const char* GetExecutableDirectory(void);

BOOLEAN MakeFileManDirectory(const char* pcDirectory);

// WARNING: THESE DELETE ALL FILES IN THE DIRECTORY (and all subdirectories if fRecursive is TRUE!!)
BOOLEAN RemoveFileManDirectory(const char* pcDirectory, BOOLEAN fRecursive);
BOOLEAN EraseDirectory(const char* pcDirectory);

typedef struct GETFILESTRUCT
{
	INT32  iFindHandle;
	char   zFileName[260];
	UINT32 uiFileAttribs;
} GETFILESTRUCT;

BOOLEAN GetFileFirst(const char* pSpec, GETFILESTRUCT* pGFStruct);
BOOLEAN GetFileNext(GETFILESTRUCT* pGFStruct);
void    GetFileClose(GETFILESTRUCT* pGFStruct);

UINT32  FileGetAttributes(const char* filename);
BOOLEAN FileClearAttributes(const char* filename);

// returns true if at end of file, else false
BOOLEAN FileCheckEndOfFile(HWFILE hFile);

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
