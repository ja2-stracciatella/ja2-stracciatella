//**************************************************************************
//
// Filename :	DbMan.c
//
//	Purpose :	function definitions for the database manager
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	#include <stdlib.h>
	#include <malloc.h>
	#include <stdio.h>
	#include "windows.h"
	#include "FileMan.h"
	#include "MemMan.h"
	#include "DbMan.h"
	#include "Debug.h"
#endif

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#define FILENAME_LENGTH					600
#define IM_FILENAME_LENGTH				100
#define BUCKET_FILENAME_LENGTH		100
#define INITIAL_NUM_HANDLES			20
#define NUM_FILES_TO_ADD_AT_A_TIME	20

#define CHECKF(exp)  if (!(exp)) { return(FALSE); }
#define CHECKV(exp)  if (!(exp)) { return; }
#define CHECKN(exp)  if (!(exp)) { return(NULL); }
#define CHECKBI(exp) if (!(exp)) { return(-1); }
#define CHECK0(exp)  if (!(exp)) { return(0); }

#define PRINT_DEBUG_INFO	FileDebugPrint();

#define ExtractFileIndex( exp )	( (HFILEINDEX)((exp)>>16) )
#define ExtractDbIndex( exp )		( (HDBINDEX)((exp)&0x0000FFFF) )

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************


// NOTE: the file format of the index file will be as follows:
//
//		UINT32 #	- number of entries to follow for filenames starting with '0'
//		entry 1 filename
//		entry 1 databasename
//		entry 2 filename
//		entry 2 databasename
//		...
//		entry # filename
//		entry # databasename
//
//		UINT32 #	- number of entries to follow for filenames starting with '1'
//		entry 1 filename
//		entry 1 databasename
//		entry 2 filename
//		entry 2 databasename
//		...
//		entry # filename
//		entry # databasename
//
//		...
//
//		UINT32 #	- number of entries to follow for filenames starting with 'z'
//		entry 1 filename
//		entry 1 databasename
//		entry 2 filename
//		entry 2 databasename
//		...
//		entry # filename
//		entry # databasename
//
//		UINT32 #### - offset in file to entries for the letter '0'
//		...
//		UINT32 #### - offset in file to entries for the letter '9'
//		UINT32 #### - offset in file to entries for the letter 'a'
//		...
//		UINT32 #### - offset in file to entries for the letter 'z'
//
//	- each set of entries for a letter will comprise a bucket

typedef struct DbBucketTag
{
	CHAR		cFirstLetter;

	CHAR		*pstrFilenames;
	CHAR		*pstrDatabasenames;
	UINT32	uiNumNames;
} DbBucket;

typedef struct DbFileTag
{
	CHAR		strFilename[FILENAME_LENGTH];
	INT32		iFileSize;				// if 0, this structure is not in use
	INT32		iOffsetIntoFile;
	INT32		iCurrentPosition;
} DbFile;

typedef struct DbInfoTag
{
	CHAR		strFilename[FILENAME_LENGTH];
	HWFILE	hFile;					// if 0, this structure is not in use

	DbFile	*pOpenFiles;
	UINT32	uiNumFiles;
} DbInfo;

typedef struct DbSystemTag
{
	DbBucket	bucket;
	CHAR		strIndexFilename[FILENAME_LENGTH];

	DbInfo	*pDBFiles;
	UINT32	uiNumDBFiles;

	BOOLEAN	fDebug;
} DbSystem;

// this is for reading the database file
typedef struct DbHeaderTag
{
	CHAR		strSignature[20];
	INT32		iNumFiles;
	INT32		iOffsetToIndex;
} DbHeader;

// this is for reading the database file
typedef struct IndexMemberTag
{
	CHAR		strFilename[IM_FILENAME_LENGTH];
	INT32		iFileSize;
	INT32		iOffsetIntoFile;
	INT32		time;
} IndexMember;

//**************************************************************************
//
//				Variables
//
//**************************************************************************

DbSystem gdb;

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

void		DbDebugPrint( void );
HDBFILE	CreateDBFileHandle( HFILEINDEX, HDBINDEX );
BOOLEAN	InitDB( DbInfo *pDBInfo, STR strFilename );
HDBINDEX OpenDatabaseContainingFile( STR strFilename );
BOOLEAN	InitFile( HWFILE hDBFile, DbFile *pFileInfo, STR strFilename );
BOOLEAN	LoadBucket( CHAR cFirstLetter );
BOOLEAN	GetShortFilename( STR strFilename, STR strPathname );

