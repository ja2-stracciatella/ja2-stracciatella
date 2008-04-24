#include "Animation_Data.h"
#include "HImage.h"
#include "LoadSaveLightSprite.h"
#include "LoadSaveSoldierCreate.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "WorldDef.h"
#include "WorldDat.h"
#include "WCheck.h"
#include "Debug.h"
#include "Smooth.h"
#include "WorldMan.h"
#include "MouseSystem.h"
#include "Sys_Globals.h"
#include "ScreenIDs.h"
#include "Render_Fun.h"
#include "Lighting.h"
#include "Structure.h"
#include "VObject.h"
#include "Soldier_Control.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "Points.h"
#include "Handle_UI.h"
#include "OppList.h"
#include "World_Items.h"
#include "RenderWorld.h"
#include "Radar_Screen.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "Exit_Grids.h"
#include "Tile_Surface.h"
#include "Rotting_Corpses.h"
#include "Keys.h"
#include "Map_Information.h"
#include "Summary_Info.h"
#include "Animated_ProgressBar.h"
#include "PathAI.h"
#include "EditorBuildings.h"
#include "FileMan.h"
#include "Map_Edgepoints.h"
#include "Environment.h"
#include "Structure_Wrap.h"
#include "Scheduling.h"
#include "EditorMapInfo.h"
#include "Pits.h"
#include "Game_Clock.h"
#include "Buildings.h"
#include "StrategicMap.h"
#include "Overhead_Map.h"
#include "Meanwhile.h"
#include "SmokeEffects.h"
#include "LightEffects.h"
#include "MemMan.h"
#include "JAScreens.h"


#define  SET_MOVEMENTCOST( a, b, c, d )				( ( gubWorldMovementCosts[ a ][ b ][ c ] < d ) ? ( gubWorldMovementCosts[ a ][ b ][ c ] = d ) : 0 );
#define  FORCE_SET_MOVEMENTCOST( a, b, c, d )	( gubWorldMovementCosts[ a ][ b ][ c ] = d )
#define  SET_CURRMOVEMENTCOST( a, b )					SET_MOVEMENTCOST( usGridNo, a, 0, b )

#define	 TEMP_FILE_FOR_TILESET_CHANGE				"jatileS34.dat"

#define	 MAP_FULLSOLDIER_SAVED				0x00000001
#define	 MAP_WORLDONLY_SAVED					0x00000002
#define	 MAP_WORLDLIGHTS_SAVED				0x00000004
#define	 MAP_WORLDITEMS_SAVED					0x00000008
#define  MAP_EXITGRIDS_SAVED					0x00000010
#define  MAP_DOORTABLE_SAVED					0x00000020
#define  MAP_EDGEPOINTS_SAVED					0x00000040
#define  MAP_AMBIENTLIGHTLEVEL_SAVED	0x00000080
#define	 MAP_NPCSCHEDULES_SAVED				0x00000100

#ifdef JA2EDITOR
#	include "LoadScreen.h"
#endif

INT32						giCurrentTilesetID = 0;

UINT32			gCurrentBackground = FIRSTTEXTURE;


static INT8 gbNewTileSurfaceLoaded[NUMBEROFTILETYPES];


void SetAllNewTileSurfacesLoaded( BOOLEAN fNew )
{
	memset( gbNewTileSurfaceLoaded, fNew, sizeof( gbNewTileSurfaceLoaded ) );
}


extern void SetInterfaceHeightLevel( );


// Global Variables
MAP_ELEMENT			*gpWorldLevelData;
UINT32					gSurfaceMemUsage;
UINT8						gubWorldMovementCosts[ WORLD_MAX ][MAXDIR][2];

// set to nonzero (locs of base gridno of structure are good) to have it defined by structure code
INT16		gsRecompileAreaTop = 0;
INT16		gsRecompileAreaLeft = 0;
INT16		gsRecompileAreaRight = 0;
INT16		gsRecompileAreaBottom = 0;

//TIMER TESTING STUFF
#ifdef JA2TESTVERSION
	extern UINT32 uiLoadWorldTime;
	extern UINT32 uiTrashWorldTime;
	extern UINT32 uiLoadMapTilesetTime;
	extern UINT32 uiLoadMapLightsTime;
	extern UINT32 uiBuildShadeTableTime;
	extern UINT32 uiNumImagesReloaded;
#endif


BOOLEAN DoorAtGridNo(const UINT32 iMapIndex)
{
	return FindStructure(iMapIndex, STRUCTURE_ANYDOOR) != NULL;
}


BOOLEAN OpenableAtGridNo(const UINT32 iMapIndex)
{
	return FindStructure(iMapIndex, STRUCTURE_OPENABLE) != NULL;
}


BOOLEAN FloorAtGridNo( UINT32 iMapIndex )
{
	LEVELNODE	*pLand;
	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;
	// Look through all objects and Search for type
	while( pLand )
	{
		if ( pLand->usIndex != NO_TILE )
		{
			const UINT32 uiTileType = GetTileType(pLand->usIndex);
			if ( uiTileType >= FIRSTFLOOR && uiTileType <= LASTFLOOR )
			{
				return TRUE;
			}
			pLand = pLand->pNext;
		}
	}
	return FALSE;
}

BOOLEAN GridNoIndoors( UINT32 iMapIndex )
{
	if( gfBasement || gfCaves )
		return TRUE;
	if( FloorAtGridNo( iMapIndex ) )
		return TRUE;
	return FALSE;
}


static UINT8 gbDefaultSurfaceUsed[NUMBEROFTILETYPES];


BOOLEAN InitializeWorld( )
{
	gSurfaceMemUsage = 0;
	giCurrentTilesetID = -1;

	// DB Adds the _8 to the names if we're in 8 bit mode.
	//ProcessTilesetNamesForBPP();

	// ATE: MEMSET LOG HEIGHT VALUES
	memset( gTileTypeLogicalHeight, 1, sizeof( gTileTypeLogicalHeight ) );

	// Memset tile database
	memset( gTileDatabase, 0, sizeof( gTileDatabase ) );

	// Init surface list
	memset( gTileSurfaceArray, 0, sizeof( gTileSurfaceArray ) );

	// Init default surface list
	memset( gbDefaultSurfaceUsed, 0, sizeof( gbDefaultSurfaceUsed ) );


	// Initialize world data

	gpWorldLevelData = MALLOCNZ(MAP_ELEMENT, WORLD_MAX);
	CHECKF( gpWorldLevelData );

	// Init room database
	InitRoomDatabase( );

	// INit tilesets
	InitEngineTilesets( );



	return( TRUE );

}


static void DestroyTileSurfaces(void);


void DeinitializeWorld( )
{
	TrashWorld();

	if ( gpWorldLevelData != NULL )
	{
		MemFree( gpWorldLevelData );
	}

	DestroyTileSurfaces( );
	FreeAllStructureFiles( );

	// Shutdown tile database data
	DeallocateTileDatabase( );
}


static BOOLEAN AddTileSurface(const char* Filename, UINT32 ubType, UINT8 ubTilesetID);


static BOOLEAN LoadTileSurfaces(char ppTileSurfaceFilenames[][32], UINT8 ubTilesetID)
{
	UINT32					uiLoop;

	UINT32					uiPercentage;
	//UINT32					uiLength;
	//UINT16					uiFillColor;

	// If no Tileset filenames are given, return error
	if (ppTileSurfaceFilenames == NULL)
	{
		return FALSE;
	}

	//uiFillColor = Get16BPPColor(FROMRGB(223, 223, 223));
	//ColorFillVideoSurfaceArea( FRAME_BUFFER, 20, 399, 622, 420, uiFillColor );
	//ColorFillVideoSurfaceArea( FRAME_BUFFER, 21, 400, 621, 419, 0 );
	//EndFrameBufferRender( );

	//uiFillColor = Get16BPPColor(FROMRGB( 100, 0, 0 ));
	// load the tile surfaces
	SetRelativeStartAndEndPercentage( 0, 1, 35, L"Tile Surfaces" );
  for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++)
	{

		uiPercentage = (uiLoop * 100) / (NUMBEROFTILETYPES-1);
		RenderProgressBar( 0, uiPercentage );

		//uiFillColor = Get16BPPColor(FROMRGB( 100 + uiPercentage , 0, 0 ));
		//ColorFillVideoSurfaceArea( FRAME_BUFFER, 22, 401, 22 + uiLength, 418, uiFillColor );
		//InvalidateRegion(0, 399, SCREEN_WIDTH, 420);
		//EndFrameBufferRender( );

		const char* filename       = ppTileSurfaceFilenames[uiLoop];
		UINT8       tileset_to_add = ubTilesetID;
		if (filename[0] == '\0')
		{
			// USE FIRST TILESET VALUE!

			// ATE: If here, don't load default surface if already loaded...
			if (gbDefaultSurfaceUsed[uiLoop]) continue;

			filename       = gTilesets[GENERIC_1].TileSurfaceFilenames[uiLoop];
			tileset_to_add = GENERIC_1;
		}

		// Adjust for tileset position
		char AdjustedFilename[128];
		sprintf(AdjustedFilename, "TILESETS/%d/%s", tileset_to_add, filename);
		if (!AddTileSurface(AdjustedFilename, uiLoop, tileset_to_add))
		{
			DestroyTileSurfaces();
			return FALSE;
		}
	}

	return( TRUE );
}


static BOOLEAN AddTileSurface(const char* Filename, UINT32 ubType, UINT8 ubTilesetID)
{
	// Delete the surface first!
	if ( gTileSurfaceArray[ ubType ] != NULL )
	{
		DeleteTileSurface( gTileSurfaceArray[ ubType ] );
		gTileSurfaceArray[ ubType ] = NULL;
	}

	TILE_IMAGERY* const TileSurf = LoadTileSurface(Filename);
	if ( TileSurf == NULL )
		return( FALSE );

	TileSurf->fType							= ubType;

	SetRaisedObjectFlag(Filename, TileSurf);

	gTileSurfaceArray[ ubType ] = TileSurf;

	// OK, if we were not the default tileset, set value indicating that!
	if ( ubTilesetID != GENERIC_1 )
	{
		gbDefaultSurfaceUsed[ ubType ] = FALSE;
	}
	else
	{
		gbDefaultSurfaceUsed[ ubType ] = TRUE;
	}

	gbNewTileSurfaceLoaded[ ubType ] = TRUE;

  return( TRUE );
}

extern BOOLEAN gfLoadShadeTablesFromTextFile;

void BuildTileShadeTables(  )
{
	UINT32					uiLoop;

#ifdef JA2TESTVERSION
	UINT32 uiStartTime = GetJA2Clock();
	uiNumImagesReloaded = 0;
#endif

	if( gfLoadShadeTablesFromTextFile )
	{ //Because we're tweaking the RGB values in the text file, always force rebuild the shadetables
		//so that the user can tweak them in the same exe session.
		memset( gbNewTileSurfaceLoaded, 1, sizeof( gbNewTileSurfaceLoaded ) );
	}

	for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++)
	{
		if ( gTileSurfaceArray[ uiLoop ] != NULL )
		{
			// Don't Create shade tables if default were already used once!
			#ifdef JA2EDITOR
				if( gbNewTileSurfaceLoaded[ uiLoop ] || gfEditorForceShadeTableRebuild )
			#else
				if( gbNewTileSurfaceLoaded[ uiLoop ]  )
      #endif
				{
					#ifdef JA2TESTVERSION
						uiNumImagesReloaded++;
					#endif
					RenderProgressBar( 0, uiLoop * 100 / NUMBEROFTILETYPES );
					CreateTilePaletteTables(gTileSurfaceArray[uiLoop]->vo);
        }
		}
	}

	#ifdef JA2TESTVERSION
		uiBuildShadeTableTime = GetJA2Clock() - uiStartTime;
	#endif
}


void DestroyTileShadeTables(void)
{
	for (UINT32 i = 0; i < NUMBEROFTILETYPES; ++i)
	{
		const TILE_IMAGERY* const ti = gTileSurfaceArray[i];
		if (ti == NULL) continue;

		// Don't delete shade tables if default are still being used...
#ifdef JA2EDITOR
		if (gbNewTileSurfaceLoaded[i] || gfEditorForceShadeTableRebuild)
#else
		if (gbNewTileSurfaceLoaded[i])
#endif
		{
			DestroyObjectPaletteTables(ti->vo);
		}
	}
}


static void DestroyTileSurfaces(void)
{
	UINT32					uiLoop;

	for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++)
	{
		if ( gTileSurfaceArray[ uiLoop ] != NULL )
		{
			DeleteTileSurface( gTileSurfaceArray[ uiLoop ] );
			gTileSurfaceArray[ uiLoop ] = NULL;
		}
	}
}


static void CompileWorldTerrainIDs(void)
{
	for (INT16 sGridNo = 0; sGridNo < WORLD_MAX; ++sGridNo)
	{
		if (!GridNoOnVisibleWorldTile(sGridNo)) continue;

		// Check if we have anything in object layer which has a terrain modifier
		const LEVELNODE* n = gpWorldLevelData[sGridNo].pObjectHead;

		if (n != NULL && giCurrentTilesetID == TEMP_19)
		{
			// ATE: CRAPOLA! Special case stuff here for the friggen pool since art was fu*ked up
			switch (n->usIndex)
			{
				case ANOTHERDEBRIS4:
				case ANOTHERDEBRIS6:
				case ANOTHERDEBRIS7:
					gpWorldLevelData[sGridNo].ubTerrainID = LOW_WATER;
					continue;
			}
		}

		if (n == NULL                   ||
				n->usIndex >= NUMBEROFTILES ||
				gTileDatabase[n->usIndex].ubTerrainID == NO_TERRAIN)
		{	// Try terrain instead!
			n = gpWorldLevelData[sGridNo].pLandHead;
		}

		const TILE_ELEMENT* const te = &gTileDatabase[n->usIndex];
		if (te->ubNumberOfTiles > 1)
		{
			for (UINT8 ubLoop = 0; ubLoop < te->ubNumberOfTiles; ++ubLoop)
			{
				const INT16 sTempGridNo = sGridNo + te->pTileLocData[ubLoop].bTileOffsetX + te->pTileLocData[ubLoop].bTileOffsetY * WORLD_COLS;
				gpWorldLevelData[sTempGridNo].ubTerrainID = te->ubTerrainID;
			}
		}
		else
		{
			gpWorldLevelData[sGridNo].ubTerrainID = te->ubTerrainID;
		}
	}
}


