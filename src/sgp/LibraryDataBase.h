#ifndef _LIBRARY_DATABASE_H
#define _LIBRARY_DATABASE_H

#include <string>
#include <vector>

#include "sgp/SGPFile.h"
#include "sgp/Types.h"

#define REAL_FILE_LIBRARY_ID		1022

#define DB_BITS_FOR_LIBRARY		10
#define DB_BITS_FOR_FILE_ID		22

#define DB_EXTRACT_LIBRARY( exp )	( exp >> DB_BITS_FOR_FILE_ID )
#define DB_EXTRACT_FILE_ID( exp )	( exp & 0x3FFFFF )

#define DB_ADD_LIBRARY_ID( exp )	( exp << DB_BITS_FOR_FILE_ID )
#define DB_ADD_FILE_ID( exp )		( exp & 0xC00000 )


struct FileHeaderStruct
{
	char*  pFileName;
	UINT32	uiFileLength;
	UINT32	uiFileOffset;
};


struct LibraryHeaderStruct
{
	std::string sLibraryPath;
	FILE* hLibraryHandle;
	UINT16	usNumberOfEntries;
	INT32		iNumFilesOpen;
	FileHeaderStruct *pFileHeader;
};

class LibraryDB
{
public:
	/** Initialize file database.
	 * @return NULL when successful, otherwise the name of failed library. */
	const char* InitializeFileDatabase(const std::string &dataDir, const std::vector<std::string> &libraries);

	/* Find library which can contain the given file.
	 * File name should use / (not \\). */
	LibraryHeaderStruct* GetLibraryFromFileName(const std::string &filename);

	/** Find file in the library.
	 * Name of the file should use / not \\. */
	BOOLEAN FindFileInTheLibrarry(const std::string &filename, LibraryFile* f);

	/** Check if file exists in the library.
	 * Name of the file should use / (not \\). */
	bool CheckIfFileExistInLibrary(const std::string &filename);

	void ShutDownFileDatabase();

protected:
	std::vector<LibraryHeaderStruct> m_libraries;
};


/* Close an individual file that is contained in the library */
void    CloseLibraryFile(LibraryFile*);
BOOLEAN LoadDataFromLibrary(LibraryFile*, void* pData, UINT32 uiBytesToRead);
BOOLEAN LibraryFileSeek(LibraryFile*, INT32 distance, FileSeekMode);

#endif
