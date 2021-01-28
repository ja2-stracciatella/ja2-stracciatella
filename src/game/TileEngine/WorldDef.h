#ifndef __WORLDDEF_H
#define __WORLDDEF_H

#include "Types.h"
#include "World_Tileset_Enums.h"

#include <string_theory/string>

struct ANITILE;
struct ITEM_POOL;
struct LIGHT_SPRITE;
struct SOLDIERTYPE;
struct STRUCTURE;

#define WORLD_TILE_X		40
#define WORLD_TILE_Y		20
#define WORLD_COLS		160
#define WORLD_ROWS		160
#define WORLD_COORD_COLS	1600
#define WORLD_COORD_ROWS	1600
#define WORLD_MAX		25600
#define CELL_X_SIZE		10
#define CELL_Y_SIZE		10

#define HALF_TILE_WIDTH		((WORLD_TILE_X / CELL_X_SIZE) / 2)	// equals to 2
#define HALF_TILE_HEIGHT	((WORLD_TILE_Y / CELL_Y_SIZE) / 2)	// equals to 1

#define CENTER_GRIDNO (WORLD_ROWS / 2 * WORLD_COLS + WORLD_COLS / 2)

#define WORLD_CLIFF_HEIGHT	80

//A macro that actually memcpy's over data and increments the pointer automatically
//based on the size.  Works like a FileRead except with a buffer instead of a file pointer.
//Used by LoadWorld() and child functions.
#define  LOADDATA( dst, src, size ) memcpy( dst, src, size ); src += size


#define LANDHEAD		0
#define MAXDIR			8

// Defines for shade levels
#define DEFAULT_SHADE_LEVEL	4


// DEFINES FOR LEVELNODE FLAGS
enum LevelnodeFlags
{
	LEVELNODE_NONE                 = 0,
	LEVELNODE_SOLDIER              = 0x00000001,
	LEVELNODE_MERCPLACEHOLDER      = 0x00000004,
	LEVELNODE_SHOW_THROUGH         = 0x00000008,
	LEVELNODE_NOZBLITTER           = 0x00000010,
	LEVELNODE_CACHEDANITILE        = 0x00000020,
	LEVELNODE_ROTTINGCORPSE        = 0x00000040,
	LEVELNODE_BUDDYSHADOW          = 0x00000080,
	LEVELNODE_HIDDEN               = 0x00000100,
	LEVELNODE_USERELPOS            = 0x00000200,
	LEVELNODE_DISPLAY_AP           = 0x00000400,
	LEVELNODE_ANIMATION            = 0x00000800,
	LEVELNODE_USEABSOLUTEPOS       = 0x00001000,
	LEVELNODE_REVEAL               = 0x00002000,
	LEVELNODE_REVEALTREES          = 0x00004000,
	LEVELNODE_USEZ                 = 0x00010000,
	LEVELNODE_DYNAMICZ             = 0x00020000,
	LEVELNODE_UPDATESAVEBUFFERONCE = 0x00040000,
	LEVELNODE_WIREFRAME            = 0x00100000,
	LEVELNODE_ITEM                 = 0x00200000,
	LEVELNODE_IGNOREHEIGHT         = 0x00400000,
	LEVELNODE_DYNAMIC              = 0x02000000,
	LEVELNODE_LASTDYNAMIC          = 0x04000000,
	LEVELNODE_PHYSICSOBJECT        = 0x08000000,
	LEVELNODE_EXITGRID             = 0x40000000,
	LEVELNODE_CAVE                 = 0x80000000
};
ENUM_BITSET(LevelnodeFlags)


