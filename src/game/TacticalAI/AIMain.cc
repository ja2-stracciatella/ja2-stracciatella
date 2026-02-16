#include "Handle_Doors.h"
#include "Structure.h"
#include "Timer_Control.h"
#include "AI.h"
#include "Isometric_Utils.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "OppList.h"
#include "Animation_Control.h"
#include "Interface.h"
#include "PathAI.h"
#include "Points.h"
#include "Items.h"
#include "Handle_Items.h"
#include "AIInternals.h"
#include "Animation_Data.h"
#include "LOS.h"
#include "TeamTurns.h"
#include "NPC.h"
#include "Dialogue_Control.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
#include "Soldier_Create.h"
#include "Explosion_Control.h"
#include "Interactive_Tiles.h"
#include "Interface_Dialogue.h"
#include "Vehicles.h"
#include "RenderWorld.h"
#include "AIList.h"
#include "Soldier_Macros.h"
#include "Bullets.h"
#include "Physics.h"
#include "Interface_Panels.h"
#include "Sound_Control.h"
#include "Civ_Quotes.h"
#include "Quests.h"
#include "Queen_Command.h"


constexpr milliseconds AI_DELAY = 100ms;


//
// Commented out/ to fix:
// lots of other stuff, I think
//

#define DEADLOCK_DELAY							15000
#define AI_LIMIT_PER_UPDATE		1

BOOLEAN gfTurnBasedAI;

INT8 gbDiff[MAX_DIFF_PARMS][5] =
{
	//       AI DIFFICULTY SETTING
	// WIMPY  EASY  NORMAL  TOUGH  ELITE
	{  -20,  -10,     0,    10,     20  },     // DIFF_ENEMY_EQUIP_MOD
	{  -10,   -5,     0,     5,     10  },     // DIFF_ENEMY_TO_HIT_MOD
	{   -2,   -1,     0,     1,      2  },     // DIFF_ENEMY_INTERRUPT_MOD
	{   50,   65,    80,    90,     95  },     // DIFF_RADIO_RED_ALERT
	{    4,    6,     8,    10,     13  }      // DIFF_MAX_COVER_RANGE
};

void InitAI(void)
{
#ifdef _DEBUG
	if (gfDisplayCoverValues)
	{
		std::fill_n(gsCoverValue, WORLD_MAX, 0x7F7F);
	}
#endif

	//If we are not loading a saved game ( if we are, this has already been called )
	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		//init the panic system
		InitPanicSystem();
	}
}


static void HandleAITacticalTraversal(SOLDIERTYPE&);
static void TurnBasedHandleNPCAI(SOLDIERTYPE* pSoldier);


