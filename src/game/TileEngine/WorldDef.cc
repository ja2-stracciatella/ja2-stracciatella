#include "WorldDef.h"
#include "Animated_ProgressBar.h"
#include "Animation_Data.h"
#include "Buildings.h"
#include "ContentManager.h"
#include "Debug.h"
#include "EditorBuildings.h"
#include "EditorMapInfo.h"
#include "Environment.h"
#include "Exit_Grids.h"
#include "FileMan.h"
#include "Game_Clock.h"
#include "GameInstance.h"
#include "GameRes.h"
#include "GameMode.h"
#include "Handle_UI.h"
#include "HImage.h"
#include "Isometric_Utils.h"
#include "JA2Types.h"
#include "Keys.h"
#include "LightEffects.h"
#include "Lighting.h"
#include "LoadSaveBasicSoldierCreateStruct.h"
#include "LoadSaveData.h"
#include "LoadSaveLightSprite.h"
#include "LoadSaveSoldierCreate.h"
#include "LoadScreen.h"
#include "Logger.h"
#include "Map_Edgepoints.h"
#include "Map_Information.h"
#include "Meanwhile.h"
#include "OppList.h"
#include "Overhead.h"
#include "Overhead_Map.h"
#include "Overhead_Types.h"
#include "PathAI.h"
#include "Random.h"
#include "Render_Fun.h"
#include "RenderWorld.h"
#include "Rotting_Corpses.h"
#include "Scheduling.h"
#include "SGPFile.h"
#include "SmokeEffects.h"
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "StrategicMap.h"
#include "Structure.h"
#include "Structure_Internals.h"
#include "Summary_Info.h"
#include "Sys_Globals.h"
#include "Tile_Animation.h"
#include "Tile_Surface.h"
#include "TileDat.h"
#include "TileDef.h"
#include "VObject.h"
#include "World_Items.h"
#include "WorldDat.h"
#include "WorldMan.h"
#include <stdexcept>
#include <string>
#include <string_theory/format>


#define SET_MOVEMENTCOST( a, b, c, d )		( ( gubWorldMovementCosts[ a ][ b ][ c ] < d ) ? ( gubWorldMovementCosts[ a ][ b ][ c ] = d ) : 0 );
#define FORCE_SET_MOVEMENTCOST( a, b, c, d )	( gubWorldMovementCosts[ a ][ b ][ c ] = d )
#define SET_CURRMOVEMENTCOST( a, b )		SET_MOVEMENTCOST( usGridNo, a, 0, b )

#define TEMP_FILE_FOR_TILESET_CHANGE "jatiles34.dat"

#define MAP_FULLSOLDIER_SAVED			0x00000001
#define MAP_WORLDLIGHTS_SAVED			0x00000004
#define MAP_WORLDITEMS_SAVED			0x00000008
#define MAP_EXITGRIDS_SAVED			0x00000010
#define MAP_DOORTABLE_SAVED			0x00000020
#define MAP_EDGEPOINTS_SAVED			0x00000040
#define MAP_AMBIENTLIGHTLEVEL_SAVED		0x00000080
#define MAP_NPCSCHEDULES_SAVED			0x00000100


TileSetID giCurrentTilesetID = TILESET_INVALID;

UINT32			gCurrentBackground = FIRSTTEXTURE;


static INT8 gbNewTileSurfaceLoaded[NUMBEROFTILETYPES];


void SetAllNewTileSurfacesLoaded( BOOLEAN fNew )
{
	std::fill(std::begin(gbNewTileSurfaceLoaded), std::end(gbNewTileSurfaceLoaded), fNew);
}


// Global Variables
UINT8						gubWorldMovementCosts[ WORLD_MAX ][MAXDIR][2];

// set to nonzero (locs of base gridno of structure are good) to have it defined by structure code
INT16		gsRecompileAreaTop = 0;
INT16		gsRecompileAreaLeft = 0;
INT16		gsRecompileAreaRight = 0;
INT16		gsRecompileAreaBottom = 0;

/** Check if the grid number is valid. */
static BOOLEAN isValidGridNo(INT32 gridNo)
{
	return (gridNo >= 0) && (gridNo < WORLD_MAX);
}

BOOLEAN DoorAtGridNo(const UINT32 iMapIndex)
{
	return FindStructure(iMapIndex, STRUCTURE_ANYDOOR) != NULL;
}


BOOLEAN OpenableAtGridNo(const UINT32 iMapIndex)
{
	return FindStructure(iMapIndex, STRUCTURE_OPENABLE) != NULL;
}


