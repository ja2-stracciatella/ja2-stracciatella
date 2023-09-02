#include "Handle_Items.h"
#include "Render_Fun.h"
#include "Structure.h"
#include "TileDat.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include "FOV.h"
#include "Fog_Of_War.h"
#include "Environment.h"
#include "Dialogue_Control.h"
#include "Random.h"

#include <algorithm>
#include <iterator>


// Room Information
UINT8						gubWorldRoomInfo[ WORLD_MAX ];
UINT8						gubWorldRoomHidden[ MAX_ROOMS ];


void InitRoomDatabase()
{
	std::fill(std::begin(gubWorldRoomInfo), std::end(gubWorldRoomInfo), NO_ROOM);
	std::fill(std::begin(gubWorldRoomHidden), std::end(gubWorldRoomHidden), TRUE);
}


UINT8 GetRoom(UINT16 const gridno)
{
	return gubWorldRoomInfo[gridno];
}


BOOLEAN InAHiddenRoom( UINT16 sGridNo, UINT8 *pubRoomNo )
{
	if ( gubWorldRoomInfo[ sGridNo ] != NO_ROOM )
	{
		if ( (gubWorldRoomHidden[ gubWorldRoomInfo[ sGridNo ] ] ) )
		{
			*pubRoomNo = gubWorldRoomInfo[ sGridNo ];
			return( TRUE );
		}
	}

	return( FALSE );
}


// @@ATECLIP TO WORLD!
void SetRecalculateWireFrameFlagRadius(const GridNo pos, const INT16 sRadius)
{
	INT16 pos_x_;
	INT16 pos_y_;
	ConvertGridNoToXY(pos, &pos_x_, &pos_y_);
	const INT16 pos_x = pos_x_;
	const INT16 pos_y = pos_y_;
	for (INT16 y = pos_y - sRadius; y < pos_y + sRadius + 2; ++y)
	{
		for (INT16 x = pos_x - sRadius; x < pos_x + sRadius + 2; ++x)
		{
			const UINT32 uiTile = MAPROWCOLTOPOS(y, x);
			gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_RECALCULATE_WIREFRAMES;
		}
	}
}


void SetGridNoRevealedFlag(UINT16 const grid_no)
{
	// Set hidden flag, for any roofs
	SetRoofIndexFlagsFromTypeRange(grid_no, FIRSTROOF, FOURTHROOF, LEVELNODE_HIDDEN);

	// ATE: Do this only if we are in a room
	if (gubWorldRoomInfo[grid_no] != NO_ROOM)
	{
		SetStructAframeFlags(grid_no, LEVELNODE_HIDDEN);

		// Find gridno one east as well
		if (grid_no + WORLD_COLS < GRIDSIZE)
		{
			SetStructAframeFlags(grid_no + WORLD_COLS, LEVELNODE_HIDDEN);
		}

		if (grid_no + 1 < GRIDSIZE)
		{
			SetStructAframeFlags(grid_no + 1, LEVELNODE_HIDDEN);
		}
	}

	// Set gridno as revealed
	gpWorldLevelData[grid_no].uiFlags |= MAPELEMENT_REVEALED;
	if (gfCaves) RemoveFogFromGridNo(grid_no);

	// ATE: If there are any structs here, we can render them with the obscured flag!
	// Look for anything but walls pn this gridno!
	for (STRUCTURE* i = gpWorldLevelData[grid_no].pStructureHead; i; i = i->pNext)
	{
		if (!(i->fFlags & STRUCTURE_SLANTED_ROOF))
		{
			if (i->sCubeOffset != STRUCTURE_ON_GROUND) continue;
			if (!(i->fFlags & STRUCTURE_OBSTACLE) || i->fFlags & (STRUCTURE_PERSON | STRUCTURE_CORPSE)) continue;
		}

		try
		{
			STRUCTURE* const base = FindBaseStructure(i);
			LEVELNODE* const node = FindLevelNodeBasedOnStructure(base);
			node->uiFlags |= LEVELNODE_SHOW_THROUGH;

			if (i->fFlags & STRUCTURE_SLANTED_ROOF)
			{
				AddSlantRoofFOVSlot(base->sGridNo);
				node->uiFlags |= LEVELNODE_HIDDEN;
			}
		}
		catch (const std::logic_error& e)
		{
			SLOGW("Failed to find LEVELNODE for a structure at grid {}. ({})", grid_no, e.what());
		}
	}

	gubWorldRoomHidden[gubWorldRoomInfo[grid_no]] = FALSE;
}