static void CompileTileMovementCosts(UINT16 usGridNo)
{
	UINT8						ubTerrainID;
	LEVELNODE *			pLand;

	STRUCTURE *			pStructure;
	BOOLEAN					fStructuresOnRoof;

	UINT8			ubDirLoop;

/*
*/

	if ( GridNoOnVisibleWorldTile( usGridNo ) )
	{
		// check for land of a different height in adjacent locations
		for ( ubDirLoop = 0; ubDirLoop < 8; ubDirLoop++ )
		{
			if ( gpWorldLevelData[ usGridNo ].sHeight !=
					 gpWorldLevelData[ usGridNo + DirectionInc( ubDirLoop ) ].sHeight )
			{
				SET_CURRMOVEMENTCOST( ubDirLoop, TRAVELCOST_OBSTACLE );
			}
		}

		// check for exit grids
		if ( ExitGridAtGridNo( usGridNo ) )
		{
			for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
			{
				SET_CURRMOVEMENTCOST( ubDirLoop, TRAVELCOST_EXITGRID );
			}
			// leave the roof alone, and continue, so that we can get values for the roof if traversable
		}

	}
	else
	{
		for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
		{
			SET_MOVEMENTCOST( usGridNo, ubDirLoop,  0, TRAVELCOST_OFF_MAP );
			SET_MOVEMENTCOST( usGridNo, ubDirLoop,  1, TRAVELCOST_OFF_MAP );
		}
		if (gpWorldLevelData[usGridNo].pStructureHead == NULL)
		{
			return;
		}
	}

	if (gpWorldLevelData[usGridNo].pStructureHead != NULL)
	{ // structures in tile
		// consider the land
		pLand = gpWorldLevelData[ usGridNo ].pLandHead;
		if ( pLand != NULL )
		{
			// Set TEMPORARY cost here

			// Get terrain type
			ubTerrainID =	gpWorldLevelData[usGridNo].ubTerrainID; // = GetTerrainType( (INT16)usGridNo );

			for (ubDirLoop=0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++)
			{
				SET_CURRMOVEMENTCOST( ubDirLoop, gTileTypeMovementCost[ ubTerrainID ] );
			}
		}

		// now consider all structures
		pStructure = gpWorldLevelData[usGridNo].pStructureHead;
		fStructuresOnRoof = FALSE;
		do
		{
			if (pStructure->sCubeOffset == STRUCTURE_ON_GROUND)
			{
				if (pStructure->fFlags & STRUCTURE_PASSABLE)
				{
					if (pStructure->fFlags & STRUCTURE_WIREFENCE && pStructure->fFlags & STRUCTURE_OPEN)
					{
						// prevent movement along the fence but allow in all other directions
						switch( pStructure->ubWallOrientation )
						{
							case OUTSIDE_TOP_LEFT:
							case INSIDE_TOP_LEFT:
								SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( EAST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( SOUTHEAST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( SOUTH, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_NOT_STANDING );
								break;

							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:
								SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( EAST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( SOUTHEAST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( SOUTH, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_NOT_STANDING );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_NOT_STANDING );
								break;
						}
					}
					// all other passable structures do not block movement in any way
				}
				else if (pStructure->fFlags & STRUCTURE_BLOCKSMOVES)
				{
					if ( (pStructure->fFlags & STRUCTURE_FENCE) && !(pStructure->fFlags & STRUCTURE_SPECIAL) )
					{
						// jumpable!
						switch( pStructure->ubWallOrientation )
						{
							case OUTSIDE_TOP_LEFT:
							case INSIDE_TOP_LEFT:
								// can be jumped north and south
								SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_FENCE );
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( EAST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( SOUTHEAST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( SOUTH, TRAVELCOST_FENCE );
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
								// set values for the tiles EXITED from this location
								FORCE_SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_NONE );
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								FORCE_SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_NONE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo - 1, WEST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
								break;

							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:
								// can be jumped east and west
								SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( EAST, TRAVELCOST_FENCE );
								SET_CURRMOVEMENTCOST( SOUTHEAST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( SOUTH, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_FENCE );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
								// set values for the tiles EXITED from this location
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								// make sure no obstacle costs exists before changing path cost to 0
								if ( gubWorldMovementCosts[ usGridNo + 1 ][ EAST ][ 0 ] < TRAVELCOST_BLOCKED )
								{
									FORCE_SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_NONE );
								}
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
								if ( gubWorldMovementCosts[ usGridNo - 1 ][ WEST ][ 0 ] < TRAVELCOST_BLOCKED )
								{
									FORCE_SET_MOVEMENTCOST( usGridNo - 1, WEST, 0, TRAVELCOST_NONE );
								}
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
								break;

							default:
								// corners aren't jumpable
								for (ubDirLoop=0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++)
								{
									SET_CURRMOVEMENTCOST( ubDirLoop, TRAVELCOST_OBSTACLE );
								}
								break;
						}
 					}
					else if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_SANDBAG && StructureHeight( pStructure ) < 2 )
					{
						for (ubDirLoop=0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++)
						{
							SET_CURRMOVEMENTCOST( ubDirLoop, TRAVELCOST_OBSTACLE );
						}

						if ( FindStructure( (UINT16) (usGridNo - WORLD_COLS), STRUCTURE_OBSTACLE ) == FALSE && FindStructure( (UINT16)(usGridNo + WORLD_COLS), STRUCTURE_OBSTACLE ) == FALSE )
						{
							FORCE_SET_MOVEMENTCOST( usGridNo, NORTH, 0, TRAVELCOST_FENCE );
							FORCE_SET_MOVEMENTCOST( usGridNo, SOUTH, 0, TRAVELCOST_FENCE );
						}

						if ( FindStructure( (UINT16)(usGridNo - 1), STRUCTURE_OBSTACLE ) == FALSE && FindStructure( (UINT16)(usGridNo + 1), STRUCTURE_OBSTACLE ) == FALSE )
						{
							FORCE_SET_MOVEMENTCOST( usGridNo, EAST, 0, TRAVELCOST_FENCE );
							FORCE_SET_MOVEMENTCOST( usGridNo, WEST, 0, TRAVELCOST_FENCE );
						}
					}
					else if ( (pStructure->fFlags & STRUCTURE_CAVEWALL ) )
          {
						for (ubDirLoop=0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++)
						{
							SET_CURRMOVEMENTCOST( ubDirLoop, TRAVELCOST_CAVEWALL );
						}
          }
					else
					{
						for (ubDirLoop=0; ubDirLoop < NUM_WORLD_DIRECTIONS; ubDirLoop++)
						{
							SET_CURRMOVEMENTCOST( ubDirLoop, TRAVELCOST_OBSTACLE );
						}
					}
				}
				else if (pStructure->fFlags & STRUCTURE_ANYDOOR) /*&& (pStructure->fFlags & STRUCTURE_OPEN))*/
				{ // NB closed doors are treated just like walls, in the section after this

					if (pStructure->fFlags & STRUCTURE_DDOOR_LEFT && (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT) )
					{
						// double door, left side (as you look on the screen)
						switch( pStructure->ubWallOrientation )
						{
							case OUTSIDE_TOP_RIGHT:
								if (pStructure->fFlags & STRUCTURE_BASE_TILE)
								{ // doorpost
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
									SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
									SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_WALL );
									// corner
									SET_MOVEMENTCOST( usGridNo + 1 + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL );
								}
								else
								{	// door
									SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_OPEN_W );
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_DOOR_OPEN_W );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_OPEN_NW );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_NW );
									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_W_W );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_NW_W );
								}
								break;

							case INSIDE_TOP_RIGHT:
								// doorpost
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
								SET_MOVEMENTCOST( usGridNo + 1,NORTHEAST, 0, TRAVELCOST_WALL );
								// door
								SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_OPEN_HERE );
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_DOOR_OPEN_HERE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_OPEN_N );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_N );
								SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_E );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_NE );
								break;

							default:
								// door with no orientation specified!?
								break;
						}
					}
					else if (pStructure->fFlags & STRUCTURE_DDOOR_RIGHT && (pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT) )
					{
						// double door, right side (as you look on the screen)
						switch( pStructure->ubWallOrientation )
						{
							case OUTSIDE_TOP_LEFT:
								if (pStructure->fFlags & STRUCTURE_BASE_TILE)
								{	// doorpost
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
									SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N )
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL );									;
									// corner
									SET_MOVEMENTCOST( usGridNo + 1 ,NORTHEAST, 0, TRAVELCOST_WALL );
								}
								else
								{ // door
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_DOOR_OPEN_N );
									SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_OPEN_N );
									SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_OPEN_NW );
									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_NW );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_N_N );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_NW_N );
								}
								break;

							case INSIDE_TOP_LEFT:
								// doorpost
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL );
								// corner
								SET_MOVEMENTCOST( usGridNo + 1 ,NORTHEAST, 0, TRAVELCOST_WALL );
								// door
								SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_OPEN_HERE );
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_DOOR_OPEN_HERE );
								SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_OPEN_W );
								SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_W );
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_S );
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_SW );
								break;
							default:
								// door with no orientation specified!?
								break;
						}
					}
					else if (pStructure->fFlags & STRUCTURE_SLIDINGDOOR && pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1)
					{
						switch( pStructure->ubWallOrientation )
						{
							case OUTSIDE_TOP_LEFT:
							case INSIDE_TOP_LEFT:
								// doorframe post in one corner of each of the tiles
								if (pStructure->fFlags & STRUCTURE_BASE_TILE)
								{
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
									SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_DOOR_CLOSED_N );

								}
								else
								{
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_DOOR_CLOSED_N);
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N);
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL );

								}
								break;
							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:
								// doorframe post in one corner of each of the tiles
								if (pStructure->fFlags & STRUCTURE_BASE_TILE)
								{
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
									SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_DOOR_CLOSED_HERE );

									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
									SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_CLOSED_W );
								}
								else
								{
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_WALL );

									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_DOOR_CLOSED_W );
									SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
									SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_WALL );
								}
								break;
						}
					}
					else
					{
						// standard door
						switch( pStructure->ubWallOrientation )
						{
							case OUTSIDE_TOP_LEFT:
								if (pStructure->fFlags & STRUCTURE_BASE_TILE)
								{	// doorframe
									SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
									SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );

									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL );

									// DO CORNERS
									SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_WALL );


									//SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
									//SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
									//SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
									//SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
									// corner
									//SET_MOVEMENTCOST( usGridNo + 1 ,NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								}
								else if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR))
								{ // door
									SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
									SET_CURRMOVEMENTCOST( EAST, TRAVELCOST_DOOR_OPEN_N );
									SET_MOVEMENTCOST( usGridNo - 1, WEST, 0, TRAVELCOST_DOOR_OPEN_NE );
									SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_WALL );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_N_N );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_NE_N );
								}
								break;

							case INSIDE_TOP_LEFT:
								SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
								SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );

								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );

								// DO CORNERS
								SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );

								// doorframe
								//SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
								//SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
								//SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								//SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
								// corner
								//SET_MOVEMENTCOST( usGridNo + 1 ,NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								// door
								if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR))
								{
									SET_CURRMOVEMENTCOST( EAST, TRAVELCOST_DOOR_OPEN_HERE );
									SET_CURRMOVEMENTCOST( SOUTHEAST, TRAVELCOST_DOOR_OPEN_HERE );
									SET_MOVEMENTCOST( usGridNo - 1, WEST, 0, TRAVELCOST_DOOR_OPEN_E );
									SET_MOVEMENTCOST( usGridNo - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_E );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_S );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS - 1, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_SE );
								}
								break;

							case OUTSIDE_TOP_RIGHT:
								if (pStructure->fFlags & STRUCTURE_BASE_TILE)
								{ // doorframe
									SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
									SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
									SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );

									SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
									SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
									SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );

									// DO CORNERS
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
									SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );

									//SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
									//SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
									//SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
									//SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
									// corner
									//SET_MOVEMENTCOST( usGridNo + 1 + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								}
								else if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR))
								{	// door
									SET_CURRMOVEMENTCOST( SOUTH, TRAVELCOST_DOOR_OPEN_W );
									SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_DOOR_OPEN_W );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_DOOR_OPEN_SW );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_SW );
									SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_DOOR_OPEN_W_W );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_SW_W );
								}
								break;

							case INSIDE_TOP_RIGHT:
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );

								SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
								SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );

								// DO CORNERS
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );

								// doorframe
								/*
								SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
								SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + 1,SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + 1,NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								// corner
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS,  NORTHWEST, 0, TRAVELCOST_OBSTACLE );
								*/
								if (!(pStructure->fFlags & STRUCTURE_SLIDINGDOOR))
								{
									// door
									SET_CURRMOVEMENTCOST( SOUTH, TRAVELCOST_DOOR_OPEN_HERE );
									SET_CURRMOVEMENTCOST( SOUTHEAST, TRAVELCOST_DOOR_OPEN_HERE );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_DOOR_OPEN_S );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHEAST, 0, TRAVELCOST_DOOR_OPEN_S );
									SET_MOVEMENTCOST( usGridNo - 1, SOUTHWEST, 0, TRAVELCOST_DOOR_OPEN_E );
									SET_MOVEMENTCOST( usGridNo - WORLD_COLS - 1, NORTHWEST, 0, TRAVELCOST_DOOR_OPEN_SE );
								}
								break;

							default:
								// door with no orientation specified!?
								break;
						}
					}

					/*
					switch( pStructure->ubWallOrientation )
					{
						case OUTSIDE_TOP_LEFT:
						case INSIDE_TOP_LEFT:
							SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_OBSTACLE );
							SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_DOOR_CLOSED_HERE );
							SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );

							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_DOOR_CLOSED_N );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );

							// DO CORNERS
							SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
							break;

						case OUTSIDE_TOP_RIGHT:
						case INSIDE_TOP_RIGHT:
							SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_OBSTACLE );
							SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_DOOR_CLOSED_HERE );
							SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_OBSTACLE );

							SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_DOOR_CLOSED_W );
							SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );

							// DO CORNERS
							SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_OBSTACLE );
							break;

						default:
							// wall with no orientation specified!?
							break;
					}
					*/


				}
				else if (pStructure->fFlags & STRUCTURE_WALLSTUFF )
				{
					//ATE: IF a closed door, set to door value
					switch( pStructure->ubWallOrientation )
					{
						case OUTSIDE_TOP_LEFT:
						case INSIDE_TOP_LEFT:
							SET_CURRMOVEMENTCOST( NORTHEAST, TRAVELCOST_WALL );
							SET_CURRMOVEMENTCOST( NORTH, TRAVELCOST_WALL );
							SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHEAST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL );

							// DO CORNERS
							SET_MOVEMENTCOST( usGridNo - 1, NORTHWEST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS - 1, SOUTHWEST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_WALL );
							break;

						case OUTSIDE_TOP_RIGHT:
						case INSIDE_TOP_RIGHT:
							SET_CURRMOVEMENTCOST( SOUTHWEST, TRAVELCOST_WALL );
							SET_CURRMOVEMENTCOST( WEST, TRAVELCOST_WALL );
							SET_CURRMOVEMENTCOST( NORTHWEST, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + 1, SOUTHEAST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + 1, NORTHEAST, 0, TRAVELCOST_WALL );

							// DO CORNERS
							SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo - WORLD_COLS, NORTHWEST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_WALL );
							SET_MOVEMENTCOST( usGridNo + WORLD_COLS, SOUTHWEST, 0, TRAVELCOST_WALL );
							break;

						default:
							// wall with no orientation specified!?
							break;
					}
				}
			}
			else
			{
				if (!(pStructure->fFlags & STRUCTURE_PASSABLE || pStructure->fFlags & STRUCTURE_NORMAL_ROOF))
				{
					fStructuresOnRoof = TRUE;
				}
			}
			pStructure = pStructure->pNext;
		} while (pStructure != NULL);

		// HIGHEST LAYER
		if ((gpWorldLevelData[ usGridNo ].pRoofHead != NULL))
		{
			if (!fStructuresOnRoof)
			{
				for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
				{
					SET_MOVEMENTCOST( usGridNo, ubDirLoop,  1, TRAVELCOST_FLAT );
				}
			}
			else
			{
				for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
				{
					SET_MOVEMENTCOST( usGridNo, ubDirLoop,  1, TRAVELCOST_OBSTACLE );
				}
			}
		}
		else
		{
			for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
			{
				SET_MOVEMENTCOST( usGridNo, ubDirLoop,  1, TRAVELCOST_OBSTACLE );
			}
		}
	}
	else
	{ // NO STRUCTURES IN TILE
		// consider just the land

		// Get terrain type
		ubTerrainID =	gpWorldLevelData[usGridNo].ubTerrainID; // = GetTerrainType( (INT16)usGridNo );
		for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
		{
			SET_MOVEMENTCOST( usGridNo ,ubDirLoop, 0, gTileTypeMovementCost[ ubTerrainID ] );
		}

/*
		pLand = gpWorldLevelData[ usGridNo ].pLandHead;
		if ( pLand != NULL )
		{
			// Set cost here

			// Get terrain type
			ubTerrainID =	GetTerrainType( (INT16)usGridNo );

			for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
			{
				SET_MOVEMENTCOST( usGridNo ,ubDirLoop, 0, gTileTypeMovementCost[ ubTerrainID ] );
			}
		}
*/
		// HIGHEST LEVEL
		if (gpWorldLevelData[ usGridNo ].pRoofHead != NULL)
		{
			for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
			{
				SET_MOVEMENTCOST( usGridNo, ubDirLoop,  1, TRAVELCOST_FLAT );
			}
		}
		else
		{
			for (ubDirLoop=0; ubDirLoop < 8; ubDirLoop++)
			{
				SET_MOVEMENTCOST( usGridNo, ubDirLoop,  1, TRAVELCOST_OBSTACLE );
			}
		}
	}
}