// THE FIRST FEW ( 4 ) bits are flags which are saved in the world
#define MAPELEMENT_REDUNDENT			0x0001
#define MAPELEMENT_REEVALUATE_REDUNDENCY	0x0002
#define MAPELEMENT_ENEMY_MINE_PRESENT		0x0004
#define MAPELEMENT_PLAYER_MINE_PRESENT		0x0008
#define MAPELEMENT_STRUCTURE_DAMAGED		0x0010
#define MAPELEMENT_REEVALUATEBLOOD		0x0020
#define MAPELEMENT_INTERACTIVETILE		0x0040
#define MAPELEMENT_RAISE_LAND_START		0x0080
#define MAPELEMENT_REVEALED			0x0100
#define MAPELEMENT_RAISE_LAND_END		0x0200
#define MAPELEMENT_REDRAW			0x0400
#define MAPELEMENT_REVEALED_ROOF		0x0800
#define MAPELEMENT_MOVEMENT_RESERVED		0x1000
#define MAPELEMENT_RECALCULATE_WIREFRAMES	0x2000
#define MAPELEMENT_ITEMPOOL_PRESENT		0x4000
#define MAPELEMENT_REACHABLE			0x8000

#define MAPELEMENT_EXT_SMOKE			0x01
#define MAPELEMENT_EXT_TEARGAS			0x02
#define MAPELEMENT_EXT_MUSTARDGAS		0x04
#define MAPELEMENT_EXT_DOOR_STATUS_PRESENT	0x08
#define MAPELEMENT_EXT_RECALCULATE_MOVEMENT	0x10
#define MAPELEMENT_EXT_NOBURN_STRUCT		0x20
#define MAPELEMENT_EXT_ROOFCODE_VISITED		0x40
#define MAPELEMENT_EXT_CREATUREGAS		0x80

#define FIRST_LEVEL				0
#define SECOND_LEVEL				1

#define FIRST_LEVEL_Z_OFFET 0.0
#define SECOND_LEVEL_Z_OFFSET 58.0

#define ANY_SMOKE_EFFECT			( MAPELEMENT_EXT_CREATUREGAS | MAPELEMENT_EXT_SMOKE | MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS )


struct LEVELNODE
{
	LEVELNODE*     pNext;
	LevelnodeFlags uiFlags;

	UINT8 ubSumLights; // LIGHTING INFO
	UINT8 ubMaxLights; // MAX LIGHTING INFO

	union
	{
		LEVELNODE *pPrevNode; // FOR LAND, GOING BACKWARDS POINTER
		STRUCTURE *pStructureData; // STRUCTURE DATA
		INT32     uiAPCost; // FOR AP DISPLAY
		INT32     iExitGridInfo;
	}; // ( 4 byte union )

	union
	{
		struct
		{
			UINT16 usIndex; // TILE DATABASE INDEX
			INT16 sCurrentFrame; // Stuff for animated tiles for a given tile location ( doors, etc )
		};

		SOLDIERTYPE *pSoldier; // POINTER TO SOLDIER

	}; // ( 4 byte union )

	union
	{
		// Some levelnodes can specify relative X and Y values!
		struct
		{
			INT16 sRelativeX; // Relative position values
			INT16 sRelativeY; // Relative position values
		};

		struct
		{
			UINT32 uiAnimHitLocationFlags;	// Animation profile flags for soldier placeholders ( prone merc hit location values )
		};

		// Some can contains index values into animated tile data
		struct
		{
			ANITILE* pAniTile;
		};

		// Can be an item pool as well...
		struct
		{
			ITEM_POOL *pItemPool; // ITEM POOLS
		};


	};

	INT16 sRelativeZ; // Relative position values
	UINT8 ubShadeLevel; // LIGHTING INFO
	UINT8 ubNaturalShadeLevel; // LIGHTING INFO
	UINT8 ubFakeShadeLevel; // LIGHTING INFO
};


#define LAND_START_INDEX			1
#define OBJECT_START_INDEX			2
#define STRUCT_START_INDEX			3
#define SHADOW_START_INDEX			4
#define MERC_START_INDEX			5
#define ROOF_START_INDEX			6
#define ONROOF_START_INDEX			7
#define TOPMOST_START_INDEX			8


