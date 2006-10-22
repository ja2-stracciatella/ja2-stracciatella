#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
	#include "Enemy_Soldier_Save.h"
#else
	#include "Types.h"
	#include "MemMan.h"
	#include "Message.h"
	#include "Item_Types.h"
	#include "Items.h"
	#include "Handle_Items.h"
	#include "StrategicMap.h"
	#include "Tactical_Save.h"
	#include <stdio.h>
	#include "Campaign_Types.h"
	#include "SaveLoadGame.h"
	#include "WorldDef.h"
	#include "Rotting_Corpses.h"
	#include "Overhead.h"
	#include "Keys.h"
	#include "Soldier_Create.h"
	#include "Soldier_Profile.h"
	#include "Isometric_Utils.h"
	#include "Soldier_Add.h"
	#include "NPC.h"
	#include "AI.h"
	#include "Game_Clock.h"
	#include "Animation_Control.h"
	#include "Map_Information.h"
	//#include "PathAI.h"
	#include "SaveLoadMap.h"
	#include "Debug.h"
	#include "Random.h"
	#include "Quests.h"
	#include "Animated_ProgressBar.h"
	#include "Text.h"
	#include "Meanwhile.h"
	#include "Enemy_Soldier_Save.h"
	#include "SmokeEffects.h"
	#include "LightEffects.h"
	#include "PathAI.h"
	#include "GameVersion.h"
	#include "Strategic.h"
	#include "Map_Screen_Interface_Map.h"
	#include "Strategic_Status.h"
	#include "Soldier_Macros.h"
	#include "SGP.h"
	#include "MessageBoxScreen.h"
	#include "Queen_Command.h"
	#include "Map_Screen_Interface_Map_Inventory.h"
	#include "ScreenIDs.h"
	#include "FileMan.h"
#endif

static BOOLEAN gfWasInMeanwhile = FALSE;


// This struct is used to save info from the NPCQuoteInfo struct that can change.
typedef struct
{
	UINT16	usFlags;

	union
	{
		INT16		sRequiredItem;			// item NPC must have to say quote
		INT16		sRequiredGridno;		// location for NPC req'd to say quote
	};

	UINT16	usGoToGridno;

} TempNPCQuoteInfoSave;



#define		NPC_TEMP_QUOTE_FILE			"Temp\\NpcQuote.tmp"


extern	UINT32			guiNumWorldItems;

extern	NPCQuoteInfo *	gpNPCQuoteInfoArray[NUM_PROFILES];

extern UINT32	guiJA2EncryptionSet;
extern UINT8 gubEncryptionArray1[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ];
extern UINT8 gubEncryptionArray2[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ];
extern UINT8 gubEncryptionArray3[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ];
extern UINT8 gubEncryptionArray4[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ];

static INT32 giErrorMessageBox = 0;


extern void InitLoadedWorld( );
extern void ReduceAmmoDroppedByNonPlayerSoldiers( SOLDIERTYPE *pSoldier, INT32 iInvSlot );

extern void StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated();

#ifdef JA2BETAVERSION
extern BOOLEAN ValidateSoldierInitLinks( UINT8 ubCode );
#endif


static BOOLEAN AddTempFileToSavedGame(HWFILE hFile, UINT32 uiType, INT16 sMapX, INT16 sMapY, INT8 bMapZ);


