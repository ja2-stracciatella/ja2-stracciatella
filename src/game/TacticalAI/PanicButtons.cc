#include "AIInternals.h"
#include "AI.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "PathAI.h"
#include "Items.h"
#include "World_Items.h"
#include "StrategicMap.h"
#include "Map_Screen_Interface_Map.h"
#include "Soldier_Profile.h"
#include "Quests.h"
#include "Queen_Command.h"

/*

Who can use the panic button?
 * in tixa only the warden
 * in other places only the army

*/

static UINT32 PercentEnemiesKilled()
{
	Assert(gTacticalStatus.Team[ ENEMY_TEAM ].bMenInSector >= 0);
	UINT32 totalEnemies = static_cast<UINT32>(gTacticalStatus.Team[ ENEMY_TEAM ].bMenInSector) + gTacticalStatus.ubArmyGuysKilled;
	if (totalEnemies == 0)
	{
		SLOGW("PercentEnemiesKilled was expecting the army in the current sector");
		return 0;
	}
	return 100 * static_cast<UINT32>(gTacticalStatus.ubArmyGuysKilled) / totalEnemies;
}

void MakeClosestEnemyChosenOne()
{
	INT16					sPathCost, sShortestPath = 1000;
	INT8					bPanicTrigger;
	INT16					sPanicTriggerGridNo;

	if ( ! (gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE) )
	{
		return;
	}

	if (!NeedToRadioAboutPanicTrigger() )
	{
		// no active panic triggers
		return;
	}

	// consider every enemy, looking for the closest capable, unbusy one
	SOLDIERTYPE* closest_enemy = NULL;
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pSoldier = *i;

		// if this merc is unconscious, or dead
		if (pSoldier->bLife < OKLIFE)
		{
			continue;  // next soldier
		}

		// if this guy's too tired to go
		if (pSoldier->bBreath < OKBREATH)
		{
			continue;  // next soldier
		}

		if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y )
		{
			if ( pSoldier->ubProfile != WARDEN )
			{
				continue; // in tixa only the warden
			}
		}
		else
		{
			if (pSoldier->bTeam != ENEMY_TEAM )
			{
				continue; // in other places only the army
			}
		}

		// if this guy is in battle with opponent(s)
		if (pSoldier->bOppCnt > 0)
		{
			continue;  // next soldier
		}

		// if this guy is still in serious shock
		if (pSoldier->bShock > 2)
		{
			continue;  // next soldier
		}

		if ( pSoldier->bLevel != 0 )
		{
			// screw having guys on the roof go for panic triggers!
			continue;  // next soldier
		}

		bPanicTrigger = ClosestPanicTrigger( pSoldier );
		if (bPanicTrigger == -1)
		{
			continue; // next soldier
		}

		sPanicTriggerGridNo = gTacticalStatus.sPanicTriggerGridNo[ bPanicTrigger ];
		if (sPanicTriggerGridNo == NOWHERE)
		{
			// this should never happen!
			continue;
		}

		// remember whether this guy had keys before
		//const INT8 bOldKeys = pSoldier->bHasKeys;

		// give him keys to see if with them he can get to the panic trigger
		pSoldier->bHasKeys = (pSoldier->bHasKeys << 1) | 1;

		// we now path directly to the panic trigger


			// if he can't get to a spot where he could get at the panic trigger
			/*
			if (FindAdjacentGridEx(pSoldier, gTacticalStatus.sPanicTriggerGridno, NULL, NULL, FALSE, FALSE) == -1)
			{
				pSoldier->bHasKeys = bOldKeys;
				continue;          // next merc
			}
			*/


		// ok, this enemy appears to be eligible

		// FindAdjacentGrid set HandGrid for us.  If we aren't at that spot already
		if (pSoldier->sGridNo != sPanicTriggerGridNo)
		{
			// get the AP cost for this enemy to go to target position
			sPathCost = PlotPath(pSoldier, sPanicTriggerGridNo, FALSE, FALSE, WALKING, 0);
		}
		else
		{
			sPathCost = 0;
		}

		// set his keys value back to what it was before this hack
		pSoldier->bHasKeys = (pSoldier->bHasKeys >> 1 );

		// if he can get there (or is already there!)
		if (sPathCost || (pSoldier->sGridNo == sPanicTriggerGridNo))
		{
			if (sPathCost < sShortestPath)
			{
				sShortestPath = sPathCost;
				closest_enemy = pSoldier;
			}
		}
	}

	// if we found have an eligible enemy, make him our "chosen one"
	if (closest_enemy != NULL)
	{
		gTacticalStatus.the_chosen_one = closest_enemy; // flag him as the chosen one
		if (closest_enemy->bAlertStatus < STATUS_RED)
		{
			closest_enemy->bAlertStatus = STATUS_RED;
			CheckForChangingOrders(closest_enemy);
		}
		SetNewSituation(closest_enemy); // set new situation for the chosen one
		closest_enemy->bHasKeys = (closest_enemy->bHasKeys << 1) | 1; // cheat and give him keys to every door
		//closest_enemy->bHasKeys = TRUE;
	}
}

