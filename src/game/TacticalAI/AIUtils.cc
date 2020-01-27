#include "AI.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "Weapons.h"
#include "OppList.h"
#include "Points.h"
#include "PathAI.h"
#include "WorldMan.h"
#include "AIInternals.h"
#include "Items.h"
#include "LOS.h"
#include "Assignments.h"
#include "Soldier_Functions.h"
#include "Buildings.h"
#include "Soldier_Macros.h"
#include "Render_Fun.h"
#include "StrategicMap.h"
#include "Environment.h"
#include "Lighting.h"
#include "Soldier_Create.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"

//
// CJC's DG->JA2 conversion notes
//
// Commented out:
//
// InWaterOrGas - gas stuff
// RoamingRange - point patrol stuff

UINT8 Urgency[NUM_STATUS_STATES][NUM_MORALE_STATES] =
{
	{URGENCY_LOW,  URGENCY_LOW,  URGENCY_LOW,  URGENCY_LOW,  URGENCY_LOW}, // green
	{URGENCY_HIGH, URGENCY_MED,  URGENCY_MED,  URGENCY_LOW,  URGENCY_LOW}, // yellow
	{URGENCY_HIGH, URGENCY_MED,  URGENCY_MED,  URGENCY_MED,  URGENCY_MED}, // red
	{URGENCY_HIGH, URGENCY_HIGH, URGENCY_HIGH, URGENCY_MED,  URGENCY_MED}  // black
};

UINT16 MovementMode[LAST_MOVEMENT_ACTION + 1][NUM_URGENCY_STATES] =
{
	{WALKING,  WALKING,  WALKING }, // AI_ACTION_NONE

	{WALKING,  WALKING,  WALKING }, // AI_ACTION_RANDOM_PATROL
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_SEEK_FRIEND
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_SEEK_OPPONENT
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_TAKE_COVER
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_GET_CLOSER

	{WALKING,  WALKING,  WALKING }, // AI_ACTION_POINT_PATROL,
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_LEAVE_WATER_GAS,
	{WALKING,  SWATTING, RUNNING }, // AI_ACTION_SEEK_NOISE,
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_ESCORTED_MOVE,
	{WALKING,  RUNNING,  RUNNING }, // AI_ACTION_RUN_AWAY,

	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_KNIFE_MOVE
	{WALKING,  WALKING,  WALKING }, // AI_ACTION_APPROACH_MERC
	{RUNNING,  RUNNING,  RUNNING }, // AI_ACTION_TRACK
	{RUNNING,  RUNNING,  RUNNING },	// AI_ACTION_EAT
	{WALKING,  RUNNING,  SWATTING},	// AI_ACTION_PICKUP_ITEM

	{WALKING,  WALKING,  WALKING},	// AI_ACTION_SCHEDULE_MOVE
	{WALKING,  WALKING,  WALKING},	// AI_ACTION_WALK
	{RUNNING,  RUNNING,  RUNNING},	// AI_ACTION_RUN
	{WALKING,  RUNNING,  RUNNING},	// AI_ACTION_MOVE_TO_CLIMB //extrapolated

	{WALKING,  RUNNING,  RUNNING},	// AI_ACTION_CHANGE_FACING //extrapolated
	{WALKING,  RUNNING,  RUNNING},	// AI_ACTION_CHANGE_STANCE //extrapolated

};

INT8 OKToAttack(const SOLDIERTYPE * pSoldier, int target)
{
	// can't shoot yourself
	if (target == pSoldier->sGridNo)
		return(NOSHOOT_MYSELF);

	if (WaterTooDeepForAttacks(pSoldier->sGridNo))
		return(NOSHOOT_WATER);

	// make sure a weapon is in hand (FEB.8 ADDITION: tossable items are also OK)
	if (!WeaponInHand(pSoldier))
	{
		return(NOSHOOT_NOWEAPON);
	}

	// JUST PUT THIS IN ON JULY 13 TO TRY AND FIX OUT-OF-AMMO SITUATIONS

	if ( GCM->getItem(pSoldier->inv[HANDPOS].usItem)->getItemClass() == IC_GUN)
	{
		if ( pSoldier->inv[HANDPOS].usItem == TANK_CANNON )
		{
			// look for another tank shell ELSEWHERE IN INVENTORY
			if ( FindLaunchable( pSoldier, TANK_CANNON ) == NO_SLOT )
			{
				return(NOSHOOT_NOLOAD);
			}
		}
		else if (pSoldier->inv[HANDPOS].ubGunShotsLeft == 0)
		{
			return(NOSHOOT_NOAMMO);
		}
	}
	else if (GCM->getItem(pSoldier->inv[HANDPOS].usItem)->getItemClass() == IC_LAUNCHER)
	{
		if ( FindLaunchable( pSoldier, pSoldier->inv[HANDPOS].usItem ) == NO_SLOT )
		{
			return(NOSHOOT_NOLOAD);
		}
	}

	return(TRUE);
}


static BOOLEAN ConsiderProne(SOLDIERTYPE* pSoldier)
{
	INT16 sOpponentGridNo;
	INT8  bOpponentLevel;
	INT32 iRange;

	if (pSoldier->bAIMorale >= MORALE_NORMAL)
	{
		return( FALSE );
	}
	// We don't want to go prone if there is a nearby enemy
	ClosestKnownOpponent( pSoldier, &sOpponentGridNo, &bOpponentLevel );
	iRange = GetRangeFromGridNoDiff( pSoldier->sGridNo, sOpponentGridNo );
	if (iRange > 10)
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

UINT8 StanceChange( SOLDIERTYPE * pSoldier, UINT8 ubAttackAPCost )
{
	// consider crouching or going prone

	if (PTR_STANDING)
	{
		if (pSoldier->bActionPoints - ubAttackAPCost >= AP_CROUCH)
		{
			if ( (pSoldier->bActionPoints - ubAttackAPCost >= AP_CROUCH + AP_PRONE) && IsValidStance( pSoldier, ANIM_PRONE ) && ConsiderProne( pSoldier ) )
			{
				return( ANIM_PRONE );
			}
			else if ( IsValidStance( pSoldier, ANIM_CROUCH ) )
			{
				return( ANIM_CROUCH );
			}
		}
	}
	else if (PTR_CROUCHED)
	{
		if ( (pSoldier->bActionPoints - ubAttackAPCost >= AP_PRONE) && IsValidStance( pSoldier, ANIM_PRONE ) && ConsiderProne( pSoldier ) )
		{
			return( ANIM_PRONE );
		}
	}
	return( 0 );
}

UINT8 ShootingStanceChange( SOLDIERTYPE * pSoldier, ATTACKTYPE * pAttack, INT8 bDesiredDirection )
{
	// Figure out the best stance for this attack

	// We don't want to go through a lot of complex calculations here,
	// just compare the chance of the bullet hitting if we are
	// standing, crouched, or prone

	UINT16 usRealAnimState, usBestAnimState;
	INT8   bBestStanceDiff=-1;
	INT8   bLoop, bStanceNum, bStanceDiff, bAPsAfterAttack;
	UINT32 uiChanceOfDamage, uiBestChanceOfDamage, uiCurrChanceOfDamage;
	UINT32 uiStanceBonus, uiMinimumStanceBonusPerChange = 20 - 3 * pAttack->ubAimTime;
	INT32  iRange;

	bStanceNum = 0;
	uiCurrChanceOfDamage = 0;

	bAPsAfterAttack = pSoldier->bActionPoints - pAttack->ubAPCost - GetAPsToReadyWeapon( pSoldier, pSoldier->usAnimState );
	if (bAPsAfterAttack < AP_CROUCH)
	{
		return( 0 );
	}
	// Unfortunately, to get this to work, we have to fake the AI guy's
	// animation state so we get the right height values
	usRealAnimState = pSoldier->usAnimState;
	usBestAnimState = pSoldier->usAnimState;
	uiBestChanceOfDamage = 0;
	iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pAttack->sTarget );

	switch( gAnimControl[usRealAnimState].ubEndHeight )
	{
		// set a stance number comparable with our loop variable so we can easily compute
		// stance differences and thus AP cost
		case ANIM_STAND:
			bStanceNum = 0;
			break;
		case ANIM_CROUCH:
			bStanceNum = 1;
			break;
		case ANIM_PRONE:
			bStanceNum = 2;
			break;
	}
	for (bLoop = 0; bLoop < 3; bLoop++)
	{
		bStanceDiff = ABS( bLoop - bStanceNum );
		if (bStanceDiff == 2 && bAPsAfterAttack < AP_CROUCH + AP_PRONE)
		{
			// can't consider this!
			continue;
		}

		switch( bLoop )
		{
			case 0:
				if ( !InternalIsValidStance( pSoldier, bDesiredDirection, ANIM_STAND ) )
				{
					continue;
				}
				pSoldier->usAnimState = STANDING;
				break;
			case 1:
				if ( !InternalIsValidStance( pSoldier, bDesiredDirection, ANIM_CROUCH ) )
				{
					continue;
				}
				pSoldier->usAnimState = CROUCHING;
				break;
			default:
				if ( !InternalIsValidStance( pSoldier, bDesiredDirection, ANIM_PRONE ) )
				{
					continue;
				}
				pSoldier->usAnimState = PRONE;
				break;
		}

		uiChanceOfDamage = SoldierToLocationChanceToGetThrough(pSoldier, pAttack->sTarget, pSoldier->bTargetLevel, pSoldier->bTargetCubeLevel, pAttack->opponent) * CalcChanceToHitGun(pSoldier, pAttack->sTarget, pAttack->ubAimTime, AIM_SHOT_TORSO, false) / 100;
		if (uiChanceOfDamage > 0)
		{
			uiStanceBonus = 0;
			// artificially augment "chance of damage" to reflect penalty to be shot at various stances
			switch( pSoldier->usAnimState )
			{
				case CROUCHING:
					if (iRange > POINT_BLANK_RANGE + 10 * (AIM_PENALTY_TARGET_CROUCHED / 3))
					{
						uiStanceBonus = AIM_BONUS_CROUCHING;
					}
					else if (iRange > POINT_BLANK_RANGE)
					{
						// reduce chance to hit with distance to the prone/immersed target
						uiStanceBonus = 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE); // penalty -3%/tile
					}
					break;
				case PRONE:
					if (iRange <= MIN_PRONE_RANGE)
					{
						// HATE being prone this close!
						uiChanceOfDamage = 0;
					}
					else //if (iRange > POINT_BLANK_RANGE)
					{
						// reduce chance to hit with distance to the prone/immersed target
						uiStanceBonus = 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE); // penalty -3%/tile
					}
					break;
				default:
					break;
			}
			// reduce stance bonus according to how much we have to change stance to get there
			//uiStanceBonus = uiStanceBonus * (4 - bStanceDiff) / 4;
			uiChanceOfDamage += uiStanceBonus;
		}

		if (bStanceDiff == 0)
		{
			uiCurrChanceOfDamage = uiChanceOfDamage;
		}
		if (uiChanceOfDamage > uiBestChanceOfDamage )
		{
			uiBestChanceOfDamage = uiChanceOfDamage;
			usBestAnimState = pSoldier->usAnimState;
			bBestStanceDiff = bStanceDiff;
		}
	}

	pSoldier->usAnimState = usRealAnimState;

	// return 0 or the best height value to be at
	if (bBestStanceDiff == 0 || ((uiBestChanceOfDamage - uiCurrChanceOfDamage) / bBestStanceDiff) < uiMinimumStanceBonusPerChange)
	{
		// better off not changing our stance!
		return( 0 );
	}
	else
	{
		return( gAnimControl[ usBestAnimState ].ubEndHeight );
	}
}


