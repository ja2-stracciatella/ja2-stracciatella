#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "NewSmooth.h"
#include "Isometric_Utils.h"
#include "EditorBuildings.h"
#include "Edit_Sys.h"
#include "Smoothing_Utils.h"
#include "Editor_Undo.h"
#include "Environment.h"
#include "Random.h"
#include "Render_Fun.h"
#include "Debug.h"


static bool CaveAtGridNo(INT32 const map_idx)
{
	if (map_idx < 0 || NOWHERE <= map_idx) return true;

	if (FindStructure(map_idx, STRUCTURE_CAVEWALL)) return true;

	// May not have structure information, so check if there is a levelnode flag
	for (LEVELNODE const* i = gpWorldLevelData[map_idx].pStructHead; i; i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CAVE) return true;
	}
	return false;
}


static UINT16 GetCaveTileIndexFromPerimeterValue(UINT8 ubTotal)
{
	UINT16 usType = FIRSTWALL;
	UINT16 usIndex; // HACK000E

	switch( ubTotal )
	{
		case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70:
		case 0x80: case 0x90: case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			//usIndex = 0;
			//break;
			return 0xffff;
		case 0x01: case 0x11: case 0x21: case 0x31: case 0x41: case 0x51: case 0x61: case 0x71:
		case 0x81: case 0x91: case 0xa1: case 0xb1: case 0xc1: case 0xd1: case 0xe1: case 0xf1:
			usType = SECONDWALL;
			usIndex = 1 + (UINT16)Random( 4 );
			break;
		case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: case 0x62: case 0x72:
		case 0x82: case 0x92: case 0xa2: case 0xb2: case 0xc2: case 0xd2: case 0xe2: case 0xf2:
			usType = SECONDWALL;
			usIndex = 5 + (UINT16)Random( 4 );
			break;
		case 0x03: case 0x13: case 0x43: case 0x53: case 0x83: case 0x93: case 0xc3: case 0xd3:
			usIndex = 1;
			break;
		case 0x04: case 0x14: case 0x24: case 0x34: case 0x44: case 0x54: case 0x64: case 0x74:
		case 0x84: case 0x94: case 0xa4: case 0xb4: case 0xc4: case 0xd4: case 0xe4: case 0xf4:
			usType = SECONDWALL;
			usIndex = 9 + (UINT16)Random( 4 );
			break;
		case 0x05: case 0x15: case 0x25: case 0x35: case 0x45: case 0x55: case 0x65: case 0x75:
		case 0x85: case 0x95: case 0xa5: case 0xb5: case 0xc5: case 0xd5: case 0xe5: case 0xf5:
			usIndex = 2 + (UINT16)Random( 2 );
			break;
		case 0x06: case 0x16: case 0x26: case 0x36: case 0x86: case 0x96: case 0xa6: case 0xb6:
			usIndex = 4;
			break;
		case 0x07: case 0x17: case 0x87: case 0x97:
			usIndex = 5;
			break;
		case 0x08: case 0x18: case 0x28: case 0x38: case 0x48: case 0x58: case 0x68: case 0x78:
		case 0x88: case 0x98: case 0xa8: case 0xb8: case 0xc8: case 0xd8: case 0xe8: case 0xf8:
			usType = SECONDWALL;
			usIndex = 13 + (UINT16)Random( 4 );
			break;
		case 0x09: case 0x29: case 0x49: case 0x69: case 0x89: case 0xa9: case 0xc9: case 0xe9:
			usIndex = 6;
			break;
		case 0x0a: case 0x1a: case 0x2a: case 0x3a: case 0x4a: case 0x5a: case 0x6a: case 0x7a:
		case 0x8a: case 0x9a: case 0xaa: case 0xba: case 0xca: case 0xda: case 0xea: case 0xfa:
			usIndex = 7 + (UINT16)Random( 2 );
			break;
		case 0x0b: case 0x4b: case 0x8b: case 0xcb:
			usIndex = 9;
			break;
		case 0x0c: case 0x1c: case 0x2c: case 0x3c: case 0x4c: case 0x5c: case 0x6c: case 0x7c:
			usIndex = 10;
			break;
		case 0x0d: case 0x2d: case 0x4d: case 0x6d:
			usIndex = 11;
			break;
		case 0x0e: case 0x1e: case 0x2e: case 0x3e:
			usIndex = 12;
			break;
		case 0x0f:
			usIndex = 13;
			break;
		case 0x19: case 0x39: case 0x59: case 0x79: case 0x99: case 0xb9: case 0xd9: case 0xf9:
			usIndex = 14 + (UINT16)Random( 2 );
			break;
		case 0x1b: case 0x5b: case 0x9b: case 0xdb:
			usIndex = 16;
			break;
		case 0x1d: case 0x3d: case 0x5d: case 0x7d:
			usIndex = 17;
			break;
		case 0x1f:
			usIndex = 18;
			break;
		case 0x23: case 0x33: case 0x63: case 0x73: case 0xa3: case 0xb3: case 0xe3: case 0xf3:
			usIndex = 19 + (UINT16)Random( 2 );
			break;
		case 0x27: case 0x37: case 0xa7: case 0xb7:
			usIndex = 21;
			break;
		case 0x2b: case 0x6b: case 0xab: case 0xeb:
			usIndex = 22;
			break;
		case 0x2f:
			usIndex = 23;
			break;
		case 0x3b: case 0x7b: case 0xbb: case 0xfb:
			usIndex = 24 + (UINT16)Random( 3 );
			break;
		case 0x3f:
			usIndex = 27;
			break;
		case 0x46: case 0x56: case 0x66: case 0x76: case 0xc6: case 0xd6: case 0xe6: case 0xf6:
			usIndex = 28 + (UINT16)Random( 2 );
			break;
		case 0x47: case 0x57: case 0xc7: case 0xd7:
			usIndex = 30;
			break;
		case 0x4e: case 0x5e: case 0x6e: case 0x7e:
			usIndex = 31;
			break;
		case 0x4f:
			usIndex = 32;
			break;
		case 0x5f:
			usIndex = 33;
			break;
		case 0x67: case 0x77: case 0xe7: case 0xf7:
			usIndex = 34 + (UINT16)Random( 3 );
			break;
		case 0x6f:
			usIndex = 37;
			break;
		case 0x7f:
			usIndex = 38 + (UINT16)Random( 2 );
			break;
		case 0x8c: case 0x9c: case 0xac: case 0xbc: case 0xcc: case 0xdc: case 0xec: case 0xfc:
			usIndex = 40 + (UINT16)Random( 2 );
			break;
		case 0x8d: case 0xad: case 0xcd: case 0xed:
			usIndex = 42;
			break;
		case 0x8e: case 0x9e: case 0xae: case 0xbe:
			usIndex = 43;
			break;
		case 0x8f:
			usIndex = 44;
			break;
		case 0x9d: case 0xbd: case 0xdd: case 0xfd:
			usIndex = 45 + (UINT16)Random( 3 );
			break;
		case 0x9f:
			usIndex = 48;
			break;
		case 0xaf:
			usIndex = 49;
			break;
		case 0xbf:
			usIndex = 50 + (UINT16)Random( 2 );
			break;
		case 0xce: case 0xde: case 0xee: case 0xfe:
			usIndex = 52 + (UINT16)Random( 3 );
			break;
		case 0xcf:
			usIndex = 55;
			break;
		case 0xdf:
			usIndex = 56 + (UINT16)Random( 2 );
			break;
		case 0xef:
			usIndex = 58 + (UINT16)Random( 2 );
			break;
		case 0xff:
			usIndex = 60 + (UINT16)Random( 6 );
			break;

		default: abort(); // XXX HACK000E
	}
	return GetTileIndexFromTypeSubIndex(usType, usIndex);
}