void HandleSoldierAI( SOLDIERTYPE *pSoldier )
{
	// ATE
	// Bail if we are engaged in a NPC conversation/ and/or sequence ... or we have a pause because
	// we just saw someone... or if there are bombs on the bomb queue
	if ( pSoldier->uiStatusFlags & SOLDIER_ENGAGEDINACTION || gTacticalStatus.fEnemySightingOnTheirTurn || (gubElementsOnExplosionQueue != 0) )
	{
		return;
	}

	if ( gfExplosionQueueActive )
	{
		return;
	}

	if (pSoldier->uiStatusFlags & SOLDIER_PC)
	{
		// if we're in autobandage, or the AI control flag is set and the player has a quote record to perform, or is a boxer,
		// let AI process this merc; otherwise abort
		if ( !(gTacticalStatus.fAutoBandageMode) && !(pSoldier->uiStatusFlags & SOLDIER_PCUNDERAICONTROL && (pSoldier->ubQuoteRecord != 0 || pSoldier->uiStatusFlags & SOLDIER_BOXER) ) )
		{
			// patch...
			if ( pSoldier->fAIFlags & AI_HANDLE_EVERY_FRAME )
			{
				pSoldier->fAIFlags &= ~AI_HANDLE_EVERY_FRAME;
			}
			return;
		}

	}

	// determine what sort of AI to use
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		gfTurnBasedAI = TRUE;
	}
	else
	{
		gfTurnBasedAI = FALSE;
	}

	// If TURN BASED and NOT NPC's turn, or realtime and not our chance to think, bail...
	if (gfTurnBasedAI)
	{
		if ( (pSoldier->bTeam != OUR_TEAM) && gTacticalStatus.ubCurrentTeam == OUR_TEAM )
		{
			return;
		}
		// why do we let the quote record thing be in here?  we're in turnbased the quote record doesn't matter,
		// we can't act out of turn!
		if ( !(pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL) )
		{
			return;
		}

		if ( pSoldier->bTeam != gTacticalStatus.ubCurrentTeam )
		{
			SLOGE("Turning off AI flag for {} because trying to act out of turn", pSoldier->ubID);
			pSoldier->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
			return;
		}
		if ( pSoldier->bMoved )
		{
			if (gfTurnBasedAI)
			{
				SLOGD("Ending turn for {} because set to moved", pSoldier->ubID);
			}
			// this guy doesn't get to act!
			EndAIGuysTurn(*pSoldier);
			return;
		}

	}
	else if ( !(pSoldier->fAIFlags & AI_HANDLE_EVERY_FRAME) ) // if set to handle every frame, ignore delay!
	{
	//#ifndef AI_PROFILING
		//Time to handle guys in realtime (either combat or not )
		if (!TIMECOUNTERDONE(pSoldier->AICounter, AI_DELAY))
		{
			// CAMFIELD, LOOK HERE!
			return;
		}
		//#endif
	}

	if ( pSoldier->fAIFlags & AI_HANDLE_EVERY_FRAME ) // if set to handle every frame, ignore delay!
	{
		if (pSoldier->ubQuoteActionID != QUOTE_ACTION_ID_TURNTOWARDSPLAYER)
		{
			// turn off flag!
			pSoldier->fAIFlags &= (~AI_HANDLE_EVERY_FRAME);
		}
	}

	// if this NPC is getting hit, abort
	if (pSoldier->fGettingHit)
	{
		return;
	}

	if ( gTacticalStatus.bBoxingState == PRE_BOXING || gTacticalStatus.bBoxingState == BOXING || gTacticalStatus.bBoxingState == WON_ROUND || gTacticalStatus.bBoxingState == LOST_ROUND )
	{
		if ( ! ( pSoldier->uiStatusFlags & SOLDIER_BOXER ) )
		{
			// do nothing!
			if (gfTurnBasedAI)
			{
				SLOGD("Ending turn for {} because not a boxer", pSoldier->ubID);
			}
			EndAIGuysTurn(*pSoldier);
			return;
		}
	}

	// if this NPC is dying, bail
	if (pSoldier->bLife < OKLIFE || !pSoldier->bActive )
	{
		if ( pSoldier->bActive && pSoldier->fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}
		if (gfTurnBasedAI)
		{
			SLOGD("Ending turn for {} because bad life/inactive", pSoldier->ubID);
		}

		EndAIGuysTurn(*pSoldier);
		return;
	}

	if ( pSoldier->fAIFlags & AI_ASLEEP )
	{
		if ( gfTurnBasedAI && pSoldier->bVisible )
		{
			// turn off sleep flag, guy's got to be able to do stuff in turnbased
			// if he's visible
			pSoldier->fAIFlags &= ~AI_ASLEEP;
		}
		else if ( !(pSoldier->fAIFlags & AI_CHECK_SCHEDULE) )
		{
			// don't do anything!
			if (gfTurnBasedAI)
			{
				SLOGD("Ending turn for {} because asleep and no scheduled action", pSoldier->ubID);
			}

			EndAIGuysTurn(*pSoldier);
			return;
		}
	}

	if (!pSoldier->bInSector && !(pSoldier->fAIFlags & AI_CHECK_SCHEDULE))
	{
		// don't do anything!
		if (gfTurnBasedAI)
		{
			SLOGD("Ending turn for {} because out of sector and no scheduled action", pSoldier->ubID);
		}

		EndAIGuysTurn(*pSoldier);
		return;
	}

	if (IsMechanical(*pSoldier) && !TANK(pSoldier))
	{
		// bail out!
		if (gfTurnBasedAI)
		{
			SLOGD("Ending turn for {} because is vehicle or robot", pSoldier->ubID);
		}

		EndAIGuysTurn(*pSoldier);
		return;
	}

	if (pSoldier->bCollapsed)
	{
		// being handled so turn off muzzle flash
		if ( pSoldier->fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}
		if (gfTurnBasedAI)
		{
			SLOGD("Ending turn for {} because unconscious", pSoldier->ubID);
		}

		// stunned/collapsed!
		CancelAIAction(pSoldier);
		EndAIGuysTurn(*pSoldier);
		return;
	}

	// in the unlikely situation (Sgt Krott et al) that we have a quote trigger going on
	// during turnbased, don't do any AI
	if ( pSoldier->ubProfile != NO_PROFILE && (pSoldier->ubProfile == SERGEANT || pSoldier->ubProfile == MIKE || pSoldier->ubProfile == JOE) && (gTacticalStatus.uiFlags & INCOMBAT) && (gfInTalkPanel || gfWaitingForTriggerTimer || !DialogueQueueIsEmpty() ) )
	{
		return;
	}

	// ATE: Did some changes here
	// DON'T rethink if we are determined to get somewhere....
	if ( pSoldier->bNewSituation == IS_NEW_SITUATION )
	{
		BOOLEAN fProcessNewSituation;

		// if this happens during an attack then do nothing... wait for the A.B.C.
		// to be reduced to 0 first -- CJC December 13th
		if ( gTacticalStatus.ubAttackBusyCount > 0 )
		{
			fProcessNewSituation = FALSE;
			// HACK!!
			if ( pSoldier->bAction == AI_ACTION_FIRE_GUN )
			{
				if ( guiNumBullets == 0 )
				{
					// abort attack!
					SLOGD("Attack busy count lobotomized due to new situation for {}", pSoldier->ubID);
					//gTacticalStatus.ubAttackBusyCount = 0;
					fProcessNewSituation = TRUE;
				}
			}
			else if ( pSoldier->bAction == AI_ACTION_TOSS_PROJECTILE )
			{
				if ( guiNumObjectSlots == 0 )
				{
					// abort attack!
					SLOGD("Attack busy count lobotomized due to new situation for {}", pSoldier->ubID);
					gTacticalStatus.ubAttackBusyCount = 0;
					fProcessNewSituation = TRUE;
				}
			}
		}
		else
		{
			fProcessNewSituation = TRUE;
		}

		if ( fProcessNewSituation )
		{
			if ( (pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL) && pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH && !GridNoOnVisibleWorldTile( pSoldier->sGridNo ) )
			{
				// traversing offmap, ignore new situations
			}
			else if ( pSoldier->ubQuoteRecord == 0 && !gTacticalStatus.fAutoBandageMode  )
			{
				// don't force, don't want escorted mercs reacting to new opponents, etc.
				// now we don't have AI controlled escorted mercs though - CJC
				CancelAIAction(pSoldier);
				// zap any next action too
				if ( pSoldier->bAction != AI_ACTION_END_COWER_AND_MOVE )
				{
					pSoldier->bNextAction = AI_ACTION_NONE;
				}
				DecideAlertStatus( pSoldier );
			}
			else
			{
				if ( pSoldier->ubQuoteRecord )
				{
					// make sure we're not using combat AI
					pSoldier->bAlertStatus = STATUS_GREEN;
				}
				pSoldier->bNewSituation = WAS_NEW_SITUATION;
			}
		}
	}
	else
	{
		// might have been in 'was' state; no longer so...
		pSoldier->bNewSituation = NOT_NEW_SITUATION;
	}
	SLOGD("handling AI for {}", pSoldier->ubID);

	/*********
	Start of new overall AI system
	********/



	if (gfTurnBasedAI)
	{
		if (GetJA2Clock() - gTacticalStatus.uiTimeSinceMercAIStart > DEADLOCK_DELAY)
		{
			// ATE: Display message that deadlock occured...
			SLOGI("Breaking Deadlock");
			// If we are in beta version, also report message!
			SLOGE("Aborting AI deadlock for {}. Please sent LOG file and SAVE.", pSoldier->ubID);
			// just abort
			EndAIDeadlock();
			if ( !(pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL) )
			{
				return;
			}
		}
	}

	if (pSoldier->bAction == AI_ACTION_NONE)
	{
		// being handled so turn off muzzle flash
		if ( pSoldier->fMuzzleFlash )
		{
			EndMuzzleFlash( pSoldier );
		}

		// figure out what to do!
		if (gfTurnBasedAI)
		{
			if (pSoldier->fNoAPToFinishMove)
			{
				// well that move must have been cancelled because we're thinking now!
				//pSoldier->fNoAPToFinishMove = FALSE;
			}
			TurnBasedHandleNPCAI( pSoldier );
		}
		else
		{
			RTHandleAI( pSoldier );
		}

	}
	else
	{

		// an old action was in progress; continue it
		if (pSoldier->bAction >= FIRST_MOVEMENT_ACTION && pSoldier->bAction <= LAST_MOVEMENT_ACTION && !pSoldier->fDelayedMovement)
		{
			if (pSoldier->ubPathIndex == pSoldier->ubPathDataSize)
			{
				if (pSoldier->sAbsoluteFinalDestination != NOWHERE)
				{
					if ( !ACTING_ON_SCHEDULE( pSoldier ) &&  SpacesAway( pSoldier->sGridNo, pSoldier->sAbsoluteFinalDestination ) < 4 )
					{
						// This is close enough... reached final destination for NPC system move
						if ( pSoldier->sAbsoluteFinalDestination != pSoldier->sGridNo )
						{
							// update NPC records to replace our final dest with this location
							ReplaceLocationInNPCDataFromProfileID( pSoldier->ubProfile, pSoldier->sAbsoluteFinalDestination, pSoldier->sGridNo );
						}
						pSoldier->sAbsoluteFinalDestination = pSoldier->sGridNo;
						// change action data so that we consider this our final destination below
						pSoldier->usActionData = pSoldier->sGridNo;
					}

					if ( pSoldier->sAbsoluteFinalDestination == pSoldier->sGridNo )
					{
						pSoldier->sAbsoluteFinalDestination = NOWHERE;

						if ( !ACTING_ON_SCHEDULE( pSoldier ) && pSoldier->ubQuoteRecord && pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST )
						{
							NPCReachedDestination( pSoldier, FALSE );
							// wait just a little bit so the queue can be processed
							pSoldier->bNextAction = AI_ACTION_WAIT;
							pSoldier->usNextActionData = 500;

						}
						else if (pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH)
						{
							HandleAITacticalTraversal(*pSoldier);
							return;
						}
					}
					else
					{
						// make sure this guy is handled next frame!
						pSoldier->uiStatusFlags |= AI_HANDLE_EVERY_FRAME;
					}
				}
				// for regular guys still have to check for leaving the map
				else if (pSoldier->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && pSoldier->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH)
				{
					HandleAITacticalTraversal(*pSoldier);
					return;
				}

				// reached destination
				SLOGD("Opponent {} reaches dest - action done", pSoldier->ubID);

				if ( pSoldier->sGridNo == pSoldier->sFinalDestination )
				{
					if ( pSoldier->bAction == AI_ACTION_MOVE_TO_CLIMB )
					{
						// successfully moved to roof!

						// fake setting action to climb roof and see if we can afford this
						pSoldier->bAction = AI_ACTION_CLIMB_ROOF;
						if (IsActionAffordable(pSoldier))
						{
							// set action to none and next action to climb roof so we do that next
							pSoldier->bAction = AI_ACTION_NONE;
							pSoldier->bNextAction = AI_ACTION_CLIMB_ROOF;
						}

					}
				}

				ActionDone(pSoldier);
			}

			//*** TRICK- TAKE INTO ACCOUNT PAUSED FOR NO TIME ( FOR NOW )
			if (pSoldier->fNoAPToFinishMove )
			{
				SoldierTriesToContinueAlongPath(pSoldier);
			}
			// ATE: Let's also test if we are in any stationary animation...
			else if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_STATIONARY ) )
			{
				// ATE: Put some ( MORE ) refinements on here....
				// If we are trying to open door, or jump fence  don't continue until done...
				if ( !pSoldier->fContinueMoveAfterStanceChange && !pSoldier->bEndDoorOpenCode )
				{
					//ATE: just a few more.....
					// If we have ANY pending aninmation that is movement.....
					if ( pSoldier->usPendingAnimation != NO_PENDING_ANIMATION && ( gAnimControl[ pSoldier->usPendingAnimation ].uiFlags & ANIM_MOVING ) )
					{
						// Don't do anything, we're waiting on a pending animation....
					}
					else
					{
						// OK, we have a move to finish...
						SLOGD("going to try to continue path for {}", pSoldier->ubID);
						SoldierTriesToContinueAlongPath(pSoldier);
					}
				}
			}
		}

	}

	/*********
	End of new overall AI system
	********/

}


