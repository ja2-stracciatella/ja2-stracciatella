#include "Soldier_Control.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Isometric_Utils.h"
#include "Interface_Panels.h"
#include "Soldier_Macros.h"
#include "StrategicMap.h"
#include "Strategic.h"
#include "Animation_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "Soldier_Add.h"
#include "Map_Information.h"
#include "FOV.h"
#include "PathAI.h"
#include "Random.h"
#include "Render_Fun.h"
#include "Meanwhile.h"
#include "Exit_Grids.h"
#include "Debug.h"
#include <stdlib.h>


// SO, STEPS IN CREATING A MERC!

// 1 ) Setup the SOLDIERCREATE_STRUCT
//			Among other things, this struct needs a sSectorX, sSectorY, and a valid InsertionDirection
//			and InsertionGridNo.
//			This GridNo will be determined by a prevouis function that will examine the sector
//			Infomration regarding placement positions and pick one
// 2 ) Call TacticalCreateSoldier() which will create our soldier
//			What it does is:	Creates a soldier in the MercPtrs[] array.
//												Allocates the Animation cache for this merc
//												Loads up the intial aniamtion file
//												Creates initial palettes, etc
//												And other cool things.
//			Now we have an allocated soldier, we just need to set him in the world!
// 3) When we want them in the world, call AddSoldierToSector().
//			This function sets the graphic in the world, lighting effects, etc
//			It also formally adds it to the TacticalSoldier slot and interface panel slot.



//Kris:  modified to actually path from sweetspot to gridno.  Previously, it only checked if the
//destination was sittable (though it was possible that that location would be trapped.
UINT16 FindGridNoFromSweetSpot(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
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
				  //uiRange = PlotPath( &soldier, sGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY, WALKING, NOT_STEALTH, FORWARD, 50 );
					uiRange = CardinalSpacesAway( sSweetGridNo, sGridNo );

				//	if ( uiRange == 0 )
				//	{
				//		uiRange = 999999;
				//	}

					if ( uiRange < uiLowestRange )
					{
						sLowestGridNo = sGridNo;
						uiLowestRange = uiRange;
					}
				}
			}
		}
	}
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
	return sLowestGridNo;
}


UINT16 FindGridNoFromSweetSpotThroughPeople(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bLevel = 0;
	soldier.bTeam = pSoldier->bTeam;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
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
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
	return sLowestGridNo;
}


//Kris:  modified to actually path from sweetspot to gridno.  Previously, it only checked if the
//destination was sittable (though it was possible that that location would be trapped.
UINT16 FindGridNoFromSweetSpotWithStructData( SOLDIERTYPE *pSoldier, UINT16 usAnimState, INT16 sSweetGridNo, INT8 ubRadius, UINT8 *pubDirection, BOOLEAN fClosestToMerc )
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2, cnt3;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT16		sLowestGridNo=-1;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;
	UINT8	ubBestDirection=0;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
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
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					BOOLEAN fDirectionFound = FALSE;
					UINT16	usOKToAddStructID;
					STRUCTURE_FILE_REF * pStructureFileRef;
					UINT16							 usAnimSurface;

					if ( pSoldier->pLevelNode != NULL )
					{
						if ( pSoldier->pLevelNode->pStructureData != NULL )
						{
							usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
						}
						else
						{
							usOKToAddStructID = INVALID_STRUCTURE_ID;
						}
					}
					else
					{
						usOKToAddStructID = INVALID_STRUCTURE_ID;
					}

					// Get animation surface...
			 		usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );
					// Get structure ref...
					pStructureFileRef = GetAnimationStructureRef( pSoldier->ubID, usAnimSurface, usAnimState );

					if( !pStructureFileRef )
					{
						Assert( 0 );
					}

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
                uiRange = 999;
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
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
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
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2, cnt3;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT16		sLowestGridNo=-1;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;
	UINT8	ubBestDirection=0;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
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
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS )
				&& gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					BOOLEAN fDirectionFound = FALSE;
					UINT16	usOKToAddStructID;
					STRUCTURE_FILE_REF * pStructureFileRef;
					UINT16							 usAnimSurface;

					if ( pSoldier->pLevelNode != NULL )
					{
						if ( pSoldier->pLevelNode->pStructureData != NULL )
						{
							usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
						}
						else
						{
							usOKToAddStructID = INVALID_STRUCTURE_ID;
						}
					}
					else
					{
						usOKToAddStructID = INVALID_STRUCTURE_ID;
					}

					// Get animation surface...
			 		usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );
					// Get structure ref...
					pStructureFileRef = GetAnimationStructureRef( pSoldier->ubID, usAnimSurface, usAnimState );

					if( !pStructureFileRef )
					{
						Assert( 0 );
					}

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
                uiRange = 999;
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
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
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
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2, cnt3;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;
	INT16 sSweetGridNo;
	SOLDIERTYPE soldier;

	sSweetGridNo = pSrcSoldier->sGridNo;


	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
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
					UINT16	usOKToAddStructID;
					STRUCTURE_FILE_REF * pStructureFileRef;
					UINT16							 usAnimSurface;

					if ( fClosestToMerc != 3 )
					{
						if ( pSoldier->pLevelNode != NULL && pSoldier->pLevelNode->pStructureData != NULL )
						{
							usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
						}
						else
						{
							usOKToAddStructID = INVALID_STRUCTURE_ID;
						}

						// Get animation surface...
			 			usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );
						// Get structure ref...
						pStructureFileRef = GetAnimationStructureRef( pSoldier->ubID, usAnimSurface, usAnimState );

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
						cnt3 = (UINT8)Random( 8 );
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
							uiRange = abs((sSweetGridNo / MAXCOL) - (sGridNo / MAXCOL)) +
								abs((sSweetGridNo % MAXROW) - (sGridNo % MAXROW));
						}

						if ( uiRange < uiLowestRange || (uiRange == uiLowestRange && PythSpacesAway( pSoldier->sGridNo, sGridNo ) < PythSpacesAway( pSoldier->sGridNo, sLowestGridNo ) ) )
						{
							sLowestGridNo		= sGridNo;
							uiLowestRange		= uiRange;
						}
					}
				}
			}
		}
	}
	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;
	return sLowestGridNo;
}