//**************************************************************************
//
//				Functions
//
//**************************************************************************

//**************************************************************************
//
// CreateDBFileHandle
//
//		Creates a file handle from two indices.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

HDBFILE CreateDBFileHandle( HFILEINDEX high, HDBINDEX low )
{
	return( (((LONG)high)<<16)|low );
}

//**************************************************************************
//
// DbSystemInit
//
//		Starts up the database system.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN InitializeDatabaseManager( STR strIndexFilename )
{
	INT32		i;
	UINT32	uiSize;

	if ( !FileExistsNoDB( strIndexFilename ) )
	{
		return(FALSE);
	}

	uiSize = INITIAL_NUM_HANDLES * sizeof(DbInfo);
	gdb.fDebug = FALSE;
	gdb.pDBFiles = MemAlloc( uiSize );
	CHECKF( gdb.pDBFiles );

	gdb.uiNumDBFiles = INITIAL_NUM_HANDLES;

	for ( i=0 ; i<INITIAL_NUM_HANDLES ; i++ )
	{
		gdb.pDBFiles[i].hFile = 0;
	}

	strcpy( gdb.strIndexFilename, strIndexFilename );

  RegisterDebugTopic( TOPIC_DATABASE_MANAGER, "Database Manager");

	return(TRUE);
}

//**************************************************************************
//
// DbSystemShutdown
//
//		Shuts down the database system.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

void ShutdownDatabaseManager( void )
{
	UINT32 i;

	MemFree( gdb.bucket.pstrFilenames );
	MemFree( gdb.bucket.pstrDatabasenames );

	// close any unclosed files
	for ( i=1 ; i<gdb.uiNumDBFiles ; i++ )
	{
		DbClose((HDBINDEX)i);
		//if ( gdb.pDBFiles[i].hFile )
		//{
		//	FileClose(gdb.pDBFiles[i].hFile);
		//	gdb.pDBFiles[i].hFile = 0;
		//}
	}

	// release memory used
	MemFree( gdb.pDBFiles );

  UnRegisterDebugTopic( TOPIC_DATABASE_MANAGER, "Database Manager" );
}

//**************************************************************************
//
// DbDebug
//
//		To set whether or not we should print debug info.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

void DbDebug( BOOLEAN fFlag )
{
	gdb.fDebug = fFlag;
}

//**************************************************************************
//
// DbExists
//
//		Checks if a database file exists.
//
// Parameter List :
//
//		STR	-> name of file to check existence of
//
// Return Value :
//
//		BOOLEAN	-> TRUE if it exists
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN DbExists( STR strFilename )
{
	return( FileExists( strFilename ) );
}

//**************************************************************************
//
// DbOpen
//
//		Opens a database.
//
// Parameter List :
//
//		STR	-> filename
//
// Return Value :
//
//		HDBINDEX	-> handle of opened file
//					-> 0 if unsuccessful
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

HDBINDEX DbOpen( STR strFilename )
{
	HDBINDEX		hDBIndex;
	UINT16		i;
	CHAR			cShortFilename[FILENAME_LENGTH];
	CHAR			cShortDBName[FILENAME_LENGTH];
	BOOLEAN		fFound;

	fFound   = FALSE;
	hDBIndex = 0;

	// first, find any already open databases, and make sure that if we're trying to
	// open one that's already open, then we just return an index to it
	for ( i=1 ; i<gdb.uiNumDBFiles ; i++ )
	{
		if ( gdb.pDBFiles[i].hFile != 0 )
		{
			GetShortFilename( cShortFilename, strFilename );
			GetShortFilename( cShortDBName, gdb.pDBFiles[i].strFilename );
			if ( strcmp( cShortFilename, cShortDBName ) == 0 )
			{
				fFound = TRUE;
				hDBIndex = i;
				break;
			}
		}
	}

	// if we didn't find the database already open, then open it
	if ( !fFound )
	{
		for ( i=1 ; i<gdb.uiNumDBFiles ; i++ )
		{
			if ( gdb.pDBFiles[i].hFile == 0 )
			{
				if ( InitDB( &gdb.pDBFiles[i], strFilename ) )
					hDBIndex = i;
				break;
			}
		}
	}

	// if we didn't have enough space to open the file, allocate more and open it
	if ( i == gdb.uiNumDBFiles )
	{
		DbInfo *pNew;

		pNew = MemRealloc( gdb.pDBFiles, gdb.uiNumDBFiles + NUM_FILES_TO_ADD_AT_A_TIME );

		if ( !pNew )
		{
			return(0);
		}
		gdb.pDBFiles = (DbInfo *)pNew;

		if ( InitDB( &gdb.pDBFiles[i], strFilename ) )
			hDBIndex = i;
	}

	//ASSERT(hDBIndex);
	DbgMessage( TOPIC_DATABASE_MANAGER, DBG_LEVEL_2, "Opening Database File" );
	DbgMessage( TOPIC_DATABASE_MANAGER, DBG_LEVEL_2, strFilename );

	return(hDBIndex);
}

