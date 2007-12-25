#include "Types.h"
#include "Lighting.h"
#include "Shade_Table_Util.h"
#include "VObject.h"
#include "Video.h"
#include "WorldDat.h"
#include "FileMan.h"
#include "MemMan.h"
#include "Debug.h"


#define		SHADE_TABLE_DIR				"ShadeTables"



CHAR8 TileSurfaceFilenames[NUMBEROFTILETYPES][32];
BOOLEAN gfForceBuildShadeTables = FALSE;

#ifdef JA2TESTVERSION
	extern UINT32 uiNumTablesSaved;
	extern UINT32 uiNumTablesLoaded;
#endif

void DetermineRGBDistributionSettings()
{
	STRING512			DataDir;
	STRING512			ShadeTableDir;
	UINT32				uiRBitMask, uiGBitMask, uiBBitMask;
	UINT32				uiPrevRBitMask, uiPrevGBitMask, uiPrevBBitMask;
	HWFILE				hfile;
	BOOLEAN				fSaveRGBDist = FALSE;
	BOOLEAN				fCleanShadeTable = FALSE;
	BOOLEAN				fLoadedPrevRGBDist = FALSE;

	//First, determine if we have a file saved.  If not, then this is the first time, and
	//all shade tables will have to be built and saved to disk.  This can be time consuming, adding up to
	//3-4 seconds to the time of a map load.
	const char* ExecDir = GetExecutableDirectory();
	sprintf( ShadeTableDir, "%s/Data/%s", ExecDir, SHADE_TABLE_DIR );

	//Check to make sure we have a ShadeTable directory.  If we don't create one!
	if( !SetFileManCurrentDirectory( ShadeTableDir ) )
	{
		if( !MakeFileManDirectory( ShadeTableDir ) )
		{
			AssertMsg( 0, "ShadeTable directory doesn't exist, and couldn't create one!" );
		}
		if( !SetFileManCurrentDirectory( ShadeTableDir ) )
		{
			AssertMsg( 0, "Couldn't access the newly created ShadeTable directory." );
		}
		fSaveRGBDist = TRUE;
	}

	if( !fSaveRGBDist )
	{ //Load the previous RGBDist and determine if it is the same one
		if( !FileExists( "RGBDist.dat" ) || FileExists( "ResetShadeTables.txt" ) )
		{ //Can't find the RGBDist.dat file.  The directory exists, but the file doesn't, which
			//means the user deleted the file manually.  Now, set it up to create a new one.
			fSaveRGBDist = TRUE;
			fCleanShadeTable = TRUE;
		}
		else
		{
			hfile = FileOpen("RGBDist.dat", FILE_ACCESS_READ);
			if( !hfile )
			{
				AssertMsg( 0, "Couldn't open RGBDist.dat, even though it exists!" );
			}
			FileRead(hfile, &uiPrevRBitMask, sizeof(UINT32));
			FileRead(hfile, &uiPrevGBitMask, sizeof(UINT32));
			FileRead(hfile, &uiPrevBBitMask, sizeof(UINT32));
			fLoadedPrevRGBDist = TRUE;
			FileClose( hfile );
		}
	}

	if( !GetPrimaryRGBDistributionMasks( &uiRBitMask, &uiGBitMask, &uiBBitMask ) )
	{
		AssertMsg( 0, "Failed to extract the current RGB distribution masks." );
	}
	if( fLoadedPrevRGBDist )
	{
		if( uiRBitMask != uiPrevRBitMask || uiGBitMask != uiPrevGBitMask || uiBBitMask != uiPrevBBitMask )
		{ //The user has changed modes since the last time he has played JA2.  This essentially can only happen if:
			//1)  The video card has been changed since the last run of JA2.
			//2)  Certain video cards have different RGB distributions in different operating systems such as
			//		the Millenium card using Windows NT or Windows 95
			//3)  The user has physically modified the RGBDist.dat file.
			fSaveRGBDist = TRUE;
			fCleanShadeTable = TRUE;
		}
	}
	if( fCleanShadeTable )
	{ //This means that we are going to remove all of the current shade tables, if any exist, and
		//start fresh.
		EraseDirectory( ShadeTableDir );
	}
	if( fSaveRGBDist )
	{ //The RGB distribution is going to be saved in a tiny file for future reference.  As long as the
		//RGB distribution never changes, the shade table will grow until eventually, all tilesets are loaded,
		//shadetables generated and saved in this directory.
		hfile = FileOpen("RGBDist.dat", FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
		if( !hfile )
		{
			AssertMsg( 0, "Couldn't create RGBDist.dat for writing!" );
		}
		FileWrite(hfile, &uiRBitMask, sizeof(UINT32));
		FileWrite(hfile, &uiGBitMask, sizeof(UINT32));
		FileWrite(hfile, &uiBBitMask, sizeof(UINT32));
		FileClose( hfile );
	}

	//We're done, so restore the executable directory to JA2\Data.
	sprintf( DataDir, "%s/Data", ExecDir );
	SetFileManCurrentDirectory( DataDir );
}

BOOLEAN LoadShadeTable( HVOBJECT pObj, UINT32 uiTileTypeIndex )
{
	HWFILE hfile;
	INT32 i;
	//ASSUMPTIONS:
	//We are assuming that the uiTileTypeIndex is referring to the correct file
	//stored in the TileSurfaceFilenames[].  If it isn't, then that is a huge problem
	//and should be fixed.  Also assumes that the directory is set to Data\ShadeTables.
	char ShadeFileName[100];
	strcpy( ShadeFileName, TileSurfaceFilenames[ uiTileTypeIndex ] );
	char* ptr = strstr(ShadeFileName, ".");
	if( !ptr )
	{
		return FALSE;
	}
	ptr++;
	strcpy(ptr, "sha");

	hfile = FileOpen(ShadeFileName, FILE_ACCESS_READ);
	if( !hfile )
	{ //File doesn't exist, so generate it
		return FALSE;
	}

	//MISSING:  Compare time stamps.

	for( i = 0; i < 16; i++ )
	{
		pObj->pShades[ i ] = MemAlloc( 512 );
		Assert( pObj->pShades[ i ] );
		FileRead(hfile, pObj->pShades[i], 512);
	}

	//The file exists, now make sure the
	FileClose( hfile );
	#ifdef JA2TESTVERSION
		uiNumTablesLoaded++;
	#endif
	return TRUE;
}

BOOLEAN SaveShadeTable( HVOBJECT pObj, UINT32 uiTileTypeIndex )
{
	HWFILE hfile;
	INT32 i;
	#ifdef JA2TESTVERSION
		uiNumTablesSaved++;
	#endif
	//ASSUMPTIONS:
	//We are assuming that the uiTileTypeIndex is referring to the correct file
	//stored in the TileSurfaceFilenames[].  If it isn't, then that is a huge problem
	//and should be fixed.  Also assumes that the directory is set to Data\ShadeTables.
	char ShadeFileName[100];
	strcpy( ShadeFileName, TileSurfaceFilenames[ uiTileTypeIndex ] );
	char* ptr = strstr(ShadeFileName, ".");
	if( !ptr )
	{
		return FALSE;
	}
	ptr++;
	strcpy(ptr, "sha");

	hfile = FileOpen(ShadeFileName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
	if( !hfile )
	{
		AssertMsg( 0, String( "Can't create %s", ShadeFileName ) );
		return FALSE;
	}
	for( i = 0; i < 16; i++ )
	{
		FileWrite(hfile, pObj->pShades[i], 512);
	}

	FileClose( hfile );
	return TRUE;
}



BOOLEAN DeleteShadeTableDir( )
{
	return( RemoveFileManDirectory( SHADE_TABLE_DIR, TRUE ) );
}