void PossiblyMakeThisEnemyChosenOne( SOLDIERTYPE * pSoldier )
{
	INT32		iAPCost, iPathCost;
	//INT8		bOldKeys;
	INT8		bPanicTrigger;
	INT16		sPanicTriggerGridNo;
	UINT32	uiPercentEnemiesKilled;

	if ( ! (gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE) )
	{
		return;
	}

	if ( pSoldier->bLevel != 0 )
	{
		// screw having guys on the roof go for panic triggers!
		return;
	}


	bPanicTrigger = ClosestPanicTrigger( pSoldier );
	if (bPanicTrigger == -1)
	{
		return;
	}

	sPanicTriggerGridNo = gTacticalStatus.sPanicTriggerGridNo[ bPanicTrigger ];

	uiPercentEnemiesKilled = PercentEnemiesKilled();
	if ( gTacticalStatus.ubPanicTolerance[ bPanicTrigger ] > uiPercentEnemiesKilled )
	{
		// not yet... not yet
		return;
	}

	//bOldKeys = pSoldier->bHasKeys;
	pSoldier->bHasKeys = (pSoldier->bHasKeys << 1) | 1;

	// if he can't get to a spot where he could get at the panic trigger
	iAPCost = AP_PULL_TRIGGER;
	if (pSoldier->sGridNo != sPanicTriggerGridNo)
	{
		iPathCost = PlotPath(pSoldier, sPanicTriggerGridNo, FALSE, FALSE, RUNNING, 0);
		if (iPathCost == 0)
		{
			//pSoldier->bHasKeys = bOldKeys;
			pSoldier->bHasKeys = (pSoldier->bHasKeys >> 1);
			return;
		}
		iAPCost += iPathCost;

	}

	if ( iAPCost <= CalcActionPoints( pSoldier ) * 2)
	{
		// go!!!
		gTacticalStatus.the_chosen_one = pSoldier;
		return;
	}
	// else return keys to normal
	//pSoldier->bHasKeys = bOldKeys;
	pSoldier->bHasKeys = (pSoldier->bHasKeys >> 1);
}