//**************************************************************************
//
// DbClose
//
//
// Parameter List :
//
//		HWFILE hFile	-> handle to database file to close
//
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

void DbClose( HDBINDEX hFile )
{
	if ( hFile && gdb.pDBFiles[hFile].hFile )
	{
		MemFree( gdb.pDBFiles[hFile].pOpenFiles );
		FileClose( gdb.pDBFiles[hFile].hFile );
		gdb.pDBFiles[hFile].hFile = 0;

		DbgMessage( TOPIC_DATABASE_MANAGER, DBG_LEVEL_2, "Closing Database File" );
	}
}

//**************************************************************************
//
// DbFileOpen
//
//		Opens a file in a database.
//
// Parameter List :
//
//		STR	   -> filename
//		UIN32		-> access - read or write, or both
//		BOOLEAN	-> delete on close
//
// Return Value :
//
//		HWFILE	-> handle of opened file
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

HDBFILE DbFileOpen( STR strFilename )
{
	HDBFILE		hFile;
	HFILEINDEX	hFileIndex;
	HDBINDEX		hDBIndex;
	UINT16		i;

	hDBIndex = OpenDatabaseContainingFile( strFilename );
	if ( !hDBIndex )
		return(0);

	for ( i=1 ; i<gdb.pDBFiles[hDBIndex].uiNumFiles ; i++ )
	{
		if ( gdb.pDBFiles[hDBIndex].pOpenFiles[i].iFileSize == 0 )
		{
			if ( InitFile( gdb.pDBFiles[hDBIndex].hFile, &gdb.pDBFiles[hDBIndex].pOpenFiles[i], strFilename ) )
				hFileIndex = i;
			break;
		}
	}
	if ( i == gdb.pDBFiles[hDBIndex].uiNumFiles )
	{
		DbFile	*pNew;

		pNew = MemRealloc( gdb.pDBFiles[hDBIndex].pOpenFiles,
								 gdb.pDBFiles[hDBIndex].uiNumFiles + NUM_FILES_TO_ADD_AT_A_TIME );

		if ( !pNew )
		{
			return(0);
		}
		gdb.pDBFiles[hDBIndex].pOpenFiles = (DbFile *)pNew;

		if ( InitFile( gdb.pDBFiles[hDBIndex].hFile, &gdb.pDBFiles[hDBIndex].pOpenFiles[i], strFilename ) )
			hFileIndex = i;
		else
			return(0);
	}

	//ASSERT(hDBIndex);
	//ASSERT(hFileIndex);

	hFile = CreateDBFileHandle( hFileIndex, hDBIndex );
	return(hFile);
}

//**************************************************************************
//
// DbFileClose
//
//
// Parameter List :
//
//		HWFILE hFile	-> handle to database file to close
//
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

void DbFileClose( HDBFILE hDBFile )
{
	HFILEINDEX	hFileIndex;
	HDBINDEX		hDBIndex;

	hFileIndex = ExtractFileIndex( hDBFile );
	hDBIndex   = ExtractDbIndex( hDBFile );

	if ( hDBIndex >= gdb.uiNumDBFiles )
		return;

	if ( gdb.pDBFiles[hDBIndex].pOpenFiles )
	{
		if ( hFileIndex >= gdb.pDBFiles[hDBIndex].uiNumFiles )
			return;

		if ( gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize )
		{
			gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize = 0;
		}
	}
}

