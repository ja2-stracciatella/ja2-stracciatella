#include "AI.h"
#include "Animation_Control.h"
#include "OppList.h"
#include "AIInternals.h"
#include "LOS.h"
#include "Physics.h"
#include "Items.h"
#include "Structure.h"
#include "TileDef.h"
#include "Weapons.h"
#include "Spread_Burst.h"
#include "Overhead.h"
#include "SkillCheck.h"
#include "Soldier_Profile.h"
#include "Isometric_Utils.h"
#include "Soldier_Macros.h"
#include "PathAI.h"
#include "GameSettings.h"
#include "StrategicMap.h"
#include "Lighting.h"
#include "Environment.h"
#include "WorldMan.h"

#include "CalibreModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"
#include "policy/GamePolicy.h"

//
// CJC DG->JA2 conversion notes
//
// Still commented out:
//
// EstimateShotDamage - stuff related to legs?
// EstimateStabDamage - stuff related to armour
// EstimateThrowDamage - waiting for grenade, armour definitions
// CheckIfTossPossible - waiting for grenade definitions

// this define should go in soldier control.h


void LoadWeaponIfNeeded(SOLDIERTYPE *pSoldier)
{
	ItemId usInHand;
	INT8 bPayloadPocket;

	usInHand = pSoldier->inv[HANDPOS].usItem;

	// if he's got a MORTAR in his hand, make sure he has a MORTARSHELL avail.
	if (usInHand == MORTAR)
	{
		bPayloadPocket = FindObj( pSoldier, MORTAR_SHELL );
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no shells, can't fire the MORTAR
		}
	}
	// if he's got a GL in his hand, make sure he has some type of GRENADE avail.
	else if (usInHand == GLAUNCHER)
	{
		bPayloadPocket = FindGLGrenade( pSoldier );
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no grenades, can't fire the GLAUNCHER
		}
	}
	else if (usInHand == TANK_CANNON)
	{
		bPayloadPocket = FindLaunchable( pSoldier, TANK_CANNON );
		if (bPayloadPocket == NO_SLOT)
		{
			return;
		}
	}
	else
	{
		// regular hand-thrown grenade in hand, nothing to load!
		return;
	}

	// remove payload from its pocket, and add it as the hand weapon's first attachment
	pSoldier->inv[HANDPOS].usAttachItem[0] = pSoldier->inv[bPayloadPocket].usItem;
	pSoldier->inv[HANDPOS].bAttachStatus[0] = pSoldier->inv[bPayloadPocket].bStatus[0];

	if ( TANK( pSoldier ) )
	{
		// don't remove ammo
		return;
	}
	// if there's only one in payload pocket (only/last grenade, or any shell)
	if ((GCM->getItem(pSoldier->inv[bPayloadPocket].usItem )->getPerPocket() == 1) || (pSoldier->inv[bPayloadPocket].ubNumberOfObjects == 1))
	{
		DeleteObj(&(pSoldier->inv[bPayloadPocket]));
	}
	else	// multiple grenades, remove one of them
	{
		pSoldier->inv[bPayloadPocket].ubNumberOfObjects--;
	}
}


static INT32 EstimateShotDamage(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubChanceToHit);


void CalcBestShot(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestShot)
{
	INT32 iAttackValue;
	INT32 iThreatValue;
	INT32 iHitRate,iBestHitRate,iPercentBetter;
	INT32 iEstDamage;
	UINT8 ubRawAPCost,ubMinAPcost,ubMaxPossibleAimTime,ubAimTime,ubBestAimTime;
	UINT8 ubChanceToHit,ubChanceToGetThrough,ubChanceToReallyHit,ubBestChanceToHit = 0;

	ubBestChanceToHit = ubBestAimTime = ubChanceToHit = 0;

	pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;

	UINT8 const ubBurstAPs = CalcAPsToBurst(CalcActionPoints(pSoldier), pSoldier->inv[HANDPOS]);

	InitAttackType(pBestShot);      // set all structure fields to defaults

	// hang a pointer into active soldier's personal opponent list
	//pbPersOL = &(pSoldier->bOppList[0]);

	// determine which attack against which target has the greatest attack value
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent->bLife) continue; // next merc

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
			continue;          // next merc

		// if this opponent is not currently in sight (ignore known but unseen!)
		if (pSoldier->bOppList[pOpponent->ubID] != SEEN_CURRENTLY)
			continue;  // next opponent

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY)
			continue;  // next opponent

		// calculate minimum action points required to shoot at this opponent
		ubMinAPcost = MinAPsToAttack(pSoldier,pOpponent->sGridNo,ADDTURNCOST);

		// if we don't have enough APs left to shoot even a snap-shot at this guy
		if (ubMinAPcost > pSoldier->bActionPoints)
			continue;          // next opponent

		// calculate chance to get through the opponent's cover (if any)

		ubChanceToGetThrough = AISoldierToSoldierChanceToGetThrough( pSoldier, pOpponent );

		//   ubChanceToGetThrough = ChanceToGetThrough(pSoldier,pOpponent->sGridNo,NOTFAKE,ACTUAL,TESTWALLS,9999,M9PISTOL,NOT_FOR_LOS);

		// if we can't possibly get through all the cover
		if (ubChanceToGetThrough == 0)
			continue;          // next opponent

		if ( (pSoldier->uiStatusFlags & SOLDIER_MONSTER) && (pSoldier->ubBodyType != QUEENMONSTER ) )
		{
			UINT16 usAnimSurface;

			usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, pSoldier->usUIMovementMode );
			const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface,pSoldier->usUIMovementMode);
			if ( pStructureFileRef )
			{
				UINT16 usStructureID;
				INT8   bDir;

				// must make sure that structure data can be added in the direction of the target
				bDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, pOpponent->sGridNo );

				// ATE: Only if we have a levelnode...
				if ( pSoldier->pLevelNode != NULL && pSoldier->pLevelNode->pStructureData != NULL )
				{
					usStructureID = pSoldier->pLevelNode->pStructureData->usStructureID;
				}
				else
				{
					usStructureID = INVALID_STRUCTURE_ID;
				}

				if (!OkayToAddStructureToWorld(pSoldier->sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(bDir)], usStructureID))
				{
					// can't turn in that dir.... next opponent
					continue;
				}
			}
		}

		// calc next attack's minimum shooting cost (excludes readying & turning)
		ubRawAPCost = MinAPsToShootOrStab(*pSoldier, pOpponent->sGridNo, FALSE);

		if (pOpponent->sGridNo != pSoldier->sLastTarget)
		{
			// raw AP cost calculation included cost of changing target!
			ubRawAPCost -= AP_CHANGE_TARGET;
		}

		iBestHitRate = 0;                     // reset best hit rate to minimum

		// calculate the maximum possible aiming time

		if ( TANK( pSoldier ) )
		{
			ubMaxPossibleAimTime = pSoldier->bActionPoints - ubMinAPcost;

			// always burst
			if ( ubMaxPossibleAimTime < ubBurstAPs )
			{
				// should this be a return instead?
				continue;
			}
			// bursts aren't aimed
			ubMaxPossibleAimTime = 0;

		}
		else
		{
			ubMaxPossibleAimTime = std::min(AP_MAX_AIM_ATTACK,pSoldier->bActionPoints - ubMinAPcost);
		}

		// consider the various aiming times
		for (ubAimTime = AP_MIN_AIM_ATTACK; ubAimTime <= ubMaxPossibleAimTime; ubAimTime++)
		{
			//HandleMyMouseCursor(KEYBOARDALSO);
			UINT8 target = AIM_SHOT_TORSO;
			if (gamepolicy(ai_better_aiming_choice)) {
				target = pSoldier->bAimShotLocation;
			}
			ubChanceToHit = (UINT8) AICalcChanceToHitGun(pSoldier, pOpponent->sGridNo, ubAimTime, target);
			// ExtMen[pOpponent->ubID].haveStats = TRUE;

			iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime);

			// if aiming for this amount of time produces a better hit rate
			if (iHitRate > iBestHitRate)
			{
				iBestHitRate = iHitRate;
				ubBestAimTime = ubAimTime;
				ubBestChanceToHit = ubChanceToHit;
			}
		}


		// if we can't get any kind of hit rate at all
		if (iBestHitRate == 0)
			continue;          // next opponent

		// calculate chance to REALLY hit: shoot accurately AND get past cover
		ubChanceToReallyHit = (ubBestChanceToHit * ubChanceToGetThrough) / 100;

		// if we can't REALLY hit at all
		if (ubChanceToReallyHit == 0)
			continue;          // next opponent

		// really limit knife throwing so it doesn't look wrong
		if ( GCM->getItem(pSoldier->usAttackingWeapon)->getItemClass() == IC_THROWING_KNIFE && (ubChanceToReallyHit < 30 || ( PythSpacesAway( pSoldier->sGridNo, pOpponent->sGridNo ) > CalcMaxTossRange( pSoldier, THROWING_KNIFE, FALSE ) / 2 ) ) )
			continue; // don't bother... next opponent

		// calculate this opponent's threat value (factor in my cover from him)
		iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,TRUE,pSoldier);

		// estimate the damage this shot would do to this opponent
		iEstDamage = EstimateShotDamage(pSoldier,pOpponent,ubBestChanceToHit);

		// calculate the combined "attack value" for this opponent
		// highest possible value before division should be about 1.8 billion...
		// normal value before division should be about 5 million...
		iAttackValue = (iEstDamage * iBestHitRate * ubChanceToReallyHit * iThreatValue) / 1000;

		// special stuff for assassins to ignore militia more
		if ( pSoldier->ubProfile >= JIM && pSoldier->ubProfile <= TYRONE && pOpponent->bTeam == MILITIA_TEAM )
		{
			iAttackValue /= 2;
		}

		// if we can hurt the guy, OR probably not, but at least it's our best
		// chance to actually hit him and maybe scare him, knock him down, etc.
		if ((iAttackValue > 0) || (ubChanceToReallyHit > pBestShot->ubChanceToReallyHit))
		{
			// if there already was another viable target
			if (pBestShot->ubChanceToReallyHit > 0)
			{
				// OK, how does our chance to hit him compare to the previous best one?
				iPercentBetter = ((ubChanceToReallyHit * 100) / pBestShot->ubChanceToReallyHit) - 100;

				// if this chance to really hit is more than 50% worse, and the other
				// guy is conscious at all
				if (iPercentBetter < -PERCENT_TO_IGNORE_THREAT && pBestShot->opponent->bLife >= OKLIFE)
					// then stick with the older guy as the better target
					continue;

				// if this chance to really hit between 50% worse to 50% better
				if (iPercentBetter < PERCENT_TO_IGNORE_THREAT)
				{
					// then the one with the higher ATTACK VALUE is the better target
					if (iAttackValue < pBestShot->iAttackValue)
						// the previous guy is more important since he's more dangerous
						continue;            // next opponent
					}
			}

			// OOOF!  That was a lot of work!  But we've got a new best target!
			pBestShot->ubPossible          = TRUE;
			pBestShot->opponent            = pOpponent;
			pBestShot->ubAimTime           = ubBestAimTime;
			pBestShot->ubChanceToReallyHit = ubChanceToReallyHit;
			pBestShot->sTarget             = pOpponent->sGridNo;
			pBestShot->bTargetLevel        = pOpponent->bLevel;
			pBestShot->iAttackValue        = iAttackValue;
			pBestShot->ubAPCost            = ubMinAPcost + ubBestAimTime;
		}
	}
}


