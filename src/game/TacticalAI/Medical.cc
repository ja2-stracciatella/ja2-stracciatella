#include "Isometric_Utils.h"
#include "Medical.h"
#include "Types.h"
#include "AI.h"
#include "AIInternals.h"
#include "Animation_Control.h"
#include "PathAI.h"
#include "Overhead.h"
#include "Items.h"
#include "Buildings.h"
#include "WorldMan.h"
#include "Assignments.h"


extern BOOLEAN gfAutoBandageFailed;

//
// This file contains code devoted to the player AI-controlled medical system.  Maybe it
// can be used or adapted for the enemies too...
//

#define NOT_GOING_TO_DIE -1
#define NOT_GOING_TO_COLLAPSE -1


static BOOLEAN FindAutobandageClimbPoint(INT16 sDesiredGridNo, BOOLEAN fClimbUp)
{
	// checks for existance of location to climb up to building, not occupied by a medic
	BUILDING *	pBuilding;
	UINT8				ubNumClimbSpots;
	UINT8				ubLoop;

	pBuilding = FindBuilding( sDesiredGridNo );
	if (!pBuilding)
	{
		return( FALSE );
	}

	ubNumClimbSpots = pBuilding->ubNumClimbSpots;

	for ( ubLoop = 0; ubLoop < ubNumClimbSpots; ubLoop++ )
	{
		const SOLDIERTYPE* const up_there   = WhoIsThere2(pBuilding->sUpClimbSpots[ubLoop],   1);
		if (up_there   != NULL && !CanCharacterAutoBandageTeammate(up_there))   continue;
		const SOLDIERTYPE* const down_there = WhoIsThere2(pBuilding->sDownClimbSpots[ubLoop], 0);
		if (down_there != NULL && !CanCharacterAutoBandageTeammate(down_there)) continue;
		return( TRUE );
	}

	return( FALSE );
}


static BOOLEAN FullPatientCheck(const SOLDIERTYPE* const pPatient)
{
	if ( CanCharacterAutoBandageTeammate( pPatient ) )
	{
		// can bandage self!
		return( TRUE );
	}

	if ( pPatient->bLevel != 0 )
	{	// look for a clear spot for jumping up

		// special "closest" search that ignores climb spots IF they are occupied by non-medics
		return( FindAutobandageClimbPoint( pPatient->sGridNo, TRUE ) );
	}
	else
	{
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			// can this character help out?
			if (CanCharacterAutoBandageTeammate(s))
			{
				// can this guy path to the patient?
				if (s->bLevel == 0)
				{
					// do a regular path check
					if (FindBestPath(s, pPatient->sGridNo, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE))
					{
						return( TRUE );
					}
				}
				else
				{
					// if on different levels, assume okay
					return( TRUE );
				}
			}
		}
	}
	return( FALSE );
}

BOOLEAN CanAutoBandage( BOOLEAN fDoFullCheck )
{
	static const SOLDIERTYPE* soldier_for_full_check = NULL;

	// returns false if we should stop being in auto-bandage mode
	UINT8					ubMedics = 0, ubPatients = 0;

	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// can this character help out?
		if (CanCharacterAutoBandageTeammate(s))
		{
			// yep, up the number of medics in sector
			ubMedics++;
		}
	}

	if ( ubMedics == 0 )
	{
		// no one that can help
		return( FALSE );
	}

	CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// can this character be helped out by a teammate?
		if (CanCharacterBeAutoBandagedByTeammate(pSoldier))
		{
			// yep, up the number of patients awaiting treatment in sector
			ubPatients++;
			if (fDoFullCheck)
			{
				if (soldier_for_full_check == NULL)
				{
					// do this guy NEXT time around
					soldier_for_full_check = pSoldier;
				}
				else if (pSoldier == soldier_for_full_check)
				{
					// test this guy
					if (!FullPatientCheck(pSoldier))
					{
						// shit!
						gfAutoBandageFailed = TRUE;
						return( FALSE );
					}
					// set soldier for full check to NULL; will be set to someone later in loop, or to
					// the first guy on the next pass
					soldier_for_full_check = NULL;
				}
			}
		}
		// is this guy REACHABLE??
	}

	if ( ubPatients == 0 )
	{
		// there is no one to help
		return( FALSE );
	}
	else
	{
		// got someone that can help and help wanted
		return( TRUE );
	}
}


