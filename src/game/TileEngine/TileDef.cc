#include <optional>
#include <stdexcept>

#include "HImage.h"
#include "Structure_Internals.h"
#include "TileDef.h"
#include "VObject.h"
#include "WorldDef.h"
#include "Debug.h"
#include "WorldMan.h"
#include "PathAI.h"
#include "Tile_Surface.h"
#include "Logger.h"
#include "Structure.h"

// GLobals
TILE_ELEMENT		gTileDatabase[ NUMBEROFTILES ];
static UINT16 gTileDatabaseSize;
UINT16					gusNumAnimatedTiles = 0;
UINT16					gusAnimatedTiles[ MAX_ANIMATED_TILES ];



UINT16						gTileTypeStartIndex[ NUMBEROFTILETYPES ];

// These values coorespond to TerrainTypeDefines order
UINT8		gTileTypeMovementCost[ NUM_TERRAIN_TYPES ] =
{
	TRAVELCOST_FLAT,						// NO_TERRAIN
	TRAVELCOST_FLAT,						// FLAT GROUND
	TRAVELCOST_FLATFLOOR,				// FLAT FLOOR
	TRAVELCOST_PAVEDROAD,				// PAVED ROAD
	TRAVELCOST_DIRTROAD,				// DIRT ROAD
	TRAVELCOST_FLAT,						// LOW_GRASS
	TRAVELCOST_FLAT,						// HIGH GRASS
	TRAVELCOST_TRAINTRACKS,			// TRAIN TRACKS
	TRAVELCOST_SHORE,						// LOW WATER
	TRAVELCOST_SHORE,						// MED WATER
	TRAVELCOST_SHORE,						// DEEP WATER
};