UINT16 DetermineMovementMode( SOLDIERTYPE * pSoldier, INT8 bAction )
{
	if ( pSoldier->fUIMovementFast )
	{
		return( RUNNING );
	}
	else if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		if (pSoldier->bAlertStatus == STATUS_GREEN)
		{
			return( WALKING );
		}
		else
		{
			return( RUNNING );
		}
	}
	else if (pSoldier->ubBodyType == COW || pSoldier->ubBodyType == CROW)
	{
		return( WALKING );
	}
	else
	{
		if ( (pSoldier->fAIFlags & AI_CAUTIOUS) && (MovementMode[bAction][Urgency[pSoldier->bAlertStatus][pSoldier->bAIMorale]] == RUNNING) )
		{
			return( WALKING );
		}
		else if ( bAction == AI_ACTION_SEEK_NOISE && pSoldier->bTeam == CIV_TEAM && !IS_MERC_BODY_TYPE( pSoldier ) )
		{
			return( WALKING );
		}
		else if ( (pSoldier->ubBodyType == HATKIDCIV || pSoldier->ubBodyType == KIDCIV) && (pSoldier->bAlertStatus == STATUS_GREEN) && Random( 10 ) == 0 )
		{
			return( KID_SKIPPING );
		}
		else
		{
			return( MovementMode[bAction][Urgency[pSoldier->bAlertStatus][pSoldier->bAIMorale]] );
		}
	}
}

void NewDest(SOLDIERTYPE *pSoldier, UINT16 usGridNo)
{
	// ATE: Setting sDestination? Tis does not make sense...
	//pSoldier->sDestination = usGridNo;
	BOOLEAN fSet = FALSE;

	if ( IS_MERC_BODY_TYPE( pSoldier ) && pSoldier->bAction == AI_ACTION_TAKE_COVER && (pSoldier->bOrders == DEFENSIVE || pSoldier->bOrders == CUNNINGSOLO || pSoldier->bOrders == CUNNINGAID ) && (SoldierDifficultyLevel( pSoldier ) >= 2) )
	{
		UINT16 usMovementMode;

		// getting real movement anim for someone who is going to take cover, not just considering
		usMovementMode = MovementMode[AI_ACTION_TAKE_COVER][Urgency[pSoldier->bAlertStatus][pSoldier->bAIMorale]];
		if ( usMovementMode != SWATTING )
		{
			// really want to look at path, see how far we could get on path while swatting
			if ( EnoughPoints( pSoldier, RecalculatePathCost( pSoldier, SWATTING ), 0, FALSE ) || (pSoldier->bLastAction == AI_ACTION_TAKE_COVER && pSoldier->usUIMovementMode == SWATTING ) )
			{
				pSoldier->usUIMovementMode = SWATTING;
			}
			else
			{
				pSoldier->usUIMovementMode = usMovementMode;
			}
		}
		else
		{
			pSoldier->usUIMovementMode = usMovementMode;
		}
		fSet = TRUE;
	}
	else
	{
		if ( pSoldier->bTeam == ENEMY_TEAM && pSoldier->bAlertStatus == STATUS_RED )
		{
			// switch( pSoldier->bAction )
			{
				/*
				case AI_ACTION_MOVE_TO_CLIMB:
				case AI_ACTION_RUN_AWAY:
					pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->bAction );
					fSet = TRUE;
					break;*/
				// default:
					if ( PreRandom( 5 - SoldierDifficultyLevel( pSoldier ) ) == 0 )
					{
						INT16 sClosestNoise = (INT16) MostImportantNoiseHeard( pSoldier, NULL, NULL, NULL );
						if ( sClosestNoise != NOWHERE && PythSpacesAway( pSoldier->sGridNo, sClosestNoise ) < MaxDistanceVisible() + 10 )
						{
							pSoldier->usUIMovementMode = SWATTING;
							fSet = TRUE;
						}
					}
					if ( !fSet )
					{
						pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->bAction );
						fSet = TRUE;
					}
					// break;
			}

		}
		else
		{
			pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, pSoldier->bAction );
			fSet = TRUE;
		}

		if ( pSoldier->usUIMovementMode == SWATTING && !IS_MERC_BODY_TYPE( pSoldier ) )
		{
			pSoldier->usUIMovementMode = WALKING;
		}
	}

	//EVENT_GetNewSoldierPath( pSoldier, pSoldier->sDestination, pSoldier->usUIMovementMode );
	// ATE: Using this more versitile version
	// Last paramater says whether to re-start the soldier's animation
	// This should be done if buddy was paused for fNoApstofinishMove...
	EVENT_InternalGetNewSoldierPath( pSoldier, usGridNo, pSoldier->usUIMovementMode , FALSE, pSoldier->fNoAPToFinishMove );

}


