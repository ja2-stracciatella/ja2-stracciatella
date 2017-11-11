#include <cstdlib>
#include <stdexcept>

#include "Types.h"
#include "FileMan.h"
#include "LibraryDataBase.h"
#include "MemMan.h"
#include "Debug.h"
#include "StrUtils.h"

#include "slog/slog.h"

#define FILENAME_SIZE		256


#define FILE_OK		0x00
#define FILE_DELETED		0xFF
#define FILE_OLD		0x01
#define FILE_DOESNT_EXIST	0xFE


// NOTE:  The following structs are also used by the datalib98 utility
struct LIBHEADER
{
	char    sLibName[FILENAME_SIZE];
	char    sPathToLibrary[FILENAME_SIZE];
	INT32   iEntries;
	INT32   iUsed;
	UINT16  iSort;
	UINT16  iVersion;
	BOOLEAN	fContainsSubDirectories;
	INT32   iReserved;
};


struct DIRENTRY
{
	char         sFileName[FILENAME_SIZE];
	UINT32       uiOffset;
	UINT32       uiLength;
	UINT8        ubState;
	UINT8        ubReserved;
	SGP_FILETIME sFileTime;
	UINT16       usReserved2;
};

static BOOLEAN InitializeLibrary(const std::string &dataDir, const char* pLibraryName, LibraryHeaderStruct* pLibHeader);


/** Initialize file database.
 * @return NULL when successful, otherwise the name of failed library. */
const char* LibraryDB::InitializeFileDatabase(const std::string &dataDir, const std::vector<std::string> &libs)
{
	for (int i = 0; i < libs.size(); i++)
	{
		LibraryHeaderStruct lib;
		if (!InitializeLibrary(dataDir, libs[i].c_str(), &lib))
		{
			return libs[i].c_str();
		}
		m_libraries.push_back(lib);
	}
	return NULL;
}


static BOOLEAN CloseLibrary(LibraryHeaderStruct *lib);


void LibraryDB::ShutDownFileDatabase()
{
	for(int i = 0; i < m_libraries.size(); i++)
	{
		CloseLibrary(&m_libraries[i]);
	}
}


static int CompareFileHeader(const void* a, const void* b)
{
	const FileHeaderStruct* fha = (const FileHeaderStruct*)a;
	const FileHeaderStruct* fhb = (const FileHeaderStruct*)b;
	return strcasecmp(fha->pFileName, fhb->pFileName);
}


// Replace all \ in a string by /
static char* Slashify(const char* s)
{
	char* const res = MALLOCN(char, strlen(s) + 1);
	char* d = res;
	do { *d++ = (*s == '\\' ? '/' : *s); } while (*s++ != '\0');
	return res;
}


static BOOLEAN InitializeLibrary(const std::string &dataDir, const char* const lib_name, LibraryHeaderStruct* const lib)
try
{
	FILE* hFile = FileMan::openForReadingCaseInsensitive(dataDir, lib_name);
	if (hFile == NULL)
	{
			fprintf(stderr, "ERROR: Failed to open library \"%s\"\n", lib_name);
			return FALSE;
	}

	// read in the library header (at the begining of the library)
	LIBHEADER	LibFileHeader;
	if (fread(&LibFileHeader, sizeof(LibFileHeader), 1, hFile) != 1) return FALSE;

	const UINT32 count_entries = LibFileHeader.iEntries;

	FileHeaderStruct* fhs = MALLOCN(FileHeaderStruct, count_entries);

	/* place the file pointer at the begining of the file headers (they are at the
	 * end of the file) */
	fseek(hFile, -(int)(count_entries * sizeof(DIRENTRY)), SEEK_END);

	UINT32 used_entries = 0;
	for (UINT32 uiLoop = 0; uiLoop < count_entries; ++uiLoop)
	{
		DIRENTRY DirEntry;
		if (fread(&DirEntry, sizeof(DirEntry), 1, hFile) != 1) return FALSE;

		if (DirEntry.ubState != FILE_OK) continue;

		// check to see if the file is not longer than it should be
		if (strlen(DirEntry.sFileName) + 1 >= FILENAME_SIZE)
		{
			SLOGW(DEBUG_TAG_LIBDB, "'%s' from the library '%s' has too long name",
				DirEntry.sFileName, lib->sLibraryPath.c_str());
		}

		FileHeaderStruct* const fh = &fhs[used_entries++];

		fh->pFileName = Slashify(DirEntry.sFileName);

		fh->uiFileOffset = DirEntry.uiOffset;
		fh->uiFileLength = DirEntry.uiLength;

		// SLOGD(DEBUG_TAG_LIBDB, "found in %s: %s", lib_name, fh->pFileName);
	}

	if (used_entries != count_entries)
	{
		fhs = REALLOC(fhs, FileHeaderStruct, used_entries);
	}

	qsort(fhs, used_entries, sizeof(*fhs), CompareFileHeader);

	lib->pFileHeader       = fhs;
	lib->usNumberOfEntries = used_entries;

	lib->sLibraryPath = LibFileHeader.sPathToLibrary;
	FileMan::slashifyPath(lib->sLibraryPath);

	lib->hLibraryHandle = hFile;
	lib->iNumFilesOpen  = 0;
	return TRUE;
}
catch (...) { return 0; }


