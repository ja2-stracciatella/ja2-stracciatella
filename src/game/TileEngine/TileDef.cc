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
			TILE_ELEMENT TileElement;
			TileElement = TILE_ELEMENT{};
			TileElement.usRegionIndex = (UINT16)cnt2;
			TileElement.hTileSurface	= TileSurf->vo;
			TileElement.sBuddyNum			= -1;

			// Check for multi-z stuff
			ZStripInfo* const* const zsi = TileSurf->vo->ppZStripInfo;
			if (zsi && zsi[cnt2]) TileElement.uiFlags |= MULTI_Z_TILE;

			// Structure database stuff!
			STRUCTURE_FILE_REF const* const sfr = TileSurf->pStructureFileRef;
			if (sfr && sfr->pubStructureData /* XXX testing wrong attribute? */)
			{
				DB_STRUCTURE_REF* const sr = &sfr->pDBStructureRef[cnt2];
				if (sr->pDBStructure) TileElement.pDBStructureRef	= sr;
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

			gTileDatabase[gTileDatabaseSize++] = TileElement;
		}

		// Handle underflow
		for (; cnt2 < gNumTilesPerType[cnt1]; ++cnt2)
		{
			TILE_ELEMENT TileElement;
			TileElement = TILE_ELEMENT{};
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