BOOLEAN IsActionAffordable(SOLDIERTYPE *pSoldier)
{
	INT8	bMinPointsNeeded = 0;

	switch (pSoldier->bAction)
	{
		case AI_ACTION_NONE:                  // maintain current position & facing
			// no cost for doing nothing!
			break;

		case AI_ACTION_CHANGE_FACING:         // turn to face another direction
			bMinPointsNeeded = (INT8) GetAPsToLook( pSoldier );
			break;

		case AI_ACTION_RANDOM_PATROL:         // move towards a particular location
		case AI_ACTION_SEEK_FRIEND:           // move towards friend in trouble
		case AI_ACTION_SEEK_OPPONENT:         // move towards a reported opponent
		case AI_ACTION_TAKE_COVER:            // run for nearest cover from threat
		case AI_ACTION_GET_CLOSER:            // move closer to a strategic location
		case AI_ACTION_POINT_PATROL:          // move towards next patrol point
		case AI_ACTION_LEAVE_WATER_GAS:       // seek nearest spot of ungassed land
		case AI_ACTION_SEEK_NOISE:            // seek most important noise heard
		case AI_ACTION_ESCORTED_MOVE:         // go where told to by escortPlayer
		case AI_ACTION_RUN_AWAY:              // run away from nearby opponent(s)
		case AI_ACTION_APPROACH_MERC:
		case AI_ACTION_TRACK:
		case AI_ACTION_EAT:
		case AI_ACTION_SCHEDULE_MOVE:
		case AI_ACTION_WALK:
		case AI_ACTION_MOVE_TO_CLIMB:
			// for movement, must have enough APs to move at least 1 tile's worth
			bMinPointsNeeded = MinPtsToMove(pSoldier);
			break;

		case AI_ACTION_PICKUP_ITEM:           // grab things lying on the ground
			bMinPointsNeeded = __max( MinPtsToMove( pSoldier ), AP_PICKUP_ITEM );
			break;

		case AI_ACTION_OPEN_OR_CLOSE_DOOR:
		case AI_ACTION_UNLOCK_DOOR:
		case AI_ACTION_LOCK_DOOR:
			bMinPointsNeeded = MinPtsToMove(pSoldier);
			break;

		case AI_ACTION_DROP_ITEM:
			bMinPointsNeeded = AP_PICKUP_ITEM;
			break;

		case AI_ACTION_FIRE_GUN:              // shoot at nearby opponent
		case AI_ACTION_TOSS_PROJECTILE:       // throw grenade at/near opponent(s)
		case AI_ACTION_KNIFE_MOVE:            // preparing to stab adjacent opponent
		case AI_ACTION_THROW_KNIFE:
			// only FIRE_GUN currently actually pays extra turning costs!
			bMinPointsNeeded = MinAPsToAttack(pSoldier,pSoldier->usActionData,ADDTURNCOST);
			break;

		case AI_ACTION_PULL_TRIGGER:          // activate an adjacent panic trigger
			bMinPointsNeeded = AP_PULL_TRIGGER;
			break;

		case AI_ACTION_USE_DETONATOR:         // grab detonator and set off bomb(s)
			bMinPointsNeeded = AP_USE_REMOTE;
			break;

		case AI_ACTION_YELLOW_ALERT:          // tell friends opponent(s) heard
		case AI_ACTION_RED_ALERT:             // tell friends opponent(s) seen
		case AI_ACTION_CREATURE_CALL:         // for now
			bMinPointsNeeded = AP_RADIO;
			break;

		case AI_ACTION_CHANGE_STANCE:                // crouch
			bMinPointsNeeded = AP_CROUCH;
			break;

		case AI_ACTION_GIVE_AID:              // help injured/dying friend
			bMinPointsNeeded = 0;
			break;

		case AI_ACTION_CLIMB_ROOF:
			if (pSoldier->bLevel == 0)
			{
				bMinPointsNeeded = AP_CLIMBROOF;
			}
			else
			{
				bMinPointsNeeded = AP_CLIMBOFFROOF;
			}
			break;

		case AI_ACTION_COWER:
		case AI_ACTION_STOP_COWERING:
		case AI_ACTION_LOWER_GUN:
		case AI_ACTION_END_COWER_AND_MOVE:
		case AI_ACTION_TRAVERSE_DOWN:
		case AI_ACTION_OFFER_SURRENDER:
			bMinPointsNeeded = 0;
			break;

		default:
			break;
	}

	// check whether or not we can afford to do this action
	if (bMinPointsNeeded > pSoldier->bActionPoints)
	{
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}


INT16 RandomFriendWithin(SOLDIERTYPE* const s)
{
	// obtain maximum roaming distance from soldier's origin
	INT16 usOrigin;
	const UINT16 usMaxDist = RoamingRange(s, &usOrigin);

	// make sure origin is a legal gridno!
	if (usOrigin >= GRIDSIZE)
	{
		return FALSE;
	}

	// build a list of the guynums of all active, eligible friendly mercs

	// go through each soldier, looking for "friends" (soldiers on same side)
	UINT8 ubFriendCount = 0;
	const SOLDIERTYPE* friends[MAXMERCS];
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const candidate = *i;
		if (candidate == s) continue; // skip ourselves

		/* if this man not neutral, but is on my side, OR if he is neutral, but so
		 * am I, then he's a "friend" for the purposes of random visitations */
		if ((candidate->bNeutral ? s->bNeutral : s->bSide == candidate->bSide) &&
				SpacesAway(s->sGridNo, candidate->sGridNo) > 1) // if we're not already neighbors
		{
			// remember his guynum, increment friend counter
			friends[ubFriendCount++] = candidate;
		}
	}

	while (ubFriendCount != 0)
	{
		// randomly select one of the remaining friends in the list
		const UINT               chosen_idx = PreRandom(ubFriendCount);
		const SOLDIERTYPE* const chosen     = friends[chosen_idx];

		/* if our movement range is NOT restricted, or this friend's within range
		 * use distance - 1, because there must be at least 1 tile 1 space closer */
		if (SpacesAway(usOrigin, chosen->sGridNo) - 1 <= usMaxDist)
		{
			// should be close enough, try to find a legal ->sDestination within 1 tile

			BOOLEAN fDirChecked[8];
			// clear dirChecked flag for all 8 directions
			for (UINT16 usDirection = 0; usDirection < 8; ++usDirection)
			{
				fDirChecked[usDirection] = FALSE;
			}

			// examine all 8 spots around friend
			// keep looking while directions remain and a satisfactory one not found
			for (UINT8 ubDirsLeft = 8; ubDirsLeft--;)
			{
				// randomly select a direction which hasn't been 'checked' yet
				UINT16 usDirection;
				do
				{
					usDirection = Random(8);
				}
				while (fDirChecked[usDirection]);

				fDirChecked[usDirection] = TRUE;

				// determine the gridno 1 tile away from current friend in this direction
				const UINT16 usDest = NewGridNo(chosen->sGridNo, DirectionInc( usDirection + 1 ));

				// if that's out of bounds, ignore it & check next direction
				if (usDest == chosen->sGridNo) continue;

				// if our movement range is NOT restricted
				if (SpacesAway(usOrigin,usDest) <= usMaxDist &&
						LegalNPCDestination(s, usDest, ENSURE_PATH, NOWATER, 0))
				{
					s->usActionData = usDest; // store this ->sDestination
					s->bPathStored  = TRUE;   // optimization - Ian
					return TRUE;
				}
			}
		}

		friends[chosen_idx] = friends[--ubFriendCount];
	}

	return FALSE;
}


INT16 RandDestWithinRange(SOLDIERTYPE *pSoldier)
{
	INT16 sRandDest = NOWHERE;
	UINT8	ubTriesLeft;
	BOOLEAN fLimited = FALSE, fFound = FALSE;
	INT16 sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXRange, sYRange, sXOffset, sYOffset;
	INT16 sOrigX, sOrigY;
	INT16 sX, sY;

	sOrigX = sOrigY = -1;
	sMaxLeft = sMaxRight = sMaxUp = sMaxDown = sXRange = sYRange = -1;

	// Try to find a random ->sDestination that's no more than maxDist away from
	// the given gridno of origin

	if (gfTurnBasedAI)
	{
		ubTriesLeft = 10;
	}
	else
	{
		ubTriesLeft = 1;
	}

	INT16  usOrigin;
	UINT16 usMaxDist = RoamingRange(pSoldier,&usOrigin);

	UINT8 room = NO_ROOM;
	if ( pSoldier->bOrders <= CLOSEPATROL && (pSoldier->bTeam == CIV_TEAM || pSoldier->ubProfile != NO_PROFILE ) )
	{
		// any other combo uses the default of room == NO_ROOM, set above
		room = GetRoom(pSoldier->usPatrolGrid[0]);
	}

	// if the maxDist is truly a restriction
	if (usMaxDist < (MAXCOL - 1))
	{
		fLimited = TRUE;

		// determine maximum horizontal limits
		sOrigX = usOrigin % MAXCOL;
		sOrigY = usOrigin / MAXCOL;

		sMaxLeft  = MIN(usMaxDist, sOrigX);
		sMaxRight = MIN(usMaxDist,MAXCOL - (sOrigX + 1));

		// determine maximum vertical limits
		sMaxUp   = MIN(usMaxDist, sOrigY);
		sMaxDown = MIN(usMaxDist,MAXROW - (sOrigY + 1));

		sXRange = sMaxLeft + sMaxRight + 1;
		sYRange = sMaxUp + sMaxDown + 1;
	}

	if (pSoldier->ubBodyType == LARVAE_MONSTER)
	{
		// only crawl 1 tile, within our roaming range
		while ((ubTriesLeft--) && !fFound)
		{
			sXOffset = (INT16) Random( 3 ) - 1; // generates -1 to +1
			sYOffset = (INT16) Random( 3 ) - 1;

			if (fLimited)
			{
				sX = pSoldier->sGridNo % MAXCOL + sXOffset;
				sY = pSoldier->sGridNo / MAXCOL + sYOffset;
				if (sX < sOrigX - sMaxLeft || sX > sOrigX + sMaxRight)
				{
					continue;
				}
				if (sY < sOrigY - sMaxUp || sY > sOrigY + sMaxDown)
				{
					continue;
				}
				sRandDest = usOrigin + sXOffset + (MAXCOL * sYOffset);
			}
			else
			{
				sRandDest = usOrigin + sXOffset + (MAXCOL * sYOffset);
			}

			if (!LegalNPCDestination(pSoldier,sRandDest,ENSURE_PATH,NOWATER,0))
			{
				sRandDest = NOWHERE;
				continue;                   // try again!
			}

			// passed all the tests, ->sDestination is acceptable
			fFound = TRUE;
			pSoldier->bPathStored = TRUE;	// optimization - Ian
		}
	}
	else
	{
		// keep rolling random ->sDestinations until one's satisfactory or retries used
		while ((ubTriesLeft--) && !fFound)
		{
			if (fLimited)
			{
				sXOffset = ((INT16)Random(sXRange)) - sMaxLeft;
				sYOffset = ((INT16)Random(sYRange)) - sMaxUp;

				sRandDest = usOrigin + sXOffset + (MAXCOL * sYOffset);
			}
			else
			{
				sRandDest = (INT16) PreRandom(GRIDSIZE);
			}

			if (room != NO_ROOM)
			{
				UINT8 const temp_room = GetRoom(sRandDest);
				if (temp_room != NO_ROOM && temp_room != room)
				{
					// outside of room available for patrol!
					sRandDest = NOWHERE;
					continue;
				}
			}

			if (!LegalNPCDestination(pSoldier,sRandDest,ENSURE_PATH,NOWATER,0))
			{
				sRandDest = NOWHERE;
				continue;                   // try again!
			}

			// passed all the tests, ->sDestination is acceptable
			fFound = TRUE;
			pSoldier->bPathStored = TRUE;	// optimization - Ian
		}
	}

	return(sRandDest); // defaults to NOWHERE
}