//  16 | 1 | 32
//  ---+---+---
//   8 |   | 2
//  ---+---+---
//  128| 4 | 64
//These values are combined in any possible order ranging in
//values from 0 - 255.  If there is a cave existing in any of
//these bordering gridnos, then the corresponding number is added
//to this total.  The lookup table has been precalculated to know
//which piece to use for all of these combinations.  In many cases,
//up to 16 combinations can share the same graphic image, as corners
//may not effect the look of the piece.
static UINT8 CalcNewCavePerimeterValue(INT32 iMapIndex)
{
	UINT8 ubTotal = 0;
	if( CaveAtGridNo( iMapIndex - WORLD_COLS ) )
		ubTotal += 0x01;	//north
	if( CaveAtGridNo( iMapIndex + 1 ) )
		ubTotal += 0x02;	//east
	if( CaveAtGridNo( iMapIndex + WORLD_COLS ) )
		ubTotal += 0x04;	//south
	if( CaveAtGridNo( iMapIndex - 1 ) )
		ubTotal += 0x08;	//west
	if( CaveAtGridNo( iMapIndex - WORLD_COLS - 1 ) )
		ubTotal += 0x10;	//north west
	if( CaveAtGridNo( iMapIndex - WORLD_COLS + 1 ) )
		ubTotal += 0x20;	//north east
	if( CaveAtGridNo( iMapIndex + WORLD_COLS + 1 ) )
		ubTotal += 0x40;	//south east
	if( CaveAtGridNo( iMapIndex + WORLD_COLS - 1 ) )
		ubTotal += 0x80;	//south west
	return ubTotal;
}

void AddCave( INT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE *pStruct;

	if( iMapIndex < 0 || iMapIndex >= GRIDSIZE )
		return;
	//First toast any existing wall (caves)
	RemoveAllStructsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
	//Now, add this piece
	if( !AddWallToStructLayer( iMapIndex, usIndex, true ) )
		return;
	//Set the cave flag
	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;
	Assert( pStruct );
	pStruct->uiFlags |= LEVELNODE_CAVE;
}

//These walls have shadows associated with them, and are draw when the wall is drawn.
#define EXTERIOR_L_SHADOW_INDEX		30
#define INTERIOR_BOTTOMEND_SHADOW_INDEX	31

//Wall Look Up Table containing variants and indices with each row being a different walltype.
static const INT8 gbWallTileLUT[NUM_WALL_TYPES][7] =
{
//  The number of variants of this tile type.
//  |   The first relative index of the wall type (FIRSTWALL, SECONDWALL, etc. )  walltype + 10
//  |   |   The 2nd relative index  ( walltype + 11 )
//  |   |   |   3rd 4th 5th 6th
//  |   |   |   |   |   |   |
	{ 6, 10, 11, 12, 27, 28, 29 }, // INTERIOR_L
	{ 6,  7,  8,  9, 24, 25, 26 }, // INTERIOR_R
	{ 6,  4,  5,  6, 21, 22, 23 }, // EXTERIOR_L
	{ 6,  1,  2,  3, 18, 19, 20 }, // EXTERIOR_R
	{ 1, 14,  0,  0,  0,  0,  0 }, // INTERIOR_CORNER
	{ 1, 15,  0,  0,  0,  0,  0 }, // INTERIOR_BOTTOMEND
	{ 1, 13,  0,  0,  0,  0,  0 }, // EXTERIOR_BOTTOMEND
	{ 1, 16,  0,  0,  0,  0,  0 }, // INTERIOR_EXTENDED
	{ 1, 57,  0,  0,  0,  0,  0 }, // EXTERIOR_EXTENDED
	{ 1, 56,  0,  0,  0,  0,  0 }, // INTERIOR_EXTENDED_BOTTOMEND
	{ 1, 17,  0,  0,  0,  0,  0 }  // EXTERIOR_EXTENDED_BOTTOMEND
};

//Roof table -- such a small table, using definitions instead.
#define TOP_ROOF_INDEX		2
#define BOTTOM_ROOF_INDEX	4
#define LEFT_ROOF_INDEX	1
#define RIGHT_ROOF_INDEX	5
#define TOPLEFT_ROOF_INDEX	3
#define TOPRIGHT_ROOF_INDEX	7
#define BOTTOMLEFT_ROOF_INDEX	8
#define BOTTOMRIGHT_ROOF_INDEX	6
#define CENTER_ROOF_BASE_INDEX	9
#define CENTER_ROOF_VARIANTS	3