#define LOCAL_RADIUS 4

void RecompileLocalMovementCosts( INT16 sCentreGridNo )
{
	INT16		usGridNo;
	INT16		sGridX, sGridY;
	INT16		sCentreGridX, sCentreGridY;
	INT8		bDirLoop;

	ConvertGridNoToXY( sCentreGridNo, &sCentreGridX, &sCentreGridY );
	for( sGridY = sCentreGridY - LOCAL_RADIUS; sGridY < sCentreGridY + LOCAL_RADIUS; sGridY++ )
	{
		for( sGridX = sCentreGridX - LOCAL_RADIUS; sGridX < sCentreGridX + LOCAL_RADIUS; sGridX++ )
		{
			usGridNo = MAPROWCOLTOPOS( sGridY, sGridX );
			// times 2 for 2 levels, times 2 for UINT16s
//			memset( &(gubWorldMovementCosts[usGridNo]), 0, MAXDIR * 2 * 2 );
			if (usGridNo < WORLD_MAX)
			{
				for( bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++)
				{
					gubWorldMovementCosts[usGridNo][bDirLoop][0] = 0;
					gubWorldMovementCosts[usGridNo][bDirLoop][1] = 0;
				}
			}
		}
	}

	// note the radius used in this loop is larger, to guarantee that the
	// edges of the recompiled areas are correct (i.e. there could be spillover)
	for( sGridY = sCentreGridY - LOCAL_RADIUS - 1; sGridY < sCentreGridY + LOCAL_RADIUS + 1; sGridY++ )
	{
		for( sGridX = sCentreGridX - LOCAL_RADIUS - 1; sGridX < sCentreGridX + LOCAL_RADIUS + 1; sGridX++ )
		{
			usGridNo = MAPROWCOLTOPOS( sGridY, sGridX );
			if (usGridNo < WORLD_MAX)
			{
				CompileTileMovementCosts( usGridNo );
			}
		}
	}
}


void RecompileLocalMovementCostsFromRadius( INT16 sCentreGridNo, INT8 bRadius )
{
	INT16		usGridNo;
	INT16		sGridX, sGridY;
	INT16		sCentreGridX, sCentreGridY;
	INT8		bDirLoop;

	ConvertGridNoToXY( sCentreGridNo, &sCentreGridX, &sCentreGridY );
	if (bRadius == 0)
	{
		// one tile check only
		for( bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++)
		{
			gubWorldMovementCosts[sCentreGridNo][bDirLoop][0] = 0;
			gubWorldMovementCosts[sCentreGridNo][bDirLoop][1] = 0;
		}
		CompileTileMovementCosts( sCentreGridNo );
	}
	else
	{
		for( sGridY = sCentreGridY - bRadius; sGridY < sCentreGridY + bRadius; sGridY++ )
		{
			for( sGridX = sCentreGridX - bRadius; sGridX < sCentreGridX + bRadius; sGridX++ )
			{
				usGridNo = MAPROWCOLTOPOS( sGridY, sGridX );
				// times 2 for 2 levels, times 2 for UINT16s
	//			memset( &(gubWorldMovementCosts[usGridNo]), 0, MAXDIR * 2 * 2 );
				if (usGridNo < WORLD_MAX)
				{
					for( bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++)
					{
						gubWorldMovementCosts[usGridNo][bDirLoop][0] = 0;
						gubWorldMovementCosts[usGridNo][bDirLoop][1] = 0;
					}
				}
			}
		}

		// note the radius used in this loop is larger, to guarantee that the
		// edges of the recompiled areas are correct (i.e. there could be spillover)
		for( sGridY = sCentreGridY - bRadius - 1; sGridY < sCentreGridY + bRadius + 1; sGridY++ )
		{
			for( sGridX = sCentreGridX - bRadius - 1; sGridX < sCentreGridX + bRadius + 1; sGridX++ )
			{
				usGridNo = MAPROWCOLTOPOS( sGridY, sGridX );
				if (usGridNo < WORLD_MAX)
				{
					CompileTileMovementCosts( usGridNo );
				}
			}
		}
	}
}

void AddTileToRecompileArea( INT16 sGridNo )
{
	INT16	sCheckGridNo;
	INT16	sCheckX;
	INT16 sCheckY;

	// Set flag to wipe and recompile MPs in this tile
	if (sGridNo < 0 || sGridNo >= WORLD_MAX)
	{
		return;
	}

	gpWorldLevelData[ sGridNo ].ubExtFlags[0] |= MAPELEMENT_EXT_RECALCULATE_MOVEMENT;

	// check Top/Left of recompile region
	sCheckGridNo = NewGridNo( sGridNo, DirectionInc( NORTHWEST ) );
	sCheckX = sCheckGridNo % WORLD_COLS;
	sCheckY = sCheckGridNo / WORLD_COLS;
	if ( sCheckX < gsRecompileAreaLeft )
	{
		gsRecompileAreaLeft = sCheckX;
	}
	if ( sCheckY < gsRecompileAreaTop )
	{
		gsRecompileAreaTop = sCheckY;
	}

	// check Bottom/Right
	sCheckGridNo = NewGridNo( sGridNo, DirectionInc( SOUTHEAST ) );
	sCheckX = sCheckGridNo % WORLD_COLS;
	sCheckY = sCheckGridNo / WORLD_COLS;
	if ( sCheckX > gsRecompileAreaRight )
	{
		gsRecompileAreaRight = sCheckX;
	}
	if ( sCheckY > gsRecompileAreaBottom )
	{
		gsRecompileAreaBottom = sCheckY;
	}
}

void RecompileLocalMovementCostsInAreaWithFlags( void )
{
	INT16		usGridNo;
	INT16		sGridX, sGridY;
	INT8		bDirLoop;

	for( sGridY = gsRecompileAreaTop; sGridY <= gsRecompileAreaBottom; sGridY++ )
	{
		for( sGridX = gsRecompileAreaLeft; sGridX < gsRecompileAreaRight; sGridX++ )
		{
			usGridNo = MAPROWCOLTOPOS( sGridY, sGridX );
			if ( usGridNo < WORLD_MAX && gpWorldLevelData[ usGridNo ].ubExtFlags[0] & MAPELEMENT_EXT_RECALCULATE_MOVEMENT )
			{
				// wipe MPs in this tile!
				for( bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++)
				{
					gubWorldMovementCosts[usGridNo][bDirLoop][0] = 0;
					gubWorldMovementCosts[usGridNo][bDirLoop][1] = 0;
				}
				// reset flag
				gpWorldLevelData[ usGridNo ].ubExtFlags[0] &= (~MAPELEMENT_EXT_RECALCULATE_MOVEMENT);
			}
		}
	}

	for( sGridY = gsRecompileAreaTop; sGridY <= gsRecompileAreaBottom; sGridY++ )
	{
		for( sGridX = gsRecompileAreaLeft; sGridX <= gsRecompileAreaRight; sGridX++ )
		{
			usGridNo = MAPROWCOLTOPOS( sGridY, sGridX );
			if (usGridNo < WORLD_MAX)
			{
				CompileTileMovementCosts( usGridNo );
			}
		}
	}
}

void RecompileLocalMovementCostsForWall( INT16 sGridNo, UINT8 ubOrientation )
{
	INT8		bDirLoop;
	INT16		sUp, sDown, sLeft, sRight;
	INT16		sX, sY, sTempGridNo;

	switch( ubOrientation )
	{
		case OUTSIDE_TOP_RIGHT:
		case INSIDE_TOP_RIGHT:
			sUp = -1;
			sDown = 1;
			sLeft = 0;
			sRight = 1;
			break;
		case OUTSIDE_TOP_LEFT:
		case INSIDE_TOP_LEFT:
			sUp = 0;
			sDown = 1;
			sLeft = -1;
			sRight = 1;
			break;
		default:
			return;
	}

	for ( sY = sUp; sY <= sDown; sY++ )
	{
		for ( sX = sLeft; sX <= sRight; sX++ )
		{
			sTempGridNo = sGridNo + sX + sY * WORLD_COLS;
			for( bDirLoop = 0; bDirLoop < MAXDIR; bDirLoop++)
			{
				gubWorldMovementCosts[sTempGridNo][bDirLoop][0] = 0;
				gubWorldMovementCosts[sTempGridNo][bDirLoop][1] = 0;
			}

			CompileTileMovementCosts( sTempGridNo );
		}
	}
}



// GLOBAL WORLD MANIPULATION FUNCTIONS
void CompileWorldMovementCosts( )
{
	UINT16					usGridNo;

	memset( gubWorldMovementCosts, 0, sizeof( gubWorldMovementCosts ) );

	CompileWorldTerrainIDs();
 	for( usGridNo = 0; usGridNo < WORLD_MAX; usGridNo++ )
	{
		CompileTileMovementCosts( usGridNo );
	}
}


#ifdef JA2EDITOR