INT16 ClosestReachableDisturbance(SOLDIERTYPE *pSoldier, UINT8 ubUnconsciousOK, BOOLEAN * pfChangeLevel )
{
	INT16   *psLastLoc, *pusNoiseGridNo;
	INT8    *pbLastLevel;
	INT16   sGridNo=-1;
	INT8    bLevel;
	BOOLEAN fClimbingNecessary, fClosestClimbingNecessary = FALSE;
	INT32   iPathCost;
	INT16   sClosestDisturbance = NOWHERE;
	INT8    *pbNoiseLevel;
	INT8    *pbPersOL,*pbPublOL;
	INT16   sClimbGridNo;

	// CJC: can't trace a path to every known disturbance!
	// for starters, try the closest one as the crow flies
	INT16   sClosestEnemy = NOWHERE, sDistToClosestEnemy = 1000, sDistToEnemy;

	*pfChangeLevel = FALSE;

	pusNoiseGridNo = &gsPublicNoiseGridno[pSoldier->bTeam];
	pbNoiseLevel = &gbPublicNoiseLevel[pSoldier->bTeam];

	// hang pointers at start of this guy's personal and public opponent opplists
	//pbPersOL = &pSoldier->bOppList[0];
	//pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][0]);
	//psLastLoc = &(gsLastKnownOppLoc[pSoldier->ubID][0]);

	// look through this man's personal & public opplists for opponents known
	INT8 bClosestLevel = 0; // XXX HACK000E
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const pOpp = *i;

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpp ) || (pSoldier->bSide == pOpp->bSide) )
		{
			continue;          // next merc
		}

		pbPersOL = pSoldier->bOppList + pOpp->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpp->ubID;
		psLastLoc = gsLastKnownOppLoc[pSoldier->ubID] + pOpp->ubID;
		pbLastLevel = gbLastKnownOppLevel[pSoldier->ubID] + pOpp->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;          // next merc
		}

		// this is possible if get here from BLACK AI in one of those rare
		// instances when we couldn't get a meaningful shot off at a guy in sight
		if ((*pbPersOL == SEEN_CURRENTLY) && (pOpp->bLife >= OKLIFE))
		{
			// don't allow this to return any valid values, this guy remains a
			// serious threat and the last thing we want to do is approach him!
			return(NOWHERE);
		}

		// if personal knowledge is more up to date or at least equal
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL))
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sGridNo = *psLastLoc;
			bLevel = *pbLastLevel;
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sGridNo = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpp->ubID];
			bLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpp->ubID];
		}

		// if we are standing at that gridno (!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;          // next merc
		}

		if (sGridNo == NOWHERE)
		{
			// huh?
			continue;
		}

		sDistToEnemy = PythSpacesAway( pSoldier->sGridNo, sGridNo );
		if (sDistToEnemy < sDistToClosestEnemy )
		{
			sClosestEnemy = sGridNo;
			bClosestLevel = bLevel;
			sDistToClosestEnemy = sDistToEnemy;
		}

	}

	if (sClosestEnemy != NOWHERE)
	{
		iPathCost = EstimatePathCostToLocation( pSoldier, sClosestEnemy, bClosestLevel, FALSE, &fClimbingNecessary, &sClimbGridNo );
		// if we can get there
		if (iPathCost != 0)
		{
			if (fClimbingNecessary)
			{
				sClosestDisturbance = sClimbGridNo;
			}
			else
			{
				sClosestDisturbance = sClosestEnemy;
			}
			fClosestClimbingNecessary = fClimbingNecessary;
		}
	}

	// if any "misc. noise" was also heard recently
	if (pSoldier->sNoiseGridno != NOWHERE && pSoldier->sNoiseGridno != sClosestDisturbance)
	{
		// test this gridno, too
		sGridNo = pSoldier->sNoiseGridno;
		bLevel = pSoldier->bNoiseLevel;

		// if we are there (at the noise gridno)
		if (sGridNo == pSoldier->sGridNo)
		{
			pSoldier->sNoiseGridno = NOWHERE;        // wipe it out, not useful anymore
			pSoldier->ubNoiseVolume = 0;
		}
		else
		{
			// get the AP cost to get to the location of the noise
			iPathCost = EstimatePathCostToLocation( pSoldier, sGridNo, bLevel, FALSE, &fClimbingNecessary, &sClimbGridNo );
			// if we can get there
			if (iPathCost != 0)
			{
				if (fClimbingNecessary)
				{
					sClosestDisturbance = sClimbGridNo;
				}
				else
				{
					sClosestDisturbance = sGridNo;
				}
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
	}


	// if any PUBLIC "misc. noise" was also heard recently
	if (*pusNoiseGridNo != NOWHERE && *pusNoiseGridNo != sClosestDisturbance )
	{
		// test this gridno, too
		sGridNo = *pusNoiseGridNo;
		bLevel = *pbNoiseLevel;

		// if we are not NEAR the noise gridno...
		if ( pSoldier->bLevel != bLevel || PythSpacesAway( pSoldier->sGridNo, sGridNo ) >= 6 || SoldierTo3DLocationLineOfSightTest( pSoldier, sGridNo, bLevel, 0, (UINT8) MaxDistanceVisible(), FALSE ) == 0 )
		// if we are NOT there (at the noise gridno)
		//	if (sGridNo != pSoldier->sGridNo)
		{
			// get the AP cost to get to the location of the noise
			iPathCost = EstimatePathCostToLocation( pSoldier, sGridNo, bLevel, FALSE, &fClimbingNecessary, &sClimbGridNo );
			// if we can get there
			if (iPathCost != 0)
			{
				if (fClimbingNecessary)
				{
					sClosestDisturbance = sClimbGridNo;
				}
				else
				{
					sClosestDisturbance = sGridNo;
				}
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
		else
		{
			// degrade our public noise a bit
			*pusNoiseGridNo -= 2;
		}
	}

	*pfChangeLevel = fClosestClimbingNecessary;
	return(sClosestDisturbance);
}


INT16 ClosestKnownOpponent(SOLDIERTYPE *pSoldier, INT16 * psGridNo, INT8 * pbLevel)
{
	INT16 sGridNo, sClosestOpponent = NOWHERE;
	INT32 iRange, iClosestRange = 1500;
	INT8  *pbPersOL, *pbPublOL;
	INT8  bLevel, bClosestLevel;

	bClosestLevel = -1;


	// NOTE: THIS FUNCTION ALLOWS RETURN OF UNCONSCIOUS AND UNREACHABLE OPPONENTS

	// hang pointers at start of this guy's personal and public opponent opplists
	pbPersOL = &pSoldier->bOppList[0];
	pbPublOL = &(gbPublicOpplist[pSoldier->bTeam][0]);

	// look through this man's personal & public opplists for opponents known
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const pOpp = *i;

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpp ) || (pSoldier->bSide == pOpp->bSide))
		{
			continue;          // next merc
		}

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->bAttitude == ATTACKSLAYONLY && pOpp->ubProfile != SLAY)
		{
			continue;  // next opponent
		}

		pbPersOL = pSoldier->bOppList + pOpp->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpp->ubID;

		// if this opponent is unknown personally and publicly
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			continue;          // next merc
		}

		// if personal knowledge is more up to date or at least equal
		if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) ||
			(*pbPersOL == *pbPublOL))
		{
			// using personal knowledge, obtain opponent's "best guess" gridno
			sGridNo = gsLastKnownOppLoc[pSoldier->ubID][pOpp->ubID];
			bLevel = gbLastKnownOppLevel[pSoldier->ubID][pOpp->ubID];
		}
		else
		{
			// using public knowledge, obtain opponent's "best guess" gridno
			sGridNo = gsPublicLastKnownOppLoc[pSoldier->bTeam][pOpp->ubID];
			bLevel = gbPublicLastKnownOppLevel[pSoldier->bTeam][pOpp->ubID];
		}

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;          // next merc
		}

		// this function is used only for turning towards closest opponent or changing stance
		// as such, if they AI is in a building,
		// we should ignore people who are on the roof of the same building as the AI
		if ( (bLevel != pSoldier->bLevel) && SameBuilding( pSoldier->sGridNo, sGridNo ) )
		{
			continue;
		}

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if (iRange < iClosestRange)
		{
			iClosestRange = iRange;
			sClosestOpponent = sGridNo;
			bClosestLevel = bLevel;
		}
	}
	if (psGridNo)
	{
		*psGridNo = sClosestOpponent;
	}
	if (pbLevel)
	{
		*pbLevel = bClosestLevel;
	}
	return( sClosestOpponent );
}