//slant roof table
#define THIN_BOTTOM		1
#define THIN_TOP		2
#define THIN_LEFT		5
#define THIN_RIGHT		6
#define THICK_BOTTOM		3
#define THICK_TOP		4
#define THICK_LEFT		7
#define THICK_RIGHT		8
#define VWALL_LEFT		32
#define VWALL_RIGHT		33
#define HWALL_LEFT		35
#define HWALL_RIGHT		34


#define FLOOR_VARIANTS		8


//These construction functions do all the smoothing.
//NOTE:  passing null for wall/roof type will force the function to search for the nearest
//  existing respective type.
static void BuildSlantRoof(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom, UINT16 usWallType, UINT16 usRoofType, BOOLEAN fVertical)
{
	INT32 i;
	INT32 iMapIndex;
	if( fVertical )
	{
		iMapIndex = iBottom * WORLD_COLS + iLeft;
		//This happens to be the only mapindex that needs to be backed up.  The rest have already been
		//done because of the building code before this.
		AddToUndoList( iMapIndex + 8 );
		//Add the closest viewable pieces.  There are two aframe walls pieces, and extended aframe roof pieces.
		UINT16 usTileIndex;
		usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, VWALL_LEFT);
		AddRoofToHead( iMapIndex + 4, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, VWALL_RIGHT);
		AddRoofToHead( iMapIndex + 8, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_LEFT);
		AddRoofToHead( iMapIndex + 3, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_RIGHT);
		AddRoofToHead( iMapIndex + 7, usTileIndex );
		for( i = iBottom - 1; i > iTop; i-- )
		{
			iMapIndex -= WORLD_COLS;
			usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THIN_LEFT);
			AddRoofToHead( iMapIndex + 3, usTileIndex );
			usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THIN_RIGHT);
			AddRoofToHead( iMapIndex + 7, usTileIndex );
		}
		iMapIndex -= WORLD_COLS;
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_LEFT);
		AddRoofToHead( iMapIndex + 3, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_RIGHT);
		AddRoofToHead( iMapIndex + 7, usTileIndex );
	}
	else
	{
		iMapIndex = iTop * WORLD_COLS + iRight;
		//This happens to be the only mapindex that needs to be backed up.  The rest have already been
		//done because of the building code before this.
		AddToUndoList( iMapIndex + 8*WORLD_COLS );
		//Add the closest viewable pieces.  There are two aframe walls pieces, and extended aframe roof pieces.
		UINT16 usTileIndex;
		usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, HWALL_LEFT);
		AddRoofToHead( iMapIndex + 4*WORLD_COLS, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, HWALL_RIGHT);
		AddRoofToHead( iMapIndex + 8*WORLD_COLS, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_TOP);
		AddRoofToHead( iMapIndex + 3*WORLD_COLS, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_BOTTOM);
		AddRoofToHead( iMapIndex + 7*WORLD_COLS, usTileIndex );
		for( i = iRight - 1; i > iLeft; i-- )
		{
			iMapIndex--;
			usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THIN_TOP);
			AddRoofToHead( iMapIndex + 3*WORLD_COLS, usTileIndex );
			usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THIN_BOTTOM);
			AddRoofToHead( iMapIndex + 7*WORLD_COLS, usTileIndex );
		}
		iMapIndex--;
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_TOP);
		AddRoofToHead( iMapIndex + 3*WORLD_COLS, usTileIndex );
		usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, THICK_BOTTOM);
		AddRoofToHead( iMapIndex + 7*WORLD_COLS, usTileIndex );
	}
}

UINT16 PickAWallPiece( UINT16 usWallPieceType )
{
	UINT16 usVariants;
	UINT16 usVariantChosen;
	UINT16 usWallPieceChosen = 0;
	if (usWallPieceType < NUM_WALL_TYPES)
	{
		usVariants = gbWallTileLUT[ usWallPieceType ][ 0 ];
		usVariantChosen = Random(usVariants) + 1;
		usWallPieceChosen = gbWallTileLUT[ usWallPieceType ][ usVariantChosen ];
	}
	return usWallPieceChosen;
}

