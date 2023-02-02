#include "AI.h"
#include "AIInternals.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "WorldMan.h"
#include "PathAI.h"
#include "Points.h"
#include "Smell.h"
#include "Strategic_Pathing.h"
#include "Soldier_Control.h"
#include "Soldier_Macros.h"
#include "Render_Fun.h"
#include "Debug.h"
#include <algorithm>
#include <array>

//
// CJC's DG->JA2 conversion notes
//
// LegalNPCDestination - mode hardcoded to walking; C.O. tear gas related stuff commented out
// TryToResumeMovement - C.O. EscortedMoveCanceled call
// GoAsFarAsPossibleTowards - C.O. stuff related to current animation esp first aid

int LegalNPCDestination(SOLDIERTYPE *pSoldier, INT16 sGridno, UINT8 ubPathMode, UINT8 ubWaterOK, UINT8 fFlags)
{
	BOOLEAN fSkipTilesWithMercs;

	if ((sGridno < 0) || (sGridno >= GRIDSIZE))
	{
		return(FALSE);
	}

	// return false if gridno on different level from merc
	if ( GridNoOnVisibleWorldTile( pSoldier->sGridNo ) && gpWorldLevelData[ pSoldier->sGridNo ].sHeight != gpWorldLevelData[ sGridno ].sHeight )
	{
		return( FALSE );
	}

	// skip mercs if turnbased and adjacent AND not doing an IGNORE_PATH check (which is used almost exclusively by GoAsFarAsPossibleTowards)
	fSkipTilesWithMercs = (gfTurnBasedAI && ubPathMode != IGNORE_PATH && SpacesAway( pSoldier->sGridNo, sGridno ) == 1 );

	// if this gridno is an OK destination
	// AND the gridno is NOT in a tear-gassed tile when we have no gas mask
	// AND someone is NOT already standing there
	// AND we're NOT already standing at that gridno
	// AND the gridno hasn't been black-listed for us

	// Nov 28 98: skip people in destination tile if in turnbased
	if ( ( NewOKDestination(pSoldier, sGridno, fSkipTilesWithMercs, pSoldier->bLevel ) ) &&
		( !InGas( pSoldier, sGridno ) ) &&
		( sGridno != pSoldier->sGridNo ) &&
		( sGridno != pSoldier->sBlackList ) )
	/*
	if ( ( NewOKDestination(pSoldier, sGridno, FALSE, pSoldier->bLevel ) ) &&
		( !(gpWorldLevelData[ sGridno ].ubExtFlags[0] & (MAPELEMENT_EXT_SMOKE | MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS)) || IsWearingHeadGear(*pSoldier, GASMASK)) &&
		( sGridno != pSoldier->sGridNo ) &&
		( sGridno != pSoldier->sBlackList ) )*/
	/*
	if ( ( NewOKDestination(pSoldier,sGridno,ALLPEOPLE, pSoldier->bLevel ) ) &&
		( !(gpWorldLevelData[ sGridno ].ubExtFlags[0] & (MAPELEMENT_EXT_SMOKE | MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS)) || IsWearingHeadGear(*pSoldier, GASMASK)) &&
		( sGridno != pSoldier->sGridNo ) &&
		( sGridno != pSoldier->sBlackList ) )*/
	{

		// if water's a problem, and gridno is in a water tile (bridges are OK)
		if (!ubWaterOK && Water(sGridno))
			return(FALSE);

		// passed all checks, now try to make sure we can get there!
		switch (ubPathMode)
		{
			// if finding a path wasn't asked for (could have already been done,
			// for example), don't bother
			case IGNORE_PATH     :	return(TRUE);

			case ENSURE_PATH     :	if ( FindBestPath( pSoldier, sGridno, pSoldier->bLevel, WALKING, COPYROUTE, fFlags ) )
				{
					return(TRUE);        // legal destination
				}
				else // got this far, but found no clear path,
				{
					// so test fails
					return(FALSE);
				}
				// *** NOTE: movement mode hardcoded to WALKING !!!!!
			case ENSURE_PATH_COST:	return PlotPath(pSoldier, sGridno, FALSE, FALSE, WALKING, 0);

			default:
				return(FALSE);
		}
	}
	else  // something failed - didn't even have to test path
		return(FALSE);       	// illegal destination
}