void CreateTileDatabase()
{
	// Loop through all surfaces and tiles and build database
	for (UINT32 cnt1 = 0; cnt1 < NUMBEROFTILETYPES; ++cnt1)
	{
		TILE_IMAGERY const* const TileSurf = gTileSurfaceArray[cnt1];
		if (!TileSurf) continue;

		// Build start index list
		gTileTypeStartIndex[cnt1] = (UINT16)gTileDatabaseSize;

		UINT32 NumRegions = TileSurf->vo->SubregionCount();

		// Handle overflow
		if (NumRegions > gNumTilesPerType[cnt1])
		{
			NumRegions = gNumTilesPerType[cnt1];
		}

		SLOGD("Type: {} Size: {} Index: {}", gTileSurfaceName[cnt1], gNumTilesPerType[cnt1], gTileDatabaseSize);

		UINT32 cnt2;
		for (cnt2 = 0; cnt2 < NumRegions; ++cnt2)
		{
			TILE_ELEMENT TileElement{};
			TileElement.usRegionIndex = (UINT16)cnt2;
			TileElement.hTileSurface	= TileSurf->vo;
			TileElement.sBuddyNum			= -1;

			// Check for multi-z stuff
			auto const& zsi = TileSurf->vo->ppZStripInfo;
			if (zsi && zsi[cnt2]) TileElement.uiFlags |= MULTI_Z_TILE;

			// Structure database stuff!
			auto const& sfr{ TileSurf->pStructureFileRef };
			if (sfr && !sfr->pubStructureData.empty())
			{
				auto & sr = sfr->pDBStructureRef[cnt2];
				if (sr.pDBStructure) TileElement.pDBStructureRef = &sr;
			}

			TileElement.fType             = (UINT16)TileSurf->fType;
			TileElement.ubTerrainID       = TileSurf->ubTerrainID;
			TileElement.usWallOrientation = NO_ORIENTATION;

			if (TileSurf->pAuxData)
			{
				AuxObjectData const& aux = TileSurf->pAuxData[cnt2];
				if (aux.fFlags & AUX_FULL_TILE)
				{
					TileElement.ubFullTile = 1;
				}
				if (aux.fFlags & AUX_ANIMATED_TILE)
				{
					AllocateAnimTileData(&TileElement, aux.ubNumberOfFrames);

					TileElement.pAnimData->bCurrentFrame = aux.ubCurrentFrame;
					for (UINT8 ubLoop = 0; ubLoop < TileElement.pAnimData->ubNumFrames; ++ubLoop)
					{
						TileElement.pAnimData->pusFrames[ubLoop] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
					}

					// set into animation controller array
					Assert(gusNumAnimatedTiles < lengthof(gusAnimatedTiles));
					gusAnimatedTiles[gusNumAnimatedTiles++] = gTileDatabaseSize;

					TileElement.uiFlags |= ANIMATED_TILE;
				}
				TileElement.usWallOrientation = aux.ubWallOrientation;
				if (aux.ubNumberOfTiles > 0)
				{
					TileElement.ubNumberOfTiles = aux.ubNumberOfTiles;
					TileElement.pTileLocData    = TileSurf->pTileLocData + aux.usTileLocIndex;
				}
			}

			SetSpecificDatabaseValues(cnt1, gTileDatabaseSize, TileElement, TileSurf->bRaisedObjectType);
			// fix incorrect double door flags. in vanilla it only affects DOOR3 in PALACE! tileset and DOOR1 in QUEEN'S TROPICAL
			if ((gTileSurfaceName[cnt1] == "DOOR1" || gTileSurfaceName[cnt1] == "DOOR2" || gTileSurfaceName[cnt1] == "DOOR3" || gTileSurfaceName[cnt1] == "DOOR4")
				&& TileElement.pDBStructureRef != nullptr)
			{
				if (TileElement.usRegionIndex == 0 && (TileElement.pDBStructureRef->pDBStructure->fFlags & (STRUCTURE_DDOOR_RIGHT|STRUCTURE_DDOOR_LEFT)))
				{
					// if a door in an open state takes up 1 tile and is flagged as outside-oriented...
					if (TileElement.usWallOrientation == OUTSIDE_TOP_RIGHT && TileElement.pDBStructureRef[4].pDBStructure->ubNumberOfTiles == 1)
					{
						// ... we found our problematic tile surface to be fixed
						TileElement.usWallOrientation = INSIDE_TOP_RIGHT;
						TileElement.pDBStructureRef->pDBStructure->ubWallOrientation = INSIDE_TOP_RIGHT;
						TileElement.pDBStructureRef[4].pDBStructure->ubWallOrientation = INSIDE_TOP_RIGHT;
						TileElement.pDBStructureRef[4].pDBStructure->ubArmour = MATERIAL_PLYWOOD_WALL;
						TileElement.pDBStructureRef[5].pDBStructure->ubWallOrientation = INSIDE_TOP_LEFT;
						TileElement.pDBStructureRef[9].pDBStructure->ubWallOrientation = INSIDE_TOP_LEFT;
						// between subindices 0-4 and 10-14 there can only be 1 right part of a double door
						TileElement.pDBStructureRef[10].pDBStructure->fFlags &= ~STRUCTURE_DDOOR_RIGHT;
						TileElement.pDBStructureRef[10].pDBStructure->fFlags |= STRUCTURE_DDOOR_LEFT;
						TileElement.pDBStructureRef[14].pDBStructure->fFlags &= ~STRUCTURE_DDOOR_RIGHT;
						TileElement.pDBStructureRef[14].pDBStructure->fFlags |= STRUCTURE_DDOOR_LEFT;
						// between subindices 5-9 and 15-19 there can only be 1 left part of a double door
						TileElement.pDBStructureRef[15].pDBStructure->fFlags &= ~STRUCTURE_DDOOR_LEFT;
						TileElement.pDBStructureRef[15].pDBStructure->fFlags |= STRUCTURE_DDOOR_RIGHT;
						TileElement.pDBStructureRef[19].pDBStructure->fFlags &= ~STRUCTURE_DDOOR_LEFT;
						TileElement.pDBStructureRef[19].pDBStructure->fFlags |= STRUCTURE_DDOOR_RIGHT;
					}					
				}
			}			
			gTileDatabase[gTileDatabaseSize++] = TileElement;
		}

		// Handle underflow
		for (; cnt2 < gNumTilesPerType[cnt1]; ++cnt2)
		{
			TILE_ELEMENT TileElement{};
			TileElement.usRegionIndex  = 0;
			TileElement.hTileSurface   = TileSurf->vo;
			TileElement.fType          = (UINT16)TileSurf->fType;
			TileElement.ubFullTile     = 0;
			TileElement.uiFlags       |= UNDERFLOW_FILLER;

			gTileDatabase[gTileDatabaseSize++] = TileElement;
		}
	}

	//Calculate mem usgae
	SLOGD("Database Sizes: {} vs {}", gTileDatabaseSize, NUMBEROFTILES);
	SLOGD("Database Types: {}", NUMBEROFTILETYPES);
	SLOGD("Database Item Mem: {}", gTileDatabaseSize * sizeof(TILE_ELEMENT));
}


static void FreeAnimTileData(TILE_ELEMENT* pTileElem);


void DeallocateTileDatabase( )
{
	INT32 cnt;

	for( cnt = 0; cnt < NUMBEROFTILES; cnt++ )
	{
		// Check if an existing set of animated tiles are in place, remove if found
		if ( gTileDatabase[ cnt ].pAnimData != NULL )
		{
			FreeAnimTileData( &gTileDatabase[ cnt ] );
		}
	}

	gTileDatabaseSize = 0;
	gusNumAnimatedTiles = 0;
}