//From a given gridNo and perspective (wallpiece), it will calculate the new piece, and
//where to place it as well as handle the special cases.
//NOTE:  Placing top and left pieces are placed relative to the gridno, and the gridNo will
// shift accordingly to place the piece.  Pretend you are the floor, and you want to place a piece to
// the left.  You pass your position, and INTERIOR_LEFT, with interior meaning from the inside of a
// building.  If you were outside the building, you would call EXTERIOR_LEFT.  The left tile will be
// placed on gridNo - 1!  Up tiles will be placed on gridNo - 160.
//NOTE:  Passing NULL for usWallType will force it to calculate the closest existing wall type, and
//  use that for building this new wall.  It is necessary for restructuring a building, but not for
//  adding on to an existing building, where the type is already known.
void BuildWallPiece( UINT32 iMapIndex, UINT8 ubWallPiece, UINT16 usWallType )
{
	INT16 sIndex;
	UINT16 ubWallClass; // XXX HACK000E
	LEVELNODE *pStruct;
	if( !usWallType )
	{
		usWallType = SearchForWallType( iMapIndex );
	}
	switch( ubWallPiece )
	{
		case EXTERIOR_TOP:
			iMapIndex -= WORLD_COLS;
			//exterior bottom left corner generated
			if ( !gfBasement && GetVerticalWall( iMapIndex - 1) && !GetVerticalWall( iMapIndex + WORLD_COLS - 1) )
			{	//Special case where a shadow has to be created as it now is a bottom corner and
				//must contribute to the bottom shadow.
				AddToUndoList( iMapIndex - 1 );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, INTERIOR_BOTTOMEND_SHADOW_INDEX);
				AddExclusiveShadow( iMapIndex - 1, usTileIndex );
			}
			if ( (pStruct = GetVerticalWall( iMapIndex )) ) //right corner
			{	//Special case where placing the new wall will generate a corner to the right, so replace
				//the vertical piece with a bottomend.
				sIndex = PickAWallPiece( EXTERIOR_BOTTOMEND );
				AddToUndoList( iMapIndex );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
				ReplaceStructIndex( iMapIndex, pStruct->usIndex, usTileIndex );
			}
			ubWallClass = EXTERIOR_L;
			if( !gfBasement )
			{
				//All exterior_l walls have shadows.
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, EXTERIOR_L_SHADOW_INDEX);
				AddExclusiveShadow( iMapIndex, usTileIndex );
			}
			break;
		case EXTERIOR_BOTTOM:
			ubWallClass = INTERIOR_L;
			if( (pStruct = GetVerticalWall( iMapIndex + WORLD_COLS - 1 )) && !GetVerticalWall( iMapIndex - 1) )
			{
				sIndex = PickAWallPiece( INTERIOR_EXTENDED );
				AddToUndoList( iMapIndex + WORLD_COLS - 1 );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
				ReplaceStructIndex( iMapIndex + WORLD_COLS - 1, pStruct->usIndex, usTileIndex );
			}
			break;
		case EXTERIOR_LEFT:
			iMapIndex--;
			ubWallClass = EXTERIOR_R;
			if( GetHorizontalWall( iMapIndex ) )
			{	//Special case where placing the new wall will generate a corner.  This piece
				//becomes an exterior bottomend, but nothing else is effected.
				ubWallClass = EXTERIOR_BOTTOMEND;
			}
			if( GetHorizontalWall( iMapIndex - WORLD_COLS + 1 ) )
			{
				if( ubWallClass == EXTERIOR_BOTTOMEND )
					ubWallClass = EXTERIOR_EXTENDED_BOTTOMEND;
				else
					ubWallClass = EXTERIOR_EXTENDED;
			}
			break;
		case EXTERIOR_RIGHT:
			ubWallClass = INTERIOR_R;
			if( GetHorizontalWall( iMapIndex - WORLD_COLS + 1 ) && !GetHorizontalWall( iMapIndex - WORLD_COLS ) )
			{
				ubWallClass = INTERIOR_EXTENDED;
			}
			else if( GetHorizontalWall( iMapIndex ) && !GetVerticalWall( iMapIndex + WORLD_COLS ) )
			{
				ubWallClass = INTERIOR_BOTTOMEND;
			}
			break;
		case INTERIOR_TOP:
			iMapIndex -= WORLD_COLS;
			ubWallClass = INTERIOR_L;
			//check for a lower left corner.
			if( ((pStruct = GetVerticalWall( iMapIndex + WORLD_COLS - 1 ))) )
			{	//Replace the piece with an extended piece.
				sIndex = PickAWallPiece( INTERIOR_EXTENDED );
				AddToUndoList( iMapIndex + WORLD_COLS - 1 );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
				ReplaceStructIndex( iMapIndex + WORLD_COLS - 1, pStruct->usIndex, usTileIndex );
				//NOTE:  Not yet checking for interior extended bottomend!
			}
			if( (pStruct = GetVerticalWall( iMapIndex )) )
			{
				sIndex = PickAWallPiece( INTERIOR_BOTTOMEND );
				AddToUndoList( iMapIndex );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
				ReplaceStructIndex( iMapIndex, pStruct->usIndex, usTileIndex );
			}
			break;
		case INTERIOR_BOTTOM:
			ubWallClass = EXTERIOR_L;
			if( (pStruct = GetVerticalWall( iMapIndex )) ) //right corner
			{	//Special case where placing the new wall will generate a corner to the right, so replace
				//the vertical piece with a bottomend.
				sIndex = PickAWallPiece( EXTERIOR_BOTTOMEND );
				AddToUndoList( iMapIndex );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
				ReplaceStructIndex( iMapIndex, pStruct->usIndex, usTileIndex );
			}
			if( (pStruct = GetVerticalWall( iMapIndex + WORLD_COLS - 1 )) && !GetVerticalWall( iMapIndex - 1) )
			{
				sIndex = PickAWallPiece( EXTERIOR_EXTENDED );
				AddToUndoList( iMapIndex + WORLD_COLS - 1 );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
				ReplaceStructIndex( iMapIndex + WORLD_COLS - 1, pStruct->usIndex, usTileIndex );
			}
			if( !gfBasement )
			{
				//All exterior_l walls have shadows.
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, EXTERIOR_L_SHADOW_INDEX);
				AddExclusiveShadow( iMapIndex, usTileIndex );
			}
			break;
		case INTERIOR_LEFT:
			iMapIndex--;
			ubWallClass = INTERIOR_R;
			if( GetHorizontalWall( iMapIndex ) )
			{
				ubWallClass = INTERIOR_BOTTOMEND;
			}
			if( !gfBasement && GetHorizontalWall( iMapIndex + 1 ) )
			{
				AddToUndoList( iMapIndex );
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, INTERIOR_BOTTOMEND_SHADOW_INDEX);
				AddExclusiveShadow( iMapIndex, usTileIndex );
			}
			if( GetHorizontalWall( iMapIndex - WORLD_COLS + 1 ) )
			{
				if( ubWallClass == INTERIOR_BOTTOMEND )
					ubWallClass = INTERIOR_EXTENDED_BOTTOMEND;
				else
					ubWallClass = INTERIOR_EXTENDED;
			}
			break;
		case INTERIOR_RIGHT:
			ubWallClass = EXTERIOR_R;
			if( GetHorizontalWall( iMapIndex ) )
			{	//Special case where placing the new wall will generate a corner.  This piece
				//becomes an exterior bottomend, but nothing else is effected.
				ubWallClass = EXTERIOR_BOTTOMEND;
			}
			if( GetHorizontalWall( iMapIndex - WORLD_COLS + 1 ) )
			{
				if( ubWallClass == EXTERIOR_BOTTOMEND )
					ubWallClass = EXTERIOR_EXTENDED_BOTTOMEND;
				else
					ubWallClass = EXTERIOR_EXTENDED;
			}
			if( !gfBasement && GetHorizontalWall( iMapIndex + 1 ) && !GetHorizontalWall( iMapIndex )
				&& !FloorAtGridNo( iMapIndex + WORLD_COLS ) )
			{
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, INTERIOR_BOTTOMEND_SHADOW_INDEX);
				AddExclusiveShadow( iMapIndex, usTileIndex );
			}
			break;

		default: abort(); // HACK000E
	}
	sIndex = PickAWallPiece( ubWallClass );
	UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usWallType, sIndex);
	AddToUndoList( iMapIndex );
	AddWallToStructLayer( iMapIndex, usTileIndex, false );
}


