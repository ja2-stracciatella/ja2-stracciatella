#include "Soldier_Control.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Isometric_Utils.h"
#include "Interface_Panels.h"
#include "Soldier_Macros.h"
#include "Strategic.h"
#include "Animation_Control.h"
#include "Soldier_Add.h"
#include "Map_Information.h"
#include "FOV.h"
#include "PathAI.h"
#include "Random.h"
#include "Render_Fun.h"
#include "Meanwhile.h"
#include "Exit_Grids.h"
#include "Debug.h"
#include "Structure.h"

#include "Soldier.h"

// SO, STEPS IN CREATING A MERC!

// 1 ) Setup the SOLDIERCREATE_STRUCT
//     Among other things, this struct needs a sSectorX, sSectorY, and a valid InsertionDirection
//     and InsertionGridNo.
//     This GridNo will be determined by a prevouis function that will examine the sector
//     Infomration regarding placement positions and pick one
// 2 ) Call TacticalCreateSoldier() which will create our soldier
//     What it does is: Creates a soldier in the Menptr[] array.
//                      Allocates the Animation cache for this merc
//                      Loads up the intial aniamtion file
//                      Creates initial palettes, etc
//                      And other cool things.
//     Now we have an allocated soldier, we just need to set him in the world!
// 3 ) When we want them in the world, call AddSoldierToSector().
//     This function sets the graphic in the world, lighting effects, etc
//     It also formally adds it to the TacticalSoldier slot and interface panel slot.



//Kris:  modified to actually path from sweetspot to gridno.  Previously, it only checked if the
//destination was sittable (though it was possible that that location would be trapped.
UINT16 FindGridNoFromSweetSpot(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT32 leftmost;
	SOLDIERTYPE soldier{};

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	// ATE: CHECK FOR BOUNDARIES!!!!!!
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, ( PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE ) );

	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					// ATE: INstead of using absolute range, use the path cost!
					//uiRange = PlotPath(&soldier, sGridNo, NO_COPYROUTE, NO_PLOT, WALKING, 50);
					uiRange = CardinalSpacesAway( sSweetGridNo, sGridNo );

					//if ( uiRange == 0 )
					//{
					//	uiRange = 999999;
					//}

					if ( uiRange < uiLowestRange )
					{
						sLowestGridNo = sGridNo;
						uiLowestRange = uiRange;
					}
				}
			}
		}
	}
	return sLowestGridNo;
}


UINT16 FindGridNoFromSweetSpotThroughPeople(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT32 leftmost;
	SOLDIERTYPE soldier{};

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier.bLevel = 0;
	soldier.bTeam = pSoldier->bTeam;
	soldier.sGridNo = sSweetGridNo;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	// ATE: CHECK FOR BOUNDARIES!!!!!!
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, ( PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE ) );

	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

					{
						if ( uiRange < uiLowestRange )
						{
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
						}
					}
				}
			}
		}
	}
	return sLowestGridNo;
}


//Kris:  modified to actually path from sweetspot to gridno.  Previously, it only checked if the
//destination was sittable (though it was possible that that location would be trapped.
UINT16 FindGridNoFromSweetSpotWithStructData( SOLDIERTYPE *pSoldier, UINT16 usAnimState, INT16 sSweetGridNo, INT8 ubRadius, UINT8 *pubDirection, BOOLEAN fClosestToMerc )
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2, cnt3;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT16 sLowestGridNo=-1;
	INT32 leftmost;
	BOOLEAN fFound = FALSE;
	SOLDIERTYPE soldier;
	UINT8 ubBestDirection=0;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier = SOLDIERTYPE{};
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	// If we are already at this gridno....
	if ( pSoldier->sGridNo == sSweetGridNo && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		*pubDirection = pSoldier->bDirection;
		return( sSweetGridNo );
	}

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	// ATE: CHECK FOR BOUNDARIES!!!!!!
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, ( PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE ) );

	uiLowestRange = 999999;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
				sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					BOOLEAN fDirectionFound = FALSE;
					UINT16 const usOKToAddStructID = GetStructureID(pSoldier);
					UINT16  usAnimSurface;

					// Get animation surface...
					usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );
					// Get structure ref...
					const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, usAnimState);
					Assert(pStructureFileRef);

					// Check each struct in each direction
					for( cnt3 = 0; cnt3 < 8; cnt3++ )
					{
						if (OkayToAddStructureToWorld(sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(cnt3)], usOKToAddStructID))
						{
							fDirectionFound = TRUE;
							break;
						}

					}

					if ( fDirectionFound )
					{
						if ( fClosestToMerc )
						{
							uiRange = FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, 0 );

							if (uiRange == 0 )
							{
								// If we don't have a path, this cannot be the best gridno!
								continue;
							}
						}
						else
						{
							uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );
						}

						if ( uiRange < uiLowestRange )
						{
							ubBestDirection = (UINT8)cnt3;
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
							fFound = TRUE;
						}
					}
				}
			}
		}
	}

	if ( fFound )
	{
		// Set direction we chose...
		*pubDirection = ubBestDirection;

		return( sLowestGridNo );
	}
	else
	{
		return( NOWHERE );
	}
}