BOOLEAN LoadDataFromLibrary(LibraryFile* const f, void* const pData, const UINT32 uiBytesToRead)
{
	if (f->pFileHeader == NULL) return FALSE;

	UINT32 const uiOffsetInLibrary = f->pFileHeader->uiFileOffset;
	UINT32 const uiLength          = f->pFileHeader->uiFileLength;
	FILE*  const hLibraryFile      = f->lib->hLibraryHandle;
	UINT32 const uiCurPos          = f->uiFilePosInFile;

	fseek(hLibraryFile, uiOffsetInLibrary + uiCurPos, SEEK_SET);

	// if we are trying to read more data than the size of the file, return an error
	if (uiBytesToRead + uiCurPos > uiLength) return FALSE;

	if (fread(pData, uiBytesToRead, 1, hLibraryFile) != 1) return FALSE;

	f->uiFilePosInFile += uiBytesToRead;
	return TRUE;
}


static const FileHeaderStruct* GetFileHeaderFromLibrary(const LibraryHeaderStruct* lib, const std::string &filename);


/** Check if file exists in the library.
 * Name of the file should use / (not \\). */
bool LibraryDB::CheckIfFileExistInLibrary(const std::string &filename)
{
	LibraryHeaderStruct const* const lib = GetLibraryFromFileName(filename);
	return lib && GetFileHeaderFromLibrary(lib, filename);
}


static BOOLEAN IsLibraryOpened(const LibraryHeaderStruct *lib);


/* Find library which can contain the given file.
 * File name should use / (not \\). */
LibraryHeaderStruct* LibraryDB::GetLibraryFromFileName(const std::string &filename)
{
	bool hasDirectoryInPath = filename.find('/') != std::string::npos;

	// Loop through all the libraries to check which library the file is in
	LibraryHeaderStruct* best_match = 0;

	for(int i = 0; i < m_libraries.size(); i++)
	{
		LibraryHeaderStruct* lib = &m_libraries[i];

		if (!IsLibraryOpened(lib)) continue;

		const char * lib_path = lib->sLibraryPath.c_str();
		if (lib_path[0] == '\0')
		{ // The library is for the default path
			if(hasDirectoryInPath) continue;
			// There is no directory in the file name
			return lib;
		}
		else
		{ // Compare the library name to the file name that is passed in
			size_t const lib_path_len = strlen(lib_path);
			if (strncasecmp(lib_path, filename.c_str(), lib_path_len) != 0) continue;
			// The directory paths are the same to the length of the lib's path
			if (best_match && strlen(best_match->sLibraryPath.c_str()) >= lib_path_len) continue;
			// We've never matched or this match's path is longer than the previous match (meaning it's more exact)
			best_match = lib;
		}
	}
	return best_match;
}


