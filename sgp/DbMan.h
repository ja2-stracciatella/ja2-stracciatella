//**************************************************************************
//
// Filename :	DbMan.h
//
//	Purpose :	prototypes for the database manager
//
// Modification history :
//
//		08oct96:HJH				- Creation
//
//**************************************************************************

#ifndef _DBMAN_H
#define _DBMAN_H

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "Types.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

#ifndef FILE_ACCESS_READ
#define FILE_ACCESS_READ	      0x01
#endif

#ifndef FILE_ACCESS_WRITE
#define FILE_ACCESS_WRITE	      0x02
#endif

#define FILE_SEEK_FROM_START		0x01	// keep in sync with fileman.h
#define FILE_SEEK_FROM_END			0x02	// keep in sync with fileman.h
#define FILE_SEEK_FROM_CURRENT	0x04	// keep in sync with fileman.h

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

typedef UINT8		BYTE;
typedef UINT32		HDBFILE;
typedef UINT16		HFILEINDEX;
typedef UINT16		HDBINDEX;

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN	InitializeDatabaseManager( STR strIndexFilename );
extern void		ShutdownDatabaseManager( void );
extern void		DbDebug( BOOLEAN f );

extern BOOLEAN	DbExists( STR filename );

extern HDBINDEX	DbOpen( STR filename );
extern void		DbClose( HDBINDEX );

extern HDBFILE	DbFileOpen( STR filename );
extern void		DbFileClose( HDBFILE );

extern BOOLEAN	DbFileRead( HDBFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead );
extern BOOLEAN	DbFileLoad( STR filename, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead );
extern BOOLEAN	DbFileSeek( HDBFILE hFile, UINT32 uiDistance, UINT8 uiHow );

extern UINT32	DbFileGetPos( HDBFILE hFile );
extern UINT32	DbFileGetSize( HDBFILE );

#ifdef __cplusplus
}
#endif


#endif