// JA2Gold: added
static BOOLEAN CloseEnoughForGrenadeToss(INT16 sGridNo, INT16 sGridNo2)
{
	INT16 sTempGridNo;
	UINT8 ubMovementCost;

	if (sGridNo == sGridNo2 )
	{
		// checking the same space; if there is a closed door next to location in ANY direction then forget it
		// (could be the player closed a door on us)
		for (INT8 bDirection = 0; bDirection < NUM_WORLD_DIRECTIONS; ++bDirection)
		{
			sTempGridNo = NewGridNo( sGridNo, DirectionInc( bDirection ) );
			ubMovementCost = gubWorldMovementCosts[ sTempGridNo ][ bDirection ][ 0 ];
			if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
			{
				ubMovementCost = DoorTravelCost( NULL, sTempGridNo, ubMovementCost, FALSE, NULL );
			}
			if ( ubMovementCost >= TRAVELCOST_BLOCKED)
			{
				return( FALSE );
			}
		}
	}
	else
	{
		if ( CardinalSpacesAway( sGridNo, sGridNo2 ) > 2 )
		{
			return( FALSE );
		}

		// we are within 1 space diagonally or at most 2 horizontally or vertically,
		// so we can now do a loop safely

		sTempGridNo = sGridNo;

		const INT8 bDirection = GetDirectionToGridNoFromGridNo(sGridNo, sGridNo2);

		// For each step of the loop, we are checking for door or obstacle movement costs.  If we
		// find we're blocked, then this is no good for grenade tossing!
		do
		{
			sTempGridNo = NewGridNo( sTempGridNo, DirectionInc( bDirection ) );
			ubMovementCost = gubWorldMovementCosts[ sTempGridNo ][ bDirection ][ 0 ];
			if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
			{
				ubMovementCost = DoorTravelCost( NULL, sTempGridNo, ubMovementCost, FALSE, NULL );
			}
			if ( ubMovementCost >= TRAVELCOST_BLOCKED)
			{
				return( FALSE );
			}
		} while( sTempGridNo != sGridNo2 );
	}

	return( TRUE );
}


static INT32 EstimateThrowDamage(SOLDIERTYPE* pSoldier, UINT8 ubItemPos, SOLDIERTYPE* pOpponent, INT16 sGridno);


