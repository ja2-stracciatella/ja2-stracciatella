#include "NewSmooth.h"
#include "TileDef.h"
#include "WorldDef.h"	//for LEVELNODE def
#include "WorldMan.h" //for RemoveXXXX()
#include "Isometric_Utils.h" //for GridNoOnVisibleWorldTile()
#include "SmartMethod.h"
#include "Smoothing_Utils.h"
#include "Editor_Undo.h"
#include "Edit_Sys.h"
#include "Environment.h"


//This method isn't foolproof, but because erasing large areas of buildings could result in
//multiple wall types for each building.  When processing the region, it is necessary to
//calculate the roof type by searching for the nearest roof tile.
UINT16 SearchForWallType( UINT32 iMapIndex )
{
	LEVELNODE *pWall;
	INT16 sOffset;
	INT16 x, y, sRadius = 0;
	if( gfBasement )
	{
		UINT16 usWallType;
		usWallType = GetRandomIndexByRange( FIRSTWALL, LASTWALL );
		if( usWallType == 0xffff )
			usWallType = FIRSTWALL;
		return usWallType;
	}
	while( sRadius < 32 )
	{
		//NOTE:  start at the higher y value and go negative because it is possible to have another
		// structure type one tile north, but not one tile south -- so it'll find the correct wall first.
		for( y = sRadius; y >= -sRadius; y-- ) for( x = -sRadius; x <= sRadius; x++ )
		{
			if (std::abs(x) == sRadius || std::abs(y) == sRadius)
			{
				sOffset = y * WORLD_COLS + x;
				if( !GridNoOnVisibleWorldTile( (INT16)(iMapIndex + sOffset) ) )
				{
					continue;
				}
				pWall = gpWorldLevelData[ iMapIndex + sOffset ].pStructHead;
				while( pWall )
				{
					const UINT32 uiTileType = GetTileType(pWall->usIndex);
					if( uiTileType >= FIRSTWALL && uiTileType <= LASTWALL )
					{	//found a roof, so return its type.
						return (UINT16)uiTileType;
					}
					//if( uiTileType >= FIRSTWINDOW && uiTileType <= LASTWINDOW )
					//{	//Window types can be converted to a wall type.
					//	return (UINT16)(FIRSTWALL + uiTileType - FIRSTWINDOW );
					//}
					pWall = pWall->pNext;
				}
			}
		}
		sRadius++;
	}
	return 0xffff;
}


/* This method isn't foolproof, because erasing large areas of buildings could
 * result in multiple roof types for each building. When processing the region,
 * it is necessary to calculate the roof type by searching for the nearest roof
 * tile. */
UINT16 SearchForRoofType(UINT32 const map_idx)
{
	for (INT16 radius = 0; radius != 32; ++radius)
	{
		for (INT16 y = -radius; y <= radius; ++y)
		{
			for (INT16 x = -radius; x <= radius; ++x)
			{
				if (std::abs(x) != radius && std::abs(y) != radius) continue;

				GridNo const grid_no = map_idx + y * WORLD_COLS + x;
				if (!GridNoOnVisibleWorldTile(grid_no)) continue;

				for (LEVELNODE const* i = gpWorldLevelData[grid_no].pRoofHead; i; i = i->pNext)
				{
					UINT32 const tile_type = GetTileType(i->usIndex);
					if (tile_type < FIRSTROOF || tile_type > LASTROOF) continue;
					// found a roof, so return its type.
					return tile_type;
				}
			}
		}
	}
	return 0xFFFF;
}


static bool RoofAtGridNo(UINT32 const map_idx)
{
	for (LEVELNODE const* i = gpWorldLevelData[map_idx].pRoofHead; i;)
	{
		if (i->usIndex == NO_TILE) continue;

		UINT32 const tile_type = GetTileType(i->usIndex);
		if (FIRSTROOF <= tile_type && tile_type <= SECONDSLANTROOF) return true;
		i = i->pNext; // XXX TODO0009 if i->usIndex == NO_TILE this is an endless loop
	}
	return false;
}