//**************************************************************************
//
// DbFileRead
//
//		To read a file in a database.
//
// Parameter List :
//
//		HWFILE		-> handle to file to read from
//		void	*	-> source buffer
//		UINT32	-> num bytes to read
//		UINT32	-> num bytes read
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN DbFileRead(HDBFILE	hDBFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead)
{
	HFILEINDEX	hFileIndex;
	HDBINDEX		hDBIndex;
	HWFILE		hFile;
	UINT32		uiStartPos, uiCurPos, uiBytesRead;

	hFileIndex = ExtractFileIndex( hDBFile );
	hDBIndex   = ExtractDbIndex( hDBFile );

	CHECKF( hDBIndex < gdb.uiNumDBFiles );
	CHECKF( gdb.pDBFiles[hDBIndex].pOpenFiles )
	CHECKF( hFileIndex < gdb.pDBFiles[hDBIndex].uiNumFiles );
	CHECKF( gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize );

	hFile			= gdb.pDBFiles[hDBIndex].hFile;
	uiStartPos	= gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iOffsetIntoFile;
	uiCurPos		= gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iCurrentPosition;

	CHECKF( hFile );

	FileSeek( hFile, uiStartPos + uiCurPos, FILE_SEEK_FROM_START );
	FileRead( hFile, pDest, uiBytesToRead, &uiBytesRead );

	if ( puiBytesRead )
		*puiBytesRead = uiBytesRead;

	gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iCurrentPosition += uiBytesToRead;

	return(TRUE);
}

//**************************************************************************
//
// DbFileLoad
//
//		To open, read, and close a file.
//
// Parameter List :
//
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN DbFileLoad(STR filename, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead)
{
	HDBFILE	hDBFile;
	UINT32	uiBytesRead;

	hDBFile = DbFileOpen( filename );
	CHECKF( hDBFile );

	DbFileRead( hDBFile, pDest, uiBytesToRead, &uiBytesRead );
	DbFileClose( hDBFile );

	if ( puiBytesRead )
		*puiBytesRead = uiBytesRead;

	return(TRUE);
}

//**************************************************************************
//
// DbFileSeek
//
//		To seek to a position in a file in a database.
//
// Parameter List :
//
//		HWFILE	-> handle to file to seek in
//		UINT32	-> distance to seek
//		UINT8		-> how to seek
//
// Return Value :
//
//		BOOLEAN	-> TRUE if successful
//					-> FALSE if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

BOOLEAN DbFileSeek( HDBFILE hDBFile, UINT32 uiDistance, UINT8 uiHow )
{
	HFILEINDEX	hFileIndex;
	HDBINDEX		hDBIndex;
	UINT32		uiStartPos, uiCurPos, uiSize;

	hFileIndex = ExtractFileIndex( hDBFile );
	hDBIndex   = ExtractDbIndex( hDBFile );

	CHECKF( hDBIndex < gdb.uiNumDBFiles );
	CHECKF( gdb.pDBFiles[hDBIndex].pOpenFiles )
	CHECKF( hFileIndex < gdb.pDBFiles[hDBIndex].uiNumFiles );
	CHECKF( gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize );

	uiStartPos	= gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iOffsetIntoFile;
	uiCurPos		= gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iCurrentPosition;
	uiSize		= gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize;

	if ( uiHow == FILE_SEEK_FROM_START )
		uiCurPos = uiDistance;
	else if ( uiHow == FILE_SEEK_FROM_END )
		uiCurPos = uiSize - uiDistance;
	else if ( uiHow == FILE_SEEK_FROM_CURRENT )
		uiCurPos += uiDistance;
	else
		return(FALSE);

	return(TRUE);
}

//**************************************************************************
//
// DbFileGetSize
//
//		To get the current position in a file.
//
// Parameter List :
//
//		HWFILE	-> handle to file
//
// Return Value :
//
//		INT32		-> current offset in file if successful
//					-> -1 if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

UINT32 DbFileGetPos( HDBFILE hDBFile )
{
	HFILEINDEX	hFileIndex;
	HDBINDEX		hDBIndex;

	hFileIndex = ExtractFileIndex( hDBFile );
	hDBIndex   = ExtractDbIndex( hDBFile );

	CHECK0( hDBIndex < gdb.uiNumDBFiles );
	CHECK0( gdb.pDBFiles[hDBIndex].pOpenFiles )
	CHECK0( hFileIndex < gdb.pDBFiles[hDBIndex].uiNumFiles );
	CHECK0( gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize );

	return(gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iCurrentPosition);
}