bool TryToResumeMovement(SOLDIERTYPE * const pSoldier, GridNo const sGridno)
{
	// have to make sure the old destination is still legal (somebody may
	// have occupied the destination gridno in the meantime!)
	if (LegalNPCDestination(pSoldier,sGridno,ENSURE_PATH,WATEROK,0))
	{
		SLOGD("{} continues movement to gridno {}...",
			pSoldier->ubID, sGridno);

		pSoldier->bPathStored = TRUE;	// optimization - Ian

		// make him go to it (needed to continue movement across multiple turns)
		NewDest(pSoldier,sGridno);

		if (pSoldier->sDestination != sGridno)
		{
			// must work even for escorted civs, can't just set the flag
			CancelAIAction(pSoldier);
		}
		return true;
	}

	// don't black-list anything here, this situation can come up quite
	// legally if another soldier gets in the way between turns
	CancelAIAction(pSoldier);
	return false;
}


static INT16 NextPatrolPoint(SOLDIERTYPE* pSoldier)
{
	// patrol slot 0 is UNUSED, so max patrolCnt is actually only 9
	if ((pSoldier->bPatrolCnt < 1) || (pSoldier->bPatrolCnt >= MAXPATROLGRIDS))
	{
		return(NOWHERE);
	}
	pSoldier->bNextPatrolPnt++;

	// if there are no more patrol points, return back to the first one
	if (pSoldier->bNextPatrolPnt > pSoldier->bPatrolCnt)
		pSoldier->bNextPatrolPnt = 1;   // ZERO is not used!

	return(pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt]);
}



bool PointPatrolAI(SOLDIERTYPE * const pSoldier)
{
	GridNo sPatrolPoint = pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt];

	// if we're already there, advance next patrol point
	if (pSoldier->sGridNo == sPatrolPoint || pSoldier->bNextPatrolPnt == 0)
	{
		// find next valid patrol point
		do
		{
			sPatrolPoint = NextPatrolPoint(pSoldier);
		}
		while ((sPatrolPoint != NOWHERE) && (NewOKDestination(pSoldier,sPatrolPoint,IGNOREPEOPLE, pSoldier->bLevel) < 1));

		// if we're back where we started, then ALL other patrol points are junk!
		if (pSoldier->sGridNo == sPatrolPoint)
		{
			// force change of orders & an abort
			sPatrolPoint = NOWHERE;
		}
	}

	// if we don't have a legal patrol point
	if (sPatrolPoint == NOWHERE)
	{
		// over-ride orders to something safer
		pSoldier->bOrders = FARPATROL;
		return false;
	}


	// make sure we can get there from here at this time, if we can't get all
	// the way there, at least do our best to get close
	if (LegalNPCDestination(pSoldier,sPatrolPoint,ENSURE_PATH,WATEROK,0))
	{
		pSoldier->bPathStored = TRUE; // optimization - Ian
		pSoldier->usActionData = sPatrolPoint;
	}
	else
	{
		// temporarily extend roaming range to infinity by changing orders, else
		// this won't work if the next patrol point is > 10 tiles away!
		auto const bOldOrders = pSoldier->bOrders;
		pSoldier->bOrders	= ONCALL;

		pSoldier->usActionData = GoAsFarAsPossibleTowards(pSoldier,sPatrolPoint,pSoldier->bAction);

		pSoldier->bOrders = bOldOrders;

		// if it's not possible to get any closer, that's OK, but fail this call
		if (pSoldier->usActionData == NOWHERE)
			return false;
	}

	// passed all tests - start moving towards next patrol point
	return true;
}