static void CalcBestThrow(SOLDIERTYPE* pSoldier, ATTACKTYPE* pBestThrow)
{
	// September 9, 1998: added code for LAWs (CJC)
	UINT8 ubLoop2;
	INT32 iAttackValue;
	INT32 iHitRate, iThreatValue, iTotalThreatValue,iOppThreatValue[MAXMERCS];
	INT16 sGridNo, sEndGridNo, sFriendTile[MAXMERCS], sOpponentTile[MAXMERCS];
	INT8  bFriendLevel[MAXMERCS], bOpponentLevel[MAXMERCS];
	INT32 iEstDamage;
	UINT8 ubFriendCnt = 0;
	UINT8 ubOpponentCnt = 0;
	SOLDIERTYPE* opponents[MAXMERCS];
	UINT8 ubRawAPCost,ubMinAPcost,ubMaxPossibleAimTime;
	UINT8 ubChanceToHit,ubChanceToGetThrough,ubChanceToReallyHit;
	UINT32 uiPenalty;
	UINT8 ubSearchRange;
	UINT16 usOppDist;
	BOOLEAN fFriendsNearby;
	ItemId usInHand, usGrenade;
	UINT8 ubOppsInRange, ubOppsAdjacent;
	BOOLEAN fSkipLocation;
	INT8  bPayloadPocket;
	INT8  bMaxLeft,bMaxRight,bMaxUp,bMaxDown,bXOffset,bYOffset;
	INT8  bPersOL, bPublOL;
	static INT16 sExcludeTile[100]; // This array is for storing tiles that we have
	UINT8 ubNumExcludedTiles = 0;		// already considered, to prevent duplication of effort
	INT32 iTossRange;
	UINT8 ubSafetyMargin = 0;
	UINT8 ubDiff;
	INT8  bEndLevel;

	usInHand = pSoldier->inv[HANDPOS].usItem;
	usGrenade = NOTHING;

	if ( EXPLOSIVE_GUN( usInHand ) )
	{
		iTossRange = GCM->getWeapon( usInHand )->usRange / CELL_X_SIZE;
	}
	else
	{
		iTossRange = CalcMaxTossRange( pSoldier, usInHand, TRUE );
	}

	// if he's got a MORTAR in his hand, make sure he has a MORTARSHELL avail.
	if (usInHand == MORTAR)
	{
		bPayloadPocket = FindObj( pSoldier, MORTAR_SHELL );
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no shells, can't fire the MORTAR
		}
		ubSafetyMargin = Explosive[ GCM->getItem(MORTAR_SHELL)->getClassIndex() ].ubRadius;
	}
	// if he's got a GL in his hand, make sure he has some type of GRENADE avail.
	else if (usInHand == GLAUNCHER)
	{
		// use up pocket 2 first, they get left as drop items
		bPayloadPocket = FindGLGrenade( pSoldier );
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no grenades, can't fire the GLAUNCHER
		}
		ubSafetyMargin = Explosive[ GCM->getItem(pSoldier->inv[ bPayloadPocket ].usItem)->getClassIndex() ].ubRadius;
		usGrenade = pSoldier->inv[ bPayloadPocket ].usItem;
	}
	else if (usInHand == ROCKET_LAUNCHER)
	{
		// put in hand
		bPayloadPocket = HANDPOS;
		// as C1
		ubSafetyMargin = Explosive[ GCM->getItem(C1)->getClassIndex() ].ubRadius;
	}
	else if (usInHand == TANK_CANNON)
	{
		bPayloadPocket = FindObj( pSoldier, TANK_SHELL );
		if (bPayloadPocket == NO_SLOT)
		{
			return;	// no grenades, can't fire the GLAUNCHER
		}
		ubSafetyMargin = Explosive[ GCM->getItem(TANK_SHELL)->getClassIndex() ].ubRadius;

	}
	else
	{
		// else it's a plain old grenade, now in his hand
		bPayloadPocket = HANDPOS;
		ubSafetyMargin = Explosive[ GCM->getItem(pSoldier->inv[ bPayloadPocket ].usItem)->getClassIndex() ].ubRadius;
		usGrenade = pSoldier->inv[ bPayloadPocket ].usItem;

		if (usGrenade == BREAK_LIGHT)
		{
			// JA2Gold: light isn't as nasty as explosives
			ubSafetyMargin /= 2;
		}
	}

	ubDiff = SoldierDifficultyLevel( pSoldier );

	// make a list of tiles one's friends are positioned in
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const pFriend = *i;

		if (pFriend->bLife == 0)
		{
			continue;
		}

		// if this man is neutral / NOT on the same side, he's not a friend
		if (pFriend->bNeutral || (pSoldier->bSide != pFriend->bSide))
		{
			continue;          // next soldier
		}

		// active friend, remember where he is so that we DON'T blow him up!
		// this includes US, since we don't want to blow OURSELVES up either
		sFriendTile[ubFriendCnt] = pFriend->sGridNo;
		bFriendLevel[ubFriendCnt] = pFriend->bLevel;
		ubFriendCnt++;
	}

	// make a list of tiles one's CURRENTLY SEEN opponents are positioned in
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		if (!pOpponent->bLife)
		{
			continue;          // next soldier
		}

		/*
		// if this soldier is inactive, at base, on assignment, or dead
		if (!pOpponent->bActive || !pOpponent->bInSector || !pOpponent->bLife)
			continue;          // next soldier
		*/

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
		{
			continue;          // next soldier
		}

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY)
		{
			continue;  // next opponent
		}


		bPersOL = pSoldier->bOppList[pOpponent->ubID];

		if ((usInHand == MORTAR) || (usInHand == GLAUNCHER))
		{
			bPublOL = gbPublicOpplist[pSoldier->bTeam][pOpponent->ubID];
			// allow long range firing, where target doesn't PERSONALLY see opponent
			if ((bPersOL != SEEN_CURRENTLY) && (bPublOL != SEEN_CURRENTLY))
			{
				continue;          // next soldier
			}
			// active KNOWN opponent, remember where he is so that we DO blow him up!
			sOpponentTile[ubOpponentCnt] = pOpponent->sGridNo;
			bOpponentLevel[ubOpponentCnt] = pOpponent->bLevel;
		}
		else
		{
			/*
			if (bPersOL != SEEN_CURRENTLY && bPersOL != SEEN_LAST_TURN)     // if not in sight right now
			{
				continue;          // next soldier
			}
			*/
			if (bPersOL == SEEN_CURRENTLY)
			{
				// active KNOWN opponent, remember where he is so that we DO blow him up!
				sOpponentTile[ubOpponentCnt] = pOpponent->sGridNo;
				bOpponentLevel[ubOpponentCnt] = pOpponent->bLevel;
			}
			else if (bPersOL == SEEN_LAST_TURN)
			{
				// cheat; only allow throw if person is REALLY within 2 tiles of where last seen

				// JA2Gold: UB checks were screwed up
				/*
				if ( pOpponent->sGridNo == gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] )
				{
					continue;
				}
				else */
				if ( !CloseEnoughForGrenadeToss( pOpponent->sGridNo, gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] ) )
				{
					continue;
				}

				sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ];
				bOpponentLevel[ubOpponentCnt] = gbLastKnownOppLevel[ pSoldier->ubID ][ pOpponent->ubID ];

				// JA2Gold: commented out
				/*
				if ( SpacesAway( pOpponent->sGridNo, gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] ) > 2 )
				{
					continue;
				}
				sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ];
				bOpponentLevel[ubOpponentCnt] = gbLastKnownOppLevel[ pSoldier->ubID ][ pOpponent->ubID ];
				*/
			}
			else if (bPersOL == HEARD_LAST_TURN )
			{
				// cheat; only allow throw if person is REALLY within 2 tiles of where last seen

				// screen out some ppl who have thrown
				if ( PreRandom( 3 ) == 0 )
				{
					continue;
				}

				// Weird detail: if the opponent is in the same location then they may have closed a door on us.
				// In which case, don't throw!

				// JA2Gold: UB checks were screwed up
				/*
				if ( pOpponent->sGridNo == gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] )
				{
					continue;
				}
				else
				*/
				if ( !CloseEnoughForGrenadeToss( pOpponent->sGridNo, gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ] ) )
				{
					continue;
				}
				if ( usGrenade != BREAK_LIGHT && !pSoldier->bUnderFire && pSoldier->bShock == 0 )
				{
					continue;
				}
				sOpponentTile[ubOpponentCnt] = gsLastKnownOppLoc[ pSoldier->ubID ][ pOpponent->ubID ];
				bOpponentLevel[ubOpponentCnt] = gbLastKnownOppLevel[ pSoldier->ubID ][ pOpponent->ubID ];
			}
			else
			{
				continue;
			}

		}


		// also remember who he is (which soldier #)
		opponents[ubOpponentCnt] = pOpponent;

		// remember how relatively dangerous this opponent is (ignore my cover)
		iOppThreatValue[ubOpponentCnt] = CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier);

		ubOpponentCnt++;
	}

	// this is try to minimize enemies wasting their (limited) toss attacks, with the exception of break lights
	if (usGrenade != BREAK_LIGHT)
	{
		switch( ubDiff )
		{
			case 0:
			case 1:
				// they won't use them until they have 2+ opponents as long as half life left
				if ((ubOpponentCnt < 2) && (pSoldier->bLife > (pSoldier->bLifeMax / 2)))
				{
					return;
				}
				break;
			case 2:
				// they won't use them until they have 2+ opponents as long as 3/4 life left
				if ((ubOpponentCnt < 2) && (pSoldier->bLife > (pSoldier->bLifeMax / 4) * 3 ))
				{
					return;
				}
				break;

			default:
				break;
		}
	}

	InitAttackType(pBestThrow);     // set all structure fields to defaults

	// look at the squares near each known opponent and try to find the one
	// place where a tossed projectile would do the most harm to the opponents
	// while avoiding one's friends
	for (UINT8 ubLoop = 0; ubLoop < ubOpponentCnt; ++ubLoop)
	{
		// search all tiles within 2 squares of this opponent
		ubSearchRange = MAX_TOSS_SEARCH_DIST;

		// determine maximum horizontal limits
		//bMaxLeft  = std::min(ubSearchRange,(sOpponentTile[ubLoop] % MAXCOL));
		bMaxLeft = ubSearchRange;
		//bMaxRight = std::min(ubSearchRange,MAXCOL - ((sOpponentTile[ubLoop] % MAXCOL) + 1));
		bMaxRight = ubSearchRange;

		// determine maximum vertical limits
		bMaxUp   = ubSearchRange;
		bMaxDown = ubSearchRange;

		// evaluate every tile for its opponent-damaging potential
		for (bYOffset = -bMaxUp; bYOffset <= bMaxDown; bYOffset++)
		{
			for (bXOffset = -bMaxLeft; bXOffset <= bMaxRight; bXOffset++)
			{
				//HandleMyMouseCursor(KEYBOARDALSO);

				// calculate the next potential gridno near this opponent
				sGridNo = sOpponentTile[ubLoop] + bXOffset + (MAXCOL * bYOffset);

				// this shouldn't ever happen
				if ((sGridNo < 0) || (sGridNo >= GRIDSIZE))
				{
					continue;
				}

				if ( PythSpacesAway( pSoldier->sGridNo, sGridNo ) > iTossRange )
				{
					// can't throw there!
					continue;
				}

				// if considering a gas/smoke grenade, check to see if there is such stuff already there!
				if ( usGrenade != NOTHING )
				{
					switch( usGrenade.inner() )
					{
						case SMOKE_GRENADE.inner():
						case GL_SMOKE_GRENADE.inner():
							// skip smoke
							if ( gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & MAPELEMENT_EXT_SMOKE )
							{
								continue;
							}
							break;
						case TEARGAS_GRENADE.inner():
							// skip tear and mustard gas
							if ( gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & (MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS) )
							{
								continue;
							}
							break;
						case MUSTARD_GRENADE.inner():
							// skip mustard gas
							if ( gpWorldLevelData[ sGridNo ].ubExtFlags[ bOpponentLevel[ubLoop] ] & MAPELEMENT_EXT_MUSTARDGAS )
							{
								continue;
							}
							break;
						default:
							break;
					}
				}

				fSkipLocation = FALSE;
				// Check to see if we have considered this tile before:
				for (ubLoop2 = 0; ubLoop2 < ubNumExcludedTiles; ubLoop2++)
				{
					if (sExcludeTile[ubLoop2] == sGridNo)
					{
						// already checked!
						fSkipLocation = TRUE;
						break;
					}
				}
				if (fSkipLocation)
				{
					continue;
				}

				// calculate minimum action points required to throw at this gridno
				ubMinAPcost = MinAPsToAttack(pSoldier,sGridNo,ADDTURNCOST);

				// if we don't have enough APs left to throw even without aiming
				if (ubMinAPcost > pSoldier->bActionPoints)
					continue;              // next gridno

				// check whether there are any friends standing near this gridno
				fFriendsNearby = FALSE;

				for (ubLoop2 = 0; ubLoop2 < ubFriendCnt; ubLoop2++)
				{
					if ( (bFriendLevel[ubLoop2] == bOpponentLevel[ubLoop]) && ( PythSpacesAway(sFriendTile[ubLoop2],sGridNo) <= ubSafetyMargin ) )
					{
						fFriendsNearby = TRUE;
						break;       // don't bother checking any other friends
					}
				}

				if (fFriendsNearby)
					continue;      // this location is no good, move along now

				// Well this place shows some promise, evaluate its "damage potential"
				iTotalThreatValue = 0;
				ubOppsInRange = 0;
				ubOppsAdjacent = 0;
				// skip this location unless it's right on top of an enemy or
				// adjacent to more than 1
				fSkipLocation = TRUE;

				for (ubLoop2 = 0; ubLoop2 < ubOpponentCnt; ubLoop2++)
				{
					usOppDist = PythSpacesAway( sOpponentTile[ubLoop2], sGridNo );

					// if this opponent is close enough to the target gridno
					if (usOppDist <= 3)
					{
						// start with this opponents base threat value
						iThreatValue = iOppThreatValue[ubLoop2];

						// estimate how much damage this tossed item would do to him
						iEstDamage = EstimateThrowDamage(pSoldier, bPayloadPocket, opponents[ubLoop2], sGridNo);

						if (usOppDist)
						{
							// reduce the estimated damage for his distance from gridno
							// use 100% at range 0, 80% at range 1, and 60% at range 2, etc.
							iEstDamage = (iEstDamage * (100 - (20 * usOppDist))) / 100;
						}
						else
						{
							// throwing right on top of someone... always consider this
							fSkipLocation = FALSE;
						}

						// add the product of his threat value & damage caused to total
						iTotalThreatValue += (iThreatValue * iEstDamage);

						// only count opponents still standing worth shooting at (in range)
						if (opponents[ubLoop2]->bLife >= OKLIFE)
						{
							ubOppsInRange++;
							if (usOppDist < 2)
							{
								ubOppsAdjacent++;
								if (ubOppsAdjacent > 1 || usGrenade == BREAK_LIGHT)
								{
									fSkipLocation = FALSE;
									// add to exclusion list so we don't consider it again
								}
							}
						}
					}
				}

				// JA2Gold
				if( gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY )
				{
					if (fSkipLocation)
					{
						continue;
					}
				}
				else
				{
					if( ubOppsInRange == 0 )
					{
						continue;
					}

					//Only use it if we are in a surface sector ( basement will be hard enough, plus more chances of mercs being clumped together )
					else if (gWorldSector.z > 0 && fSkipLocation)
					{
						continue;
					}
				}

				// JA2Gold change to >=
				if (ubOppsAdjacent >= 1 && ubNumExcludedTiles < 100)
				{
					// add to exclusion list so we don't calculate for this location twice
					sExcludeTile[ubNumExcludedTiles] = sGridNo;
					ubNumExcludedTiles++;
				}

				// calculate chance to get through any cover to this gridno
				//ubChanceToGetThrough = ChanceToGetThrough(pSoldier,sGridNo,NOTFAKE,ACTUAL,TESTWALLS,9999,M9PISTOL,NOT_FOR_LOS);

				if ( EXPLOSIVE_GUN( usInHand ) )
				{
					ubChanceToGetThrough = AISoldierToLocationChanceToGetThrough( pSoldier, sGridNo, bOpponentLevel[ubLoop], 0 );
					if ( ubChanceToGetThrough == 0)
					{
						continue; // next gridno
					}
				}
				else
				{
					ubChanceToGetThrough = 100 * CalculateLaunchItemChanceToGetThrough( pSoldier, &(pSoldier->inv[ HANDPOS ] ),
									sGridNo, bOpponentLevel[ubLoop], 0, &sEndGridNo, TRUE, &bEndLevel, FALSE );
					// if we can't possibly get through all the cover
					if (ubChanceToGetThrough == 0 )
					{
						if ( bEndLevel == bOpponentLevel[ubLoop] && ubSafetyMargin > 1 )
						{
							// rate "chance of hitting" according to how far away this is from the target
							// but keeping in mind that we don't want to hit far, subtract 1 from the radius here
							// to penalize being far from the target
							uiPenalty = 100 * PythSpacesAway( sGridNo, sEndGridNo ) / (ubSafetyMargin - 1);
							if ( uiPenalty < 100 )
							{
								ubChanceToGetThrough = 100 - (UINT8) uiPenalty;
							}
							else
							{
								continue;
							}
						}
						else
						{
							continue;
						}
					}
				}

				// this is try to minimize enemies wasting their (few) mortar shells or LAWs
				// they won't use them on less than 2 targets as long as half life left
				if ((usInHand == MORTAR || usInHand == ROCKET_LAUNCHER) && (ubOppsInRange < 2) &&
					(pSoldier->bLife > (pSoldier->bLifeMax / 2)))
				{
					continue;              // next gridno
				}

				// calculate the maximum possible aiming time
				ubMaxPossibleAimTime = std::min(AP_MAX_AIM_ATTACK,pSoldier->bActionPoints - ubMinAPcost);

				// calc next attack's minimum AP cost (excludes readying & turning)

				// since grenades & shells are far too valuable to waste, ALWAYS
				// aim for the longest time possible!

				if ( EXPLOSIVE_GUN( usInHand ) )
				{
					ubRawAPCost   = MinAPsToShootOrStab(*pSoldier, sGridNo, FALSE);
					ubChanceToHit = (UINT8) AICalcChanceToHitGun(pSoldier, sGridNo, ubMaxPossibleAimTime, AIM_SHOT_TORSO );
				}
				else
				{
					// NB grenade launcher is NOT a direct fire weapon!
					ubRawAPCost   = MinAPsToThrow(*pSoldier, sGridNo, FALSE);
					ubChanceToHit = (UINT8) CalcThrownChanceToHit( pSoldier, sGridNo, ubMaxPossibleAimTime, AIM_SHOT_TORSO );
				}

				// mortars are inherently quite inaccurate, don't get proximity bonus
				// rockets can go right past people too so...
				if (usInHand != MORTAR && EXPLOSIVE_GUN( usInHand ) )
				{
					// special 50% to Hit bonus: this reflects that even if a tossed item
					// misses by a bit, it's still likely to affect the intended target(s)
					ubChanceToHit += (ubChanceToHit / 2);

					// still can't let it go over 100% chance, though
					if (ubChanceToHit > 100)
					{
						ubChanceToHit = 100;
					}
				}

				iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubMaxPossibleAimTime);

				// calculate chance to REALLY hit: throw accurately AND get past cover
				ubChanceToReallyHit = (ubChanceToHit * ubChanceToGetThrough) / 100;

				// if we can't REALLY hit at all
				if (ubChanceToReallyHit == 0)
					continue;              // next gridno

				// calculate the combined "attack value" for this opponent
				// maximum possible attack value here should be about 140 million
				// typical attack value here should be about 500 thousand
				iAttackValue = (iHitRate * ubChanceToReallyHit * iTotalThreatValue) / 1000;

				// unlike SHOOTing and STABbing, find strictly the highest attackValue
				if (iAttackValue > pBestThrow->iAttackValue)
				{
					SLOGD("CalcBestThrow: new best attackValue vs {} = {}\n", opponents[ubLoop]->ubID, iAttackValue);

					// OOOF!  That was a lot of work!  But we've got a new best target!
					pBestThrow->ubPossible           = TRUE;
					pBestThrow->opponent            = opponents[ubLoop];
					pBestThrow->ubAimTime           = ubMaxPossibleAimTime;
					pBestThrow->ubChanceToReallyHit = ubChanceToReallyHit;
					pBestThrow->sTarget             = sGridNo;
					pBestThrow->iAttackValue        = iAttackValue;
					pBestThrow->ubAPCost            = ubMinAPcost + ubMaxPossibleAimTime;
					pBestThrow->bTargetLevel				= bOpponentLevel[ubLoop];

				}
			}
		}
	}

	// this is try to minimize enemies wasting their (limited) toss attacks:
	switch( ubDiff )
	{
		case 0:
		case 1:
			// don't use unless have a 70% chance to hit
			if (pBestThrow->ubChanceToReallyHit < 70)
			{
				pBestThrow->ubPossible = FALSE;
			}
			break;
		case 2:
			// don't use unless have a 50% chance to hit
			if (pBestThrow->ubChanceToReallyHit < 50)
			{
				pBestThrow->ubPossible = FALSE;
			}
			break;
		default:
			break;
	}

}


