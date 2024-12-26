#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Structure_Wrap.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include "Rotting_Corpses.h"
#include "Soldier_Control.h"


BOOLEAN	IsRoofPresentAtGridno( INT16 sGridNo )
{
	return FindStructure(sGridNo, STRUCTURE_ROOF) != NULL;
}


BOOLEAN	IsJumpableWindowPresentAtGridNo( INT32 sGridNo, INT8 bStartingDir)
{
	STRUCTURE const* pStructure = FindStructure( sGridNo, STRUCTURE_WALLNWINDOW );
	const BOOLEAN fIntactWindowsAlso = false; // TODO

	if ( pStructure )
	{
		if (!(pStructure->fFlags & STRUCTURE_WALLNWINDOW) || (pStructure->fFlags & STRUCTURE_SPECIAL)) return false;

		switch(bStartingDir)
		{
			case SOUTH:
			case NORTH:
				if (pStructure->ubWallOrientation != OUTSIDE_TOP_LEFT &&
					pStructure->ubWallOrientation != INSIDE_TOP_LEFT)
				{
					return false;
				}
				break;
			case EAST:
			case WEST:
				if (pStructure->ubWallOrientation != OUTSIDE_TOP_RIGHT &&
					pStructure->ubWallOrientation != INSIDE_TOP_RIGHT)
				{
					return false;
				}
				break;
		}

		// XXX left out 1.13 prison window check, hope STRUCTURE_OPEN is enough
		if ( fIntactWindowsAlso || ( pStructure->fFlags & STRUCTURE_OPEN ) ) return true;
	}

	return false;
}

BOOLEAN	IsJumpableFencePresentAtGridno( INT16 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_OBSTACLE );

	if ( pStructure )
	{
		if ( pStructure->fFlags & STRUCTURE_FENCE && !(pStructure->fFlags & STRUCTURE_SPECIAL) )
		{
			return true;
		}
		if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_SANDBAG &&
			StructureHeight( pStructure ) < 2 )
		{
			return true;
		}
	}

	return false;
}

STRUCTURE* GetWallStructOfSameOrientationAtGridno(GridNo const grid_no, INT8 const orientation)
{
	FOR_EACH_STRUCTURE(pStructure, grid_no, STRUCTURE_WALLSTUFF)
	{
		if (pStructure->ubWallOrientation != orientation) continue;

		STRUCTURE* const base = FindBaseStructure(pStructure);
		if (!base) continue;

		return base;
	}
	return 0;
}


BOOLEAN IsDoorVisibleAtGridNo( INT16 sGridNo )
{
	STRUCTURE *pStructure;
	INT16     sNewGridNo;

	pStructure = FindStructure( sGridNo, STRUCTURE_ANYDOOR );

	if ( pStructure != NULL )
	{
		// Check around based on orientation
		switch( pStructure->ubWallOrientation )
		{
			case INSIDE_TOP_LEFT:
			case OUTSIDE_TOP_LEFT:

				// Here, check north direction
				sNewGridNo = NewGridNo( sGridNo, DirectionInc( NORTH ) );

				if ( IsRoofVisible2( sNewGridNo ) )
				{
					// OK, now check south, if true, she's not visible
					sNewGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );

					if ( IsRoofVisible2( sNewGridNo ) )
					{
						return false;
					}
				}
				break;

			case INSIDE_TOP_RIGHT:
			case OUTSIDE_TOP_RIGHT:

				// Here, check west direction
				sNewGridNo = NewGridNo( sGridNo, DirectionInc( WEST ) );

				if ( IsRoofVisible2( sNewGridNo ) )
				{
					// OK, now check south, if true, she's not visible
					sNewGridNo = NewGridNo( sGridNo, DirectionInc( EAST ) );

					if ( IsRoofVisible2( sNewGridNo ) )
					{
						return false;
					}
				}
				break;

		}

	}

	// Return true here, even if she does not exist
	return true;
}


BOOLEAN	WallExistsOfTopLeftOrientation( INT16 sGridNo )
{
	// CJC: changing to search only for normal walls, July 16, 1998
	FOR_EACH_STRUCTURE(pStructure, sGridNo, STRUCTURE_WALL)
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
			pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
		{
			return true;
		}
	}

	return false;
}