void EndAIGuysTurn(SOLDIERTYPE& s)
{

	if (!gfTurnBasedAI) return;

	TacticalStatusType& ts = gTacticalStatus;
	if (ts.fSomeoneHit)
	{
		ts.fSomeoneHit = FALSE;
	}
	else
	{
		SayCloseCallQuotes();
	}

	// If civ in civ group and hostile, try to change nearby guys to hostile
	if (s.ubCivilianGroup != NON_CIV_GROUP && !s.bNeutral)
	{
		if (!(s.uiStatusFlags & SOLDIER_BOXER) ||
			(ts.bBoxingState != PRE_BOXING && ts.bBoxingState != BOXING))
		{
			ProfileID const first_pid = CivilianGroupMembersChangeSidesWithinProximity(&s);
			if (first_pid != NO_PROFILE) TriggerFriendWithHostileQuote(first_pid);
		}
	}

	if (ts.uiFlags & SHOW_ALL_ROOFS && ts.uiFlags & INCOMBAT)
	{
		ts.uiFlags &= ~SHOW_ALL_ROOFS;
		SetRenderFlags(RENDER_FLAG_FULL);
		InvalidateWorldRedundency();
	}

	// End this NPC's control, move to next dude
	EndRadioLocator(&s);
	s.uiStatusFlags   &= ~SOLDIER_UNDERAICONTROL;
	s.fTurnInProgress  = FALSE;
	s.bMoved           = TRUE;
	s.bBypassToGreen   = FALSE;

	SLOGD("Ending control for {}", s.ubID);

	// Find the next AI guy
	if (SOLDIERTYPE* const s = RemoveFirstAIListEntry())
	{
		StartNPCAI(*s);
	}
	else
	{ // We are at the end, return control to next team
		SLOGD("Ending AI turn");
		EndAITurn();
	}
}


void EndAIDeadlock()
{
	// Escape enemy's turn

	// Find enemy with problem and free him up
	FOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE& s = *i;
		if (!s.bInSector) continue;
		if (!(s.uiStatusFlags & SOLDIER_UNDERAICONTROL)) continue;
		CancelAIAction(&s);
		if (gfTurnBasedAI)
		{
			SLOGD("Ending turn for {} because breaking deadlock", s.ubID);
		}

		SLOGD("Number of bullets in the air is {}", guiNumBullets);
		SLOGD("Setting attack busy count to 0 from deadlock break");
		gTacticalStatus.ubAttackBusyCount = 0;

		EndAIGuysTurn(s);
		return;
	}

	StartPlayerTeamTurn(TRUE, FALSE);
}


void StartNPCAI(SOLDIERTYPE& s)
{
	SetSoldierAsUnderAiControl(&s);

	s.fTurnInProgress      = TRUE;
	s.sLastTwoLocations[0] = NOWHERE;
	s.sLastTwoLocations[1] = NOWHERE;

	RefreshAI(&s);
	SLOGD("Giving control to {}", s.ubID);

	TacticalStatusType& ts = gTacticalStatus;
	ts.uiTimeSinceMercAIStart = GetJA2Clock();

	/* important: if "fPausedAnimation" is TRUE, then we have to turn it off else
	 * HandleSoldierAI() will not be called! */

	// Locate to soldier, if we are not in an interrupt situation.
	if ((ts.uiFlags & INCOMBAT) && gubOutOfTurnPersons == 0)
	{
		if ((!(s.uiStatusFlags & SOLDIER_VEHICLE) || GetNumberInVehicle(GetVehicle(s.bVehicleID)) != 0) &&
				((s.bVisible != -1 && s.bLife != 0) || ts.uiFlags & SHOW_ALL_MERCS))
		{
			// If we are on a roof, set flag for rendering.
			if (s.bLevel != 0 && ts.uiFlags & INCOMBAT)
			{
				ts.uiFlags |= SHOW_ALL_ROOFS;
				SetRenderFlags(RENDER_FLAG_FULL);
				InvalidateWorldRedundency();
			}

			// Skip locator for green friendly militia.
			if (s.bTeam != MILITIA_TEAM || s.bSide != Side::FRIENDLY || s.bAlertStatus != STATUS_GREEN)
			{
				LocateSoldier(&s, SETLOCATORFAST);
			}
		}

		UpdateEnemyUIBar();
	}
	DecideAlertStatus(&s);
}


void FreeUpNPCFromPendingAction( 	SOLDIERTYPE *pSoldier )
{
	if ( pSoldier )
	{
		if ( pSoldier->bAction == AI_ACTION_PENDING_ACTION
			|| pSoldier->bAction == AI_ACTION_OPEN_OR_CLOSE_DOOR
			|| pSoldier->bAction == AI_ACTION_CREATURE_CALL
			|| pSoldier->bAction == AI_ACTION_YELLOW_ALERT
			|| pSoldier->bAction == AI_ACTION_RED_ALERT
			|| pSoldier->bAction == AI_ACTION_UNLOCK_DOOR
			|| pSoldier->bAction == AI_ACTION_PULL_TRIGGER
			|| pSoldier->bAction == AI_ACTION_LOCK_DOOR	)
		{
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				if ( pSoldier->ubQuoteRecord == NPC_ACTION_KYLE_GETS_MONEY )
				{
					// Kyle after getting money
					pSoldier->ubQuoteRecord = 0;
					TriggerNPCRecord( KYLE, 11 );
				}
				else if (pSoldier->usAnimState == END_OPENSTRUCT)
				{
					TriggerNPCWithGivenApproach(pSoldier->ubProfile, APPROACH_DONE_OPEN_STRUCTURE);
				}
				else if (pSoldier->usAnimState == PICKUP_ITEM || pSoldier->usAnimState == ADJACENT_GET_ITEM )
				{
					TriggerNPCWithGivenApproach(pSoldier->ubProfile, APPROACH_DONE_GET_ITEM);
				}
			}
			ActionDone(pSoldier);
		}
	}
}


void FreeUpNPCFromAttacking(SOLDIERTYPE* const pSoldier)
{
	ActionDone(pSoldier);

	/*
	if (pSoldier->bActionInProgress)
	{
		SLOGD("FreeUpNPCFromAttacking for {}", pSoldier->ubID);
		if (pSoldier->bAction == AI_ACTION_FIRE_GUN)
		{
			if (pSoldier->bDoBurst)
			{
				if (pSoldier->bBulletsLeft == 0)
				{
					// now find the target and have them say "close call" quote if
					// applicable
					pTarget = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bTargetLevel);
					if (pTarget && pTarget->bTeam == OUR_TEAM && pTarget->fCloseCall && pTarget->bShock == 0)
					{
						// say close call quote!
						TacticalCharacterDialogue( pTarget, QUOTE_CLOSE_CALL );
						pTarget->fCloseCall = FALSE;
					}
					ActionDone(pSoldier);
					pSoldier->bDoBurst = FALSE;
				}
			}
			else
			{
				pTarget = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bTargetLevel);
				if (pTarget && pTarget->bTeam == OUR_TEAM && pTarget->fCloseCall && pTarget->bShock == 0)
				{
					// say close call quote!
					TacticalCharacterDialogue( pTarget, QUOTE_CLOSE_CALL );
					pTarget->fCloseCall = FALSE;
				}
				ActionDone(pSoldier);
			}
		}
		else if ((pSoldier->bAction == AI_ACTION_TOSS_PROJECTILE) || (pSoldier->bAction == AI_ACTION_KNIFE_STAB))
		{
			ActionDone(pSoldier);
		}
	}

	// DO WE NEED THIS???
	//pSoldier->sTarget = NOWHERE;

	// This is here to speed up resolution of interrupts that have already been
	// delayed while AttackingPerson was still set (causing ChangeControl to
	// bail).  Without it, an interrupt would have to wait until next ani frame!
	//if (SwitchTo > -1)
	//  ChangeControl();
	*/
}


void FreeUpNPCFromTurning(SOLDIERTYPE* pSoldier)
{
	// if NPC is in the process of changing facing, mark him as being done!
	if ((pSoldier->bAction == AI_ACTION_CHANGE_FACING) && pSoldier->bActionInProgress)
	{
		SLOGD("FreeUpNPCFromTurning: our action {}, desdir {} dir {}",
			pSoldier->bAction, pSoldier->bDesiredDirection, pSoldier->bDirection);
		ActionDone(pSoldier);
	}
}


void FreeUpNPCFromStanceChange(SOLDIERTYPE *pSoldier )
{
	// are we/were we doing something?
	if (pSoldier->bActionInProgress)
	{
		// check and see if we were changing stance
		if (pSoldier->bAction == AI_ACTION_CHANGE_STANCE || pSoldier->bAction == AI_ACTION_COWER || pSoldier->bAction == AI_ACTION_STOP_COWERING)
		{
			// yes we were - are we finished?
			if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == pSoldier->usActionData )
			{
				// yes! Free us up to do other fun things
				ActionDone(pSoldier);
			}
		}
	}
}

void FreeUpNPCFromRoofClimb(SOLDIERTYPE *pSoldier )
{
	// are we/were we doing something?
	if (pSoldier->bActionInProgress)
	{
		// check and see if we were climbing
		if (pSoldier->bAction == AI_ACTION_CLIMB_ROOF)
		{
			// yes! Free us up to do other fun things
			ActionDone(pSoldier);
		}
	}
}




