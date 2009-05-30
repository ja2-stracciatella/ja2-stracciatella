#include "Handle_Items.h"
#include "Render_Fun.h"
#include "Structure.h"
#include "TileDat.h"
#include "WorldDef.h"
#include "Input.h"
#include "SysUtil.h"
#include "WorldMan.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include "FOV.h"
#include "Fog_Of_War.h"
#include "Environment.h"
#include "Dialogue_Control.h"
#include "Random.h"


// Room Information
UINT8						gubWorldRoomInfo[ WORLD_MAX ];
UINT8						gubWorldRoomHidden[ MAX_ROOMS ];


void InitRoomDatabase()
{
	memset( gubWorldRoomInfo, NO_ROOM, sizeof( gubWorldRoomInfo ) );
	memset( gubWorldRoomHidden, TRUE, sizeof( gubWorldRoomHidden ) );
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


void SetGridNoRevealedFlag( UINT16 sGridNo )
{
//	UINT32 cnt;
//  ITEM_POOL					*pItemPool;
//	INT16							sX, sY;
	LEVELNODE					*pNode = NULL;
	STRUCTURE					*pStructure, *pBase;

	// Set hidden flag, for any roofs
	SetRoofIndexFlagsFromTypeRange( sGridNo, FIRSTROOF, FOURTHROOF, LEVELNODE_HIDDEN  );

	// ATE: Do this only if we are in a room...
	if ( gubWorldRoomInfo[ sGridNo ] != NO_ROOM )
	{
		SetStructAframeFlags(  sGridNo, LEVELNODE_HIDDEN );
		// Find gridno one east as well...

		if ( ( sGridNo + WORLD_COLS ) < NOWHERE )
		{
			SetStructAframeFlags(  sGridNo + WORLD_COLS, LEVELNODE_HIDDEN );
		}

		if ( ( sGridNo + 1 ) < NOWHERE )
		{
			SetStructAframeFlags(  sGridNo + 1, LEVELNODE_HIDDEN );
		}
	}

	// Set gridno as revealed
	gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_REVEALED;
	if( gfCaves )
	{
		RemoveFogFromGridNo( sGridNo );
	}

	// ATE: If there are any structs here, we can render them with the obscured flag!
	// Look for anything but walls pn this gridno!
	pStructure	=  gpWorldLevelData[ (INT16)sGridNo ].pStructureHead;

	while ( pStructure != NULL )
	{
		if ( pStructure->sCubeOffset == STRUCTURE_ON_GROUND || ( pStructure->fFlags & STRUCTURE_SLANTED_ROOF ) )
		{
			if ( ( (pStructure->fFlags & STRUCTURE_OBSTACLE ) && !( pStructure->fFlags & ( STRUCTURE_PERSON | STRUCTURE_CORPSE ) ) ) || ( pStructure->fFlags & STRUCTURE_SLANTED_ROOF ) )
			{
				pBase = FindBaseStructure( pStructure );

				// Get LEVELNODE for struct and remove!
				pNode = FindLevelNodeBasedOnStructure( pBase->sGridNo, pBase );
				pNode->uiFlags |= LEVELNODE_SHOW_THROUGH;

				if ( pStructure->fFlags & STRUCTURE_SLANTED_ROOF )
				{
					AddSlantRoofFOVSlot( pBase->sGridNo );

					// Set hidden...
					pNode->uiFlags |= LEVELNODE_HIDDEN;

				}
			}
		}

		pStructure = pStructure->pNext;
	}

	gubWorldRoomHidden[ gubWorldRoomInfo[ sGridNo ] ] = FALSE;

}


void ExamineGridNoForSlantRoofExtraGraphic( UINT16 sCheckGridNo )
{
	LEVELNODE					*pNode = NULL;
	STRUCTURE					*pStructure, *pBase;
	UINT8							ubLoop;
	DB_STRUCTURE_TILE	**	ppTile;
	INT16							sGridNo;
	BOOLEAN						fChanged = FALSE;

	// CHECK FOR A SLANTED ROOF HERE....
	pStructure = FindStructure( sCheckGridNo, STRUCTURE_SLANTED_ROOF );

	if ( pStructure != NULL )
	{
		// We have a slanted roof here ... find base and remove...
		pBase = FindBaseStructure( pStructure );

		// Get LEVELNODE for struct and remove!
		pNode = FindLevelNodeBasedOnStructure( pBase->sGridNo, pBase );

		// Loop through each gridno and see if revealed....
		for ( ubLoop = 0; ubLoop < pBase->pDBStructureRef->pDBStructure->ubNumberOfTiles; ubLoop++ )
		{
			ppTile = pBase->pDBStructureRef->ppTile;
			sGridNo = pBase->sGridNo + ppTile[ ubLoop ]->sPosRelToBase;

			if (sGridNo < 0 || sGridNo > WORLD_MAX)
			{
				continue;
			}

			// Given gridno,
			// IF NOT REVEALED AND HIDDEN....
			if ( !( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED ) && pNode->uiFlags & LEVELNODE_HIDDEN )
			{
				// Add graphic if one does not already exist....
				if (!FindTypeInRoofLayer(sGridNo, SLANTROOFCEILING))
				{
					// Add
					AddRoofToHead( sGridNo, SLANTROOFCEILING1 );
					fChanged = TRUE;
				}
			}

			// Revealed?
			if ( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REVEALED )
			{
				///Remove any slant roof items if they exist
				if (LEVELNODE const* const roof = FindTypeInRoofLayer(sGridNo, SLANTROOFCEILING))
				{
					RemoveRoof(sGridNo, roof->usIndex);
					fChanged = TRUE;
				}
			}

		}
	}

	if ( fChanged )
	{
		// DIRTY THE WORLD!
		InvalidateWorldRedundency();
		SetRenderFlags(RENDER_FLAG_FULL );
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