BOOLEAN	WallExistsOfTopRightOrientation( INT16 sGridNo )
{
	// CJC: changing to search only for normal walls, July 16, 1998
	FOR_EACH_STRUCTURE(pStructure, sGridNo, STRUCTURE_WALL)
	{
		// Check orientation
		if ( pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
			pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
		{
			return true;
		}
	}

	return false;
}

BOOLEAN WallOrClosedDoorExistsOfTopLeftOrientation( INT16 sGridNo )
{
	FOR_EACH_STRUCTURE(pStructure, sGridNo, STRUCTURE_WALLSTUFF)
	{
		// skip it if it's an open door
		if (!((pStructure->fFlags & STRUCTURE_ANYDOOR) &&
			(pStructure->fFlags & STRUCTURE_OPEN )))
		{
			// Check orientation
			if (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
				pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT)
			{
				return true;
			}
		}
	}

	return false;
}

BOOLEAN WallOrClosedDoorExistsOfTopRightOrientation( INT16 sGridNo )
{
	FOR_EACH_STRUCTURE(pStructure, sGridNo, STRUCTURE_WALLSTUFF)
	{
		// skip it if it's an open door
		if (!((pStructure->fFlags & STRUCTURE_ANYDOOR) && (pStructure->fFlags & STRUCTURE_OPEN)))
		{
			// Check orientation
			if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
				pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
			{
				return true;
			}
		}
	}

	return false;
}

BOOLEAN OpenRightOrientedDoorWithDoorOnRightOfEdgeExists( INT16 sGridNo )
{
	FOR_EACH_STRUCTURE(pStructure, sGridNo, STRUCTURE_ANYDOOR)
	{
		if (!(pStructure->fFlags & STRUCTURE_OPEN)) break;
		// Check orientation
		if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
			pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
		{
			if ((pStructure->fFlags & STRUCTURE_DOOR) ||
				(pStructure->fFlags & STRUCTURE_DDOOR_RIGHT))
			{
				return true;
			}
		}
	}

	return false;
}

BOOLEAN OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists( INT16 sGridNo )
{
	FOR_EACH_STRUCTURE(pStructure, sGridNo, STRUCTURE_ANYDOOR)
	{
		if (!(pStructure->fFlags & STRUCTURE_OPEN)) break;
		// Check orientation
		if (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
			pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT)
		{
			if ((pStructure->fFlags & STRUCTURE_DOOR) ||
				(pStructure->fFlags & STRUCTURE_DDOOR_LEFT))
			{
				return true;
			}
		}
	}

	return false;
}


static STRUCTURE* FindCuttableWireFenceAtGridNo(INT16 sGridNo)
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_WIREFENCE );
	if (pStructure != NULL && pStructure->ubWallOrientation != NO_ORIENTATION &&
		!(pStructure->fFlags & STRUCTURE_OPEN))
	{
		return( pStructure );
	}
	return( NULL );
}

BOOLEAN CutWireFence( INT16 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindCuttableWireFenceAtGridNo( sGridNo );
	if (pStructure)
	{
		pStructure = SwapStructureForPartnerAndStoreChangeInMap(pStructure);
		if (pStructure)
		{
			RecompileLocalMovementCosts( sGridNo );
			SetRenderFlags( RENDER_FLAG_FULL );
			return true;
		}
	}
	return false;
}

BOOLEAN IsCuttableWireFenceAtGridNo( INT16 sGridNo )
{
	return( FindCuttableWireFenceAtGridNo( sGridNo ) != NULL );
}


UINT8 IsRepairableStructAtGridNo(const INT16 sGridNo, SOLDIERTYPE** const tgt)
{
	// OK, first look for a vehicle....
	SOLDIERTYPE* const s = WhoIsThere2(sGridNo, 0);
	if (tgt != NULL) *tgt = s;

	if (s != NULL && s->uiStatusFlags & SOLDIER_VEHICLE) return 2;
	// Then for over a robot....

	return false;
}


SOLDIERTYPE* GetRefuelableStructAtGridNo(INT16 sGridNo)
{
	// OK, first look for a vehicle....
	SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, 0);
	return tgt != NULL && tgt->uiStatusFlags & SOLDIER_VEHICLE ? tgt : NULL;
}

INT16 FindDoorAtGridNoOrAdjacent( INT16 sGridNo )
{
	STRUCTURE *pStructure;
	STRUCTURE *pBaseStructure;
	INT16     sTestGridNo;

	sTestGridNo = sGridNo;
	pStructure = FindStructure( sTestGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
		return( pBaseStructure->sGridNo );
	}

	sTestGridNo = sGridNo + DirectionInc( NORTH );
	pStructure = FindStructure( sTestGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
		return( pBaseStructure->sGridNo );
	}

	sTestGridNo = sGridNo + DirectionInc( WEST );
	pStructure = FindStructure( sTestGridNo, STRUCTURE_ANYDOOR );
	if (pStructure)
	{
		pBaseStructure = FindBaseStructure( pStructure );
		return( pBaseStructure->sGridNo );
	}

	return( NOWHERE );
}



BOOLEAN IsCorpseAtGridNo( INT16 sGridNo, UINT8 ubLevel )
{
	if ( GetCorpseAtGridNo( sGridNo , ubLevel ) != NULL )
	{
		return true;
	}
	else
	{
		return false;
	}
}


BOOLEAN SetOpenableStructureToClosed( INT16 sGridNo, UINT8 ubLevel )
{
	STRUCTURE *pStructure;
	STRUCTURE *pNewStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_OPENABLE );
	if ( !pStructure )
	{
		return false;
	}

	if ( pStructure->fFlags & STRUCTURE_OPEN )
	{
		pNewStructure = SwapStructureForPartner(pStructure);
		if ( pNewStructure != NULL)
		{
			RecompileLocalMovementCosts( sGridNo );
			SetRenderFlags( RENDER_FLAG_FULL );
		}
	}
	// else leave it as is!
	return true;
}