static void EraseRoof(UINT32 iMapIndex);


void RebuildRoofUsingFloorInfo(INT32 const map_idx, UINT16 roof_type)
{
	if (roof_type ==      0) roof_type = SearchForRoofType(map_idx);
	if (roof_type == 0xFFFF) return; // No roof type around, so don't draw one.

	AddToUndoList(map_idx);
	EraseRoof(map_idx);

	/* Analyse the neighbouring tiles for floors and set the flags.
	 * NOTE: There is no support for less than 2 side on a roof, so if there is,
	 * draw TOPLEFT */
	bool   const top      = !FloorAtGridNo(map_idx - WORLD_COLS);
	bool   const left     = !FloorAtGridNo(map_idx - 1);
	bool   const bottom   = !FloorAtGridNo(map_idx + WORLD_COLS);
	bool   const right    = !FloorAtGridNo(map_idx + 1);
	UINT16 const roof_idx =
		top    && left  ? TOPLEFT_ROOF_INDEX     :
		top    && right ? TOPRIGHT_ROOF_INDEX    :
		bottom && left  ? BOTTOMLEFT_ROOF_INDEX  :
		bottom && right ? BOTTOMRIGHT_ROOF_INDEX :
		top             ? TOP_ROOF_INDEX         :
		bottom          ? BOTTOM_ROOF_INDEX      :
		left            ? LEFT_ROOF_INDEX        :
		right           ? RIGHT_ROOF_INDEX       :
		CENTER_ROOF_BASE_INDEX + Random(CENTER_ROOF_VARIANTS);
	UINT16 const tile_idx = GetTileIndexFromTypeSubIndex(roof_type, roof_idx);
	AddRoofToHead(map_idx, tile_idx);
	// If the editor view roofs is off, then the new roofs need to be hidden.
	if (!fBuildingShowRoofs) HideStructOfGivenType(map_idx, roof_type, true);
}


/* Given a gridno, it will erase the current roof, and calculate the new roof
 * piece based on the wall orientions giving priority to the top and left walls
 * before anything else.
 * NOTE:  passing 0 for roof_type will force the function to calculate the
 * nearest roof type, and use that for the new roof. This is needed when erasing
 * parts of multiple buildings simultaneously. */
void RebuildRoof(UINT32 const map_idx, UINT16 roof_type)
{
	if (roof_type ==      0) roof_type = SearchForRoofType(map_idx);
	if (roof_type == 0xFFFF) return; // No roof type around, so don't draw one.

	AddToUndoList(map_idx);
	EraseRoof(map_idx);

	//Analyse the mapindex for walls and set the flags.
	//NOTE:  There is no support for more than 2 side on a roof, so if there is, draw TOPLEFT
	bool   const top      = GetHorizontalWall(map_idx - WORLD_COLS);
	bool   const left     = GetVerticalWall(  map_idx - 1);
	bool   const bottom   = GetHorizontalWall(map_idx);
	bool   const right    = GetVerticalWall(  map_idx);
	UINT16 const roof_idx =
		top     && left  ? TOPLEFT_ROOF_INDEX     :
		top     && right ? TOPRIGHT_ROOF_INDEX    :
		bottom  && left  ? BOTTOMLEFT_ROOF_INDEX  :
		bottom  && right ? BOTTOMRIGHT_ROOF_INDEX :
		top              ? TOP_ROOF_INDEX         :
		bottom           ? BOTTOM_ROOF_INDEX      :
		left             ? LEFT_ROOF_INDEX        :
		right            ? RIGHT_ROOF_INDEX       :
		CENTER_ROOF_BASE_INDEX + Random(CENTER_ROOF_VARIANTS);
	UINT16 const tile_idx = GetTileIndexFromTypeSubIndex(roof_type, roof_idx);
	AddRoofToHead(map_idx, tile_idx);
	// If the editor view roofs is off, then the new roofs need to be hidden.
	if (!fBuildingShowRoofs) HideStructOfGivenType(map_idx, roof_type, true);
}


static void BulldozeNature(UINT32 iMapIndex)
{
	AddToUndoList( iMapIndex );
	RemoveAllStructsOfTypeRange( iMapIndex, FIRSTISTRUCT,LASTISTRUCT );
	RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTCLIFFSHADOW, LASTCLIFFSHADOW );
	RemoveAllStructsOfTypeRange( iMapIndex, FIRSTOSTRUCT,LASTOSTRUCT ); //outside objects.
	RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTSHADOW,LASTSHADOW );
	RemoveAllStructsOfTypeRange( iMapIndex, FIRSTROAD, LASTROAD );
	RemoveAllObjectsOfTypeRange( iMapIndex, DEBRISROCKS, LASTDEBRIS );
	RemoveAllObjectsOfTypeRange( iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS );
}


static void EraseRoof(UINT32 iMapIndex)
{
	AddToUndoList( iMapIndex );
	RemoveAllRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, LASTITEM );
	RemoveAllOnRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, LASTITEM );
	RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTROOF, LASTSLANTROOF );
}