bool RandomPointPatrolAI(SOLDIERTYPE * const pSoldier)
{
	GridNo sPatrolPoint = pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt];

	// if we're already there, advance next patrol point
	if (pSoldier->sGridNo == sPatrolPoint || pSoldier->bNextPatrolPnt == 0)
	{
		// find next valid patrol point
		// we keep a count of the # of times we are in here to make sure we don't get into an endless
		//loop
		INT8 bCnt{0};
		do
		{
			// usPatrolGrid[0] gets used for centre of close etc patrols, so we have to add 1 to the Random #
			auto const patrolIndex = PreRandom(pSoldier->bPatrolCnt) + 1;
			sPatrolPoint = pSoldier->usPatrolGrid[patrolIndex];
			bCnt++;
		}
		while ( (sPatrolPoint == pSoldier->sGridNo) || ( (sPatrolPoint != NOWHERE) && (bCnt < pSoldier->bPatrolCnt) && (NewOKDestination(pSoldier,sPatrolPoint,IGNOREPEOPLE, pSoldier->bLevel ) < 1)) );

		if (bCnt == pSoldier->bPatrolCnt)
		{
			// ok, we tried doing this randomly, didn't work well, so now do a linear search
			pSoldier->bNextPatrolPnt = 0;
			do
			{
				sPatrolPoint = NextPatrolPoint(pSoldier);
			}
			while ((sPatrolPoint != NOWHERE) && (NewOKDestination(pSoldier,sPatrolPoint,IGNOREPEOPLE, pSoldier->bLevel) < 1));
		}

		// do nothing this time around
		if (pSoldier->sGridNo == sPatrolPoint)
		{
			return false;
		}
	}

	// if we don't have a legal patrol point
	if (sPatrolPoint == NOWHERE)
	{
		// over-ride orders to something safer
		pSoldier->bOrders = FARPATROL;
		return false;
	}

	// make sure we can get there from here at this time, if we can't get all
	// the way there, at least do our best to get close
	if (LegalNPCDestination(pSoldier,sPatrolPoint,ENSURE_PATH,WATEROK,0))
	{
		pSoldier->bPathStored = TRUE; // optimization - Ian
		pSoldier->usActionData = sPatrolPoint;
	}
	else
	{
		// temporarily extend roaming range to infinity by changing orders, else
		// this won't work if the next patrol point is > 10 tiles away!
		auto const bOldOrders = pSoldier->bOrders;
		pSoldier->bOrders	= SEEKENEMY;

		pSoldier->usActionData = GoAsFarAsPossibleTowards(pSoldier,sPatrolPoint,pSoldier->bAction);

		pSoldier->bOrders = bOldOrders;

		// if it's not possible to get any closer, that's OK, but fail this call
		if (pSoldier->usActionData == NOWHERE)
			return false;
	}


	// passed all tests - start moving towards next patrol point
	SLOGD("{} - POINT PATROL to grid {}", pSoldier->name, pSoldier->usActionData);
	return true;
}