static void RemoveWorldWireFrameTiles(void);
static void SaveMapLights(HWFILE hfile);


// SAVING CODE
BOOLEAN SaveWorld(const char *puiFilename)
{
	INT32			cnt;
	UINT32		uiSoldierSize;
	UINT32		uiFlags;
	UINT32		uiNumWarningsCaught = 0;
	HWFILE		hfile;
	LEVELNODE	*pLand;
	LEVELNODE	*pObject;
	LEVELNODE	*pStruct;
	LEVELNODE	*pShadow;
	LEVELNODE	*pRoof;
	LEVELNODE	*pOnRoof;
	LEVELNODE	*pTailLand=NULL;
	UINT16		usNumExitGrids = 0;
	UINT16    usTypeSubIndex;
	UINT8			LayerCount;
	UINT8			ObjectCount;
	UINT8			StructCount;
	UINT8			ShadowCount;
	UINT8			RoofCount;
	UINT8			OnRoofCount;
	UINT8			ubType;
	UINT8			ubTypeSubIndex;
	UINT8			ubTest = 1;
	CHAR8			aFilename[ 255 ];
	UINT8			ubCombine;
	UINT8			bCounts[ WORLD_MAX ][8];

	sprintf( aFilename, "MAPS/%s", puiFilename );

	// Open file
	hfile = FileOpen(aFilename, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);

	if ( !hfile )
	{
		return( FALSE );
	}

	// Write JA2 Version ID
	FileWrite(hfile, &gdMajorMapVersion, sizeof(FLOAT));
	if( gdMajorMapVersion >= 4.00 )
	{
		FileWrite(hfile, &gubMinorMapVersion, sizeof(UINT8));
	}

	// Write FLAGS FOR WORLD
	//uiFlags = MAP_WORLDONLY_SAVED;
	uiFlags = 0;
	uiFlags |= MAP_FULLSOLDIER_SAVED;
	uiFlags |= MAP_EXITGRIDS_SAVED;
	uiFlags |= MAP_WORLDLIGHTS_SAVED;
	uiFlags |= MAP_DOORTABLE_SAVED;
	uiFlags |= MAP_WORLDITEMS_SAVED;
	uiFlags |= MAP_EDGEPOINTS_SAVED;
	if( gfBasement || gfCaves )
		uiFlags |= MAP_AMBIENTLIGHTLEVEL_SAVED;
	uiFlags |= MAP_NPCSCHEDULES_SAVED;

	FileWrite(hfile, &uiFlags, sizeof(INT32));

	// Write tileset ID
	FileWrite(hfile, &giCurrentTilesetID, sizeof(INT32));

	// Write SOLDIER CONTROL SIZE
	uiSoldierSize = sizeof( SOLDIERTYPE );
	FileWrite(hfile, &uiSoldierSize, sizeof(INT32));


	// REMOVE WORLD VISIBILITY TILES
	RemoveWorldWireFrameTiles( );


	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Write out height values
		FileWrite(hfile, &gpWorldLevelData[cnt].sHeight, sizeof(INT16));
	}

	// Write out # values - we'll have no more than 15 per level!
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Determine number of land
		pLand = gpWorldLevelData[ cnt ].pLandHead;
		LayerCount = 0;

		while( pLand != NULL )
		{
			LayerCount++;
			pLand = pLand->pNext;
		}
		if( LayerCount > 15 )
		{
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"SAVE ABORTED!  Land count too high (%d) for gridno %d."
				L"  Need to fix before map can be saved!  There are %d additional warnings.",
				LayerCount, cnt, uiNumWarningsCaught );
			gfErrorCatch = TRUE;
			FileClose( hfile );
			return FALSE;
		}
		if( LayerCount > 10 )
		{
			uiNumWarningsCaught++;
			gfErrorCatch = TRUE;
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"Warnings %d -- Last warning:  Land count warning of %d for gridno %d.",
				uiNumWarningsCaught, LayerCount, cnt );
		}
		bCounts[ cnt ][ 0 ] = LayerCount;

		// Combine # of land layers with worlddef flags ( first 4 bits )
		ubCombine = (UINT8)( ( LayerCount&0xf ) | ( (gpWorldLevelData[ cnt ].uiFlags&0xf)<<4 ) );
		// Write combination
		FileWrite(hfile, &ubCombine, sizeof(ubCombine));


		// Determine # of objects
		pObject = gpWorldLevelData[ cnt ].pObjectHead;
		ObjectCount = 0;
		while( pObject != NULL )
		{
			// DON'T WRITE ANY ITEMS
			if ( !(pObject->uiFlags & ( LEVELNODE_ITEM ) ) )
			{
				//Make sure this isn't a UI Element
				const UINT32 uiTileType = GetTileType(pObject->usIndex);
				if( uiTileType < FIRSTPOINTERS )
					ObjectCount++;
			}
			pObject = pObject->pNext;
		}
		if( ObjectCount > 15 )
		{
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"SAVE ABORTED!  Object count too high (%d) for gridno %d."
				L"  Need to fix before map can be saved!  There are %d additional warnings.",
				ObjectCount, cnt, uiNumWarningsCaught );
			gfErrorCatch = TRUE;
			FileClose( hfile );
			return FALSE;
		}
		if( ObjectCount > 10 )
		{
			uiNumWarningsCaught++;
			gfErrorCatch = TRUE;
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"Warnings %d -- Last warning:  Object count warning of %d for gridno %d.",
				uiNumWarningsCaught, ObjectCount, cnt );
		}
		bCounts[ cnt ][ 1 ] = ObjectCount;

		// Determine # of structs
		pStruct = gpWorldLevelData[ cnt ].pStructHead;
		StructCount = 0;
		while( pStruct != NULL )
		{
			// DON'T WRITE ANY ITEMS
			if ( !(pStruct->uiFlags & ( LEVELNODE_ITEM ) ) )
			{
				StructCount++;
			}
			pStruct = pStruct->pNext;
		}
		if( StructCount > 15 )
		{
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"SAVE ABORTED!  Struct count too high (%d) for gridno %d."
				L"  Need to fix before map can be saved!  There are %d additional warnings.",
				StructCount, cnt, uiNumWarningsCaught );
			gfErrorCatch = TRUE;
			FileClose( hfile );
			return FALSE;
		}
		if( StructCount > 10 )
		{
			uiNumWarningsCaught++;
			gfErrorCatch = TRUE;
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"Warnings %d -- Last warning:  Struct count warning of %d for gridno %d.",
				uiNumWarningsCaught, StructCount, cnt );
		}
		bCounts[ cnt ][ 2 ] = StructCount;

		ubCombine = (UINT8)( ( ObjectCount&0xf ) | ( (StructCount&0xf)<<4 ) );
		// Write combination
		FileWrite(hfile, &ubCombine, sizeof(ubCombine));


		// Determine # of shadows
		pShadow = gpWorldLevelData[ cnt ].pShadowHead;
		ShadowCount = 0;
		while( pShadow != NULL )
		{
			// Don't write any shadowbuddys or exit grids
			if ( !(pShadow->uiFlags & ( LEVELNODE_BUDDYSHADOW  | LEVELNODE_EXITGRID ) ) )
			{
				ShadowCount++;
			}
			pShadow = pShadow->pNext;
		}
		if( ShadowCount > 15 )
		{
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"SAVE ABORTED!  Shadow count too high (%d) for gridno %d."
				L"  Need to fix before map can be saved!  There are %d additional warnings.",
				ShadowCount, cnt, uiNumWarningsCaught );
			gfErrorCatch = TRUE;
			FileClose( hfile );
			return FALSE;
		}
		if( ShadowCount > 10 )
		{
			uiNumWarningsCaught++;
			gfErrorCatch = TRUE;
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"Warnings %d -- Last warning:  Shadow count warning of %d for gridno %d.",
				uiNumWarningsCaught, ShadowCount, cnt );
		}
		bCounts[ cnt ][ 3 ] = ShadowCount;

		// Determine # of Roofs
		pRoof = gpWorldLevelData[ cnt ].pRoofHead;
		RoofCount = 0;
		while( pRoof != NULL )
		{
			// ATE: Don't save revealed roof info...
			if ( pRoof->usIndex != SLANTROOFCEILING1 )
			{
				RoofCount++;
			}
			pRoof = pRoof->pNext;
		}
		if( RoofCount > 15 )
		{
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"SAVE ABORTED!  Roof count too high (%d) for gridno %d."
				L"  Need to fix before map can be saved!  There are %d additional warnings.",
				RoofCount, cnt, uiNumWarningsCaught );
			gfErrorCatch = TRUE;
			FileClose( hfile );
			return FALSE;
		}
		if( RoofCount > 10 )
		{
			uiNumWarningsCaught++;
			gfErrorCatch = TRUE;
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"Warnings %d -- Last warning:  Roof count warning of %d for gridno %d.",
				uiNumWarningsCaught, RoofCount, cnt );
		}
		bCounts[ cnt ][ 4 ] = RoofCount;

		ubCombine = (UINT8)( ( ShadowCount&0xf ) | ( (RoofCount&0xf)<<4 ) );
		// Write combination
		FileWrite(hfile, &ubCombine, sizeof(ubCombine));


		// Write OnRoof layer
		// Determine # of OnRoofs
		pOnRoof = gpWorldLevelData[ cnt ].pOnRoofHead;
		OnRoofCount = 0;

		while( pOnRoof != NULL )
		{
			OnRoofCount++;
			pOnRoof = pOnRoof->pNext;
		}
		if( OnRoofCount > 15 )
		{
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"SAVE ABORTED!  OnRoof count too high (%d) for gridno %d."
				L"  Need to fix before map can be saved!  There are %d additional warnings.",
				OnRoofCount, cnt, uiNumWarningsCaught );
			gfErrorCatch = TRUE;
			FileClose( hfile );
			return FALSE;
		}
		if( OnRoofCount > 10 )
		{
			uiNumWarningsCaught++;
			gfErrorCatch = TRUE;
			swprintf(gzErrorCatchString, lengthof(gzErrorCatchString), L"Warnings %d -- Last warning:  OnRoof count warning of %d for gridno %d.",
				uiNumWarningsCaught, OnRoofCount, cnt );
		}
		bCounts[ cnt ][ 5 ] = RoofCount;


		// Write combination of onroof and nothing...
		ubCombine = (UINT8)( ( OnRoofCount&0xf ) );
		// Write combination
		FileWrite(hfile, &ubCombine, sizeof(ubCombine));



	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {

		if ( bCounts[ cnt ][ 0 ] == 0 )
		{
			FileWrite(hfile, &ubTest, sizeof(UINT8));
			FileWrite(hfile, &ubTest, sizeof(UINT8));
		}
		else
		{

			// Write land layers
			// Write out land peices backwards so that they are loaded properly
			pLand = gpWorldLevelData[ cnt ].pLandHead;
			// GET TAIL
			while( pLand != NULL )
			{
				pTailLand = pLand;
				pLand = pLand->pNext;
			}

			while( pTailLand != NULL )
			{
				// Write out object type and sub-index
				const UINT32 uiType = GetTileType(pTailLand->usIndex);
				ubType = (UINT8)uiType;
				GetTypeSubIndexFromTileIndexChar( uiType, pTailLand->usIndex, &ubTypeSubIndex );
				FileWrite(hfile, &ubType, sizeof(UINT8));
				FileWrite(hfile, &ubTypeSubIndex, sizeof(UINT8));

				pTailLand = pTailLand->pPrevNode;
			}
		}
	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {

		// Write object layer
		pObject = gpWorldLevelData[ cnt ].pObjectHead;
		while( pObject != NULL )
		{
			// DON'T WRITE ANY ITEMS
			if ( !(pObject->uiFlags & ( LEVELNODE_ITEM ) ) )
			{
				// Write out object type and sub-index
				const UINT32 uiType = GetTileType(pObject->usIndex);
				//Make sure this isn't a UI Element
				if( uiType < FIRSTPOINTERS )
				{
					//We are writing 2 bytes for the type subindex in the object layer because the
					//ROADPIECES slot contains more than 256 subindices.
					ubType = (UINT8)uiType;
					GetTypeSubIndexFromTileIndex( uiType, pObject->usIndex, &usTypeSubIndex );
					FileWrite(hfile, &ubType, sizeof(UINT8));
					FileWrite(hfile, &usTypeSubIndex, sizeof(UINT16));
				}
			}
			pObject = pObject->pNext;
		}
	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Write struct layer
		pStruct = gpWorldLevelData[ cnt ].pStructHead;
		while( pStruct != NULL )
		{
			// DON'T WRITE ANY ITEMS
			if ( !(pStruct->uiFlags & ( LEVELNODE_ITEM ) ) )
			{
				// Write out object type and sub-index
				const UINT32 uiType = GetTileType(pStruct->usIndex);
				ubType = (UINT8)uiType;
				GetTypeSubIndexFromTileIndexChar( uiType, pStruct->usIndex, &ubTypeSubIndex );
					FileWrite(hfile, &ubType, sizeof(UINT8));
					FileWrite(hfile, &ubTypeSubIndex, sizeof(UINT8));
			}

			pStruct = pStruct->pNext;
		}
	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Write shadows
		pShadow = gpWorldLevelData[ cnt ].pShadowHead;
		while( pShadow != NULL )
		{
			// Dont't write any buddys or exit grids
			if ( !(pShadow->uiFlags & ( LEVELNODE_BUDDYSHADOW | LEVELNODE_EXITGRID ) ) )
			{
				// Write out object type and sub-index
				// Write out object type and sub-index
				const UINT32 uiType = GetTileType(pShadow->usIndex);
				ubType = (UINT8)uiType;
				GetTypeSubIndexFromTileIndexChar( uiType, pShadow->usIndex, &ubTypeSubIndex );
				FileWrite(hfile, &ubType, sizeof(UINT8));
				FileWrite(hfile, &ubTypeSubIndex, sizeof(UINT8));

			}
			else if( pShadow->uiFlags & LEVELNODE_EXITGRID )
			{	//count the number of exitgrids
				usNumExitGrids++;
			}

			pShadow = pShadow->pNext;
		}
	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		pRoof = gpWorldLevelData[ cnt ].pRoofHead;
		while( pRoof != NULL )
		{
			// ATE: Don't save revealed roof info...
			if ( pRoof->usIndex != SLANTROOFCEILING1 )
			{
				// Write out object type and sub-index
				const UINT32 uiType = GetTileType(pRoof->usIndex);
				ubType = (UINT8)uiType;
				GetTypeSubIndexFromTileIndexChar( uiType, pRoof->usIndex, &ubTypeSubIndex );
				FileWrite(hfile, &ubType, sizeof(UINT8));
				FileWrite(hfile, &ubTypeSubIndex, sizeof(UINT8));
			}

			pRoof = pRoof->pNext;
		}
	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Write OnRoofs
		pOnRoof = gpWorldLevelData[ cnt ].pOnRoofHead;
		while( pOnRoof != NULL )
		{
			// Write out object type and sub-index
			const UINT32 uiType = GetTileType(pOnRoof->usIndex);
			ubType = (UINT8)uiType;
			GetTypeSubIndexFromTileIndexChar( uiType, pOnRoof->usIndex, &ubTypeSubIndex );
			FileWrite(hfile, &ubType, sizeof(UINT8));
			FileWrite(hfile, &ubTypeSubIndex, sizeof(UINT8));


			pOnRoof = pOnRoof->pNext;
		}
	}

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Write out room information
		FileWrite(hfile, &gubWorldRoomInfo[cnt], sizeof(INT8));

	}

	if ( uiFlags & MAP_WORLDITEMS_SAVED )
	{
		// Write out item information
		SaveWorldItemsToMap( hfile );
	}

	if( uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED )
	{
		FileWrite(hfile, &gfBasement, 1);
		FileWrite(hfile, &gfCaves, 1);
		FileWrite(hfile, &ubAmbientLightLevel, 1);
	}

	if( uiFlags & MAP_WORLDLIGHTS_SAVED )
	{

		SaveMapLights( hfile );
	}

	SaveMapInformation( hfile );

	if( uiFlags & MAP_FULLSOLDIER_SAVED )
	{
		SaveSoldiersToMap( hfile );
	}
	if( uiFlags & MAP_EXITGRIDS_SAVED )
	{
		SaveExitGrids( hfile, usNumExitGrids );
	}
	if( uiFlags & MAP_DOORTABLE_SAVED )
	{
		SaveDoorTableToMap( hfile );
	}
	if( uiFlags & MAP_EDGEPOINTS_SAVED )
	{
		CompileWorldMovementCosts();
		GenerateMapEdgepoints();
		SaveMapEdgepoints( hfile );
	}
	if( uiFlags & MAP_NPCSCHEDULES_SAVED )
	{
		SaveSchedules( hfile );
	}

	FileClose( hfile );

	strlcpy(g_filename, puiFilename, lengthof(g_filename));

	return( TRUE );
}