void SetLandIndex(INT32 const iMapIndex, UINT16 const usIndex, UINT32 const uiNewType)
{
	UINT8  ubLastHighLevel;
	if (LEVELNODE const* const land = FindTypeInLandLayer(iMapIndex, uiNewType))
	{
		ReplaceLandIndex(iMapIndex, land->usIndex, usIndex);
	}
	else if (AnyHeigherLand(iMapIndex, uiNewType, &ubLastHighLevel))
	{
		InsertLandIndexAtLevel(iMapIndex, usIndex, ubLastHighLevel + 1);
	}
	else
	{
		AddLandToHead(iMapIndex, usIndex);
	}
}


bool GetTypeLandLevel(UINT32 const map_idx, UINT32 const new_type, UINT8* const out_level)
{
	UINT8 level = 0;
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pLandHead; i; ++level, i = i->pNext)
	{
		if (i->usIndex              == NO_TILE)  continue;
		if (GetTileType(i->usIndex) != new_type) continue;
		*out_level = level;
		return true;
	}
	return false;
}


UINT16 GetSubIndexFromTileIndex(const UINT16 usTileIndex)
{
	const UINT32 uiType = GetTileType(usTileIndex);
	return usTileIndex - gTileTypeStartIndex[uiType] + 1;
}


UINT16 GetTypeSubIndexFromTileIndex(UINT32 const uiCheckType, UINT16 const usIndex)
{
	// Tile database is zero-based, Type indecies are 1-based!
	if (uiCheckType >= NUMBEROFTILETYPES)
	{
		throw std::logic_error("Tried to get sub-index from invalid tile type");
	}
	return usIndex - gTileTypeStartIndex[uiCheckType] + 1;
}



UINT16 GetTileIndexFromTypeSubIndex(UINT32 uiCheckType, UINT16 usSubIndex)
{
	Assert(uiCheckType < NUMBEROFTILETYPES);
	// Tile database is zero-based, Type indices are 1-based!
	return usSubIndex + gTileTypeStartIndex[uiCheckType] - 1;
}


// Database access functions
UINT32 GetTileType(const UINT16 usIndex)
{
	Assert(usIndex < lengthof(gTileDatabase));
	return gTileDatabase[usIndex].fType;
}

std::optional<UINT32> GetTileTypeSafe(UINT16 tileIndex)
{
	if (tileIndex < std::size(gTileDatabase))
	{
		return { gTileDatabase[tileIndex].fType };
	}
	return std::nullopt;
}


UINT32 GetTileFlags(const UINT16 usIndex)
{
	Assert(usIndex < lengthof(gTileDatabase));
	return gTileDatabase[usIndex].uiFlags;
}


UINT8 GetTileTypeLogicalHeight(UINT32 const type)
{
	Assert(type < lengthof(gTileTypeLogicalHeight));
	return gTileTypeLogicalHeight[type];
}


bool AnyHeigherLand(UINT32 const map_idx, UINT32 const src_type, UINT8* const out_last_level)
{
	// Check that src type is not head
	UINT8 src_type_level = 0;
	if (GetTypeLandLevel(map_idx, src_type, &src_type_level) &&
			src_type_level == LANDHEAD)
	{
		return false;
	}

	UINT8 level          = 0;
	bool  found          = false;
	UINT8 src_log_height = GetTileTypeLogicalHeight(src_type);
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pLandHead; i; ++level, i = i->pNext)
	{
		// Get type and height
		UINT32 const tile_type = GetTileType(i->usIndex);
		if (GetTileTypeLogicalHeight(tile_type) > src_log_height)
		{
			*out_last_level = level;
			found = TRUE;
		}
	}
	return found;
}

UINT16 GetWallOrientation(UINT16 usIndex)
{
	Assert(usIndex < lengthof(gTileDatabase));
	return gTileDatabase[usIndex].usWallOrientation;
}


void AllocateAnimTileData(TILE_ELEMENT* const pTileElem, UINT8 const ubNumFrames)
{
	pTileElem->pAnimData            = new TILE_ANIMATION_DATA{};
	pTileElem->pAnimData->pusFrames = new UINT16[ubNumFrames]{};

	// Set # if frames!
	pTileElem->pAnimData->ubNumFrames = ubNumFrames;
}


static void FreeAnimTileData(TILE_ELEMENT* pTileElem)
{
	if ( pTileElem->pAnimData != NULL )
	{
		// Free frames list
		delete[] pTileElem->pAnimData->pusFrames;

		// Free frames
		delete pTileElem->pAnimData;
	}
}