static int CompareFileNames(const void* key, const void* member);


static const char* g_current_lib_path;


/* Performsperforms a binary search of the library.  It adds the libraries path
 * to the file in the library and then string compared that to the name that we
 * are searching for. */
static const FileHeaderStruct* GetFileHeaderFromLibrary(const LibraryHeaderStruct* const lib, const std::string &filename)
{
	g_current_lib_path = lib->sLibraryPath.c_str();
	return (const FileHeaderStruct*)bsearch(filename.c_str(), lib->pFileHeader, lib->usNumberOfEntries, sizeof(*lib->pFileHeader), CompareFileNames);
}


static int CompareFileNames(const void* key, const void* member)
{
	const FileHeaderStruct* const TempFileHeader = (const FileHeaderStruct*)member;

	// XXX: need to optimize this
	// XXX: the whole thing requires refactoring
	char sFileNameWithPath[FILENAME_SIZE];
	sprintf(sFileNameWithPath, "%s%s", g_current_lib_path, TempFileHeader->pFileName);

	return strcasecmp((const char*)key, sFileNameWithPath);
}


/** Find file in the library.
 * Name of the file should use / not \\. */
BOOLEAN LibraryDB::FindFileInTheLibrarry(const std::string &filename, LibraryFile* f)
{
	//Check if the file can be contained from an open library ( the path to the file a library path )
	LibraryHeaderStruct* const lib = GetLibraryFromFileName(filename);
	if (!lib) return FALSE;

	//if the file is in a library, get the file
	const FileHeaderStruct* const pFileHeader = GetFileHeaderFromLibrary(lib, filename);
	if (pFileHeader == NULL) return FALSE;

	//increment the number of open files
	lib->iNumFilesOpen++;

	f->lib         = lib;
	f->pFileHeader = pFileHeader;
	return TRUE;
}


void CloseLibraryFile(LibraryFile* const f)
{
	--f->lib->iNumFilesOpen;
}


BOOLEAN LibraryFileSeek(LibraryFile* const f, INT32 distance, const FileSeekMode how)
{
	UINT32 pos;
	switch (how)
	{
		case FILE_SEEK_FROM_START:   pos = 0;                            break;
		case FILE_SEEK_FROM_END:     pos = f->pFileHeader->uiFileLength; break;
		case FILE_SEEK_FROM_CURRENT: pos = f->uiFilePosInFile;           break;
		default:                     return FALSE;
	}
	f->uiFilePosInFile = pos + distance;
	return TRUE;
}


static BOOLEAN CloseLibrary(LibraryHeaderStruct *lib)
{
	UINT32	uiLoop1;

	//if the library isnt loaded, dont close it
	if( !IsLibraryOpened(lib) )
		return( FALSE );

	//if there are any open files, loop through the library and close down whatever file is still open
	if (lib->iNumFilesOpen)
	{
		SLOGE(DEBUG_TAG_LIBDB, "CloseLibrary(): %s library still has %d open files.", lib->sLibraryPath.c_str(), lib->iNumFilesOpen);
	}

	//Free up the memory used for each file name
	for (uiLoop1 = 0; uiLoop1 < lib->usNumberOfEntries; uiLoop1++)
	{
		MemFree(lib->pFileHeader[uiLoop1].pFileName);
		lib->pFileHeader[uiLoop1].pFileName = NULL;
	}

	//Free up the memory needed for the Library File Headers
	if (lib->pFileHeader)
	{
		MemFree(lib->pFileHeader);
		lib->pFileHeader = NULL;
	}

	fclose(lib->hLibraryHandle);
	lib->hLibraryHandle = NULL;

	return( TRUE );
}


static BOOLEAN IsLibraryOpened(const LibraryHeaderStruct *lib)
{
	return lib->hLibraryHandle != NULL;
}

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(LibraryDatabase, asserts)
{
	EXPECT_EQ(sizeof(LIBHEADER), 532);
	EXPECT_EQ(sizeof(DIRENTRY), 280);
}

#endif