static UINT16 FindGridNoFromSweetSpotWithStructDataUsingGivenDirectionFirst(SOLDIERTYPE* pSoldier, UINT16 usAnimState, INT16 sSweetGridNo, INT8 ubRadius, UINT8* pubDirection, BOOLEAN fClosestToMerc, INT8 bGivenDirection)
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2, cnt3;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT16 sLowestGridNo = -1;
	INT32 leftmost;
	BOOLEAN fFound = FALSE;
	SOLDIERTYPE soldier;
	UINT8 ubBestDirection = 0;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier = SOLDIERTYPE{};
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	// If we are already at this gridno....
	if ( pSoldier->sGridNo == sSweetGridNo && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		*pubDirection = pSoldier->bDirection;
		return( sSweetGridNo );
	}


	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	// ATE: CHECK FOR BOUNDARIES!!!!!!
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, ( PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE ) );

	uiLowestRange = 999999;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
				sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					BOOLEAN fDirectionFound = FALSE;
					UINT16 const usOKToAddStructID = GetStructureID(pSoldier);
					UINT16  usAnimSurface;

					// Get animation surface...
					usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );
					// Get structure ref...
					const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, usAnimState);
					Assert(pStructureFileRef);

					// OK, check the perfered given direction first
					if (OkayToAddStructureToWorld(sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(bGivenDirection)], usOKToAddStructID))
					{
						fDirectionFound = TRUE;
						cnt3 = bGivenDirection;
					}
					else
					{
						// Check each struct in each direction
						for( cnt3 = 0; cnt3 < 8; cnt3++ )
						{
							if ( cnt3 != bGivenDirection )
							{
								if (OkayToAddStructureToWorld(sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(cnt3)], usOKToAddStructID))
								{
									fDirectionFound = TRUE;
									break;
								}
							}
						}
					}

					if ( fDirectionFound )
					{
						if ( fClosestToMerc )
						{
							uiRange = FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE, 0 );

							if (uiRange == 0 )
							{
								// If we don't have a path, this cannot be the best gridno!
								continue;
							}
						}
						else
						{
							uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );
						}

						if ( uiRange < uiLowestRange )
						{
							ubBestDirection = (UINT8)cnt3;
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
							fFound = TRUE;
						}
					}
				}
			}
		}
	}

	if ( fFound )
	{
		// Set direction we chose...
		*pubDirection = ubBestDirection;

		return( sLowestGridNo );
	}
	else
	{
		return( NOWHERE );
	}
}