UINT16 FindGridNoFromSweetSpotExcludingSweetSpot(const SOLDIERTYPE* const pSoldier, const INT16 sSweetGridNo, const INT8 ubRadius)
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;

	sTop		= ubRadius;
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
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;

	sTop		= ubRadius;
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
	INT16	sActionGridNo;

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
	INT16		sX, sY;
	INT16		sGridNo;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;
	UINT32		cnt = 0;
	SOLDIERTYPE soldier;
	UINT8 ubSaveNPCAPBudget;
	UINT8 ubSaveNPCDistLimit;
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	UINT8	 ubRoomNum;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	ubSaveNPCAPBudget = gubNPCAPBudget;
	ubSaveNPCDistLimit = gubNPCDistLimit;
	gubNPCAPBudget = 0;
	gubNPCDistLimit = ubRadius;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	memset( &soldier, 0, sizeof( SOLDIERTYPE ) );
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = sSweetGridNo;

	sTop		= ubRadius;
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
		sX = (UINT16)Random( ubRadius );
		sY = (UINT16)Random( ubRadius );

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
					if ( !InARoom( sGridNo, &ubRoomNum ) )
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

	gubNPCAPBudget = ubSaveNPCAPBudget;
	gubNPCDistLimit = ubSaveNPCDistLimit;

	return( sGridNo );
}


static void AddSoldierToSectorGridNo(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8 ubDirection, BOOLEAN fUseAnimation, UINT16 usAnimState, UINT16 usAnimCode);