void ActionDone(SOLDIERTYPE *pSoldier)
{
	// if an action is currently selected
	if (pSoldier->bAction != AI_ACTION_NONE)
	{
		if (pSoldier->uiStatusFlags & SOLDIER_MONSTER)
		{
			SLOGD("Cancelling actiondone: our action {}, desdir {} dir {}",
				pSoldier->bAction, pSoldier->bDesiredDirection, pSoldier->bDirection);
		}

		// If doing an attack, reset attack busy count and # of bullets
		//if ( gTacticalStatus.ubAttackBusyCount )
		//{
		//	gTacticalStatus.ubAttackBusyCount = 0;
		//	SLOGD("Setting attack busy count to 0 due to Action Done");
		//	pSoldier->bBulletsLeft = 0;
		//}

		// cancel any turning & movement by making current settings desired ones
		pSoldier->sFinalDestination	= pSoldier->sGridNo;

		if ( !pSoldier->fNoAPToFinishMove )
		{
			EVENT_StopMerc(pSoldier);
			AdjustNoAPToFinishMove( pSoldier, FALSE );
		}

		// cancel current action
		pSoldier->bLastAction       = pSoldier->bAction;
		pSoldier->bAction           = AI_ACTION_NONE;
		pSoldier->usActionData      = NOWHERE;
		pSoldier->bActionInProgress = FALSE;
		pSoldier->fDelayedMovement  = FALSE;

		/*
		if ( pSoldier->bLastAction == AI_ACTION_CHANGE_STANCE || pSoldier->bLastAction == AI_ACTION_COWER || pSoldier->bLastAction == AI_ACTION_STOP_COWERING )
		{
			SoldierGotoStationaryStance( pSoldier );
		}
		*/


		// make sure pathStored is not left TRUE by accident.
		// This is possible if we decide on an action that we have no points for
		// (but which set pathStored).  The action is retained until next turn,
		// although NewDest isn't called.  A newSit. could cancel it before then!
		pSoldier->bPathStored = FALSE;
	}
}


// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////

//	O L D    D G    A I    C O D E

// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////////////


// GLOBALS:

UINT8 SkipCoverCheck = FALSE;
THREATTYPE Threat[MAXMERCS];


// threat percentage is based on the certainty of opponent knowledge:
// opplist value:        -4  -3  -2  -1 SEEN  1    2   3   4   5
int ThreatPercent[10] = { 20, 40, 60, 80, 25, 100, 90, 75, 60, 45 };



void NPCDoesAct(SOLDIERTYPE *pSoldier)
{
	// if the action is visible and we're in a hidden turnbased mode, go to turnbased
	if (!(gTacticalStatus.uiFlags & INCOMBAT) &&
			(pSoldier->bAction == AI_ACTION_FIRE_GUN ||
			pSoldier->bAction == AI_ACTION_TOSS_PROJECTILE ||
			pSoldier->bAction == AI_ACTION_KNIFE_MOVE ||
			pSoldier->bAction == AI_ACTION_KNIFE_STAB ||
			pSoldier->bAction == AI_ACTION_THROW_KNIFE))
	{
		DisplayHiddenTurnbased( pSoldier );
	}

	if (gfHiddenInterrupt)
	{
		DisplayHiddenInterrupt( pSoldier );
	}
	// *** IAN deleted lots of interrupt related code here to simplify JA2	development

	// CJC Feb 18 99: make sure that soldier is not in the middle of a turn due to visual crap to make enemies
	// face and point their guns at us
	if ( pSoldier->bDesiredDirection != pSoldier->bDirection )
	{
		pSoldier->bDesiredDirection = pSoldier->bDirection;
	}
}


static void NPCDoesNothing(SOLDIERTYPE* pSoldier)
{
	// NPC, for whatever reason, did/could not start an action, so end his turn
	//pSoldier->moved = TRUE;

	if (gfTurnBasedAI)
	{
		SLOGD("Ending turn for {} because doing no-action", pSoldier->ubID);
	}

	EndAIGuysTurn(*pSoldier);

	// *** IAN deleted lots of interrupt related code here to simplify JA2	development
}


void CancelAIAction(SOLDIERTYPE* const pSoldier)
{
	if (SkipCoverCheck)
		SLOGD("CancelAIAction: SkipCoverCheck turned OFF");

	// re-enable cover checking, something is new or something strange happened
	SkipCoverCheck = FALSE;

	// turn off new situation flag to stop this from repeating all the time!
	if ( pSoldier->bNewSituation == IS_NEW_SITUATION )
	{
		pSoldier->bNewSituation = WAS_NEW_SITUATION;
	}

	// turn off RED/YELLOW status "bypass to Green", to re-check all actions
	pSoldier->bBypassToGreen = FALSE;

	ActionDone(pSoldier);
}


bool ActionInProgress(SOLDIERTYPE * const pSoldier)
{
	// if NPC has a desired destination, but isn't currently going there
	if ((pSoldier->sFinalDestination != NOWHERE) && (pSoldier->sDestination != pSoldier->sFinalDestination))
	{
		// return success (TRUE) if we successfully resume the movement
		return(TryToResumeMovement(pSoldier,pSoldier->sFinalDestination));
	}


	// this here should never happen, but it seems to (turns sometimes hang!)
	if ((pSoldier->bAction == AI_ACTION_CHANGE_FACING) && (pSoldier->bDesiredDirection != pSoldier->usActionData))
	{
		SLOGD("ActionInProgress: WARNING - CONTINUING FACING CHANGE...");

		// don't try to pay any more APs for this, it was paid for once already!
		pSoldier->bDesiredDirection = (INT8) pSoldier->usActionData;   // turn to face direction in actionData
	}

	// needs more time to complete action
	return true;
}