bool BuildingAtGridNo(UINT32 const map_idx)
{
	return RoofAtGridNo(map_idx) || FloorAtGridNo(map_idx);
}


static LEVELNODE* GetHorizontalFence(UINT32 map_idx);
static LEVELNODE* GetVerticalFence(UINT32 map_idx);


bool ValidDecalPlacement(UINT32 const map_idx)
{
	return
		GetVerticalWall(map_idx)   ||
		GetHorizontalWall(map_idx) ||
		GetVerticalFence(map_idx)  ||
		GetHorizontalFence(map_idx);
}


LEVELNODE* GetVerticalWall(UINT32 const map_idx)
{
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pStructHead; i; i = i->pNext)
	{
		if (i->usIndex == NO_TILE) continue;

		UINT32 const tile_type = GetTileType(i->usIndex);
		if ((FIRSTWALL <= tile_type && tile_type <= LASTWALL) ||
				(FIRSTDOOR <= tile_type && tile_type <= LASTDOOR))
		{
			UINT16 const wall_orientation = GetWallOrientation(i->usIndex);
			if (wall_orientation != INSIDE_TOP_RIGHT && wall_orientation != OUTSIDE_TOP_RIGHT) continue;
			return i;
		}
	}
	return 0;
}


LEVELNODE* GetHorizontalWall(UINT32 const map_idx)
{
	for (LEVELNODE* i = gpWorldLevelData[ map_idx ].pStructHead; i; i = i->pNext)
	{
		if (i->usIndex == NO_TILE) continue;

		UINT32 const tile_type = GetTileType(i->usIndex);
		if ((FIRSTWALL <= tile_type && tile_type <= LASTWALL) ||
				(FIRSTDOOR <= tile_type && tile_type <= LASTDOOR))
		{
			UINT16 const wall_orientation = GetWallOrientation(i->usIndex);
			if (wall_orientation != INSIDE_TOP_LEFT && wall_orientation != OUTSIDE_TOP_LEFT) continue;
			return i;
		}
	}
	return 0;
}


UINT16 GetVerticalWallType(UINT32 const map_idx)
{
	LEVELNODE const* const wall = GetVerticalWall(map_idx);
	return wall ? GetWallType(wall, map_idx) : 0;
}


UINT16 GetHorizontalWallType(UINT32 const map_idx)
{
	LEVELNODE const* const wall = GetHorizontalWall(map_idx);
	return wall ? GetWallType(wall, map_idx) : 0;
}


static LEVELNODE* GetVerticalFence(UINT32 const map_idx)
{
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pStructHead; i; i = i->pNext)
	{
		if (i->usIndex == NO_TILE)                  continue;
		if (GetTileType(i->usIndex) != FENCESTRUCT) continue;

		UINT16 const wall_orientation = GetWallOrientation(i->usIndex);
		if (wall_orientation != INSIDE_TOP_RIGHT && wall_orientation != OUTSIDE_TOP_RIGHT) continue;
		return i;
	}
	return 0;
}


static LEVELNODE* GetHorizontalFence(UINT32 const map_idx)
{
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pStructHead; i; i = i->pNext)
	{
		if (i->usIndex == NO_TILE)                  continue;
		if (GetTileType(i->usIndex) != FENCESTRUCT) continue;

		UINT16 const wall_orientation = GetWallOrientation(i->usIndex);
		if (wall_orientation != INSIDE_TOP_LEFT && wall_orientation != OUTSIDE_TOP_LEFT) continue;
		return i;
	}
	return 0;
}


void EraseHorizontalWall( UINT32 iMapIndex )
{
	LEVELNODE *pWall;
	pWall = GetHorizontalWall( iMapIndex );
	if( pWall )
	{
		AddToUndoList( iMapIndex );
		RemoveStruct( iMapIndex, pWall->usIndex );
		RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
	}
}

void EraseVerticalWall( UINT32 iMapIndex )
{
	LEVELNODE *pWall;
	pWall = GetVerticalWall( iMapIndex );
	if( pWall )
	{
		AddToUndoList( iMapIndex );
		RemoveStruct( iMapIndex, pWall->usIndex );
		RemoveAllShadowsOfTypeRange( iMapIndex, FIRSTWALL, LASTWALL );
	}
}


