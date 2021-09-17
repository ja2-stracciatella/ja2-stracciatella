#include "Buffer.h"
#include "Font_Control.h"
#include "Handle_Items.h"
#include "Structure.h"
#include "TileDef.h"
#include "Types.h"
#include "SaveLoadMap.h"
#include "Overhead.h"
#include "FileMan.h"
#include "Tactical_Save.h"
#include "Debug.h"
#include "WorldMan.h"
#include "StrategicMap.h"
#include "Campaign_Types.h"
#include "Render_Fun.h"
#include "FOV.h"
#include "WorldDef.h"
#include "Exit_Grids.h"
#include "Message.h"
#include "GameSettings.h"
#include "Smell.h"
#include "MemMan.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include <memory>

#define NUM_REVEALED_BYTES 3200

extern BOOLEAN gfLoadingExitGrids;


bool ApplyMapChangesToMapTempFile::active_ = false;


//  There are 3200 bytes, and each bit represents the revelaed status.
//	3200 bytes * 8 bits = 25600 map elements
UINT8				*gpRevealedMap;


// Opens the map modification temp file (m_*) of the given sector for append, and marks it in the sector flag.
static std::unique_ptr<AutoSGPFile> OpenMapModificationTempFile(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	SetSectorFlag(sSectorX, sSectorY, bSectorZ, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS);

	return std::make_unique<AutoSGPFile>(GCM->openTempFileForAppend(GetMapTempFileName(SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, sSectorX, sSectorY, bSectorZ)));
}

// Writes map modification to the open temp file
static void SaveModifiedMapStructToMapTempFile(MODIFY_MAP const* const pMap, AutoSGPFile& hFile)
{
	hFile->write(pMap, sizeof(MODIFY_MAP));
}

// Opens the map temp file, writes the modification, then close the file
static void SaveModifiedMapStructToMapTempFile(MODIFY_MAP const* const pMap, INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	std::unique_ptr<AutoSGPFile> hFile = OpenMapModificationTempFile(sSectorX, sSectorY, bSectorZ);
	SaveModifiedMapStructToMapTempFile(pMap, *hFile);
}


static void AddBloodOrSmellFromMapTempFileToMap(MODIFY_MAP* pMap);
static void AddObjectFromMapTempFileToMap(UINT32 uiMapIndex, UINT16 usIndex);
static void DamageStructsFromMapTempFile(MODIFY_MAP* pMap);
static bool ModifyWindowStatus(GridNo);
static void RemoveSavedStructFromMap(UINT32 uiMapIndex, UINT16 usIndex);
static void SetOpenableStructStatusFromMapTempFile(UINT32 uiMapIndex, BOOLEAN fOpened);