INT8 PanicAI(SOLDIERTYPE *pSoldier, UINT8 ubCanMove)
{
	BOOLEAN		fFoundRoute = FALSE;
	INT8			bSlot;
	INT32			iPathCost;
	INT8			bPanicTrigger;
	INT16			sPanicTriggerGridNo;

	// if there are panic bombs here
	if (gTacticalStatus.fPanicFlags & PANIC_BOMBS_HERE)
	{
		// if enemy is holding a portable panic bomb detonator, he tries to use it
		bSlot = FindObj( pSoldier, REMOTEBOMBTRIGGER);
		if (bSlot != NO_SLOT)
		{
			//////////////////////////////////////////////////////////////////////
			// ACTIVATE DETONATOR: blow up sector's panic bombs
			//////////////////////////////////////////////////////////////////////

			// if we have enough APs to activate it now
			if (pSoldier->bActionPoints >= AP_USE_REMOTE)
			{
				SLOGD("%s is activating his detonator", pSoldier->name.c_str());
				// blow up all the PANIC bombs!
				return(AI_ACTION_USE_DETONATOR);
			}
			else     // otherwise, wait a turn
			{
				pSoldier->usActionData = NOWHERE;
				return(AI_ACTION_NONE);
			}
		}
	}

	// no panic bombs, or no portable detonator

	// if there's a panic trigger here (DOESN'T MATTER IF ANY PANIC BOMBS EXIST!)
	if ( gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE )
	{
		// Have WE been chosen to go after the trigger?
		if (pSoldier == gTacticalStatus.the_chosen_one)
		{
			bPanicTrigger = ClosestPanicTrigger( pSoldier );
			if (bPanicTrigger == -1)
			{
				// augh!
				return( -1 );
			}
			sPanicTriggerGridNo = gTacticalStatus.sPanicTriggerGridNo[ bPanicTrigger ];

			// if not standing on the panic trigger
			if (pSoldier->sGridNo != sPanicTriggerGridNo)
			{
				// determine whether we can still get there
				iPathCost = PlotPath(pSoldier, sPanicTriggerGridNo, FALSE, FALSE, RUNNING, 0);
				if (iPathCost != 0)
				{
					fFoundRoute = TRUE;
				}
			}
			else
			{
				fFoundRoute = TRUE;
			}

			// if we managed to find an adjacent spot
			if (fFoundRoute)
			{
				// if we are at that spot now
				if (pSoldier->sGridNo == sPanicTriggerGridNo)
				{
					////////////////////////////////////////////////////////////////
					// PULL THE PANIC TRIGGER!
					////////////////////////////////////////////////////////////////

					// and we have enough APs left to pull the trigger
					if (pSoldier->bActionPoints >= AP_PULL_TRIGGER)
					{
						// blow up the all the PANIC bombs (or just the journal)
						pSoldier->usActionData = sPanicTriggerGridNo;
						SLOGD("%s pulls panic trigger at grid %d",
									pSoldier->name.c_str(), pSoldier->usActionData);
						return(AI_ACTION_PULL_TRIGGER);
					}
					else       // otherwise, wait a turn
					{
						pSoldier->usActionData = NOWHERE;
						return(AI_ACTION_NONE);
					}
				}
				else           // we are NOT at the HandGrid spot
				{
					// if we can move at least 1 square's worth
					if (ubCanMove)
					{
						// if we can get to the HandGrid spot to yank the trigger
						// animations don't allow trigger-pulling from water, so we won't!
						if (LegalNPCDestination(pSoldier,sPanicTriggerGridNo,ENSURE_PATH,NOWATER,0))
						{
							pSoldier->usActionData = sPanicTriggerGridNo;
							pSoldier->bPathStored = TRUE;
							return(AI_ACTION_GET_CLOSER);
						}
						else       // Oh oh, the chosen one can't get to the trigger!
						{
							SLOGD("!legalDest - ChosenOne can't get to the trigger!");
							gTacticalStatus.the_chosen_one = NULL; // strip him of his Chosen One status
							MakeClosestEnemyChosenOne();     // and replace him!
						}
					}
					else         // can't move, wait 1 turn
					{
						pSoldier->usActionData = NOWHERE;
						return(AI_ACTION_NONE);
					}
				}
			}
			else     // Oh oh, the chosen one can't get to the trigger!
			{
				SLOGD("!adjacentFound - ChosenOne can't get to the trigger!");
				gTacticalStatus.the_chosen_one = NULL; // strip him of his Chosen One status
				MakeClosestEnemyChosenOne();   // and replace him!
			}
		}
	}

	// no action decided
	return(-1);
}

void InitPanicSystem( void )
{
	// start by assuming there is no panic bombs or triggers here
	gTacticalStatus.the_chosen_one = NULL;
	FindPanicBombsAndTriggers();
}

