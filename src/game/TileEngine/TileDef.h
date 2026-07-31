#ifndef __TILEDEF_H
#define __TILEDEF_H

#include "JA2Types.h"
#include "TileDat.h"
#include <memory>
#include <optional>

// CATEGORY TYPES
#define NO_TILE				64000
#define REQUIRES_SMOOTHING_TILE		19

enum TileElementFlags
{
	WALL_TILE              = 1U <<  0,
	ANIMATED_TILE          = 1U <<  1,
	DYNAMIC_TILE           = 1U <<  2,
	IGNORE_WORLD_HEIGHT    = 1U <<  3,
	FULL3D_TILE            = 1U <<  4,
	MULTI_Z_TILE           = 1U <<  5,
	OBJECTLAYER_USEZHEIGHT = 1U <<  6,
	ROOFSHADOW_TILE        = 1U <<  7,
	ROOF_TILE              = 1U <<  8,
	HAS_SHADOW_BUDDY       = 1U <<  9,
	AFRAME_TILE            = 1U << 10,
	CLIFFHANG_TILE         = 1U << 11,
	UNDERFLOW_FILLER       = 1U << 12
};
ENUM_BITSET(TileElementFlags)


#define MAX_ANIMATED_TILES		200
#define WALL_HEIGHT			50
// Fixes projectiles penetrating roof surfaces unimpededly
#define ROOF_HIT_ADJUSTMENT			2


enum Orientation : uint8_t
{
	ORIENT_NONE          = 0x00,
	ORIENT_INSIDE_LEFT   = 0x01,
	ORIENT_INSIDE_RIGHT  = 0x02,
	ORIENT_OUTSIDE_LEFT  = 0x04,
	ORIENT_OUTSIDE_RIGHT = 0x08,

	// Combination flags
	ORIENT_LEFT          = 0x05,
	ORIENT_RIGHT         = 0x0A,
	ORIENT_INSIDE        = 0x03,
	ORIENT_OUTSIDE       = 0x0C,
};
ENUM_BITSET(Orientation)

// Assumes the flag always has only one bit set to 1
static inline uint8_t GetBitPositionIndex(const uint8_t flag)
{
	return ( 0x34572610U >> ((((flag * 29) >> 5) & 7) * 4) ) & 7;
}

// TERRAIN ID VALUES.
enum TerrainTypeDefines : UINT8
{
	NO_TERRAIN,
	FLAT_GROUND,
	FLAT_FLOOR,
	PAVED_ROAD,
	DIRT_ROAD,
	LOW_GRASS,
	HIGH_GRASS,
	TRAIN_TRACKS,
	LOW_WATER,
	MED_WATER,
	DEEP_WATER,
	NUM_TERRAIN_TYPES
};


// These structures are placed in a list and used for all tile imagery
struct TILE_IMAGERY
{
	HVOBJECT           vo;
	AuxObjectData      *pAuxData;
	RelTileLoc         *pTileLocData;
	std::unique_ptr<STRUCTURE_FILE_REF> pStructureFileRef;
	TileTypeDefines    fType;
	TerrainTypeDefines ubTerrainID;
	bool               bRaisedObjectType;
};


// Tile data element
struct TILE_ELEMENT
{
	HVOBJECT             hTileSurface;
	DB_STRUCTURE_REF*    pDBStructureRef;
	RelTileLoc*          pTileLocData;
	TileElementFlags     uiFlags;
	UINT16               fType;
	UINT16               usRegionIndex;
	INT16                sBuddyNum;
	TerrainTypeDefines   ubTerrainID;
	UINT8                ubNumberOfTiles;

	// Land and overlay type
	Orientation          usWallOrientation;
	UINT8                ubFullTile;

	// For animated tiles
	INT8                 bCurrentFrame;
	UINT8                ubNumFrames;
	UINT16*              pusFrames;
};


// Globals used
extern TILE_ELEMENT gTileDatabase[NUMBEROFTILES];
extern UINT16       gTileTypeStartIndex[NUMBEROFTILETYPES];

static inline const TILE_ELEMENT* TileElemFromTileType(const UINT16 tile_type)
{
	return &gTileDatabase[gTileTypeStartIndex[tile_type]];
}

extern UINT16 gusNumAnimatedTiles;
extern UINT16 gusAnimatedTiles[MAX_ANIMATED_TILES];
extern UINT8  gTileTypeMovementCost[NUM_TERRAIN_TYPES];

void CreateTileDatabase(void);



// Land level manipulation functions

void SetLandIndex(INT32 iMapIndex, UINT16 usIndex, UINT32 uiNewType);

bool GetTypeLandLevel(UINT32 map_idx, UINT32 new_type, UINT8* out_level);

TerrainTypeDefines GetTerrainType(GridNo);
bool Water(TerrainTypeDefines);

// Database access functions
UINT16 GetSubIndexFromTileIndex(UINT16 usIndex);

UINT16 GetTypeSubIndexFromTileIndex(UINT32 uiCheckType, UINT16 usIndex);

UINT16 GetTileIndexFromTypeSubIndex(UINT32 uiCheckType, UINT16 usSubIndex);
UINT32 GetTileType(UINT16 usIndex);
std::optional<UINT32> GetTileTypeSafe(UINT16 tileIndex);
UINT32 GetTileFlags(UINT16 usIndex);

UINT8 GetTileTypeLogicalHeight(UINT32 type);
bool  AnyHeigherLand(UINT32 map_idx, UINT32 src_type, UINT8* out_last_level);
Orientation GetWallOrientation(UINT16 usIndex);

void SetSpecificDatabaseValues(UINT16 type, UINT16 database_elem, TILE_ELEMENT&, bool use_raised_object_type);

void AllocateAnimTileData(TILE_ELEMENT* pTileElem, UINT8 ubNumFrames);
void DeallocateTileDatabase(void);

#endif