UINT16 FindGridNoFromSweetSpotWithStructDataFromSoldier(const SOLDIERTYPE* const pSoldier, const UINT16 usAnimState, const INT8 ubRadius, const BOOLEAN fClosestToMerc, const SOLDIERTYPE* const pSrcSoldier)
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2, cnt3;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT32 leftmost;
	INT16 sSweetGridNo;
	SOLDIERTYPE soldier{};

	sSweetGridNo = pSrcSoldier->sGridNo;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	// ATE: CHECK FOR BOUNDARIES!!!!!!
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, ( PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE ) );

	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					BOOLEAN fDirectionFound = FALSE;
					UINT16  usAnimSurface;

					if ( fClosestToMerc != 3 )
					{
						UINT16 const usOKToAddStructID = GetStructureID(pSoldier);

						// Get animation surface...
						usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );
						// Get structure ref...
						const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, usAnimState);
						Assert(pStructureFileRef);

						// Check each struct in each direction
						for( cnt3 = 0; cnt3 < 8; cnt3++ )
						{
							if (OkayToAddStructureToWorld(sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(cnt3)], usOKToAddStructID))
							{
								fDirectionFound = TRUE;
								break;
							}

						}
					}
					else
					{
						fDirectionFound = TRUE;
					}

					if ( fDirectionFound )
					{
						if ( fClosestToMerc == 1 )
						{
							uiRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );
						}
						else if ( fClosestToMerc == 2 )
						{
							uiRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo ) + GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );
						}
						else
						{
							//uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );
							uiRange = std::abs((sSweetGridNo / MAXCOL) - (sGridNo / MAXCOL)) +
								std::abs((sSweetGridNo % MAXROW) - (sGridNo % MAXROW));
						}

						if ( uiRange < uiLowestRange || (uiRange == uiLowestRange && PythSpacesAway( pSoldier->sGridNo, sGridNo ) < PythSpacesAway( pSoldier->sGridNo, sLowestGridNo ) ) )
						{
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
						}
					}
				}
			}
		}
	}
	return sLowestGridNo;
}


UINT16 FindGridNoFromSweetSpotExcludingSweetSpot(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT32 leftmost;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;

			if ( sSweetGridNo == sGridNo )
			{
				continue;
			}

			if ( sGridNo >=0 && sGridNo < WORLD_MAX &&
				sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{

					// Go on sweet stop
					if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
					{
						uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

						if ( uiRange < uiLowestRange )
						{
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
						}
					}
			}
		}
	}

	return sLowestGridNo;
}


UINT16 FindGridNoFromSweetSpotExcludingSweetSpotInQuardent(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius, const INT8 ubQuardentDir)
{
	INT16 sTop, sBottom;
	INT16 sLeft, sRight;
	INT16 cnt1, cnt2;
	INT16 sGridNo;
	INT32 uiRange, uiLowestRange = 999999;
	INT32 leftmost;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	// Switch on quadrent
	if ( ubQuardentDir == SOUTHEAST )
	{
		sBottom = 0;
		sLeft = 0;
	}

	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;

			if ( sSweetGridNo == sGridNo )
			{
				continue;
			}

			if ( sGridNo >=0 && sGridNo < WORLD_MAX &&
				sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{

					// Go on sweet stop
					if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
					{
						uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

						if ( uiRange < uiLowestRange )
						{
							sLowestGridNo = sGridNo;
							uiLowestRange = uiRange;
						}
					}
			}
		}
	}

	return sLowestGridNo;
}


BOOLEAN CanSoldierReachGridNoInGivenTileLimit( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT16 sMaxTiles, INT8 bLevel )
{
	INT32 iNumTiles;
	INT16 sActionGridNo;

	if ( pSoldier->bLevel != bLevel )
	{
		return( FALSE );
	}

	sActionGridNo = FindAdjacentGridEx(pSoldier, sGridNo, NULL, NULL, FALSE, FALSE);

	if ( sActionGridNo == -1 )
	{
		sActionGridNo = sGridNo;
	}

	if ( sActionGridNo == pSoldier->sGridNo )
	{
		return( TRUE );
	}

	iNumTiles = FindBestPath( pSoldier, sActionGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_IGNORE_PERSON_AT_DEST );

	if ( iNumTiles <= sMaxTiles && iNumTiles != 0 )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


UINT16 FindRandomGridNoFromSweetSpot(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16   sX, sY;
	INT16   sGridNo;
	INT32   leftmost;
	BOOLEAN fFound = FALSE;
	UINT32  cnt = 0;
	SOLDIERTYPE soldier;
	INT16   sTop, sBottom;
	INT16   sLeft, sRight;
	INT16   cnt1, cnt2;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier = SOLDIERTYPE{};
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	// ATE: CHECK FOR BOUNDARIES!!!!!!
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, ( PATH_IGNORE_PERSON_AT_DEST | PATH_THROUGH_PEOPLE ) );

	do
	{
		sX = Random( ubRadius * 2 + 1 ) - ubRadius;
		sY = Random( ubRadius * 2 + 1 ) - ubRadius;

		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * sY ) )/ WORLD_COLS ) * WORLD_COLS;

		sGridNo = sSweetGridNo + ( WORLD_COLS * sY ) + sX;

		if ( sGridNo >=0 && sGridNo < WORLD_MAX &&
			sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS )
			&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
		{
			// Go on sweet stop
			if ( NewOKDestination( pSoldier, sGridNo, TRUE , pSoldier->bLevel) )
			{
				// If we are a crow, we need this additional check
				if ( pSoldier->ubBodyType == CROW )
				{
					if (GetRoom(sGridNo) == NO_ROOM)
					{
						fFound = TRUE;
					}
				}
				else
				{
					fFound = TRUE;
				}
			}
		}

		cnt++;

		if ( cnt > 2000 )
		{
			return( NOWHERE );
		}

	} while( !fFound );

	return( sGridNo );
}