static void EraseFloor(UINT32 iMapIndex)
{
	AddToUndoList( iMapIndex );
	RemoveAllLandsOfTypeRange( iMapIndex, FIRSTFLOOR, LASTFLOOR );
}

void EraseWalls( UINT32 iMapIndex )
{
	AddToUndoList( iMapIndex );
	RemoveAllStructsOfTypeRange( iMapIndex, FIRSTTEXTURE, LASTITEM );
	RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
	RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTDOORSHADOW, LASTDOORSHADOW );
	RemoveAllObjectsOfTypeRange( iMapIndex, DEBRISROCKS, LASTDEBRIS );
	RemoveAllTopmostsOfTypeRange( iMapIndex, WIREFRAMES, WIREFRAMES );
	RemoveAllObjectsOfTypeRange( iMapIndex, DEBRIS2MISC, DEBRIS2MISC );
	RemoveAllObjectsOfTypeRange( iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS );
}

void EraseBuilding( UINT32 iMapIndex )
{
	EraseRoof( iMapIndex );
	EraseFloor( iMapIndex );
	EraseWalls( iMapIndex );
	gubWorldRoomInfo[ iMapIndex ] = 0;
}


//Specialized function that will delete only the TOP_RIGHT oriented wall in the gridno to the left
//and the TOP_LEFT oriented wall in the gridno up one as well as the other building information at this
//gridno.
static void EraseFloorOwnedBuildingPieces(UINT32 iMapIndex)
{
	LEVELNODE	*pStruct = NULL;

	if( !gfBasement && !FloorAtGridNo( iMapIndex ) )
	{	//We don't have ownership issues if there isn't a floor here.
		return;
	}
	EraseBuilding( iMapIndex );
	//FIRST, SEARCH AND DESTROY FOR A LEFT NEIGHBORING TILE WITH A TOP_RIGHT ORIENTED WALL
	pStruct = gpWorldLevelData[ iMapIndex - 1 ].pStructHead;
	while( pStruct != NULL )
	{
		if ( pStruct->usIndex != NO_TILE )
		{
			const UINT32 uiTileType = GetTileType(pStruct->usIndex);
			if ( (uiTileType >= FIRSTWALL && uiTileType <= LASTWALL) ||
				(uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) )
			{
				UINT16 usWallOrientation = GetWallOrientation(pStruct->usIndex);
				if( usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT )
				{
					AddToUndoList( iMapIndex - 1 );
					RemoveStruct( iMapIndex - 1, pStruct->usIndex );
					RemoveAllShadowsOfTypeRange( iMapIndex - 1, FIRSTWALL, LASTWALL );
					break; //otherwise, it'll crash because pStruct is toast.
				}
			}
		}
		pStruct = pStruct->pNext;
	}
	//FINALLY, SEARCH AND DESTROY FOR A TOP NEIGHBORING TILE WITH A TOP_LEFT ORIENTED WALL
	pStruct = gpWorldLevelData[ iMapIndex - WORLD_COLS ].pStructHead;
	while( pStruct != NULL )
	{
		if ( pStruct->usIndex != NO_TILE )
		{
			const UINT32 uiTileType = GetTileType(pStruct->usIndex);
			if ( (uiTileType >= FIRSTWALL && uiTileType <= LASTWALL) ||
				(uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR) )
			{
				UINT16 usWallOrientation = GetWallOrientation(pStruct->usIndex);
				if( usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT )
				{
					AddToUndoList( iMapIndex - WORLD_COLS );
					RemoveStruct( iMapIndex - WORLD_COLS , pStruct->usIndex );
					RemoveAllShadowsOfTypeRange( iMapIndex - WORLD_COLS, FIRSTWALL, LASTWALL );
					break; //otherwise, it'll crash because pStruct is toast.
				}
			}
		}
		pStruct = pStruct->pNext;
	}
}


void RemoveCaveSectionFromWorld( SGPRect *pSelectRegion )
{
	UINT32 top, left, right, bottom, x, y;
	UINT32 iMapIndex;
	UINT16 usIndex;
	UINT8 ubPerimeterValue;
	top = pSelectRegion->iTop;
	left = pSelectRegion->iLeft;
	right = pSelectRegion->iRight;
	bottom = pSelectRegion->iBottom;
	//Pass 1:  Remove all pieces in area
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		AddToUndoList( iMapIndex );
		RemoveAllStructsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
	}
	//Past 2:  Go around outside perimeter and smooth each piece
	for( y = top - 1; y <= bottom + 1; y++ ) for( x = left - 1; x <= right + 1; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		if( CaveAtGridNo( iMapIndex ) )
		{
			ubPerimeterValue = CalcNewCavePerimeterValue( iMapIndex );
			usIndex = GetCaveTileIndexFromPerimeterValue( ubPerimeterValue );
			AddToUndoList( iMapIndex );
			if( usIndex != 0xffff )
				AddCave( iMapIndex, usIndex );
			else
			{	//change piece to stalagmite...
				RemoveAllStructsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
			}
		}
	}
}