static void TurnBasedHandleNPCAI(SOLDIERTYPE* pSoldier)
{
	/*
	// If man is inactive/at base/dead/unconscious
	if (!pSoldier->bActive || !pSoldier->bInSector || (pSoldier->bLife < OKLIFE))
	{
		NPCDoesNothing(pSoldier);
		return;
	}

	if (IsOnCivTeam(pSoldier) && pSoldier->service &&
		(pSoldier->bNeutral || MedicsMissionIsEscort(pSoldier)))
	{
		NPCDoesNothing(pSoldier);
		return;
	}
	*/



	/*
	anim = pSoldier->anitype[pSoldier->anim];

	// If man is down on the ground
	if (anim < BREATHING)
	{
		// if he lacks the breath, or APs to get up this turn (life checked above)
		// OR... (new June 13/96 Ian) he's getting first aid...
		if ((pSoldier->bBreath < OKBREATH) || (pSoldier->bActionPoints < (AP_GET_UP + AP_ROLL_OVER))
			|| pSoldier->service)
		{
			NPCDoesNothing(pSoldier);
			return;
		}
		else
		{
			// wait until he gets up first, only then worry about deciding his AI
			return;
		}
	}


	// if NPC's has been forced to stop by an opponent's interrupt or similar
	if (pSoldier->forcedToStop)
	{
		return;
	}

	// if we are still in the midst in an uninterruptable animation
	if (!AnimControl[anim].interruptable)
	{
		return;      // wait a while, let the animation finish first
	}

	*/

	// yikes, this shouldn't occur! we should be trying to finish our move!
	// pSoldier->fNoAPToFinishMove = FALSE;

	/*
	// move this clause outside of the function...
	if (pSoldier->bNewSituation)
		// don't force, don't want escorted mercs reacting to new opponents, etc.
		CancelAIAction(pSoldier);

	*/

	if ((pSoldier->bAction != AI_ACTION_NONE) && pSoldier->bActionInProgress)
	{
		/*
		if (pSoldier->bAction == AI_ACTION_RANDOM_PATROL)
		{
			if (pSoldier->ubPathIndex == pSoldier->ubPathDataSize)
			//if (pSoldier->usActionData == pSoldier->sGridNo )
			//(IC?) if (pSoldier->bAction == AI_ACTION_RANDOM_PATROL && ( pSoldier->ubPathIndex == pSoldier->ubPathDataSize ) )
			//(old?) if (pSoldier->bAction == AI_ACTION_RANDOM_PATROL && ( pSoldier->usActionData == pSoldier->sGridNo ) )
			{
			SLOGD("OPPONENT {} REACHES DEST - ACTION DONE", pSoldier->ubID);
				ActionDone(pSoldier);
			}

			// *** TRICK- TAKE INTO ACCOUNT PAUSED FOR NO TIME ( FOR NOW )
			if (pSoldier->fNoAPToFinishMove)
			//if (pSoldier->bAction == AI_ACTION_RANDOM_PATROL && pSoldier->fNoAPToFinishMove)
			{
				// OK, we have a move to finish...
				SLOGD("GONNA TRY TO CONTINUE PATH FOR {}", pSoldier->ubID);
				SoldierTriesToContinueAlongPath(pSoldier);

				// since we just gave up on our action due to running out of points, better end our turn
				//EndAIGuysTurn(*pSoldier);
			}
		}
		*/

		// if action should remain in progress
		if (ActionInProgress(pSoldier))
		{
			// let it continue
			return;
		}
	}


	SLOGD("HandleManAI - DECIDING for guynum {}({}) at gridno {}, APs {}",
		pSoldier->ubID, pSoldier->name, pSoldier->sGridNo, pSoldier->bActionPoints);


	// if man has nothing to do
	if (pSoldier->bAction == AI_ACTION_NONE)
	{
		// make sure this flag is turned off (it already should be!)
		pSoldier->bActionInProgress = FALSE;

		// Since we're NEVER going to "continue" along an old path at this point,
		// then it would be nice place to reinitialize "pathStored" flag for
		// insurance purposes.
		//
		// The "pathStored" variable controls whether it's necessary to call
		// findNewPath() after you've called NewDest(). Since the AI calls
		// findNewPath() itself, a speed gain can be obtained by avoiding
		// redundancy.
		//
		// The "normal" way for pathStored to be reset is inside
		// SetNewCourse() [which gets called after NewDest()].
		//
		// The only reason we would NEED to reinitialize it here is if I've
		// incorrectly set pathStored to TRUE in a process that doesn't end up
		// calling NewDest()
		pSoldier->bPathStored = FALSE;

		// decide on the next action
		if (pSoldier->bNextAction != AI_ACTION_NONE)
		{
			// do the next thing we have to do...
			if ( pSoldier->bNextAction == AI_ACTION_END_COWER_AND_MOVE )
			{
				if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
				{
					pSoldier->bAction = AI_ACTION_STOP_COWERING;
					pSoldier->usActionData = ANIM_STAND;
				}
				else if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight < ANIM_STAND )
				{
					// stand up!
					pSoldier->bAction = AI_ACTION_CHANGE_STANCE;
					pSoldier->usActionData = ANIM_STAND;
				}
				else
				{
					pSoldier->bAction = AI_ACTION_NONE;
				}
				if ( pSoldier->sGridNo == pSoldier->usNextActionData )
				{
					// no need to walk after this
					pSoldier->bNextAction = AI_ACTION_NONE;
					pSoldier->usNextActionData = NOWHERE;
				}
				else
				{
					pSoldier->bNextAction = AI_ACTION_WALK;
					// leave next-action-data as is since that's where we want to go
				}
			}
			else
			{
				pSoldier->bAction = pSoldier->bNextAction;
				pSoldier->usActionData = pSoldier->usNextActionData;
				pSoldier->bTargetLevel = pSoldier->bNextTargetLevel;
				pSoldier->bNextAction = AI_ACTION_NONE;
				pSoldier->usNextActionData = 0;
				pSoldier->bNextTargetLevel = 0;
			}
			if (pSoldier->bAction == AI_ACTION_PICKUP_ITEM)
			{
				// the item pool index was stored in the special data field
				pSoldier->uiPendingActionData1 = pSoldier->iNextActionSpecialData;
			}
		}
		else if ( pSoldier->sAbsoluteFinalDestination != NOWHERE )
		{
			if ( ACTING_ON_SCHEDULE( pSoldier ) )
			{
				pSoldier->bAction = AI_ACTION_SCHEDULE_MOVE;
			}
			else
			{
				pSoldier->bAction = AI_ACTION_WALK;
			}
			pSoldier->usActionData = pSoldier->sAbsoluteFinalDestination;
		}
		else
		{
			if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
			{
				if (CREATURE_OR_BLOODCAT( pSoldier ))
				{
					pSoldier->bAction = CreatureDecideAction( pSoldier );
				}
				else if (pSoldier->ubBodyType == CROW)
				{
					pSoldier->bAction = CrowDecideAction( pSoldier );
				}
				else
				{
					pSoldier->bAction = DecideAction(pSoldier);
				}
			}
		}

		if (pSoldier->bAction == AI_ACTION_ABSOLUTELY_NONE)
		{
			pSoldier->bAction = AI_ACTION_NONE;
		}

		// if he chose to continue doing nothing
		if (pSoldier->bAction == AI_ACTION_NONE)
		{
			NPCDoesNothing(pSoldier);  // sets pSoldier->moved to TRUE
			return;
		}



		/*
		// if we somehow just caused an uninterruptable animation to occur
		// This is mainly to finish a weapon_AWAY anim that preceeds a TOSS attack
		if (!AnimControl[ pSoldier->anitype[pSoldier->anim] ].interruptable)
		{
			SLOGD("Uninterruptable animation {}, skipping guy {}",pSoldier->anitype[pSoldier->anim],pSoldier->ubID);

			return;      // wait a while, let the animation finish first
		}
		*/

		// to get here, we MUST have an action selected, but not in progress...

		// see if we can afford to do this action
		if (IsActionAffordable(pSoldier))
		{
			NPCDoesAct(pSoldier);

			// perform the chosen action
			pSoldier->bActionInProgress = ExecuteAction(pSoldier); // if started, mark us as busy

			if ( !pSoldier->bActionInProgress && pSoldier->sAbsoluteFinalDestination != NOWHERE )
			{
				// turn based... abort this guy's turn
				EndAIGuysTurn(*pSoldier);
			}
		}
		else
		{
			HaltMoveForSoldierOutOfPoints(*pSoldier);
			return;
		}
	}
}


static void ManChecksOnFriends(SOLDIERTYPE* pSoldie);


void RefreshAI(SOLDIERTYPE *pSoldier)
{
	// produce our own private "mine map" so we can avoid the ones we can detect
	// MarkDetectableMines(pSoldier);

	// whether last attack hit or not doesn't matter once control has been lost
	pSoldier->bLastAttackHit = FALSE;

	// get an up-to-date alert status for this guy
	DecideAlertStatus(pSoldier);

	if (pSoldier->bAlertStatus == STATUS_YELLOW)
		SkipCoverCheck = FALSE;

	// if he's in battle or knows opponents are here
	if (gfTurnBasedAI)
	{
		if ((pSoldier->bAlertStatus == STATUS_BLACK) || (pSoldier->bAlertStatus == STATUS_RED))
		{
			// always freshly rethink things at start of his turn
			pSoldier->bNewSituation = IS_NEW_SITUATION;
		}
		else
		{
			// make sure any paths stored during out last AI decision but not reacted
			// to (probably due to lack of APs) get re-tested by the ExecuteAction()
			// function in AI, since the ->sDestination may no longer be legal now!
			pSoldier->bPathStored = FALSE;

			// if not currently engaged, or even alerted
			// take a quick look around to see if any friends seem to be in trouble
			ManChecksOnFriends(pSoldier);

			// allow stationary GREEN Civilians to turn again at least 1/turn!
		}
		pSoldier->bLastAction = AI_ACTION_NONE;

	}
}


static void AIDecideRadioAnimation(SOLDIERTYPE* pSoldier)
{
	if ( pSoldier->ubBodyType != REGMALE && pSoldier->ubBodyType != BIGMALE )
	{
		// no animation available
		ActionDone( pSoldier );
		return;
	}

	if (IsOnCivTeam(pSoldier) && pSoldier->ubCivilianGroup != KINGPIN_CIV_GROUP)
	{
		// don't play anim
		ActionDone( pSoldier );
		return;
	}

	switch( gAnimControl[ pSoldier->usAnimState ].ubEndHeight )
	{
		case ANIM_STAND:

			EVENT_InitNewSoldierAnim( pSoldier, AI_RADIO, 0 , FALSE );
			break;

		case ANIM_CROUCH:

			EVENT_InitNewSoldierAnim( pSoldier, AI_CR_RADIO, 0 , FALSE );
			break;

		case ANIM_PRONE:

			ActionDone( pSoldier );
			break;
	}
}