static void AddSoldierToSectorGridNo(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8 ubDirection, BOOLEAN fUseAnimation, UINT16 usAnimState, UINT16 usAnimCode);


static void InternalAddSoldierToSector(SOLDIERTYPE* const s, BOOLEAN calculate_direction, BOOLEAN const use_animation, UINT16 const anim_state, UINT16 const anim_code)
{
	if (!s->bActive) return;

	// ATE: Make sure life of elliot is OK if from a meanwhile
	if (AreInMeanwhile() && s->ubProfile == ELLIOT && s->bLife < OKLIFE)
	{
		s->bLife = 25;
	}

	// ADD SOLDIER TO SLOT!
	if (s->uiStatusFlags & SOLDIER_OFF_MAP)
	{
		AddAwaySlot(s);
		// Guy is NOT "in sector"
		s->bInSector = FALSE;
	}
	else
	{
		AddMercSlot(s);
		// Add guy to sector flag
		s->bInSector = TRUE;
	}

	// If a driver or passenger - stop here!
	if (s->uiStatusFlags & SOLDIER_DRIVER)    return;
	if (s->uiStatusFlags & SOLDIER_PASSENGER) return;

	CheckForAndAddMercToTeamPanel(s);

	s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK;
	s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_SMELLED_CREATURE;
	s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES;

	INT16 gridno;
	UINT8 direction;
	UINT8 calculated_direction;
	if (s->bTeam == CREATURE_TEAM)
	{
		gridno    = FindGridNoFromSweetSpotWithStructData(s, STANDING, s->sInsertionGridNo, 7, &calculated_direction, FALSE);
		direction = calculate_direction ? calculated_direction : s->ubInsertionDirection;
	}
	else
	{
		if (s->sInsertionGridNo == NOWHERE)
		{
			//Add the soldier to the respective entrypoint.  This is an error condition.
		}

		if (s->uiStatusFlags & SOLDIER_VEHICLE)
		{
			gridno = FindGridNoFromSweetSpotWithStructDataUsingGivenDirectionFirst(s, STANDING, s->sInsertionGridNo, 12, &calculated_direction, FALSE, s->ubInsertionDirection);
			// ATE: Override insertion direction
			s->ubInsertionDirection = calculated_direction;
		}
		else
		{
			gridno = FindGridNoFromSweetSpot(s, s->sInsertionGridNo, 7);
			if (gridno == NOWHERE)
			{
				// ATE: Error condition - if nowhere use insertion gridno!
				gridno = s->sInsertionGridNo;
			}
			calculated_direction = GetDirectionToGridNoFromGridNo(gridno, CENTER_GRIDNO);
		}

		// Override calculated direction if we were told to....
		if (s->ubInsertionDirection >= 100)
		{
			s->ubInsertionDirection -= 100;
			calculate_direction      = FALSE;
		}

		if (calculate_direction)
		{
			direction = calculated_direction;

			// Check if we need to get direction from exit grid
			if (s->bUseExitGridForReentryDirection)
			{
				s->bUseExitGridForReentryDirection = FALSE;

				// OK, we know there must be an exit gridno SOMEWHERE close
				INT16 const sExitGridNo = FindClosestExitGrid(s, gridno, 10);
				if (sExitGridNo != NOWHERE)
				{
					// We found one, calculate direction
					direction = (UINT8)GetDirectionToGridNoFromGridNo(sExitGridNo, gridno);
				}
			}
		}
		else
		{
			direction = s->ubInsertionDirection;
		}
	}

	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) direction = s->bDirection;
	if (direction >= 100) direction -= 100;
	AddSoldierToSectorGridNo(s, gridno, direction, use_animation, anim_state, anim_code);

	CheckForPotentialAddToBattleIncrement(s);
}