INT16 ClosestSeenOpponent(SOLDIERTYPE *pSoldier, INT16 * psGridNo, INT8 * pbLevel)
{
	INT16 sGridNo, sClosestOpponent = NOWHERE;
	INT32 iRange, iClosestRange = 1500;
	INT8  *pbPersOL;
	INT8  bLevel, bClosestLevel;

	bClosestLevel = -1;

	// look through this man's personal & public opplists for opponents known
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const pOpp = *i;

		// if this merc is neutral/on same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpp ) || (pSoldier->bSide == pOpp->bSide))
		{
			continue;          // next merc
		}

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->bAttitude == ATTACKSLAYONLY && pOpp->ubProfile != SLAY)
		{
			continue;  // next opponent
		}

		pbPersOL = pSoldier->bOppList + pOpp->ubID;

		// if this opponent is not seen personally
		if (*pbPersOL != SEEN_CURRENTLY)
		{
			continue;          // next merc
		}

		// since we're dealing with seen people, use exact gridnos
		sGridNo = pOpp->sGridNo;
		bLevel = pOpp->bLevel;

		// if we are standing at that gridno(!, obviously our info is old...)
		if (sGridNo == pSoldier->sGridNo)
		{
			continue;          // next merc
		}

		// this function is used only for turning towards closest opponent or changing stance
		// as such, if they AI is in a building,
		// we should ignore people who are on the roof of the same building as the AI
		if ( (bLevel != pSoldier->bLevel) && SameBuilding( pSoldier->sGridNo, sGridNo ) )
		{
			continue;
		}

		// I hope this will be good enough; otherwise we need a fractional/world-units-based 2D distance function
		//sRange = PythSpacesAway( pSoldier->sGridNo, sGridNo);
		iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

		if (iRange < iClosestRange)
		{
			iClosestRange = iRange;
			sClosestOpponent = sGridNo;
			bClosestLevel = bLevel;
		}
	}
	if (psGridNo)
	{
		*psGridNo = sClosestOpponent;
	}
	if (pbLevel)
	{
		*pbLevel = bClosestLevel;
	}
	return( sClosestOpponent );
}


INT16 ClosestPC(const SOLDIERTYPE* pSoldier, INT16* psDistance)
{
	// used by NPCs... find the closest PC

	// NOTE: skips EPCs!

	INT16 sMinDist = (INT16)WORLD_MAX;
	INT16 sDist;
	INT16 sGridNo = NOWHERE;

	CFOR_EACH_IN_TEAM(pTargetSoldier, OUR_TEAM)
	{
		if (!pTargetSoldier->bInSector)
		{
			continue;
		}

		// if not conscious, skip him
		if (pTargetSoldier->bLife < OKLIFE)
		{
			continue;
		}

		if ( AM_AN_EPC( pTargetSoldier ) )
		{
			continue;
		}

		sDist = PythSpacesAway(pSoldier->sGridNo,pTargetSoldier->sGridNo);

		// if this PC is not visible to the soldier, then add a penalty to the distance
		// so that we weight in favour of visible mercs
		if (pTargetSoldier->bTeam != pSoldier->bTeam && pSoldier->bOppList[pTargetSoldier->ubID] != SEEN_CURRENTLY)
		{
			sDist += 10;
		}

		if (sDist < sMinDist)
		{
			sMinDist = sDist;
			sGridNo = pTargetSoldier->sGridNo;
		}
	}

	if ( psDistance )
	{
		*psDistance = sMinDist;
	}

	return( sGridNo );
}


static INT16 FindClosestClimbPointAvailableToAI(SOLDIERTYPE* pSoldier, INT16 sStartGridNo, INT16 sDesiredGridNo, BOOLEAN fClimbUp)
{
	INT16 sGridNo;
	INT16 sRoamingOrigin;
	INT16 sRoamingRange;

	if ( pSoldier->uiStatusFlags & SOLDIER_PC )
	{
		sRoamingOrigin = pSoldier->sGridNo;
		sRoamingRange = 99;
	}
	else
	{
		sRoamingRange = RoamingRange( pSoldier, &sRoamingOrigin );
	}

	// since climbing necessary involves going an extra tile, we compare against 1 less than the roam range...
	// or add 1 to the distance to the climb point

	sGridNo = FindClosestClimbPoint( sStartGridNo, sDesiredGridNo, fClimbUp );


	if ( PythSpacesAway( sRoamingOrigin, sGridNo ) + 1 > sRoamingRange )
	{
		return( NOWHERE );
	}
	else
	{
		return( sGridNo );
	}
}

BOOLEAN ClimbingNecessary( SOLDIERTYPE * pSoldier, INT16 sDestGridNo, INT8 bDestLevel )
{
	if (pSoldier->bLevel == bDestLevel)
	{
		if ( (pSoldier->bLevel == 0) || ( gubBuildingInfo[ pSoldier->sGridNo ] == gubBuildingInfo[ sDestGridNo ] ) )
		{
			return( FALSE );
		}
		else // different buildings!
		{
			return( TRUE );
		}
	}
	else
	{
		return( TRUE );
	}
}

INT16 GetInterveningClimbingLocation( SOLDIERTYPE * pSoldier, INT16 sDestGridNo, INT8 bDestLevel, BOOLEAN * pfClimbingNecessary )
{
	if (pSoldier->bLevel == bDestLevel)
	{
		if ( (pSoldier->bLevel == 0) || ( gubBuildingInfo[ pSoldier->sGridNo ] == gubBuildingInfo[ sDestGridNo ] ) )
		{
			// on ground or same building... normal!
			*pfClimbingNecessary = FALSE;
			return( NOWHERE );
		}
		else
		{
			// different buildings!
			// yes, pass in same gridno twice... want closest climb-down spot for building we are on!
			*pfClimbingNecessary = TRUE;
			return( FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, pSoldier->sGridNo, FALSE ) );
		}
	}
	else
	{
		*pfClimbingNecessary = TRUE;
		// different levels
		if (pSoldier->bLevel == 0)
		{
			// got to go UP onto building
			return( FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, sDestGridNo, TRUE ) );
		}
		else
		{
			// got to go DOWN off building
			return( FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, pSoldier->sGridNo, FALSE ) );
		}
	}
}

INT16 EstimatePathCostToLocation( SOLDIERTYPE * pSoldier, INT16 sDestGridNo, INT8 bDestLevel, BOOLEAN fAddCostAfterClimbingUp, BOOLEAN * pfClimbingNecessary, INT16 * psClimbGridNo )
{
	INT16 sPathCost;
	INT16 sClimbGridNo;

	if (pSoldier->bLevel == bDestLevel)
	{
		if ( (pSoldier->bLevel == 0) || ( gubBuildingInfo[ pSoldier->sGridNo ] == gubBuildingInfo[ sDestGridNo ] ) )
		{
			// on ground or same building... normal!
			sPathCost = EstimatePlotPath(pSoldier, sDestGridNo, FALSE, FALSE, WALKING, 0);
			*pfClimbingNecessary = FALSE;
			*psClimbGridNo = NOWHERE;
		}
		else
		{
			// different buildings!
			// yes, pass in same gridno twice... want closest climb-down spot for building we are on!
			sClimbGridNo = FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, pSoldier->sGridNo, FALSE );
			if (sClimbGridNo == NOWHERE)
			{
				sPathCost = 0;
			}
			else
			{
				sPathCost = PlotPath(pSoldier, sClimbGridNo, FALSE, FALSE, WALKING, 0);
				if (sPathCost != 0)
				{
					// add in cost of climbing down
					if (fAddCostAfterClimbingUp)
					{
						// add in cost of later climbing up, too
						sPathCost += AP_CLIMBOFFROOF + AP_CLIMBROOF;
						// add in an estimate of getting there after climbing down
						sPathCost += (AP_MOVEMENT_FLAT + WALKCOST) * PythSpacesAway( sClimbGridNo, sDestGridNo );
					}
					else
					{
						sPathCost += AP_CLIMBOFFROOF;
						// add in an estimate of getting there after climbing down, *but not on top of roof*
						sPathCost += (AP_MOVEMENT_FLAT + WALKCOST) * PythSpacesAway( sClimbGridNo, sDestGridNo ) / 2;
					}
					*pfClimbingNecessary = TRUE;
					*psClimbGridNo = sClimbGridNo;
				}
			}
		}
	}
	else
	{
		// different levels
		if (pSoldier->bLevel == 0)
		{
			//got to go UP onto building
			sClimbGridNo = FindClosestClimbPointAvailableToAI( pSoldier,  pSoldier->sGridNo, sDestGridNo, TRUE );
		}
		else
		{
			// got to go DOWN off building
			sClimbGridNo = FindClosestClimbPointAvailableToAI( pSoldier, pSoldier->sGridNo, pSoldier->sGridNo, FALSE );
		}

		if (sClimbGridNo == NOWHERE)
		{
			sPathCost = 0;
		}
		else
		{
			sPathCost = PlotPath(pSoldier, sClimbGridNo, FALSE, FALSE, WALKING, 0);
			if (sPathCost != 0)
			{
				// add in the cost of climbing up or down
				if (pSoldier->bLevel == 0)
				{
					// must climb up
					sPathCost += AP_CLIMBROOF;
					if (fAddCostAfterClimbingUp)
					{
						// add to path a rough estimate of how far to go from the climb gridno to the friend
						// estimate walk cost
						sPathCost += (AP_MOVEMENT_FLAT + WALKCOST) * PythSpacesAway( sClimbGridNo, sDestGridNo );
					}
				}
				else
				{
					// must climb down
					sPathCost += AP_CLIMBOFFROOF;
					// add to path a rough estimate of how far to go from the climb gridno to the friend
					// estimate walk cost
					sPathCost += (AP_MOVEMENT_FLAT + WALKCOST) * PythSpacesAway( sClimbGridNo, sDestGridNo );
				}
				*pfClimbingNecessary = TRUE;
				*psClimbGridNo = sClimbGridNo;
			}
		}
	}

	return( sPathCost );
}