void AddCaveSectionToWorld( SGPRect *pSelectRegion )
{
	INT32 top, left, right, bottom, x, y;
	UINT32 uiMapIndex;
	UINT16 usIndex;
	UINT8 ubPerimeterValue;
	top = pSelectRegion->iTop;
	left = pSelectRegion->iLeft;
	right = pSelectRegion->iRight;
	bottom = pSelectRegion->iBottom;
	//Pass 1:  Add bogus piece to each gridno in region
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		uiMapIndex = y * WORLD_COLS + x;
		if( uiMapIndex < GRIDSIZE )
		{
			usIndex = GetCaveTileIndexFromPerimeterValue( 0xff );
			AddToUndoList( uiMapIndex );
			AddCave( uiMapIndex, usIndex );
		}
	}
	//Past 2:  Go around outside perimeter and smooth each piece
	for( y = top - 1; y <= bottom + 1; y++ ) for( x = left - 1; x <= right + 1; x++ )
	{
		uiMapIndex = y * WORLD_COLS + x;
		if( uiMapIndex < GRIDSIZE )
		{
			if( CaveAtGridNo( uiMapIndex ) )
			{
				ubPerimeterValue = CalcNewCavePerimeterValue( uiMapIndex );
				usIndex = GetCaveTileIndexFromPerimeterValue( ubPerimeterValue );
				AddToUndoList( uiMapIndex );
				if( usIndex != 0xffff )
					AddCave( uiMapIndex, usIndex );
				else
				{	//change piece to stalagmite...
					RemoveAllStructsOfTypeRange( uiMapIndex, FIRSTWALL, LASTWALL );
				}
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------
// END OF PRIVATE FUNCTION IMPLEMENTATION
//--------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
// BEGIN PUBLIC FUNCTION IMPLEMENTATION
//--------------------------------------------------------------------------------------------------

//When the user removes a section from a building, it will not only erase the
//entire highlighted area, it'll repair the building itself so there are no
//outside walls missing from the new building.
void RemoveBuildingSectionFromWorld( SGPRect *pSelectRegion )
{
	UINT32 top, left, right, bottom, x, y;
	UINT32 iMapIndex;
	UINT16 usFloorType;
	BOOLEAN fFloor;

	top = pSelectRegion->iTop;
	left = pSelectRegion->iLeft;
	right = pSelectRegion->iRight;
	bottom = pSelectRegion->iBottom;

	//1ST PASS:  Erase all building owned by the floor tile if there is one.
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		EraseFloorOwnedBuildingPieces( iMapIndex ); //Erase possible top and left walls in bordering tiles.
	}
	//2ND PASS:  Build new walls whereever there are neighboring floor tiles.
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		//NOTE:  Top and bottom walls MUST be placed first -- it minimizes the number of special cases.
		if( y == top )
		{
			fFloor = FloorAtGridNo( iMapIndex - WORLD_COLS );
			if( (gfBasement && !fFloor) || (!gfBasement && fFloor && !GetHorizontalWall( iMapIndex - WORLD_COLS )) )
				BuildWallPiece( iMapIndex, EXTERIOR_TOP, 0 );
		}
		if( y == bottom )
		{
			fFloor = FloorAtGridNo( iMapIndex + WORLD_COLS );
			if( (gfBasement && !fFloor) || (!gfBasement && fFloor && !GetHorizontalWall( iMapIndex )) )
				BuildWallPiece( iMapIndex, EXTERIOR_BOTTOM, 0 );
		}
		if( x == left )
		{
			fFloor = FloorAtGridNo( iMapIndex - 1 );
			if( (gfBasement && !fFloor) || (!gfBasement && fFloor && !GetVerticalWall( iMapIndex - 1 )) )
				BuildWallPiece( iMapIndex, EXTERIOR_LEFT, 0 );
		}
		if( x == right )
		{
			fFloor = FloorAtGridNo( iMapIndex + 1);
			if( (gfBasement && !fFloor) || (!gfBasement && fFloor && !GetVerticalWall( iMapIndex )) )
				BuildWallPiece( iMapIndex, EXTERIOR_RIGHT, 0 );
		}
	}
	//3RD PASS:  Go around the outside of the region, and rebuild the roof.
	if( gfBasement )
	{
		usFloorType = GetRandomIndexByRange( FIRSTFLOOR, LASTFLOOR );
		if( usFloorType == 0xffff )
			usFloorType = FIRSTFLOOR;
		for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
		{
			iMapIndex = y * WORLD_COLS + x;
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usFloorType, 1);
			AddLandToHead( iMapIndex, (UINT16)( usTileIndex + Random( FLOOR_VARIANTS ) ) );
		}
	}
	for( y = top - 1; y <= bottom + 1; y++ ) for( x = left - 1; x <= right + 1; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		if( y == top - 1 || y == bottom + 1 || x == left - 1 || x == right + 1 )
		{
			if( (!gfBasement && FloorAtGridNo( iMapIndex )) || (gfBasement && !FloorAtGridNo( iMapIndex )) )
			{
				RebuildRoof( iMapIndex, 0 );
			}
		}
	}
}