#endif


#define NUM_DIR_SEARCHES				5
INT8	bDirectionsForShadowSearch[ NUM_DIR_SEARCHES ] =
{
	 WEST,
	 SOUTHWEST,
	 SOUTH,
	 SOUTHEAST,
	 EAST
};


static void OptimizeMapForShadows(void)
{
	INT32 cnt, dir;
	INT16 sNewGridNo;
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		// CHECK IF WE ARE A TREE HERE
		if ( IsTreePresentAtGridno( (INT16)cnt ) )
		{
			// CHECK FOR A STRUCTURE A FOOTPRINT AWAY
			for ( dir = 0; dir < NUM_DIR_SEARCHES; dir++ )
			{
				sNewGridNo = NewGridNo( (INT16)cnt, (UINT16)DirectionInc( bDirectionsForShadowSearch[ dir ] ) );

				if ( gpWorldLevelData[ sNewGridNo ].pStructureHead == NULL )
				{
					break;
				}
			}
			// If we made it here, remove shadow!
			// We're full of structures
			if ( dir == NUM_DIR_SEARCHES )
			{
				RemoveAllShadows( cnt );
				// Display message to effect
			}
		}
	}
}


static void SetBlueFlagFlags(void)
{
	INT32		cnt;
	LEVELNODE * pNode;

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		pNode = gpWorldLevelData[ cnt ].pStructHead;
		while ( pNode )
		{
			if ( pNode->usIndex == BLUEFLAG_GRAPHIC)
			{
				gpWorldLevelData[ cnt ].uiFlags |= MAPELEMENT_PLAYER_MINE_PRESENT;
				break;
			}
			pNode = pNode->pNext;
		}
	}
}


void InitLoadedWorld(void)
{
	//if the current sector is not valid, dont init the world
	if( gWorldSectorX == 0 || gWorldSectorY == 0 )
	{
		return;
	}

	// COMPILE MOVEMENT COSTS
	CompileWorldMovementCosts( );

	// COMPILE WORLD VISIBLIY TILES
	CalculateWorldWireFrameTiles( TRUE );

	LightSpriteRenderAll();

	OptimizeMapForShadows( );

	SetInterfaceHeightLevel( );

	// ATE: if we have a slide location, remove it!
	gTacticalStatus.sSlideTarget = NOWHERE;

	SetBlueFlagFlags();
}

#ifdef JA2EDITOR
//This is a specialty function that is very similar to LoadWorld, except that it
//doesn't actually load the world, it instead evaluates the map and generates summary
//information for use within the summary editor.  The header is defined in Summary Info.h,
//not worlddef.h -- though it's not likely this is going to be used anywhere where it would
//matter.
extern double MasterStart, MasterEnd;
extern BOOLEAN gfUpdatingNow;

BOOLEAN EvaluateWorld(const char* pSector, UINT8 ubLevel)
{
	FLOAT	dMajorMapVersion;
	HWFILE	hfile;
	MAPCREATE_STRUCT mapInfo;
	UINT32					uiFileSize;
	UINT32 uiFlags;
	INT32 cnt;
	INT32 i;
	INT32 iTilesetID;
	wchar_t str[40];
	UINT8	bCounts[ WORLD_MAX ][8];
	UINT8 ubCombine;
	UINT8 ubMinorMapVersion;


	//Make sure the file exists... if not, then return false
	char szFilename[40];
	strlcpy(szFilename, pSector, lengthof(szFilename));
	if( ubLevel % 4  )
	{
		char str[4];
		sprintf( str, "_b%d", ubLevel % 4 );
		strcat( szFilename, str );
	}
	if( ubLevel >= 4 )
	{
		strcat( szFilename, "_a" );
	}
	strcat( szFilename, ".dat" );
	char szDirFilename[50];
	sprintf( szDirFilename, "MAPS/%s", szFilename );

	if( gfMajorUpdate )
	{
		if( !LoadWorld( szFilename ) ) //error
			return FALSE;
		FileClearAttributes( szDirFilename );
		SaveWorld( szFilename );
	}

	hfile = FileOpen(szDirFilename, FILE_ACCESS_READ);
	if( !hfile )
		return FALSE;

	uiFileSize = FileGetSize( hfile );
	INT8*       pBuffer     = MALLOCN(INT8, uiFileSize);
	INT8* const pBufferHead = pBuffer;
	FileRead(hfile, pBuffer, uiFileSize);
	FileClose( hfile );

	swprintf(str, lengthof(str), L"Analyzing map %hs", szFilename);
	if( !gfUpdatingNow )
		SetRelativeStartAndEndPercentage( 0, 0, 100, str );
	else
		SetRelativeStartAndEndPercentage( 0, (UINT16)MasterStart, (UINT16)MasterEnd, str );

	RenderProgressBar( 0, 0 );
	//RenderProgressBar( 1, 0 );

	//clear the summary file info
	SUMMARYFILE* const pSummary = MALLOCZ(SUMMARYFILE);
	Assert( pSummary );
	pSummary->ubSummaryVersion = GLOBAL_SUMMARY_VERSION;
	pSummary->dMajorMapVersion = gdMajorMapVersion;

	//skip JA2 Version ID
	LOADDATA( &dMajorMapVersion, pBuffer, sizeof( FLOAT ) );
	if( dMajorMapVersion >= 4.00 )
	{
		LOADDATA( &ubMinorMapVersion, pBuffer, sizeof( UINT8 ) );
	}

	//Read FLAGS FOR WORLD
	LOADDATA( &uiFlags, pBuffer, sizeof( INT32 ) );

	//Read tilesetID
	LOADDATA( &iTilesetID, pBuffer, sizeof( INT32 ) );
	pSummary->ubTilesetID = (UINT8)iTilesetID;

	//skip soldier size
	pBuffer += sizeof( INT32 );

	//skip height values
	pBuffer += sizeof( INT16 ) * WORLD_MAX;

	//read layer counts
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		if( !( cnt % 2560 ) )
		{
			RenderProgressBar( 0, (cnt / 2560)+1 ); //1 - 10
			//RenderProgressBar( 1, (cnt / 2560)+1 ); //1 - 10
		}
		// Read combination of land/world flags
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );
		// split
		bCounts[ cnt ][0] = (UINT8)(ubCombine&0xf);
		gpWorldLevelData[ cnt ].uiFlags |= (UINT8)((ubCombine&0xf0)>>4);
		// Read #objects, structs
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );
		// split
		bCounts[ cnt ][1] = (UINT8)(ubCombine&0xf);
		bCounts[ cnt ][2] = (UINT8)((ubCombine&0xf0)>>4);
		// Read shadows, roof
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );
		// split
		bCounts[ cnt ][3] = (UINT8)(ubCombine&0xf);
		bCounts[ cnt ][4] = (UINT8)((ubCombine&0xf0)>>4);
  	// Read OnRoof, nothing
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );
		// split
		bCounts[ cnt ][5] = (UINT8)(ubCombine&0xf);
		//bCounts[ cnt ][4] = (UINT8)((ubCombine&0xf0)>>4);
		bCounts[ cnt ][6] = bCounts[cnt][0] + bCounts[cnt][1] +
												bCounts[cnt][2] + bCounts[cnt][3] +
												bCounts[cnt][4] + bCounts[cnt][5];
	}
	//skip all layers
	for( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if( !( cnt % 320 ) )
		{
			RenderProgressBar( 0, (cnt / 320)+11 ); //11 - 90
			//RenderProgressBar( 1, (cnt / 320)+11 ); //11 - 90
		}
		pBuffer += sizeof( UINT16 ) * bCounts[cnt][6];
		pBuffer += bCounts[cnt][1];
	}

	//extract highest room number
	{
		UINT8 ubRoomNum;
		for( cnt = 0; cnt < WORLD_MAX; cnt++ )
		{
			LOADDATA( &ubRoomNum, pBuffer, 1 );
			if( ubRoomNum > pSummary->ubNumRooms )
			{
				pSummary->ubNumRooms = ubRoomNum;
			}
		}
	}

	if( uiFlags & MAP_WORLDITEMS_SAVED )
	{
		UINT32 temp;
		RenderProgressBar( 0, 91 );
		//RenderProgressBar( 1, 91 );
		//get number of items (for now)
		LOADDATA( &temp, pBuffer, 4 );
		pSummary->usNumItems = (UINT16)temp;
		//Important:  Saves the file position (byte offset) of the position where the numitems
		//            resides.  Checking this value and comparing to usNumItems will ensure validity.
		if( pSummary->usNumItems )
		{
			pSummary->uiNumItemsPosition = pBuffer - pBufferHead - 4;
		}
		//Skip the contents of the world items.
		pBuffer += sizeof( WORLDITEM ) * pSummary->usNumItems;
	}

	if( uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED )
	{
		pBuffer += 3;
	}

	if( uiFlags & MAP_WORLDLIGHTS_SAVED )
	{
		UINT8 ubTemp;
		RenderProgressBar( 0, 92 );
		//RenderProgressBar( 1, 92 );
		//skip number of light palette entries
		LOADDATA( &ubTemp, pBuffer, 1 );
		pBuffer += sizeof( SGPPaletteEntry ) * ubTemp;
		//get number of lights
		LOADDATA( &pSummary->usNumLights, pBuffer, 2 );
		//skip the light loading
		for( cnt = 0; cnt < pSummary->usNumLights; cnt++ )
		{
			UINT8 ubStrLen;
			pBuffer += 24; // size of a LIGHT_SPRITE on disk
			LOADDATA( &ubStrLen, pBuffer, 1 );
			if( ubStrLen )
			{
				pBuffer += ubStrLen;
			}
		}
	}

	//read the mapinformation
	LOADDATA( &mapInfo, pBuffer, sizeof( MAPCREATE_STRUCT ) );

	pSummary->MapInfo = mapInfo;

	if( uiFlags & MAP_FULLSOLDIER_SAVED )
	{
		TEAMSUMMARY *pTeam=NULL;
		BASIC_SOLDIERCREATE_STRUCT basic;
		SOLDIERCREATE_STRUCT priority;
		RenderProgressBar( 0, 94 );
		//RenderProgressBar( 1, 94 );

		pSummary->uiEnemyPlacementPosition = pBuffer - pBufferHead;

		for( i=0; i < pSummary->MapInfo.ubNumIndividuals ; i++ )
		{
			LOADDATA( &basic, pBuffer, sizeof( BASIC_SOLDIERCREATE_STRUCT ) );

			switch( basic.bTeam )
			{
				case ENEMY_TEAM:		pTeam = &pSummary->EnemyTeam;			break;
				case CREATURE_TEAM:	pTeam = &pSummary->CreatureTeam;	break;
				case MILITIA_TEAM:	pTeam = &pSummary->RebelTeam;			break;
				case CIV_TEAM:			pTeam = &pSummary->CivTeam;				break;
			}
			if( basic.bOrders == RNDPTPATROL || basic.bOrders == POINTPATROL )
			{ //make sure the placement has at least one waypoint.
				if( !basic.bPatrolCnt )
				{
					pSummary->ubEnemiesReqWaypoints++;
				}
			}
			else if( basic.bPatrolCnt )
			{
				pSummary->ubEnemiesHaveWaypoints++;
			}
			if( basic.fPriorityExistance )
				pTeam->ubExistance++;
			switch( basic.bRelativeAttributeLevel )
			{
				case 0:	pTeam->ubBadA++;		break;
				case 1:	pTeam->ubPoorA++;		break;
				case 2:	pTeam->ubAvgA++;		break;
				case 3:	pTeam->ubGoodA++;		break;
				case 4:	pTeam->ubGreatA++;	break;
			}
			switch( basic.bRelativeEquipmentLevel )
			{
				case 0:	pTeam->ubBadE++;		break;
				case 1:	pTeam->ubPoorE++;		break;
				case 2:	pTeam->ubAvgE++;		break;
				case 3:	pTeam->ubGoodE++;		break;
				case 4:	pTeam->ubGreatE++;	break;
			}
			if( basic.fDetailedPlacement )
			{ //skip static priority placement
				BYTE Data[1040];
				LOADDATA(Data, pBuffer, sizeof(Data));
				ExtractSoldierCreateUTF16(Data, &priority);
				if( priority.ubProfile != NO_PROFILE )
					pTeam->ubProfile++;
				else
					pTeam->ubDetailed++;
				if( basic.bTeam == CIV_TEAM )
				{
					if( priority.ubScheduleID )
						pSummary->ubCivSchedules++;
					if( priority.bBodyType == COW )
						pSummary->ubCivCows++;
					else if( priority.bBodyType == BLOODCAT )
						pSummary->ubCivBloodcats++;
				}
			}
			if( basic.bTeam == ENEMY_TEAM )
			{
				switch( basic.ubSoldierClass )
				{
					case SOLDIER_CLASS_ADMINISTRATOR:
						pSummary->ubNumAdmins++;
						if( basic.fPriorityExistance )
							pSummary->ubAdminExistance++;
						if( basic.fDetailedPlacement )
						{
							if( priority.ubProfile != NO_PROFILE )
								pSummary->ubAdminProfile++;
							else
								pSummary->ubAdminDetailed++;
						}
						break;
					case SOLDIER_CLASS_ELITE:
						pSummary->ubNumElites++;
						if( basic.fPriorityExistance )
							pSummary->ubEliteExistance++;
						if( basic.fDetailedPlacement )
						{
							if( priority.ubProfile != NO_PROFILE )
								pSummary->ubEliteProfile++;
							else
								pSummary->ubEliteDetailed++;
						}
						break;
					case SOLDIER_CLASS_ARMY:
						pSummary->ubNumTroops++;
						if( basic.fPriorityExistance )
							pSummary->ubTroopExistance++;
						if( basic.fDetailedPlacement )
						{
							if( priority.ubProfile != NO_PROFILE )
								pSummary->ubTroopProfile++;
							else
								pSummary->ubTroopDetailed++;
						}
						break;
				}
			}
			else if( basic.bTeam == CREATURE_TEAM )
			{
				if( basic.bBodyType == BLOODCAT )
					pTeam->ubNumAnimals++;
			}
			pTeam->ubTotal++;
		}
		RenderProgressBar( 0, 96 );
		//RenderProgressBar( 1, 96 );
	}

	if( uiFlags & MAP_EXITGRIDS_SAVED )
	{
		EXITGRID exitGrid;
		INT32 loop;
		UINT16 usMapIndex;
		BOOLEAN fExitGridFound;
		RenderProgressBar( 0, 98 );
		//RenderProgressBar( 1, 98 );

		LOADDATA( &cnt, pBuffer, 2 );

		for( i = 0; i < cnt; i++ )
		{
			LOADDATA( &usMapIndex, pBuffer, 2 );
			LOADDATA( &exitGrid, pBuffer, 5 );
			fExitGridFound = FALSE;
			for( loop = 0; loop < pSummary->ubNumExitGridDests; loop++ )
			{
				if( pSummary->ExitGrid[ loop ].usGridNo == exitGrid.usGridNo &&
						pSummary->ExitGrid[ loop ].ubGotoSectorX == exitGrid.ubGotoSectorX &&
						pSummary->ExitGrid[ loop ].ubGotoSectorY == exitGrid.ubGotoSectorY &&
						pSummary->ExitGrid[ loop ].ubGotoSectorZ == exitGrid.ubGotoSectorZ )
				{ //same destination.
					pSummary->usExitGridSize[ loop ]++;
					fExitGridFound = TRUE;
					break;
				}
			}
			if( !fExitGridFound )
			{
				if( loop >= 4 )
				{
					pSummary->fTooManyExitGridDests = TRUE;
				}
				else
				{
					pSummary->ubNumExitGridDests++;
					pSummary->usExitGridSize[ loop ]++;
					pSummary->ExitGrid[ loop ].usGridNo = exitGrid.usGridNo;
					pSummary->ExitGrid[ loop ].ubGotoSectorX = exitGrid.ubGotoSectorX;
					pSummary->ExitGrid[ loop ].ubGotoSectorY = exitGrid.ubGotoSectorY;
					pSummary->ExitGrid[ loop ].ubGotoSectorZ = exitGrid.ubGotoSectorZ;
					if( pSummary->ExitGrid[ loop ].ubGotoSectorX != exitGrid.ubGotoSectorX ||
							pSummary->ExitGrid[ loop ].ubGotoSectorY != exitGrid.ubGotoSectorY )
					{
						pSummary->fInvalidDest[ loop ] = TRUE;
					}
				}
			}
		}
	}

	if( uiFlags & MAP_DOORTABLE_SAVED )
	{
		DOOR Door;

		LOADDATA( &pSummary->ubNumDoors, pBuffer, 1 );

		for( cnt = 0; cnt < pSummary->ubNumDoors; cnt++ )
		{
			LOADDATA( &Door, pBuffer, sizeof( DOOR ) );

			if( Door.ubTrapID && Door.ubLockID )
				pSummary->ubNumDoorsLockedAndTrapped++;
			else if( Door.ubLockID )
				pSummary->ubNumDoorsLocked++;
			else if( Door.ubTrapID )
				pSummary->ubNumDoorsTrapped++;
		}
	}

	RenderProgressBar( 0, 100 );
	//RenderProgressBar( 1, 100 );

	MemFree( pBufferHead );

	WriteSectorSummaryUpdate( szFilename, ubLevel, pSummary );
	return TRUE;
}
#endif