INT16 InternalGoAsFarAsPossibleTowards(SOLDIERTYPE *pSoldier, INT16 sDesGrid, INT8 bReserveAPs, INT8 bAction, INT8 fFlags )
{
	INT16 sLoop,sAPCost;
	INT16 sTempDest,sGoToGrid;
	INT16 sOrigin;
	UINT16 usMaxDist;
	INT8 fPathFlags;

	INT8 bAPsLeft = -1; // XXX HACK000E

	if ( bReserveAPs == -1 )
	{
		// default reserve points
		if ( CREATURE_OR_BLOODCAT( pSoldier ) )
		{
			bReserveAPs = 0;
		}
		else
		{
			bReserveAPs = MAX_AP_CARRIED;
		}
	}

	sTempDest = -1;

	// obtain maximum roaming distance from soldier's sOrigin
	usMaxDist = RoamingRange(pSoldier,&sOrigin);

	UINT8 room_required = NO_ROOM;
	if (pSoldier->bOrders <= CLOSEPATROL &&
		(pSoldier->bTeam == CIV_TEAM || pSoldier->ubProfile != NO_PROFILE) &&
		/* make sure this doesn't interfere with pathing for scripts */
		pSoldier->sAbsoluteFinalDestination != NOWHERE)
	{
		room_required = GetRoom(pSoldier->usPatrolGrid[0]);
	}

	pSoldier->usUIMovementMode = DetermineMovementMode(pSoldier, bAction );
	if ( pSoldier->usUIMovementMode == RUNNING && fFlags & FLAG_CAUTIOUS )
	{
		pSoldier->usUIMovementMode = WALKING;
	}

	// if soldier is ALREADY at the desired destination, quit right away
	if (sDesGrid == pSoldier->sGridNo)
	{
		return(NOWHERE);
	}

	// don't try to approach go after noises or enemies actually in water
	// would be too easy to throw rocks in water, etc. & distract the AI
	if (Water(sDesGrid))
	{
		return(NOWHERE);
	}

	fPathFlags = 0;
	if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{	/*
		if ( PythSpacesAway( pSoldier->sGridNo, sDesGrid ) <= PATH_CLOSE_RADIUS )
		{
			// then do a limited range path search and see if we can get there
			gubNPCDistLimit = 10;
			if ( !LegalNPCDestination( pSoldier, sDesGrid, ENSURE_PATH, NOWATER, fPathFlags) )
			{
				gubNPCDistLimit = 0;
				return( NOWHERE );
			}
			else
			{
				// allow attempt to path without 'good enough' flag on
				gubNPCDistLimit = 0;
			}
		}
		else
		{
		*/
			fPathFlags = PATH_CLOSE_GOOD_ENOUGH;
		//}
	}

	// first step: try to find an OK destination at or near the desired gridno
	if (!LegalNPCDestination(pSoldier,sDesGrid,ENSURE_PATH,NOWATER,fPathFlags))
	{
		SLOGD("destination Grid # itself not valid, looking around it");
		if ( CREATURE_OR_BLOODCAT( pSoldier ) )
		{
			// we tried to get close, failed; abort!
			return( NOWHERE );
		}
		else
		{
			// else look at the 8 nearest gridnos to sDesGrid for a valid destination
			bool fFound = false;

			// examine all 8 spots around 'sDesGrid'
			// keep looking while directions remain and a satisfactory one not found
			// randomly select a direction which hasn't been 'checked' yet
			std::array<UINT8, 8> directions{ 0, 1, 2, 3, 4, 5, 6, 7 };
			std::shuffle(directions.begin(), directions.end(), gRandomEngine);

			for (UINT8 const ubDirection : directions)
			{
				// determine the gridno 1 tile away from current friend in this direction
				sTempDest = NewGridNo(sDesGrid, DirectionInc(ubDirection));

				// if that's out of bounds, ignore it & check next direction
				if (sTempDest == sDesGrid)
					continue;

				if (LegalNPCDestination(pSoldier,sTempDest,ENSURE_PATH,NOWATER,0))
				{
					fFound = true;           // found a spot
					break;                   // stop checking in other directions
				}
			}

			if (!fFound)
			{
				return(NOWHERE);
			}

			// found a good grid #, this becomes our actual desired grid #
			sDesGrid = sTempDest;
		}
	}

	// HAVE FOUND AN OK destination AND PLOTTED A VALID BEST PATH TO IT
	sGoToGrid = pSoldier->sGridNo; // start back where soldier is standing now
	sAPCost = 0; // initialize path cost counter

	// we'll only go as far along the plotted route as is within our
	// permitted roaming range, and we'll stop as soon as we're down to <= 5 APs

	for (sLoop = 0; sLoop < (pSoldier->ubPathDataSize - pSoldier->ubPathIndex); sLoop++)
	{
		// what is the next gridno in the path?

		//sTempDest = NewGridNo( sGoToGrid,DirectionInc( pSoldier->ubPathingData[sLoop] + 1) );
		sTempDest = NewGridNo( sGoToGrid,DirectionInc( pSoldier->ubPathingData[sLoop]) );

		// this should NEVER be out of bounds
		if (sTempDest == sGoToGrid)
		{
			break; // quit here, sGoToGrid is where we are going
		}

		// if this takes us beyond our permitted "roaming range"
		if (SpacesAway(sOrigin,sTempDest) > usMaxDist)
			break; // quit here, sGoToGrid is where we are going

		if (room_required != NO_ROOM && room_required != GetRoom(sTempDest))
		{
			break; // quit here, limited by room!
		}

		if ( (fFlags & FLAG_STOPSHORT) && SpacesAway( sDesGrid, sTempDest ) <= STOPSHORTDIST )
		{
			break; // quit here, sGoToGrid is where we are going
		}

		// if this gridno is NOT a legal NPC destination
		// DONT'T test path again - that would replace the traced path! - Ian
		// NOTE: It's OK to go *THROUGH* water to try and get to the destination!
		if (!LegalNPCDestination(pSoldier,sTempDest,IGNORE_PATH,WATEROK,0))
			break; // quit here, sGoToGrid is where we are going


		// CAN'T CALL PathCost() HERE! IT CALLS findBestPath() and overwrites
		//       pathRouteToGo !!!  Gotta calculate the cost ourselves - Ian
		//
		//ubAPsLeft = pSoldier->bActionPoints - PathCost(pSoldier,sTempDest,FALSE,FALSE,FALSE,FALSE,FALSE);

		if (gfTurnBasedAI)
		{
			// if we're just starting the "costing" process (first gridno)
			if (sLoop == 0)
			{
				/*
				// first, add any additional costs - such as intermediate animations, etc.
				switch(pSoldier->anitype[pSoldier->anim])
				{
					// in theory, no NPC should ever be in one of these animations as
					// things stand (they don't medic anyone), but leave it for robustness
					case START_AID   :
					case GIVING_AID  : sAnimCost = AP_STOP_FIRST_AID;
						break;

					case TWISTOMACH  :
					case COLLAPSED   : sAnimCost = AP_GET_UP;
						break;

					case TWISTBACK   :
					case UNCONSCIOUS : sAnimCost = (AP_ROLL_OVER + AP_GET_UP);
						break;

					default          : sAnimCost = 0;
				}

				// this is our first cost
				sAPCost += sAnimCost;
				*/

				if (pSoldier->usUIMovementMode == RUNNING)
				{
					sAPCost += AP_START_RUN_COST;
				}
			}

			// ATE: Direction here?
			sAPCost += EstimateActionPointCost( pSoldier, sTempDest, (INT8) pSoldier->ubPathingData[sLoop], pSoldier->usUIMovementMode, (INT8) sLoop, (INT8) pSoldier->ubPathDataSize );

			bAPsLeft = pSoldier->bActionPoints - sAPCost;
		}

		// if after this, we have <= 5 APs remaining, that's far enough, break out
		// (the idea is to preserve APs so we can crouch or react if
		// necessary, and benefit from the carry-over next turn if not needed)
		// This routine is NOT used by any GREEN AI, so such caution is warranted!

		if ( gfTurnBasedAI && (bAPsLeft < bReserveAPs) )
			break;
		else
		{
			sGoToGrid = sTempDest;    // we're OK up to here

			// if exactly 5 APs left, don't bother checking any further
			if ( gfTurnBasedAI && (bAPsLeft == bReserveAPs) )
				break;
		}
	}


	// if it turned out we couldn't go even 1 tile towards the desired gridno
	if (sGoToGrid == pSoldier->sGridNo)
	{
		return(NOWHERE);             // then go nowhere
	}
	else
	{
		// possible optimization - stored path IS good if we're going all the way
		if (sGoToGrid == sDesGrid)
		{
			pSoldier->bPathStored = TRUE;
			pSoldier->sFinalDestination = sGoToGrid;
		}
		else if ( pSoldier->ubPathIndex == 0 )
		{
			// we can hack this surely! -- CJC
			pSoldier->bPathStored = TRUE;
			pSoldier->sFinalDestination = sGoToGrid;
			pSoldier->ubPathDataSize = sLoop + 1;
		}

		SLOGD("{} to {} with {} APs left", pSoldier->ubID,
					sGoToGrid, pSoldier->bActionPoints );
		return( sGoToGrid );
	}
}

