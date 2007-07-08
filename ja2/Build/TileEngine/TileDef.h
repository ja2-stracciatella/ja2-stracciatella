#ifndef __TILEDEF_H
#define __TILEDEF_H

#include "VObject.h"
#include "TileDat.h"
#include "Structure.h"

// CATEGORY TYPES
#define NO_TILE									64000
#define ERASE_TILE							65000
#define REQUIRES_SMOOTHING_TILE	19
#define NUM_WALL_ORIENTATIONS		40

#define	WALL_TILE								0x00000001
#define ANIMATED_TILE						0x00000002
#define DYNAMIC_TILE						0x00000004
#define IGNORE_WORLD_HEIGHT			0x00000008
#define ROAD_TILE								0x00000010
#define FULL3D_TILE							0x00000020
#define MULTI_Z_TILE						0x00000080
#define OBJECTLAYER_USEZHEIGHT	0x00000100
#define	ROOFSHADOW_TILE					0x00000200
#define	ROOF_TILE								0x00000400
#define	TRANSLUCENT_TILE				0x00000800
#define HAS_SHADOW_BUDDY				0x00001000
#define AFRAME_TILE							0x00002000
#define HIDDEN_TILE							0x00004000
#define CLIFFHANG_TILE					0x00008000
#define UNDERFLOW_FILLER        0x00010000


#define MAX_ANIMATED_TILES			200
#define	WALL_HEIGHT		50


//Kris:  Added the last two bottom corner orientation values.  This won't effect
//current code, but there is new code that makes use of this.  A function called
//UINT8 CalculateWallOrientationsAtGridNo( INT32 iMapIndex ) that will look at all
//of the walls and return the last two wall orientations for tiles with two proper
//wall pieces.
enum WallOrientationDefines
{
	NO_ORIENTATION, INSIDE_TOP_LEFT, INSIDE_TOP_RIGHT, OUTSIDE_TOP_LEFT,
	OUTSIDE_TOP_RIGHT, INSIDE_BOTTOM_CORNER, OUTSIDE_BOTTOM_CORNER
};

// TERRAIN ID VALUES.
enum
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

} TerrainTypeDefines;



// These structures are placed in a list and used for all tile imagery
typedef struct
{
	HVOBJECT									vo;
	UINT32										fType;
  AuxObjectData *						pAuxData;
	RelTileLoc *							pTileLocData;
	STRUCTURE_FILE_REF *			pStructureFileRef;
	UINT8											ubTerrainID;
	BYTE											bRaisedObjectType;

	// Reserved for added room and 32-byte boundaries
	BYTE													bReserved[ 2 ];

} TILE_IMAGERY, *PTILE_IMAGERY;

typedef struct
{
	UINT16							*pusFrames;
	INT8								bCurrentFrame;
	UINT8								ubNumFrames;

} TILE_ANIMATION_DATA;


// Tile data element
typedef struct
{
	UINT16							fType;
	HVOBJECT						hTileSurface;
	DB_STRUCTURE_REF *	pDBStructureRef;
	UINT32							uiFlags;
	RelTileLoc *				pTileLocData;
	UINT16							usRegionIndex;
	INT16								sBuddyNum;
	UINT8								ubTerrainID;
	UINT8								ubNumberOfTiles;

	UINT8								bZOffsetX;
	UINT8								bZOffsetY;

	// This union contains different data based on tile type
	union
	{
		// Land and overlay type
		struct
		{
			 INT16					sOffsetHeight;
			 UINT16					usWallOrientation;
			 UINT8					ubFullTile;

			 // For animated tiles
			 TILE_ANIMATION_DATA	*pAnimData;
		};

	};

	// Reserved for added room and 32-byte boundaries
	BYTE													bReserved[ 3 ];


} TILE_ELEMENT, *PTILE_ELEMENT;


// Globals used
TILE_ELEMENT		gTileDatabase[ NUMBEROFTILES ];
UINT16					gTileDatabaseSize;
UINT16					gTileTypeStartIndex[ NUMBEROFTILETYPES ];

UINT16					gusNumAnimatedTiles;
UINT16					gusAnimatedTiles[ MAX_ANIMATED_TILES ];
UINT8					gTileTypeMovementCost[ NUM_TERRAIN_TYPES ];

void CreateTileDatabase( );



// Land level manipulation functions
BOOLEAN GetLandHeadType( INT32 iMapIndex, UINT32 *puiType );

BOOLEAN SetLandIndex(INT32 iMapIndex, UINT16 usIndex, UINT32 uiNewType);

BOOLEAN GetTypeLandLevel( UINT32 iMapIndex, UINT32 uiNewType, UINT8 *pubLevel );


// Database access functions
BOOLEAN GetSubIndexFromTileIndex( UINT16 usIndex, UINT16 *pusSubIndex );
BOOLEAN GetTypeSubIndexFromTileIndex( UINT32 uiCheckType, UINT16 usIndex, UINT16 *pusSubIndex );
BOOLEAN GetTypeSubIndexFromTileIndexChar( UINT32 uiCheckType, UINT16 usIndex, UINT8 *pusSubIndex );
BOOLEAN	GetTileIndexFromTypeSubIndex( UINT32 uiCheckType, UINT16 usSubIndex, UINT16 *pusTileIndex );
BOOLEAN	GetTileType( UINT16 usIndex, UINT32 *puiType );
BOOLEAN	GetTileFlags( UINT16 usIndex, UINT32 *puiFlags );

UINT8 GetTileTypeLogicalHeight(UINT32 fType);
BOOLEAN AnyHeigherLand( UINT32 iMapIndex, UINT32 uiSrcType, UINT8 *pubLastLevel );
UINT16 GetWallOrientation(UINT16 usIndex);

void	SetSpecificDatabaseValues( UINT16 usType, UINT16 uiDatabaseElem, TILE_ELEMENT *TileElement, BOOLEAN fUseRaisedObjectType );

BOOLEAN AllocateAnimTileData( TILE_ELEMENT *pTileElem, UINT8 ubNumFrames );
void DeallocateTileDatabase( );

#endif