extern void LoadShadeTablesFromTextFile();


static void LoadMapLights(INT8** hBuffer);


BOOLEAN LoadWorld(const char *puiFilename)
{
	HWFILE					hfile;
	FLOAT						dMajorMapVersion;
	UINT32					uiFlags;
	UINT32					uiSoldierSize;
	UINT32					uiFileSize;
	UINT32					fp, offset;
#ifdef JA2TESTVERSION
	UINT32					uiStartTime;
	UINT32					uiLoadWorldStartTime;
#endif
	INT32						cnt, cnt2;
	INT32						iTilesetID;
	UINT16					usTypeSubIndex;
	UINT8						ubType;
	UINT8						ubSubIndex;
	CHAR8						aFilename[ 50 ];
	UINT8						ubCombine;
	UINT8							bCounts[ WORLD_MAX ][8];
	BOOLEAN					fGenerateEdgePoints = FALSE;
#ifdef JA2TESTVERSION
	uiLoadWorldStartTime = GetJA2Clock();
#endif

	LoadShadeTablesFromTextFile();

	// Append exension to filename!
	sprintf(aFilename, "MAPS/%s", puiFilename);

	// RESET FLAGS FOR OUTDOORS/INDOORS
	gfBasement = FALSE;
	gfCaves = FALSE;

	// Open file
	hfile = FileOpen(aFilename, FILE_ACCESS_READ);

	if ( !hfile )
	{
		SET_ERROR("Could not load map file %s", aFilename);
		return( FALSE );
	}

	SetRelativeStartAndEndPercentage( 0, 0, 1, L"Trashing world..." );
#ifdef JA2TESTVERSION
	uiStartTime = GetJA2Clock();
#endif

	TrashWorld();

#ifdef JA2TESTVERSION
	uiTrashWorldTime = GetJA2Clock() - uiStartTime;
#endif

	LightReset();

	//Get the file size and alloc one huge buffer for it.
	//We will use this buffer to transfer all of the data from.
	uiFileSize = FileGetSize( hfile );
	INT8*       pBuffer     = MALLOCN(INT8, uiFileSize);
	INT8* const pBufferHead = pBuffer;
	FileRead(hfile, pBuffer, uiFileSize);
	FileClose( hfile );

	// Read JA2 Version ID
	LOADDATA( &dMajorMapVersion, pBuffer, sizeof( FLOAT ) );

#if defined RUSSIAN
	if (dMajorMapVersion != 6.00)
	{
		return FALSE;
	}
#endif

	UINT8 ubMinorMapVersion;
	if (dMajorMapVersion >= 4.00)
	{
		LOADDATA(&ubMinorMapVersion, pBuffer, sizeof(UINT8));
	}
	else
	{
		ubMinorMapVersion = 0;
	}

	// CHECK FOR NON-COMPATIBLE VERSIONS!
	// CHECK FOR MAJOR MAP VERSION INCOMPATIBLITIES
	//if ( dMajorMapVersion < gdMajorMapVersion )
	//{
		//AssertMsg( 0, "Major version conflict.  Should have force updated this map already!!!" );
		//SET_ERROR(  "Incompatible JA2 map version: %f, map version is now at %f", gdLoadedMapVersion, gdMapVersion );
		//return( FALSE );
	//}

	// Read FLAGS FOR WORLD
	LOADDATA( &uiFlags, pBuffer, sizeof( INT32 ) );

	LOADDATA( &iTilesetID, pBuffer, sizeof( INT32 ) );

#ifdef JA2TESTVERSION
	uiStartTime = GetJA2Clock();
#endif
	CHECKF(LoadMapTileset(iTilesetID));
#ifdef JA2TESTVERSION
	uiLoadMapTilesetTime = GetJA2Clock() - uiStartTime;
#endif

	// Load soldier size
	LOADDATA( &uiSoldierSize, pBuffer, sizeof( INT32 ) );

	//FP 0x000010

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Read height values
		LOADDATA( &gpWorldLevelData[ cnt ].sHeight, pBuffer, sizeof( INT16 ) );
	}

	//FP 0x00c810

	SetRelativeStartAndEndPercentage( 0, 35, 40, L"Counting layers..." );
	RenderProgressBar( 0, 100 );

	// Read layer counts
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
  {
		// Read combination of land/world flags
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );

		// split
		bCounts[ cnt ][0] = (UINT8)(ubCombine&0xf);
		gpWorldLevelData[ cnt ].uiFlags |= (UINT8)((ubCombine&0xf0)>>4);

		// Read #objects, structs
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );

		// split
		bCounts[ cnt ][1] = (UINT8)(ubCombine&0xf);
		bCounts[ cnt ][2] = (UINT8)((ubCombine&0xf0)>>4);

		// Read shadows, roof
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );

		// split
		bCounts[ cnt ][3] = (UINT8)(ubCombine&0xf);
		bCounts[ cnt ][4] = (UINT8)((ubCombine&0xf0)>>4);

   	// Read OnRoof, nothing
		LOADDATA( &ubCombine, pBuffer, sizeof( UINT8 ) );

		// split
		bCounts[ cnt ][5] = (UINT8)(ubCombine&0xf);
	}

	//FP 0x025810
	fp = 0x025810;
	offset = 0;

	SetRelativeStartAndEndPercentage( 0, 40, 43, L"Loading land layers...");
	RenderProgressBar( 0, 100 );

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
 		// Read new values
		if( bCounts[ cnt ][ 0 ] > 10 )
		{
			cnt = cnt;
		}
		for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 0 ]; cnt2++ )
		{
			LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
			LOADDATA( &ubSubIndex, pBuffer, sizeof( UINT8 ) );

			// Get tile index
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);

			// Add layer
			AddLandToHead( cnt, usTileIndex );

			offset += 2;
		}
	}

	fp += offset;
	offset = 0;

	SetRelativeStartAndEndPercentage( 0, 43, 46, L"Loading object layer...");
	RenderProgressBar( 0, 100 );

	if (ubMinorMapVersion < 15)
	{ //Old loads
		for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
		{
			// Set objects
			for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 1 ]; cnt2++ )
			{

				LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
				LOADDATA( &ubSubIndex, pBuffer, sizeof( UINT8 ) );
				if ( ubType >= FIRSTPOINTERS )
				{
					continue;
				}
				// Get tile index
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);
				// Add layer
				AddObjectToTail( cnt, usTileIndex );
			}
		}
	}
	else
	{ //New load require UINT16 for the type subindex due to the fact that ROADPIECES
		//contain over 300 type subindices.
		for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
		{
			// Set objects
			if( bCounts[ cnt ][ 1 ] > 10 )
			{
				cnt = cnt;
			}
			for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 1 ]; cnt2++ )
			{

				LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
				LOADDATA( &usTypeSubIndex, pBuffer, sizeof( UINT16 ) );
				if ( ubType >= FIRSTPOINTERS )
				{
					continue;
				}
				// Get tile index
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, usTypeSubIndex);
				// Add layer
				AddObjectToTail( cnt, usTileIndex );

				offset += 3;
			}
		}
	}

	fp += offset;
	offset = 0;

	SetRelativeStartAndEndPercentage( 0, 46, 49, L"Loading struct layer..." );
	RenderProgressBar( 0, 100 );

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		// Set structs
		if( bCounts[ cnt ][ 2 ] > 10 )
		{
			cnt = cnt;
		}
		for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 2 ]; cnt2++ )
		{
			LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
			LOADDATA( &ubSubIndex, pBuffer, sizeof( UINT8 ) );

			// Get tile index
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);

      if ( ubMinorMapVersion <= 25 )
      {
        // Check patching for phantom menace struct data...
        if ( gTileDatabase[ usTileIndex ].uiFlags & UNDERFLOW_FILLER )
        {
			    usTileIndex = GetTileIndexFromTypeSubIndex(ubType, 1);
        }
      }

			// Add layer
			AddStructToTail( cnt, usTileIndex );

			offset += 2;
		}
	}

	fp += offset;
	offset = 0;

	SetRelativeStartAndEndPercentage( 0, 49, 52, L"Loading shadow layer..." );
	RenderProgressBar( 0, 100 );

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if( bCounts[ cnt ][ 3 ] > 10 )
		{
			cnt = cnt;
		}
		for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 3 ]; cnt2++ )
		{
			LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
			LOADDATA( &ubSubIndex, pBuffer, sizeof( UINT8 ) );

			// Get tile index
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);

			// Add layer
			AddShadowToTail( cnt, usTileIndex );

			offset += 2;
		}
	}

	fp += offset;
	offset = 0;

	SetRelativeStartAndEndPercentage( 0, 52, 55, L"Loading roof layer..." );
	RenderProgressBar( 0, 100 );

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if( bCounts[ cnt ][ 4 ] > 10 )
		{
			cnt = cnt;
		}
		for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 4 ]; cnt2++ )
		{
			LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
			LOADDATA( &ubSubIndex, pBuffer, sizeof( UINT8 ) );

			// Get tile index
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);

			// Add layer
			AddRoofToTail( cnt, usTileIndex );

			offset += 2;
		}
	}

	fp += offset;
	offset = 0;

	SetRelativeStartAndEndPercentage( 0, 55, 58, L"Loading on roof layer..." );
	RenderProgressBar( 0, 100 );

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if( bCounts[ cnt ][ 5 ] > 10 )
		{
			cnt = cnt;
		}
		for( cnt2 = 0; cnt2 < bCounts[ cnt ][ 5 ]; cnt2++ )
		{
			LOADDATA( &ubType, pBuffer, sizeof( UINT8 ) );
			LOADDATA( &ubSubIndex, pBuffer, sizeof( UINT8 ) );

			// Get tile index
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);

			// Add layer
			AddOnRoofToTail( cnt, usTileIndex );

			offset += 2;
		}
	}

	fp += offset;
	offset = 0;