void AddSoldierToSector(SOLDIERTYPE* const s)
{
	InternalAddSoldierToSector(s, TRUE, FALSE, 0 , 0);
}


void AddSoldierToSectorNoCalculateDirection(SOLDIERTYPE* const s)
{
	InternalAddSoldierToSector(s, FALSE, FALSE, 0, 0);
}


void AddSoldierToSectorNoCalculateDirectionUseAnimation(SOLDIERTYPE* const s, UINT16 const usAnimState, UINT16 const usAnimCode)
{
	InternalAddSoldierToSector(s, FALSE, TRUE, usAnimState, usAnimCode);
}


static void PlaceSoldierNearSweetSpot(SOLDIERTYPE* const s, const UINT16 anim, const GridNo sweet_spot)
{
	// OK, look for suitable placement....
	UINT8 new_direction;
	const GridNo good_pos = FindGridNoFromSweetSpotWithStructData(s, anim, sweet_spot, 5, &new_direction, FALSE);
	EVENT_SetSoldierPosition(s, good_pos, SSP_NONE);
	EVENT_SetSoldierDirection(s, new_direction);
	EVENT_SetSoldierDesiredDirection(s, new_direction);
}


static void InternalSoldierInSectorSleep(SOLDIERTYPE* const s, INT16 const gridno)
{
	if (!s->bInSector) return;
	UINT16 const anim = AM_AN_EPC(s) ? STANDING : SLEEPING;
	PlaceSoldierNearSweetSpot(s, anim, gridno);
	EVENT_InitNewSoldierAnim( s, anim, 1, TRUE);
}


static void SoldierInSectorIncompaciated(SOLDIERTYPE* const s, INT16 const gridno)
{
	if (!s->bInSector) return;
	PlaceSoldierNearSweetSpot(s, STAND_FALLFORWARD_STOP, gridno);
	EVENT_InitNewSoldierAnim( s, STAND_FALLFORWARD_STOP, 1, TRUE);
}


static void SoldierInSectorAnim(SOLDIERTYPE* const s, INT16 const gridno, UINT16 anim_state)
{
	if (!s->bInSector) return;
	PlaceSoldierNearSweetSpot(s, anim_state, gridno);
	if (!IS_MERC_BODY_TYPE(s)) anim_state = STANDING;
	EVENT_InitNewSoldierAnim(s, anim_state, 1, TRUE);
}


void SoldierInSectorPatient(SOLDIERTYPE* const s, INT16 const gridno)
{
	SoldierInSectorAnim(s, gridno, BEING_PATIENT);
}


void SoldierInSectorDoctor(SOLDIERTYPE* const s, INT16 const gridno)
{
	SoldierInSectorAnim(s, gridno, BEING_DOCTOR);
}


void SoldierInSectorRepair(SOLDIERTYPE* const s, INT16 const gridno)
{
	SoldierInSectorAnim(s, gridno, BEING_REPAIRMAN);
}