struct MAP_ELEMENT
{
	union
	{
		struct
		{
			LEVELNODE *pLandHead; //0
			LEVELNODE *pLandStart; //1

			LEVELNODE *pObjectHead; //2

			LEVELNODE *pStructHead; //3

			LEVELNODE *pShadowHead; //4

			LEVELNODE *pMercHead; //5

			LEVELNODE *pRoofHead; //6

			LEVELNODE *pOnRoofHead; //7

			LEVELNODE *pTopmostHead; //8
		};

		LEVELNODE *pLevelNodes[ 9 ];
	};

	STRUCTURE *pStructureHead;
	STRUCTURE *pStructureTail;

	UINT16 uiFlags;
	UINT8 ubExtFlags[2];
	UINT8 sHeight;
	UINT8 ubAdjacentSoldierCnt;
	UINT8 ubTerrainID;

	UINT8 ubReservedSoldierID;
	UINT8 ubBloodInfo;
	UINT8 ubSmellInfo;
};

/**
 * Specifies where to load a tile surface.
 */
struct TILE_SURFACE_RESOURCE
{
	ST::string resourceFileName;  // the exact resource file name on the VFS
	TileSetID tilesetID;  // the actual Tileset ID, after considering default tileset

	// whether or not the default tileset is being used
	BOOLEAN isDefaultTileset()
	{
		return (tilesetID == GENERIC_1 || tilesetID == DEFAULT_JA25_TILESET);
	}
};

// World Data
extern MAP_ELEMENT* gpWorldLevelData;

#define FOR_EACH_WORLD_TILE(iter) \
	for (MAP_ELEMENT* iter = gpWorldLevelData, * const iter##__end = gpWorldLevelData + WORLD_MAX; iter != iter##__end; ++iter)

// World Movement Costs
extern UINT8 gubWorldMovementCosts[WORLD_MAX][MAXDIR][2];


extern TileSetID giCurrentTilesetID;

extern INT16 gsRecompileAreaTop;
extern INT16 gsRecompileAreaLeft;
extern INT16 gsRecompileAreaRight;
extern INT16 gsRecompileAreaBottom;

void InitializeWorld(void);
void DeinitializeWorld(void);

void BuildTileShadeTables(void);
void DestroyTileShadeTables(void);


void TrashWorld(void);

/* Deletes everything then re-creates the world with simple ground tiles */
void NewWorld(void);

BOOLEAN SaveWorld(const char *puiFilename);

void LoadWorld(char const* filename);
void CompileWorldMovementCosts(void);
void RecompileLocalMovementCosts( INT16 sCentreGridNo );
void RecompileLocalMovementCostsFromRadius( INT16 sCentreGridNo, INT8 bRadius );

TileSetID GetDefaultTileset();
// Tilesets may not provide all tile types.If a tile type is not available in a tileset, we fall back and use the surface in the default tileset.
TILE_SURFACE_RESOURCE GetAdjustedTilesetResource(TileSetID tilesetID, UINT32 uiTileType, ST::string const& filePrefix = "");

void LoadMapTileset(TileSetID);

void CalculateWorldWireFrameTiles( BOOLEAN fForce );

void ReloadTileset(TileSetID);

bool FloorAtGridNo(UINT32 map_idx);
BOOLEAN DoorAtGridNo( UINT32 iMapIndex );
BOOLEAN GridNoIndoors( UINT32 iMapIndex );


BOOLEAN OpenableAtGridNo( UINT32 iMapIndex );

void RecompileLocalMovementCostsInAreaWithFlags( void );
void AddTileToRecompileArea( INT16 sGridNo );

void InitLoadedWorld(void);

BOOLEAN IsSoldierLight(const LIGHT_SPRITE* l);

void FreeLevelNodeList(LEVELNODE** const head);

void SetAllNewTileSurfacesLoaded(BOOLEAN fNew);

void RecompileLocalMovementCostsForWall(INT16 sGridNo, UINT8 ubOrientation);

#endif
