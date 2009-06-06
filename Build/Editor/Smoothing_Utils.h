#ifdef JA2EDITOR
#ifndef __SMOOTHING_UTILS_H
#define __SMOOTHING_UTILS_H

#include "WorldDef.h"

//Use these values when specifically replacing a wall with new type.
enum {	//Wall tile types
	INTERIOR_L,										//interior wall with a top left orientation
	INTERIOR_R,										//interior wall with a top right orientation
	EXTERIOR_L,										//exterior wall with a top left orientation
	EXTERIOR_R,										//exterior wall with a top right orientation
	INTERIOR_CORNER,							//special interior end piece with top left orientation.
	//The rest of these walls are special wall tiles for top right orientations.
	INTERIOR_BOTTOMEND,						//interior wall for bottom corner
	EXTERIOR_BOTTOMEND,						//exterior wall for bottom corner
	INTERIOR_EXTENDED,						//extended interior wall for top corner
	EXTERIOR_EXTENDED,						//extended exterior wall for top corner
	INTERIOR_EXTENDED_BOTTOMEND,	//extended interior wall for both top and bottom corners.
	EXTERIOR_EXTENDED_BOTTOMEND,	//extended exterior wall for both top and bottom corners.
	NUM_WALL_TYPES
};

//Use these values when passing a ubWallPiece to BuildWallPieces.
enum {
	NO_WALLS,
	INTERIOR_TOP,
	INTERIOR_BOTTOM,
	INTERIOR_LEFT,
	INTERIOR_RIGHT,
	EXTERIOR_TOP,
	EXTERIOR_BOTTOM,
	EXTERIOR_LEFT,
	EXTERIOR_RIGHT,
};

//in newsmooth.c
extern void EraseWalls( UINT32 iMapIndex );
extern void BuildWallPiece( UINT32 iMapIndex, UINT8 ubWallPiece, UINT16 usWallType );
//in Smoothing Utils
void RestoreWalls( UINT32 iMapIndex );
UINT16 SearchForRoofType( UINT32 iMapIndex );
UINT16 SearchForWallType( UINT32 iMapIndex );
bool       BuildingAtGridNo(UINT32 map_idx);
LEVELNODE* GetHorizontalWall( UINT32 iMapIndex );
LEVELNODE* GetVerticalWall( UINT32 iMapIndex );
UINT16 GetHorizontalWallType( UINT32 iMapIndex );
UINT16 GetVerticalWallType( UINT32 iMapIndex );
void EraseHorizontalWall( UINT32 iMapIndex );
void EraseVerticalWall( UINT32 iMapIndex );
void ChangeVerticalWall( UINT32 iMapIndex, UINT16 usNewPiece );
BOOLEAN ValidDecalPlacement( UINT32 iMapIndex );

#endif
#endif