//**************************************************************************
//
// DbFileGetSize
//
//		To get the current file size.
//
// Parameter List :
//
//		HWFILE	-> handle to file
//
// Return Value :
//
//		INT32		-> file size in file if successful
//					-> 0 if not
//
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

UINT32 DbFileGetSize( HDBFILE hDBFile )
{
	HFILEINDEX	hFileIndex;
	HDBINDEX		hDBIndex;

	hFileIndex = ExtractFileIndex( hDBFile );
	hDBIndex   = ExtractDbIndex( hDBFile );

	CHECK0( hDBIndex < gdb.uiNumDBFiles );
	CHECK0( gdb.pDBFiles[hDBIndex].pOpenFiles )
	CHECK0( hFileIndex < gdb.pDBFiles[hDBIndex].uiNumFiles );
	CHECK0( gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize );

	return(gdb.pDBFiles[hDBIndex].pOpenFiles[hFileIndex].iFileSize);
}

//**************************************************************************
//
// DbDebugPrint
//
//		To print the debug state to output.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		24sep96:HJH		-> creation
//
//**************************************************************************

void DbDebugPrint( void )
{
}

//**************************************************************************
//
// InitDB
//
//		To initialize a database info structure.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		15oct96:HJH		-> creation
//
//**************************************************************************

BOOLEAN InitDB( DbInfo *pDBInfo, STR strFilename )
{
	HWFILE		hFile;
	INT32			i;

	CHECKF(pDBInfo);

	hFile = FileOpen( strFilename, FILE_ACCESS_READ, FALSE );
	CHECKF( hFile );

	strcpy( pDBInfo->strFilename, strFilename );
	pDBInfo->hFile = hFile;
	pDBInfo->pOpenFiles = MemAlloc(INITIAL_NUM_HANDLES * sizeof(DbFile));

	if ( !pDBInfo->pOpenFiles )
	{
		FileClose( hFile );
		return(FALSE);
	}

	for ( i=0 ; i<INITIAL_NUM_HANDLES ; i++ )
	{
		pDBInfo->pOpenFiles[i].iFileSize = 0;
	}
	pDBInfo->uiNumFiles = INITIAL_NUM_HANDLES;

	return(TRUE);
}

//**************************************************************************
//
// OpenDatabaseContainingFile
//
//		To open, or find the already opened database, containing the given file.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		15oct96:HJH		-> creation
//
//**************************************************************************

HDBINDEX OpenDatabaseContainingFile( STR strFilename )
{
	CHAR		cFirst, cFilename[FILENAME_LENGTH], cCheckname[FILENAME_LENGTH];
	UINT32	i;
	HDBINDEX	hIndex;

	hIndex = 0;
	GetShortFilename( cFilename, strFilename );

	cFirst = cFilename[0];

	if ( cFirst != gdb.bucket.cFirstLetter )
		LoadBucket( cFirst );

	for ( i=0 ; i<gdb.bucket.uiNumNames ; i++ )
	{
		GetShortFilename( cCheckname, &(gdb.bucket.pstrFilenames[i*BUCKET_FILENAME_LENGTH]) );

		if ( strcmp( cCheckname, cFilename ) == 0 )
		{
			break;
		}
	}

	if ( i < gdb.bucket.uiNumNames )
	{
		hIndex = DbOpen( &gdb.bucket.pstrDatabasenames[i*BUCKET_FILENAME_LENGTH] );
	}

	return( hIndex );
}

//**************************************************************************
//
// InitFile
//
//		To initialize a file info structure.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		15oct96:HJH		-> creation
//
//**************************************************************************