INT16 GoAsFarAsPossibleTowards(SOLDIERTYPE *pSoldier, INT16 sDesGrid, INT8 bAction)
{
	return( InternalGoAsFarAsPossibleTowards( pSoldier, sDesGrid, -1, bAction, 0 ) );
}

void SoldierTriesToContinueAlongPath(SOLDIERTYPE *pSoldier)
{
	INT16 usNewGridNo,bAPCost;


	// turn off the flag now that we're going to do something about it...
	// ATE: USed to be redundent, now if called befroe NewDest can cause some side efects...
	// AdjustNoAPToFinishMove( pSoldier, FALSE );

	if (pSoldier->bNewSituation == IS_NEW_SITUATION)
	{
		CancelAIAction(pSoldier);
		return;
	}

	if (pSoldier->usActionData >= GRIDSIZE)
	{
		CancelAIAction(pSoldier);
		return;
	}

	if (!NewOKDestination( pSoldier,pSoldier->usActionData, TRUE, pSoldier->bLevel ))
	{
		CancelAIAction(pSoldier);
		return;
	}

	if (IsActionAffordable(pSoldier))
	{
		if (!pSoldier->bActionInProgress)
		{
			// start a move that didn't even get started before...
			// hope this works...
			NPCDoesAct(pSoldier);

			// perform the chosen action
			pSoldier->bActionInProgress = ExecuteAction(pSoldier); // if started, mark us as busy
		}
		else
		{
			// otherwise we shouldn't have to do anything(?)
		}
	}
	else
	{
		CancelAIAction(pSoldier);
		SLOGD("Soldier ({}) HAS NOT ENOUGH AP to continue along path", pSoldier->ubID);
	}

	usNewGridNo = NewGridNo( (UINT16)pSoldier->sGridNo, DirectionInc( pSoldier->ubPathingData[ pSoldier->ubPathIndex ] ) );

	// Find out how much it takes to move here!
	bAPCost = EstimateActionPointCost( pSoldier, usNewGridNo, (INT8)pSoldier->ubPathingData[ pSoldier->ubPathIndex ], pSoldier->usUIMovementMode, (INT8) pSoldier->ubPathIndex, (INT8) pSoldier->ubPathDataSize );

	if (pSoldier->bActionPoints >= bAPCost)
	{
		// seems to have enough points...
		NewDest(pSoldier,usNewGridNo);
		// maybe we didn't actually start the action last turn...
		pSoldier->bActionInProgress = TRUE;
		SLOGD("Soldier ({}) continues along path", pSoldier->ubID);
	}
	else
	{
		CancelAIAction(pSoldier);
		SLOGD("Soldier ({}) HAS NOT ENOUGH AP to continue along path", pSoldier->ubID);
	}
}