void LoadAllMapChangesFromMapTempFileAndApplyThem()
{
	UINT32     uiNumberOfElementsSavedBackToFile = 0; // added becuase if no files get saved back to disk, the flag needs to be erased
	UINT32     cnt;
	MODIFY_MAP *pMap;

	ST::string const zMapName = GetMapTempFileName( SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//If the file doesnt exists, its no problem.
	if (!GCM->doesTempFileExist(zMapName)) return;

	UINT32                  uiNumberOfElements;
	SGP::Buffer<MODIFY_MAP> pTempArrayOfMaps;
	{
		AutoSGPFile hFile(GCM->openTempFileForReading(zMapName));

		//Get the size of the file
		uiNumberOfElements = hFile->size() / sizeof(MODIFY_MAP);

		//Read the map temp file into a buffer
		pTempArrayOfMaps.Allocate(uiNumberOfElements);
		hFile->read(pTempArrayOfMaps, sizeof(*pTempArrayOfMaps) * uiNumberOfElements);
	}

	//Delete the file
	GCM->deleteTempFile( zMapName );
	std::unique_ptr<AutoSGPFile> tempMapFile = OpenMapModificationTempFile(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

	for( cnt=0; cnt< uiNumberOfElements; cnt++ )
	{
		pMap = &pTempArrayOfMaps[ cnt ];

		//Switch on the type that should either be added or removed from the map
		switch( pMap->ubType )
		{
			//If we are adding to the map
			case SLM_LAND:
				break;
			case SLM_OBJECT:
			{
				UINT16 usIndex = GetTileIndexFromTypeSubIndex(pMap->usImageType, pMap->usSubImageIndex);
				AddObjectFromMapTempFileToMap( pMap->usGridNo, usIndex );

				// Save this struct back to the temp file
				SaveModifiedMapStructToMapTempFile(pMap, *tempMapFile);

				//Since the element is being saved back to the temp file, increment the #
				uiNumberOfElementsSavedBackToFile++;
				break;
			}

			case SLM_STRUCT:
			{
				UINT16 const usIndex = GetTileIndexFromTypeSubIndex(pMap->usImageType, pMap->usSubImageIndex);
				if (!IndexExistsInStructLayer(pMap->usGridNo, usIndex))
				{
					AddStructToTail(pMap->usGridNo, usIndex);
				}

				// Save this struct back to the temp file
				SaveModifiedMapStructToMapTempFile(pMap, *tempMapFile);

				//Since the element is being saved back to the temp file, increment the #
				uiNumberOfElementsSavedBackToFile++;
				break;
			}

			case SLM_SHADOW:
				break;
			case SLM_MERC:
				break;
			case SLM_ROOF:
				break;
			case SLM_ONROOF:
				break;
			case SLM_TOPMOST:
				break;


			//Remove objects out of the world
			case SLM_REMOVE_LAND:
				break;
			case SLM_REMOVE_OBJECT:
				break;
			case SLM_REMOVE_STRUCT:

				// ATE: OK, dor doors, the usIndex can be varied, opened, closed, etc
				// we MUSTR delete ANY door type on this gridno
				// Since we can only have one door per gridno, we're safe to do so.....
				if ( pMap->usImageType >= FIRSTDOOR && pMap->usImageType <= FOURTHDOOR )
				{
					// Remove ANY door...
					RemoveAllStructsOfTypeRange( pMap->usGridNo, FIRSTDOOR, FOURTHDOOR );
				}
				else
				{
					UINT16 usIndex = GetTileIndexFromTypeSubIndex(pMap->usImageType, pMap->usSubImageIndex);
					RemoveSavedStructFromMap( pMap->usGridNo, usIndex );
				}

				// Save this struct back to the temp file
				SaveModifiedMapStructToMapTempFile(pMap, *tempMapFile);

				//Since the element is being saved back to the temp file, increment the #
				uiNumberOfElementsSavedBackToFile++;
				break;
			case SLM_REMOVE_SHADOW:
				break;
			case SLM_REMOVE_MERC:
				break;
			case SLM_REMOVE_ROOF:
				break;
			case SLM_REMOVE_ONROOF:
				break;
			case SLM_REMOVE_TOPMOST:
				break;


			case SLM_BLOOD_SMELL:
				AddBloodOrSmellFromMapTempFileToMap( pMap );
				break;

			case SLM_DAMAGED_STRUCT:
				DamageStructsFromMapTempFile( pMap );
				break;

			case SLM_EXIT_GRIDS:
				{
					EXITGRID ExitGrid;
					gfLoadingExitGrids = TRUE;
					ExitGrid.usGridNo = pMap->usSubImageIndex;
					ExitGrid.ubGotoSectorX = (UINT8) pMap->usImageType;
					ExitGrid.ubGotoSectorY = (UINT8) ( pMap->usImageType >> 8 ) ;
					ExitGrid.ubGotoSectorZ = pMap->ubExtra;

					AddExitGridToWorld( pMap->usGridNo, &ExitGrid );
					gfLoadingExitGrids = FALSE;

					// Save this struct back to the temp file
					SaveModifiedMapStructToMapTempFile(pMap, *tempMapFile);

					//Since the element is being saved back to the temp file, increment the #
					uiNumberOfElementsSavedBackToFile++;
				}
				break;

			case SLM_OPENABLE_STRUCT:
				SetOpenableStructStatusFromMapTempFile( pMap->usGridNo, (BOOLEAN)pMap->usImageType );
				break;

			case SLM_WINDOW_HIT:
				if ( ModifyWindowStatus( pMap->usGridNo ) )
				{
					// Save this struct back to the temp file
					SaveModifiedMapStructToMapTempFile(pMap, *tempMapFile);

					//Since the element is being saved back to the temp file, increment the #
					uiNumberOfElementsSavedBackToFile++;
				}
				break;

			default:
				SLOGA("Map Type not in switch when loading map changes from temp file");
				break;
		}

	}

	//if no elements are saved back to the file, remove the flag indicating that there is a temp file
	if( uiNumberOfElementsSavedBackToFile == 0 )
	{
		ReSetSectorFlag( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS );
	}
}


static void AddToMapTempFile(UINT32 const uiMapIndex, UINT16 const usIndex, UINT8 const type)
{
	if (!ApplyMapChangesToMapTempFile::IsActive())    return;
	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

	UINT32 const uiType     = GetTileType(usIndex);
	UINT16 const usSubIndex = GetSubIndexFromTileIndex(usIndex);

	MODIFY_MAP m;
	m = MODIFY_MAP{};
	m.usGridNo        = uiMapIndex;
	m.usImageType     = uiType;
	m.usSubImageIndex = usSubIndex;
	m.ubType          = type;
	SaveModifiedMapStructToMapTempFile(&m, gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
}


void AddStructToMapTempFile(UINT32 const uiMapIndex, UINT16 const usIndex)
{
	AddToMapTempFile(uiMapIndex, usIndex, SLM_STRUCT);
}


void AddObjectToMapTempFile(UINT32 const uiMapIndex, UINT16 const usIndex)
{
	AddToMapTempFile(uiMapIndex, usIndex, SLM_OBJECT);
}


static void AddObjectFromMapTempFileToMap(UINT32 uiMapIndex, UINT16 usIndex)
{
	AddObjectToHead( uiMapIndex, usIndex );
}


void AddRemoveObjectToMapTempFile(UINT32 const uiMapIndex, UINT16 const usIndex)
{
	AddToMapTempFile(uiMapIndex, usIndex, SLM_REMOVE_OBJECT);
}


void RemoveStructFromMapTempFile(UINT32 const uiMapIndex, UINT16 const usIndex)
{
	AddToMapTempFile(uiMapIndex, usIndex, SLM_REMOVE_STRUCT);
}


static void RemoveSavedStructFromMap(UINT32 uiMapIndex, UINT16 usIndex)
{
	RemoveStruct( uiMapIndex, usIndex );
}


static void AddOpenableStructStatusToMapTempFile(UINT32 uiMapIndex, BOOLEAN fOpened, AutoSGPFile& hFile);
static void SetSectorsRevealedBit(UINT16 usMapIndex);


void SaveBloodSmellAndRevealedStatesFromMapToTempFile()
{
	MODIFY_MAP Map;
	UINT16	cnt;
	STRUCTURE * pStructure;

	gpRevealedMap = new UINT8[NUM_REVEALED_BYTES]{};

	std::unique_ptr<AutoSGPFile> tempFile = OpenMapModificationTempFile(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

	//Loop though all the map elements
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		//if there is either blood or a smell on the tile, save it
		if( gpWorldLevelData[cnt].ubBloodInfo || gpWorldLevelData[cnt].ubSmellInfo )
		{
			Map = MODIFY_MAP{};


			// Save the BloodInfo in the bottom byte and the smell info in the upper byte
			Map.usGridNo	= cnt;
//			Map.usIndex			= gpWorldLevelData[cnt].ubBloodInfo | ( gpWorldLevelData[cnt].ubSmellInfo << 8 );
			Map.usImageType = gpWorldLevelData[cnt].ubBloodInfo;
			Map.usSubImageIndex = gpWorldLevelData[cnt].ubSmellInfo;


			Map.ubType			= SLM_BLOOD_SMELL;

			//Save the change to the map file
			SaveModifiedMapStructToMapTempFile(&Map, *tempFile);
		}


		//if the element has been revealed
		if( gpWorldLevelData[cnt].uiFlags & MAPELEMENT_REVEALED )
		{
			SetSectorsRevealedBit( cnt );
		}

		//if there is a structure that is damaged
		if( gpWorldLevelData[cnt].uiFlags & MAPELEMENT_STRUCTURE_DAMAGED )
		{
			//loop through all the structures and add all that are damaged
			FOR_EACH_STRUCTURE(pCurrent, cnt, STRUCTURE_BASE_TILE)
			{
				//if the structure has been damaged
				if( pCurrent->ubHitPoints < pCurrent->pDBStructureRef->pDBStructure->ubHitPoints )
				{
					UINT8	ubBitToSet = 0x80;
					UINT8	ubLevel=0;

					if( pCurrent->sCubeOffset != 0 )
						ubLevel |= ubBitToSet;

					Map = MODIFY_MAP{};

					// Save the Damaged value
					Map.usGridNo	= cnt;
//					Map.usIndex			= StructureFlagToType( pCurrent->fFlags ) | ( pCurrent->ubHitPoints << 8 );
					Map.usImageType = StructureFlagToType( pCurrent->fFlags );
					Map.usSubImageIndex = pCurrent->ubHitPoints;


					Map.ubType			= SLM_DAMAGED_STRUCT;
					Map.ubExtra			= pCurrent->ubWallOrientation | ubLevel;

					//Save the change to the map file
					SaveModifiedMapStructToMapTempFile(&Map, *tempFile);
				}
			}
		}

		pStructure = FindStructure( cnt, STRUCTURE_OPENABLE );

		//if this structure
		if( pStructure )
		{
			// if the current structure has an openable structure in it, and it is NOT a door
			if( !( pStructure->fFlags & STRUCTURE_ANYDOOR ) )
			{
				BOOLEAN			fStatusOnTheMap;

				fStatusOnTheMap = ( ( pStructure->fFlags & STRUCTURE_OPEN ) != 0 );

				AddOpenableStructStatusToMapTempFile(cnt, fStatusOnTheMap, *tempFile);
			}
		}
	}
}


// The BloodInfo is saved in the bottom byte and the smell info in the upper byte
static void AddBloodOrSmellFromMapTempFileToMap(MODIFY_MAP* pMap)
{
	gpWorldLevelData[ pMap->usGridNo ].ubBloodInfo = (UINT8)pMap->usImageType;

	//if the blood and gore option IS set, add blood
	if( gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ] )
	{
		// Update graphics for both levels...
		gpWorldLevelData[ pMap->usGridNo ].uiFlags |= MAPELEMENT_REEVALUATEBLOOD;
		UpdateBloodGraphics( pMap->usGridNo, 0 );
		gpWorldLevelData[ pMap->usGridNo ].uiFlags |= MAPELEMENT_REEVALUATEBLOOD;
		UpdateBloodGraphics( pMap->usGridNo, 1 );
	}

	gpWorldLevelData[ pMap->usGridNo ].ubSmellInfo = (UINT8)pMap->usSubImageIndex;
}


void SaveRevealedStatusArrayToRevealedTempFile(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	Assert( gpRevealedMap != NULL );

	AutoSGPFile hFile(GCM->openTempFileForWriting(GetMapTempFileName( SF_REVEALED_STATUS_TEMP_FILE_EXISTS, sSectorX, sSectorY, bSectorZ ), true));

	//Write the revealed array to the Revealed temp file
	hFile->write(gpRevealedMap, NUM_REVEALED_BYTES);

	SetSectorFlag( sSectorX, sSectorY, bSectorZ, SF_REVEALED_STATUS_TEMP_FILE_EXISTS );

	delete[] gpRevealedMap;
	gpRevealedMap = NULL;
}


static void SetMapRevealedStatus(void);


void LoadRevealedStatusArrayFromRevealedTempFile()
{
	ST::string const zMapName = GetMapTempFileName( SF_REVEALED_STATUS_TEMP_FILE_EXISTS, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//If the file doesnt exists, its no problem.
	if (!GCM->doesTempFileExist(zMapName)) return;

	{
		AutoSGPFile hFile(GCM->openTempFileForReading(zMapName));

		Assert( gpRevealedMap == NULL );
		gpRevealedMap = new UINT8[NUM_REVEALED_BYTES]{};

		// Load the Reveal map array structure
		hFile->read(gpRevealedMap, NUM_REVEALED_BYTES);
	}

	//Loop through and set the bits in the map that are revealed
	SetMapRevealedStatus();

	delete[] gpRevealedMap;
	gpRevealedMap = NULL;
}


static void SetSectorsRevealedBit(UINT16 usMapIndex)
{
	UINT16	usByteNumber;
	UINT8		ubBitNumber;

	usByteNumber = usMapIndex / 8;
	ubBitNumber  = usMapIndex % 8;

	gpRevealedMap[ usByteNumber ] |= 1 << ubBitNumber;
}


static void SetMapRevealedStatus(void)
{
	UINT16	usByteCnt;
	UINT8		ubBitCnt;
	UINT16	usMapIndex;

	AssertMsg(gpRevealedMap != NULL, "gpRevealedMap is NULL.  DF 1");

	ClearSlantRoofs( );

	//Loop through all bytes in the array
	for( usByteCnt=0; usByteCnt< 3200; usByteCnt++)
	{
		//loop through all the bits in the byte
		for( ubBitCnt=0; ubBitCnt<8; ubBitCnt++)
		{
			usMapIndex = ( usByteCnt * 8 ) + ubBitCnt;

			if( gpRevealedMap[ usByteCnt ] & ( 1 << ubBitCnt ) )
			{
				gpWorldLevelData[ usMapIndex ].uiFlags |= MAPELEMENT_REVEALED;
				SetGridNoRevealedFlag( usMapIndex );
			}
			else
			{
				gpWorldLevelData[ usMapIndex ].uiFlags &= (~MAPELEMENT_REVEALED );
			}
		}
	}

	ExamineSlantRoofFOVSlots( );

}


static void DamageStructsFromMapTempFile(MODIFY_MAP* pMap)
{
	STRUCTURE *pCurrent=NULL;
	INT8			bLevel;
	UINT8			ubWallOrientation;
	UINT8			ubBitToSet = 0x80;
	UINT8			ubType=0;


	//Find the base structure
	pCurrent = FindStructure( (INT16)pMap->usGridNo, STRUCTURE_BASE_TILE );

	if( pCurrent == NULL )
		return;

	bLevel = pMap->ubExtra & ubBitToSet;
	ubWallOrientation = pMap->ubExtra & ~ubBitToSet;
	ubType = (UINT8) pMap->usImageType;


	//Check to see if the desired strucure node is in this tile
	pCurrent = FindStructureBySavedInfo( pMap->usGridNo, ubType, ubWallOrientation, bLevel );

	if( pCurrent != NULL )
	{
		//Assign the hitpoints
		pCurrent->ubHitPoints = (UINT8)( pMap->usSubImageIndex );

		gpWorldLevelData[ pCurrent->sGridNo ].uiFlags |= MAPELEMENT_STRUCTURE_DAMAGED;
	}
}


void AddStructToUnLoadedMapTempFile( UINT32 uiMapIndex, UINT16 usIndex, INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ  )
{
	MODIFY_MAP Map;

	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
		return;

	const UINT32 uiType     = GetTileType(usIndex);
	const UINT16 usSubIndex = GetSubIndexFromTileIndex(usIndex);

	Map = MODIFY_MAP{};

	Map.usGridNo = (UINT16)uiMapIndex;
//	Map.usIndex		= usIndex;
	Map.usImageType = (UINT16)uiType;
	Map.usSubImageIndex = usSubIndex;


	Map.ubType		= SLM_STRUCT;

	SaveModifiedMapStructToMapTempFile( &Map, sSectorX, sSectorY, ubSectorZ );
}


void RemoveStructFromUnLoadedMapTempFile( UINT32 uiMapIndex, UINT16 usIndex, INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ  )
{
	MODIFY_MAP Map;

	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
		return;

	const UINT32 uiType     = GetTileType(usIndex);
	const UINT16 usSubIndex = GetSubIndexFromTileIndex(usIndex);

	Map = MODIFY_MAP{};

	Map.usGridNo	= (UINT16)uiMapIndex;
//	Map.usIndex			= usIndex;
	Map.usImageType = (UINT16)uiType;
	Map.usSubImageIndex = usSubIndex;

	Map.ubType			= SLM_REMOVE_STRUCT;

	SaveModifiedMapStructToMapTempFile( &Map, sSectorX, sSectorY, ubSectorZ );
}


void AddExitGridToMapTempFile( UINT16 usGridNo, EXITGRID *pExitGrid, INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ )
{
	MODIFY_MAP Map;

	if (!ApplyMapChangesToMapTempFile::IsActive())
	{
		SLOGD("Called AddExitGridToMapTempFile() without holding ApplyMapChangesToMapTempFile");
		return;
	}

	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
		return;

	Map = MODIFY_MAP{};

	Map.usGridNo = usGridNo;
//	Map.usIndex		= pExitGrid->ubGotoSectorX;

	Map.usImageType = pExitGrid->ubGotoSectorX | ( pExitGrid->ubGotoSectorY << 8 );
	Map.usSubImageIndex = pExitGrid->usGridNo;

	Map.ubExtra		= pExitGrid->ubGotoSectorZ;
	Map.ubType		= SLM_EXIT_GRIDS;

	SaveModifiedMapStructToMapTempFile( &Map, sSectorX, sSectorY, ubSectorZ );
}

BOOLEAN RemoveGraphicFromTempFile( UINT32 uiMapIndex, UINT16 usIndex, INT16 sSectorX, INT16 sSectorY, UINT8 ubSectorZ )
try
{
	MODIFY_MAP *pMap;
	BOOLEAN	fRetVal=FALSE;
	UINT32	cnt;

	ST::string const zMapName = GetMapTempFileName( SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, sSectorX, sSectorY, ubSectorZ );

	UINT32                  uiNumberOfElements;
	SGP::Buffer<MODIFY_MAP> pTempArrayOfMaps;
	{
		AutoSGPFile hFile(GCM->openTempFileForReading(zMapName));

		//Get the number of elements in the file
		uiNumberOfElements = hFile->size() / sizeof(MODIFY_MAP);

		//Read the map temp file into a buffer
		pTempArrayOfMaps.Allocate(uiNumberOfElements);
		hFile->read(pTempArrayOfMaps, sizeof(*pTempArrayOfMaps) * uiNumberOfElements);
	}

	//Delete the file
	GCM->deleteTempFile( zMapName );

	//Get the image type and subindex
	const UINT32 uiType     = GetTileType(usIndex);
	const UINT16 usSubIndex = GetSubIndexFromTileIndex(usIndex);

	for( cnt=0; cnt< uiNumberOfElements; cnt++ )
	{
		pMap = &pTempArrayOfMaps[ cnt ];

		//if this is the peice we are looking for
		if( pMap->usGridNo == uiMapIndex && pMap->usImageType == uiType && pMap->usSubImageIndex == usSubIndex )
		{
			//Do nothin
			fRetVal = TRUE;
		}
		else
		{
			//save the struct back to the temp file
			SaveModifiedMapStructToMapTempFile( pMap, sSectorX, sSectorY, ubSectorZ );
		}
	}

	return( fRetVal );
}
catch (...) { return FALSE; }

// Appends the status of openable struct status to the map modification temp file (m_*)
static void AddOpenableStructStatusToMapTempFile(UINT32 uiMapIndex, BOOLEAN fOpened, AutoSGPFile& hFile)
{
	MODIFY_MAP Map;

	Map = MODIFY_MAP{};

	Map.usGridNo = (UINT16)uiMapIndex;
	Map.usImageType = fOpened;

	Map.ubType = SLM_OPENABLE_STRUCT;

	SaveModifiedMapStructToMapTempFile(&Map, hFile);
}

void AddWindowHitToMapTempFile( UINT32 uiMapIndex )
{
	MODIFY_MAP Map;

	Map = MODIFY_MAP{};

	Map.usGridNo = (UINT16)uiMapIndex;
	Map.ubType = SLM_WINDOW_HIT;

	SaveModifiedMapStructToMapTempFile( &Map, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
}


static bool ModifyWindowStatus(GridNo const grid_no)
{
	STRUCTURE* const s = FindStructure(grid_no, STRUCTURE_WALLNWINDOW);
	if (!s) return false; // Forget it, window could be destroyed
	SwapStructureForPartner(s);
	return true;
}


static void SetOpenableStructStatusFromMapTempFile(UINT32 uiMapIndex, BOOLEAN fOpened)
{
	STRUCTURE *pStructure;
	STRUCTURE *pBase;
	BOOLEAN   fStatusOnTheMap;
	INT16     sBaseGridNo = (INT16)uiMapIndex;

	pStructure = FindStructure( (UINT16)uiMapIndex, STRUCTURE_OPENABLE );

	if( pStructure == NULL )
	{
		SLOGD("SetOpenableStructStatusFromMapTempFile( %d, %d ) failed to find the openable struct.", uiMapIndex, fOpened );
		return;
	}

	fStatusOnTheMap = ( ( pStructure->fFlags & STRUCTURE_OPEN ) != 0 );

	if( fStatusOnTheMap != fOpened )
	{
		// Adjust the item's gridno to the base of struct.....
		pBase = FindBaseStructure( pStructure );

		// Get LEVELNODE for struct and remove!
		if (pBase)
		{
			sBaseGridNo = pBase->sGridNo;
		}

		if (!SwapStructureForPartner(pStructure))
		{
			//an error occured
		}

		// Adjust visiblity of any item pools here....
		// ATE: Nasty bug here - use base gridno for structure for items!
		// since items always drop to base gridno in AddItemToPool
		if (fOpened)
		{
			// We are open, make un-hidden if so....
			SetItemsVisibilityOn(sBaseGridNo, 0, ANY_VISIBILITY_VALUE, FALSE);
		}
		else
		{
			// Make sure items are hidden...
			SetItemsVisibilityHidden(sBaseGridNo, 0);
		}
	}
}


void ChangeStatusOfOpenableStructInUnloadedSector(UINT16 const usSectorX, UINT16 const usSectorY, INT8 const bSectorZ, UINT16 const usGridNo, BOOLEAN const fChangeToOpen)
{
	ST::string const map_name = GetMapTempFileName(SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, usSectorX, usSectorY, bSectorZ);

	// If the file doesn't exists, it's no problem.
	if (!GCM->doesTempFileExist(map_name)) return;

	UINT32                  uiNumberOfElements;
	SGP::Buffer<MODIFY_MAP> mm;
	{
		// Read the map temp file into a buffer
		AutoSGPFile src(GCM->openTempFileForReading(map_name));

		uiNumberOfElements = src->size() / sizeof(MODIFY_MAP);

		mm.Allocate(uiNumberOfElements);
		src->read(mm, sizeof(*mm) * uiNumberOfElements);
	}

	for (UINT32 i = 0; i < uiNumberOfElements; ++i)
	{
		MODIFY_MAP* const m = &mm[i];
		if (m->ubType != SLM_OPENABLE_STRUCT || m->usGridNo != usGridNo) continue;
		// This element is of the same type and on the same gridno

		// Change to the desired settings
		m->usImageType = fChangeToOpen;
		break;
	}

	AutoSGPFile dst(GCM->openTempFileForWriting(map_name, true));
	dst->write(mm, sizeof(*mm) * uiNumberOfElements);
}