static BOOLEAN GuySawEnemyThisTurnOrBefore(SOLDIERTYPE* pSoldier)
{
	UINT8 ubTeamLoop;
	UINT8 ubIDLoop;

	for ( ubTeamLoop = 0; ubTeamLoop < MAXTEAMS; ubTeamLoop++ )
	{
		if ( gTacticalStatus.Team[ ubTeamLoop ].bSide != pSoldier->bSide )
		{
			// consider guys in this team, which isn't on our side
			for ( ubIDLoop = gTacticalStatus.Team[ ubTeamLoop ].bFirstID; ubIDLoop <= gTacticalStatus.Team[ ubTeamLoop ].bLastID; ubIDLoop++ )
			{
				// if this guy SAW an enemy recently...
				if ( pSoldier->bOppList[ ubIDLoop ] >= SEEN_CURRENTLY )
				{
					return( TRUE );
				}
			}
		}
	}

	return( FALSE );
}

INT16 ClosestReachableFriendInTrouble(SOLDIERTYPE *pSoldier, BOOLEAN * pfClimbingNecessary)
{
	INT16 sPathCost, sClosestFriend = NOWHERE, sShortestPath = 1000, sClimbGridNo;
	BOOLEAN fClimbingNecessary, fClosestClimbingNecessary = FALSE;

	// civilians don't really have any "friends", so they don't bother with this
	if (IsOnCivTeam(pSoldier)) return sClosestFriend;

	// consider every friend of this soldier (locations assumed to be known)
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pFriend = *i;

		// if this merc is neutral or NOT on the same side, he's not a friend
		if (pFriend->bNeutral || (pSoldier->bSide != pFriend->bSide))
		{
			continue;          // next merc
		}

		// if this "friend" is actually US
		if (pFriend == pSoldier) continue; // next merc

		// CJC: restrict "last one to radio" to only if that guy saw us this turn or last turn

		// if this friend is not under fire, and isn't the last one to radio
		if (!pFriend->bUnderFire &&
			(pFriend != gTacticalStatus.Team[pFriend->bTeam].last_merc_to_radio || !GuySawEnemyThisTurnOrBefore(pFriend)))
		{
			continue;          // next merc
		}

		// if we're already neighbors
		if (SpacesAway(pSoldier->sGridNo,pFriend->sGridNo) == 1)
		{
			continue;          // next merc
		}

		// get the AP cost to go to this friend's gridno
		sPathCost = EstimatePathCostToLocation( pSoldier, pFriend->sGridNo, pFriend->bLevel, TRUE, &fClimbingNecessary, &sClimbGridNo );

		// if we can get there
		if (sPathCost != 0)
		{
			if (sPathCost < sShortestPath)
			{
				if (fClimbingNecessary)
				{
					sClosestFriend = sClimbGridNo;
				}
				else
				{
					sClosestFriend = pFriend->sGridNo;
				}

				sShortestPath = sPathCost;
				fClosestClimbingNecessary = fClimbingNecessary;
			}
		}
	}
	*pfClimbingNecessary = fClosestClimbingNecessary;
	return(sClosestFriend);
}

INT16 DistanceToClosestFriend( SOLDIERTYPE * pSoldier )
{
	// find the distance to the closest person on the same team
	INT16 sMinDist = 1000;
	INT16 sDist;

	CFOR_EACH_IN_TEAM(pTargetSoldier, pSoldier->bTeam)
	{
		if (pTargetSoldier == pSoldier) continue;

		if (pSoldier->bInSector)
		{
			if (!pTargetSoldier->bInSector)
			{
				continue;
			}
			// if not conscious, skip him
			else if (pTargetSoldier->bLife < OKLIFE)
			{
				continue;
			}
		}
		else
		{
			// compare sector #s
			if ( (pSoldier->sSectorX != pTargetSoldier->sSectorX) ||
				(pSoldier->sSectorY != pTargetSoldier->sSectorY) ||
				(pSoldier->bSectorZ != pTargetSoldier->bSectorZ) )
			{
				continue;
			}
			else if (pTargetSoldier->bLife < OKLIFE)
			{
				continue;
			}
			else
			{
				// well there's someone who could be near
				return( 1 );
			}
		}

		sDist = SpacesAway(pSoldier->sGridNo,pTargetSoldier->sGridNo);

		if (sDist < sMinDist)
		{
			sMinDist = sDist;
		}
	}

	return( sMinDist );
}


static bool InSmoke(SOLDIERTYPE const* const s, GridNo const gridno)
{
	return gpWorldLevelData[gridno].ubExtFlags[s->bLevel] & MAPELEMENT_EXT_SMOKE;
}


bool InWaterGasOrSmoke(SOLDIERTYPE const* const s, GridNo const gridno)
{
	return WaterTooDeepForAttacks(gridno) || InSmoke(s, gridno) || InGas(s, gridno);
}


bool InGasOrSmoke(SOLDIERTYPE const* const s, GridNo const gridno)
{
	return InSmoke(s, gridno) || InGas(s, gridno);
}


bool InWaterOrGas(SOLDIERTYPE const* const s, GridNo const grid_no)
{
	return WaterTooDeepForAttacks(grid_no) || InGas(s, grid_no);
}


bool InGas(SOLDIERTYPE const* const s, GridNo const grid_no)
{
	return
		gpWorldLevelData[grid_no].ubExtFlags[s->bLevel] & (MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS) &&
		!IsWearingHeadGear(*s, GASMASK);
}


bool WearGasMaskIfAvailable(SOLDIERTYPE* const s)
{
	INT8 const slot = FindObj(s, GASMASK);
	if (slot == NO_SLOT || slot == HEAD1POS || slot == HEAD2POS) return false;

	INT8 const new_slot =
		s->inv[HEAD1POS].usItem == NOTHING ? HEAD1POS :
		s->inv[HEAD2POS].usItem == NOTHING ? HEAD2POS :
		HEAD1POS; // Screw it, going in position 1 anyhow

	RearrangePocket(s, slot, new_slot, TRUE);
	return true;
}


BOOLEAN InLightAtNight( INT16 sGridNo, INT8 bLevel )
{
	UINT8 ubBackgroundLightLevel;

	// do not consider us to be "in light" if we're in an underground sector
	if ( gbWorldSectorZ > 0 )
	{
		return( FALSE );
	}

	ubBackgroundLightLevel = GetTimeOfDayAmbientLightLevel();

	if ( ubBackgroundLightLevel < NORMAL_LIGHTLEVEL_DAY + 2 )
	{
		// don't consider it nighttime, too close to daylight levels
		return( FALSE );
	}

	// could've been placed here, ignore the light
	if (GetRoom(sGridNo) != NO_ROOM) return FALSE;

	// NB light levels are backwards, so a lower light level means the
	// spot in question is BRIGHTER
	return LightTrueLevel(sGridNo, bLevel) < ubBackgroundLightLevel;
}