BOOLEAN CanCharacterAutoBandageTeammate(const SOLDIERTYPE* const pSoldier)
{
	// if the soldier isn't active or in sector, we have problems..leave
	if (!pSoldier->bInSector || pSoldier->uiStatusFlags & SOLDIER_VEHICLE || pSoldier->bAssignment == VEHICLE)
	{
		return( FALSE );
	}

	// they must have oklife or more, not be collapsed, have some level of medical competence, and have a med kit of some sort
	if ( (pSoldier->bLife >= OKLIFE) && !(pSoldier->bCollapsed) && (pSoldier->bMedical > 0) && (FindObjClass( pSoldier, IC_MEDKIT ) != NO_SLOT) )
	{
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN CanCharacterBeAutoBandagedByTeammate(const SOLDIERTYPE* const pSoldier)
{
	// if the soldier isn't in sector, we have problems..leave
	if (!pSoldier->bInSector || pSoldier->uiStatusFlags & SOLDIER_VEHICLE || pSoldier->bAssignment == VEHICLE)
	{
		return( FALSE );
	}

	if ( (pSoldier->bLife > 0) && (pSoldier->bBleeding > 0) )
	{
		// someone's bleeding and not being given first aid!
		return( TRUE );
	}

	return( FALSE );
}


static INT8 FindBestPatient(SOLDIERTYPE* pSoldier, BOOLEAN* pfDoClimb)
{
	INT16						bBestPriority = 0;
	INT16						sPatientGridNo;
	INT16						sShortestPath = 1000, sPathCost, sOtherMedicPathCost;
	SOLDIERTYPE *		pBestPatient = NULL;
	INT8						bPatientPriority;
	INT16						sClimbGridNo, sBestClimbGridNo = NOWHERE, sShortestClimbPath = 1000;
	BOOLEAN					fClimbingNecessary;

	GridNo sBestAdjGridNo     = NOWHERE; // XXX HACK000E
	GridNo sBestPatientGridNo = NOWHERE; // XXX HACK000E

	gubGlobalPathFlags = PATH_THROUGH_PEOPLE;

	// search for someone who needs aid
	FOR_EACH_IN_TEAM(pPatient, OUR_TEAM)
	{
		if (!pPatient->bInSector)
		{
			continue; // NEXT!!!
		}

		if (pPatient->bLife > 0 && pPatient->bBleeding && pPatient->ubServiceCount == 0)
		{
			if (pPatient->bLife < OKLIFE)
			{
				bPatientPriority = 3;
			}
			else if (pPatient->bLife < OKLIFE * 2)
			{
				bPatientPriority = 2;
			}
			else
			{
				bPatientPriority = 1;
			}

			if (bPatientPriority >= bBestPriority)
			{
				if ( !ClimbingNecessary( pSoldier, pPatient->sGridNo, pPatient->bLevel ) )
				{

					sPatientGridNo = pPatient->sGridNo;
					INT16 sAdjacentGridNo = FindAdjacentGridEx(pSoldier, sPatientGridNo, NULL, NULL, FALSE, FALSE);
					if ( sAdjacentGridNo == -1 && gAnimControl[ pPatient->usAnimState ].ubEndHeight == ANIM_PRONE )
					{
						// prone; could be the base tile is inaccessible but the rest isn't...
						for (UINT8 cnt2 = 0; cnt2 < NUM_WORLD_DIRECTIONS; ++cnt2)
						{
							sPatientGridNo = pPatient->sGridNo + DirectionInc( cnt2 );
							if (WhoIsThere2(sPatientGridNo, pPatient->bLevel) == pPatient)
							{
								// patient is also here, try this location
								sAdjacentGridNo = FindAdjacentGridEx(pSoldier, sPatientGridNo, NULL, NULL, FALSE, FALSE);
								if ( sAdjacentGridNo != -1 )
								{
									break;
								}
							}
						}
					}

					if (sAdjacentGridNo != -1)
					{
						if (sAdjacentGridNo == pSoldier->sGridNo)
						{
							sPathCost = 1;
						}
						else
						{
							sPathCost = PlotPath(pSoldier, sAdjacentGridNo, FALSE, FALSE, RUNNING, 0);
						}

						if ( sPathCost != 0 )
						{
							// we can get there... can anyone else?
							SOLDIERTYPE* const pOtherMedic = pPatient->auto_bandaging_medic;
							if (pOtherMedic != NULL && pOtherMedic != pSoldier)
							{
								// only switch to this patient if our distance is closer than
								// the other medic's
								const INT16 sOtherAdjacentGridNo = FindAdjacentGridEx(pOtherMedic, sPatientGridNo, NULL, NULL, FALSE, FALSE);
								if (sOtherAdjacentGridNo != -1)
								{

									if (sOtherAdjacentGridNo == pOtherMedic->sGridNo)
									{
										sOtherMedicPathCost = 1;
									}
									else
									{
										sOtherMedicPathCost = PlotPath(pOtherMedic, sOtherAdjacentGridNo, FALSE, FALSE, RUNNING, 0);
									}

									if (sPathCost >= sOtherMedicPathCost)
									{
										// this patient is best served by the merc moving to them now
										continue;
									}
								}
							}

							if (bPatientPriority == bBestPriority)
							{
								// compare path distances
								if ( sPathCost > sShortestPath )
								{
									continue;
								}
							}


							sShortestPath = sPathCost;
							pBestPatient = pPatient;
							sBestPatientGridNo = sPatientGridNo;
							bBestPriority = bPatientPriority;
							sBestAdjGridNo = sAdjacentGridNo;

						}
					}

				}
				else
				{
					sClimbGridNo = NOWHERE;
					// see if guy on another building etc and we need to climb somewhere
					sPathCost = EstimatePathCostToLocation( pSoldier, pPatient->sGridNo, pPatient->bLevel, FALSE, &fClimbingNecessary, &sClimbGridNo );
					// if we can get there
					if ( sPathCost != 0 && fClimbingNecessary && sPathCost < sShortestClimbPath )
					{
						sBestClimbGridNo = sClimbGridNo;
						sShortestClimbPath = sPathCost;
					}

				}

			}

		}
	}

	gubGlobalPathFlags = 0;

	if (pBestPatient)
	{
		if (pBestPatient->auto_bandaging_medic != NULL)
		{
			// cancel that medic
			CancelAIAction(pBestPatient->auto_bandaging_medic);
		}
		pBestPatient->auto_bandaging_medic = pSoldier;
		*pfDoClimb = FALSE;
		if ( CardinalSpacesAway( pSoldier->sGridNo, sBestPatientGridNo ) == 1 )
		{
			pSoldier->usActionData = sBestPatientGridNo;
			return( AI_ACTION_GIVE_AID );
		}
		else
		{
			pSoldier->usActionData = sBestAdjGridNo;
			return( AI_ACTION_GET_CLOSER );
		}
	}
	else if (sBestClimbGridNo != NOWHERE )
	{
		*pfDoClimb = TRUE;
		pSoldier->usActionData = sBestClimbGridNo;
		return( AI_ACTION_MOVE_TO_CLIMB );
	}
	else
	{
		return( AI_ACTION_NONE );
	}
}

INT8 DecideAutoBandage( SOLDIERTYPE * pSoldier )
{
	INT8					bSlot;
	BOOLEAN				fDoClimb;


	if (pSoldier->bMedical == 0 || pSoldier->service_partner != NULL)
	{
		// don't/can't make decision
		return( AI_ACTION_NONE );
	}

	bSlot = FindObjClass( pSoldier, IC_MEDKIT );
	if (bSlot == NO_SLOT)
	{
		// no medical kit!
		return( AI_ACTION_NONE );
	}

	if (pSoldier->bBleeding)
	{
		// heal self first!
		pSoldier->usActionData = pSoldier->sGridNo;
		if (bSlot != HANDPOS)
		{
			pSoldier->bSlotItemTakenFrom = bSlot;

			SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bSlot]) );
			/*
			TempObj = OBJECTTYPE{};
			// move the med kit out to temp obj
			SwapObjs( &TempObj, &(pSoldier->inv[bSlot]) );
			// swap the med kit with whatever was in the hand
			SwapObjs( &TempObj, &(pSoldier->inv[HANDPOS]) );
			// replace whatever was in the hand somewhere in inventory
			AutoPlaceObject( pSoldier, &TempObj, FALSE );
			*/
		}
		return( AI_ACTION_GIVE_AID );
	}

//	pSoldier->usActionData = FindClosestPatient( pSoldier );
	pSoldier->bAction = FindBestPatient( pSoldier, &fDoClimb );
	if (pSoldier->bAction != AI_ACTION_NONE)
	{
		pSoldier->usUIMovementMode = RUNNING;
		if (bSlot != HANDPOS)
		{
			pSoldier->bSlotItemTakenFrom = bSlot;

			SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bSlot]) );
		}
		return( pSoldier->bAction );
	}

	// do nothing
	return( AI_ACTION_NONE );
}