INT8 ClosestPanicTrigger( SOLDIERTYPE * pSoldier )
{
	INT8		bLoop;
	INT16		sDistance;
	INT16		sClosestDistance = 1000;
	INT8		bClosestTrigger = -1;
	UINT32	uiPercentEnemiesKilled;

	if (gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y)
	{
		if (pSoldier->ubProfile != WARDEN)
		{
			return -1; // in tixa only the warden
		}
	}
	else
	{
		if (pSoldier->bTeam != ENEMY_TEAM)
		{
			return -1; // in other places only the army
		}
	}

	uiPercentEnemiesKilled = PercentEnemiesKilled();

	for ( bLoop = 0; bLoop < NUM_PANIC_TRIGGERS; bLoop++ )
	{
		if ( gTacticalStatus.sPanicTriggerGridNo[ bLoop ] != NOWHERE )
		{

			if ( gTacticalStatus.ubPanicTolerance[ bLoop ] > uiPercentEnemiesKilled )
			{
				// not yet... not yet...
				continue; // next trigger
			}

			// in Tixa
			if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y )
			{
				// screen out the second/later panic trigger if the first one hasn't been triggered
				if ( bLoop > 0 && gTacticalStatus.sPanicTriggerGridNo[ bLoop - 1 ] != NOWHERE )
				{
					break;
				}
			}

			sDistance = PythSpacesAway( pSoldier->sGridNo, gTacticalStatus.sPanicTriggerGridNo[ bLoop ] );
			if (sDistance < sClosestDistance)
			{
				sClosestDistance = sDistance;
				bClosestTrigger = bLoop;
			}
		}
	}

	return( bClosestTrigger );
}

BOOLEAN NeedToRadioAboutPanicTrigger( void )
{
	UINT32		uiPercentEnemiesKilled;
	INT8			bLoop;

	if (!(gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE) ||
			gTacticalStatus.the_chosen_one != NULL)
	{
		// already done!
		return( FALSE );
	}

	if ( gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y )
	{
		const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(WARDEN);
		if (!pSoldier || pSoldier == gTacticalStatus.the_chosen_one)
		{
			return( FALSE ); // in tixa only the warden
		}
	}
	else if (!IsTeamActive(ENEMY_TEAM))
	{
		return FALSE; // in other places only the army
	}

	uiPercentEnemiesKilled = PercentEnemiesKilled();

	for ( bLoop = 0; bLoop < NUM_PANIC_TRIGGERS; bLoop++ )
	{
		// if the bomb exists and its tolerance has been exceeded
		if ( (gTacticalStatus.sPanicTriggerGridNo[ bLoop ] != NOWHERE) && ( uiPercentEnemiesKilled >= gTacticalStatus.ubPanicTolerance[ bLoop ] ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


#define STAIRCASE_GRIDNO 12067
#define STAIRCASE_DIRECTION 0

INT8 HeadForTheStairCase( SOLDIERTYPE * pSoldier )
{
	UNDERGROUND_SECTORINFO * pBasementInfo;

	pBasementInfo = FindUnderGroundSector( 3, MAP_ROW_P, 1 );
	if ( pBasementInfo && pBasementInfo->uiTimeCurrentSectorWasLastLoaded != 0 && ( pBasementInfo->ubNumElites + pBasementInfo->ubNumTroops + pBasementInfo->ubNumAdmins ) < 5 )
	{
		return( AI_ACTION_NONE );
	}

	if ( PythSpacesAway( pSoldier->sGridNo, STAIRCASE_GRIDNO ) < 2 )
	{
		return( AI_ACTION_TRAVERSE_DOWN );
	}
	else
	{
		if ( LegalNPCDestination( pSoldier, STAIRCASE_GRIDNO, ENSURE_PATH, WATEROK, 0 ) )
		{
			pSoldier->usActionData = STAIRCASE_GRIDNO;
			return( AI_ACTION_GET_CLOSER );
		}
	}
	return( AI_ACTION_NONE );
}

#define WARDEN_ALARM_GRIDNO 9376
#define WARDEN_GAS_GRIDNO 9216
// in both cases, direction 6