bool FloorAtGridNo(UINT32 const map_idx)
{
	for (LEVELNODE const* i = gpWorldLevelData[map_idx].pLandHead; i;)
	{
		if (i->usIndex == NO_TILE) continue;

		UINT32 const tile_type = GetTileType(i->usIndex);
		if (FIRSTFLOOR <= tile_type && tile_type <= LASTFLOOR) return true;
		i = i->pNext; // XXX TODO0009 if i->usIndex == NO_TILE this is an endless loop
	}
	return false;
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


void InitializeWorld()
{
	giCurrentTilesetID = TILESET_INVALID;

	// DB Adds the _8 to the names if we're in 8 bit mode.
	//ProcessTilesetNamesForBPP();

	// ATE: MEMSET LOG HEIGHT VALUES
	std::fill(std::begin(gTileTypeLogicalHeight), std::end(gTileTypeLogicalHeight), 1);

	// Memset tile database
	std::fill(std::begin(gTileDatabase), std::end(gTileDatabase), TILE_ELEMENT{});

	// Init surface list
	std::fill(std::begin(gTileSurfaceArray), std::end(gTileSurfaceArray), nullptr);

	// Init default surface list
	std::fill(std::begin(gbDefaultSurfaceUsed), std::end(gbDefaultSurfaceUsed), 0);

	// Init room database
	InitRoomDatabase( );

	// INit tilesets
	InitEngineTilesets( );
}


static void DestroyTileSurfaces(void);


void DeinitializeWorld( )
{
	TrashWorld();

	DestroyTileSurfaces( );
	FreeAllStructureFiles( );

	// Shutdown tile database data
	DeallocateTileDatabase( );
}


static void AddTileSurface(ST::string const& filename, UINT32 const tileType);

TileSetID GetDefaultTileset() {
	return (gubNumTilesets == JA25_NUM_TILESETS) // If we have the number of tilesets for JA25 useJA25 default, else vanilla default
		? DEFAULT_JA25_TILESET
		: GENERIC_1;
}

TILE_SURFACE_RESOURCE GetAdjustedTilesetResource(TileSetID tilesetID, UINT32 uiTileType, ST::string const& filePrefix)
{
	if (tilesetID >= gubNumTilesets) throw std::logic_error("invalid tilesetID");

	ST::string *filename = &gTilesets[tilesetID].zTileSurfaceFilenames[uiTileType];
	if (filename->empty())
	{
		// Try loading from default tileset
		tilesetID = (gubNumTilesets == JA25_NUM_TILESETS && uiTileType == SPECIALTILES)
			? DEFAULT_JA25_TILESET     // If the map is SPECIALTILES (and JA25 tilesets available), use DEFAULT_JA25_TILESET
			: GetDefaultTileset()      // Else use default
		;
		filename = &gTilesets[tilesetID].zTileSurfaceFilenames[uiTileType];
	}

	TILE_SURFACE_RESOURCE res;
	res.tilesetID = tilesetID;
	res.resourceFileName = GCM->getTilesetResourceName(tilesetID, filePrefix + *filename);
	return res;
}

static void LoadTileSurfaces(TileSetID const tileset_id)
try
{
	SetRelativeStartAndEndPercentage(0, 1, 35, "Tile Surfaces");
	for (UINT32 i = 0; i != NUMBEROFTILETYPES; ++i)
	{
		UINT32 const percentage = i * 100 / (NUMBEROFTILETYPES - 1);
		RenderProgressBar(0, percentage);

		auto res = GetAdjustedTilesetResource(tileset_id, i);
		BOOLEAN fUseDefault = res.isDefaultTileset();

		// don't load default surface if already loaded
		if (fUseDefault && gbDefaultSurfaceUsed[i]) continue;

		AddTileSurface(res.resourceFileName, i);

		// OK, if we are the default tileset, set value indicating that!
		gbDefaultSurfaceUsed[i] = fUseDefault;
	}
}
catch (...)
{
	DestroyTileSurfaces();
	throw;
}


static void AddTileSurface(ST::string const& filename, UINT32 const type)
{
	TILE_IMAGERY*& slot = gTileSurfaceArray[type];

	// Delete the surface first!
	if (slot)
	{
		DeleteTileSurface(slot);
		slot = NULL;
	}

	TILE_IMAGERY* const t = LoadTileSurface(filename);
	t->fType = type;
	SetRaisedObjectFlag(filename, t);

	slot = t;

	gbNewTileSurfaceLoaded[type] = TRUE;
}

void BuildTileShadeTables()
{
	for (UINT32 i = 0; i != NUMBEROFTILETYPES; ++i)
	{
		TILE_IMAGERY const* const t = gTileSurfaceArray[i];
		if (!t) continue;

		// Don't create shade tables if default were already used once!
		if(GameMode::getInstance()->isEditorMode())
		{
			if (!gbNewTileSurfaceLoaded[i] && !gfEditorForceShadeTableRebuild) continue;
		}
		else
		{
			if (!gbNewTileSurfaceLoaded[i]) continue;
		}
		RenderProgressBar(0, i * 100 / NUMBEROFTILETYPES);
		CreateTilePaletteTables(t->vo);
	}
}


void DestroyTileShadeTables(void)
{
	for (UINT32 i = 0; i < NUMBEROFTILETYPES; ++i)
	{
		const TILE_IMAGERY* const ti = gTileSurfaceArray[i];
		if (ti == NULL) continue;

		// Don't delete shade tables if default are still being used...
		if(GameMode::getInstance()->isEditorMode())
		{
			if (gbNewTileSurfaceLoaded[i] || gfEditorForceShadeTableRebuild)
			{
				ti->vo->DestroyPalettes();
			}
		}
		else
		{
			if (gbNewTileSurfaceLoaded[i])
			{
				ti->vo->DestroyPalettes();
			}
		}
	}
}


static void DestroyTileSurfaces(void)
{
	FOR_EACH(TILE_IMAGERY*, i, gTileSurfaceArray)
	{
		if (!*i) continue;
		DeleteTileSurface(*i);
		*i = 0;
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

	if(!isValidGridNo(usGridNo))
	{
		return;
	}

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
								if (gubWorldMovementCosts[usGridNo - WORLD_COLS][NORTH][0] < TRAVELCOST_BLOCKED)
								{
									// make sure no obstacle costs exists before changing path cost to 0
									FORCE_SET_MOVEMENTCOST(usGridNo - WORLD_COLS, NORTH, 0, TRAVELCOST_NONE);
								}
								SET_MOVEMENTCOST( usGridNo - WORLD_COLS + 1, NORTHEAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + 1, EAST, 0, TRAVELCOST_OBSTACLE );
								SET_MOVEMENTCOST( usGridNo + WORLD_COLS + 1, SOUTHEAST, 0, TRAVELCOST_OBSTACLE );
								if (gubWorldMovementCosts[usGridNo + WORLD_COLS][SOUTH][0] < TRAVELCOST_BLOCKED)
								{
									FORCE_SET_MOVEMENTCOST(usGridNo + WORLD_COLS, SOUTH, 0, TRAVELCOST_NONE);
								}
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

						if (FindStructure(usGridNo - WORLD_COLS, STRUCTURE_OBSTACLE) == NULL &&
								FindStructure(usGridNo + WORLD_COLS, STRUCTURE_OBSTACLE) == NULL)
						{
							FORCE_SET_MOVEMENTCOST( usGridNo, NORTH, 0, TRAVELCOST_FENCE );
							FORCE_SET_MOVEMENTCOST( usGridNo, SOUTH, 0, TRAVELCOST_FENCE );
						}

						if (FindStructure(usGridNo - 1, STRUCTURE_OBSTACLE) == NULL &&
								FindStructure(usGridNo + 1, STRUCTURE_OBSTACLE) == NULL)
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
			if (isValidGridNo(usGridNo))
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
			CompileTileMovementCosts( usGridNo );
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
				if (isValidGridNo(usGridNo))
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
				CompileTileMovementCosts( usGridNo );
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
	if (!isValidGridNo(sGridNo))
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
			if ( isValidGridNo(usGridNo) && gpWorldLevelData[ usGridNo ].ubExtFlags[0] & MAPELEMENT_EXT_RECALCULATE_MOVEMENT )
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
			CompileTileMovementCosts( usGridNo );
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

	for (auto& i : gubWorldMovementCosts)
	{
		for (auto& j : i)
		{
			std::fill(std::begin(j), std::end(j), 0);
		}
	}

	CompileWorldTerrainIDs();
 	for( usGridNo = 0; usGridNo < WORLD_MAX; usGridNo++ )
	{
		CompileTileMovementCosts( usGridNo );
	}
}


static bool LimitCheck(UINT8 n, INT32 gridno, UINT32& n_warnings, const ST::string& kind)
{
	if (n > 15)
	{
		SetErrorCatchString(
			ST::format("SAVE ABORTED!  {} count too high ({}) for gridno {}.  Need to fix before map can be saved!  There are {} additional warnings.",
			kind, n, gridno, n_warnings));
		return false;
	}
	if (n > 10)
	{
		++n_warnings;
		SetErrorCatchString(
			ST::format("Warnings {} -- Last warning:  {} count warning of {} for gridno {}.",
			n_warnings, kind, n, gridno));
	}
	return true;
}


static void WriteLevelNode(HWFILE const f, LEVELNODE const* const n)
{
	// Write out object type and sub-index
	UINT16 const idx            = n->usIndex;
	UINT32 const type           = GetTileType(idx);
	UINT8  const type_sub_index = (UINT8)GetTypeSubIndexFromTileIndex(type, idx);
	BYTE  data[2];
	DataWriter d{data};
	INJ_U8(d, (UINT8)type)
	INJ_U8(d, (UINT8)type_sub_index)
	Assert(d.getConsumed() == lengthof(data));
	f->write(data, sizeof(data));
}


static void RemoveWorldWireFrameTiles();
static void SaveMapLights(HWFILE);

BOOLEAN SaveWorldToSGPFile(SGPFile* f)
try
{
	// Write JA2 Version ID
	FLOAT mapVersion = getMajorMapVersion();
	f->write(&mapVersion, sizeof(FLOAT));
	if (mapVersion >= 4.00)
	{
		f->write(&gubMinorMapVersion, sizeof(UINT8));
	}

	// Write FLAGS FOR WORLD
	UINT32 flags = 0;
	flags |= MAP_FULLSOLDIER_SAVED;
	flags |= MAP_WORLDLIGHTS_SAVED;
	flags |= MAP_WORLDITEMS_SAVED;
	flags |= MAP_EXITGRIDS_SAVED;
	flags |= MAP_DOORTABLE_SAVED;
	flags |= MAP_EDGEPOINTS_SAVED;
	flags |= MAP_NPCSCHEDULES_SAVED;
	if (gfBasement || gfCaves)
		flags |= MAP_AMBIENTLIGHTLEVEL_SAVED;

	f->write(&flags, sizeof(INT32));

	// Write tileset ID
	f->write(&giCurrentTilesetID, sizeof(INT32));

	// Write soldier control size
	UINT32 const uiSoldierSize = sizeof(SOLDIERTYPE);
	f->write(&uiSoldierSize, sizeof(UINT32));

	// Remove world visibility tiles
	RemoveWorldWireFrameTiles();

	MAP_ELEMENT const* const world_data = gpWorldLevelData;

	{ // Write out height values
		UINT8 heights[2 * WORLD_MAX];
		for (INT32 i = 0; i != WORLD_MAX; ++i)
		{
			heights[2 * i]     = world_data[i].sHeight;
			heights[2 * i + 1] = 0; // Filler byte
		}
		f->write(heights, sizeof(heights));
	}

	// Write out # values - we'll have no more than 15 per level!
	UINT32 n_warnings = 0;
	UINT8  ubCombine;
	for (INT32 cnt = 0; cnt < WORLD_MAX; ++cnt)
	{
		MAP_ELEMENT const& e = world_data[cnt];

		// Determine # of land
		UINT8 n_layers = 0;
		for (LEVELNODE const* i = e.pLandHead; i; i = i->pNext) ++n_layers;
		if (!LimitCheck(n_layers, cnt, n_warnings, "Land")) return FALSE;

		// Combine # of land layers with worlddef flags (first 4 bits)
		ubCombine = (n_layers & 0xf) | ((e.uiFlags & 0xf) << 4);
		f->write(&ubCombine, sizeof(ubCombine));


		// Determine # of objects
		UINT8 n_objects = 0;
		for (LEVELNODE const* i = e.pObjectHead; i; i = i->pNext)
		{
			// DON'T WRITE ANY ITEMS
			if (i->uiFlags & LEVELNODE_ITEM) continue;
			//Make sure this isn't a UI Element
			UINT32 const uiTileType = GetTileType(i->usIndex);
			if (uiTileType >= FIRSTPOINTERS) continue;
			++n_objects;
		}
		if (!LimitCheck(n_objects, cnt, n_warnings, "Object")) return FALSE;

		// Determine # of structs
		UINT8 n_structs = 0;
		for (LEVELNODE const* i = e.pStructHead; i; i = i->pNext)
		{
			// DON'T WRITE ANY ITEMS
			if (i->uiFlags & LEVELNODE_ITEM) continue;
			++n_structs;
		}
		if (!LimitCheck(n_structs, cnt, n_warnings, "Struct")) return FALSE;

		ubCombine = (n_objects & 0xf) | ((n_structs & 0xf) << 4);
		f->write(&ubCombine, sizeof(ubCombine));


		// Determine # of shadows
		UINT8 n_shadows = 0;
		for (LEVELNODE const* i = e.pShadowHead; i; i = i->pNext)
		{
			// Don't write any shadowbuddys or exit grids
			if (i->uiFlags & (LEVELNODE_BUDDYSHADOW  | LEVELNODE_EXITGRID)) continue;
			++n_shadows;
		}
		if (!LimitCheck(n_shadows, cnt, n_warnings, "Shadow")) return FALSE;

		// Determine # of Roofs
		UINT8 n_roofs = 0;
		for (LEVELNODE const* i = e.pRoofHead; i; i = i->pNext)
		{
			// ATE: Don't save revealed roof info...
			if (i->usIndex == SLANTROOFCEILING1) continue;
			++n_roofs;
		}
		if (!LimitCheck(n_roofs, cnt, n_warnings, "Roof")) return FALSE;

		ubCombine = (n_shadows & 0xf) | ((n_roofs & 0xf) << 4);
		f->write(&ubCombine, sizeof(ubCombine));


		// Determine # of OnRoofs
		UINT8 n_on_roofs = 0;
		for (LEVELNODE const* i = e.pOnRoofHead; i; i = i->pNext)
		{
			++n_on_roofs;
		}
		if (!LimitCheck(n_on_roofs, cnt, n_warnings, "OnRoof")) return FALSE;

		// Write combination of onroof and nothing
		ubCombine = n_on_roofs & 0xf;
		f->write(&ubCombine, sizeof(ubCombine));
	}

	if(getMajorMapVersion() == 6.00 && gubMinorMapVersion == 26)
	{
		// the data appears to be 37 INT32/UINT32 numbers and is present in russian ja2 maps
		UINT8 data[148] = {0};
		f->write(&data, sizeof(data));
	}

	UINT8 const test[] = { 1, 1 };
	FOR_EACH_WORLD_TILE(e)
	{ // Write land layers
		LEVELNODE const* i = e->pLandHead;
		if (!i)
		{
			f->write(&test, sizeof(test));
		}
		else
		{ // Write out land pieces backwards so that they are loaded properly
			while (i->pNext) i = i->pNext;
			for (; i; i = i->pPrevNode)
			{
				WriteLevelNode(f, i);
			}
		}
	}

	FOR_EACH_WORLD_TILE(e)
	{ // Write object layer
		for (LEVELNODE const* i = e->pObjectHead; i; i = i->pNext)
		{
			// Don't write any items
			if (i->uiFlags & LEVELNODE_ITEM) continue;

			// Write out object type and sub-index
			UINT32 const type = GetTileType(i->usIndex);
			// Make sure this isn't a UI Element
			if (type >= FIRSTPOINTERS) continue;

			/* We are writing 2 bytes for the type subindex in the object layer
			 * because the ROADPIECES slot contains more than 256 subindices. */
			UINT16 const type_sub_index = GetTypeSubIndexFromTileIndex(type, i->usIndex);

			BYTE  data[3];
			DataWriter d{data};
			INJ_U8( d, (UINT8)type)
			INJ_U16(d, type_sub_index) // XXX misaligned
			Assert(d.getConsumed() == lengthof(data));
			f->write(data, sizeof(data));
		}
	}

	FOR_EACH_WORLD_TILE(e)
	{ // Write struct layer
		for (LEVELNODE const* i = e->pStructHead; i; i = i->pNext)
		{
			// Don't write any items
			if (i->uiFlags & LEVELNODE_ITEM) continue;

			WriteLevelNode(f, i);
		}
	}

	UINT16 n_exit_grids = 0;
	FOR_EACH_WORLD_TILE(e)
	{ // Write shadows
		for (LEVELNODE const* i = e->pShadowHead; i; i = i->pNext)
		{
			// Dont't write any buddys or exit grids
			if (!(i->uiFlags & (LEVELNODE_BUDDYSHADOW | LEVELNODE_EXITGRID)))
			{
				WriteLevelNode(f, i);
			}
			else if (i->uiFlags & LEVELNODE_EXITGRID)
			{	// Count the number of exitgrids
				++n_exit_grids;
			}
		}
	}

	FOR_EACH_WORLD_TILE(e)
	{
		for (LEVELNODE const* i = e->pRoofHead; i; i = i->pNext)
		{
			// ATE: Don't save revealed roof info
			if (i->usIndex == SLANTROOFCEILING1) continue;

			WriteLevelNode(f, i);
		}
	}

	FOR_EACH_WORLD_TILE(e)
	{ // Write OnRoofs
		for (LEVELNODE const* i = e->pOnRoofHead; i; i = i->pNext)
		{
			WriteLevelNode(f, i);
		}
	}

	// Write out room information
	f->write(gubWorldRoomInfo, sizeof(gubWorldRoomInfo));

	if (flags & MAP_WORLDITEMS_SAVED)
	{
		SaveWorldItemsToMap(f);
	}

	if (flags & MAP_AMBIENTLIGHTLEVEL_SAVED)
	{
		f->write(&gfBasement,          1);
		f->write(&gfCaves,             1);
		f->write(&ubAmbientLightLevel, 1);
	}

	if (flags & MAP_WORLDLIGHTS_SAVED)
	{
		SaveMapLights(f);
	}

	SaveMapInformation(f);

	if (flags & MAP_FULLSOLDIER_SAVED)
	{
		SaveSoldiersToMap(f);
	}
	if (flags & MAP_EXITGRIDS_SAVED)
	{
		SaveExitGrids(f, n_exit_grids);
	}
	if (flags & MAP_DOORTABLE_SAVED)
	{
		SaveDoorTableToMap(f);
	}
	if (flags & MAP_EDGEPOINTS_SAVED)
	{
		CompileWorldMovementCosts();
		GenerateMapEdgepoints();
		SaveMapEdgepoints(f);
	}
	if (flags & MAP_NPCSCHEDULES_SAVED)
	{
		SaveSchedules(f);
	}

	return TRUE;
}
catch (...) { return FALSE; }

BOOLEAN SaveWorldAbsolute(const ST::string &absolutePath)
try
{
	AutoSGPFile f(FileMan::openForWriting(absolutePath));
	return SaveWorldToSGPFile(f);
}
catch (...) { return FALSE; }

static void OptimizeMapForShadows()
{
	UINT8 const bDirectionsForShadowSearch[] =
	{
		WEST,
		SOUTHWEST,
		SOUTH,
		SOUTHEAST,
		EAST
	};

	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		// Is there a tree here?
		if (FindStructure(cnt, STRUCTURE_TREE) == NULL) continue;

		// Check for a structure a footprint away
		for (UINT8 const* dir = bDirectionsForShadowSearch;; ++dir)
		{
			if (dir == endof(bDirectionsForShadowSearch))
			{ // We're full of structures
				RemoveAllShadows(cnt);
				break;
			}
			GridNo const gridno = NewGridNo(cnt, DirectionInc(*dir));
			if (!gpWorldLevelData[gridno].pStructureHead) break;
		}
	}
}


static void SetBlueFlagFlags(void)
{
	FOR_EACH_WORLD_TILE(i)
	{
		for (LEVELNODE const* k = i->pStructHead; k; k = k->pNext)
		{
			if (k->usIndex != BLUEFLAG_GRAPHIC) continue;
			i->uiFlags |= MAPELEMENT_PLAYER_MINE_PRESENT;
			break;
		}
	}
}


void InitLoadedWorld(void)
{
	//if the current sector is not valid, dont init the world
	if (!gWorldSector.IsValid())
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


extern double MasterStart, MasterEnd;
extern BOOLEAN gfUpdatingNow;

/* This is a specialty function that is very similar to LoadWorld, except that
 * it doesn't actually load the world, it instead evaluates the map and
 * generates summary information for use within the summary editor.  The header
 * is defined in Summary Info.h, not worlddef.h -- though it's not likely this
 * is going to be used anywhere where it would matter. */
BOOLEAN EvaluateWorld(const ST::string& pSector, const UINT8 ubLevel)
try
{
	// Make sure the file exists... if not, then return false
	ST::string filename = ST::format("{}{}{.0d}{}.dat",
		pSector,
		ubLevel % 4 != 0 ? "_b" : "",
		ubLevel % 4,
		ubLevel     >= 4 ? "_a" : ""
	);

	if (gfMajorUpdate)
	{
		LoadWorld(filename);
		SaveWorldAbsolute(filename);
	}

	AutoSGPFile f(GCM->openMapForReading(filename));

	ST::string str = ST::format("Analyzing map {}", filename);
	if (!gfUpdatingNow)
	{
		SetRelativeStartAndEndPercentage(0, 0, 100, str);
	}
	else
	{
		SetRelativeStartAndEndPercentage(0, (UINT16)MasterStart, (UINT16)MasterEnd, str);
	}

	RenderProgressBar(0, 0);

	//clear the summary file info
	SUMMARYFILE* const pSummary = new SUMMARYFILE{};
	pSummary->ubSummaryVersion = GLOBAL_SUMMARY_VERSION;
	pSummary->dMajorMapVersion = getMajorMapVersion();

	//skip JA2 Version ID
	FLOAT	dMajorMapVersion;
	f->read(&dMajorMapVersion, sizeof(dMajorMapVersion));
	if (dMajorMapVersion >= 4.00)
	{
		f->seek(sizeof(UINT8), FILE_SEEK_FROM_CURRENT);
	}

	//Read FLAGS FOR WORLD
	UINT32 uiFlags;
	f->read(&uiFlags, sizeof(uiFlags));

	//Read tilesetID
	INT32 iTilesetID;
	f->read(&iTilesetID, sizeof(iTilesetID));
	pSummary->ubTilesetID = (UINT8)iTilesetID;

	// Skip soldier size and height values
	f->seek(sizeof(UINT32) + (1 + 1) * WORLD_MAX, FILE_SEEK_FROM_CURRENT);

	// Skip all layers
	INT32 skip = 0;
	for (UINT32 row = 0; row != WORLD_ROWS; ++row)
	{
		if (row % 16 == 0) RenderProgressBar(0, row * 90 / WORLD_ROWS + 1); // 1 - 90

		UINT8 combine[WORLD_COLS][4];
		f->read(combine, sizeof(combine));
		for (UINT8 const (*i)[4] = combine; i != endof(combine); ++i)
		{
			skip +=
				((*i)[0] & 0x0F) * 2 + // #land
				((*i)[1] & 0x0F) * 3 + // #objects
				((*i)[1] >> 4)   * 2 + // #structs
				((*i)[2] & 0x0F) * 2 + // #shadows
				((*i)[2] >> 4)   * 2 + // #roof
				((*i)[3] & 0x0F) * 2;  // #on roof
		}
	}
	f->seek(skip, FILE_SEEK_FROM_CURRENT);

	//extract highest room number
	UINT8 max_room = 0;
	for (INT32 row = 0; row != WORLD_ROWS; ++row)
	{
		UINT8 room[WORLD_COLS];
		f->read(room, sizeof(room));
		for (INT32 col = 0; col != WORLD_COLS; ++col)
		{
			if (max_room < room[col]) max_room = room[col];
		}
	}
	pSummary->ubNumRooms = max_room;

	if (uiFlags & MAP_WORLDITEMS_SAVED)
	{
		RenderProgressBar(0, 91);
		//Important:  Saves the file position (byte offset) of the position where the numitems
		//            resides.  Checking this value and comparing to usNumItems will ensure validity.
		pSummary->uiNumItemsPosition = f->pos();
		//get number of items (for now)
		UINT32 n_items;
		f->read(&n_items, sizeof(n_items));
		pSummary->usNumItems = n_items;
		//Skip the contents of the world items.
		f->seek(sizeof(WORLDITEM) * n_items, FILE_SEEK_FROM_CURRENT);
	}

	if (uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED) f->seek(3, FILE_SEEK_FROM_CURRENT);

	if (uiFlags & MAP_WORLDLIGHTS_SAVED)
	{
		RenderProgressBar(0, 92);

		//skip number of light palette entries
		UINT8 n_light_colours;
		f->read(&n_light_colours, sizeof(n_light_colours));
		f->seek(sizeof(SGPPaletteEntry) * n_light_colours, FILE_SEEK_FROM_CURRENT);

		//get number of lights
		f->read(&pSummary->usNumLights, sizeof(pSummary->usNumLights));
		//skip the light loading
		for (INT32 n = pSummary->usNumLights; n != 0; --n)
		{
			f->seek(24 /* size of a LIGHT_SPRITE on disk */, FILE_SEEK_FROM_CURRENT);
			UINT8 ubStrLen;
			f->read(&ubStrLen, sizeof(ubStrLen));
			f->seek(ubStrLen, FILE_SEEK_FROM_CURRENT);
		}
	}

	//read the mapinformation
	f->read(&pSummary->MapInfo, sizeof(pSummary->MapInfo));

	if (uiFlags & MAP_FULLSOLDIER_SAVED)
	{
		RenderProgressBar(0, 94);

		pSummary->uiEnemyPlacementPosition = f->pos();

		for (INT32 i = 0; i < pSummary->MapInfo.ubNumIndividuals; ++i)
		{
			BASIC_SOLDIERCREATE_STRUCT basic;
			ExtractBasicSoldierCreateStructFromFile(f, basic);

			TEAMSUMMARY* pTeam = NULL;
			switch (basic.bTeam)
			{
				case ENEMY_TEAM:    pTeam = &pSummary->EnemyTeam;    break;
				case CREATURE_TEAM: pTeam = &pSummary->CreatureTeam; break;
				case MILITIA_TEAM:  pTeam = &pSummary->RebelTeam;    break;
				case CIV_TEAM:      pTeam = &pSummary->CivTeam;      break;
			}

			if (basic.bOrders == RNDPTPATROL || basic.bOrders == POINTPATROL)
			{ //make sure the placement has at least one waypoint.
				if (!basic.bPatrolCnt)
				{
					++pSummary->ubEnemiesReqWaypoints;
				}
			}
			else if (basic.bPatrolCnt)
			{
				++pSummary->ubEnemiesHaveWaypoints;
			}

			if (basic.fPriorityExistance) ++pTeam->ubExistance;

			switch (basic.bRelativeAttributeLevel)
			{
				case 0:	++pTeam->ubBadA;   break;
				case 1:	++pTeam->ubPoorA;  break;
				case 2:	++pTeam->ubAvgA;   break;
				case 3:	++pTeam->ubGoodA;  break;
				case 4:	++pTeam->ubGreatA; break;
			}

			switch (basic.bRelativeEquipmentLevel)
			{
				case 0:	++pTeam->ubBadE;   break;
				case 1:	++pTeam->ubPoorE;  break;
				case 2:	++pTeam->ubAvgE;   break;
				case 3:	++pTeam->ubGoodE;  break;
				case 4:	++pTeam->ubGreatE; break;
			}

			SOLDIERCREATE_STRUCT priority;
			if (basic.fDetailedPlacement)
			{ //skip static priority placement

				// Always use windows format because here we are loading a map
				// file, not a user save
				ExtractSoldierCreateFromFile(f, &priority, false);

				if (priority.ubProfile != NO_PROFILE)
					++pTeam->ubProfile;
				else
					++pTeam->ubDetailed;

				if (basic.bTeam == CIV_TEAM)
				{
					if (priority.ubScheduleID) ++pSummary->ubCivSchedules;
					switch (priority.bBodyType)
					{
						case COW:      ++pSummary->ubCivCows; break;
						case BLOODCAT: ++pSummary->ubCivBloodcats; break;
					}
				}
			}

			if (basic.bTeam == ENEMY_TEAM)
			{
				switch (basic.ubSoldierClass)
				{
					case SOLDIER_CLASS_ADMINISTRATOR:
						++pSummary->ubNumAdmins;
						if (basic.fPriorityExistance) ++pSummary->ubAdminExistance;
						if (basic.fDetailedPlacement)
						{
							if (priority.ubProfile != NO_PROFILE)
								++pSummary->ubAdminProfile;
							else
								++pSummary->ubAdminDetailed;
						}
						break;

					case SOLDIER_CLASS_ELITE:
						++pSummary->ubNumElites;
						if (basic.fPriorityExistance) ++pSummary->ubEliteExistance;
						if (basic.fDetailedPlacement)
						{
							if (priority.ubProfile != NO_PROFILE)
								++pSummary->ubEliteProfile;
							else
								++pSummary->ubEliteDetailed;
						}
						break;

					case SOLDIER_CLASS_ARMY:
						++pSummary->ubNumTroops;
						if (basic.fPriorityExistance) ++pSummary->ubTroopExistance;
						if (basic.fDetailedPlacement)
						{
							if (priority.ubProfile != NO_PROFILE)
								++pSummary->ubTroopProfile;
							else
								++pSummary->ubTroopDetailed;
						}
						break;
				}
			}
			else if (basic.bTeam == CREATURE_TEAM)
			{
				if (basic.bBodyType == BLOODCAT) ++pTeam->ubNumAnimals;
			}
			++pTeam->ubTotal;
		}
		RenderProgressBar(0, 96);
	}

	if (uiFlags & MAP_EXITGRIDS_SAVED)
	{
		RenderProgressBar(0, 98);

		UINT16 cnt;
		f->read(&cnt, sizeof(cnt));

		for (INT32 n = cnt; n != 0; --n)
		{
			UINT16 usMapIndex;
			f->read(&usMapIndex, sizeof(usMapIndex));
			EXITGRID exitGrid;
			f->read(&exitGrid, 5 /* XXX sic! The 6th byte luckily is padding */);
			for (INT32 loop = 0;; ++loop)
			{
				if (loop >= pSummary->ubNumExitGridDests)
				{
					if (loop >= 4)
					{
						pSummary->fTooManyExitGridDests = TRUE;
					}
					else
					{
						++pSummary->ubNumExitGridDests;
						++pSummary->usExitGridSize[loop];
						EXITGRID* const eg = &pSummary->ExitGrid[loop];
						eg->usGridNo      = exitGrid.usGridNo;
						eg->ubGotoSector = exitGrid.ubGotoSector;
					}
					break;
				}

				const EXITGRID* const eg = &pSummary->ExitGrid[loop];
				if (eg->usGridNo      == exitGrid.usGridNo      &&
						eg->ubGotoSector == exitGrid.ubGotoSector)
				{ //same destination.
					++pSummary->usExitGridSize[loop];
					break;
				}
			}
		}
	}

	if (uiFlags & MAP_DOORTABLE_SAVED)
	{
		f->read(&pSummary->ubNumDoors, sizeof(pSummary->ubNumDoors));

		for (INT32 n = pSummary->ubNumDoors; n != 0; --n)
		{
			DOOR Door;
			f->read(&Door, sizeof(Door));

			if      (Door.ubLockID && Door.ubTrapID) ++pSummary->ubNumDoorsLockedAndTrapped;
			else if (Door.ubLockID)                  ++pSummary->ubNumDoorsLocked;
			else if (Door.ubTrapID)                  ++pSummary->ubNumDoorsTrapped;
		}
	}

	RenderProgressBar(0, 100);

	WriteSectorSummaryUpdate(filename, ubLevel, pSummary);
	return TRUE;
}
catch (...) { return FALSE; }


static void LoadMapLights(HWFILE);

void LoadWorldFromSGPFile(SGPFile *f);

void LoadWorld(const ST::string &name)
try
{
	AutoSGPFile f(GCM->openMapForReading(name));
	LoadWorldFromSGPFile(f);
}
catch (const std::runtime_error& err)
{
	SET_ERROR(ST::format("Could not load map with name '{}': {}", name, err.what()));
	throw;
}

void LoadWorldAbsolute(const ST::string &absolutePath)
try
{
	AutoSGPFile f{FileMan::openForReading(absolutePath)};
	LoadWorldFromSGPFile(f);
}
catch (const std::runtime_error& err)
{
	SET_ERROR(ST::format("Could not load map from absolute file '{}': {}", absolutePath, err.what()));
	throw;
}

/// Internal load world that reads from sgp file
void LoadWorldFromSGPFile(SGPFile *f)
{
	// Reset flags for outdoors/indoors
	gfBasement = FALSE;
	gfCaves    = FALSE;

	SetRelativeStartAndEndPercentage(0, 0, 1, "Trashing world...");
	TrashWorld();

	LightReset();

	// Read JA2 Version ID
	FLOAT dMajorMapVersion;
	f->read(&dMajorMapVersion, sizeof(dMajorMapVersion));

	UINT8 ubMinorMapVersion;
	if (dMajorMapVersion >= 4.00)
	{
		// major version 4 probably started in minor version 15 since
		// this value is needed to detect the change in the object layer
		f->read(&ubMinorMapVersion, sizeof(ubMinorMapVersion));
	}
	else
	{
		ubMinorMapVersion = 0;
	}

	if (dMajorMapVersion > 6.00 || ubMinorMapVersion > 26)
	{
		throw std::runtime_error("newer versions are not supported");
	}

	// Read flags for world
	UINT32 uiFlags;
	f->read(&uiFlags, sizeof(uiFlags));

	INT32 iTilesetID;
	f->read(&iTilesetID, sizeof(iTilesetID));

	LoadMapTileset(static_cast<TileSetID>(iTilesetID));

	// Skip soldier size
	f->seek(4, FILE_SEEK_FROM_CURRENT);

	{ // Read height values
		MAP_ELEMENT* world = gpWorldLevelData;
		for (UINT32 row = 0; row != WORLD_ROWS; ++row)
		{
			BYTE height[WORLD_COLS * 2];
			f->read(height, sizeof(height));
			for (BYTE const* i = height; i != endof(height); i += 2)
			{
				(world++)->sHeight = *i;
			}
		}
	}

	SetRelativeStartAndEndPercentage(0, 35, 40, "Counting layers...");
	RenderProgressBar(0, 100);

	UINT8 bCounts[WORLD_MAX][6];
	{ // Read layer counts
		UINT8        (*cnt)[6] = bCounts;
		MAP_ELEMENT* world     = gpWorldLevelData;
		for (UINT32 row = 0; row != WORLD_ROWS; ++row)
		{
			BYTE combine[WORLD_COLS][4];
			f->read(combine, sizeof(combine));
			for (UINT8 const (*i)[4] = combine; i != endof(combine); ++world, ++cnt, ++i)
			{
				// Read combination of land/world flags
				(*cnt)[0]       = (*i)[0] & 0x0F;
				world->uiFlags |= (*i)[0] >> 4;

				// Read #objects, structs
				(*cnt)[1] = (*i)[1] & 0x0F;
				(*cnt)[2] = (*i)[1] >> 4;

				// Read shadows, roof
				(*cnt)[3] = (*i)[2] & 0x0F;
				(*cnt)[4] = (*i)[2] >> 4;

				// Read OnRoof, nothing
				(*cnt)[5] = (*i)[3] & 0x0F;
			}
		}
	}

	SetRelativeStartAndEndPercentage(0, 40, 43, "Loading land layers...");
	RenderProgressBar(0, 100);

	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		for (INT32 n = bCounts[cnt][0]; n != 0; --n)
		{
			BYTE data[2];
			f->read(data, sizeof(data));

			UINT8       ubType;
			UINT8       ubSubIndex;
			DataReader d{data};
			EXTR_U8(d, ubType)
			EXTR_U8(d, ubSubIndex)
			Assert(d.getConsumed() == lengthof(data));

			UINT16 const usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);
			AddLandToHead(cnt, usTileIndex);
		}
	}

	SetRelativeStartAndEndPercentage(0, 43, 46, "Loading object layer...");
	RenderProgressBar(0, 100);

	if (ubMinorMapVersion < 15)
	{ // Old loads
		for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
		{
			for (INT32 n = bCounts[cnt][1]; n != 0; --n)
			{
				BYTE data[2];
				f->read(data, sizeof(data));

				UINT8       ubType;
				UINT8       ubSubIndex;
				DataReader d{data};
				EXTR_U8(d, ubType)
				EXTR_U8(d, ubSubIndex)
				Assert(d.getConsumed() == lengthof(data));

				if (ubType >= FIRSTPOINTERS) continue;
				UINT16 const usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);
				AddObjectToTail(cnt, usTileIndex);
			}
		}
	}
	else
	{ /* New load: Require UINT16 for the type subindex due to the fact that
		 * ROADPIECES contains over 300 type subindices. */
		for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
		{
			for (INT32 n = bCounts[cnt][1]; n != 0; --n)
			{
				BYTE data[3];
				f->read(data, sizeof(data));

				UINT8       ubType;
				UINT16      usTypeSubIndex;
				DataReader d{data};
				EXTR_U8( d, ubType)
				EXTR_U16(d, usTypeSubIndex)
				Assert(d.getConsumed() == lengthof(data));

				if (ubType >= FIRSTPOINTERS) continue;
				UINT16 const usTileIndex = GetTileIndexFromTypeSubIndex(ubType, usTypeSubIndex);
				AddObjectToTail(cnt, usTileIndex);
			}
		}
	}

	SetRelativeStartAndEndPercentage(0, 46, 49, "Loading struct layer...");
	RenderProgressBar(0, 100);

	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{ // Set structs
		for (INT32 n = bCounts[cnt][2]; n != 0; --n)
		{
			BYTE data[2];
			f->read(data, sizeof(data));

			UINT8       ubType;
			UINT8       ubSubIndex;
			DataReader d{data};
			EXTR_U8(d, ubType)
			EXTR_U8(d, ubSubIndex)
			Assert(d.getConsumed() == lengthof(data));

			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);

			if (ubMinorMapVersion <= 25)
			{
				// Check patching for phantom menace struct data
				if (gTileDatabase[usTileIndex].uiFlags & UNDERFLOW_FILLER)
				{ /* HACK000F Workaround: Skip underflow fillers, when there is more
					 * than one struct on this tile, otherwise adding the underflow
					 * replacement struct will fail */
					if (bCounts[cnt][2] > 1) continue;

					usTileIndex = GetTileIndexFromTypeSubIndex(ubType, 1);
				}
			}

			try
			{
				AddStructToTail(cnt, usTileIndex);
			}
			catch (FailedToAddNode const&)
			{ /* HACK0010 Workaround: Ignore, because there are defective maps with
				 * overlapping objects */
			}
		}
	}

	SetRelativeStartAndEndPercentage(0, 49, 52, "Loading shadow layer...");
	RenderProgressBar(0, 100);

	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		for (INT32 n = bCounts[cnt][3]; n != 0; --n)
		{
			BYTE data[2];
			f->read(data, sizeof(data));

			UINT8       ubType;
			UINT8       ubSubIndex;
			DataReader d{data};
			EXTR_U8(d, ubType)
			EXTR_U8(d, ubSubIndex)
			Assert(d.getConsumed() == lengthof(data));

			UINT16 const usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);
			AddShadowToTail(cnt, usTileIndex);
		}
	}

	SetRelativeStartAndEndPercentage(0, 52, 55, "Loading roof layer...");
	RenderProgressBar(0, 100);

	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		for (INT32 n = bCounts[cnt][4]; n != 0; --n)
		{
			BYTE data[2];
			f->read(data, sizeof(data));

			UINT8       ubType;
			UINT8       ubSubIndex;
			DataReader d{data};
			EXTR_U8(d, ubType)
			EXTR_U8(d, ubSubIndex)
			Assert(d.getConsumed() == lengthof(data));

			UINT16 const usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);
			AddRoofToTail(cnt, usTileIndex);
		}
	}

	SetRelativeStartAndEndPercentage(0, 55, 58, "Loading on roof layer...");
	RenderProgressBar(0, 100);

	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		for (INT32 n = bCounts[cnt][5]; n != 0; --n)
		{
			BYTE data[2];
			f->read(data, sizeof(data));

			UINT8       ubType;
			UINT8       ubSubIndex;
			DataReader d{data};
			EXTR_U8(d, ubType)
			EXTR_U8(d, ubSubIndex)
			Assert(d.getConsumed() == lengthof(data));

			UINT16 const usTileIndex = GetTileIndexFromTypeSubIndex(ubType, ubSubIndex);
			AddOnRoofToTail(cnt, usTileIndex);
		}
	}

	if(dMajorMapVersion == 6.00 && ubMinorMapVersion == 26)
	{
		// the data appears to be 37 INT32/UINT32 numbers and is present in russian ja2 maps
		f->seek(148, FILE_SEEK_FROM_CURRENT);
	}

	SetRelativeStartAndEndPercentage(0, 58, 59, "Loading room information...");
	RenderProgressBar(0, 100);

	f->read(gubWorldRoomInfo, sizeof(gubWorldRoomInfo));

	if(GameMode::getInstance()->isEditorMode())
	{
		UINT8 max_room_no = 0;
		for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
		{
			if (max_room_no < gubWorldRoomInfo[cnt])
				max_room_no = gubWorldRoomInfo[cnt];
		}
		if (max_room_no < 255) ++max_room_no;
		gubMaxRoomNumber = max_room_no;
	}

	std::fill(std::begin(gubWorldRoomHidden), std::end(gubWorldRoomHidden), TRUE);

	SetRelativeStartAndEndPercentage(0, 59, 61, "Loading items...");
	RenderProgressBar(0, 100);

	if (uiFlags & MAP_WORLDITEMS_SAVED)
	{ // Load out item information
		LoadWorldItemsFromMap(f);
	}

	SetRelativeStartAndEndPercentage(0, 62, 85, "Loading lights...");
	RenderProgressBar(0, 0);

	if (uiFlags & MAP_AMBIENTLIGHTLEVEL_SAVED)
	{ // Ambient light levels are only saved in underground levels
		f->read(&gfBasement,          sizeof(gfBasement));
		f->read(&gfCaves,             sizeof(gfCaves));
		f->read(&ubAmbientLightLevel, sizeof(ubAmbientLightLevel));
	}
	else
	{ // We are above ground.
		gfBasement = FALSE;
		gfCaves    = FALSE;
		if (!gfEditMode)
		{
			ubAmbientLightLevel = GetTimeOfDayAmbientLightLevel();
		}
		else
		{
			ubAmbientLightLevel = 4;
		}
	}
	if (uiFlags & MAP_WORLDLIGHTS_SAVED)
	{
		LoadMapLights(f);
	}
	else
	{ // Set some default value for lighting
		SetDefaultWorldLightingColors();
	}
	LightSetBaseLevel(ubAmbientLightLevel);

	SetRelativeStartAndEndPercentage(0, 85, 86, "Loading map information...");
	RenderProgressBar(0, 0);

	LoadMapInformation(f);

	if (dMajorMapVersion >= 4.00 && gMapInformation.ubMapVersion != ubMinorMapVersion)
	{
		throw std::runtime_error("map version must match minor version");
	}

	if (uiFlags & MAP_FULLSOLDIER_SAVED)
	{
		SetRelativeStartAndEndPercentage(0, 86, 87, "Loading placements...");
		RenderProgressBar(0, 0);
		LoadSoldiersFromMap(f, false);
	}
	if (uiFlags & MAP_EXITGRIDS_SAVED)
	{
		SetRelativeStartAndEndPercentage(0, 87, 88, "Loading exit grids...");
		RenderProgressBar(0, 0);
		LoadExitGrids(f);
	}
	if (uiFlags & MAP_DOORTABLE_SAVED)
	{
		SetRelativeStartAndEndPercentage(0, 89, 90, "Loading door tables...");
		RenderProgressBar(0, 0);
		LoadDoorTableFromMap(f);
	}
	bool generate_edge_points;
	if (uiFlags & MAP_EDGEPOINTS_SAVED)
	{
		SetRelativeStartAndEndPercentage(0, 90, 91, "Loading edgepoints...");
		RenderProgressBar(0, 0);
		// Only if the map had the older edgepoint system
		generate_edge_points = !LoadMapEdgepoints(f);
	}
	else
	{
		generate_edge_points = true;
	}
	if (uiFlags & MAP_NPCSCHEDULES_SAVED)
	{
		SetRelativeStartAndEndPercentage(0, 91, 92, "Loading NPC schedules...");
		RenderProgressBar(0, 0);
		LoadSchedules(f);
	}

	// check unexpected versions
	if (dMajorMapVersion == 6.00 && ubMinorMapVersion == 26)
	{
		// the unknown data is skipped
		SLOGD("Map is a russian ja2 map");
	}
	else if (dMajorMapVersion == 5.00 && ubMinorMapVersion >= 24 && ubMinorMapVersion <= 25)
	{
		SLOGD("Map is a non-russian ja2 map");
	}
	else if (dMajorMapVersion == 5.00 && ubMinorMapVersion == 26)
	{
		// file structure is the same but the game has different items
		SLOGW("Map is a ja2 wildfire map, expect problems");
	}
	else
	{
		// ja2 demo has 3.13
		SLOGW("Map has an unexpected version ({} {}), expect problems", dMajorMapVersion, gMapInformation.ubMapVersion);
	}

	ValidateAndUpdateMapVersionIfNecessary();

	SetRelativeStartAndEndPercentage(0, 93, 94, "Init Loaded World...");
	RenderProgressBar(0, 0);
	InitLoadedWorld();

	if (generate_edge_points)
	{
		SetRelativeStartAndEndPercentage(0, 94, 95, "Generating map edgepoints...");
		RenderProgressBar(0, 0);
		CompileWorldMovementCosts();
		GenerateMapEdgepoints();
	}

	RenderProgressBar(0, 20);

	SetRelativeStartAndEndPercentage(0, 95, 100, "General initialization...");
	// Reset AI!
	InitOpponentKnowledgeSystem();

	RenderProgressBar(0, 30);
	RenderProgressBar(0, 40);

	// Check if our selected guy is gone!
	if (g_selected_man && !g_selected_man->bActive)
	{
		SetSelectedMan(0);
	}

	RenderProgressBar(0, 60);

	InvalidateWorldRedundency();

	RenderProgressBar(0, 80);

	gfWorldLoaded = TRUE;

	GenerateBuildings();

	RenderProgressBar(0, 100);
}