static INT32 EstimateStabDamage(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubChanceToHit, BOOLEAN fBladeAttack);


void CalcBestStab(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab, BOOLEAN fBladeAttack )
{
	INT32 iAttackValue;
	INT32 iThreatValue,iHitRate,iBestHitRate,iPercentBetter, iEstDamage;
	BOOLEAN fSurpriseStab;
	UINT8 ubRawAPCost,ubMinAPCost,ubMaxPossibleAimTime,ubAimTime;
	UINT8 ubChanceToHit,ubChanceToReallyHit,ubBestChanceToHit = 0;
	UINT16 usTrueMovementMode;

	UINT8 ubBestAimTime = (UINT8)-1; // XXX HACK000E

	InitAttackType(pBestStab);      // set all structure fields to defaults

	pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;

	// temporarily make this guy run so we get a proper AP cost value
	// from CalcTotalAPsToAttack
	usTrueMovementMode = pSoldier->usUIMovementMode;
	pSoldier->usUIMovementMode = RUNNING;

	// determine which attack against which target has the greatest attack value

	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent->bLife) continue; // next merc

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide) )
			continue;          // next merc

		// if this opponent is not currently in sight (ignore known but unseen!)
		if (pSoldier->bOppList[pOpponent->ubID] != SEEN_CURRENTLY)
			continue;          // next merc

		// if this opponent is not on the same level
		if (pSoldier->bLevel != pOpponent->bLevel)
			continue;          // next merc

		// Special stuff for Carmen the bounty hunter
		if (pSoldier->bAttitude == ATTACKSLAYONLY && pOpponent->ubProfile != SLAY)
			continue;  // next opponent

		// calculate minimum action points required to stab at this opponent
		ubMinAPCost = CalcTotalAPsToAttack( pSoldier,pOpponent->sGridNo,ADDTURNCOST, 0 );

		//ubMinAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo,ADDTURNCOST);

		// Human: if I don't have enough APs left to get there & stab at this guy, skip 'im.
		// Monster:  I'll do an extra check later on to see if I can reach the guy this turn.

		// if 0 is returned then no path!
		if ( ubMinAPCost > pSoldier->bActionPoints || ubMinAPCost == 0 )
		{
			continue;
			/*
			if ( CREATURE_OR_BLOODCAT( pSoldier ) )
			{
				// hardcode ubMinAPCost so that aiming time is 0 and can start move to stab
				// at any time
			ubMinAPCost = pSoldier->bActionPoints;
			}
			else
			{
			continue;          // next merc
			}
			*/
		}

		// calc next attack's minimum stabbing cost (excludes movement & turning)
		ubRawAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo, FALSE) - AP_CHANGE_TARGET;

		// determine if this is a surprise stab (must be next to opponent & unseen)
		fSurpriseStab = FALSE;        // assume it is not a surprise stab

		// if opponent doesn't see the attacker
		if (pOpponent->bOppList[pSoldier->ubID] != SEEN_CURRENTLY)
		{
			fSurpriseStab = TRUE;

			// and he's only one space away from attacker
			if (SpacesAway(pSoldier->sGridNo,pOpponent->sGridNo) == 1)
			{
				fSurpriseStab = TRUE;   // we got 'im lined up where we want 'im!
			}
		}


		iBestHitRate = 0;                     // reset best hit rate to minimum

		// calculate the maximum possible aiming time
		ubMaxPossibleAimTime = std::min(AP_MAX_AIM_ATTACK,pSoldier->bActionPoints - ubMinAPCost);

		// consider the various aiming times
		for (ubAimTime = AP_MIN_AIM_ATTACK; ubAimTime <= ubMaxPossibleAimTime; ubAimTime++)
		{
			//HandleMyMouseCursor(KEYBOARDALSO);
			if (!fSurpriseStab)
			{
				if (fBladeAttack)
				{
					ubChanceToHit = (UINT8) CalcChanceToStab(pSoldier,pOpponent,ubAimTime);
				}
				else
				{
					ubChanceToHit = (UINT8) CalcChanceToPunch(pSoldier,pOpponent,ubAimTime);
				}
			}
			else
				ubChanceToHit = MAXCHANCETOHIT;

			iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime);

			// if aiming for this amount of time produces a better hit rate
			if (iHitRate > iBestHitRate)
			{
				iBestHitRate = iHitRate;
				ubBestAimTime = ubAimTime;
				ubBestChanceToHit = ubChanceToHit;
			}
		}


		// if we can't get any kind of hit rate at all
		if (iBestHitRate == 0)
			continue;          // next opponent

		// stabs are not affected by cover, so the chance to REALLY hit is the same
		ubChanceToReallyHit = ubBestChanceToHit;

		// calculate this opponent's threat value
		// NOTE: ignore my cover!  By the time I run beside him I won't have any!
		iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier);


		// estimate the damage this stab would do to this opponent
		iEstDamage = EstimateStabDamage(pSoldier,pOpponent,ubBestChanceToHit, fBladeAttack );

		// calculate the combined "attack value" for this opponent
		// highest possible value before division should be about 1 billion...
		// normal value before division should be about 5 million...
		iAttackValue = ( iEstDamage * iBestHitRate * ubChanceToReallyHit * iThreatValue) / 1000;

		// if we can hurt the guy, OR probably not, but at least it's our best
		// chance to actually hit him and maybe scare him, knock him down, etc.
		if ((iAttackValue > 0) || (ubChanceToReallyHit > pBestStab->ubChanceToReallyHit))
		{
			// if there already was another viable target
			if (pBestStab->ubChanceToReallyHit > 0)
			{
				// OK, how does our chance to hit him compare to the previous best one?
				iPercentBetter = ((ubChanceToReallyHit * 100) / pBestStab->ubChanceToReallyHit) - 100;

				// if this chance to really hit is more than 50% worse, and the other
				// guy is conscious at all
				if (iPercentBetter < -PERCENT_TO_IGNORE_THREAT && pBestStab->opponent->bLife >= OKLIFE)
					// then stick with the older guy as the better target
					continue;

				// if this chance to really hit between 50% worse to 50% better
				if (iPercentBetter < PERCENT_TO_IGNORE_THREAT)
				{
					// then the one with the higher ATTACK VALUE is the better target
					if (iAttackValue < pBestStab->iAttackValue)
						// the previous guy is more important since he's more dangerous
						continue;            // next opponent
				}
			}

			// OOOF!  That was a lot of work!  But we've got a new best target!
			pBestStab->ubPossible          = TRUE;
			pBestStab->opponent            = pOpponent;
			pBestStab->ubAimTime           = ubBestAimTime;
			pBestStab->ubChanceToReallyHit = ubChanceToReallyHit;
			pBestStab->sTarget             = pOpponent->sGridNo;
			pBestStab->bTargetLevel        = pOpponent->bLevel;
			pBestStab->iAttackValue        = iAttackValue;
			pBestStab->ubAPCost            = ubMinAPCost + ubBestAimTime;
		}
	}

	pSoldier->usUIMovementMode = usTrueMovementMode;
}