void HaltMoveForSoldierOutOfPoints(SOLDIERTYPE& s)
{
	// if a special move, ignore this
	if (gAnimControl[s.usAnimState].uiFlags & ANIM_SPECIALMOVE) return;

	// Record that this merc can no longer animate and why
	AdjustNoAPToFinishMove(&s, TRUE);

	// We'll keep his action intact though
	SLOGD("NO AP TO FINISH MOVE for {} ({} APs left)", s.ubID, s.bActionPoints);

	// If this dude is under AI right now, then pass the baton to someone else
	if (s.uiStatusFlags & SOLDIER_UNDERAICONTROL)
	{
		SLOGD("Ending turn for {} because out of APs for movement", s.ubID);
		EndAIGuysTurn(s);
	}
}


#define RADIUS 3

INT16 TrackScent( SOLDIERTYPE * pSoldier )
{
	// This function returns the best gridno to go to based on the scent being followed,
	// and the soldier (creature/animal)'s current direction (which is used to resolve
	// ties.
	INT32        iXDiff, iYDiff, iXIncr;
	INT32        iStart, iXStart, iYStart;
	INT32        iGridNo;
	INT8         bDir;
	INT32        iBestGridNo = NOWHERE;
	UINT8        ubBestDirDiff = 5, ubBestStrength = 0;
	UINT8        ubDirDiff, ubStrength;
	UINT8        ubSoughtSmell;
	MAP_ELEMENT *pMapElement;

	iStart = pSoldier->sGridNo;
	iXStart = iStart % WORLD_COLS;
	iYStart = iStart / WORLD_COLS;

	if (CREATURE_OR_BLOODCAT( pSoldier ) ) // or bloodcats
	{
		// tracking humans; search the edges of a 7x7 square for the
		// most promising tile
		ubSoughtSmell = HUMAN;
		for (iYDiff = -RADIUS; iYDiff < (RADIUS + 1); iYDiff++)
		{
			if (iYStart + iYDiff < 0)
			{
				// outside of map! might be on map further down...
				continue;
			}
			else if (iYStart + iYDiff > WORLD_ROWS)
			{
				// outside of bottom of map! abort!
				break;
			}
			if (iYDiff == -RADIUS || iYDiff == RADIUS)
			{
				iXIncr = 1;
			}
			else
			{
				// skip over the spots in the centre of the square
				iXIncr = RADIUS * 2;
			}
			for (iXDiff = -RADIUS; iXDiff < (RADIUS + 1); iXDiff += iXIncr)
			{
				iGridNo = iStart + iXDiff + iYDiff * WORLD_ROWS;
				if (std::abs(iGridNo % WORLD_ROWS - iXStart) > RADIUS)
				{
					// wrapped across map!
					continue;
				}
				if (LegalNPCDestination(pSoldier,pSoldier->usActionData,ENSURE_PATH,WATEROK,0))
				{
					// check this location out
					pMapElement = &(gpWorldLevelData[iGridNo]);
					if (pMapElement->ubSmellInfo && (SMELL_TYPE( pMapElement->ubSmellInfo ) == ubSoughtSmell))
					{
						ubStrength = SMELL_STRENGTH( pMapElement->ubSmellInfo );
						if (ubStrength > ubBestStrength )
						{
							iBestGridNo = iGridNo;
							ubBestStrength = ubStrength;
							bDir = atan8( (INT16) iXStart, (INT16) iYStart, (INT16) (iXStart + iXDiff), (INT16) (iYStart + iYDiff) );
							// now convert it into a difference in degree between it and our current dir
							ubBestDirDiff = std::abs(pSoldier->bDirection - bDir);
							if (ubBestDirDiff > 4 ) // dir 0 compared with dir 6, for instance
							{
								ubBestDirDiff = 8 - ubBestDirDiff;
							}
						}
						else if (ubStrength == ubBestStrength)
						{
							if (iBestGridNo == NOWHERE)
							{
								// first place we've found with the same strength
								iBestGridNo = iGridNo;
								ubBestStrength = ubStrength;
							}
							else
							{
								// use directions to decide between the two
								// start by calculating direction to the new gridno
								bDir = atan8( (INT16) iXStart, (INT16) iYStart, (INT16) (iXStart + iXDiff), (INT16) (iYStart + iYDiff) );
								// now convert it into a difference in degree between it and our current dir
								ubDirDiff = std::abs(pSoldier->bDirection - bDir);
								if (ubDirDiff > 4 ) // dir 0 compared with dir 6, for instance
								{
									ubDirDiff = 8 - ubDirDiff;
								}
								if (ubDirDiff < ubBestDirDiff || ((ubDirDiff == ubBestDirDiff) && Random( 2 )))
								{
									// follow this trail as its closer to the one we're following!
									// (in the case of a tie, we tossed a coin)
									ubBestDirDiff = ubDirDiff;

								}
							}
						}
					}
				}
			}
			// go on to next tile
		}
		// go on to next row
	}
	else
	{
		// who else can track?
	}
	if (iBestGridNo != NOWHERE )
	{
		pSoldier->usActionData = (INT16) iBestGridNo;
		return( (INT16) iBestGridNo );
	}
	return( 0 );
}