void NewWorld()
{
	SetSelectedMan(0);
	TrashWorld();

	// Create world randomly from tiles
	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		// Set land index
		UINT16 const idx = Random(10);
		AddLandToHead(cnt, idx);
	}

	InitRoomDatabase();
	gfWorldLoaded = TRUE;
}


void FreeLevelNodeList(LEVELNODE** const head)
{
	LEVELNODE* i = *head;
	*head = NULL;

	while (i != NULL)
	{
		LEVELNODE* const next = i->pNext;
		delete i;
		i = next;
	}
}


void TrashWorld(void)
{
	if (!gfWorldLoaded) return;

	TrashWorldItems();
	TrashOverheadMap();

	DecaySmokeEffects(0xffffff, false);
	DecayLightEffects(0xffffff, false);
	ResetSmokeEffects();
	ResetLightEffects();

	// Set soldiers to not active!
	FOR_EACH_SOLDIER(s)
	{
		if (s->bTeam == OUR_TEAM)
		{
			s->pLevelNode = NULL; // Just delete levelnode
		}
		else
		{
			TacticalRemoveSoldier(*s); // Delete from world
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

	FOR_EACH_WORLD_TILE(me)
	{
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
	std::fill_n(gpWorldLevelData, WORLD_MAX, MAP_ELEMENT{});

	// Set some default flags
	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags |= MAPELEMENT_RECALCULATE_WIREFRAMES;
	}

	TrashDoorTable();
	TrashMapEdgepoints();
	TrashDoorStatusArray();

	gfWorldLoaded = FALSE;
}


void LoadMapTileset(TileSetID const id)
{
	if (id >= gubNumTilesets)
	{
		throw std::logic_error("Tried to load tileset with invalid ID");
	}

	// Init tile surface used values
	std::fill(std::begin(gbNewTileSurfaceLoaded), std::end(gbNewTileSurfaceLoaded), 0);

	if (id == giCurrentTilesetID) return;

	LoadTileSurfaces(id);

	// Set terrain costs
	gTilesets[id].MovementCostFnc();

	DeallocateTileDatabase();
	CreateTileDatabase();

	// Set global id for tileset (for saving!)
	giCurrentTilesetID = id;
}


static void AddWireFrame(GridNo const gridno, UINT16 const idx, bool const forced)
{
	for (LEVELNODE* i = gpWorldLevelData[gridno].pTopmostHead; i; i = i->pNext)
	{ // Check if one of the same type exists!
		if (i->usIndex == idx) return;
	}

	LEVELNODE* const n = AddTopmostToTail(gridno, idx);
	if (forced) n->uiFlags |= LEVELNODE_WIREFRAME;
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


static bool IsHiddenTileMarkerThere(GridNo);
static bool IsRoofVisibleForWireframe(GridNo);
static void RemoveWireFrameTiles(GridNo);


void CalculateWorldWireFrameTiles( BOOLEAN fForce )
{
	INT32     cnt;
	STRUCTURE *pStructure;
	INT16     sGridNo;
	UINT8     ubWallOrientation;
	INT8      bNumWallsSameGridNo;
	UINT16    usWireFrameIndex;

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
									AddWireFrame((INT16)cnt, WIREFRAMES4, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
								}
								break;

							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:

								// Get gridno
								sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );

								if ( IsRoofVisibleForWireframe( sGridNo ) && !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
								{
									AddWireFrame((INT16)cnt, WIREFRAMES3, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
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
									AddWireFrame((INT16)cnt, WIREFRAMES2, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
								}
								break;

							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:

								// Get gridno
								sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );

								if ( IsRoofVisibleForWireframe( sGridNo ) && !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) )
								{
									AddWireFrame((INT16)cnt, WIREFRAMES1, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
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

									AddWireFrame((INT16)cnt, usWireFrameIndex, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);

									// Check along our direction to see if we are a corner
									sGridNo = NewGridNo( (INT16)cnt, DirectionInc( WEST ) );
									sGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
									if (!IsHiddenTileMarkerThere(sGridNo))
									{
										// Place corner!
										AddWireFrame((INT16)cnt, WIREFRAMES9, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
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

									AddWireFrame((INT16)cnt, usWireFrameIndex, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);

									// Check along our direction to see if we are a corner
									sGridNo = NewGridNo( (INT16)cnt, DirectionInc( NORTH ) );
									sGridNo = NewGridNo( sGridNo, DirectionInc( EAST ) );
									if (!IsHiddenTileMarkerThere(sGridNo))
									{
										// Place corner!
										AddWireFrame((INT16)cnt, WIREFRAMES8, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
									}

								}
								break;

						}

						// Check for both walls
						if ( bNumWallsSameGridNo == 2 )
						{
							sGridNo = NewGridNo( (INT16)cnt, DirectionInc( EAST ) );
							sGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
							AddWireFrame((INT16)cnt, WIREFRAMES7, (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) != 0);
						}
					}
				}

				pStructure = pStructure->pNext;
			}
		}
	}
}


static void RemoveWorldWireFrameTiles()
{
	for (INT32 cnt = 0; cnt != WORLD_MAX; ++cnt)
	{
		RemoveWireFrameTiles(cnt);
	}
}


static void RemoveWireFrameTiles(GridNo const gridno)
{
	for (LEVELNODE* i = gpWorldLevelData[gridno].pTopmostHead; i;)
	{
		LEVELNODE* const next = i->pNext;

		if (i->usIndex < NUMBEROFTILES &&
				gTileDatabase[i->usIndex].fType == WIREFRAMES)
		{
			RemoveTopmost(gridno, i->usIndex);
		}

		i = next;
	}
}


static bool IsHiddenTileMarkerThere(GridNo const gridno)
{
	return gfBasement || FindStructure(gridno, STRUCTURE_ROOF);
}


void ReloadTileset(TileSetID const ubID)
{
	TileSetID const iCurrTilesetID = giCurrentTilesetID;

	// Set gloabal
	giCurrentTilesetID = ubID;

	// Save Map
	AutoSGPFile f1(GCM->tempFiles()->openForWriting(TEMP_FILE_FOR_TILESET_CHANGE, true));
	SaveWorldToSGPFile(f1);
	f1.Deallocate();

	//IMPORTANT:  If this is not set, the LoadTileset() will assume that
	//it is loading the same tileset and ignore it...
	giCurrentTilesetID = iCurrTilesetID;

	// Load Map with new tileset
	AutoSGPFile f2(GCM->tempFiles()->openForReading(TEMP_FILE_FOR_TILESET_CHANGE));
	LoadWorldFromSGPFile(f2);
	f2.Deallocate();

	// Delete file
	GCM->tempFiles()->deleteFile(TEMP_FILE_FOR_TILESET_CHANGE);
}


BOOLEAN IsSoldierLight(const LIGHT_SPRITE* const l)
{
	CFOR_EACH_SOLDIER(s)
	{
		if (s->light == l) return TRUE;
	}
	return FALSE;
}


static void SaveMapLights(HWFILE hfile)
{
	UINT16 usNumLights = 0;

	// Save the current light colors!
	const UINT8 ubNumColors = 1;
	hfile->write(&ubNumColors, 1);
	const SGPPaletteEntry* LColor = LightGetColor();
	hfile->write(LColor, sizeof(*LColor));

	//count number of non-merc lights.
	CFOR_EACH_LIGHT_SPRITE(l)
	{
		if (!IsSoldierLight(l)) ++usNumLights;
	}

	//save the number of lights.
	hfile->write(&usNumLights, 2);

	CFOR_EACH_LIGHT_SPRITE(l)
	{
		if (!IsSoldierLight(l)) InjectLightSpriteIntoFile(hfile, l);
	}
}


static void LoadMapLights(HWFILE const f)
{
	SGPPaletteEntry	LColors[3];
	UINT8 ubNumColors;
	UINT16 usNumLights;

	//reset the lighting system, so that any current lights are toasted.
	LightReset();

	// read in the light colors!
	f->read(&ubNumColors, sizeof(ubNumColors));
	f->read(LColors,      sizeof(*LColors) * ubNumColors); // XXX buffer overflow if ubNumColors is too large

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

	f->read(&usNumLights, sizeof(usNumLights));
	for (INT32 cnt = 0; cnt < usNumLights; ++cnt)
	{
		ExtractLightSprite(f, light_time);
	}
}


static bool IsRoofVisibleForWireframe(GridNo const sMapPos)
{
	return gfBasement || FindStructure(sMapPos, STRUCTURE_ROOF);
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(WorldDef, asserts)
{
	EXPECT_EQ(sizeof(TEAMSUMMARY), 15u);
	EXPECT_EQ(sizeof(SUMMARYFILE), 416u);
}

#endif