static void ChangeWall(LEVELNODE const* const wall, UINT32 const map_idx, UINT16 const new_piece)
{
	if (!wall) return;

	UINT32 const tile_type = GetTileType(wall->usIndex);
	if (tile_type < FIRSTWALL || LASTWALL < tile_type) return;

	// We have the wall, now change its type
	INT16 const idx = PickAWallPiece(new_piece);
	AddToUndoList(map_idx);
	UINT16 const tile_idx = GetTileIndexFromTypeSubIndex(tile_type, idx);
	ReplaceStructIndex(map_idx, wall->usIndex, tile_idx);
}

void ChangeVerticalWall(UINT32 const map_idx, UINT16 const new_piece)
{
	ChangeWall(GetVerticalWall(map_idx), map_idx, new_piece);
}


UINT16 GetWallType(LEVELNODE const* const wall, UINT32 const map_idx)
{
	UINT32 const tile_type = GetTileType(wall->usIndex);
	/* Doors do not contain the wall type, so search for the nearest wall to
	 * extract it */
	return
		FIRSTDOOR <= tile_type && tile_type <= LASTDOOR ? SearchForWallType(map_idx) :
		(UINT16)tile_type;
}


void RestoreWalls(UINT32 const map_idx)
{
	bool done = false;

	if (LEVELNODE const* const wall = GetHorizontalWall(map_idx))
	{
		UINT16 const wall_type        = GetWallType(wall, map_idx);
		UINT16 const wall_orientation = GetWallOrientation(wall->usIndex);
		AddToUndoList(map_idx);
		RemoveStruct(map_idx, wall->usIndex);
		RemoveAllShadowsOfTypeRange(map_idx, FIRSTWALL, LASTWALL);
		switch (wall_orientation)
		{
			case OUTSIDE_TOP_LEFT: BuildWallPiece(map_idx, INTERIOR_BOTTOM, wall_type); break;
			case INSIDE_TOP_LEFT:  BuildWallPiece(map_idx, EXTERIOR_BOTTOM, wall_type); break;
		}
		done = true;
	}

	if (LEVELNODE const* const wall = GetVerticalWall(map_idx))
	{
		UINT16 const wall_type        = GetWallType(wall, map_idx);
		UINT16 const wall_orientation = GetWallOrientation(wall->usIndex);
		AddToUndoList(map_idx);
		RemoveStruct(map_idx, wall->usIndex);
		RemoveAllShadowsOfTypeRange(map_idx, FIRSTWALL, LASTWALL);
		switch (wall_orientation)
		{
			case OUTSIDE_TOP_RIGHT: BuildWallPiece(map_idx, INTERIOR_RIGHT, wall_type); break;
			case INSIDE_TOP_RIGHT:  BuildWallPiece(map_idx, EXTERIOR_RIGHT, wall_type); break;
		}
		done = true;
	}

	if (done) return;

	/* We are in a special case here. The user is attempting to restore a wall,
	 * though nothing is here. We will hook into the smart wall method by tricking
	 * it into using the local wall type, but only if we have adjacent walls. */
	LEVELNODE* wall = GetHorizontalWall(map_idx - 1);
	if (!wall) wall = GetHorizontalWall(map_idx + 1);
	if (!wall) wall = GetVerticalWall(map_idx - WORLD_COLS);
	if (!wall) wall = GetVerticalWall(map_idx + WORLD_COLS);
	if (!wall) return;

	/* Found a wall.  Let's back up the current wall value, and restore it after
	 * pasting a smart wall. */
	UINT16 const wall_type = GetWallType(wall, map_idx);
	if (wall_type == 0xFFFF) return;

	UINT8 const save_wall_ui_value = gubWallUIValue; // Save the wall UI value
	gubWallUIValue = (UINT8)wall_type;               // Trick the UI value
	PasteSmartWall(map_idx);                         // Paste smart wall with fake UI value
	gubWallUIValue = save_wall_ui_value;             // Restore the real UI value
}