INT8 ExecuteAction(SOLDIERTYPE *pSoldier)
{
	// in most cases, merc will change location, or may cause damage to opponents,
	// so a new cover check will be necessary.  Exceptions handled individually.
	SkipCoverCheck = FALSE;

	// reset this field, too
	pSoldier->bLastAttackHit = FALSE;

	if (gfTurnBasedAI || gTacticalStatus.fAutoBandageMode)
	{
		SLOGD("{} does {} (a.d. {}) in {} with {} APs left",
			pSoldier->ubID, gzActionStr[pSoldier->bAction], pSoldier->usActionData,
			pSoldier->sGridNo, pSoldier->bActionPoints);
	}

	SLOGD("{} does {} (a.d. {}) at time {}", pSoldier->ubID,
		gzActionStr[pSoldier->bAction], pSoldier->usActionData, GetJA2Clock());

	switch (pSoldier->bAction)
	{
		case AI_ACTION_NONE:                  // maintain current position & facing
			// do nothing
			break;

		case AI_ACTION_WAIT: // hold AI_ACTION_NONE for a specified time
			if (gfTurnBasedAI)
			{
				// probably an action set as a next-action in the realtime prior to combat
				// do nothing
			}
			else
			{
				RESETTIMECOUNTER( pSoldier->AICounter, pSoldier->usActionData );
			}
			ActionDone( pSoldier );
			break;

		case AI_ACTION_CHANGE_FACING:         // turn this way & that to look
			// as long as we don't see anyone new, cover won't have changed
			// if we see someone new, it will cause a new situation & remove this
			SkipCoverCheck = TRUE;

			SLOGD("ExecuteAction: SkipCoverCheck ON");

			//pSoldier->bDesiredDirection = (UINT8) ;   // turn to face direction in actionData
			SendSoldierSetDesiredDirectionEvent( pSoldier, pSoldier->usActionData );
			// now we'll have to wait for the turning to finish; no need to call TurnSoldier here
			//TurnSoldier( pSoldier );
			/*
			if (!StartTurn(pSoldier,pSoldier->usActionData,FASTTURN))
			{
				// ZAP NPC's remaining action points so this isn't likely to repeat
				pSoldier->bActionPoints = 0;

				CancelAIAction(pSoldier);
				return(FALSE);         // nothing is in progress
			}
			*/
			break;

		case AI_ACTION_PICKUP_ITEM: // grab something!
			SoldierPickupItem( pSoldier, pSoldier->uiPendingActionData1, pSoldier->usActionData, 0 );
			break;

		case AI_ACTION_DROP_ITEM: // drop item in hand
			SoldierDropItem( pSoldier, &(pSoldier->inv[HANDPOS]) );
			DeleteObj( &(pSoldier->inv[HANDPOS]) );
			pSoldier->bAction = AI_ACTION_PENDING_ACTION;
			break;

		case AI_ACTION_MOVE_TO_CLIMB:
			if ( pSoldier->usActionData == pSoldier->sGridNo )
			{
				// change action to climb now and try that.
				pSoldier->bAction = AI_ACTION_CLIMB_ROOF;
				if (IsActionAffordable(pSoldier))
				{
					return( ExecuteAction( pSoldier ) );
				}
				else
				{
					// no action started
					return( FALSE );
				}
			}
			// fall through
		case AI_ACTION_RANDOM_PATROL:         // move towards a particular location
		case AI_ACTION_SEEK_FRIEND:           // move towards friend in trouble
		case AI_ACTION_SEEK_OPPONENT:         // move towards a reported opponent
		case AI_ACTION_TAKE_COVER:            // run for nearest cover from threat
		case AI_ACTION_GET_CLOSER:            // move closer to a strategic location

		case AI_ACTION_POINT_PATROL:          // move towards next patrol point
		case AI_ACTION_LEAVE_WATER_GAS:       // seek nearest spot of ungassed land
		case AI_ACTION_SEEK_NOISE:            // seek most important noise heard
		case AI_ACTION_RUN_AWAY:              // run away from nearby opponent(s)

		case AI_ACTION_APPROACH_MERC:         // walk up to someone to talk
		case AI_ACTION_TRACK:                 // track by ground scent
		case AI_ACTION_EAT:                   // monster approaching corpse
		case AI_ACTION_SCHEDULE_MOVE:
		case AI_ACTION_WALK:
		case AI_ACTION_RUN:

			if ( gfTurnBasedAI && pSoldier->bAlertStatus < STATUS_BLACK )
			{
				if ( pSoldier->sLastTwoLocations[0] == NOWHERE )
				{
					pSoldier->sLastTwoLocations[0] = pSoldier->sGridNo;
				}
				else if ( pSoldier->sLastTwoLocations[1] == NOWHERE )
				{
					pSoldier->sLastTwoLocations[1] = pSoldier->sGridNo;
				}
				// check for loop
				else if ( pSoldier->usActionData == pSoldier->sLastTwoLocations[1] && pSoldier->sGridNo == pSoldier->sLastTwoLocations[0] )
				{
					SLOGD("{} in movement loop, aborting turn", pSoldier->ubID);

					// loop found!
					ActionDone( pSoldier );
					EndAIGuysTurn(*pSoldier);
				}
				else
				{
					pSoldier->sLastTwoLocations[0] = pSoldier->sLastTwoLocations[1];
					pSoldier->sLastTwoLocations[1] = pSoldier->sGridNo;
				}
			}

			// Randomly do growl...
			if ( pSoldier->ubBodyType == BLOODCAT )
			{
				if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
				{
					if ( Random( 2 ) == 0 )
					{
						PlaySoldierJA2Sample(pSoldier, SoundRange<BLOODCAT_GROWL_1, BLOODCAT_GROWL_4>(), HIGHVOLUME, 1, TRUE);
					}
				}
			}

			// on YELLOW/GREEN status, NPCs keep the actions from turn to turn
			// (newSituation is intentionally NOT set in NewSelectedNPC()), so the
			// possibility exists that NOW the actionData is no longer a valid
			// NPC ->sDestination (path got blocked, someone is now standing at that
			// gridno, etc.)  So we gotta check again that the ->sDestination's legal!

			// optimization - Ian (if up-to-date path is known, do not check again)
			if (!pSoldier->bPathStored)
			{
				if ( (pSoldier->sAbsoluteFinalDestination != NOWHERE || gTacticalStatus.fAutoBandageMode) && !(gTacticalStatus.uiFlags & INCOMBAT) )
				{
					// NPC system move, allow path through
					if (LegalNPCDestination(pSoldier,pSoldier->usActionData,ENSURE_PATH,WATEROK, PATH_THROUGH_PEOPLE ))
					{
						// optimization - Ian: prevent another path call in SetNewCourse()
						pSoldier->bPathStored = TRUE;
					}
				}
				else
				{
					if (LegalNPCDestination(pSoldier,pSoldier->usActionData,ENSURE_PATH,WATEROK, 0))
					{
						// optimization - Ian: prevent another path call in SetNewCourse()
						pSoldier->bPathStored = TRUE;
					}
				}

				// if we STILL don't have a path
				if ( !pSoldier->bPathStored )
				{
					// Check if we were told to move by NPC stuff
					if ( pSoldier->sAbsoluteFinalDestination != NOWHERE && !(gTacticalStatus.uiFlags & INCOMBAT) )
					{
						SLOGE("AI {} failed to get path for dialogue-related move!", pSoldier->name);

						// Are we close enough?
						if ( !ACTING_ON_SCHEDULE( pSoldier ) && SpacesAway( pSoldier->sGridNo, pSoldier->sAbsoluteFinalDestination ) < 4 )
						{
							// This is close enough...
							ReplaceLocationInNPCDataFromProfileID( pSoldier->ubProfile, pSoldier->sAbsoluteFinalDestination, pSoldier->sGridNo );
							NPCGotoGridNo( pSoldier->ubProfile, pSoldier->sGridNo, (UINT8) (pSoldier->ubQuoteRecord - 1) );
						}
						else
						{
							// This is important, so try taking a path through people (and bumping them aside)
							if (LegalNPCDestination(pSoldier,pSoldier->usActionData,ENSURE_PATH,WATEROK, PATH_THROUGH_PEOPLE))
							{
								// optimization - Ian: prevent another path call in SetNewCourse()
								pSoldier->bPathStored = TRUE;
							}
							else
							{
								// Have buddy wait a while...
								pSoldier->bNextAction = AI_ACTION_WAIT;
								pSoldier->usNextActionData = (UINT16)REALTIME_AI_DELAY;
							}
						}

						if (!pSoldier->bPathStored)
						{
							CancelAIAction(pSoldier);
							return(FALSE);         // nothing is in progress
						}
					}
					else
					{
						CancelAIAction(pSoldier);
						return(FALSE);         // nothing is in progress
					}
				}
			}

			// add on anything necessary to traverse off map edge
			switch( pSoldier->ubQuoteActionID )
			{
				case QUOTE_ACTION_ID_TRAVERSE_EAST:
					pSoldier->sOffWorldGridNo = pSoldier->usActionData;
					AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->usActionData, EAST );
					break;
				case QUOTE_ACTION_ID_TRAVERSE_SOUTH:
					pSoldier->sOffWorldGridNo = pSoldier->usActionData;
					AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->usActionData, SOUTH );
					break;
				case QUOTE_ACTION_ID_TRAVERSE_WEST:
					pSoldier->sOffWorldGridNo = pSoldier->usActionData;
					AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->usActionData, WEST );
					break;
				case QUOTE_ACTION_ID_TRAVERSE_NORTH:
					pSoldier->sOffWorldGridNo = pSoldier->usActionData;
					AdjustSoldierPathToGoOffEdge( pSoldier, pSoldier->usActionData, NORTH );
					break;
				default:
					break;
			}

			NewDest(pSoldier,pSoldier->usActionData);    // set new ->sDestination to actionData

			// make sure it worked (check that pSoldier->sDestination == pSoldier->usActionData)
			if (pSoldier->sFinalDestination != pSoldier->usActionData)
			{
				// temporarily black list this gridno to stop enemy from going there
				pSoldier->sBlackList = (INT16) pSoldier->usActionData;

				SLOGW("Setting blacklist for {} to {}",
							pSoldier->ubID, pSoldier->sBlackList);

				CancelAIAction(pSoldier);
				return(FALSE);         // nothing is in progress
			}

			// cancel any old black-listed gridno, got a valid new ->sDestination
			pSoldier->sBlackList = NOWHERE;
			break;

		case AI_ACTION_ESCORTED_MOVE:         // go where told to by escortPlayer
			// since this is a delayed move, gotta make sure that it hasn't become
			// illegal since escort orders were issued (->sDestination/route blocked).
			// So treat it like a CONTINUE movement, and handle errors that way
			if (!TryToResumeMovement(pSoldier,pSoldier->usActionData))
			{
				// don't black-list anything here, and action already got canceled
				return(FALSE);         // nothing is in progress
			}

			// cancel any old black-listed gridno, got a valid new ->sDestination
			pSoldier->sBlackList = NOWHERE;
			break;

		case AI_ACTION_TOSS_PROJECTILE:       // throw grenade at/near opponent(s)
			LoadWeaponIfNeeded(pSoldier);
			// fallthrough

		case AI_ACTION_KNIFE_MOVE:            // preparing to stab opponent
			if (pSoldier->bAction == AI_ACTION_KNIFE_MOVE) // if statement because toss falls through
			{
				pSoldier->usUIMovementMode = DetermineMovementMode( pSoldier, AI_ACTION_KNIFE_MOVE );
			}
			// fallthrough

		case AI_ACTION_FIRE_GUN:              // shoot at nearby opponent
		case AI_ACTION_THROW_KNIFE:						// throw knife at nearby opponent
		{
			// randomly decide whether to say civ quote
			if ( pSoldier->bVisible != -1 && pSoldier->bTeam != MILITIA_TEAM )
			{
				// ATE: Make sure it's a person :)
				if ( IS_MERC_BODY_TYPE( pSoldier ) && pSoldier->ubProfile == NO_PROFILE )
				{
					// CC, ATE here - I put in some TEMP randomness...
					if ( Random( 50 ) == 0 )
					{
						StartCivQuote( pSoldier );
					}
				}
			}

			ItemHandleResult const iRetCode = HandleItem(pSoldier, pSoldier->usActionData, pSoldier->bTargetLevel, pSoldier->inv[HANDPOS].usItem, FALSE);
			if ( iRetCode != ITEM_HANDLE_OK)
			{
				if ( iRetCode != ITEM_HANDLE_BROKEN ) // if the item broke, this is 'legal' and doesn't need reporting
				{
					SLOGW("AI {} got error code {} from HandleItem, doing action {}, has {} APs... aborting deadlock!",
								pSoldier->ubID, iRetCode, pSoldier->bAction, pSoldier->bActionPoints);
				}
				CancelAIAction(pSoldier);
				if (gfTurnBasedAI)
				{
					SLOGD("Ending turn for {} because of error from HandleItem", pSoldier->ubID);
				}
				EndAIGuysTurn(*pSoldier);
			}
			break;
		}

		case AI_ACTION_PULL_TRIGGER:          // activate an adjacent panic trigger

			// turn to face trigger first
			if ( FindStructure( (INT16)(pSoldier->sGridNo + DirectionInc( NORTH )), STRUCTURE_SWITCH ) )
			{
				SendSoldierSetDesiredDirectionEvent( pSoldier, NORTH );
			}
			else
			{
				SendSoldierSetDesiredDirectionEvent( pSoldier, WEST );
			}

			EVENT_InitNewSoldierAnim( pSoldier, AI_PULL_SWITCH, 0 , FALSE );

			DeductPoints( pSoldier, AP_PULL_TRIGGER, 0 );

			//gTacticalStatus.fPanicFlags					= 0; // turn all flags off
			gTacticalStatus.the_chosen_one = NULL;
			break;

		case AI_ACTION_USE_DETONATOR:
			//gTacticalStatus.fPanicFlags					= 0; // turn all flags off
			gTacticalStatus.the_chosen_one = NULL;
			//gTacticalStatus.sPanicTriggerGridno	= NOWHERE;

			// grab detonator and set off bomb(s)
			DeductPoints( pSoldier, AP_USE_REMOTE, BP_USE_DETONATOR);// pay for it!
			SetOffPanicBombs(pSoldier, 0);

			// action completed immediately, cancel it right away
			pSoldier->usActionData = NOWHERE;
			pSoldier->bLastAction = pSoldier->bAction;
			pSoldier->bAction = AI_ACTION_NONE;
			return(FALSE);           // no longer in progress

		case AI_ACTION_RED_ALERT:             // tell friends opponent(s) seen
			// if a computer merc, and up to now they didn't know you're here
			if (!(pSoldier->uiStatusFlags & SOLDIER_PC) &&
				(
					!gTacticalStatus.Team[pSoldier->bTeam].bAwareOfOpposition ||
					(
						gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE &&
						gTacticalStatus.the_chosen_one == NULL
					)
				))
			{
				HandleInitialRedAlert(pSoldier->bTeam);
			}
			SLOGD("AI radios your position!" );
			// fallthrough
		case AI_ACTION_YELLOW_ALERT:          // tell friends opponent(s) heard
			SLOGD("Debug: AI radios about a noise!" );
			DeductPoints(pSoldier,AP_RADIO,BP_RADIO);// pay for it!
			RadioSightings(pSoldier,EVERYBODY,pSoldier->bTeam);      // about everybody
			// action completed immediately, cancel it right away

			// ATE: Change to an animation!
			AIDecideRadioAnimation( pSoldier );
			//return(FALSE);           // no longer in progress
			break;

		case AI_ACTION_CREATURE_CALL:									// creature calling to others
			DeductPoints(pSoldier,AP_RADIO,BP_RADIO);// pay for it!
			CreatureCall( pSoldier );
			//return( FALSE ); // no longer in progress
			break;

		case AI_ACTION_CHANGE_STANCE:                // crouch
			if ( gAnimControl[ pSoldier->usAnimState ].ubHeight == pSoldier->usActionData )
			{
				// abort!
				ActionDone( pSoldier );
				return( FALSE );
			}

			SkipCoverCheck = TRUE;
			SLOGD("ExecuteAction: SkipCoverCheck ON");
			ChangeSoldierStance(pSoldier, pSoldier->usActionData);
			break;

		case AI_ACTION_COWER:
			// make sure action data is set right
			if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
			{
				// nothing to do!
				ActionDone( pSoldier );
				return( FALSE );
			}
			else
			{
				pSoldier->usActionData = ANIM_CROUCH;
				SetSoldierCowerState( pSoldier, TRUE );
			}
			break;

		case AI_ACTION_STOP_COWERING:
			// make sure action data is set right
			if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
			{
				pSoldier->usActionData = ANIM_STAND;
				SetSoldierCowerState( pSoldier, FALSE );
			}
			else
			{
				// nothing to do!
				ActionDone( pSoldier );
				return( FALSE );
			}
			break;

		case AI_ACTION_GIVE_AID:              // help injured/dying friend
		{
			//pSoldier->usUIMovementMode = RUNNING;
			ItemHandleResult const iRetCode = HandleItem(pSoldier, pSoldier->usActionData, 0, pSoldier->inv[HANDPOS].usItem, FALSE);
			if ( iRetCode != ITEM_HANDLE_OK)
			{
				SLOGW("AI {} got error code {} from HandleItem, doing action {}... aborting deadlock!",
							pSoldier->ubID, iRetCode, pSoldier->bAction);
				CancelAIAction(pSoldier);
				EndAIGuysTurn(*pSoldier);
			}
			break;
		}

		case AI_ACTION_OPEN_OR_CLOSE_DOOR:
		case AI_ACTION_UNLOCK_DOOR:
		case AI_ACTION_LOCK_DOOR:
			{
				STRUCTURE *pStructure;
				INT8      bDirection;
				INT16     sDoorGridNo;

				bDirection = (INT8) GetDirectionFromGridNo( pSoldier->usActionData, pSoldier );
				if (bDirection == EAST || bDirection == SOUTH)
				{
					sDoorGridNo = pSoldier->sGridNo;
				}
				else
				{
					sDoorGridNo = pSoldier->sGridNo + DirectionInc( bDirection );
				}

				pStructure = FindStructure( sDoorGridNo, STRUCTURE_ANYDOOR );
				if (pStructure == NULL)
				{
					SLOGD("AI {} tried to open door it could not then find in {}",
								pSoldier->ubID, sDoorGridNo );
					CancelAIAction(pSoldier);
					EndAIGuysTurn(*pSoldier);
					break;
				}

				StartInteractiveObject(sDoorGridNo, *pStructure, *pSoldier, bDirection);
				InteractWithOpenableStruct(*pSoldier, *pStructure, bDirection);
			}
			break;

		case AI_ACTION_LOWER_GUN:
			// for now, just do "action done"
			ActionDone( pSoldier );
			break;

		case AI_ACTION_CLIMB_ROOF:
			if (pSoldier->bLevel == 0)
			{
				BeginSoldierClimbUpRoof( pSoldier );
			}
			else
			{
				BeginSoldierClimbDownRoof( pSoldier );
			}
			break;

		case AI_ACTION_END_TURN:
			ActionDone( pSoldier );
			if (gfTurnBasedAI)
			{
				EndAIGuysTurn(*pSoldier);
			}
			return( FALSE );         // nothing is in progress

		case AI_ACTION_TRAVERSE_DOWN:
			if (gfTurnBasedAI)
			{
				EndAIGuysTurn(*pSoldier);
			}
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				gMercProfiles[ pSoldier->ubProfile ].bSectorZ++;
				gMercProfiles[ pSoldier->ubProfile ].fUseProfileInsertionInfo = FALSE;
			}
			TacticalRemoveSoldier(*pSoldier);
			CheckForEndOfBattle( TRUE );

			return( FALSE );         // nothing is in progress

		case AI_ACTION_OFFER_SURRENDER:
			// start the offer of surrender!
			StartCivQuote( pSoldier );
			break;

		default:
			return(FALSE);
	}

	// return status indicating execution of action was properly started
	return(TRUE);
}