static BOOLEAN InternalAddSoldierToSector(SOLDIERTYPE* const pSoldier, BOOLEAN fCalculateDirection, const BOOLEAN fUseAnimation, const UINT16 usAnimState, const UINT16 usAnimCode)
{
	UINT8					ubDirection, ubCalculatedDirection;
	INT16					sGridNo;
	INT16					sExitGridNo;

	if ( pSoldier->bActive  )
	{
    // ATE: Make sure life of elliot is OK if from a meanwhile
    if ( AreInMeanwhile() && pSoldier->ubProfile == ELLIOT )
    {
      if ( pSoldier->bLife < OKLIFE )
      {
        pSoldier->bLife = 25;
      }
    }

		// ADD SOLDIER TO SLOT!
		if (pSoldier->uiStatusFlags & SOLDIER_OFF_MAP)
		{
			AddAwaySlot( pSoldier );

			// Guy is NOT "in sector"
			pSoldier->bInSector = FALSE;

		}
		else
		{
			AddMercSlot( pSoldier );

			// Add guy to sector flag
			pSoldier->bInSector = TRUE;

		}

		// If a driver or passenger - stop here!
		if ( pSoldier->uiStatusFlags & SOLDIER_DRIVER || pSoldier->uiStatusFlags & SOLDIER_PASSENGER )
		{
			return( FALSE );
		}

		// Add to panel
		CheckForAndAddMercToTeamPanel( pSoldier );

		pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK);
		pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_SMELLED_CREATURE);
		pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES);

		// Add to interface if the are ours
		if ( pSoldier->bTeam == CREATURE_TEAM )
		{
			sGridNo = FindGridNoFromSweetSpotWithStructData( pSoldier, STANDING, pSoldier->sInsertionGridNo, 7, &ubCalculatedDirection, FALSE );
			if( fCalculateDirection )
				ubDirection = ubCalculatedDirection;
			else
				ubDirection = pSoldier->ubInsertionDirection;
		}
		else
		{
			if( pSoldier->sInsertionGridNo == NOWHERE )
			{ //Add the soldier to the respective entrypoint.  This is an error condition.


			}
			if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				sGridNo = FindGridNoFromSweetSpotWithStructDataUsingGivenDirectionFirst( pSoldier, STANDING, pSoldier->sInsertionGridNo, 12, &ubCalculatedDirection, FALSE, pSoldier->ubInsertionDirection );
        // ATE: Override insertion direction
				pSoldier->ubInsertionDirection = ubCalculatedDirection;
			}
			else
			{
				sGridNo = FindGridNoFromSweetSpot(pSoldier, pSoldier->sInsertionGridNo, 7);

        // ATE: Error condition - if nowhere use insertion gridno!
        if ( sGridNo == NOWHERE )
        {
          sGridNo = pSoldier->sInsertionGridNo;
        }
				else
				{
					ubCalculatedDirection = GetDirectionToGridNoFromGridNo(sGridNo, CENTER_GRIDNO);
				}
			}

			// Override calculated direction if we were told to....
			if ( pSoldier->ubInsertionDirection > 100 )
			{
				pSoldier->ubInsertionDirection = pSoldier->ubInsertionDirection - 100;
				fCalculateDirection = FALSE;
			}

			if ( fCalculateDirection )
			{
				ubDirection = ubCalculatedDirection;

				// Check if we need to get direction from exit grid...
				if ( pSoldier->bUseExitGridForReentryDirection )
				{
					pSoldier->bUseExitGridForReentryDirection = FALSE;

					// OK, we know there must be an exit gridno SOMEWHERE close...
					sExitGridNo = FindClosestExitGrid( pSoldier, sGridNo, 10 );

					if ( sExitGridNo != NOWHERE )
					{
						// We found one
						// Calculate direction...
						ubDirection = (UINT8)GetDirectionToGridNoFromGridNo( sExitGridNo, sGridNo );
					}
				}
			}
			else
			{
				ubDirection = pSoldier->ubInsertionDirection;
			}
		}

		//Add
		if(gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
			AddSoldierToSectorGridNo( pSoldier, sGridNo, pSoldier->bDirection, fUseAnimation, usAnimState, usAnimCode );
		else
			AddSoldierToSectorGridNo( pSoldier, sGridNo, ubDirection, fUseAnimation, usAnimState, usAnimCode );

		CheckForPotentialAddToBattleIncrement( pSoldier );

		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN AddSoldierToSector(SOLDIERTYPE* const s)
{
	return InternalAddSoldierToSector(s, TRUE, FALSE, 0 , 0);
}


BOOLEAN AddSoldierToSectorNoCalculateDirection(SOLDIERTYPE* const s)
{
	return InternalAddSoldierToSector(s, FALSE, FALSE, 0, 0);
}


BOOLEAN AddSoldierToSectorNoCalculateDirectionUseAnimation(SOLDIERTYPE* const s, UINT16 usAnimState, UINT16 usAnimCode)
{
	return InternalAddSoldierToSector(s, FALSE, TRUE, usAnimState, usAnimCode);
}


static void PlaceSoldierNearSweetSpot(SOLDIERTYPE* const s, const UINT16 anim, const GridNo sweet_spot)
{
	// OK, look for suitable placement....
	UINT8	new_direction;
	const GridNo good_pos = FindGridNoFromSweetSpotWithStructData(s, anim, sweet_spot, 5, &new_direction, FALSE);
	EVENT_SetSoldierPosition(s, good_pos, SSP_NONE);
	EVENT_SetSoldierDirection(s, new_direction);
	EVENT_SetSoldierDesiredDirection(s, new_direction);
}


static void InternalSoldierInSectorSleep(SOLDIERTYPE* pSoldier, INT16 sGridNo)
{
	UINT16	usAnim = SLEEPING;

	if ( !pSoldier->bInSector )
	{
		return;
	}

	if ( AM_AN_EPC( pSoldier ) )
	{
	   usAnim = STANDING;
	}

	PlaceSoldierNearSweetSpot(pSoldier, usAnim, sGridNo);

	if ( AM_AN_EPC( pSoldier ) )
	{
		EVENT_InitNewSoldierAnim( pSoldier, STANDING, 1, TRUE );
	}
	else
	{
		EVENT_InitNewSoldierAnim(pSoldier, SLEEPING, 1, TRUE);
	}
}


static void SoldierInSectorIncompaciated(SOLDIERTYPE* pSoldier, INT16 sGridNo)
{
	if ( !pSoldier->bInSector )
	{
		return;
	}

	PlaceSoldierNearSweetSpot(pSoldier, STAND_FALLFORWARD_STOP, sGridNo);

	EVENT_InitNewSoldierAnim( pSoldier, STAND_FALLFORWARD_STOP, 1, TRUE );
}


void SoldierInSectorPatient( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	if ( !pSoldier->bInSector )
	{
		return;
	}

	PlaceSoldierNearSweetSpot(pSoldier, BEING_PATIENT, sGridNo);

	if ( !IS_MERC_BODY_TYPE( pSoldier ) )
	{
		EVENT_InitNewSoldierAnim( pSoldier, STANDING, 1, TRUE );
	}
	else
	{
		EVENT_InitNewSoldierAnim( pSoldier, BEING_PATIENT, 1, TRUE );
	}
}


void SoldierInSectorDoctor( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	if ( !pSoldier->bInSector )
	{
		return;
	}

	PlaceSoldierNearSweetSpot(pSoldier, BEING_DOCTOR, sGridNo);

	if ( !IS_MERC_BODY_TYPE( pSoldier ) )
	{
		EVENT_InitNewSoldierAnim( pSoldier, STANDING, 1, TRUE );
	}
	else
	{
		EVENT_InitNewSoldierAnim( pSoldier, BEING_DOCTOR, 1, TRUE );
	}
}


void SoldierInSectorRepair( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	if ( !pSoldier->bInSector )
	{
		return;
	}

	PlaceSoldierNearSweetSpot(pSoldier, BEING_REPAIRMAN, sGridNo);

	if ( !IS_MERC_BODY_TYPE( pSoldier ) )
	{
		EVENT_InitNewSoldierAnim( pSoldier, STANDING, 1, TRUE );
	}
	else
	{
		EVENT_InitNewSoldierAnim( pSoldier, BEING_REPAIRMAN, 1, TRUE );
	}
}


static void AddSoldierToSectorGridNo(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8 ubDirection, BOOLEAN fUseAnimation, UINT16 usAnimState, UINT16 usAnimCode)
{
	UINT8	ubInsertionCode;

	// Add merc to gridno

	// Set reserved location!
	pSoldier->sReservedMovementGridNo = NOWHERE;

	// Save OLD insertion code.. as this can change...
	ubInsertionCode = pSoldier->ubStrategicInsertionCode;

	// Remove any pending animations
	pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
	pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;
	pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
	pSoldier->ubPendingAction		 = NO_PENDING_ACTION;

	//If we are not loading a saved game
	SetSoldierPosFlags set_pos_flags = SSP_NONE;
	if( (gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// Set final dest to be the same...
		set_pos_flags = SSP_NO_DEST | SSP_NO_FINAL_DEST;
	}

	// If this is a special insertion location, get path!
	if ( ubInsertionCode == INSERTION_CODE_ARRIVING_GAME )
	{
		EVENT_SetSoldierPosition(pSoldier, sGridNo, set_pos_flags);
		EVENT_SetSoldierDirection( pSoldier, ubDirection );
		EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
	}
	else if ( ubInsertionCode == INSERTION_CODE_CHOPPER )
	{

	}
	else
	{
		EVENT_SetSoldierPosition(pSoldier, sGridNo, set_pos_flags);

		//if we are loading, dont set the direction ( they are already set )
		if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
		{
			EVENT_SetSoldierDirection( pSoldier, ubDirection );

			EVENT_SetSoldierDesiredDirection( pSoldier, ubDirection );
		}
	}

	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		if ( !( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
		{
			if ( pSoldier->bTeam == gbPlayerNum )
			{
				RevealRoofsAndItems( pSoldier, TRUE, FALSE, pSoldier->bLevel, TRUE );

        // ATE: Patch fix: If we are in an non-interruptable animation, stop!
        if ( pSoldier->usAnimState == HOPFENCE )
        {
          pSoldier->fInNonintAnim = FALSE;
          SoldierGotoStationaryStance( pSoldier );
        }

				EVENT_StopMerc( pSoldier, sGridNo, ubDirection );
			}
		}

		// If just arriving, set a destination to walk into from!
		if ( ubInsertionCode == INSERTION_CODE_ARRIVING_GAME )
		{
			// Find a sweetspot near...
			const INT16 sNewGridNo = FindGridNoFromSweetSpot(pSoldier, gMapInformation.sNorthGridNo, 4);
			EVENT_GetNewSoldierPath( pSoldier, sNewGridNo, WALKING );
		}

		// If he's an enemy... set presence
		if ( !pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum ) )
		{
      // ATE: Added if not bloodcats
      // only do this once they are seen.....
      if ( pSoldier->ubBodyType != BLOODCAT )
      {
			  SetEnemyPresence( );
      }
		}
	}

	if ( !( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
	{
		//if we are loading a 'pristine' map ( ie, not loading a saved game )
		if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
		{
			// ATE: Double check if we are on the roof that there is a roof there!
			if ( pSoldier->bLevel == 1 )
			{
				if ( !FindStructure( pSoldier->sGridNo, STRUCTURE_ROOF ) )
				{
					SetSoldierHeight( pSoldier, (FLOAT)( 0 )  );
				}
			}

			if ( ubInsertionCode != INSERTION_CODE_ARRIVING_GAME )
			{
				// default to standing on arrival
				if ( pSoldier->usAnimState != HELIDROP )
				{
					if ( fUseAnimation )
					{
						EVENT_InitNewSoldierAnim( pSoldier, usAnimState, usAnimCode, TRUE );
					}
					else if ( pSoldier->ubBodyType != CROW )
					{
						EVENT_InitNewSoldierAnim( pSoldier, STANDING, 1, TRUE );
					}
				}

				// ATE: if we are below OK life, make them lie down!
				if ( pSoldier->bLife < OKLIFE )
				{
					SoldierInSectorIncompaciated( pSoldier, pSoldier->sInsertionGridNo );
				}
				else if ( pSoldier->fMercAsleep == TRUE )
				{
					InternalSoldierInSectorSleep(pSoldier, pSoldier->sInsertionGridNo);
				}
				else if ( pSoldier->bAssignment == PATIENT )
				{
					SoldierInSectorPatient( pSoldier, pSoldier->sInsertionGridNo );
				}
				else if ( pSoldier->bAssignment == DOCTOR )
				{
					SoldierInSectorDoctor( pSoldier, pSoldier->sInsertionGridNo );
				}
				else if ( pSoldier->bAssignment == REPAIR )
				{
					SoldierInSectorRepair( pSoldier, pSoldier->sInsertionGridNo );
				}

        // ATE: Make sure movement mode is up to date!
  			pSoldier->usUIMovementMode =  GetMoveStateBasedOnStance( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight );

			}
		}
		else
		{
			// THIS ALL SHOULD HAVE BEEN HANDLED BY THE FACT THAT A GAME WAS LOADED

			//EVENT_InitNewSoldierAnim( pSoldier, pSoldier->usAnimState, pSoldier->usAniCode, TRUE );

			// if the merc had a final destination, get the merc walking there
			//if( pSoldier->sFinalDestination != pSoldier->sGridNo )
			//{
			//	EVENT_GetNewSoldierPath( pSoldier, pSoldier->sFinalDestination, pSoldier->usUIMovementMode );
			//}
		}
	}
}


// IsMercOnTeam() checks to see if the passed in Merc Profile ID is currently on the player's team
BOOLEAN IsMercOnTeam(UINT8 ubMercID)
{
	const SOLDIERTYPE* const s = GetSoldierFromMercID(ubMercID);
	return s != NULL;
}


// ATE: Added this new function for contract renewals
BOOLEAN IsMercOnTeamAndAlive(UINT8 ubMercID)
{
	const SOLDIERTYPE* const s = GetSoldierFromMercID(ubMercID);
	return s != NULL && s->bLife > 0;
}


BOOLEAN IsMercOnTeamAndInOmertaAlready(UINT8 ubMercID)
{
	const SOLDIERTYPE* const s = GetSoldierFromMercID(ubMercID);
	return s != NULL && s->bAssignment != IN_TRANSIT;
}


BOOLEAN IsMercOnTeamAndInOmertaAlreadyAndAlive(UINT8 ubMercID)
{
	const SOLDIERTYPE* const s = GetSoldierFromMercID(ubMercID);
	return s != NULL && s->bAssignment != IN_TRANSIT && s->bLife > 0;
}


SOLDIERTYPE* GetSoldierFromMercID(UINT8 ubMercID)
{
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->ubProfile == ubMercID) return s;
	}
	return NULL;
}