void ExamineGridNoForSlantRoofExtraGraphic(GridNo const check_grid_no)
{
	// Check for a slanted roof here
	STRUCTURE* const s = FindStructure(check_grid_no, STRUCTURE_SLANTED_ROOF);
	if (!s) return;

	// We have a slanted roof here, find base and remove
	bool                            changed      = false;
	STRUCTURE*                const base         = FindBaseStructure(s);
	LEVELNODE*                const node         = FindLevelNodeBasedOnStructure(base);
	bool                      const hidden       = node->uiFlags & LEVELNODE_HIDDEN;
	GridNo                    const base_grid_no = base->sGridNo;
	DB_STRUCTURE_TILE* const* const tile         = base->pDBStructureRef->ppTile;
	DB_STRUCTURE_TILE* const* const end          = tile + base->pDBStructureRef->pDBStructure->ubNumberOfTiles;
	// Loop through each gridno and see if revealed
	for (DB_STRUCTURE_TILE* const* i = tile; i != end; ++i)
	{
		GridNo const grid_no = base_grid_no + (*i)->sPosRelToBase;
		if (grid_no < 0 || WORLD_MAX <= grid_no) continue;

		if (gpWorldLevelData[grid_no].uiFlags & MAPELEMENT_REVEALED)
		{ // Remove any slant roof items if they exist
			if (LEVELNODE const* const roof = FindTypeInRoofLayer(grid_no, SLANTROOFCEILING))
			{
				RemoveRoof(grid_no, roof->usIndex);
				changed = true;
			}
		}
		else
		{ // Add graphic if one does not already exist
			if (hidden && !FindTypeInRoofLayer(grid_no, SLANTROOFCEILING))
			{
				AddRoofToHead(grid_no, SLANTROOFCEILING1);
				changed = true;
			}
		}
	}

	if (changed)
	{ // Dirty the world
		InvalidateWorldRedundency();
		SetRenderFlags(RENDER_FLAG_FULL);
	}
}


void RemoveRoomRoof( UINT16 sGridNo, UINT8 bRoomNum, SOLDIERTYPE *pSoldier )
{
	UINT32 cnt;
	BOOLEAN						fSaidItemSeenQuote = FALSE;

//	STRUCTURE					*pStructure;//, *pBase;

	// LOOP THORUGH WORLD AND CHECK ROOM INFO
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if ( gubWorldRoomInfo[ cnt ] == bRoomNum )
		{

			SetGridNoRevealedFlag( (UINT16)cnt );

			RemoveRoofIndexFlagsFromTypeRange( cnt, FIRSTROOF, SECONDSLANTROOF, LEVELNODE_REVEAL  );

			// Reveal any items if here!
			if (SetItemsVisibilityOn(cnt, 0, INVISIBLE, TRUE))
			{
				if ( !fSaidItemSeenQuote )
				{
					fSaidItemSeenQuote = TRUE;

					if ( pSoldier != NULL )
					{
						TacticalCharacterDialogue( pSoldier, (UINT16)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2 ) ) );
					}
				}
			}

			// OK, re-set writeframes ( in a radius )
			SetRecalculateWireFrameFlagRadius(cnt, 2);
		}
	}

	//for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	//{
	//	if ( gubWorldRoomInfo[ cnt ] == bRoomNum )
	//	{
	//		ExamineGridNoForSlantRoofExtraGraphic( (UINT16)cnt );
	//	}
	//}

	// DIRTY THE WORLD!
	InvalidateWorldRedundency();
	SetRenderFlags(RENDER_FLAG_FULL );

	CalculateWorldWireFrameTiles( FALSE );
}