// SaveMapTempFilesToSavedGameFile() Looks for and opens all Map Modification files.  It add each mod file to the save game file.
BOOLEAN SaveMapTempFilesToSavedGameFile( HWFILE hFile )
{
	UNDERGROUND_SECTORINFO *TempNode = gpUndergroundSectorInfoHead;
	INT16 sMapX;
	INT16 sMapY;


	//Save the current sectors open temp files to the disk
//	SaveCurrentSectorsItemsToTempItemFile();

	//
	//Loop though all the array elements to see if there is a data file to be saved
	//

	//First look through the above ground sectors
	for( sMapY=1; sMapY<=16; sMapY++ )
	{
		for( sMapX=1; sMapX<=16; sMapX++ )
		{
			//Save the Temp Item Files to the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ITEM_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_ITEM_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			// Save the Rotting Corpse Temp file to the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ROTTING_CORPSE_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}


			// Save the Map Modifications Temp file to the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			//Save the Door Table temp file to the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_DOOR_TABLE_TEMP_FILES_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_DOOR_TABLE_TEMP_FILES_EXISTS, sMapX, sMapY, 0 );
			}

			//Save the revealed status temp file to the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_REVEALED_STATUS_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_REVEALED_STATUS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			//Save the door status temp file to the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_DOOR_STATUS_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_DOOR_STATUS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_CIV_PRESERVED_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_CIV_PRESERVED_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS )
			{
				AddTempFileToSavedGame( hFile, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}


			//if any other file is to be saved
		}
	}


	//then look throught all the underground sectors
	while( TempNode )
	{
		if( TempNode->uiFlags & SF_ITEM_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_ITEM_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_ROTTING_CORPSE_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		// Save the Map Modifications Temp file to the saved game file
		if( TempNode->uiFlags & SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}


		//Save the door table to the saved game file
		if( TempNode->uiFlags & SF_DOOR_TABLE_TEMP_FILES_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_DOOR_TABLE_TEMP_FILES_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		//Save the revealed status temp file to the saved game file
		if( TempNode->uiFlags & SF_REVEALED_STATUS_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_REVEALED_STATUS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		//Save the door status temp file to the saved game file
		if( TempNode->uiFlags & SF_DOOR_STATUS_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_DOOR_STATUS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_CIV_PRESERVED_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_CIV_PRESERVED_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS )
		{
			AddTempFileToSavedGame( hFile, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		//ttt


		TempNode = TempNode->next;
	}


	return( TRUE );
}


static BOOLEAN RetrieveTempFileFromSavedGame(HWFILE hFile, UINT32 uiType, INT16 sMapX, INT16 sMapY, INT8 bMapZ);
static void SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems(INT16 sMapX, INT16 sMapY, INT8 bMapZ, BOOLEAN fLoadingGame);


// LoadMapTempFilesFromSavedGameFile() loads all the temp files from the saved game file and writes them into the temp directory
BOOLEAN	LoadMapTempFilesFromSavedGameFile( HWFILE hFile )
{
	UNDERGROUND_SECTORINFO *TempNode = gpUndergroundSectorInfoHead;
	INT16 sMapX;
	INT16 sMapY;
	UINT32	uiPercentage;
	UINT32		iCounter = 0;


	// HACK FOR GABBY
	if ( (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && guiSaveGameVersion < 81 )
	{
		if ( gMercProfiles[ GABBY ].bMercStatus != MERC_IS_DEAD )
		{
			// turn off alternate flags for the sectors he could be in
			// randomly place him in one of the two possible sectors
			SectorInfo[ SECTOR( 14, MAP_ROW_L ) ].uiFlags &= ~SF_USE_ALTERNATE_MAP;
			SectorInfo[ SECTOR(  8, MAP_ROW_L ) ].uiFlags &= ~SF_USE_ALTERNATE_MAP;

			if ( Random( 2 ) )
			{
				SectorInfo[ SECTOR( 11, MAP_ROW_H ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
				gMercProfiles[ GABBY ].sSectorX = 11;
				gMercProfiles[ GABBY ].sSectorY = MAP_ROW_H;
			}
			else
			{
				SectorInfo[ SECTOR(  4, MAP_ROW_I ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
				gMercProfiles[ GABBY ].sSectorX = 4;
				gMercProfiles[ GABBY ].sSectorY = MAP_ROW_I;
			}
		}
	}

	//
	//Loop though all the array elements to see if there is a data file to be loaded
	//

	//First look through the above ground sectors
	for( sMapY=1; sMapY<=16; sMapY++ )
	{
		for( sMapX=1; sMapX<=16; sMapX++ )
		{
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ITEM_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_ITEM_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );

				//sync up the temp file data to the sector structure data
				SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems( sMapX, sMapY, 0, TRUE );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ROTTING_CORPSE_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_DOOR_TABLE_TEMP_FILES_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_DOOR_TABLE_TEMP_FILES_EXISTS, sMapX, sMapY, 0 );
			}

			//Get the revealed status temp file From the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_REVEALED_STATUS_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_REVEALED_STATUS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			//Get the revealed status temp file From the saved game file
			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_DOOR_STATUS_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_DOOR_STATUS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_CIV_PRESERVED_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_CIV_PRESERVED_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
				if ( (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && guiSaveGameVersion < 78 )
				{
					INT8 pMapName[ 128 ];

					// KILL IT!!! KILL KIT!!!! IT IS CORRUPTED!!!
					GetMapTempFileName( SF_CIV_PRESERVED_TEMP_FILE_EXISTS, pMapName, sMapX, sMapY, 0 );
					FileDelete( pMapName );

					// turn off the flag
					SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags &= (~SF_CIV_PRESERVED_TEMP_FILE_EXISTS);

				}
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}

			if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS )
			{
				RetrieveTempFileFromSavedGame( hFile, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, sMapX, sMapY, 0 );
			}


			//if any other file is to be saved

			iCounter++;

			//update the progress bar
			uiPercentage = (iCounter * 100) / (255);

			RenderProgressBar( 0, uiPercentage );
		}
	}


	//then look throught all the underground sectors
	while( TempNode )
	{
		if( TempNode->uiFlags & SF_ITEM_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_ITEM_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );

			//sync up the temp file data to the sector structure data
			SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems( TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ, TRUE );
		}

		if( TempNode->uiFlags & SF_ROTTING_CORPSE_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}


		if( TempNode->uiFlags & SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_DOOR_TABLE_TEMP_FILES_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_DOOR_TABLE_TEMP_FILES_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		//Get the revealed status temp file From the saved game file
		if( TempNode->uiFlags & SF_REVEALED_STATUS_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_REVEALED_STATUS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		//Get the revealed status temp file From the saved game file
		if( TempNode->uiFlags & SF_DOOR_STATUS_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_DOOR_STATUS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_CIV_PRESERVED_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_CIV_PRESERVED_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
			if ( (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && guiSaveGameVersion < 78 )
			{
				INT8 pMapName[ 128 ];

				// KILL IT!!! KILL KIT!!!! IT IS CORRUPTED!!!
				GetMapTempFileName( SF_CIV_PRESERVED_TEMP_FILE_EXISTS, pMapName, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
				FileDelete( pMapName );

				// turn off the flag
				TempNode->uiFlags &= (~SF_CIV_PRESERVED_TEMP_FILE_EXISTS);

			}

		}

		if( TempNode->uiFlags & SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		if( TempNode->uiFlags & SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS )
		{
			RetrieveTempFileFromSavedGame( hFile, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, TempNode->ubSectorX, TempNode->ubSectorY, TempNode->ubSectorZ );
		}

		TempNode = TempNode->next;
	}
//ttt
	return( TRUE );
}


BOOLEAN SaveWorldItemsToTempItemFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems, WORLDITEM* pData)
{
	HWFILE	hFile;
	UINT32	uiNumBytesWritten=0;
	CHAR8		zMapName[ 128 ];

	GetMapTempFileName( SF_ITEM_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );

	//Open the file for writing, Create it if it doesnt exist
	hFile = FileOpen( zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
	if( hFile == 0 )
	{
		//Error opening map modification file
		return( FALSE );
	}

	//Save the size of the ITem table
	FileWrite( hFile, &uiNumberOfItems, sizeof( UINT32 ), &uiNumBytesWritten );
	if( uiNumBytesWritten != sizeof( UINT32 ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}


	//if there are items to save..
	if( uiNumberOfItems != 0 )
	{
		//Save the ITem array
		FileWrite( hFile, pData, uiNumberOfItems * sizeof( WORLDITEM ), &uiNumBytesWritten );
		if( uiNumBytesWritten != uiNumberOfItems * sizeof( WORLDITEM ) )
		{
			//Error Writing size of array to disk
			FileClose( hFile );
			return( FALSE );
		}
	}

	FileClose( hFile );

	SetSectorFlag( sMapX, sMapY, bMapZ, SF_ITEM_TEMP_FILE_EXISTS );

	SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems( sMapX, sMapY, bMapZ, FALSE );

	return( TRUE );
}



BOOLEAN LoadWorldItemsFromTempItemFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ, WORLDITEM *pData )
{
	UINT32	uiNumBytesRead=0;
	HWFILE	hFile;
	CHAR8		zMapName[ 128 ];
	UINT32 uiNumberOfItems=0;

	GetMapTempFileName( SF_ITEM_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );


	//Check to see if the file exists
	if( !FileExists( zMapName ) )
	{
		//If the file doesnt exists, its no problem.
		return( TRUE );
	}

	//Open the file for reading
	hFile = FileOpen( zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE );
	if( hFile == 0 )
	{
		//Error opening map modification file,
		return( FALSE );
	}


	// Load the size of the World ITem table
	FileRead( hFile, &uiNumberOfItems, sizeof( UINT32 ), &uiNumBytesRead );
	if( uiNumBytesRead != sizeof( UINT32 ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}

	// Load the World ITem table
	FileRead( hFile, pData, uiNumberOfItems * sizeof( WORLDITEM ), &uiNumBytesRead );
	if( uiNumBytesRead != uiNumberOfItems * sizeof( WORLDITEM ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}

	FileClose( hFile );

	return( TRUE );
}


BOOLEAN GetNumberOfWorldItemsFromTempItemFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 *pSizeOfData, BOOLEAN fIfEmptyCreate )
{
	UINT32	uiNumBytesRead=0;
	HWFILE	hFile;
	CHAR8		zMapName[ 128 ];
	UINT32	uiNumberOfItems=0;

	GetMapTempFileName( SF_ITEM_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );


	//Check if the file DOESNT exists
	if( !FileExistsNoDB( zMapName ) )
	{
		if( fIfEmptyCreate )
		{
			WORLDITEM TempWorldItems[ 10 ];
			UINT32		uiNumberOfItems = 10;
			UINT32		uiNumBytesWritten=0;

			//If the file doesnt exists, create a file that has an initial amount of Items
			hFile = FileOpen( zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
			if( hFile == 0 )
			{
				//Error opening item modification file
				return( FALSE );
			}

			memset( TempWorldItems, 0, ( sizeof( WORLDITEM ) * 10 ) );

			//write the the number of item in the maps item file
			FileWrite( hFile, &uiNumberOfItems, sizeof( UINT32 ), &uiNumBytesWritten );
			if( uiNumBytesWritten != sizeof( UINT32 ) )
			{
				//Error Writing size of array to disk
				FileClose( hFile );
				return( FALSE );
			}

			//write the the number of item in the maps item file
			FileWrite( hFile, TempWorldItems, uiNumberOfItems * sizeof( WORLDITEM ), &uiNumBytesWritten );
			if( uiNumBytesWritten != uiNumberOfItems * sizeof( WORLDITEM ) )
			{
				//Error Writing size of array to disk
				FileClose( hFile );
				return( FALSE );
			}

			//Close the file
			FileClose( hFile );
		}
		else
		{
			// the file doesnt exist
			*pSizeOfData = 0;

			return( TRUE );
		}
	}


	//Open the file for reading, if it exists
	hFile = FileOpen( zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE );
	if( hFile == 0 )
	{
		//Error opening map modification file
		return( FALSE );
	}


	// Load the size of the World ITem table
	FileRead( hFile, &uiNumberOfItems, sizeof( UINT32 ), &uiNumBytesRead );
	if( uiNumBytesRead != sizeof( UINT32 ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}

	*pSizeOfData = uiNumberOfItems;

	FileClose( hFile );

	return( TRUE );
}


static BOOLEAN DeleteTempItemMapFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);


BOOLEAN AddItemsToUnLoadedSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ, INT16 sGridNo, UINT32 uiNumberOfItemsToAdd, OBJECTTYPE *pObject, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT8 bVisible, BOOLEAN fReplaceEntireFile )
{
	UINT32	uiNumberOfItems=0;
	WORLDITEM *pWorldItems;
	UINT32	cnt;
	UINT32	uiLoop1=0;


	if( !GetNumberOfWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, &uiNumberOfItems, TRUE ) )
	{
		//Errror getting the numbers of the items from the sector
		return( FALSE );
	}

	//Allocate memeory for the item
	pWorldItems = MemAlloc( sizeof( WORLDITEM ) * uiNumberOfItems );
	if( pWorldItems == NULL )
	{
		//Error Allocating memory for the temp item array
		return( FALSE );
	}

	//Clear the memory
	memset( pWorldItems, 0, sizeof( WORLDITEM ) * uiNumberOfItems );

	//Load in the sectors Item Info
	if( !LoadWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, pWorldItems ) )
	{
		//error reading in the items from the Item mod file
		MemFree( pWorldItems );
		return( FALSE );
	}


	//if we are to replace the entire file
	if( fReplaceEntireFile )
	{
		//first loop through and mark all entries that they dont exists
		for( cnt=0; cnt<uiNumberOfItems; cnt++)
			pWorldItems[ cnt ].fExists = FALSE;

		//Now delete the item temp file
		DeleteTempItemMapFile( sMapX, sMapY, bMapZ );
	}

	//loop through all the objects to add
	for( uiLoop1=0; uiLoop1 < uiNumberOfItemsToAdd; uiLoop1++)
	{
		//Loop through the array to see if there is a free spot to add an item to it
		for( cnt=0; cnt < uiNumberOfItems; cnt++)
		{
			if( pWorldItems[ cnt ].fExists == FALSE )
			{
				//We have found a free spot, break
				break;
			}
		}

		if( cnt == ( uiNumberOfItems ) )
		{
			//Error, there wasnt a free spot.  Reallocate memory for the array
			pWorldItems = MemRealloc( pWorldItems, sizeof( WORLDITEM ) * (uiNumberOfItems + 1 ) );
			if( pWorldItems == NULL )
			{
				//error realloctin memory
				return( FALSE );
			}

			//Increment the total number of item in the array
			uiNumberOfItems++;

			//set the spot were the item is to be added
			cnt = uiNumberOfItems - 1;
		}

		pWorldItems[ cnt ].fExists = TRUE;
		pWorldItems[ cnt ].sGridNo = sGridNo;
		pWorldItems[ cnt ].ubLevel = ubLevel;
		pWorldItems[ cnt ].usFlags = usFlags;
		pWorldItems[ cnt ].bVisible = bVisible;
		pWorldItems[ cnt ].bRenderZHeightAboveLevel = bRenderZHeightAboveLevel;


		//Check
		if( sGridNo == NOWHERE && !( pWorldItems[ cnt ].usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT ) )
		{
			pWorldItems[ cnt ].usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;

			// Display warning.....
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Error: Trying to add item ( %d: %s ) to invalid gridno in unloaded sector. Please Report.", pWorldItems[ cnt ].o.usItem, ItemNames[pWorldItems[ cnt ].o.usItem] );
		}


		memcpy( &(pWorldItems[ cnt ].o), &pObject[uiLoop1], sizeof( OBJECTTYPE ) );
	}

	//Save the Items to the the file
	SaveWorldItemsToTempItemFile( sMapX, sMapY, bMapZ, uiNumberOfItems, pWorldItems );


	//Free the memory used to load in the item array
	MemFree( pWorldItems );

	return( TRUE );
}

extern BOOLEAN gfInMeanwhile;


static void SaveNPCInformationToProfileStruct(void);
static void SetLastTimePlayerWasInSector(void);
static BOOLEAN SaveRottingCorpsesToTempCorpseFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);


BOOLEAN SaveCurrentSectorsInformationToTempItemFile( )
{
	BOOLEAN fShouldBeInMeanwhile = FALSE;
	if( gfWasInMeanwhile )
	{ //Don't save a temp file for the meanwhile scene map.
		gfWasInMeanwhile = FALSE;
		return TRUE;
	}
	else if( AreInMeanwhile() )
	{
		gfInMeanwhile = FALSE;
		fShouldBeInMeanwhile = TRUE;
	}

	//If we havent been to tactical yet
	if( ( gWorldSectorX == 0 ) && ( gWorldSectorY == 0 ) )
	{
		return( TRUE );
	}


	//Save the Blood, smell and the revealed status for map elements
	SaveBloodSmellAndRevealedStatesFromMapToTempFile();

	// handle all reachable before save
	HandleAllReachAbleItemsInTheSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );



	//Save the Items to the the file
	if( !SaveWorldItemsToTempItemFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, guiNumWorldItems, gWorldItems ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveWorldItemsToTempItemFile()" ) );
		return( FALSE );
	}

	//Save the rotting corpse array to the temp rotting corpse file
	if( !SaveRottingCorpsesToTempCorpseFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveRottingCorpsesToTempCorpseFile()" ) );
		return( FALSE );
	}

	//save the Doortable array to the temp door map file
	if( !SaveDoorTableToDoorTableTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveDoorTableToDoorTableTempFile()" ) );
		return( FALSE );
	}

	//save the 'revealed'status of the tiles
	if( !SaveRevealedStatusArrayToRevealedTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveRevealedStatusArrayToRevealedTempFile()" ) );
		return( FALSE );
	}

	//save the door open status to the saved game file
	if( !SaveDoorStatusArrayToDoorStatusTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveDoorStatusArrayToDoorStatusTempFile()" ) );
		return( FALSE );
	}

	//Save the enemies to the temp file
	if( !NewWayOfSavingEnemyAndCivliansToTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, TRUE, FALSE ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in NewWayOfSavingEnemyAndCivliansToTempFile( Enemy, Creature Team )" ) );
		return( FALSE );
	}

	//Save the civilian info to the temp file
	if( !NewWayOfSavingEnemyAndCivliansToTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, FALSE, FALSE ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in NewWayOfSavingEnemyAndCivliansToTempFile( Civ Team )" ) );
		return( FALSE );
	}

	//Save the smoke effects info to the temp file
	if( !SaveSmokeEffectsToMapTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveSmokeEffectsToMapTempFile" ) );
		return( FALSE );
	}

	//Save the smoke effects info to the temp file
	if( !SaveLightEffectsToMapTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("SaveCurrentSectorsInformationToTempItemFile:  failed in SaveLightEffectsToMapTempFile" ) );
		return( FALSE );
	}


//ttt

	//Save any other info here


	//Save certain information from the NPC's soldier structure to the Merc structure
	SaveNPCInformationToProfileStruct( );

	//Save the time the player was last in the sector
	SetLastTimePlayerWasInSector();


	if( fShouldBeInMeanwhile )
	{
		gfInMeanwhile = TRUE;
	}

	return( TRUE );
}


void HandleAllReachAbleItemsInTheSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	// find out which items in the list are reachable
	UINT32 uiCounter = 0;
	UINT8	ubDir, ubMovementCost;
	BOOLEAN fReachable = FALSE;
	INT16 sGridNo = NOWHERE, sGridNo2 = NOWHERE;
	INT16	sNewLoc;

	SOLDIERTYPE * pSoldier;
	BOOLEAN	fSecondary = FALSE;

	if ( guiNumWorldItems == 0 )
	{
		return;
	}

	if ( gMapInformation.sCenterGridNo != -1 )
	{
		sGridNo = gMapInformation.sCenterGridNo;
	}
	else if ( gMapInformation.sNorthGridNo != -1 )
	{
		// use any!
		sGridNo = gMapInformation.sNorthGridNo;
	}
	else if ( gMapInformation.sEastGridNo != -1 )
	{
		// use any!
		sGridNo = gMapInformation.sEastGridNo;
	}
	else if ( gMapInformation.sSouthGridNo != -1 )
	{
		// use any!
		sGridNo = gMapInformation.sSouthGridNo;
	}
	else if ( gMapInformation.sWestGridNo != -1 )
	{
		// use any!
		sGridNo = gMapInformation.sWestGridNo;
	}
	else
	{
		AssertMsg( 0, String( "Map %c%d_b%d does not have any entry points!", 'A' + gWorldSectorY - 1, gWorldSectorX, gbWorldSectorZ ) );
		return;
	}

	if ( gMapInformation.sIsolatedGridNo != -1 )
	{
		sGridNo2 = gMapInformation.sIsolatedGridNo;

		for( uiCounter = gTacticalStatus.Team[ gbPlayerNum ].bFirstID; uiCounter < gTacticalStatus.Team[ gbPlayerNum ].bLastID; uiCounter++ )
		{
			pSoldier = MercPtrs[ uiCounter ];
			if ( pSoldier && pSoldier->bActive && pSoldier->bLife > 0 && pSoldier->sSectorX == sSectorX && pSoldier->sSectorY == sSectorY && pSoldier->bSectorZ == bSectorZ )
			{
				if ( FindBestPath( pSoldier, sGridNo2, pSoldier->bLevel, WALKING, NO_COPYROUTE, 0 ) )
				{
					fSecondary = TRUE;
					break;
				}
			}
		}

		if ( !fSecondary )
		{
			sGridNo2 = NOWHERE;
		}

	}

	GlobalItemsReachableTest( sGridNo, sGridNo2 );

	for( uiCounter = 0; uiCounter < guiNumWorldItems; uiCounter++ )
	{

		// reset reachablity
		fReachable = FALSE;

		// item doesn't exist, ignore it
		if( gWorldItems[ uiCounter ].fExists == FALSE )
		{
			continue;
		}

		// if the item is trapped then flag it as unreachable, period
		if ( gWorldItems[ uiCounter ].o.bTrap > 0 )
		{
			fReachable = FALSE;
		}
		else if ( ItemTypeExistsAtLocation( gWorldItems[ uiCounter ].sGridNo, OWNERSHIP, gWorldItems[ uiCounter ].ubLevel, NULL ) )
		{
			fReachable = FALSE;
		}
		else if ( gWorldItems[ uiCounter ].o.usItem == CHALICE )
		{
			fReachable = FALSE;
		}
		else if ( gpWorldLevelData[ gWorldItems[ uiCounter ].sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
		{
			// the gridno itself is reachable so the item is reachable
			fReachable = TRUE;
		}
		else if ( gWorldItems[ uiCounter ].ubLevel > 0 )
		{
			// items on roofs are always reachable
			fReachable = TRUE;
		}
		else
		{
			// check the 4 grids around the item, if any is reachable...then the item is reachable
			for ( ubDir = 0; ubDir < NUM_WORLD_DIRECTIONS; ubDir += 2 )
			{
				sNewLoc = NewGridNo( gWorldItems[ uiCounter ].sGridNo, DirectionInc( ubDir ) );
				if ( sNewLoc != gWorldItems[ uiCounter ].sGridNo )
				{
					// then it's a valid gridno, so test it
					// requires non-wall movement cost from one location to the other!
					if ( gpWorldLevelData[ sNewLoc ].uiFlags & MAPELEMENT_REACHABLE )
					{
 						ubMovementCost = gubWorldMovementCosts[ gWorldItems[ uiCounter ].sGridNo ][ gOppositeDirection[ ubDir ] ][ 0 ];
						// if we find a door movement cost, if the door is open the gridno should be accessible itself
						if ( ubMovementCost != TRAVELCOST_DOOR && ubMovementCost != TRAVELCOST_WALL )
						{
							fReachable = TRUE;
							break;
						}
					}
				}
			}
		}

		if( fReachable )
		{
			gWorldItems[ uiCounter ].usFlags |= WORLD_ITEM_REACHABLE;
		}
		else
		{
			gWorldItems[ uiCounter ].usFlags &= ~( WORLD_ITEM_REACHABLE );
		}
	}
}


static BOOLEAN DoesTempFileExistsForMap(UINT32 uiType, INT16 sMapX, INT16 sMapY, INT8 bMapZ);
static BOOLEAN LoadAndAddWorldItemsFromTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
static UINT32 GetLastTimePlayerWasInSector(void);
static BOOLEAN LoadRottingCorpsesFromTempCorpseFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ);
static void LoadNPCInformationFromProfileStruct(void);


BOOLEAN LoadCurrentSectorsInformationFromTempItemsFile()
{
	BOOLEAN fUsedTempFile = FALSE;

	//
	// Load in the sectors ITems
	//

	if( AreInMeanwhile() )
	{ //There will never be a temp file for the meanwhile scene, so return TRUE.  However,
		//set a flag to not save it either!
		gfWasInMeanwhile = TRUE;

		// OK  - this is true except for interrotations - we need that item temp file to be
		// processed!
		if ( GetMeanwhileID() == INTERROGATION )
		{
			//If there is a file, load in the Items array
			if( DoesTempFileExistsForMap( SF_ITEM_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
			{
				if( !LoadAndAddWorldItemsFromTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
					return( FALSE );
			}

			gfWasInMeanwhile = FALSE;
		}
		return TRUE;
	}

	//if we are in an above ground sector

	//If there is a file, load in the Items array
	if( DoesTempFileExistsForMap( SF_ITEM_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadAndAddWorldItemsFromTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
			return( FALSE );
	}

	//If there is a rotting corpse temp file, load the data from the temp file
	if( DoesTempFileExistsForMap( SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadRottingCorpsesFromTempCorpseFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
			return( FALSE );
	}

	//If there is a map modifications file, load the data from the temp file
	if( DoesTempFileExistsForMap( SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadAllMapChangesFromMapTempFileAndApplyThem( ) )
			return( FALSE );
	}

	//if there is a door table temp file, load the data from the temp file
	if( DoesTempFileExistsForMap( SF_DOOR_TABLE_TEMP_FILES_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadDoorTableFromDoorTableTempFile( ) )
			return( FALSE );
	}

	//if there is a revealed status temp file, load the data from the temp file
	if( DoesTempFileExistsForMap( SF_REVEALED_STATUS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadRevealedStatusArrayFromRevealedTempFile( ) )
			return( FALSE );
	}

	//if there is a door status temp file, load the data from the temp file
	if( DoesTempFileExistsForMap( SF_DOOR_STATUS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadDoorStatusArrayFromDoorStatusTempFile( ) )
			return( FALSE );
	}


	//if the save is an older version, use theold way of oading it up
	if( guiSavedGameVersion < 57 )
	{
		if( DoesTempFileExistsForMap( SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
		{
			fUsedTempFile = TRUE;
			if( !LoadEnemySoldiersFromTempFile( ) )
				return( FALSE );
		}
	}

	//else use the new way of loading the enemy and civilian placements
	else
	{
		if( DoesTempFileExistsForMap( SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
		{
			fUsedTempFile = TRUE;
			if( !NewWayOfLoadingEnemySoldiersFromTempFile( ) )
				return( FALSE );
		}

		if( DoesTempFileExistsForMap( SF_CIV_PRESERVED_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
		{
			fUsedTempFile = TRUE;
			if( !NewWayOfLoadingCiviliansFromTempFile( ) )
				return( FALSE );
		}
	}

	if( DoesTempFileExistsForMap( SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadSmokeEffectsFromMapTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
			return( FALSE );
	}

	if( DoesTempFileExistsForMap( SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fUsedTempFile = TRUE;
		if( !LoadLightEffectsFromMapTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
			return( FALSE );
	}

//ttt

	// Check to see if any npc are in this sector, if so load up some saved data for them
	LoadNPCInformationFromProfileStruct();

	//if we are loading a saved game
//	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		//Init the world since we have modified the map
		InitLoadedWorld();
	}


	// Get the last time the player was in the sector
	guiTimeCurrentSectorWasLastLoaded = GetLastTimePlayerWasInSector();

	if( fUsedTempFile )
	{
//		ValidateSoldierInitLinks( 3 );
	}


	StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated();

	return( TRUE );
}


static void SetLastTimePlayerWasInSector(void)
{
	if( !gbWorldSectorZ )
		SectorInfo[ SECTOR( gWorldSectorX,gWorldSectorY) ].uiTimeCurrentSectorWasLastLoaded = GetWorldTotalMin();
	else if( gbWorldSectorZ > 0 )
	{
		UNDERGROUND_SECTORINFO *pTempNode = gpUndergroundSectorInfoHead;

		pTempNode = gpUndergroundSectorInfoHead;

		//loop through and look for the right underground sector
		while( pTempNode )
		{
			if( ( pTempNode->ubSectorX == gWorldSectorX ) &&
					( pTempNode->ubSectorY == gWorldSectorY ) &&
					( pTempNode->ubSectorZ == gbWorldSectorZ ) )
			{
				//set the flag indicating that ther is a temp item file exists for the sector
				pTempNode->uiTimeCurrentSectorWasLastLoaded = GetWorldTotalMin();
				return;	//break out
			}
			pTempNode = pTempNode->next;
		}
		#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Failed to Set the 'uiTimeCurrentSectorWasLastLoaded' for an underground sector" );
		#endif
	}
}


static UINT32 GetLastTimePlayerWasInSector(void)
{
	if( !gbWorldSectorZ )
		return( SectorInfo[ SECTOR( gWorldSectorX,gWorldSectorY) ].uiTimeCurrentSectorWasLastLoaded );
	else if( gbWorldSectorZ > 0 )
	{
		UNDERGROUND_SECTORINFO *pTempNode = gpUndergroundSectorInfoHead;

		pTempNode = gpUndergroundSectorInfoHead;

		//loop through and look for the right underground sector
		while( pTempNode )
		{
			if( ( pTempNode->ubSectorX == gWorldSectorX ) &&
					( pTempNode->ubSectorY == gWorldSectorY ) &&
					( pTempNode->ubSectorZ == gbWorldSectorZ ) )
			{
				//set the flag indicating that ther is a temp item file exists for the sector
				return( pTempNode->uiTimeCurrentSectorWasLastLoaded );
			}
			pTempNode = pTempNode->next;
		}

		#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Failed to Get the 'uiTimeCurrentSectorWasLastLoaded' from an underground sector" );
		#endif

		return( 0 );
	}
	return( 0 );
}


static BOOLEAN LoadAndAddWorldItemsFromTempFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	UINT32	uiNumberOfItems=0;
	WORLDITEM *pWorldItems = NULL;
	UINT32	cnt;
  INT16   sNewGridNo;

	//Get the number of items from the file
	if( !GetNumberOfWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, &uiNumberOfItems, TRUE ) )
	{
		//Error getting the numbers of the items from the sector
		return( FALSE );
	}

	if( uiNumberOfItems )
	{
		pWorldItems = MemAlloc( sizeof( WORLDITEM ) * uiNumberOfItems );
		if( pWorldItems == NULL )
		{
			//Error Allocating memory for the temp item array
			return( FALSE );
		}
	}
	else
	{
		//if there are no items in the temp, the player might have cleared all of them out, check to see
		//If we have already been to the sector
		if( GetSectorFlagStatus( sMapX, sMapY, bMapZ, SF_ALREADY_LOADED ) )
		{
			//
			//Completly replace the current sectors item table because all the items SHOULD be in the temp file!!
			//

			//Destroy the current sectors item table
			TrashWorldItems();

			//Add each item to the pool, handle below, outside of the if
		}

		//there are no items in the file
		return( TRUE );
	}

	//Clear the memory
	memset( pWorldItems, 0, sizeof( WORLDITEM ) * uiNumberOfItems );

	//Load the World Items from the file
	if( !LoadWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, pWorldItems ) )
		return( FALSE );


	//If we have already been to the sector
	if( GetSectorFlagStatus( sMapX, sMapY, bMapZ, SF_ALREADY_LOADED ) )
	{
		//
		//Completly replace the current sectors item table because all the items SHOULD be in the temp file!!
		//

		//Destroy the current sectors item table
		TrashWorldItems();

		//Add each item to the pool, handle below, outside of the if
	}

	//
	//Append the items in the file with to the current sectors item table
	//

	//Loop through all the items loaded from the file
	for( cnt=0; cnt< uiNumberOfItems; cnt++)
	{
		//If the item in the array is valid
		if( pWorldItems[cnt].fExists )
		{
			//Check the flags to see if we have to find a gridno to place the items at
			if( pWorldItems[cnt].usFlags & WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO )
			{
			  sNewGridNo = FindNearestAvailableGridNoForItem( pWorldItems[cnt].sGridNo, 5 );
			  if( sNewGridNo == NOWHERE )
				  sNewGridNo = FindNearestAvailableGridNoForItem( pWorldItems[cnt].sGridNo, 15 );

        if ( sNewGridNo != NOWHERE )
        {
          pWorldItems[cnt].sGridNo = sNewGridNo;
        }
			}

			//If the item has an invalid gridno, use the maps entry point
			if( pWorldItems[cnt].usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT )
			{
				pWorldItems[cnt].sGridNo = gMapInformation.sCenterGridNo;
			}

			//add the item to the world
			AddItemToPool( pWorldItems[cnt].sGridNo, &pWorldItems[cnt].o, pWorldItems[cnt].bVisible, pWorldItems[cnt].ubLevel, pWorldItems[cnt].usFlags, pWorldItems[cnt].bRenderZHeightAboveLevel );
		}
	}

	return( TRUE );
}


static BOOLEAN AddTempFileToSavedGame(HWFILE hFile, UINT32 uiType, INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	CHAR8		zMapName[ 128 ];

	GetMapTempFileName( uiType, zMapName, sMapX, sMapY, bMapZ );

	//Save the map temp file to the saved game file
	if( !SaveFilesToSavedGame( zMapName, hFile ) )
		return( FALSE );

	return( TRUE );
}


static BOOLEAN RetrieveTempFileFromSavedGame(HWFILE hFile, UINT32 uiType, INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	CHAR8		zMapName[ 128 ];

	GetMapTempFileName( uiType, zMapName, sMapX, sMapY, bMapZ );

	//Load the map temp file from the saved game file
 	if( !LoadFilesFromSavedGame( zMapName, hFile ) )
		return( FALSE );

	return( TRUE );
}


static BOOLEAN InitTempNpcQuoteInfoForNPCFromTempFile(void);


//Deletes the Temp map Directory
BOOLEAN InitTacticalSave( BOOLEAN fCreateTempDir )
{
	UINT32	uiRetVal;

	//If the Map Temp directory exists, removes the temp files
	uiRetVal = FileGetAttributes( MAPS_DIR );
	if( uiRetVal != 0xFFFFFFFF )
	{
		if( uiRetVal & FILE_ATTRIBUTES_DIRECTORY )
		{
			//Erase the directory
			if( !EraseDirectory( MAPS_DIR ) )
			{
				//error erasing the temporary maps directory
			}
		}
	}
	else
	{
		if( !MakeFileManDirectory( MAPS_DIR ) )
		{
			//Erro creating the temp map directory
			AssertMsg( 0, "Error creating the Temp Directory.");
		}
	}

	if( fCreateTempDir )
	{
		//Create the initial temp file for the Npc Quote Info
		InitTempNpcQuoteInfoForNPCFromTempFile();
	}

 	return( TRUE );
}


static BOOLEAN SaveRottingCorpsesToTempCorpseFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	HWFILE	hFile;
	UINT32	uiNumBytesWritten=0;
//	CHAR8		zTempName[ 128 ];
	CHAR8		zMapName[ 128 ];
	UINT32	uiNumberOfCorpses=0;
	INT32		iCount;

/*
	//Convert the current sector location into a file name
	GetMapFileName( sMapX,sMapY, bMapZ, zTempName, FALSE );

	//add the 'r' for 'Rotting Corpses' to the front of the map name
	sprintf( zMapName, "%s\\r_%s", MAPS_DIR, zTempName);
*/

	GetMapTempFileName( SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );


	//Open the file for writing, Create it if it doesnt exist
	hFile = FileOpen( zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
	if( hFile == 0 )
	{
		//Error opening map modification file
		return( FALSE );
	}


	//Determine how many rotting corpses there are
	for(iCount=0; iCount < giNumRottingCorpse; iCount++)
	{
		if( gRottingCorpse[iCount].fActivated == TRUE )
			uiNumberOfCorpses++;
	}


	//Save the number of the Rotting Corpses array table
	FileWrite( hFile, &uiNumberOfCorpses, sizeof( UINT32 ), &uiNumBytesWritten );
	if( uiNumBytesWritten != sizeof( UINT32 ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}

	//Loop through all the carcases in the array and save the active ones
	for(iCount=0; iCount < giNumRottingCorpse; iCount++)
	{
		if( gRottingCorpse[iCount].fActivated == TRUE )
		{
			//Save the RottingCorpse info array
			FileWrite( hFile, &gRottingCorpse[iCount].def, sizeof( ROTTING_CORPSE_DEFINITION ), &uiNumBytesWritten );
			if( uiNumBytesWritten != sizeof( ROTTING_CORPSE_DEFINITION ) )
			{
				//Error Writing size of array to disk
				FileClose( hFile );
				return( FALSE );
			}
		}
	}

	FileClose( hFile );

	// Set the flag indicating that there is a rotting corpse Temp File
//	SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags |= SF_ROTTING_CORPSE_TEMP_FILE_EXISTS;
	SetSectorFlag( sMapX, sMapY, bMapZ, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS );

	return( TRUE );
}


static BOOLEAN DeleteTempItemMapFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	UINT8 bSectorId = 0;
//	CHAR8		zTempName[ 128 ];
	CHAR8		zMapName[ 128 ];

	// grab the sector id
	bSectorId = SECTOR( sMapX, sMapY );
/*
	//Convert the current sector location into a file name
	GetMapFileName( sMapX,sMapY, bMapZ, zTempName, FALSE );

	sprintf( zMapName, "%s\\r_%s", MAPS_DIR, zTempName);
*/
	GetMapTempFileName( SF_ITEM_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );

	//Check to see if the file exists
	if( !FileExists( zMapName ) )
	{
		//If the file doesnt exists, its no problem.
		return( TRUE );
	}

	// the sector info flag being reset
	ReSetSectorFlag( sMapX, sMapY, bMapZ, SF_ITEM_TEMP_FILE_EXISTS );

	return( TRUE );
}


static BOOLEAN LoadRottingCorpsesFromTempCorpseFile(INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	HWFILE	hFile;
	UINT32	uiNumBytesRead=0;
//	CHAR8		zTempName[ 128 ];
	CHAR8		zMapName[ 128 ];
	UINT32	uiNumberOfCorpses=0;
	UINT32		cnt;
	ROTTING_CORPSE_DEFINITION		def;
  BOOLEAN                     fDontAddCorpse = FALSE;
  INT8                        bTownId;


	//Delete the existing rotting corpse array
	RemoveCorpses( );
/*
	//Convert the current sector location into a file name
	GetMapFileName( sMapX,sMapY, bMapZ, zTempName, FALSE );

	sprintf( zMapName, "%s\\r_%s", MAPS_DIR, zTempName);
*/
	GetMapTempFileName( SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );


	//Check to see if the file exists
	if( !FileExists( zMapName ) )
	{
		//If the file doesnt exists, its no problem.
		return( TRUE );
	}

	//Open the file for reading
	hFile = FileOpen( zMapName, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE );
	if( hFile == 0 )
	{
		//Error opening map modification file,
		return( FALSE );
	}

	// Load the number of Rotting corpses
	FileRead( hFile, &uiNumberOfCorpses, sizeof( UINT32 ), &uiNumBytesRead );
	if( uiNumBytesRead != sizeof( UINT32 ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}

  // Get town ID for use later....
	bTownId = GetTownIdForSector( gWorldSectorX, gWorldSectorY );

	for( cnt=0; cnt<uiNumberOfCorpses; cnt++ )
	{
    fDontAddCorpse = FALSE;

		// Load the Rotting corpses info
		FileRead( hFile, &def, sizeof( ROTTING_CORPSE_DEFINITION ), &uiNumBytesRead );
		if( uiNumBytesRead != sizeof( ROTTING_CORPSE_DEFINITION ) )
		{
			//Error Writing size of array to disk
			FileClose( hFile );
			return( FALSE );
		}


		//Check the flags to see if we have to find a gridno to place the rotting corpses at
		if( def.usFlags & ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO )
		{
			def.sGridNo = FindNearestAvailableGridNoForCorpse( &def, 5 );
			if( def.sGridNo == NOWHERE )
				def.sGridNo = FindNearestAvailableGridNoForCorpse( &def, 15 );

      // ATE: Here we still could have a bad location, but send in NOWHERE
      // to corpse function anyway, 'cause it will iwth not drop it or use
      // a map edgepoint....
		}
		else if( def.usFlags & ROTTING_CORPSE_USE_NORTH_ENTRY_POINT )
		{
			def.sGridNo = gMapInformation.sNorthGridNo;
		}
		else if( def.usFlags & ROTTING_CORPSE_USE_SOUTH_ENTRY_POINT )
		{
			def.sGridNo = gMapInformation.sSouthGridNo;;
		}
		else if( def.usFlags & ROTTING_CORPSE_USE_EAST_ENTRY_POINT )
		{
			def.sGridNo = gMapInformation.sEastGridNo;
		}
		else if( def.usFlags & ROTTING_CORPSE_USE_WEST_ENTRY_POINT )
		{
			def.sGridNo = gMapInformation.sWestGridNo;
		}
		//Recalculate the dx,dy info
		def.dXPos = CenterX( def.sGridNo );
		def.dYPos = CenterY( def.sGridNo );

    // If not from loading a save....
	  if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
    {
      // ATE: Don't place corpses if
      // a ) in a town and b) indoors
      if ( gbWorldSectorZ == 0 )
      {
        if ( bTownId != BLANK_SECTOR )
        {
          // Are we indoors?
          if( FloorAtGridNo( def.sGridNo ) )
          {
             // OK, finally, check TOC vs game time to see if at least some time has passed
	           if ( ( GetWorldTotalMin( ) - def.uiTimeOfDeath ) >= 30 )
             {
                fDontAddCorpse = TRUE;
             }
          }
        }
      }
    }

    if ( !fDontAddCorpse )
    {
		  //add the rotting corpse info
		  if( AddRottingCorpse( &def ) == -1 )
		  {
			  DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Failed to add a corpse to GridNo # %d", def.sGridNo ) );

  /*
			  Assert( 0 );
			  FileClose( hFile );
			  return( FALSE );
  */
		  }
    }
	}

	FileClose( hFile );

	//Check to see if we have to start decomposing the corpses
	HandleRottingCorpses( );

	return( TRUE );
}




BOOLEAN AddWorldItemsToUnLoadedSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ, INT16 sGridNo, UINT32 uiNumberOfItems, WORLDITEM *pWorldItem, BOOLEAN fOverWrite )
{
	UINT32 uiLoop;
	BOOLEAN fLoop=fOverWrite;

	for( uiLoop=0; uiLoop<uiNumberOfItems; uiLoop++)
	{
		//If the item exists
		if( pWorldItem[uiLoop].fExists )
		{
			AddItemsToUnLoadedSector( sMapX, sMapY, bMapZ, pWorldItem[uiLoop].sGridNo, 1, &pWorldItem[ uiLoop ].o, pWorldItem[ uiLoop ].ubLevel, pWorldItem[ uiLoop ].usFlags, pWorldItem[ uiLoop ].bRenderZHeightAboveLevel, pWorldItem[ uiLoop ].bVisible, fLoop );

			fLoop = FALSE;
		}
	}

	return( TRUE );
}


static BOOLEAN SaveTempNpcQuoteInfoForNPCToTempFile(UINT8 ubNpcId);


static void SaveNPCInformationToProfileStruct(void)
{
	UINT32								cnt;
	SOLDIERTYPE *					pSoldier;
	MERCPROFILESTRUCT *		pProfile;

	//Loop through the active NPC's

	// Only do this on save now... on traversal this is handled in the strategic code
	if ( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		return;
	}

	for ( cnt = 0; cnt < guiNumMercSlots; cnt++)
	{
		pSoldier = MercSlots[ cnt ];

		//if it is an active NPC
		if ( pSoldier && pSoldier->ubProfile != NO_PROFILE && pSoldier->bTeam == CIV_TEAM )
		{
			//Save Temp Npc Quote Info array
			SaveTempNpcQuoteInfoForNPCToTempFile( pSoldier->ubProfile );

			pProfile = &(gMercProfiles[ pSoldier->ubProfile ]);

			pProfile->ubQuoteActionID = pSoldier->ubQuoteActionID;
			pProfile->ubQuoteRecord = pSoldier->ubQuoteRecord;

			// if the merc is NOT added due to flag set, return
			if ( pProfile->ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR )
			{
				continue;
			}

			if ( pProfile->ubMiscFlags3 & PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE )
			{
				continue;
			}

			pProfile->fUseProfileInsertionInfo = TRUE;
			pProfile->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			//if ( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
			//{
				// if we are saving the game, save the NPC's current location
				pProfile->usStrategicInsertionData = pSoldier->sGridNo;
			//}
			/*
			else
			{
				// If the NPC is moving, save the final destination, else save the current location
				if ( pSoldier->sFinalDestination != pSoldier->sGridNo )
				{
					pProfile->usStrategicInsertionData = pSoldier->sFinalDestination;
				}
				else
				{
					pProfile->usStrategicInsertionData = pSoldier->sGridNo;
				}
			}
			*/

		}
	}
}

extern void EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination( SOLDIERTYPE *pSoldier, FLOAT dNewXPos, FLOAT dNewYPos, BOOLEAN fUpdateDest,  BOOLEAN fUpdateFinalDest );


static INT16 GetSoldierIDFromAnyMercID(UINT8 ubMercID);
static BOOLEAN LoadTempNpcQuoteInfoForNPCFromTempFile(UINT8 ubNpcId);


static void LoadNPCInformationFromProfileStruct(void)
{
	UINT32					cnt;
	INT16						sSoldierID;
	SOLDIERTYPE *		pSoldier;

	// CJC: disabled this Dec 21, 1998 as unnecessary (and messing up quote files for recruited/escorted NPCs
	return;

	for ( cnt = FIRST_RPC; cnt < NUM_PROFILES; cnt++ )
	{
		if ( gMercProfiles[ cnt ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
		{
			// don't load
			continue;
		}

		sSoldierID = GetSoldierIDFromAnyMercID( (UINT8)cnt );

		//if the soldier is not loaded, return
		if( sSoldierID == -1 )
			continue;

		//if we cant get a pointer to the soldier, continue
		if( !GetSoldier( &pSoldier, sSoldierID ) )
			continue;

		// load quote info if it exists
		if ( gMercProfiles[ cnt ].ubMiscFlags & PROFILE_MISC_FLAG_TEMP_NPC_QUOTE_DATA_EXISTS )
		{
			LoadTempNpcQuoteInfoForNPCFromTempFile( (UINT8)cnt );
		}

		// load insertion info
		/*
		if ( gMercProfiles[ cnt ] )
		{
			pSoldier->ubInsertionCode = pProfile->ubStrategicInsertionCode;
			pSoldier->usInsertionData = pProfile->usStrategicInsertionData;
		}
		*/

	}


/*
	INT16 sX, sY;
	UINT16	cnt;
	SOLDIERTYPE		*pSoldier;
	INT16			sSoldierID;
	INT16		sXPos, sYPos;

	sXPos = sYPos = 0;

	//Loop through the active NPC's
//	cnt = gTacticalStatus.Team[ OUR_TEAM ].bLastID + 1;
	for(cnt=FIRST_RPC; cnt<NUM_PROFILES; cnt++)
	{
		if ( gMercProfiles[ cnt ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
		{
			// don't load
			continue;
		}

		sSoldierID = GetSoldierIDFromAnyMercID( (UINT8)cnt );

		//if the soldier is not loaded, return
		if( sSoldierID == -1 )
			continue;

		//if we cant get a pointer to the soldier, continue
		if( !GetSoldier( &pSoldier, sSoldierID ) )
			continue;

		pSoldier->ubQuoteActionID = gMercProfiles[ cnt ].ubQuoteActionID;
		pSoldier->ubQuoteRecord = gMercProfiles[ cnt ].ubQuoteRecord;

		if( gMercProfiles[ cnt ].fUseProfileInsertionInfo == PROFILE_USE_GRIDNO )
		{
			sX = CenterX( gMercProfiles[ cnt ].sGridNo );
			sY = CenterY( gMercProfiles[ cnt ].sGridNo );

			//Load the Temp Npc Quote Info array

			//if the NPC has been recruited, continue
			if( gMercProfiles[ cnt ].ubMiscFlags & PROFILE_MISC_FLAG_TEMP_NPC_QUOTE_DATA_EXISTS )
				LoadTempNpcQuoteInfoForNPCFromTempFile( (UINT8)cnt );

			//if the NPC has been recruited, continue
			if( gMercProfiles[ cnt ].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED )
				continue;

			//If the NPC was supposed to do something when they reached their target destination
			if( pSoldier->sGridNo == pSoldier->sFinalDestination )
			{
				if (pSoldier->ubQuoteRecord && pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST )
				{
					//the mercs gridno has to be the same as the final destination
					EVENT_SetSoldierPosition( pSoldier, (FLOAT) sX, (FLOAT) sY );

					NPCReachedDestination( pSoldier, FALSE );
				}
			}

			//If the NPC's gridno is not nowhere, set him to that position
			if( gMercProfiles[ cnt ].sGridNo != NOWHERE )
			{
				if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
				{
					//Set the NPC's destination
					pSoldier->sDestination = gMercProfiles[ cnt ].sGridNo;
					pSoldier->sDestXPos = sXPos;
					pSoldier->sDestYPos = sYPos;

					// We have moved to a diferent sector and are returning to it, therefore the merc should be in the final dest
					EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination( pSoldier, (FLOAT) sX, (FLOAT) sY, FALSE, TRUE );
				}

				//else we are saving
				else
				{

					//Set the NPC's position
	//				EVENT_SetSoldierPosition( pSoldier, (FLOAT) sX, (FLOAT) sY );
					EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination( pSoldier, (FLOAT) sX, (FLOAT) sY, FALSE, FALSE );
				}
			}
		}

		//else if we are NOT to use the gridno, dont use it but reset the flag
		else if( gMercProfiles[ cnt ].fUseProfileInsertionInfo == PROFILE_DONT_USE_GRIDNO )
		{
			gMercProfiles[ cnt ].fUseProfileInsertionInfo = PROFILE_NOT_SET;
		}
	}
*/
}


BOOLEAN GetNumberOfActiveWorldItemsFromTempFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 *pNumberOfData )
{
	UINT32	uiNumberOfItems=0;
	WORLDITEM *pWorldItems;
	UINT32	cnt;
	UINT32	uiNumberOfActive=0;
	BOOLEAN	fFileLoaded = FALSE;
	UNDERGROUND_SECTORINFO *TempNode = gpUndergroundSectorInfoHead;


	//
	// Load in the sectors ITems
	//

	//If there is a file, load in the Items array
	if( bMapZ  == 0 )
	{
		if( SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & SF_ITEM_TEMP_FILE_EXISTS )
			fFileLoaded = TRUE;
	}
	else
	{
		while( TempNode )
		{
			if( TempNode->ubSectorX == sMapX && TempNode->ubSectorY == sMapY && TempNode->ubSectorZ == bMapZ )
			{
				if( TempNode->uiFlags & SF_ITEM_TEMP_FILE_EXISTS )
					fFileLoaded = TRUE;

				break;
			}

			TempNode = TempNode->next;
		}
	}

	if( fFileLoaded )
	{
		//Get the number of items from the file
		if( !GetNumberOfWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, &uiNumberOfItems, TRUE ) )
		{
			//Error getting the numbers of the items from the sector
			return( FALSE );
		}

		//If there items in the data file
		if( uiNumberOfItems != 0 )
		{
			pWorldItems = MemAlloc( sizeof( WORLDITEM ) * uiNumberOfItems );
			if( pWorldItems == NULL )
			{
				//Error Allocating memory for the temp item array
				return( FALSE );
			}

			//Clear the memory
			memset( pWorldItems, 0, sizeof( WORLDITEM ) * uiNumberOfItems );

			//Load the World Items from the file
			if( !LoadWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, pWorldItems ) )
				return( FALSE );

			uiNumberOfActive = 0;
			for( cnt=0; cnt<uiNumberOfItems; cnt++ )
			{
				if( pWorldItems[cnt].fExists )
					uiNumberOfActive++;
			}
			MemFree( pWorldItems );
		}
		*pNumberOfData = uiNumberOfActive;
	}
	else
		*pNumberOfData = 0;

	return( TRUE );
}


static BOOLEAN DoesTempFileExistsForMap(UINT32 uiType, INT16 sMapX, INT16 sMapY, INT8 bMapZ)
{
	UNDERGROUND_SECTORINFO *TempNode = gpUndergroundSectorInfoHead;

	if( bMapZ == 0 )
	{
		return( (SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags & uiType) ? 1 : 0 );
	}
	else
	{
		while( TempNode )
		{
			if( TempNode->ubSectorX == sMapX && TempNode->ubSectorY == sMapY && TempNode->ubSectorZ == bMapZ )
			{
				return( (TempNode->uiFlags & uiType ) ? 1 : 0 );
			}
			TempNode = TempNode->next;
		}
	}

	return( FALSE );
}


static INT16 GetSoldierIDFromAnyMercID(UINT8 ubMercID)
{
	UINT16 cnt;
	UINT8		ubLastTeamID;
	SOLDIERTYPE		*pTeamSoldier;

	cnt = gTacticalStatus.Team[ OUR_TEAM ].bFirstID;

	ubLastTeamID = TOTAL_SOLDIERS;

  // look for all mercs on the same team,
  for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= ubLastTeamID; cnt++,pTeamSoldier++)
	{
		if( pTeamSoldier->bActive )
		{
			if ( pTeamSoldier->ubProfile == ubMercID )
			{
				return( cnt );
			}
		}
	}

	return( -1 );
}


//Initializes the NPC temp array
static BOOLEAN InitTempNpcQuoteInfoForNPCFromTempFile(void)
{
	UINT32	uiNumBytesWritten;
	UINT8	ubCnt;
	UINT8	ubOne=1;
	UINT8	ubZero=0;
	TempNPCQuoteInfoSave TempNpcQuote[ NUM_NPC_QUOTE_RECORDS ];
	UINT32	uiSizeOfTempArray = sizeof( TempNPCQuoteInfoSave ) * NUM_NPC_QUOTE_RECORDS;
	UINT16	usCnt1;
	HWFILE	hFile;


	//Open the temp npc file
	hFile = FileOpen( NPC_TEMP_QUOTE_FILE, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
	if( hFile == 0 )
	{
		//Error opening temp npc quote info
		return( FALSE );
	}


	//loop through all the npc accounts and write the temp array to disk
	for( usCnt1=0; usCnt1< ( NUM_PROFILES-FIRST_RPC ); usCnt1++)
	{

		memset( TempNpcQuote, 0, uiSizeOfTempArray );

		//Loop through and build the temp array to save
		for( ubCnt=0; ubCnt<NUM_NPC_QUOTE_RECORDS; ubCnt++ )
		{
			if( gpNPCQuoteInfoArray[ usCnt1 ] )
			{
				TempNpcQuote[ ubCnt ].usFlags				= gpNPCQuoteInfoArray[ usCnt1 ][ ubCnt ].fFlags;
				TempNpcQuote[ ubCnt ].sRequiredItem = gpNPCQuoteInfoArray[ usCnt1 ][ ubCnt ].sRequiredItem;
				TempNpcQuote[ ubCnt ].usGoToGridno	= gpNPCQuoteInfoArray[ usCnt1 ][ ubCnt ].usGoToGridno;
			}
		}

		//Save the array to a temp file
		FileWrite( hFile, TempNpcQuote, uiSizeOfTempArray, &uiNumBytesWritten );
		if( uiNumBytesWritten != uiSizeOfTempArray )
		{
			FileClose( hFile );
			return( FALSE );
		}
	}

	FileClose( hFile );
	return( TRUE );
}


static BOOLEAN SaveTempNpcQuoteInfoForNPCToTempFile(UINT8 ubNpcId)
{
	UINT32	uiNumBytesWritten;
	UINT8	ubCnt;
	UINT8	ubOne=1;
	UINT8	ubZero=0;
	TempNPCQuoteInfoSave TempNpcQuote[ NUM_NPC_QUOTE_RECORDS ];
	UINT32	uiSizeOfTempArray = sizeof( TempNPCQuoteInfoSave ) * NUM_NPC_QUOTE_RECORDS;
	UINT32	uiSpotInFile = ubNpcId - FIRST_RPC;
	HWFILE	hFile=0;

	//if there are records to save
	if( gpNPCQuoteInfoArray[ ubNpcId ] )
	{

		hFile = FileOpen( NPC_TEMP_QUOTE_FILE, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
		if( hFile == 0 )
		{
			//Error opening temp npc quote info
			return( FALSE );
		}

		memset( TempNpcQuote, 0, uiSizeOfTempArray );

		//Loop through and build the temp array to save
		for( ubCnt=0; ubCnt<NUM_NPC_QUOTE_RECORDS; ubCnt++ )
		{
			TempNpcQuote[ ubCnt ].usFlags				= gpNPCQuoteInfoArray[ ubNpcId ][ ubCnt ].fFlags;
			TempNpcQuote[ ubCnt ].sRequiredItem = gpNPCQuoteInfoArray[ ubNpcId ][ ubCnt ].sRequiredItem;
			TempNpcQuote[ ubCnt ].usGoToGridno	= gpNPCQuoteInfoArray[ ubNpcId ][ ubCnt ].usGoToGridno;
		}

		//Seek to the correct spot in the file
		FileSeek( hFile, uiSpotInFile * uiSizeOfTempArray, FILE_SEEK_FROM_START );

		//Save the array to a temp file
		FileWrite( hFile, TempNpcQuote, uiSizeOfTempArray, &uiNumBytesWritten );
		if( uiNumBytesWritten != uiSizeOfTempArray )
		{
			FileClose( hFile );
			return( FALSE );
		}

		//Set the fact that the merc has the temp npc quote data
		gMercProfiles[ ubNpcId ].ubMiscFlags |= PROFILE_MISC_FLAG_TEMP_NPC_QUOTE_DATA_EXISTS;

		FileClose( hFile );
	}


	return( TRUE );
}


static BOOLEAN LoadTempNpcQuoteInfoForNPCFromTempFile(UINT8 ubNpcId)
{
	UINT32	uiNumBytesRead;
	UINT8		ubCnt;
	UINT8		ubOne=1;
	UINT8		ubZero=0;
	TempNPCQuoteInfoSave TempNpcQuote[ NUM_NPC_QUOTE_RECORDS ];
	UINT32	uiSizeOfTempArray = sizeof( TempNPCQuoteInfoSave ) * NUM_NPC_QUOTE_RECORDS;
	UINT32	uiSpotInFile = ubNpcId - FIRST_RPC;
	HWFILE	hFile;


	//Init the array
	memset( TempNpcQuote, 0, uiSizeOfTempArray );


	//If there isnt already memory allocated, allocate memory to hold the array
	if( gpNPCQuoteInfoArray[ ubNpcId ] == NULL )
	{
		gpNPCQuoteInfoArray[ ubNpcId ] = MemAlloc( sizeof( NPCQuoteInfo ) * NUM_NPC_QUOTE_RECORDS );
		if( gpNPCQuoteInfoArray[ ubNpcId ] == NULL )
			return( FALSE );
	}


	hFile = FileOpen( NPC_TEMP_QUOTE_FILE, FILE_ACCESS_READ | FILE_OPEN_ALWAYS, FALSE );
	if( hFile == 0 )
	{
		//Error opening temp npc quote info
		return( FALSE );
	}

	//Seek to the correct spot in the file
	FileSeek( hFile, uiSpotInFile * uiSizeOfTempArray, FILE_SEEK_FROM_START );

	//Save the array to a temp file
	FileRead( hFile, TempNpcQuote, uiSizeOfTempArray, &uiNumBytesRead );
	if( uiNumBytesRead != uiSizeOfTempArray )
	{
		FileClose( hFile );
		return( FALSE );
	}



	//Loop through and build the temp array to save
	for( ubCnt=0; ubCnt<NUM_NPC_QUOTE_RECORDS; ubCnt++ )
	{
		gpNPCQuoteInfoArray[ ubNpcId ][ ubCnt ].fFlags					= TempNpcQuote[ ubCnt ].usFlags;
		gpNPCQuoteInfoArray[ ubNpcId ][ ubCnt ].sRequiredItem		= TempNpcQuote[ ubCnt ].sRequiredItem;
		gpNPCQuoteInfoArray[ ubNpcId ][ ubCnt ].usGoToGridno		= TempNpcQuote[ ubCnt ].usGoToGridno;
	}

	FileClose( hFile );

	return( TRUE );
}







void ChangeNpcToDifferentSector( UINT8 ubNpcId, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	if (gMercProfiles[ ubNpcId ].ubMiscFlags2 & PROFILE_MISC_FLAG2_LEFT_COUNTRY)
	{
		// override location, this person is OUTTA here
		sSectorX = 0;
		sSectorY = 0;
		bSectorZ = 0;
	}
	// Carmen no longer traverses out, he is temporarily removed instead

	gMercProfiles[ ubNpcId ].sSectorX = sSectorX;
	gMercProfiles[ ubNpcId ].sSectorY = sSectorY;
	gMercProfiles[ ubNpcId ].bSectorZ = bSectorZ;

	gMercProfiles[ ubNpcId ].fUseProfileInsertionInfo = FALSE;

	/*
	if (!gMercProfiles[ ubNpcId ].ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR)
	{
		gMercProfiles[ ubNpcId ].fUseProfileInsertionInfo = PROFILE_DONT_USE_GRIDNO;
	}
	*/
}





BOOLEAN AddRottingCorpseToUnloadedSectorsRottingCorpseFile( INT16 sMapX, INT16 sMapY, INT8 bMapZ, ROTTING_CORPSE_DEFINITION *pRottingCorpseDef )
{
	HWFILE	hFile;
	UINT32	uiNumberOfCorpses;
//	CHAR8		zTempName[ 128 ];
	CHAR8		zMapName[ 128 ];
	UINT32	uiNumBytesRead;
	UINT32	uiNumBytesWritten;

/*
	//Convert the current sector location into a file name
	GetMapFileName( sMapX,sMapY, bMapZ, zTempName, FALSE );

	//add the 'r' for 'Rotting Corpses' to the front of the map name
	sprintf( zMapName, "%s\\r_%s", MAPS_DIR, zTempName);
*/
	GetMapTempFileName( SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, zMapName, sMapX, sMapY, bMapZ );

	//CHECK TO SEE if the file exist
	if( FileExists( zMapName ) )
	{
		//Open the file for reading
		hFile = FileOpen( zMapName, FILE_ACCESS_READWRITE | FILE_OPEN_EXISTING, FALSE );
		if( hFile == 0 )
		{
			//Error opening map modification file,
			return( FALSE );
		}

		// Load the number of Rotting corpses
		FileRead( hFile, &uiNumberOfCorpses, sizeof( UINT32 ), &uiNumBytesRead );
		if( uiNumBytesRead != sizeof( UINT32 ) )
		{
			//Error Writing size of array to disk
			FileClose( hFile );
			return( FALSE );
		}
	}
	else
	{
		//the file doesnt exists, create a new one
		hFile = FileOpen( zMapName, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE );
		if( hFile == 0 )
		{
			//Error opening map modification file
			return( FALSE );
		}
		uiNumberOfCorpses = 0;
	}

	//Start at the begining of the file
	FileSeek( hFile, 0, FILE_SEEK_FROM_START );

	//Add on to the number and save it back to disk
	uiNumberOfCorpses++;

	FileWrite( hFile, &uiNumberOfCorpses, sizeof( UINT32 ), &uiNumBytesWritten );
	if( uiNumBytesWritten != sizeof( UINT32 ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}


	//Go to the end of the file
	FileSeek( hFile, 0, FILE_SEEK_FROM_END );

	//Append the new rotting corpse def to the end of the file
	FileWrite( hFile, pRottingCorpseDef, sizeof( ROTTING_CORPSE_DEFINITION ), &uiNumBytesWritten );
	if( uiNumBytesWritten != sizeof( ROTTING_CORPSE_DEFINITION ) )
	{
		//Error Writing size of array to disk
		FileClose( hFile );
		return( FALSE );
	}


	FileClose( hFile );
	SetSectorFlag( sMapX, sMapY, bMapZ, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS );
	return( TRUE );
}


static BOOLEAN SetUnderGroundSectorFlag(INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ, UINT32 uiFlagToSet)
{
	UNDERGROUND_SECTORINFO *pTempNode = gpUndergroundSectorInfoHead;

	pTempNode = gpUndergroundSectorInfoHead;

	//loop through and look for the right underground sector
	while( pTempNode )
	{
		if( ( pTempNode->ubSectorX == (UINT8)sSectorX ) &&
				( pTempNode->ubSectorY == (UINT8)sSectorY ) &&
				( pTempNode->ubSectorZ == ubSectorZ ) )
		{
			//set the flag indicating that ther is a temp item file exists for the sector
			pTempNode->uiFlags |= uiFlagToSet;

			return( TRUE );
		}
		pTempNode = pTempNode->next;
	}

	return( FALSE );
}


static BOOLEAN ReSetUnderGroundSectorFlag(INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ, UINT32 uiFlagToSet)
{
	UNDERGROUND_SECTORINFO *pTempNode = gpUndergroundSectorInfoHead;

	pTempNode = gpUndergroundSectorInfoHead;

	//loop through and look for the right underground sector
	while( pTempNode )
	{
		if( ( pTempNode->ubSectorX == (UINT8)sSectorX ) &&
				( pTempNode->ubSectorY == (UINT8)sSectorY ) &&
				( pTempNode->ubSectorZ == ubSectorZ ) )
		{
			//set the flag indicating that ther is a temp item file exists for the sector
			pTempNode->uiFlags &= ~( uiFlagToSet );

			return( TRUE );
		}
		pTempNode = pTempNode->next;
	}

	return( FALSE );
}


static BOOLEAN GetUnderGroundSectorFlagStatus(INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ, UINT32 uiFlagToCheck)
{
	UNDERGROUND_SECTORINFO *pTempNode = gpUndergroundSectorInfoHead;

	pTempNode = gpUndergroundSectorInfoHead;

	//loop through and look for the right underground sector
	while( pTempNode )
	{
		if( ( pTempNode->ubSectorX == (UINT8)sSectorX ) &&
				( pTempNode->ubSectorY == (UINT8)sSectorY ) &&
				( pTempNode->ubSectorZ == ubSectorZ ) )
		{
			//set the flag indicating that ther is a temp item file exists for the sector
			if( pTempNode->uiFlags & uiFlagToCheck )
				return( TRUE );
			else
				return( FALSE );
		}
		pTempNode = pTempNode->next;
	}
	return( FALSE );
}


BOOLEAN SetSectorFlag( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet )
{
	if( uiFlagToSet == SF_ALREADY_VISITED )
	{
		// do certain things when particular sectors are visited
		if ( ( sMapX == TIXA_SECTOR_X ) && ( sMapY == TIXA_SECTOR_Y ) )
		{
			// Tixa prison (not seen until Tixa visited)
			SectorInfo[ SEC_J9 ].uiFacilitiesFlags |= SFCF_PRISON;
		}

		if ( ( sMapX == GUN_RANGE_X ) && ( sMapY == GUN_RANGE_Y ) && ( bMapZ == GUN_RANGE_Z ) )
		{
			// Alma shooting range (not seen until sector visited)
			SectorInfo[ SEC_H13 ].uiFacilitiesFlags |= SFCF_GUN_RANGE;
			SectorInfo[ SEC_H14 ].uiFacilitiesFlags |= SFCF_GUN_RANGE;
			SectorInfo[ SEC_I13 ].uiFacilitiesFlags |= SFCF_GUN_RANGE;
			SectorInfo[ SEC_I14 ].uiFacilitiesFlags |= SFCF_GUN_RANGE;
		}

		if ( !GetSectorFlagStatus( sMapX, sMapY, bMapZ, SF_ALREADY_VISITED ) )
		{
			// increment daily counter of sectors visited
			gStrategicStatus.ubNumNewSectorsVisitedToday++;
			if ( gStrategicStatus.ubNumNewSectorsVisitedToday == NEW_SECTORS_EQUAL_TO_ACTIVITY )
			{
				// visited enough to count as an active day
				UpdateLastDayOfPlayerActivity( (UINT16) GetWorldDay() );
			}
		}
	}

	if( bMapZ == 0 )
		SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags |= uiFlagToSet;
	else
		SetUnderGroundSectorFlag( sMapX, sMapY, bMapZ, uiFlagToSet );

	return( TRUE );
}

BOOLEAN ReSetSectorFlag( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet )
{
	if( bMapZ == 0 )
		SectorInfo[ SECTOR( sMapX,sMapY) ].uiFlags &= ~( uiFlagToSet );
	else
		ReSetUnderGroundSectorFlag( sMapX, sMapY, bMapZ, uiFlagToSet );

	return( TRUE );
}

BOOLEAN GetSectorFlagStatus( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, UINT32 uiFlagToSet )
{
	if( bMapZ == 0 )
		return( (SectorInfo[ SECTOR( sMapX, sMapY ) ].uiFlags & uiFlagToSet ) ? 1 : 0 );
	else
		return( (GetUnderGroundSectorFlagStatus( sMapX, sMapY, bMapZ, uiFlagToSet ) ) ? 1 : 0  );
}



BOOLEAN AddDeadSoldierToUnLoadedSector( INT16 sMapX, INT16 sMapY, UINT8 bMapZ, SOLDIERTYPE *pSoldier, INT16  sGridNo, UINT32 uiFlags )
{
	UINT32			uiNumberOfItems;
	WORLDITEM		*pWorldItems=NULL;
	UINT				i;
	UINT8				bCount=0;
	UINT16			uiFlagsForWorldItems=0;
	UINT16			usFlagsForRottingCorpse=0;
	ROTTING_CORPSE_DEFINITION		Corpse;
	INT16				sXPos, sYPos;
	UINT32			uiDeathAnim;
	UINT32			uiPossibleDeathAnims[] = {	GENERIC_HIT_DEATH,
																					FALLBACK_HIT_DEATH,
																					PRONE_HIT_DEATH,
																					FLYBACK_HIT_DEATH };
	const UINT8				ubNumOfDeaths=4;



	//setup the flags for the items and the rotting corpses
	if( uiFlags & ADD_DEAD_SOLDIER_USE_GRIDNO )
	{
		uiFlagsForWorldItems		= 0;
		usFlagsForRottingCorpse	= 0;
	}

	else if( uiFlags & ADD_DEAD_SOLDIER_TO_SWEETSPOT )
	{
		uiFlagsForWorldItems		|= WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE;
		usFlagsForRottingCorpse	|= ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO;
	}
	else
		AssertMsg( 0, "ERROR!!	Flag not is Switch statement");



	//
	//Create an array of objects from the mercs inventory
	//

	//go through and and find out how many items there are
	uiNumberOfItems = 0;
	for ( i = 0; i < NUM_INV_SLOTS; i++ )
	{
		if( pSoldier->inv[ i ].usItem != 0 )
		{
			// if not a player soldier
			if ( pSoldier->bTeam != gbPlayerNum )
			{
				// this percent of the time, they don't drop stuff they would've dropped in tactical...
				if ( Random( 100 ) < 75 )
				{
					// mark it undroppable...
					pSoldier->inv[ i ].fFlags |= OBJECT_UNDROPPABLE;
				}
			}

			//if the item can be dropped
			if( !( pSoldier->inv[ i ].fFlags & OBJECT_UNDROPPABLE ) || pSoldier->bTeam == gbPlayerNum )
			{

        uiNumberOfItems++;
			}
		}
	}

  // If a robot, don't drop anything...
  if ( AM_A_ROBOT( pSoldier ) )
  {
    uiNumberOfItems = 0;
  }

	//if there are items to add
	if( uiNumberOfItems )
	{
		//allocate memory for the world item array
		pWorldItems = MemAlloc( sizeof( WORLDITEM ) * uiNumberOfItems );
		if( pWorldItems == NULL )
		{
			//Error Allocating memory for the temp item array
			return( FALSE );
		}
		//Clear the memory
		memset( pWorldItems, 0, sizeof( WORLDITEM ) * uiNumberOfItems );

		//loop through all the soldiers items and add them to the world item array
		bCount = 0;
		for ( i = 0; i < NUM_INV_SLOTS; i++ )
		{
			if( pSoldier->inv[ i ].usItem != 0 )
			{
				//if the item can be dropped
				if( !(pSoldier->inv[ i ].fFlags & OBJECT_UNDROPPABLE) || pSoldier->bTeam == gbPlayerNum )
				{
					ReduceAmmoDroppedByNonPlayerSoldiers( pSoldier, i );

					pWorldItems[ bCount ].fExists = TRUE;
					pWorldItems[ bCount ].sGridNo = sGridNo;
					pWorldItems[ bCount ].ubLevel = (UINT8)pSoldier->bLevel;
					pWorldItems[ bCount ].usFlags = uiFlagsForWorldItems;
					pWorldItems[ bCount ].bVisible = TRUE;
					pWorldItems[ bCount ].bRenderZHeightAboveLevel = 0;

					memcpy( &(pWorldItems[ bCount ].o), &pSoldier->inv[i], sizeof( OBJECTTYPE ) );
					bCount++;
				}
			}
		}

		//Add the soldiers items to an unloaded sector
		AddWorldItemsToUnLoadedSector( sMapX, sMapY, bMapZ, sGridNo, uiNumberOfItems, pWorldItems, FALSE );
	}

  DropKeysInKeyRing( pSoldier, sGridNo, pSoldier->bLevel, 1, FALSE, 0, TRUE );

	//
	//Convert the soldier into a rottng corpse
	//

	memset( &Corpse, 0, sizeof( ROTTING_CORPSE_DEFINITION ) );

	// Setup some values!
	Corpse.ubBodyType							= pSoldier->ubBodyType;
	Corpse.sGridNo								= sGridNo;

	ConvertGridNoToXY( sGridNo, &sXPos, &sYPos );

	Corpse.dXPos									= (FLOAT)( CenterX( sXPos ) );
	Corpse.dYPos									= (FLOAT)( CenterY( sYPos ) );
	Corpse.sHeightAdjustment			= pSoldier->sHeightAdjustment;
	Corpse.bVisible								=	TRUE;

	SET_PALETTEREP_ID ( Corpse.HeadPal,		pSoldier->HeadPal );
	SET_PALETTEREP_ID ( Corpse.VestPal,		pSoldier->VestPal );
	SET_PALETTEREP_ID ( Corpse.SkinPal,		pSoldier->SkinPal );
	SET_PALETTEREP_ID ( Corpse.PantsPal,   pSoldier->PantsPal );

	Corpse.bDirection	= pSoldier->bDirection;

	// Set time of death
	Corpse.uiTimeOfDeath = GetWorldTotalMin( );

	//if the dead body shot be the result of a Jfk headshot, set it
	if( uiFlags & ADD_DEAD_SOLDIER__USE_JFK_HEADSHOT_CORPSE )
		uiDeathAnim = JFK_HITDEATH;

	//else chose a random death sequence
	else
		uiDeathAnim = uiPossibleDeathAnims[ Random( 4 ) ];


	// Set type
	Corpse.ubType	= (UINT8)gubAnimSurfaceCorpseID[ pSoldier->ubBodyType][ uiDeathAnim ];

	Corpse.usFlags |= usFlagsForRottingCorpse;

	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile( sMapX, sMapY, bMapZ, &Corpse);

	//FRee the memory used for the pWorldItem array
	MemFree( pWorldItems );
	pWorldItems = NULL;

	return( TRUE );
}

BOOLEAN SaveTempNpcQuoteArrayToSaveGameFile( HWFILE hFile )
{
	return( SaveFilesToSavedGame( NPC_TEMP_QUOTE_FILE, hFile ) );
}

BOOLEAN LoadTempNpcQuoteArrayToSaveGameFile( HWFILE hFile )
{
	return( LoadFilesFromSavedGame( NPC_TEMP_QUOTE_FILE, hFile ) );
}


static void TempFileLoadErrorMessageReturnCallback(UINT8 ubRetVal)
{
	gfProgramIsRunning = FALSE;
}


//if you call this function, make sure you return TRUE (if applicable) to make the game
//think it succeeded the load.  This sets up the dialog for the game exit, after the hacker
//message appears.
void InitExitGameDialogBecauseFileHackDetected()
{
	SGPRect CenteringRect= {0, 0, 639, 479 };

	// do message box and return
	giErrorMessageBox = DoMessageBox( MSG_BOX_BASIC_STYLE, pAntiHackerString[ ANTIHACKERSTR_EXITGAME ],
											GAME_SCREEN, MSG_BOX_FLAG_OK, TempFileLoadErrorMessageReturnCallback, &CenteringRect );
}



UINT32 MercChecksum( SOLDIERTYPE * pSoldier )
{
	UINT32	uiChecksum = 1;
	UINT32	uiLoop;

	uiChecksum += (pSoldier->bLife + 1);
	uiChecksum *= (pSoldier->bLifeMax + 1);
	uiChecksum += (pSoldier->bAgility + 1);
	uiChecksum *= (pSoldier->bDexterity + 1);
	uiChecksum += (pSoldier->bStrength + 1);
	uiChecksum *= (pSoldier->bMarksmanship + 1);
	uiChecksum += (pSoldier->bMedical + 1);
	uiChecksum *= (pSoldier->bMechanical + 1);
	uiChecksum += (pSoldier->bExplosive + 1);

	// put in some multipliers too!
	uiChecksum *= (pSoldier->bExpLevel + 1);
	uiChecksum += (pSoldier->ubProfile + 1);

	for ( uiLoop = 0; uiLoop < NUM_INV_SLOTS; uiLoop++ )
	{
		uiChecksum += pSoldier->inv[ uiLoop ].usItem;
		uiChecksum += pSoldier->inv[ uiLoop ].ubNumberOfObjects;
	}

	return( uiChecksum );
}

UINT32 ProfileChecksum( MERCPROFILESTRUCT * pProfile )
{
	UINT32	uiChecksum = 1;
	UINT32	uiLoop;

	uiChecksum += (pProfile->bLife + 1);
	uiChecksum *= (pProfile->bLifeMax + 1);
	uiChecksum += (pProfile->bAgility + 1);
	uiChecksum *= (pProfile->bDexterity + 1);
	uiChecksum += (pProfile->bStrength + 1);
	uiChecksum *= (pProfile->bMarksmanship + 1);
	uiChecksum += (pProfile->bMedical + 1);
	uiChecksum *= (pProfile->bMechanical + 1);
	uiChecksum += (pProfile->bExplosive + 1);

	// put in some multipliers too!
	uiChecksum *= (pProfile->bExpLevel + 1);

	for ( uiLoop = 0; uiLoop < NUM_INV_SLOTS; uiLoop++ )
	{
		uiChecksum += pProfile->inv[ uiLoop ];
		uiChecksum += pProfile->bInvNumber[ uiLoop ];
	}

	return( uiChecksum );
}


static const UINT8* GetRotationArray(void)
{
	// based on guiJA2EncryptionSet
	if ( guiJA2EncryptionSet < BASE_NUMBER_OF_ROTATION_ARRAYS * 6 )
	{
		if ( guiJA2EncryptionSet < BASE_NUMBER_OF_ROTATION_ARRAYS * 3 )
		{
			return( gubEncryptionArray1[ guiJA2EncryptionSet % (BASE_NUMBER_OF_ROTATION_ARRAYS * 3) ]);
		}
		else
		{
			return( gubEncryptionArray2[ guiJA2EncryptionSet % (BASE_NUMBER_OF_ROTATION_ARRAYS * 3) ]);
		}
	}
	else
	{
		if ( guiJA2EncryptionSet < BASE_NUMBER_OF_ROTATION_ARRAYS * 9 )
		{
			return( gubEncryptionArray3[ guiJA2EncryptionSet % (BASE_NUMBER_OF_ROTATION_ARRAYS * 3) ]);
		}
		else
		{
			return( gubEncryptionArray4[ guiJA2EncryptionSet % (BASE_NUMBER_OF_ROTATION_ARRAYS * 3) ]);
		}
	}
}

BOOLEAN NewJA2EncryptedFileRead( HWFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead )
{
	UINT32	uiLoop;
	UINT8		ubArrayIndex = 0;
	UINT8		ubLastByte = 0;
	UINT8		ubLastByteForNextLoop;
	BOOLEAN	fRet;
	UINT8 *	pMemBlock;

	const UINT8* pubRotationArray = GetRotationArray();

	fRet = FileRead( hFile, pDest, uiBytesToRead, puiBytesRead );
	if ( fRet )
	{
		pMemBlock = pDest;
		for ( uiLoop = 0; uiLoop < *puiBytesRead; uiLoop++ )
		{
			ubLastByteForNextLoop = pMemBlock[ uiLoop ];
			pMemBlock[ uiLoop ] -= (ubLastByte + pubRotationArray[ ubArrayIndex ]);
			ubArrayIndex++;
			if ( ubArrayIndex >= NEW_ROTATION_ARRAY_SIZE )
			{
				ubArrayIndex = 0;
			}
			ubLastByte = ubLastByteForNextLoop;
		}
	}

	return( fRet );
}


BOOLEAN NewJA2EncryptedFileWrite( HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite, UINT32 *puiBytesWritten )
{
	UINT32	uiLoop;
	UINT8		ubArrayIndex = 0;
	UINT8		ubLastByte = 0;//, ubTemp;
	UINT8 * pMemBlock;
	BOOLEAN	fRet;

	pMemBlock = (UINT8 *) MemAlloc( uiBytesToWrite );

	if ( !pMemBlock )
	{
		return( FALSE );
	}
	memset( pMemBlock, 0, uiBytesToWrite );

	const UINT8* pubRotationArray = GetRotationArray();

	memcpy( pMemBlock, pDest, uiBytesToWrite );
	for ( uiLoop = 0; uiLoop < uiBytesToWrite; uiLoop++ )
	{
		pMemBlock[ uiLoop ] += ubLastByte + pubRotationArray[ ubArrayIndex ];
		ubArrayIndex++;
		if ( ubArrayIndex >= NEW_ROTATION_ARRAY_SIZE )
		{
			ubArrayIndex = 0;
		}
		ubLastByte = pMemBlock[ uiLoop ];
	}

	fRet = FileWrite( hFile, pMemBlock, uiBytesToWrite, puiBytesWritten );

	MemFree( pMemBlock );

	return( fRet );

}


#define ROTATION_ARRAY_SIZE 46
static const UINT8 ubRotationArray[46] = { 132, 235, 125, 99, 15, 220, 140, 89, 205, 132, 254, 144, 217, 78, 156, 58, 215, 76, 163, 187, 55, 49, 65, 48, 156, 140, 201, 68, 184, 13, 45, 69, 102, 185, 122, 225, 23, 250, 160, 220, 114, 240, 64, 175, 057, 233 };

BOOLEAN JA2EncryptedFileRead( HWFILE hFile, PTR pDest, UINT32 uiBytesToRead, UINT32 *puiBytesRead )
{
	UINT32	uiLoop;
	UINT8		ubArrayIndex = 0;
	//UINT8		ubLastNonBlank = 0;
	UINT8		ubLastByte = 0;
	UINT8		ubLastByteForNextLoop;
	BOOLEAN	fRet;
	UINT8 *	pMemBlock;

	fRet = FileRead( hFile, pDest, uiBytesToRead, puiBytesRead );
	if ( fRet )
	{
		pMemBlock = pDest;
		for ( uiLoop = 0; uiLoop < *puiBytesRead; uiLoop++ )
		{
			ubLastByteForNextLoop = pMemBlock[ uiLoop ];
			pMemBlock[ uiLoop ] -= (ubLastByte + ubRotationArray[ ubArrayIndex ]);
			ubArrayIndex++;
			if ( ubArrayIndex >= ROTATION_ARRAY_SIZE )
			{
				ubArrayIndex = 0;
			}
			ubLastByte = ubLastByteForNextLoop;
		}
	}

	return( fRet );
}


BOOLEAN JA2EncryptedFileWrite( HWFILE hFile, PTR pDest, UINT32 uiBytesToWrite, UINT32 *puiBytesWritten )
{
	UINT32	uiLoop;
	UINT8		ubArrayIndex = 0;
	//UINT8		ubLastNonBlank = 0;
	UINT8		ubLastByte = 0;//, ubTemp;
	UINT8 * pMemBlock;
	BOOLEAN	fRet;

	pMemBlock = (UINT8 *) MemAlloc( uiBytesToWrite );

	if ( !pMemBlock )
	{
		return( FALSE );
	}
	memset( pMemBlock, 0, uiBytesToWrite );


	memcpy( pMemBlock, pDest, uiBytesToWrite );
	for ( uiLoop = 0; uiLoop < uiBytesToWrite; uiLoop++ )
	{
		//ubTemp = pMemBlock[ uiLoop ];
		pMemBlock[ uiLoop ] += ubLastByte + ubRotationArray[ ubArrayIndex ];
		ubArrayIndex++;
		if ( ubArrayIndex >= ROTATION_ARRAY_SIZE )
		{
			ubArrayIndex = 0;
		}
		ubLastByte = pMemBlock[ uiLoop ];
		/*
		if ( pMemBlock[ uiLoop ] )
		{
			// store last non blank
			ubLastNonBlank = pMemBlock[ uiLoop ];

			pMemBlock[ uiLoop ] += ubRotationArray[ ubArrayIndex ];
			ubArrayIndex++;
			if ( ubArrayIndex >= ROTATION_ARRAY_SIZE )
			{
				ubArrayIndex = 0;
			}
		}
		else // zero byte
		{
			pMemBlock[ uiLoop ] = ubLastNonBlank + ubRotationArray[ ubArrayIndex ];
			ubArrayIndex++;
			if ( ubArrayIndex >= ROTATION_ARRAY_SIZE )
			{
				ubArrayIndex = 0;
			}
		}
		*/
	}

	fRet = FileWrite( hFile, pMemBlock, uiBytesToWrite, puiBytesWritten );

	MemFree( pMemBlock );

	return( fRet );

}

void GetMapTempFileName( UINT32 uiType, STR pMapName, INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	CHAR	zTempName[512];

	//Convert the current sector location into a file name
	GetMapFileName( sMapX,sMapY, bMapZ, zTempName, FALSE, FALSE );

	switch( uiType )
	{
		case SF_ITEM_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\i_%s", MAPS_DIR, zTempName);
			break;

		case SF_ROTTING_CORPSE_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\r_%s", MAPS_DIR, zTempName);
			break;

		case SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\m_%s", MAPS_DIR, zTempName);
			break;

		case SF_DOOR_TABLE_TEMP_FILES_EXISTS:
			sprintf( pMapName, "%s\\d_%s", MAPS_DIR, zTempName);
			break;

		case SF_REVEALED_STATUS_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\v_%s", MAPS_DIR, zTempName);
			break;

		case SF_DOOR_STATUS_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\ds_%s", MAPS_DIR, zTempName);
			break;

		case SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\e_%s", MAPS_DIR, zTempName);
			break;

		case SF_CIV_PRESERVED_TEMP_FILE_EXISTS:
			// NB save game version 0 is "saving game"
			if ( (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && guiSaveGameVersion != 0 && guiSaveGameVersion < 78 )
			{
				sprintf( pMapName, "%s\\c_%s", MAPS_DIR, zTempName);
			}
			else
			{
				sprintf( pMapName, "%s\\cc_%s", MAPS_DIR, zTempName);
			}
			break;

		case SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\sm_%s", MAPS_DIR, zTempName);
			break;

		case SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS:
			sprintf( pMapName, "%s\\l_%s", MAPS_DIR, zTempName);
			break;

		default:
			Assert( 0 );
			break;
	}
}


static UINT32 UpdateLoadedSectorsItemInventory(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems);


UINT32	GetNumberOfVisibleWorldItemsFromSectorStructureForSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	UINT32 uiNumberOfItems=0;
	UNDERGROUND_SECTORINFO *pSector=NULL;

	//if the sector is above ground
	if( bMapZ == 0 )
	{
		uiNumberOfItems = SectorInfo[ SECTOR( sMapX, sMapY ) ].uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;
	}
	else
	{
		//find the underground sector
		pSector = FindUnderGroundSector( sMapX, sMapY, bMapZ );
		if( pSector != NULL )
		{
			//get the number of items
			uiNumberOfItems = pSector->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;
		}
	}

	//if there is a sector loaded
	if( gfWorldLoaded )
	{
		//and it is the sector we are interested in
		if( sMapX == gWorldSectorX && sMapY == gWorldSectorY && bMapZ == gbWorldSectorZ )
		{
			//since items might have been added, update
			uiNumberOfItems = UpdateLoadedSectorsItemInventory( sMapX, sMapY, bMapZ, uiNumberOfItems );
		}
	}


	return( uiNumberOfItems );
}

void	SetNumberOfVisibleWorldItemsInSectorStructureForSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems )
{
	UNDERGROUND_SECTORINFO *pSector=NULL;

	//if the sector is above ground
	if( bMapZ == 0 )
	{
		SectorInfo[ SECTOR( sMapX, sMapY ) ].uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer = uiNumberOfItems;
	}
	else
	{
		//find the underground sector
		pSector = FindUnderGroundSector( sMapX, sMapY, bMapZ );
		if( pSector != NULL )
		{
			//get the number of items
			pSector->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer = uiNumberOfItems ;
		}
	}
}


static void SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems(INT16 sMapX, INT16 sMapY, INT8 bMapZ, BOOLEAN fLoadingGame)
{
	UINT32 uiTotalNumberOfItems = 0, uiTotalNumberOfRealItems = 0;
	WORLDITEM * pTotalSectorList = NULL;
	UINT32 uiItemCount = 0;
	INT32 iCounter = 0;
	BOOLEAN	fReturn;


	// get total number, visable and invisible
	fReturn = GetNumberOfActiveWorldItemsFromTempFile( sMapX, sMapY, bMapZ, &( uiTotalNumberOfRealItems ) );
	Assert( fReturn );

	fReturn = GetNumberOfWorldItemsFromTempItemFile( sMapX, sMapY, bMapZ, &( uiTotalNumberOfItems ), FALSE );
	Assert( fReturn );

	if( uiTotalNumberOfItems > 0 )
	{
		// allocate space for the list
		pTotalSectorList = MemAlloc( sizeof( WORLDITEM ) * uiTotalNumberOfItems );

		// now load into mem
		LoadWorldItemsFromTempItemFile(  sMapX,  sMapY, bMapZ, pTotalSectorList );
	}

	// now run through list and
	for( iCounter = 0; ( UINT32 )( iCounter )< uiTotalNumberOfRealItems; iCounter++ )
	{
		// if visible to player, then state fact
		if( IsMapScreenWorldItemVisibleInMapInventory( &pTotalSectorList[ iCounter ] ) )
		{
			uiItemCount += pTotalSectorList[ iCounter ].o.ubNumberOfObjects;
		}
	}

	// if anything was alloced, then get rid of it
	if( pTotalSectorList != NULL )
	{
		MemFree( pTotalSectorList );
		pTotalSectorList = NULL;
	}

	#ifdef JA2BETAVERSION
	if( fLoadingGame && guiSaveGameVersion >= 86 )
	{
		UINT32 uiReported = GetNumberOfVisibleWorldItemsFromSectorStructureForSector( sMapX, sMapY, bMapZ );

		if( uiItemCount != uiReported )
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"SynchronizeItemTempFile()  Error!  Reported %d, should be %d", uiReported, uiItemCount  );
	}
	#endif

	//record the number of items
	SetNumberOfVisibleWorldItemsInSectorStructureForSector( sMapX, sMapY, bMapZ, uiItemCount );
}


static UINT32 UpdateLoadedSectorsItemInventory(INT16 sMapX, INT16 sMapY, INT8 bMapZ, UINT32 uiNumberOfItems)
{
	UINT32	uiCounter;
	UINT32	uiItemCounter=0;

	if( !gfWorldLoaded )
		return( 0 );

	//loop through all the world items
	for( uiCounter= 0; uiCounter < guiNumWorldItems; uiCounter++ )
	{
		//if the item CAN be visible in mapscreen sector inventory
		if( IsMapScreenWorldItemVisibleInMapInventory( &gWorldItems[ uiCounter ] ) )
		{
			//increment
			uiItemCounter += gWorldItems[ uiCounter ].o.ubNumberOfObjects;
		}
	}

	//if the item count is DIFFERENT
	if( uiItemCounter != uiNumberOfItems )
	{
		//Update the value in the sector info struct
		SetNumberOfVisibleWorldItemsInSectorStructureForSector( sMapX, sMapY, bMapZ, uiItemCounter );
	}

	//return the number of items
	return( uiItemCounter );
}