void CalcTentacleAttack(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab )
{
	INT32 iAttackValue;
	INT32 iThreatValue,iHitRate,iBestHitRate, iEstDamage;
	BOOLEAN fSurpriseStab;
	UINT8 ubRawAPCost,ubMinAPCost,ubMaxPossibleAimTime,ubAimTime;
	UINT8 ubChanceToHit,ubChanceToReallyHit,ubBestChanceToHit = 0;

	UINT8 ubBestAimTime = (UINT8)-1; // XXX HACK000E

	InitAttackType(pBestStab);      // set all structure fields to defaults

	pSoldier->usAttackingWeapon = pSoldier->inv[HANDPOS].usItem;

	// determine which attack against which target has the greatest attack value
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const pOpponent = *i;

		// if this merc is inactive, at base, on assignment, or dead
		if (!pOpponent->bLife) continue; // next merc

		// if this man is neutral / on the same side, he's not an opponent
		if ( CONSIDERED_NEUTRAL( pSoldier, pOpponent ) || (pSoldier->bSide == pOpponent->bSide))
			continue;          // next merc

		// if this opponent is not currently in sight (ignore known but unseen!)
		if (pSoldier->bOppList[pOpponent->ubID] != SEEN_CURRENTLY)
			continue;          // next merc

		// if this opponent is not on the same level
		if (pSoldier->bLevel != pOpponent->bLevel)
			continue;          // next merc

		// if this opponent is outside the range of our tentacles
		if (GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pOpponent->sGridNo ) > GCM->getWeapon( CREATURE_QUEEN_TENTACLES)->usRange)
		{
			continue; // next merc
		}

		// calculate minimum action points required to stab at this opponent
		ubMinAPCost = CalcTotalAPsToAttack( pSoldier,pOpponent->sGridNo,ADDTURNCOST, 0 );
		//ubMinAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo,ADDTURNCOST);


		// calc next attack's minimum stabbing cost (excludes movement & turning)
		ubRawAPCost = MinAPsToAttack(pSoldier,pOpponent->sGridNo, FALSE) - AP_CHANGE_TARGET;

		// determine if this is a surprise stab (for tentacles, enemy must not see us, no dist limit)
		fSurpriseStab = FALSE;        // assume it is not a surprise stab

		// if opponent doesn't see the attacker
		if (pOpponent->bOppList[pSoldier->ubID] != SEEN_CURRENTLY)
		{
			fSurpriseStab = TRUE;   // we got 'im lined up where we want 'im!
		}

		iBestHitRate = 0;                     // reset best hit rate to minimum

		// calculate the maximum possible aiming time

		//ubMaxPossibleAimTime = std::min(AP_MAX_AIM_ATTACK,pSoldier->bActionPoints - ubMinAPCost);
		ubMaxPossibleAimTime = 0;

		// consider the various aiming times
		for (ubAimTime = AP_MIN_AIM_ATTACK; ubAimTime <= ubMaxPossibleAimTime; ubAimTime++)
		{
			//HandleMyMouseCursor(KEYBOARDALSO);
			if (!fSurpriseStab)
			{
				ubChanceToHit = (UINT8) CalcChanceToStab(pSoldier,pOpponent,ubAimTime);
			}
			else
				ubChanceToHit = MAXCHANCETOHIT;

			iHitRate = (pSoldier->bActionPoints * ubChanceToHit) / (ubRawAPCost + ubAimTime);

			// if aiming for this amount of time produces a better hit rate
			if (iHitRate > iBestHitRate)
			{
				iBestHitRate = iHitRate;
				ubBestAimTime = ubAimTime;
				ubBestChanceToHit = ubChanceToHit;
			}
		}

		// if we can't get any kind of hit rate at all
		if (iBestHitRate == 0)
			continue;          // next opponent

		// stabs are not affected by cover, so the chance to REALLY hit is the same
		ubChanceToReallyHit = ubBestChanceToHit;

		// calculate this opponent's threat value
		// NOTE: ignore my cover!  By the time I run beside him I won't have any!
		iThreatValue = CalcManThreatValue(pOpponent,pSoldier->sGridNo,FALSE,pSoldier);

		// estimate the damage this stab would do to this opponent
		iEstDamage = EstimateStabDamage(pSoldier,pOpponent,ubBestChanceToHit, TRUE );

		// calculate the combined "attack value" for this opponent
		// highest possible value before division should be about 1 billion...
		// normal value before division should be about 5 million...
		iAttackValue = ( iEstDamage * iBestHitRate * ubChanceToReallyHit * iThreatValue) / 1000;

		// if we can hurt the guy, OR probably not, but at least it's our best
		// chance to actually hit him and maybe scare him, knock him down, etc.
		if (iAttackValue > 0)
		{
			// OOOF!  That was a lot of work!  But we've got a new best target!
			pBestStab->ubPossible          = TRUE;
			pBestStab->opponent            = pOpponent;
			pBestStab->ubAimTime           = ubBestAimTime;
			pBestStab->ubChanceToReallyHit = ubChanceToReallyHit;
			pBestStab->sTarget             = pOpponent->sGridNo;
			pBestStab->bTargetLevel        = pOpponent->bLevel;

			// ADD this target's attack value to our TOTAL...
			pBestStab->iAttackValue				+= iAttackValue;

			pBestStab->ubAPCost            = ubMinAPCost + ubBestAimTime;

		}
	}
}