INT8 CalcMorale(SOLDIERTYPE *pSoldier)
{
	INT32 iOurTotalThreat = 0, iTheirTotalThreat = 0;
	INT16 sOppThreatValue, sFrndThreatValue, sMorale;
	INT32 iPercent;
	INT8  bMostRecentOpplistValue;
	INT8  bMoraleCategory;
	INT8  *pbPersOL, *pbPublOL;

	// if army guy has NO weapons left then panic!
	if ( pSoldier->bTeam == ENEMY_TEAM )
	{
		if ( FindAIUsableObjClass( pSoldier, IC_WEAPON ) == NO_SLOT )
		{
			return( MORALE_HOPELESS );
		}
	}

	// hang pointers to my personal opplist, my team's public opplist, and my
	// list of previously seen opponents
	const INT8* pSeenOpp = &gbSeenOpponents[pSoldier->ubID][0];

	// loop through every one of my possible opponents
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		// if this merc is inactive, at base, on assignment, dead, unconscious
		if (pOpponent->bLife < OKLIFE) continue; // next merc

		// if this merc is neutral/on same side, he's not an opponent, skip him!
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
			continue;          // next merc

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY)
		{
			continue;  // next opponent
		}

		pbPersOL = pSoldier->bOppList + pOpponent->ubID;
		pbPublOL = gbPublicOpplist[pSoldier->bTeam] + pOpponent->ubID;
		pSeenOpp = gbSeenOpponents[pSoldier->ubID] + pOpponent->ubID;

		// if this opponent is unknown to me personally AND unknown to my team, too
		if ((*pbPersOL == NOT_HEARD_OR_SEEN) && (*pbPublOL == NOT_HEARD_OR_SEEN))
		{
			// if I have never seen him before anywhere in this sector, either
			if (!(*pSeenOpp))
				continue;        // next merc

			// have seen him in the past, so he remains something of a threat
			bMostRecentOpplistValue = 0;        // uses the free slot for 0 opplist
		}
		else         // decide which opplist is more current
		{
			// if personal knowledge is more up to date or at least equal
			if ((gubKnowledgeValue[*pbPublOL - OLDEST_HEARD_VALUE][*pbPersOL - OLDEST_HEARD_VALUE] > 0) || (*pbPersOL == *pbPublOL))
				bMostRecentOpplistValue = *pbPersOL;      // use personal
			else
				bMostRecentOpplistValue = *pbPublOL;      // use public
		}

		iPercent = ThreatPercent[bMostRecentOpplistValue - OLDEST_HEARD_VALUE];

		sOppThreatValue = (iPercent * CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier)) / 100;

		// ADD this to their running total threatValue (decreases my MORALE)
		iTheirTotalThreat += sOppThreatValue;

		// NOW THE FUN PART: SINCE THIS OPPONENT IS KNOWN TO ME IN SOME WAY,
		// ANY FRIENDS OF MINE THAT KNOW ABOUT HIM BOOST MY MORALE.  SO, LET'S GO
		// THROUGH THEIR PERSONAL OPPLISTS AND CHECK WHICH OF MY FRIENDS KNOW
		// SOMETHING ABOUT HIM AND WHAT THEIR THREAT VALUE TO HIM IS.

		FOR_EACH_MERC(j)
		{
			SOLDIERTYPE* const pFriend = *j;

			// if this merc is inactive, at base, on assignment, dead, unconscious
			if (pFriend->bLife < OKLIFE) continue; // next merc

			// if this merc is not on my side, then he's NOT one of my friends

			// WE CAN'T AFFORD TO CONSIDER THE ENEMY OF MY ENEMY MY FRIEND, HERE!
			// ONLY IF WE ARE ACTUALLY OFFICIALLY CO-OPERATING TOGETHER (SAME SIDE)
			if ( pFriend->bNeutral && !( pSoldier->ubCivilianGroup != NON_CIV_GROUP && pSoldier->ubCivilianGroup == pFriend->ubCivilianGroup ) )
			{
				continue;        // next merc
			}

			if ( pSoldier->bSide != pFriend->bSide )
				continue;        // next merc

			// THIS TEST IS INVALID IF A COMPUTER-TEAM IS PLAYING CO-OPERATIVELY
			// WITH A NON-COMPUTER TEAM SINCE THE OPPLISTS INVOLVED ARE NOT
			// UP-TO-DATE.  THIS SITUATION IS CURRENTLY NOT POSSIBLE IN HTH/DG.

			// ALSO NOTE THAT WE COUNT US AS OUR (BEST) FRIEND FOR THESE CALCULATIONS

			// subtract HEARD_2_TURNS_AGO (which is negative) to make values start at 0 and
			// be positive otherwise
			iPercent = ThreatPercent[pFriend->bOppList[pOpponent->ubID] - OLDEST_HEARD_VALUE];

			// reduce the percentage value based on how far away they are from the enemy, if they only hear him
			if ( pFriend->bOppList[ pOpponent->ubID ] <= HEARD_LAST_TURN )
			{
				iPercent -= PythSpacesAway( pSoldier->sGridNo, pFriend->sGridNo ) * 2;
				if ( iPercent <= 0 )
				{
					//ignore!
					continue;
				}
			}

			sFrndThreatValue = (iPercent * CalcManThreatValue(pFriend,pOpponent->sGridNo,FALSE,pSoldier)) / 100;

			// ADD this to our running total threatValue (increases my MORALE)
			// We multiply by sOppThreatValue to PRO-RATE this based on opponent's
			// threat value to ME personally.  Divide later by sum of them all.
			iOurTotalThreat += sOppThreatValue * sFrndThreatValue;
		}
	}


	// if they are no threat whatsoever
	if (!iTheirTotalThreat)
		sMorale = 500;        // our morale is just incredible
	else
	{
		// now divide sOutTotalThreat by sTheirTotalThreat to get the REAL value
		iOurTotalThreat /= iTheirTotalThreat;

		// calculate the morale (100 is even, < 100 is us losing, > 100 is good)
		sMorale = (INT16) ((100 * iOurTotalThreat) / iTheirTotalThreat);
	}


	if (sMorale <= 25)              // odds 1:4 or worse
		bMoraleCategory = MORALE_HOPELESS;
	else if (sMorale <= 50)         // odds between 1:4 and 1:2
		bMoraleCategory = MORALE_WORRIED;
	else if (sMorale <= 150)        // odds between 1:2 and 3:2
		bMoraleCategory = MORALE_NORMAL;
	else if (sMorale <= 300)        // odds between 3:2 and 3:1
		bMoraleCategory = MORALE_CONFIDENT;
	else                           // odds better than 3:1
		bMoraleCategory = MORALE_FEARLESS;


	switch (pSoldier->bAttitude)
	{
		case DEFENSIVE:	bMoraleCategory--; break;
		case BRAVESOLO:	bMoraleCategory += 2; break;
		case BRAVEAID:	bMoraleCategory += 2; break;
		case CUNNINGSOLO:    break;
		case CUNNINGAID:     break;
		case AGGRESSIVE:	bMoraleCategory++; break;
	}

	// make idiot administrators much more aggressive
	if ( pSoldier->ubSoldierClass == SOLDIER_CLASS_ADMINISTRATOR )
	{
		bMoraleCategory += 2;
	}


	// if still full of energy
	if (pSoldier->bBreath > 75)
		bMoraleCategory++;
	else
	{
		// if getting a bit low on breath
		if (pSoldier->bBreath < 40)
			bMoraleCategory--;

		// if getting REALLY low on breath
		if (pSoldier->bBreath < 10)
			bMoraleCategory--;
	}


	// if still very healthy
	if (pSoldier->bLife > 75)
		bMoraleCategory++;
	else
	{
		// if getting a bit low on life
		if (pSoldier->bLife < 40)
			bMoraleCategory--;

		// if getting REALLY low on life
		if (pSoldier->bLife < 20)
			bMoraleCategory--;
	}


	// if soldier is currently not under fire
	if (!pSoldier->bUnderFire)
		bMoraleCategory++;


	// if adjustments made it outside the allowed limits
	if (bMoraleCategory < MORALE_HOPELESS)
		bMoraleCategory = MORALE_HOPELESS;
	else
	{
		if (bMoraleCategory > MORALE_FEARLESS)
			bMoraleCategory = MORALE_FEARLESS;
	}

	// brave guys never get hopeless, at worst they get worried
	if (bMoraleCategory == MORALE_HOPELESS &&
		(pSoldier->bAttitude == BRAVESOLO || pSoldier->bAttitude == BRAVEAID))
		bMoraleCategory = MORALE_WORRIED;

	SLOGD("Morale = %d (category %d), iOurTotalThreat %d, iTheirTotalThreat %d",
				sMorale, bMoraleCategory, iOurTotalThreat, iTheirTotalThreat);

	return(bMoraleCategory);
}

INT32 CalcManThreatValue( SOLDIERTYPE *pEnemy, INT16 sMyGrid, UINT8 ubReduceForCover, SOLDIERTYPE * pMe )
{
	INT32	iThreatValue = 0;
	BOOLEAN fForCreature = CREATURE_OR_BLOODCAT( pMe );

	// If man is inactive, at base, on assignment, dead, unconscious
	if (!pEnemy->bActive || !pEnemy->bInSector || !pEnemy->bLife)
	{
		// he's no threat at all, return a negative number
		iThreatValue = -999;
		return(iThreatValue);
	}

	// in boxing mode, let only a boxer be considered a threat.
	if ( (gTacticalStatus.bBoxingState == BOXING) && !(pEnemy->uiStatusFlags & SOLDIER_BOXER) )
	{
		iThreatValue = -999;
		return( iThreatValue );
	}

	if (fForCreature)
	{
		// health (1-100)
		iThreatValue += pEnemy->bLife;
		// bleeding (more attactive!) (1-100)
		iThreatValue += pEnemy->bBleeding;
		// decrease according to distance
		iThreatValue = (iThreatValue * 10) / (10 + PythSpacesAway( sMyGrid, pEnemy->sGridNo ) );

	}
	else
	{
		// ADD twice the man's level (2-20)
		iThreatValue += pEnemy->bExpLevel;

		// ADD man's total action points (10-35)
		iThreatValue += CalcActionPoints(pEnemy);

		// ADD 1/2 of man's current action points (4-17)
		iThreatValue += (pEnemy->bActionPoints / 2);

		// ADD 1/10 of man's current health (0-10)
		iThreatValue += (pEnemy->bLife / 10);

		if (pEnemy->bAssignment < ON_DUTY )
		{
			// ADD 1/4 of man's protection percentage (0-25)
			iThreatValue += ArmourPercent( pEnemy ) / 4;

			// ADD 1/5 of man's marksmanship skill (0-20)
			iThreatValue += (pEnemy->bMarksmanship / 5);

			if ( GCM->getItem(pEnemy->inv[HANDPOS].usItem)->isWeapon() )
			{
				// ADD the deadliness of the item(weapon) he's holding (0-50)
				iThreatValue += GCM->getWeapon(pEnemy->inv[HANDPOS].usItem)->ubDeadliness;
			}
		}

		// SUBTRACT 1/5 of man's bleeding (0-20)
		iThreatValue -= (pEnemy->bBleeding / 5);

		// SUBTRACT 1/10 of man's breath deficiency (0-10)
		iThreatValue -= ((100 - pEnemy->bBreath) / 10);

		// SUBTRACT man's current shock value
		iThreatValue -= pEnemy->bShock;
	}

	// if I have a specifically defined spot where I'm at (sometime I don't!)
	if (sMyGrid != NOWHERE)
	{
		// ADD 10% if man's already been shooting at me
		if (pEnemy->sLastTarget == sMyGrid)
		{
			iThreatValue += (iThreatValue / 10);
		}
		else
		{
			// ADD 5% if man's already facing me
			if (pEnemy->bDirection == GetDirectionToGridNoFromGridNo(pEnemy->sGridNo, sMyGrid))
			{
				iThreatValue += (iThreatValue / 20);
			}
		}
	}

	// if this man is conscious
	if (pEnemy->bLife >= OKLIFE)
	{
		// and we were told to reduce threat for my cover
		if (ubReduceForCover && (sMyGrid != NOWHERE))
		{
			// Reduce iThreatValue to same % as the chance HE has shoot through at ME
			//iThreatValue = (iThreatValue * ChanceToGetThrough( pEnemy, myGrid, FAKE, ACTUAL, TESTWALLS, 9999, M9PISTOL, NOT_FOR_LOS)) / 100;
			//iThreatValue = (iThreatValue * SoldierTo3DLocationChanceToGetThrough( pEnemy, myGrid, FAKE, ACTUAL, TESTWALLS, 9999, M9PISTOL, NOT_FOR_LOS)) / 100;
			iThreatValue = iThreatValue * SoldierToLocationChanceToGetThrough(pEnemy, sMyGrid, pMe->bLevel, 0, pMe) / 100;
		}
	}
	else
	{
		// if he's still something of a threat
		if (iThreatValue > 0)
		{
			// drastically reduce his threat value (divide by 5 to 18)
			iThreatValue /= (4 + (OKLIFE - pEnemy->bLife));
		}
	}

	// threat value of any opponent can never drop below 1
	if (iThreatValue < 1)
	{
		iThreatValue = 1;
	}
	return(iThreatValue);
}