BOOLEAN InitFile( HWFILE hDBFile, DbFile *pFileInfo, STR strFilename )
{
	DbHeader	header;
	IndexMember	im;
	UINT32	uiBytesRead;
	INT32		i;
	CHAR		cFilename[FILENAME_LENGTH], cShortFilename[FILENAME_LENGTH];

	CHECKF( pFileInfo );
	CHECKF( strFilename );
	CHECKF( hDBFile );

	FileSeek( hDBFile, 0, FILE_SEEK_FROM_START );
	FileRead( hDBFile, &header, sizeof(DbHeader), &uiBytesRead );
	CHECKF( uiBytesRead == sizeof(DbHeader) );
	FileSeek( hDBFile, header.iOffsetToIndex, FILE_SEEK_FROM_START );

	GetShortFilename( cShortFilename, strFilename );
	for ( i=0 ; i<header.iNumFiles ; i++ )
	{
		FileRead( hDBFile, &im, sizeof(IndexMember), &uiBytesRead );
		CHECKF( uiBytesRead == sizeof(IndexMember) );
		GetShortFilename( cFilename, im.strFilename );
		if ( strcmp( cFilename, cShortFilename ) == 0 )
		{
			strcpy( pFileInfo->strFilename, strFilename );
			pFileInfo->iFileSize = im.iFileSize;
			pFileInfo->iOffsetIntoFile = im.iOffsetIntoFile;
			pFileInfo->iCurrentPosition = 0;
			return(TRUE);
		}
	}

	return(FALSE);
}

//**************************************************************************
//
// LoadBucket
//
//		To load a bucketfull of file-database name pairs.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		15oct96:HJH		-> creation
//
//**************************************************************************

BOOLEAN LoadBucket( CHAR cFirstLetter )
{
	HWFILE	hFile;
	UINT32	i, uiPosition, uiBytesRead, uiNumEntries, uiSize;
	CHAR		cFilename[FILENAME_LENGTH], cDatabasename[FILENAME_LENGTH];

	uiNumEntries = 0;

	MemFree( gdb.bucket.pstrFilenames );
	MemFree( gdb.bucket.pstrDatabasenames );

	hFile = FileOpen( gdb.strIndexFilename, FILE_ACCESS_READ, FALSE );
	CHECKF(hFile);
	uiSize = FileGetSize( hFile );

	if ( cFirstLetter >= 'a' && cFirstLetter <= 'z' )
		uiPosition = 'z' - cFirstLetter + 1;
	else
		uiPosition = 26 + ('9' - cFirstLetter + 1);

	FileSeek( hFile, sizeof(UINT32)*uiPosition, FILE_SEEK_FROM_END );
	FileRead( hFile, &uiPosition, sizeof(UINT32), &uiBytesRead );
	if ( uiPosition > uiSize )
	{
		// error
		return(FALSE);
	}

	FileSeek( hFile, uiPosition, FILE_SEEK_FROM_START );
	FileRead( hFile, &uiNumEntries, sizeof(UINT32), &uiBytesRead );

	CHECKF(uiNumEntries);

	gdb.bucket.pstrFilenames = (CHAR *)MemAlloc( uiNumEntries * FILENAME_LENGTH);
	CHECKF(gdb.bucket.pstrFilenames);

	gdb.bucket.pstrDatabasenames = (CHAR *)MemAlloc( uiNumEntries * FILENAME_LENGTH);
	if ( !gdb.bucket.pstrDatabasenames )
	{
		MemFree(gdb.bucket.pstrFilenames);
		return(FALSE);
	}


	for ( i=0 ; i<uiNumEntries ; i++ )
	{
		FileRead( hFile, cFilename, BUCKET_FILENAME_LENGTH, &uiBytesRead );
		FileRead( hFile, cDatabasename, BUCKET_FILENAME_LENGTH, &uiBytesRead );

		strcpy( &gdb.bucket.pstrFilenames[i*BUCKET_FILENAME_LENGTH], cFilename );
		strcpy( &gdb.bucket.pstrDatabasenames[i*BUCKET_FILENAME_LENGTH], cDatabasename );
	}

	FileClose( hFile );

	gdb.bucket.uiNumNames = uiNumEntries;
	gdb.bucket.cFirstLetter = cFirstLetter;

	DbgMessage( TOPIC_DATABASE_MANAGER, DBG_LEVEL_2, "Loading Different Bucket" );

	return(TRUE);
}

//**************************************************************************
//
// GetShortFilename
//
//		To chop up a path, and return just the short filename.
//
// Parameter List :
// Return Value :
// Modification history :
//
//		15oct96:HJH		-> creation
//
//**************************************************************************

BOOLEAN GetShortFilename( STR strFilename, STR strPathname )
{
	CHAR *pcLast;

	pcLast = strrchr( strPathname, '\\' );

	if ( pcLast )
	{
		pcLast = pcLast + 1;
		strcpy( strFilename, pcLast );
	}
	else
		strcpy( strFilename, strPathname );

	return(TRUE);
}