static UINT8 NumMercsCloseTo(INT16 sGridNo, UINT8 ubMaxDist)
{
	INT8 bNumber = 0;
	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const s = *i;
		if (s->bTeam == OUR_TEAM &&
			s->bLife >= OKLIFE      &&
			PythSpacesAway(sGridNo, s->sGridNo) <= ubMaxDist)
		{
			++bNumber;
		}
	}
	return bNumber;
}


static INT32 EstimateShotDamage(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubChanceToHit)
{
	INT32 iRange,iMaxRange,iPowerLost;
	INT32 iDamage;
	UINT8 ubBonus;
	INT32 iHeadProt = 0, iTorsoProt = 0, iLegProt = 0;
	INT32 iTotalProt;
	INT8 bPlatePos;
	UINT8	ubAmmoType;

	/*
	if ( pOpponent->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// only thing that can damage vehicles is HEAP rounds?
		return( 0 );
	}
	*/

	if ( GCM->getItem(pSoldier->inv[pSoldier->ubAttackingHand].usItem)->isThrowingKnife() )
	{
		ubAmmoType = AMMO_KNIFE;
	}
	else
	{
		ubAmmoType = pSoldier->inv[pSoldier->ubAttackingHand].ubGunAmmoType;
	}

	// calculate distance to target, obtain his gun's maximum range rating

	iRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, pOpponent->sGridNo );
	iMaxRange = GCM->getWeapon(pSoldier->inv[HANDPOS].usItem)->usRange;

	// bullet loses speed and penetrating power, 50% loss per maximum range
	iPowerLost = ((50 * iRange) / iMaxRange);

	// up to 50% extra impact for making particularly accurate successful shots
	ubBonus = ubChanceToHit / 4;       // /4 is really /2 and /2 again

	iDamage = (GCM->getWeapon(pSoldier->inv[HANDPOS].usItem)->ubImpact *
						(100 - iPowerLost + ubBonus)) / 100;

	// if opponent is wearing a helmet
	if (pOpponent->inv[HELMETPOS].usItem != NOTHING)
	{
		iHeadProt += (INT32) Armour[GCM->getItem(pOpponent->inv[HELMETPOS].usItem)->getClassIndex()].ubProtection *
				(INT32) pOpponent->inv[HELMETPOS].bStatus[0] / 100;
	}

	// if opponent is wearing a protective vest
	if (ubAmmoType != AMMO_MONSTER && ubAmmoType != AMMO_KNIFE)
	{
		// monster spit and knives ignore kevlar vests
		if (pOpponent->inv[VESTPOS].usItem != NOTHING)
		{
			iTorsoProt += (INT32) Armour[GCM->getItem(pOpponent->inv[VESTPOS].usItem)->getClassIndex()].ubProtection *
					(INT32) pOpponent->inv[VESTPOS].bStatus[0] / 100;
		}
	}

	// check for ceramic plates; these do affect monster spit
	bPlatePos = FindAttachment( &(pOpponent->inv[VESTPOS]), CERAMIC_PLATES );
	if (bPlatePos != -1)
	{
		iTorsoProt += (INT32) Armour[GCM->getItem(pOpponent->inv[VESTPOS].usAttachItem[bPlatePos])->getClassIndex()].ubProtection *
				(INT32) pOpponent->inv[VESTPOS].bAttachStatus[bPlatePos] / 100;
	}


	// if opponent is wearing armoured leggings (LEGPOS)
	if (ubAmmoType != AMMO_MONSTER && ubAmmoType != AMMO_KNIFE)
	{	// monster spit and knives ignore kevlar leggings
		if (pOpponent->inv[LEGPOS].usItem != NOTHING)
		{
			iLegProt += (INT32) Armour[GCM->getItem(pOpponent->inv[LEGPOS].usItem)->getClassIndex()].ubProtection *
					(INT32) pOpponent->inv[LEGPOS].bStatus[0] / 100;
		}
	}

	// 15% of all shots are to the head, 80% are to the torso.  Calc. avg. prot.
	// NB: make AI guys shoot at head 15% of time, 5% of time at legs

	iTotalProt = ((15 * iHeadProt) + (75 * iTorsoProt) + 5 * iLegProt) / 100;
	switch (ubAmmoType)
	{
		case AMMO_HP:
			iTotalProt = AMMO_ARMOUR_ADJUSTMENT_HP( iTotalProt );
			break;
		case AMMO_AP:
			iTotalProt = AMMO_ARMOUR_ADJUSTMENT_AP( iTotalProt );
			break;
		case AMMO_SUPER_AP:
			iTotalProt = AMMO_ARMOUR_ADJUSTMENT_SAP( iTotalProt );
			break;
		default:
			break;
	}

	iDamage -= iTotalProt;
	if (ubAmmoType == AMMO_HP)
	{
		// increase after-armour damage
		iDamage = AMMO_DAMAGE_ADJUSTMENT_HP( iDamage );
	}

	if (ubAmmoType == AMMO_MONSTER)
	{
		// cheat and emphasize shots
		//iDamage = (iDamage * 15) / 10;
		ItemId gas;
		switch (pSoldier->inv[pSoldier->ubAttackingHand].usItem.inner())
		{
			// explosive damage is 100-200% that of the rated, so multiply by 3/2s here
			case CREATURE_QUEEN_SPIT.inner():    gas = LARGE_CREATURE_GAS;      break;
			case CREATURE_OLD_MALE_SPIT.inner(): gas = SMALL_CREATURE_GAS;      break;
			default:                     gas = VERY_SMALL_CREATURE_GAS; break;
		}
		const EXPLOSIVETYPE* const e = &Explosive[GCM->getItem(gas)->getClassIndex()];
		iDamage += e->ubDamage * NumMercsCloseTo(pOpponent->sGridNo, e->ubRadius) * 3 / 2;
	}

	if (iDamage < 1)
		iDamage = 1;  // assume we can do at LEAST 1 pt minimum damage

	return( iDamage );
}