#if defined RUSSIAN
	{
		UINT32 uiNums[37];
		LOADDATA( uiNums, pBuffer, 37 * sizeof( INT32 ) );
	}
#endif

	SetRelativeStartAndEndPercentage( 0, 58, 59, L"Loading room information..." );
	RenderProgressBar( 0, 100 );

	#ifdef JA2EDITOR
		gubMaxRoomNumber = 0;
		for( cnt = 0; cnt < WORLD_MAX; cnt++ )
		{
			// Read room information
			LOADDATA( &gubWorldRoomInfo[ cnt ], pBuffer, sizeof( INT8 ) );
				//Got to set the max room number
				if( gubWorldRoomInfo[ cnt ] > gubMaxRoomNumber )
					gubMaxRoomNumber = gubWorldRoomInfo[ cnt ];
		}
		if( gubMaxRoomNumber < 255 )
			gubMaxRoomNumber++;
	#else
		LOADDATA( gubWorldRoomInfo, pBuffer, sizeof( INT8 ) * WORLD_MAX );
	#endif

	// ATE; Memset this array!
	if( 0 )
	{ //for debugging purposes
		memset( gubWorldRoomInfo, 0, sizeof( gubWorldRoomInfo ) );
	}

	memset( gubWorldRoomHidden, TRUE, sizeof( gubWorldRoomHidden ) );

	SetRelativeStartAndEndPercentage( 0, 59, 61, L"Loading items..." );
	RenderProgressBar( 0, 100 );

	if( uiFlags & MAP_WORLDITEMS_SAVED )
	{
		// Load out item information
		gfLoadPitsWithoutArming = TRUE;
		LoadWorldItemsFromMap( &pBuffer );
		gfLoadPitsWithoutArming = FALSE;
	}

	SetRelativeStartAndEndPercentage( 0, 62, 85, L"Loading lights..." );
	RenderProgressBar( 0, 0 );

	if( uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED )
	{ //Ambient light levels are only saved in underground levels
		LOADDATA( &gfBasement, pBuffer, 1 );
		LOADDATA( &gfCaves, pBuffer, 1 );
		LOADDATA( &ubAmbientLightLevel, pBuffer, 1 );
	}
	else
	{ //We are above ground.
		gfBasement = FALSE;
		gfCaves = FALSE;
		if( !gfEditMode && guiCurrentScreen != MAPUTILITY_SCREEN )
		{
			ubAmbientLightLevel = GetTimeOfDayAmbientLightLevel();
		}
		else
		{
			ubAmbientLightLevel = 4;
		}
	}
#ifdef JA2TESTVERSION
	uiStartTime = GetJA2Clock();
#endif
	if( uiFlags & MAP_WORLDLIGHTS_SAVED )
	{
		LoadMapLights( &pBuffer );
	}
	else
	{
		// Set some default value for lighting
		SetDefaultWorldLightingColors( );
	}
	LightSetBaseLevel( ubAmbientLightLevel );
#ifdef JA2TESTVERSION
	uiLoadMapLightsTime = GetJA2Clock() - uiStartTime;
#endif


	SetRelativeStartAndEndPercentage( 0, 85, 86, L"Loading map information..." );
	RenderProgressBar( 0, 0 );

	LoadMapInformation( &pBuffer );

	if( uiFlags & MAP_FULLSOLDIER_SAVED )
	{
		SetRelativeStartAndEndPercentage( 0, 86, 87, L"Loading placements..." );
		RenderProgressBar( 0, 0 );
		LoadSoldiersFromMap( &pBuffer );
	}
	if( uiFlags & MAP_EXITGRIDS_SAVED )
	{
		SetRelativeStartAndEndPercentage( 0, 87, 88, L"Loading exit grids..." );
		RenderProgressBar( 0, 0 );
		LoadExitGrids( &pBuffer );
	}
	if( uiFlags & MAP_DOORTABLE_SAVED )
	{
		SetRelativeStartAndEndPercentage( 0, 89, 90, L"Loading door tables..." );
		RenderProgressBar( 0, 0 );
		LoadDoorTableFromMap( &pBuffer );
	}
	if( uiFlags & MAP_EDGEPOINTS_SAVED )
	{
		SetRelativeStartAndEndPercentage( 0, 90, 91, L"Loading edgepoints..." );
		RenderProgressBar( 0, 0 );
		if( !LoadMapEdgepoints( &pBuffer ) )
			fGenerateEdgePoints = TRUE; //only if the map had the older edgepoint system
	}
	else
	{
		fGenerateEdgePoints = TRUE;
	}
	if( uiFlags & MAP_NPCSCHEDULES_SAVED )
	{
		SetRelativeStartAndEndPercentage( 0, 91, 92, L"Loading NPC schedules..." );
		RenderProgressBar( 0, 0 );
		LoadSchedules( &pBuffer );
	}

	ValidateAndUpdateMapVersionIfNecessary();

	//if we arent loading a saved game
//	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		SetRelativeStartAndEndPercentage( 0, 93, 94, L"Init Loaded World..." );
		RenderProgressBar( 0, 0 );
		InitLoadedWorld( );
	}

	if( fGenerateEdgePoints )
	{
		SetRelativeStartAndEndPercentage( 0, 94, 95, L"Generating map edgepoints..." );
		RenderProgressBar( 0, 0 );
		CompileWorldMovementCosts();
		GenerateMapEdgepoints();
	}

	RenderProgressBar( 0, 20 );

	SetRelativeStartAndEndPercentage( 0, 95, 100, L"General initialization..." );
	// RESET AI!
	InitOpponentKnowledgeSystem();

	RenderProgressBar( 0, 30 );

	//AllTeamsLookForAll(NO_INTERRUPTS);

	RenderProgressBar( 0, 40 );

	// CHECK IF OUR SELECTED GUY IS GONE!
	if (g_selected_man != NULL && !g_selected_man->bActive)
	{
		SetSelectedMan(NULL);
	}

	RenderProgressBar( 0, 60 );

	InvalidateWorldRedundency( );

	LoadRadarScreenBitmap( puiFilename );

	RenderProgressBar( 0, 80 );

	gfWorldLoaded = TRUE;

#ifdef JA2EDITOR
	strlcpy(g_filename, puiFilename, lengthof(g_filename));
#endif

	//Remove this rather large chunk of memory from the system now!
	MemFree( pBufferHead );

#ifdef JA2TESTVERSION
	uiLoadWorldTime = GetJA2Clock() - uiLoadWorldStartTime;
#endif

#ifdef JA2TESTVERSION

  // ATE: Not while updating maps!
  if ( guiCurrentScreen != MAPUTILITY_SCREEN )
  {
  	GenerateBuildings();
  }

#endif

	RenderProgressBar( 0, 100 );


	DequeueAllKeyBoardEvents();


	return( TRUE );
}


BOOLEAN NewWorld( void )
{
	UINT16				NewIndex;
	INT32					cnt;

	SetSelectedMan(NULL);

	TrashWorld();

	// Create world randomly from tiles
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		// Set land index
		NewIndex = (UINT16)(rand( ) % 10);
		AddLandToHead( cnt, NewIndex );
	}


	InitRoomDatabase( );

	gfWorldLoaded = TRUE;

	return( TRUE );
}


void FreeLevelNodeList(LEVELNODE** const head)
{
	LEVELNODE* i = *head;
	*head = NULL;

	while (i != NULL)
	{
		LEVELNODE* const next = i->pNext;
		MemFree(i);
		i = next;
	}
}


void TrashWorld(void)
{
	if (!gfWorldLoaded) return;

	TrashWorldItems();
	TrashOverheadMap();
	ResetSmokeEffects();
	ResetLightEffects();

	// Set soldiers to not active!
	FOR_ALL_NON_PLANNING_SOLDIERS(s)
	{
		if (s->bTeam == gbPlayerNum)
		{
			s->pLevelNode = NULL; // Just delete levelnode
		}
		else
		{
			TacticalRemoveSoldier(s); // Delete from world
		}
	}

	RemoveCorpses();
	DeleteAniTiles();

	// Kill both soldier init lists.
	UseEditorAlternateList();
	KillSoldierInitList();
	UseEditorOriginalList();
	KillSoldierInitList();

	DestroyAllSchedules();

	// On trash world check if we have to set up the first meanwhile
	HandleFirstMeanWhileSetUpWithTrashWorld();

	for (INT32 i = 0; i < WORLD_MAX; ++i)
	{
		MAP_ELEMENT* const me = &gpWorldLevelData[i];

		// Free the memory associated with the map tile link lists
		FreeLevelNodeList(&me->pLandHead);
		FreeLevelNodeList(&me->pObjectHead);
		FreeLevelNodeList(&me->pStructHead);
		FreeLevelNodeList(&me->pShadowHead);
		FreeLevelNodeList(&me->pMercHead);
		FreeLevelNodeList(&me->pRoofHead);
		FreeLevelNodeList(&me->pOnRoofHead);
		FreeLevelNodeList(&me->pTopmostHead);

		while (me->pStructureHead != NULL)
		{
			if (!DeleteStructureFromWorld(me->pStructureHead))
			{
				// ERROR!!!!!!
				break;
			}
		}
	}

	// Zero world
	memset(gpWorldLevelData, 0, WORLD_MAX * sizeof(*gpWorldLevelData));

	// Set some default flags
	for (INT32 i = 0; i < WORLD_MAX; ++i)
	{
		gpWorldLevelData[i].uiFlags |= MAPELEMENT_RECALCULATE_WIREFRAMES;
	}

	TrashDoorTable();
	TrashMapEdgepoints();
	TrashDoorStatusArray();

	gfWorldLoaded = FALSE;
#ifdef JA2EDITOR
	strcpy(g_filename, "none");
#endif
}


static void TrashMapTile(const INT16 MapTile)
{
	MAP_ELEMENT* const me = &gpWorldLevelData[MapTile];

	// Free the memory associated with the map tile link lists
	me->pLandStart = NULL;
	FreeLevelNodeList(&me->pLandHead);
	FreeLevelNodeList(&me->pObjectHead);
	FreeLevelNodeList(&me->pStructHead);
	FreeLevelNodeList(&me->pShadowHead);
	FreeLevelNodeList(&me->pMercHead);
	FreeLevelNodeList(&me->pRoofHead);
	FreeLevelNodeList(&me->pOnRoofHead);
	FreeLevelNodeList(&me->pTopmostHead);

	while (me->pStructureHead != NULL)
	{
		DeleteStructureFromWorld(me->pStructureHead);
	}
}