void AddBuildingSectionToWorld( SGPRect *pSelectRegion )
{
	INT32 top, left, right, bottom, x, y;
	UINT32 iMapIndex;
	UINT16 usFloorType, usWallType;
	BOOLEAN fNewBuilding;
	BOOLEAN fSlantRoof = false;
	BOOLEAN fVertical;
	BOOLEAN fFloor;
	top = pSelectRegion->iTop;
	left = pSelectRegion->iLeft;
	right = pSelectRegion->iRight;
	bottom = pSelectRegion->iBottom;

	UINT16 usRoofType = (UINT16)-1; // XXX HACK000E

	//Special case scenario:
	//If the user selects a floor without walls, then it is implied that the user wishes to
	//change the floor for say a kitchen which might have a different floor type.
	usWallType = GetRandomIndexByRange( FIRSTWALL, LASTWALL );
	usFloorType = GetRandomIndexByRange( FIRSTFLOOR, LASTFLOOR );
	if( usWallType == 0xffff && usFloorType != 0xffff )
	{ //allow user to place floors
		for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
		{
			iMapIndex = y * WORLD_COLS + x;
			EraseFloor( iMapIndex );
			UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usFloorType, 1);
			AddLandToHead( iMapIndex, (UINT16)( usTileIndex + Random( FLOOR_VARIANTS ) ) );
		}
		//we are done!
		return;
	}

	//1ST PASS:  Determine if there are any floor tiles in this region.  If there are, then
	//  that signifies that we are concantenating this building to an existing one.  Otherwise,
	//  we are just drawing an individual building.  If we find a floor, extract the type so
	//  we know how to draw it later.
	fNewBuilding = true;
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		if( FloorAtGridNo( iMapIndex ) )
		{
			LEVELNODE *pFloor;
			//If a floor is found, then we are adding to an existing structure.
			fNewBuilding = false;
			//Extract the floor type.  We already checked if there was a floor here, so it is assumed.
			pFloor = gpWorldLevelData[ iMapIndex ].pLandHead;
			while( pFloor )
			{
				const UINT32 uiTileType = GetTileType(pFloor->usIndex);
				if( uiTileType >= FIRSTFLOOR && uiTileType <= LASTFLOOR )
				{
					usFloorType = (UINT16)uiTileType;
					break;
				}
			}
			usWallType = SearchForWallType( iMapIndex );
			usRoofType = SearchForRoofType( iMapIndex );
			if( usWallType != 0xffff && usRoofType != 0xffff && usFloorType !=0xffff )
			{	//we have extracted all of the information we need, so we can break out.
				y = bottom;
				break;
			}
		}
	}

	if( fNewBuilding )
	{
		//if( gfBasement )
		//	return;
		//Get materials via selection window method.
		usWallType = GetRandomIndexByRange( FIRSTWALL, LASTWALL );
		usFloorType = GetRandomIndexByRange( FIRSTFLOOR, LASTFLOOR );
		usRoofType = GetRandomIndexByRange( FIRSTROOF, LASTROOF );
		if( usRoofType == 0xffff )
		{
			usRoofType = GetRandomIndexByRange( FIRSTSLANTROOF, LASTSLANTROOF );
			if( usRoofType != 0xffff )
			{
				if( !gfBasement )
					fSlantRoof = true;
				else
					usRoofType = FIRSTROOF;
			}
		}
		if( usWallType == 0xffff )
			return;
	}

	//2ND PASS:  Remove all walls in the region that border no floor tile, or simply walls
	//  that are considered exterior walls.  That way, it won't wreck the inside of a building
	//  if you select too much interior.  Also, gridnos that delete walls will also delete the
	//  floor and roof tiles there.  That signifies that the floorless parts will be resmoothed,
	//  and rebuilt in the third pass.
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		if( gfBasement )
		{
			EraseBuilding( iMapIndex );
		}
		else if( FloorAtGridNo( iMapIndex ) && !fNewBuilding)
		{
			if( y >= top && !FloorAtGridNo( iMapIndex - WORLD_COLS ) )
			{
				EraseHorizontalWall( iMapIndex - WORLD_COLS );
				EraseFloor( iMapIndex );
				EraseRoof( iMapIndex );
			}
			if( y <= bottom && !FloorAtGridNo( iMapIndex + WORLD_COLS ) )
			{
				EraseHorizontalWall( iMapIndex );
				EraseFloor( iMapIndex );
				EraseRoof( iMapIndex );
			}
			if( x >= left && !FloorAtGridNo( iMapIndex - 1 ) )
			{
				EraseVerticalWall( iMapIndex - 1 );
				EraseFloor( iMapIndex );
				EraseRoof( iMapIndex );
			}
			if( x <= right && !FloorAtGridNo( iMapIndex + 1 ) )
			{
				EraseVerticalWall( iMapIndex );
				EraseFloor( iMapIndex );
				EraseRoof( iMapIndex );
			}
		}
		else	//we will be building outside of this structure, so bulldoze the nature -- trees, rocks, etc.
		{
			BulldozeNature( iMapIndex );
		}
	}
	//3RD PASS:  Process the region, and all walls of floorless tiles are rebuilt from interior perspective.
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		if( !FloorAtGridNo( iMapIndex ) )
		{
			if( y == top && !GetHorizontalWall( iMapIndex - WORLD_COLS ) )
			{
				fFloor = FloorAtGridNo( iMapIndex - WORLD_COLS );
				if( gfBasement == fFloor )
					BuildWallPiece( iMapIndex, INTERIOR_TOP, usWallType);
			}
			if( y == bottom && !GetHorizontalWall( iMapIndex ) )
			{
				fFloor = FloorAtGridNo( iMapIndex + WORLD_COLS );
				if( gfBasement == fFloor )
					BuildWallPiece( iMapIndex, INTERIOR_BOTTOM, usWallType );
			}
			if( x == left && !GetVerticalWall( iMapIndex - 1 ) )
			{
				fFloor = FloorAtGridNo( iMapIndex - 1 );
				if( gfBasement == fFloor )
					BuildWallPiece( iMapIndex, INTERIOR_LEFT, usWallType );
			}
			if( x == right && !GetVerticalWall( iMapIndex ) )
			{
				fFloor = FloorAtGridNo( iMapIndex + 1 );
				if( gfBasement == fFloor )
					BuildWallPiece( iMapIndex, INTERIOR_RIGHT, usWallType );
			}
		}
	}

	//If we are dealing with slant roofs then build the whole thing now.
	//Slant roofs always have a width or height of 8 tiles.
	if( fSlantRoof )
	{
		fVertical = (bottom - top == 7) ? false : true;
		BuildSlantRoof( left, top, right, bottom, usWallType, usRoofType, fVertical );
	}

	//4TH PASS:  Process the region, and all floorless tiles get new roofs and floors.
	for( y = top; y <= bottom; y++ ) for( x = left; x <= right; x++ )
	{
		iMapIndex = y * WORLD_COLS + x;
		if( !FloorAtGridNo( iMapIndex ) )
		{
			if( !fSlantRoof )
				RebuildRoof( iMapIndex, usRoofType );
			if( usFloorType != 0xffff && !gfBasement )
			{
				UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usFloorType, 1);
				AddLandToHead( iMapIndex, (UINT16)( usTileIndex + Random( FLOOR_VARIANTS ) ) );
			}
		}
	}
}


void AnalyseCaveMapForStructureInfo()
{
	FOR_EACH_WORLD_TILE(i)
	{
		for (LEVELNODE* k = i->pStructHead; k; k = k->pNext)
		{
			if (k->usIndex == NO_TILE) continue;

			if (GetTileType(k->usIndex) != FIRSTWALL) continue;

			UINT16 const sub_idx = GetSubIndexFromTileIndex(k->usIndex);
			if (sub_idx < 60 || 65 < sub_idx) continue;

			k->uiFlags |= LEVELNODE_CAVE;
		}
	}
}