static INT32 EstimateThrowDamage(SOLDIERTYPE* pSoldier, UINT8 ubItemPos, SOLDIERTYPE* pOpponent, INT16 sGridno)
{
	UINT8 ubExplosiveIndex;
	INT32 iExplosDamage, iBreathDamage, iArmourAmount, iDamage = 0;
	INT8  bSlot;

	switch ( pSoldier->inv[ ubItemPos ].usItem.inner() )
	{
		case GL_SMOKE_GRENADE.inner():
		case SMOKE_GRENADE.inner():
			// Don't want to value throwing smoke very much.  This value is based relative
			// to the value for knocking somebody down, which was giving values that were
			// too high
			return( 5 );
		case ROCKET_LAUNCHER.inner():
			ubExplosiveIndex = GCM->getItem(C1)->getClassIndex();
			break;
		default:
			ubExplosiveIndex = GCM->getItem(pSoldier->inv[ubItemPos].usItem)->getClassIndex();
			break;
	}

	// JA2Gold: added
	if ( pSoldier->inv[ubItemPos].usItem == BREAK_LIGHT )
	{
		return( 5 * ( LightTrueLevel( pOpponent->sGridNo, pOpponent->bLevel ) - NORMAL_LIGHTLEVEL_DAY ) );
	}


	iExplosDamage = ( ( (INT32) Explosive[ ubExplosiveIndex ].ubDamage ) * 3) / 2;
	iBreathDamage = ( ( (INT32) Explosive[ ubExplosiveIndex ].ubStunDamage ) * 5) / 4;

	if ( Explosive[ ubExplosiveIndex ].ubType == EXPLOSV_TEARGAS || Explosive[ ubExplosiveIndex ].ubType == EXPLOSV_MUSTGAS )
	{
		// if target gridno is outdoors (where tear gas lasts only 1-2 turns)
		if (gpWorldLevelData[sGridno].ubTerrainID != FLAT_FLOOR)
			iBreathDamage /= 2;       // reduce effective breath damage by 1/2

		bSlot = FindObj( pOpponent, GASMASK );
		if (bSlot == HEAD1POS || bSlot == HEAD2POS)
		{
			// take condition of the gas mask into account - it could be leaking
			iBreathDamage = (iBreathDamage * (100 - pOpponent->inv[bSlot].bStatus[0])) / 100;
		}

	}
	else if (iExplosDamage)
	{
		// EXPLOSION DAMAGE is spread amongst locations
		iArmourAmount = ArmourVersusExplosivesPercent( pSoldier );
		iExplosDamage -= iExplosDamage * iArmourAmount / 100;

		if (iExplosDamage < 1)
			iExplosDamage = 1;
	}

	// if this opponent is standing
	if (gAnimControl[ pSoldier->usAnimState ].ubEndHeight == ANIM_STAND)
	{
		// 15 pt. flat bonus for knocking him down (for ANY type of explosion)
		iDamage += 15;
	}

	if ( pOpponent->bBreath < OKBREATH || AM_A_ROBOT( pOpponent ) )
	{
		// don't bother to count breath damage against people already down
		iBreathDamage = 0;
	}

	// estimate combined "damage" value considering combined health/breath damage
	iDamage += iExplosDamage + (iBreathDamage / 3);

	// approximate chance of the grenade going off (Ian's formulas are too funky)
	// then use that to reduce the expected damage because thing may not blow!
	iDamage = (iDamage * pSoldier->inv[ubItemPos].bStatus[0]) / 100;

	return( iDamage);
}


static INT32 EstimateStabDamage(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOpponent, UINT8 ubChanceToHit, BOOLEAN fBladeAttack)
{
	INT32 iImpact, iFluke, iBonus;

	if (fBladeAttack)
	{
		iImpact = GCM->getWeapon( pSoldier->usAttackingWeapon )->ubImpact;
		iImpact += EffectiveStrength( pSoldier ) / 20; // 0 to 5 for strength, adjusted by damage taken
	}
	else
	{
		// NB martial artists don't get a bonus for using brass knuckles!
		if (pSoldier->usAttackingWeapon != NOTHING && !( HAS_SKILL_TRAIT( pSoldier, MARTIALARTS ) ) )
		{
			iImpact = GCM->getWeapon( pSoldier->usAttackingWeapon )->ubImpact;
		}
		else
		{
			// base HTH damage
			iImpact = 5;
		}
		iImpact += EffectiveStrength( pSoldier ) / 5; // 0 to 20 for strength, adjusted by damage taken
	}


	iImpact += (pSoldier->bExpLevel / 2); // 0 to 4 for level

	iFluke = 0;
	iBonus = ubChanceToHit / 4;				// up to 50% extra impact for accurate attacks

	iImpact = iImpact * (100 + iFluke + iBonus) / 100;

	if (!fBladeAttack)
	{
		// add bonuses for hand-to-hand and martial arts
		iImpact = iImpact * (100 + gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pSoldier, MARTIALARTS)) / 100;
		iImpact = iImpact * (100 + gbSkillTraitBonus[HANDTOHAND]  * NUM_SKILL_TRAITS(pSoldier, HANDTOHAND))  / 100;
		// Here, if we're doing a bare-fisted attack,
		// we want to pay attention just to wounds inflicted
		iImpact = iImpact / PUNCH_REAL_DAMAGE_PORTION;
	}

	if (iImpact < 1)
	{
		iImpact = 1;
	}

	return( iImpact );
}


static INT8 TryToReload(SOLDIERTYPE* const s)
{
	OBJECTTYPE& hand = s->inv[HANDPOS];
	const WeaponModel *weapon = GCM->getWeapon(hand.usItem);
	INT8 const slot = FindAmmo(s, weapon->calibre, weapon->ubMagSize, NO_SLOT);
	return slot != NO_SLOT && ReloadGun(s, &hand, &s->inv[slot]) ?
		TRUE : NOSHOOT_NOAMMO;
}


INT8 CanNPCAttack(SOLDIERTYPE *pSoldier)
{
	INT8 bCanAttack;
	INT8 bWeaponIn;

	// NEUTRAL civilians are not allowed to attack, but those that are not
	// neutral (KILLNPC mission guynums, escorted guys) can, if they're armed
	if (IsOnCivTeam(pSoldier) && pSoldier->bNeutral) return FALSE;

	// test if if we are able to attack (in general, not at any specific target)
	bCanAttack = OKToAttack(pSoldier,NOWHERE);

	// if soldier can't attack because he doesn't have a weapon or is out of ammo
	// or his weapon isn't loaded
	if ( bCanAttack == NOSHOOT_NOAMMO ) // || NOLOAD
	{
		// try to reload it
		bCanAttack = TryToReload( pSoldier );
	}
	else if (bCanAttack == NOSHOOT_NOWEAPON)
	{
		// look for another weapon
		bWeaponIn = FindAIUsableObjClass( pSoldier, IC_WEAPON );
		if (bWeaponIn != NO_SLOT)
		{
			RearrangePocket( pSoldier, HANDPOS, bWeaponIn, FOREVER );
			// look for another weapon if this one is 1-handed
			if ( (GCM->getItem(pSoldier->inv[ HANDPOS ].usItem)->getItemClass() == IC_GUN) && !(GCM->getItem(pSoldier->inv[ HANDPOS ].usItem)->isTwoHanded() ) )
			{
				// look for another pistol/SMG if available
				bWeaponIn = FindAIUsableObjClassWithin( pSoldier, IC_WEAPON, BIGPOCK1POS, SMALLPOCK8POS );
				if (bWeaponIn != NO_SLOT && (GCM->getItem(pSoldier->inv[ bWeaponIn ].usItem)->getItemClass() == IC_GUN) && !(GCM->getItem(pSoldier->inv[ bWeaponIn ].usItem)->isTwoHanded() ) )
				{
					RearrangePocket( pSoldier, SECONDHANDPOS, bWeaponIn, FOREVER );
				}
			}
			// might need to reload
			bCanAttack = CanNPCAttack( pSoldier );
		}
	}
	return( bCanAttack );
}

void CheckIfTossPossible(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow)
{
	UINT8 ubMinAPcost;

	if ( TANK( pSoldier ) )
	{
		pBestThrow->bWeaponIn = FindObj( pSoldier, TANK_CANNON );
	}
	else
	{
		pBestThrow->bWeaponIn = FindAIUsableObjClass( pSoldier, IC_LAUNCHER );

		if ( pBestThrow->bWeaponIn == NO_SLOT )
		{
			// Consider rocket launcher/cannon
			pBestThrow->bWeaponIn = FindObj( pSoldier, ROCKET_LAUNCHER );
			if ( pBestThrow->bWeaponIn == NO_SLOT )
			{
				// no rocket launcher, consider grenades
				pBestThrow->bWeaponIn = FindThrowableGrenade( pSoldier );
			}
			else
			{
				// Have rocket launcher... maybe have grenades as well.  which one to use?
				if ( pSoldier->bAIMorale > MORALE_WORRIED && PreRandom( 2 ) )
				{
					pBestThrow->bWeaponIn = FindThrowableGrenade( pSoldier );
				}
			}
		}
	}

	// if the soldier does have a tossable item somewhere
	if (pBestThrow->bWeaponIn != NO_SLOT)
	{
		// if it's in his holster, swap it into his hand temporarily
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			RearrangePocket(pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY);
		}

		// get the minimum cost to attack with this tossable item
		ubMinAPcost = MinAPsToAttack( pSoldier, pSoldier->sLastTarget, DONTADDTURNCOST);

		// if we can afford the minimum AP cost to throw this tossable item
		if (pSoldier->bActionPoints >= ubMinAPcost)
		{
			// then look around for a worthy target (which sets bestThrow.ubPossible)
			CalcBestThrow( pSoldier, pBestThrow );
		}

		// if it was in his holster, swap it back into his holster for now
		if (pBestThrow->bWeaponIn != HANDPOS)
		{
			RearrangePocket( pSoldier, HANDPOS, pBestThrow->bWeaponIn, TEMPORARILY );
		}
	}
}