BOOLEAN LoadMapTileset( INT32 iTilesetID )
{

	if ( iTilesetID >= NUM_TILESETS )
	{
		return( FALSE );
	}

	// Init tile surface used values
	memset( gbNewTileSurfaceLoaded, 0, sizeof( gbNewTileSurfaceLoaded ) );

	if( iTilesetID == giCurrentTilesetID )
	{
		return( TRUE );
	}

	// Get free memory value nere
	gSurfaceMemUsage = guiMemTotal;

	// LOAD SURFACES
	CHECKF(LoadTileSurfaces(&gTilesets[iTilesetID].TileSurfaceFilenames[0], iTilesetID));

	// SET TERRAIN COSTS
	if ( gTilesets[ iTilesetID ].MovementCostFnc != NULL )
	{
		gTilesets[ iTilesetID ].MovementCostFnc( );
	}
	else
	{
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Tileset %d has no callback function for movement costs. Using default.", iTilesetID) );
		SetTilesetOneTerrainValues( );
	}

	// RESET TILE DATABASE
	DeallocateTileDatabase( );

	CreateTileDatabase( );

	// SET GLOBAL ID FOR TILESET ( FOR SAVING! )
	giCurrentTilesetID = iTilesetID;

	return( TRUE );
}


static void AddWireFrame(INT16 sGridNo, UINT16 usIndex, BOOLEAN fForced)
{
	LEVELNODE			*pTopmost, *pTopmostTail;


	pTopmost = gpWorldLevelData[ sGridNo ].pTopmostHead;

	while ( pTopmost != NULL )
	{
		// Check if one of the samer type exists!
		if ( pTopmost->usIndex == usIndex )
		{
			return;
		}
		pTopmost = pTopmost->pNext;
	}

	pTopmostTail = AddTopmostToTail( sGridNo, usIndex );

	if ( fForced )
	{
		pTopmostTail->uiFlags |= LEVELNODE_WIREFRAME;
	}

}


static UINT16 GetWireframeGraphicNumToUseForWall(const INT16 sGridNo, STRUCTURE* const s)
{
	const STRUCTURE* const base_structure = FindBaseStructure(s);
	if (base_structure)
	{
		// Find levelnode...
		for (const LEVELNODE* n = gpWorldLevelData[sGridNo].pStructHead; n != NULL; n = n->pNext)
		{
			if (n->pStructureData == base_structure)
			{
				// Get Subindex for this wall...
				const UINT16 usSubIndex = GetSubIndexFromTileIndex(n->usIndex);
				switch (usSubIndex) // Check for broken pieces...
				{
					case 48:
					case 52: return WIREFRAMES12;
					case 49:
					case 53: return WIREFRAMES13;
					case 50:
					case 54: return WIREFRAMES10;
					case 51:
					case 55: return WIREFRAMES11;
				}
				break;
			}
		}
	}

	switch (s->ubWallOrientation)
	{
		case OUTSIDE_TOP_LEFT:
		case INSIDE_TOP_LEFT:   return WIREFRAMES6; break;
		case OUTSIDE_TOP_RIGHT:
		case INSIDE_TOP_RIGHT:  return WIREFRAMES5; break;
	}

	return 0;
}


static INT8 IsHiddenTileMarkerThere(INT16 sGridNo);
static BOOLEAN IsRoofVisibleForWireframe(INT16 sMapPos);
static void RemoveWireFrameTiles(INT16 sGridNo);


void CalculateWorldWireFrameTiles( BOOLEAN fForce )
{
	INT32					cnt;
	STRUCTURE		 *pStructure;
	INT16					sGridNo;
	UINT8					ubWallOrientation;
	INT8					bHiddenVal;
	INT8					bNumWallsSameGridNo;
  UINT16        usWireFrameIndex;

	// Create world randomly from tiles
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if ( gpWorldLevelData[ cnt ].uiFlags & MAPELEMENT_RECALCULATE_WIREFRAMES || fForce )
		{
			// Turn off flag
			gpWorldLevelData[ cnt ].uiFlags &= (~MAPELEMENT_RECALCULATE_WIREFRAMES );

			// Remove old ones
			RemoveWireFrameTiles( (INT16)cnt );

			bNumWallsSameGridNo = 0;

			// Check our gridno, if we have a roof over us that has not beenr evealed, no need for a wiereframe
			if ( IsRoofVisibleForWireframe( (UINT16)cnt ) && !( gpWorldLevelData[ cnt ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				continue;
			}

			pStructure = gpWorldLevelData[ cnt ].pStructureHead;

			while ( pStructure != NULL )
			{
				// Check for doors
				if ( pStructure->fFlags & STRUCTURE_ANYDOOR )
				{
					// ATE: need this additional check here for hidden doors!
					if ( pStructure->fFlags & STRUCTURE_OPENABLE )
					{
						// Does the gridno we are over have a non-visible tile?
						// Based on orientation
						ubWallOrientation = pStructure->ubWallOrientation;

						switch( ubWallOrientation )
						{
							case OUTSIDE_TOP_LEFT:
							case INSIDE_TOP_LEFT:

								// Get gridno
								sGridNo = NewGridNo( (INT16)cnt, DirectionInc( SOUTH ) );

								if ( IsRoofVisibleForWireframe( sGridNo ) && !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
								{
									AddWireFrame( (INT16)cnt, WIREFRAMES4, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );
								}
								break;

							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:

								// Get gridno
								sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );

								if ( IsRoofVisibleForWireframe( sGridNo ) && !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
								{
									AddWireFrame( (INT16)cnt, WIREFRAMES3 , (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );
								}
								break;

						}
					}
				}
				// Check for windows
				else
        {
          if ( pStructure->fFlags & STRUCTURE_WALLNWINDOW )
				  {
					  // Does the gridno we are over have a non-visible tile?
					  // Based on orientation
					  ubWallOrientation = pStructure->ubWallOrientation;

					  switch( ubWallOrientation )
					  {
						  case OUTSIDE_TOP_LEFT:
						  case INSIDE_TOP_LEFT:

							  // Get gridno
							  sGridNo = NewGridNo( (INT16)cnt, DirectionInc( SOUTH ) );

							  if ( IsRoofVisibleForWireframe( sGridNo ) && !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
							  {
								  AddWireFrame( (INT16)cnt, WIREFRAMES2, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );
							  }
							  break;

						  case OUTSIDE_TOP_RIGHT:
						  case INSIDE_TOP_RIGHT:

							  // Get gridno
							  sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );

							  if ( IsRoofVisibleForWireframe( sGridNo ) && !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
							  {
								  AddWireFrame( (INT16)cnt, WIREFRAMES1, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );
							  }
							  break;

					  }

				  }

				  // Check for walls
				  if ( pStructure->fFlags & STRUCTURE_WALLSTUFF )
				  {
					  // Does the gridno we are over have a non-visible tile?
					  // Based on orientation
					  ubWallOrientation = pStructure->ubWallOrientation;

            usWireFrameIndex = GetWireframeGraphicNumToUseForWall( (UINT16)cnt, pStructure );

					  switch( ubWallOrientation )
					  {
						  case OUTSIDE_TOP_LEFT:
						  case INSIDE_TOP_LEFT:

							  // Get gridno
							  sGridNo = NewGridNo( (INT16)cnt, DirectionInc( SOUTH ) );

							  if ( IsRoofVisibleForWireframe( sGridNo ) )
							  {
								   bNumWallsSameGridNo++;

								   AddWireFrame( (INT16)cnt, usWireFrameIndex, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );

								   // Check along our direction to see if we are a corner
								   sGridNo = NewGridNo( (INT16)cnt, DirectionInc( WEST ) );
								   sGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
								   bHiddenVal = IsHiddenTileMarkerThere( sGridNo );
								   // If we do not exist ( -1 ) or are revealed ( 1 )
								   if ( bHiddenVal == -1 || bHiddenVal == 1 )
								   {
									   // Place corner!
									   AddWireFrame( (INT16)cnt, WIREFRAMES9, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );
								   }
							  }
							  break;

						  case OUTSIDE_TOP_RIGHT:
						  case INSIDE_TOP_RIGHT:

							  // Get gridno
							  sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );

							  if ( IsRoofVisibleForWireframe( sGridNo ) )
							  {
								   bNumWallsSameGridNo++;

								   AddWireFrame( (INT16)cnt, usWireFrameIndex, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );

								   // Check along our direction to see if we are a corner
								   sGridNo = NewGridNo( (INT16)cnt, DirectionInc( NORTH ) );
								   sGridNo = NewGridNo( sGridNo, DirectionInc( EAST ) );
								   bHiddenVal = IsHiddenTileMarkerThere( sGridNo );
								   // If we do not exist ( -1 ) or are revealed ( 1 )
								   if ( bHiddenVal == -1 || bHiddenVal == 1 )
								   {
									   // Place corner!
									   AddWireFrame( (INT16)cnt, WIREFRAMES8, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 ) );
								   }

							  }
							  break;

					  }

					  // Check for both walls
					  if ( bNumWallsSameGridNo == 2 )
					  {
						   sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );
						   sGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
						   AddWireFrame( (INT16)cnt, WIREFRAMES7, (BOOLEAN)( ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) != 0 )  );
					  }
          }
				}

				pStructure = pStructure->pNext;
			}
		}
	}
}


#ifdef JA2EDITOR

static void RemoveWorldWireFrameTiles(void)
{
	INT32					cnt;

	// Create world randomly from tiles
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		RemoveWireFrameTiles( (INT16)cnt );
	}
}

#endif


static void RemoveWireFrameTiles(INT16 sGridNo)
{
	LEVELNODE			*pTopmost, *pNewTopmost;
	TILE_ELEMENT *	pTileElement;

	pTopmost = gpWorldLevelData[ sGridNo ].pTopmostHead;

	while ( pTopmost != NULL )
	{
		pNewTopmost = pTopmost->pNext;

		if ( pTopmost->usIndex < NUMBEROFTILES )
		{
			pTileElement = &(gTileDatabase[ pTopmost->usIndex ]);

			if ( pTileElement->fType == WIREFRAMES )
			{
				RemoveTopmost( sGridNo, pTopmost->usIndex );
			}
		}

		pTopmost = pNewTopmost;
	}

}


static INT8 IsHiddenTileMarkerThere(INT16 sGridNo)
{
	STRUCTURE * pStructure;

	if ( !gfBasement )
	{
		pStructure = FindStructure( sGridNo, STRUCTURE_ROOF );

		if ( pStructure != NULL )
		{
			//if ( !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				return( 2 );
			}

			// if we are here, a roof exists but has been revealed
			return( 1 );
		}
	}
	else
	{
		//if ( InARoom( sGridNo, &ubRoom ) )
		{
			//if ( !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				return( 2 );
			}

			return( 1 );
		}
	}

	return( -1 );
}


#ifdef JA2EDITOR

void ReloadTileset( UINT8 ubID )
{
	CHAR8	aFilename[ 255 ];
	INT32 iCurrTilesetID = giCurrentTilesetID;

	// Set gloabal
	giCurrentTilesetID = ubID;

	// Save Map
	SaveWorld( TEMP_FILE_FOR_TILESET_CHANGE );

	//IMPORTANT:  If this is not set, the LoadTileset() will assume that
	//it is loading the same tileset and ignore it...
	giCurrentTilesetID = iCurrTilesetID;

	// Load Map with new tileset
	LoadWorld( TEMP_FILE_FOR_TILESET_CHANGE );

	// Delete file
	sprintf( aFilename, "MAPS/%s", TEMP_FILE_FOR_TILESET_CHANGE );

	FileDelete( aFilename );
}

#endif


BOOLEAN IsSoldierLight(const LIGHT_SPRITE* const l)
{
	CFOR_ALL_NON_PLANNING_SOLDIERS(s)
	{
		if (s->light == l) return TRUE;
	}
	return FALSE;
}


#ifdef JA2EDITOR

static void SaveMapLights(HWFILE hfile)
{
	UINT16 usNumLights = 0;

	// Save the current light colors!
	const UINT8 ubNumColors = 1;
	FileWrite(hfile, &ubNumColors, 1);
	const SGPPaletteEntry* LColor = LightGetColor();
	FileWrite(hfile, LColor, sizeof(*LColor));

	//count number of non-merc lights.
	CFOR_ALL_LIGHT_SPRITES(l)
	{
		if (!IsSoldierLight(l)) ++usNumLights;
	}

	//save the number of lights.
	FileWrite(hfile, &usNumLights, 2);

	CFOR_ALL_LIGHT_SPRITES(l)
	{
		if (!IsSoldierLight(l)) InjectLightSpriteIntoFile(hfile, l);
	}
}

#endif


static void LoadMapLights(INT8** hBuffer)
{
	SGPPaletteEntry	LColors[3];
	UINT8 ubNumColors;
	UINT16 usNumLights;

	//reset the lighting system, so that any current lights are toasted.
	LightReset();

	// read in the light colors!
	LOADDATA( &ubNumColors, *hBuffer, 1 );
	LOADDATA( LColors, *hBuffer, sizeof(SGPPaletteEntry)*ubNumColors );

	LOADDATA( &usNumLights, *hBuffer, 2 );

	LightSetColor(LColors);

	//Determine which lights are valid for the current time.
	UINT32 light_time = 0;
	if( !gfEditMode )
	{
		const UINT32 uiHour = GetWorldHour();
		if( uiHour >= NIGHT_TIME_LIGHT_START_HOUR || uiHour < NIGHT_TIME_LIGHT_END_HOUR )
		{
			light_time |= LIGHT_NIGHTTIME;
		}
		if( uiHour >= PRIME_TIME_LIGHT_START_HOUR )
		{
			light_time |= LIGHT_PRIMETIME;
		}
	}

	for (INT32 cnt = 0; cnt < usNumLights; ++cnt)
	{
		ExtractLightSprite((const BYTE**)hBuffer, light_time);
	}
}


static BOOLEAN IsRoofVisibleForWireframe(const INT16 sMapPos)
{
	return
		gfBasement ||
		FindStructure(sMapPos, STRUCTURE_ROOF) != NULL;
}