INT16 RoamingRange(SOLDIERTYPE *pSoldier, INT16 * pusFromGridNo)
{
	if ( CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		if ( pSoldier->bAlertStatus == STATUS_BLACK )
		{
			*pusFromGridNo = pSoldier->sGridNo; // from current position!
			return(MAX_ROAMING_RANGE);
		}
	}
	if ( pSoldier->bOrders == POINTPATROL || pSoldier->bOrders == RNDPTPATROL )
	{
		// roam near NEXT PATROL POINT, not from where merc starts out
		*pusFromGridNo = pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt];
	}
	else
	{
		// roam around where mercs started
		//*pusFromGridNo = pSoldier->sInitialGridNo;
		*pusFromGridNo = pSoldier->usPatrolGrid[0];
	}

	switch (pSoldier->bOrders)
	{
		// JA2 GOLD: give non-NPCs a 5 tile roam range for cover in combat when being shot at
		case STATIONARY:
			if (pSoldier->ubProfile != NO_PROFILE || (pSoldier->bAlertStatus < STATUS_BLACK && !(pSoldier->bUnderFire)))
			{
				return( 0 );
			}
			else
			{
				return( 5 );
			}
		case ONGUARD:
			return( 5 );
		case CLOSEPATROL:
			return( 15 );
		case RNDPTPATROL:
		case POINTPATROL:
			return(10 );     // from nextPatrolGrid, not whereIWas
		case FARPATROL:
			if (pSoldier->bAlertStatus < STATUS_RED)
			{
				return( 25 );
			}
			else
			{
				return( 50 );
			}
		case ONCALL:
			if (pSoldier->bAlertStatus < STATUS_RED)
			{
				return( 10 );
			}
			else
			{
				return( 30 );
			}
		case SEEKENEMY:
			*pusFromGridNo = pSoldier->sGridNo; // from current position!
			return(MAX_ROAMING_RANGE);
		default:
			return(0);
	}
}


void RearrangePocket(SOLDIERTYPE *pSoldier, INT8 bPocket1, INT8 bPocket2, UINT8 bPermanent)
{
	// NB there's no such thing as a temporary swap for now...
	SwapObjs( &(pSoldier->inv[bPocket1]), &(pSoldier->inv[bPocket2]) );
}


bool FindBetterSpotForItem(SOLDIERTYPE& s, INT8 const slot)
{
	/* Look for a place in the slots to put an item in a hand or armour position,
	 * and move it there. */
	if (slot >= BIGPOCK1POS) return false;

	UINT16 const item = s.inv[slot].usItem;
	if (item == NOTHING) return true; // That's just fine

	INT8 new_slot;
	if (GCM->getItem(item)->getPerPocket() != 0)
	{ // Try a small pocket first
		new_slot = FindEmptySlotWithin(&s, SMALLPOCK1POS, SMALLPOCK8POS);
		if (new_slot == NO_SLOT) goto try_big_pocket;
	}
	else
	{ // Look for a big pocket
try_big_pocket:
		new_slot = FindEmptySlotWithin(&s, BIGPOCK1POS, BIGPOCK4POS);
		if (new_slot == NO_SLOT) return false;
	}

	RearrangePocket(&s, HANDPOS, new_slot, FOREVER);
	return true;
}


UINT8 GetTraversalQuoteActionID( INT8 bDirection )
{
	switch( bDirection )
	{
		case NORTHEAST: // east
			return( QUOTE_ACTION_ID_TRAVERSE_EAST );

		case SOUTHEAST: // south
			return( QUOTE_ACTION_ID_TRAVERSE_SOUTH );

		case SOUTHWEST: // west
			return( QUOTE_ACTION_ID_TRAVERSE_WEST );

		case NORTHWEST: // north
			return( QUOTE_ACTION_ID_TRAVERSE_NORTH );

		default:
			return( 0 );
	}
}

UINT8 SoldierDifficultyLevel( const SOLDIERTYPE * pSoldier )
{
	INT8 bDifficultyBase;
	INT8 bDifficulty;

	// difficulty modifier ranges from 0 to 100
	// and we want to end up with a number between 0 and 4 (4=hardest)
	// to a base of 1, divide by 34 to get a range from 1 to 3
	bDifficultyBase = 1 + ( CalcDifficultyModifier( pSoldier->ubSoldierClass ) / 34 );

	switch( pSoldier->ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:
			bDifficulty = bDifficultyBase - 1;
			break;

		case SOLDIER_CLASS_ARMY:
			bDifficulty = bDifficultyBase;
			break;

		case SOLDIER_CLASS_ELITE:
			bDifficulty = bDifficultyBase + 1;
			break;

		// hard code militia;
		case SOLDIER_CLASS_GREEN_MILITIA:
			bDifficulty = 2;
			break;

		case SOLDIER_CLASS_REG_MILITIA:
			bDifficulty = 3;
			break;

		case SOLDIER_CLASS_ELITE_MILITIA:
			bDifficulty = 4;
			break;

		default:
			if (pSoldier->bTeam == CREATURE_TEAM)
			{
				bDifficulty = bDifficultyBase + pSoldier->bLevel / 4;
			}
			else // civ...
			{
				bDifficulty = (bDifficultyBase + pSoldier->bLevel / 4) - 1;
			}
			break;

	}

	bDifficulty = __max( bDifficulty, 0 );
	bDifficulty = __min( bDifficulty, 4 );

	return( (UINT8) bDifficulty );
}

BOOLEAN ValidCreatureTurn( SOLDIERTYPE * pCreature, INT8 bNewDirection )
{
	INT8    bDirChange;
	INT8    bTempDir;
	INT8    bLoop;
	BOOLEAN fFound;

	bDirChange = (INT8) QuickestDirection( pCreature->bDirection, bNewDirection );

	for( bLoop = 0; bLoop < 2; bLoop++ )
	{
		fFound = TRUE;

		bTempDir = pCreature->bDirection;

		do
		{

			bTempDir += bDirChange;
			if (bTempDir < NORTH)
			{
				bTempDir = NORTHWEST;
			}
			else if (bTempDir > NORTHWEST)
			{
				bTempDir = NORTH;
			}
			if (!InternalIsValidStance( pCreature, bTempDir, ANIM_STAND ))
			{
				fFound = FALSE;
				break;
			}

		} while ( bTempDir != bNewDirection );

		if ( fFound )
		{
			break;
		}
		else if ( bLoop > 0 )
		{
			// can't find a dir!
			return( FALSE );
		}
		else
		{
			// try the other direction
			bDirChange = bDirChange * -1;
		}
	}

	return( TRUE );
}

INT32 RangeChangeDesire( const SOLDIERTYPE * pSoldier )
{
	INT32 iRangeFactorMultiplier;

	iRangeFactorMultiplier = pSoldier->bAIMorale - 1;
	switch (pSoldier->bAttitude)
	{
		case DEFENSIVE:      iRangeFactorMultiplier += -1; break;
		case BRAVESOLO:      iRangeFactorMultiplier +=  2; break;
		case BRAVEAID:       iRangeFactorMultiplier +=  2; break;
		case CUNNINGSOLO:    iRangeFactorMultiplier +=  0; break;
		case CUNNINGAID:     iRangeFactorMultiplier +=  0; break;
		case ATTACKSLAYONLY:
		case AGGRESSIVE:     iRangeFactorMultiplier +=  1; break;
	}
	if ( gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes > 0 )
	{
		iRangeFactorMultiplier += gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes;
	}
	return( iRangeFactorMultiplier );
}

BOOLEAN ArmySeesOpponents( void )
{
	CFOR_EACH_IN_TEAM(s, ENEMY_TEAM)
	{
		if (s->bInSector && s->bLife >= OKLIFE && s->bOppCnt > 0)
		{
			return TRUE;
		}
	}

	return( FALSE );
}