static INT8 CountAdjacentSpreadTargets(SOLDIERTYPE* pSoldier, INT16 sFirstTarget, INT8 bTargetLevel)
{
	// return the number of people next to this guy for burst-spread purposes

	INT8 bDirLoop, bDir, bCheckDir, bTargetIndex, bTargets;
	INT16 sTarget;
	SOLDIERTYPE *pTargets[5] = {NULL};

	bTargetIndex = -1;
	bCheckDir = -1;

	pTargets[2] = WhoIsThere2(sFirstTarget, bTargetLevel);
	if (pTargets[2] == NULL)
	{
		return( 0 );
	}
	bTargets = 1;

	bDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, sFirstTarget );

	for (bDirLoop = 0; bDirLoop < 8; bDirLoop++)
	{
		if (bDir % 2)
		{
			// odd direction = diagonal direction
			switch( bDirLoop )
			{
				case 0:
					bCheckDir = (bDir + 6) % 8;
					bTargetIndex = 0;
					break;
				case 1:
					bCheckDir = (bDir + 5) % 8;
					bTargetIndex = 1;
					break;
				case 2:
					bCheckDir = (bDir + 7) % 8;
					bTargetIndex = 1;
					break;
				case 3:
					bCheckDir = (bDir + 3) % 8;
					bTargetIndex = 3;
					break;
				case 4:
					bCheckDir = (bDir + 1) % 8;
					bTargetIndex = 3;
					break;
				case 5:
					bCheckDir = (bDir + 2) % 8;
					bTargetIndex = 4;
					break;
				case 6:
					// check in front
					bCheckDir = (bDir + 4) % 8;
					bTargetIndex = 1;
					break;
				case 7:
					// check behind
					bCheckDir = (bDir) % 8;
					bTargetIndex = 3;
					break;
			}
		}
		else
		{
			// even = straight
			switch( bDirLoop )
			{
				case 0:
					bCheckDir = (bDir + 5) % 8;
					bTargetIndex = 1;
					break;
				case 1:
					bCheckDir = (bDir + 6) % 8;
					bTargetIndex = 1;
					break;
				case 2:
					bCheckDir = (bDir + 7) % 8;
					bTargetIndex = 1;
					break;
				case 3:
					bCheckDir = (bDir + 3) % 8;
					bTargetIndex = 3;
					break;
				case 4:
					bCheckDir = (bDir + 2) % 8;
					bTargetIndex = 3;
					break;
				case 5:
					bCheckDir = (bDir + 1) % 8;
					bTargetIndex = 3;
					break;
				case 6:
					// check in front
					bCheckDir = (bDir + 4) % 8;
					bTargetIndex = 1;
					break;
				case 7:
					// check behind
					bCheckDir = (bDir) % 8;
					bTargetIndex = 3;
					break;

			}
		}
		if (bDirLoop == 6 && bTargets > 1)
		{
			// we're done!  otherwise we continue and try to find people in front/behind
			break;
		}
		else if (pTargets[bTargetIndex] != NULL)
		{
			continue;
		}
		sTarget = sFirstTarget + DirIncrementer[bCheckDir];
		SOLDIERTYPE* const pTarget = WhoIsThere2(sTarget, bTargetLevel);
		if (pTarget)
		{
			// check to see if guy is visible
			if (pSoldier->bOppList[ pTarget->ubID ] == SEEN_CURRENTLY)
			{
				pTargets[bTargetIndex] = pTarget;
				bTargets++;
			}
		}
	}
	return( bTargets - 1 );
}

INT16 CalcSpreadBurst( SOLDIERTYPE * pSoldier, INT16 sFirstTarget, INT8 bTargetLevel )
{
	INT8	bDirLoop, bDir, bCheckDir, bTargetIndex = 0, bLoop, bTargets;
	INT16	sTarget;
	SOLDIERTYPE* pTargets[5] = {NULL};
	INT8 bAdjacents, bOtherAdjacents;


	bCheckDir = -1;

	pTargets[2] = WhoIsThere2(sFirstTarget, bTargetLevel);
	if (pTargets[2] == NULL)
	{
		return( sFirstTarget );
	}
	bTargets = 1;
	bAdjacents = CountAdjacentSpreadTargets( pSoldier, sFirstTarget, bTargetLevel );

	bDir = (INT8) GetDirectionToGridNoFromGridNo( pSoldier->sGridNo, sFirstTarget );

	for (bDirLoop = 0; bDirLoop < 8; bDirLoop++)
	{
		if (bDir % 2)
		{
			// odd direction = diagonal direction
			switch( bDirLoop )
			{
				case 0:
					bCheckDir = (bDir + 6) % 8;
					bTargetIndex = 0;
					break;
				case 1:
					bCheckDir = (bDir + 5) % 8;
					bTargetIndex = 1;
					break;
				case 2:
					bCheckDir = (bDir + 7) % 8;
					bTargetIndex = 1;
					break;
				case 3:
					bCheckDir = (bDir + 3) % 8;
					bTargetIndex = 3;
					break;
				case 4:
					bCheckDir = (bDir + 1) % 8;
					bTargetIndex = 3;
					break;
				case 5:
					bCheckDir = (bDir + 2) % 8;
					bTargetIndex = 4;
					break;
				case 6:
					// check in front
					bCheckDir = (bDir + 4) % 8;
					bTargetIndex = 1;
					break;
				case 7:
					// check behind
					bCheckDir = (bDir) % 8;
					bTargetIndex = 3;
					break;
			}
		}
		else
		{
			// even = straight
			switch( bDirLoop )
			{
				case 0:
					bCheckDir = (bDir + 5) % 8;
					bTargetIndex = 1;
					break;
				case 1:
					bCheckDir = (bDir + 6) % 8;
					bTargetIndex = 1;
					break;
				case 2:
					bCheckDir = (bDir + 7) % 8;
					bTargetIndex = 1;
					break;
				case 3:
					bCheckDir = (bDir + 3) % 8;
					bTargetIndex = 3;
					break;
				case 4:
					bCheckDir = (bDir + 2) % 8;
					bTargetIndex = 3;
					break;
				case 5:
					bCheckDir = (bDir + 1) % 8;
					bTargetIndex = 3;
					break;
				case 6:
					// check in front
					bCheckDir = (bDir + 4) % 8;
					bTargetIndex = 1;
					break;
				case 7:
					// check behind
					bCheckDir = (bDir) % 8;
					bTargetIndex = 3;
					break;

			}
		}
		if (bDirLoop == 6 && bTargets > 1)
		{
			// we're done!  otherwise we continue and try to find people in front/behind
			break;
		}
		else if (pTargets[bTargetIndex] != NULL)
		{
			continue;
		}
		sTarget = sFirstTarget + DirIncrementer[bCheckDir];
		SOLDIERTYPE* const pTarget = WhoIsThere2(sTarget, bTargetLevel);
		if (pTarget && pSoldier->bOppList[ pTarget->ubID ] == SEEN_CURRENTLY)
		{
			bOtherAdjacents = CountAdjacentSpreadTargets( pSoldier, sTarget, bTargetLevel );
			if (bOtherAdjacents > bAdjacents)
			{
				// we should do a spread-burst there instead!
				return( CalcSpreadBurst( pSoldier, sTarget, bTargetLevel ) );
			}
			pTargets[bTargetIndex] = pTarget;
			bTargets++;
		}
	}

	if (bTargets > 1)
	{
		// Move all the locations down in the array if necessary
		// Check the 4th position
		if (pTargets[3] == NULL && pTargets[4] != NULL)
		{
			pTargets[3] = pTargets[4];
			pTargets[4] = NULL;
		}
		// Check the first two positions; we know the 3rd value is set because
		// it's our initial target
		if (pTargets[1] == NULL)
		{
			pTargets[1] = pTargets[2];
			pTargets[2] = pTargets[3];
			pTargets[3] = pTargets[4];
			pTargets[4] = NULL;
		}
		if (pTargets[0] == NULL)
		{
			pTargets[0] = pTargets[1];
			pTargets[1] = pTargets[2];
			pTargets[2] = pTargets[3];
			pTargets[3] = pTargets[4];
			pTargets[4] = NULL;
		}
		// now 50% chance to reorganize to fire in reverse order
		if (Random( 2 ))
		{
			for( bLoop = 0; bLoop < bTargets / 2; bLoop++)
			{
				SOLDIERTYPE* const pTarget = pTargets[bLoop];
				pTargets[bLoop] = pTargets[bTargets - 1 - bLoop];
				pTargets[bTargets - 1 - bLoop] = pTarget;
			}
		}
		AIPickBurstLocations( pSoldier, bTargets, pTargets );
		pSoldier->fDoSpread = TRUE;
	}
	return( sFirstTarget );
}

INT16 AdvanceToFiringRange( SOLDIERTYPE * pSoldier, INT16 sClosestOpponent )
{
	// see how far we can go down a path and still shoot

	INT8   bAttackCost, bTrueActionPoints;
	UINT16 usActionData;

	bAttackCost = MinAPsToAttack(pSoldier, sClosestOpponent, ADDTURNCOST);

	if (bAttackCost >= pSoldier->bActionPoints)
	{
		// probably want to go as far as possible!
		// return( NOWHERE );
		return( GoAsFarAsPossibleTowards( pSoldier, sClosestOpponent, AI_ACTION_SEEK_OPPONENT ) );
	}

	bTrueActionPoints = pSoldier->bActionPoints;

	pSoldier->bActionPoints -= bAttackCost;

	usActionData = GoAsFarAsPossibleTowards( pSoldier, sClosestOpponent, AI_ACTION_SEEK_OPPONENT );

	pSoldier->bActionPoints = bTrueActionPoints;

	return( usActionData );
}