void CheckForChangingOrders(SOLDIERTYPE *pSoldier)
{
	switch( pSoldier->bAlertStatus )
	{
		case STATUS_GREEN:
			if ( !CREATURE_OR_BLOODCAT( pSoldier ) )
			{
				if ( pSoldier->bTeam == CIV_TEAM && pSoldier->ubProfile != NO_PROFILE && pSoldier->bNeutral && gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo != NOWHERE && pSoldier->ubCivilianGroup != QUEENS_CIV_GROUP )
				{
					// must make them uncower first, then return to start location
					pSoldier->bNextAction = AI_ACTION_END_COWER_AND_MOVE;
					pSoldier->usNextActionData = gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo;
					gMercProfiles[ pSoldier->ubProfile ].sPreCombatGridNo = NOWHERE;
				}
				else if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
				{
					pSoldier->bNextAction = AI_ACTION_STOP_COWERING;
					pSoldier->usNextActionData = ANIM_STAND;
				}
				else
				{
					pSoldier->bNextAction = AI_ACTION_CHANGE_STANCE;
					pSoldier->usNextActionData = ANIM_STAND;
				}
			}
			break;
		case STATUS_YELLOW:
			break;
		default:
			if ((pSoldier->bOrders == ONGUARD) || (pSoldier->bOrders == CLOSEPATROL))
			{
				// crank up ONGUARD to CLOSEPATROL, and CLOSEPATROL to FARPATROL
				pSoldier->bOrders++;       // increase roaming range by 1 category
			}
			else if ( pSoldier->bTeam == MILITIA_TEAM )
			{
				// go on alert!
				pSoldier->bOrders = SEEKENEMY;
			}
			else if ( CREATURE_OR_BLOODCAT( pSoldier ) )
			{
				if (pSoldier->bOrders != STATIONARY && pSoldier->bOrders != ONCALL)
				{
					pSoldier->bOrders = SEEKENEMY;
				}
			}

			if ( pSoldier->ubProfile == WARDEN )
			{
				// Tixa
				MakeClosestEnemyChosenOne();
			}
			break;
	}
}