static void AddSoldierToSectorGridNo(SOLDIERTYPE* const s, INT16 const sGridNo, UINT8 const ubDirection, BOOLEAN const fUseAnimation, UINT16 const usAnimState, UINT16 const usAnimCode)
{
	// Add merc to gridno

	// Set reserved location!
	s->sReservedMovementGridNo = NOWHERE;

	// Save OLD insertion code.. as this can change...
	UINT8 const insertion_code = s->ubStrategicInsertionCode;

	Soldier{s}.removePendingAnimation();

	//If we are not loading a saved game
	SetSoldierPosFlags set_pos_flags = SSP_NONE;
	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME)
	{
		// Set final dest to be the same...
		set_pos_flags = SSP_NO_DEST | SSP_NO_FINAL_DEST;
	}

	// If this is a special insertion location, get path!
	if (insertion_code == INSERTION_CODE_ARRIVING_GAME)
	{
		EVENT_SetSoldierPosition(        s, sGridNo, set_pos_flags);
		EVENT_SetSoldierDirection(       s, ubDirection);
		EVENT_SetSoldierDesiredDirection(s, ubDirection);
	}
	else if (insertion_code != INSERTION_CODE_CHOPPER)
	{
		EVENT_SetSoldierPosition(s, sGridNo, set_pos_flags);

		// if we are loading, dont set the direction (they are already set)
		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
		{
			EVENT_SetSoldierDirection(       s, ubDirection);
			EVENT_SetSoldierDesiredDirection(s, ubDirection);
		}
	}

	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

	if (!(s->uiStatusFlags & SOLDIER_DEAD) && s->bTeam == OUR_TEAM)
	{
		RevealRoofsAndItems(s, FALSE);

		// ATE: Patch fix: If we are in an non-interruptable animation, stop!
		if (s->usAnimState == HOPFENCE)
		{
			s->fInNonintAnim = FALSE;
			SoldierGotoStationaryStance(s);
		}

		EVENT_StopMerc(s, sGridNo, ubDirection);
	}

	// If just arriving, set a destination to walk into from!
	if (insertion_code == INSERTION_CODE_ARRIVING_GAME)
	{
		// Find a sweetspot near...
		INT16 const new_gridno = FindGridNoFromSweetSpot(s, gMapInformation.sNorthGridNo, 4);
		EVENT_GetNewSoldierPath(s, new_gridno, WALKING);
	}

	// If he's an enemy... set presence
	// ATE: Added if not bloodcats, only do this once they are seen
	if (!s->bNeutral && s->bSide != Side::FRIENDLY && s->ubBodyType != BLOODCAT)
	{
		SetEnemyPresence();
	}

	if (s->uiStatusFlags & SOLDIER_DEAD) return;

	// ATE: Double check if we are on the roof that there is a roof there!
	if (s->bLevel == 1 && !FindStructure(s->sGridNo, STRUCTURE_ROOF))
	{
		SetSoldierHeight(s, 0.0);
	}

	if (insertion_code == INSERTION_CODE_ARRIVING_GAME) return;

	// default to standing on arrival
	if (s->usAnimState != HELIDROP)
	{
		if (fUseAnimation)
		{
			EVENT_InitNewSoldierAnim(s, usAnimState, usAnimCode, TRUE);
		}
		else if (s->ubBodyType != CROW)
		{
			EVENT_InitNewSoldierAnim(s, STANDING, 1, TRUE);
		}
	}

	// ATE: if we are below OK life, make them lie down!
	if (s->bLife < OKLIFE)
	{
		SoldierInSectorIncompaciated(s, s->sInsertionGridNo);
	}
	else if (s->fMercAsleep)
	{
		InternalSoldierInSectorSleep(s, s->sInsertionGridNo);
	}
	else switch (s->bAssignment)
	{
		case PATIENT: SoldierInSectorPatient(s, s->sInsertionGridNo); break;
		case DOCTOR:  SoldierInSectorDoctor( s, s->sInsertionGridNo); break;
		case REPAIR:  SoldierInSectorRepair( s, s->sInsertionGridNo); break;
	}

	// ATE: Make sure movement mode is up to date!
	s->usUIMovementMode = GetMoveStateBasedOnStance(s, gAnimControl[s->usAnimState].ubEndHeight);
}


// IsMercOnTeam() checks to see if the passed in Merc Profile ID is currently on the player's team
BOOLEAN IsMercOnTeam(UINT8 ubMercID)
{
	const SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(ubMercID);
	return s != NULL;
}


BOOLEAN IsMercOnTeamAndInOmertaAlreadyAndAlive(UINT8 ubMercID)
{
	const SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(ubMercID);
	return s != NULL && s->bAssignment != IN_TRANSIT && s->bLife > 0;
}