void InitAttackType(ATTACKTYPE *pAttack)
{
	// initialize the given bestAttack structure fields to their default values
	pAttack->ubPossible          = FALSE;
	pAttack->opponent            = NULL;
	pAttack->ubAimTime           = 0;
	pAttack->ubChanceToReallyHit = 0;
	pAttack->sTarget             = NOWHERE;
	pAttack->iAttackValue        = 0;
	pAttack->ubAPCost            = 0;
}

void HandleInitialRedAlert(INT8 bTeam)
{
	if (!gTacticalStatus.Team[bTeam].bAwareOfOpposition)
	{
		SLOGD("Enemies on team {} prompted to go on RED ALERT!", bTeam);
	}

	// if there is a stealth mission in progress here, and a panic trigger exists
	if ( bTeam == ENEMY_TEAM && (gTacticalStatus.fPanicFlags & PANIC_TRIGGERS_HERE) )
	{
		// they're going to be aware of us now!
		MakeClosestEnemyChosenOne();
	}

	static const SGPSector meduna(3, MAP_ROW_P);
	if (bTeam == ENEMY_TEAM && gWorldSector == meduna)
	{
		// alert Queen and Joe if they are around
		SOLDIERTYPE *			pSoldier;

		pSoldier = FindSoldierByProfileID(QUEEN);
		if ( pSoldier )
		{
			pSoldier->bAlertStatus = STATUS_RED;
		}

		pSoldier = FindSoldierByProfileID(JOE);
		if ( pSoldier )
		{
			pSoldier->bAlertStatus = STATUS_RED;
		}
	}

	// remember enemies are alerted, prevent another red alert from happening
	gTacticalStatus.Team[ bTeam ].bAwareOfOpposition = TRUE;
}


static void ManChecksOnFriends(SOLDIERTYPE* pSoldier)
{
	INT16 sDistVisible;

	// THIS ROUTINE SHOULD ONLY BE CALLED FOR SOLDIERS ON STATUS GREEN or YELLOW

	// go through each soldier, looking for "friends" (soldiers on same side)
	for (auto const * const pFriend : ActiveMercs())
	{
		// if this man is neutral / NOT on my side, he's not my friend
		if (pFriend->bNeutral || (pSoldier->bSide != pFriend->bSide))
			continue;  // next merc

		// if this merc is actually ME
		if (pFriend == pSoldier) continue; // next merc

		sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, pFriend->sGridNo, pFriend->bLevel );
		// if we can see far enough to see this friend
		if (PythSpacesAway(pSoldier->sGridNo,pFriend->sGridNo) <= sDistVisible)
		{
			// and can trace a line of sight to his x,y coordinates
			//if (1) //*** SoldierToSoldierLineOfSightTest(pSoldier,pFriend,STRAIGHT,TRUE))
			if (SoldierToSoldierLineOfSightTest(pSoldier, pFriend, (UINT8)sDistVisible, TRUE))
			{
				// if my friend is in battle or something is clearly happening there
				if ((pFriend->bAlertStatus >= STATUS_RED) || pFriend->bUnderFire || (pFriend->bLife < OKLIFE))
				{
					pSoldier->bAlertStatus = STATUS_RED;
					CheckForChangingOrders(pSoldier);
					SetNewSituation( pSoldier );
					break;         // don't bother checking on any other friends
				}
				else
				{
					// if he seems suspicious or acts like he thought he heard something
					// and I'm still on status GREEN
					if ((pFriend->bAlertStatus == STATUS_YELLOW) &&
						(pSoldier->bAlertStatus < STATUS_YELLOW))
					{
						pSoldier->bAlertStatus = STATUS_YELLOW;    // also get suspicious
						SetNewSituation( pSoldier );
						pSoldier->sNoiseGridno = pFriend->sGridNo;  // pretend FRIEND made noise
						pSoldier->ubNoiseVolume = 3;                // remember this for 3 turns
						// keep check other friends, too, in case any are already on RED
					}
				}
			}
		}
	}
}


void SetNewSituation( SOLDIERTYPE * pSoldier )
{
	if ( pSoldier->bTeam != OUR_TEAM )
	{
		if ( pSoldier->ubQuoteRecord == 0 && !gTacticalStatus.fAutoBandageMode && !(pSoldier->bNeutral && gTacticalStatus.uiFlags & ENGAGED_IN_CONV) )
		{
			// allow new situation to be set
			pSoldier->bNewSituation = IS_NEW_SITUATION;

			if ( gTacticalStatus.ubAttackBusyCount != 0 )
			{
				SLOGD("bNewSituation is set for {} when ABC !=0.", pSoldier->ubID);
			}

			if (!(gTacticalStatus.uiFlags & INCOMBAT))
			{
				// reset delay if necessary!
				RESETTIMECOUNTER( pSoldier->AICounter, Random( 1000 ) );
			}
		}
	}
}


static void HandleAITacticalTraversal(SOLDIERTYPE& s)
{
	HandleNPCChangesForTacticalTraversal(&s);

	if (s.ubProfile != NO_PROFILE &&
		NPCHasUnusedRecordWithGivenApproach(s.ubProfile, APPROACH_DONE_TRAVERSAL))
	{
		GetProfile(s.ubProfile).ubMiscFlags3 |= PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL;
	}
	else
	{
		s.ubQuoteActionID = 0;
	}
	if (gfTurnBasedAI)
	{
		SLOGD("Ending turn for {} because traversing out", s.ubID);
	}

	EndAIGuysTurn(s);
	RemoveManAsTarget(&s);
	if (s.bTeam == CIV_TEAM && s.fAIFlags & AI_CHECK_SCHEDULE)
	{
		MoveSoldierFromMercToAwaySlot(&s);
		s.bInSector = FALSE;
	}
	else
	{
		ProcessQueenCmdImplicationsOfDeath(&s);
		TacticalRemoveSoldier(s);
	}
	CheckForEndOfBattle(TRUE);
}
