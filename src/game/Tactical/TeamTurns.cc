#include "LoadSaveData.h"
#include "Timer_Control.h"
#include "Types.h"
#include "FileMan.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Animation_Control.h"
#include "Points.h"
#include "OppList.h"
#include "Sound_Control.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Font_Control.h"
#include "AI.h"
#include "Message.h"
#include "Text.h"
#include "TeamTurns.h"
#include "Smell.h"
#include "Game_Clock.h"
#include "GameSettings.h"
#include "Soldier_Functions.h"
#include "Queen_Command.h"
#include "PathAI.h"
#include "Strategic_Turns.h"
#include "Lighting.h"
#include "Environment.h"
#include "Explosion_Control.h"
#include "Dialogue_Control.h"
#include "Soldier_Profile_Type.h"
#include "SmokeEffects.h"
#include "LightEffects.h"
#include "Meanwhile.h"
#include "SkillCheck.h"
#include "AIInternals.h"
#include "AIList.h"
#include "RenderWorld.h"
#include "Rotting_Corpses.h"
#include "Squads.h"
#include "Soldier_Macros.h"
#include "NPC.h"
#include "Debug.h"
#include "Items.h"
#include "Logger.h"

// for that single policy check :|
#include "GamePolicy.h"
#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>

static SOLDIERTYPE* gOutOfTurnOrder[MAXMERCS];
UINT8 gubOutOfTurnPersons = 0;


static inline SOLDIERTYPE* LatestInterruptGuy(void)
{
	return gOutOfTurnOrder[gubOutOfTurnPersons];
}


#define REMOVE_LATEST_INTERRUPT_GUY()	(DeleteFromIntList( (UINT8) (gubOutOfTurnPersons), TRUE ))
#define INTERRUPTS_OVER		(gubOutOfTurnPersons == 1)

BOOLEAN gfHiddenInterrupt = FALSE;
static SOLDIERTYPE* gLastInterruptedGuy = NULL;

extern SightFlags gubSightFlags;


#define MIN_APS_TO_INTERRUPT		4

void ClearIntList( void )
{
	std::fill(std::begin(gOutOfTurnOrder), std::end(gOutOfTurnOrder), nullptr);
	gubOutOfTurnPersons = 0;
}


void StartPlayerTeamTurn( BOOLEAN fDoBattleSnd, BOOLEAN fEnteringCombatMode )
{
	// Start the turn of player charactors

	//
	// PATCH 1.06:
	//
	// make sure set properly in gTacticalStatus:
	gTacticalStatus.ubCurrentTeam = OUR_TEAM;

	InitPlayerUIBar( FALSE );


	// Are we in combat already?
	if ( gTacticalStatus.uiFlags & INCOMBAT )
	{
		PlayJA2Sample(ENDTURN_1, MIDVOLUME, 1, MIDDLEPAN);
	}

	// Check for victory conditions

	// Are we in combat already?
	if ( gTacticalStatus.uiFlags & INCOMBAT )
	{
		SOLDIERTYPE* sel = GetSelectedMan();
		if (sel != NULL)
		{
			// Check if this guy is able to be selected....
			if (sel->bLife < OKLIFE)
			{
				SelectNextAvailSoldier(sel);
				sel = GetSelectedMan();
			}

			// Slide to selected guy...
			if (sel != NULL)
			{
				SlideTo(sel, SETLOCATOR);

				// Say ATTENTION SOUND...
				if (fDoBattleSnd) DoMercBattleSound(sel, BATTLE_SOUND_ATTN1);

				if ( gsInterfaceLevel == 1 )
				{
					gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
					InvalidateWorldRedundency( );
					SetRenderFlags(RENDER_FLAG_FULL);
					ErasePath();
				}
			}
		}
	}

	// Dirty panel interface!
	fInterfacePanelDirty = DIRTYLEVEL2;

	// Adjust time now!
	UpdateClock( );

	if ( !fEnteringCombatMode )
	{
		CheckForEndOfCombatMode( TRUE );
	}


	// Signal UI done enemy's turn
	guiPendingOverrideEvent = LU_ENDUILOCK;

	// ATE: Reset killed on attack variable.. this is because sometimes timing is such
	/// that a baddie can die and still maintain it's attacker ID
	gTacticalStatus.fKilledEnemyOnAttack = FALSE;

	// Save if we are in Dead is Dead mode
	DoDeadIsDeadSaveIfNecessary();

	HandleTacticalUI( );
}


static void FreezeInterfaceForEnemyTurn(void)
{
	// Reset flags
	gfPlotNewMovement = TRUE;

	// Erase path
	ErasePath();

	// Setup locked UI
	guiPendingOverrideEvent = LU_BEGINUILOCK;

	// Remove any UI messages!
	if (g_ui_message_overlay != NULL)
	{
		EndUIMessage( );
	}
}


static void EndInterrupt(BOOLEAN fMarkInterruptOccurred);


void EndTurn( UINT8 ubNextTeam )
{
	//Check for enemy pooling (add enemies if there happens to be more than the max in the
	//current battle.  If one or more slots have freed up, we can add them now.

	// Save if we are in Dead is Dead mode
	DoDeadIsDeadSaveIfNecessary();

	/*
	if ( CheckForEndOfCombatMode( FALSE ) )
	{
		return;
	}
	*/

	if (INTERRUPT_QUEUED)
	{
		EndInterrupt( FALSE );
	}
	else
	{
		AddPossiblePendingEnemiesToBattle();

		//InitEnemyUIBar( );

		FreezeInterfaceForEnemyTurn();

		// Loop through all mercs and set to moved
		FOR_EACH_IN_TEAM(s, gTacticalStatus.ubCurrentTeam)
		{
			// Only do this for mercs that are actually in this sector
			if (!s->bInSector) continue;

			s->bMoved = TRUE;
			// Cancel merc movement if continue path was not used
			const INT16 sAPCost = PtsToMoveDirection(s, s->ubPathingData[0]);
			if (EnoughPoints(s, sAPCost, 0, FALSE))
			{
				s->sFinalDestination=s->sGridNo;
				s->fNoAPToFinishMove = 0;
			}
		}

		gTacticalStatus.ubCurrentTeam  = ubNextTeam;

		BeginTeamTurn( gTacticalStatus.ubCurrentTeam );

		BetweenTurnsVisibilityAdjustments();
	}
}

void EndAITurn( void )
{
	if (INTERRUPT_QUEUED)
	{
		EndInterrupt( FALSE );
	}
	else
	{
		FOR_EACH_IN_TEAM(s, gTacticalStatus.ubCurrentTeam)
		{
			s->bMoved = TRUE;
			// record old life value... for creature AI; the human AI might
			// want to use this too at some point
			s->bOldLife = s->bLife;
		}

		gTacticalStatus.ubCurrentTeam++;
		BeginTeamTurn( gTacticalStatus.ubCurrentTeam );
	}
}

void EndAllAITurns( void )
{
	// warp turn to the player's turn

	if (INTERRUPT_QUEUED)
	{
		EndInterrupt( FALSE );
	}

	if ( gTacticalStatus.ubCurrentTeam != OUR_TEAM )
	{
		FOR_EACH_IN_TEAM(s, gTacticalStatus.ubCurrentTeam)
		{
			s->bMoved = TRUE;
			s->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
			// record old life value... for creature AI; the human AI might
			// want to use this too at some point
			s->bOldLife = s->bLife;
		}

		gTacticalStatus.ubCurrentTeam = OUR_TEAM;
		//BeginTeamTurn( gTacticalStatus.ubCurrentTeam );
	}
}


static void EndTurnEvents(void)
{
	// HANDLE END OF TURN EVENTS
	// handle team services like healing
	HandleTeamServices( OUR_TEAM );
	// handle smell and blood decay
	DecaySmells();
	// decay bomb timers and maybe set some off!
	DecayBombTimers();

	DecaySmokeEffects(GetWorldTotalSeconds(), true);
	DecayLightEffects(GetWorldTotalSeconds(), true);

	// decay AI warning values from corpses
	DecayRottingCorpseAIWarnings();
}


void BeginTeamTurn( UINT8 ubTeam )
{
	while( 1 )
	{
		if ( ubTeam > LAST_TEAM )
		{
			// End turn!!
			ubTeam = OUR_TEAM;
			gTacticalStatus.ubCurrentTeam = OUR_TEAM;
			EndTurnEvents();

		}
		else if (!IsTeamActive(ubTeam))
		{
			// inactive team, skip to the next one
			ubTeam++;
			gTacticalStatus.ubCurrentTeam++;
			// skip back to the top, as we are processing another team now.
			continue;
		}


		BeginLoggingForBleedMeToos( TRUE );

		// decay team's public opplist
		DecayPublicOpplist( ubTeam );

		FOR_EACH_IN_TEAM(i, ubTeam)
		{
			SOLDIERTYPE& s = *i;
			if (s.bLife <= 0) continue;
			// decay personal opplist, and refresh APs and BPs
			EVENT_BeginMercTurn(s);
		}

		if (gTacticalStatus.bBoxingState == LOST_ROUND || gTacticalStatus.bBoxingState == WON_ROUND || gTacticalStatus.bBoxingState == DISQUALIFIED )
		{
			// we have no business being in here any more!
			return;
		}

		BeginLoggingForBleedMeToos( FALSE );



		if (ubTeam == OUR_TEAM )
		{
			// ATE: Check if we are still in a valid battle...
			// ( they could have blead to death above )
			if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				StartPlayerTeamTurn( TRUE, FALSE );
			}
			break;
		}
		else
		{
			// Set First enemy merc to AI control
			if ( BuildAIListForTeam( ubTeam ) )
			{
				SOLDIERTYPE* const s = RemoveFirstAIListEntry();
				if (s != NULL)
				{
					// Dirty panel interface!
					fInterfacePanelDirty = DIRTYLEVEL2;
					AddTopMessage(COMPUTER_TURN_MESSAGE);
					StartNPCAI(*s);
					return;
				}
			}

			// This team is dead/inactive/being skipped in boxing
			// skip back to the top to process the next team
			ubTeam++;
			gTacticalStatus.ubCurrentTeam++;
		}
	}
}

void DisplayHiddenInterrupt( SOLDIERTYPE * pSoldier )
{
	// If the AI got an interrupt but this has been hidden from the player until this point,
	// this code will display the interrupt

	if (!gfHiddenInterrupt)
	{
		return;
	}

	if (pSoldier->bVisible != -1) SlideTo(pSoldier, SETLOCATOR);

	guiPendingOverrideEvent = LU_BEGINUILOCK;

	// Dirty panel interface!
	fInterfacePanelDirty = DIRTYLEVEL2;

	// Erase path!
	ErasePath();

	// Reset flags
	gfPlotNewMovement = TRUE;

	// Stop our guy....
	SOLDIERTYPE* const latest = LatestInterruptGuy();
	if (latest)
	{
		AdjustNoAPToFinishMove(latest, TRUE);
		// Stop him from going to prone position if doing a turn while prone
		latest->fTurningFromPronePosition = FALSE;
	}

	// get rid of any old overlay message
	const MESSAGE_TYPES msg =
		pSoldier->bTeam == MILITIA_TEAM ? MILITIA_INTERRUPT_MESSAGE:
					COMPUTER_INTERRUPT_MESSAGE;
	AddTopMessage(msg);

	gfHiddenInterrupt = FALSE;
}

void DisplayHiddenTurnbased( SOLDIERTYPE * pActingSoldier )
{
	// This code should put the game in turn-based and give control to the AI-controlled soldier
	// whose pointer has been passed in as an argument (we were in non-combat and the AI is doing
	// something visible, i.e. making an attack)

	if ( AreInMeanwhile( ) )
	{
		return;
	}

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		// pointless call here; do nothing
		return;
	}

	// Enter combat mode starting with this side's turn
	gTacticalStatus.ubCurrentTeam  = pActingSoldier->bTeam;

	CommonEnterCombatModeCode( );

	SetSoldierAsUnderAiControl( pActingSoldier );
	SLOGD("Giving AI control to {}", pActingSoldier->ubID);
	pActingSoldier->fTurnInProgress = TRUE;
	gTacticalStatus.uiTimeSinceMercAIStart = GetJA2Clock();

	if ( gTacticalStatus.ubTopMessageType != COMPUTER_TURN_MESSAGE)
	{
		// Dirty panel interface!
		fInterfacePanelDirty = DIRTYLEVEL2;
		AddTopMessage(COMPUTER_TURN_MESSAGE);
	}

	// freeze the user's interface
	FreezeInterfaceForEnemyTurn();
}


static BOOLEAN EveryoneInInterruptListOnSameTeam(void)
{
	UINT8 ubLoop;
	UINT8 ubTeam = 255;

	for (ubLoop = 1; ubLoop <= gubOutOfTurnPersons; ubLoop++)
	{
		if ( ubTeam == 255 )
		{
			ubTeam = gOutOfTurnOrder[ubLoop]->bTeam;
		}
		else
		{
			if (gOutOfTurnOrder[ubLoop]->bTeam != ubTeam)
			{
				return( FALSE );
			}
		}
	}
	return( TRUE );
}


void SayCloseCallQuotes(void)
{
	// report any close call quotes for us here
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (OkControllableMerc(s) &&
			s->fCloseCall &&
			s->bNumHitsThisTurn == 0 &&
			!(s->usQuoteSaidExtFlags & SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL) &&
			Random(3) == 0)
		{
			// say close call quote!
			TacticalCharacterDialogue(s, QUOTE_CLOSE_CALL);
			s->usQuoteSaidExtFlags |= SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL;
		}
		s->fCloseCall = FALSE;
	}
}


static void DeleteFromIntList(UINT8 ubIndex, BOOLEAN fCommunicate);


static void StartInterrupt(void)
{
	SOLDIERTYPE *first_interrupter = LatestInterruptGuy();
	const INT8  bTeam = first_interrupter->bTeam;
	SOLDIERTYPE *Interrupter = first_interrupter;

	// display everyone on int queue!
	for (INT32 cnt = gubOutOfTurnPersons; cnt > 0; --cnt)
	{
		SLOGD("STARTINT: Q position {}: {}", cnt, gOutOfTurnOrder[cnt]->ubID);
	}

	gTacticalStatus.fInterruptOccurred = TRUE;

	FOR_EACH_SOLDIER(s)
	{
		s->bMovedPriorToInterrupt = s->bMoved;
		s->bMoved                 = TRUE;
	}

	if (first_interrupter->bTeam == OUR_TEAM)
	{
		// start interrupts for everyone on our side at once
		UINT8   ubInterrupters = 0;

		// build string for display of who gets interrupt
		while( 1 )
		{
			Interrupter->bMoved = FALSE;
			SLOGD("INTERRUPT: popping {} off of the interrupt queue", Interrupter->ubID);

			REMOVE_LATEST_INTERRUPT_GUY();
			// now LatestInterruptGuy() is the guy before the previous
			Interrupter = LatestInterruptGuy();

			if (Interrupter == NULL) // previously emptied slot!
			{
				continue;
			}
			else if (Interrupter->bTeam != bTeam)
			{
				break;
			}
		}

		ST::string sTemp = g_langRes->Message[STR_INTERRUPT_FOR];

		// build string in separate loop here, want to linearly process squads...
		for (INT32 iSquad = 0; iSquad < NUMBER_OF_SQUADS; ++iSquad)
		{
			FOR_EACH_IN_SQUAD(i, iSquad)
			{
				SOLDIERTYPE const* const s = *i;
				if (!s->bActive)   continue;
				if (!s->bInSector) continue;
				if (s->bMoved)     continue;
				// then this guy got an interrupt...
				ubInterrupters++;
				if ( ubInterrupters > 6 )
				{
					// flush... display string, then clear it (we could have 20 names!)
					// add comma to end, we know we have another person after this...
					sTemp += ", ";
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sTemp );
					sTemp.clear();
					ubInterrupters = 1;
				}

				if ( ubInterrupters > 1 )
				{
					sTemp += ", ";
				}
				sTemp += s->name;
			}
		}

		if (!sTemp.empty())
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sTemp);
		}

		SLOGD("INTERRUPT: starting interrupt for {}", first_interrupter->ubID);

		// Select guy....
		SelectSoldier(first_interrupter, SELSOLDIER_ACKNOWLEDGE | SELSOLDIER_FORCE_RESELECT);

		// ATE; Slide to guy who got interrupted!
		SlideTo(gLastInterruptedGuy, SETLOCATOR);

		// Dirty panel interface!
		fInterfacePanelDirty = DIRTYLEVEL2;
		gTacticalStatus.ubCurrentTeam   = first_interrupter->bTeam;

		// Signal UI done enemy's turn
		guiPendingOverrideEvent = LU_ENDUILOCK;
		HandleTacticalUI( );

		InitPlayerUIBar( TRUE );
		//AddTopMessage(PLAYER_INTERRUPT_MESSAGE);

		PlayJA2Sample(ENDTURN_1, MIDVOLUME, 1, MIDDLEPAN);

		SayCloseCallQuotes();
	}
	else
	{
		// start interrupts for everyone on that side at once... and start AI with the lowest # guy

		// what we do is set everyone to moved except for people with interrupts at the moment
		/*
		FOR_EACH_IN_TEAM(s, first_interrupter->bTeam)
		{
			s->bMovedPriorToInterrupt = s->bMoved;
			s->bMoved                 = TRUE;
		}
		*/

		while( 1 )
		{
			Interrupter->bMoved = FALSE;

			SLOGD("INTERRUPT: popping {} off of the interrupt queue", Interrupter->ubID);

			REMOVE_LATEST_INTERRUPT_GUY();
			// now LatestInterruptGuy() is the guy before the previous
			Interrupter = LatestInterruptGuy();
			if (Interrupter == NULL) // previously emptied slot!
			{
				continue;
			}
			else if (Interrupter->bTeam != bTeam)
			{
				break;
			}
			else if (Interrupter->ubID < first_interrupter->ubID)
			{
				first_interrupter = Interrupter;
			}
		}


		// here we have to rebuilt the AI list!
		BuildAIListForTeam( bTeam );

		// set to the new first interrupter
		SOLDIERTYPE* const pSoldier = RemoveFirstAIListEntry();

		//if ( gTacticalStatus.ubCurrentTeam == OUR_TEAM)
		if ( pSoldier->bTeam != OUR_TEAM )
		{
			// we're being interrupted by the computer!
			// we delay displaying any interrupt message until the computer
			// does something...
			gfHiddenInterrupt = TRUE;
			gTacticalStatus.fUnLockUIAfterHiddenInterrupt = FALSE;
		}
		// otherwise it's the AI interrupting another AI team

		gTacticalStatus.ubCurrentTeam  = pSoldier->bTeam;

		SLOGD("Interrupt ( could be hidden )" );

		StartNPCAI(*pSoldier);
	}

	if ( !gfHiddenInterrupt )
	{
		// Stop this guy....
		SOLDIERTYPE* const latest = LatestInterruptGuy();
		AdjustNoAPToFinishMove(latest, TRUE);
		latest->fTurningFromPronePosition = FALSE;
	}
}


static void EndInterrupt(BOOLEAN fMarkInterruptOccurred)
{
	BOOLEAN					fFound;
	UINT8						ubMinAPsToAttack;

	for (INT32 cnt = gubOutOfTurnPersons; cnt > 0; --cnt)
	{
		SLOGD("ENDINT: Q position {}: {}", cnt, gOutOfTurnOrder[cnt]->ubID);
	}

	// ATE: OK, now if this all happended on one frame, we may not have to stop
	// guy from walking... so set this flag to false if so...
	if ( fMarkInterruptOccurred )
	{
		// flag as true if an int occurs which ends an interrupt (int loop)
		gTacticalStatus.fInterruptOccurred = TRUE;
	}
	else
	{
		gTacticalStatus.fInterruptOccurred = FALSE;
	}

	// Loop through all mercs and see if any passed on this interrupt
	FOR_EACH_IN_TEAM(s, gTacticalStatus.ubCurrentTeam)
	{
		if (s->bInSector && !s->bMoved && s->bActionPoints == s->bIntStartAPs)
		{
			ubMinAPsToAttack = MinAPsToAttack(s, s->sLastTarget, FALSE);
			if (0 < ubMinAPsToAttack && ubMinAPsToAttack <= s->bActionPoints)
			{
				s->bPassedLastInterrupt = TRUE;
			}
		}
	}

	if ( !EveryoneInInterruptListOnSameTeam() )
	{
		gfHiddenInterrupt = FALSE;

		// resume interrupted interrupt
		StartInterrupt();
	}
	else
	{
		SOLDIERTYPE* const interrupted = LatestInterruptGuy();
		SLOGD("INTERRUPT: interrupt over, {}'s team regains control", interrupted->ubID);

		FOR_EACH_SOLDIER(s)
		{
			// AI guys only here...
			if (s->bActionPoints == 0)
			{
				s->bMoved = TRUE;
			}
			else if (s->bTeam != OUR_TEAM && s->bNewSituation == IS_NEW_SITUATION)
			{
				s->bMoved = FALSE;
			}
			else
			{
				s->bMoved = s->bMovedPriorToInterrupt;
			}
		}

		// change team
		gTacticalStatus.ubCurrentTeam = interrupted->bTeam;
		// switch appropriate messages & flags
		if (interrupted->bTeam == OUR_TEAM)
		{
			// set everyone on the team to however they were set moved before the interrupt
			// must do this before selecting soldier...
			/*
			FOR_EACH_IN_TEAM(s, gTacticalStatus.ubCurrentTeam)
			{
				s->bMoved = s->bMovedPriorToInterrupt;
			}
			*/

			ClearIntList();

			// Select soldier....
			if (interrupted->bLife < OKLIFE)
			{
				SelectNextAvailSoldier(interrupted);
			}
			else
			{
				SelectSoldier(interrupted, SELSOLDIER_NONE);
			}

			if (gfHiddenInterrupt)
			{
				// Try to make things look like nothing happened at all.
				gfHiddenInterrupt = FALSE;

				// If we can continue a move, do so!
				SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel->fNoAPToFinishMove && interrupted->ubReasonCantFinishMove != REASON_STOPPED_SIGHT)
				{
					// Continue
					AdjustNoAPToFinishMove(sel, FALSE);

					if (sel->sGridNo != sel->sFinalDestination)
					{
						EVENT_GetNewSoldierPath(sel, sel->sFinalDestination, sel->usUIMovementMode);
					}
					else
					{
						UnSetUIBusy(interrupted);
					}
				}
				else
				{
					UnSetUIBusy(interrupted);
				}

				if ( gTacticalStatus.fUnLockUIAfterHiddenInterrupt )
				{
					gTacticalStatus.fUnLockUIAfterHiddenInterrupt = FALSE;
					UnSetUIBusy(interrupted);
				}
			}
			else
			{
				// Signal UI done enemy's turn
				/// ATE: This used to be ablow so it would get done for
				// both hidden interrupts as well - NOT good because
				// hidden interrupts should leave it locked if it was already...
				guiPendingOverrideEvent = LU_ENDUILOCK;
				HandleTacticalUI( );

				SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel != NULL)
				{
					SlideTo(sel, SETLOCATOR);

					// Say ATTENTION SOUND...
					DoMercBattleSound(sel, BATTLE_SOUND_ATTN1);

					if ( gsInterfaceLevel == 1 )
					{
						gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
						InvalidateWorldRedundency( );
						SetRenderFlags(RENDER_FLAG_FULL);
						ErasePath();
					}
				}
				// 2 indicates that we're ending an interrupt and going back to
				// normal player's turn without readjusting time left in turn (for
				// timed turns)
				InitPlayerUIBar( 2 );
			}

		}
		else
		{
			// this could be set to true for AI-vs-AI interrupts
			gfHiddenInterrupt = FALSE;

			// Dirty panel interface!
			fInterfacePanelDirty = DIRTYLEVEL2;

			// Erase path!
			ErasePath();

			// Reset flags
			gfPlotNewMovement = TRUE;

			// restart AI with first available soldier
			fFound = FALSE;

			// rebuild list for this team if anyone on the team is still available
			INT32 cnt = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
			for (SOLDIERTYPE* pTempSoldier = &GetMan(cnt); cnt <= gTacticalStatus.Team[gTacticalStatus.ubCurrentTeam].bLastID; cnt++, pTempSoldier++)
			{
				if ( pTempSoldier->bActive && pTempSoldier->bInSector && pTempSoldier->bLife >= OKLIFE )
				{
					fFound = TRUE;
					break;
				}
			}

			if ( fFound )
			{
				// reset found flag because we are rebuilding the AI list
				fFound = FALSE;

				if ( BuildAIListForTeam( gTacticalStatus.ubCurrentTeam ) )
				{
					// now bubble up everyone left in the interrupt queue, starting
					// at the front of the array
					for (INT32 cnt = 1; cnt <= gubOutOfTurnPersons; ++cnt)
					{
						MoveToFrontOfAIList(gOutOfTurnOrder[cnt]);
					}

					SOLDIERTYPE* const s = RemoveFirstAIListEntry();
					if (s != NULL)
					{
						fFound = TRUE;
						StartNPCAI(*s);
					}
				}

			}

			AddTopMessage(COMPUTER_TURN_MESSAGE);

			// Signal UI done enemy's turn
			guiPendingOverrideEvent = LU_BEGINUILOCK;

			ClearIntList();

			if (!fFound) EndAITurn();
		}

		// Reset our interface!
		fInterfacePanelDirty = DIRTYLEVEL2;

	}
}


BOOLEAN StandardInterruptConditionsMet(const SOLDIERTYPE* const pSoldier, const SOLDIERTYPE* const pOpponent, const INT8 bOldOppList)
{
	//UINT8 ubAniType;
	UINT8 ubMinPtsNeeded;
	INT8  bDir;

	if ((gTacticalStatus.uiFlags & INCOMBAT) && !(gubSightFlags & SIGHT_INTERRUPT))
	{
		return( FALSE );
	}

	if ( gTacticalStatus.ubAttackBusyCount > 0 )
	{
		return( FALSE );
	}

	if (pOpponent == NULL)
	{
		// no opponent, so controller of 'ptr' makes the call instead
		// ALEX
		if (gWhoThrewRock == NULL)
		{
			return(FALSE);
		}
	}

	// in non-combat allow interrupt points to be calculated freely (everyone's in control!)
	// also allow calculation for storing in AllTeamsLookForAll
	if ( (gTacticalStatus.uiFlags & INCOMBAT) && ( gubBestToMakeSightingSize != BEST_SIGHTING_ARRAY_SIZE_ALL_TEAMS_LOOK_FOR_ALL ) )
	{
		// if his team's already in control
		if (pSoldier->bTeam == gTacticalStatus.ubCurrentTeam )
		{
		// CJC, July 9 1998
			// NO ONE EVER interrupts his own team
			return FALSE;
		}
		else if ( gTacticalStatus.bBoxingState != NOT_BOXING )
		{
			// while anything to do with boxing is going on, skip interrupts!
			return( FALSE );
		}

	}

	if ( !(pSoldier->bActive) || !(pSoldier->bInSector ) )
	{
		return( FALSE );
	}

	// soldiers at less than OKLIFE can't perform any actions
	if (pSoldier->bLife < OKLIFE)
	{
		return(FALSE);
	}

	// soldiers out of breath are about to fall over, no interrupt
	if (pSoldier->bBreath < OKBREATH || pSoldier->bCollapsed )
	{
		return(FALSE);
	}

	// if soldier doesn't have enough APs
	if ( pSoldier->bActionPoints < MIN_APS_TO_INTERRUPT )
	{
		return( FALSE );
	}

	// soldiers gagging on gas are too busy about holding their cookies down...
	if ( pSoldier->uiStatusFlags & SOLDIER_GASSED )
	{
		return(FALSE);
	}

	// a soldier already engaged in a life & death battle is too busy doing his
	// best to survive to worry about "getting the jump" on additional threats
	if (pSoldier->bUnderFire)
	{
		return(FALSE);
	}

	if (pSoldier->bCollapsed)
	{
		return( FALSE );
	}

	// don't allow neutral folks to get interrupts
	if (pSoldier->bNeutral)
	{
		return( FALSE );
	}

	// no EPCs allowed to get interrupts
	if ( AM_AN_EPC( pSoldier ) && !AM_A_ROBOT( pSoldier ) )
	{
		return( FALSE );
	}


	// don't let mercs on assignment get interrupts
	if ( pSoldier->bTeam == OUR_TEAM && pSoldier->bAssignment >= ON_DUTY)
	{
		return( FALSE );
	}


	// the bare minimum default is enough APs left to TURN
	ubMinPtsNeeded = AP_CHANGE_FACING;

	// if the opponent is SOMEBODY
	if (pOpponent != NULL)
	{
		// if the soldiers are on the same side
		if (pSoldier->bSide == pOpponent->bSide)
		{
			// human/civilians on same side can't interrupt each other
			if (pSoldier->uiStatusFlags & SOLDIER_PC || IsOnCivTeam(pSoldier))
			{
				return(FALSE);
			}
			else	// enemy
			{
				// enemies can interrupt EACH OTHER, but enemies and civilians on the
				// same side (but different teams) can't interrupt each other.
				if (pSoldier->bTeam != pOpponent->bTeam)
				{
					return(FALSE);
				}
			}
		}

		// if the interrupted opponent is not the selected character, then the only
		// people eligible to win an interrupt are those on the SAME SIDE AS
		// the selected character, ie. his friends...
		if ( pOpponent->bTeam == OUR_TEAM )
		{
			const SOLDIERTYPE* const sel = GetSelectedMan();
			if (pOpponent != sel && pSoldier->bSide != sel->bSide)
			{
				return( FALSE );
			}
		}
		else
		{
			if ( !(pOpponent->uiStatusFlags & SOLDIER_UNDERAICONTROL) && (pSoldier->bSide != pOpponent->bSide))
			{
				return( FALSE );
			}
		}

		// an non-active soldier can't interrupt a soldier who is also non-active!
		if ((pOpponent->bTeam != gTacticalStatus.ubCurrentTeam) && (pSoldier->bTeam != gTacticalStatus.ubCurrentTeam))
		{
			return(FALSE);
		}


		// if this is a "SEEING" interrupt
		if (pSoldier->bOppList[pOpponent->ubID] == SEEN_CURRENTLY)
		{
			// if pSoldier already saw the opponent last "look" or at least this turn
			if ((bOldOppList == SEEN_CURRENTLY) || (bOldOppList == SEEN_THIS_TURN))
			{
				return(FALSE);     // no interrupt is possible
			}

			// if the soldier is behind him and not very close, forget it
			bDir = atan8( pSoldier->sX, pSoldier->sY, pOpponent->sX, pOpponent->sY );
			if (OppositeDirection(pSoldier->bDesiredDirection) == bDir)
			{
				// directly behind; allow interrupts only within # of tiles equal to level
				if ( PythSpacesAway( pSoldier->sGridNo, pOpponent->sGridNo ) > EffectiveExpLevel( pSoldier ) )
				{
					return( FALSE );
				}
			}

			// if the soldier isn't currently crouching
			if (!PTR_CROUCHED)
			{
				ubMinPtsNeeded = AP_CROUCH;
			}
			else
			{
				ubMinPtsNeeded = MinPtsToMove(pSoldier);
			}
		}
		else   // this is a "HEARING" interrupt
		{
			// if the opponent can't see the "interrupter" either, OR
			// if the "interrupter" already has any opponents already in sight, OR
			// if the "interrupter" already heard the active soldier this turn
			if ((pOpponent->bOppList[pSoldier->ubID] != SEEN_CURRENTLY) || (pSoldier->bOppCnt > 0) || (bOldOppList == HEARD_THIS_TURN))
			{
				return(FALSE);     // no interrupt is possible
			}
		}
	}


	// soldiers without sufficient APs to do something productive can't interrupt
	if (pSoldier->bActionPoints < ubMinPtsNeeded)
	{
		return(FALSE);
	}

	// soldier passed on the chance to react during previous interrupt this turn
	if (pSoldier->bPassedLastInterrupt && !gamepolicy(multiple_interrupts))
	{
		return(FALSE);
	}


#ifdef RECORDINTERRUPT
	// this usually starts a new series of logs, so that's why the blank line
	fprintf(InterruptFile, "\nStandardInterruptConditionsMet by %d vs. %d\n", pSoldier->guynum, pOpponent->ubID);
#endif

	return(TRUE);
}


INT8 CalcInterruptDuelPts(const SOLDIERTYPE* const pSoldier, const SOLDIERTYPE* const opponent, BOOLEAN fUseWatchSpots)
{
	INT8 bPoints;
	INT8 bLightLevel;

	// extra check to make sure neutral folks never get interrupts
	if (pSoldier->bNeutral)
	{
		return( NO_INTERRUPT );
	}

	// BASE is one point for each experience level.

	// Robot has interrupt points based on the controller...
	// Controller's interrupt points are reduced by 2 for being distracted...
	if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT && CanRobotBeControlled( pSoldier ) )
	{
		bPoints = EffectiveExpLevel(pSoldier->robot_remote_holder) - 2;
	}
	else
	{
		bPoints = EffectiveExpLevel( pSoldier );
		/*
		if ( pSoldier->bTeam == ENEMY_TEAM )
		{
			// modify by the difficulty level setting
			bPoints += gbDiff[ DIFF_ENEMY_INTERRUPT_MOD ][ SoldierDifficultyLevel( pSoldier ) ];
			bPoints = std::max(bPoints, 9);
		}
		*/

		if ( ControllingRobot( pSoldier ) )
		{
			bPoints -= 2;
		}
	}

	if (fUseWatchSpots)
	{
		// if this is a previously noted spot of enemies, give bonus points!
		bPoints += GetWatchedLocPoints(pSoldier->ubID, opponent->sGridNo, opponent->bLevel);
	}

	// LOSE one point for each 2 additional opponents he currently sees, above 2
	if (pSoldier->bOppCnt > 2)
	{
		// subtract 1 here so there is a penalty of 1 for seeing 3 enemies
		bPoints -= (pSoldier->bOppCnt - 1) / 2;
	}

	// LOSE one point if he's trying to interrupt only by hearing
	if (pSoldier->bOppList[opponent->ubID] == HEARD_THIS_TURN)
	{
		bPoints--;
	}

	// if soldier is still in shock from recent injuries, that penalizes him
	bPoints -= pSoldier->bShock;

	const UINT8 ubDistance = PythSpacesAway(pSoldier->sGridNo, opponent->sGridNo);

	// if we are in combat mode - thus doing an interrupt rather than determine who gets first turn -
	// then give bonus
	if ( (gTacticalStatus.uiFlags & INCOMBAT) && (pSoldier->bTeam != gTacticalStatus.ubCurrentTeam) )
	{
		// passive player gets penalty due to range
		bPoints -= (ubDistance / 10);
	}
	else
	{
		// either non-combat or the player with the current turn... i.e. active...
		// unfortunately we can't use opplist here to record whether or not we saw this guy before, because at
		// this point the opplist has been updated to seen.  But we can use gbSeenOpponents ...

		// this soldier is moving, so give them a bonus for crawling or swatting at long distances
		if (!gbSeenOpponents[opponent->ubID][pSoldier->ubID])
		{
			if (pSoldier->usAnimState == SWATTING && ubDistance > (MaxDistanceVisible() / 2) ) // more than 1/2 sight distance
			{
				bPoints++;
			}
			else if (pSoldier->usAnimState == CRAWLING && ubDistance > (MaxDistanceVisible() / 4) ) // more than 1/4 sight distance
			{
				bPoints += ubDistance / STRAIGHT;
			}
		}
	}

	// whether active or not, penalize people who are running
	if (pSoldier->usAnimState == RUNNING && !gbSeenOpponents[pSoldier->ubID][opponent->ubID])
	{
		bPoints -= 2;
	}

	if (pSoldier->service_partner != NULL)
	{
		// distracted by being bandaged/doing bandaging
		bPoints -= 2;
	}

	if ( HAS_SKILL_TRAIT( pSoldier, NIGHTOPS ) )
	{
		bLightLevel = LightTrueLevel(pSoldier->sGridNo, pSoldier->bLevel);
		if (bLightLevel > NORMAL_LIGHTLEVEL_DAY + 3)
		{
			// it's dark, give a bonus for interrupts
			bPoints += 1 * NUM_SKILL_TRAITS( pSoldier, NIGHTOPS );
		}
	}

	// if he's a computer soldier

	// CJC note: this will affect friendly AI as well...

	if ( pSoldier->uiStatusFlags & SOLDIER_PC )
	{
		if ( pSoldier->bAssignment >= ON_DUTY )
		{
			// make sure don't get interrupts!
			bPoints = -10;
		}

		// GAIN one point if he's previously seen the opponent
		// check for TRUE because -1 means we JUST saw him (always so here)
		if (gbSeenOpponents[pSoldier->ubID][opponent->ubID] == TRUE)
		{
			bPoints++;  // seen him before, easier to react to him
		}
	}
	else if ( pSoldier->bTeam == ENEMY_TEAM )
	{
		// GAIN one point if he's previously seen the opponent
		// check for TRUE because -1 means we JUST saw him (always so here)
		if (gbSeenOpponents[pSoldier->ubID][opponent->ubID] == TRUE)
		{
			bPoints++;  // seen him before, easier to react to him
		}
		else if (gbPublicOpplist[pSoldier->bTeam][opponent->ubID] != NOT_HEARD_OR_SEEN)
		{
			// GAIN one point if opponent has been recently radioed in by his team
			bPoints++;
		}
	}

	if ( TANK( pSoldier ) )
	{
		// reduce interrupt possibilities for tanks!
		bPoints /= 2;
	}

	if (bPoints >= AUTOMATIC_INTERRUPT)
	{
		bPoints = AUTOMATIC_INTERRUPT - 1; // hack it to one less than max so its legal
	}
	SLOGD("Calculating int pts for {} vs {}, number is {}",
		pSoldier->ubID, opponent->ubID, bPoints);
	return( bPoints );
}

BOOLEAN InterruptDuel( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pOpponent)
{
	BOOLEAN fResult = FALSE;

	// if opponent can't currently see us and we can see them
	if ( pSoldier->bOppList[ pOpponent->ubID ] == SEEN_CURRENTLY && pOpponent->bOppList[pSoldier->ubID] != SEEN_CURRENTLY )
	{
		fResult = TRUE; // we automatically interrupt
		// fix up our interrupt duel pts if necessary
		if (pSoldier->bInterruptDuelPts < pOpponent->bInterruptDuelPts)
		{
			pSoldier->bInterruptDuelPts = pOpponent->bInterruptDuelPts;
		}
	}
	else
	{
		// If our total points is HIGHER, then we interrupt him anyway
		if (pSoldier->bInterruptDuelPts > pOpponent->bInterruptDuelPts)
		{
			fResult = TRUE;
		}
	}
	//ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, ST::format("Interrupt duel {} ({} pts) vs {} ({} pts)", pSoldier->ubID, pSoldier->bInterruptDuelPts, pOpponent->ubID, pOpponent->bInterruptDuelPts) );
	return( fResult );
}


static void DeleteFromIntList(UINT8 ubIndex, BOOLEAN fCommunicate)
{
	UINT8 ubLoop;

	if ( ubIndex > gubOutOfTurnPersons)
	{
		return;
	}
	SLOGD("INTERRUPT: removing ID {}", gOutOfTurnOrder[ubIndex]->ubID);

	// if we're NOT deleting the LAST entry in the int list
	if (ubIndex < gubOutOfTurnPersons)
	{
		// not the last entry, must move all those behind it over to fill the gap
		for (ubLoop = ubIndex; ubLoop < gubOutOfTurnPersons; ubLoop++)
		{
			gOutOfTurnOrder[ubLoop] = gOutOfTurnOrder[ubLoop + 1];
		}
	}

	// either way, whack the last entry to NOBODY and decrement the list size
	gOutOfTurnOrder[gubOutOfTurnPersons] = NULL;
	gubOutOfTurnPersons--;
}


void AddToIntList(SOLDIERTYPE* const s, const BOOLEAN fGainControl, const BOOLEAN fCommunicate)
{
	UINT8 ubLoop;

	SLOGD("INTERRUPT: adding ID {} who {}",
				s->ubID, fGainControl ? "gains control" : "loses control");

	// check whether 'who' is already anywhere on the queue after the first index
	// which we want to preserve so we can restore turn order
	for (ubLoop = 2; ubLoop <= gubOutOfTurnPersons; ubLoop++)
	{
		if (gOutOfTurnOrder[ubLoop] == s)
		{
			if (!fGainControl)
			{
				// he's LOSING control; that's it, we're done, DON'T add him to the queue again
				gLastInterruptedGuy = s;
				return;
			}
			else
			{
				// GAINING control, so delete him from this slot (because later he'll
				// get added to the end and we don't want him listed more than once!)
				DeleteFromIntList( ubLoop, FALSE );
			}
		}
	}

	// increment total (making index valid) and add him to list
	gubOutOfTurnPersons++;
	gOutOfTurnOrder[gubOutOfTurnPersons] = s;

	// if the guy is gaining control
	if (fGainControl)
	{
		// record his initial APs at the start of his interrupt at this time
		// this is not the ideal place for this, but it's the best I could do...
		s->bIntStartAPs = s->bActionPoints;
	}
	else
	{
		gLastInterruptedGuy = s;
		// turn off AI control flag if they lost control
		if (s->uiStatusFlags & SOLDIER_UNDERAICONTROL)
		{
			SLOGD("Taking away AI control from {}", s->ubID);
			s->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
		}
	}
}


static void VerifyOutOfTurnOrderArray(void)
{
	UINT8		ubTeamHighest[ MAXTEAMS ] = { 0 };
	UINT8		ubTeamsInList;
	UINT8		ubNextIndex;
	UINT8		ubTeam;
	UINT8		ubLoop, ubLoop2;
	BOOLEAN	fFoundLoop = FALSE;

	for (ubLoop = 1; ubLoop <= gubOutOfTurnPersons; ubLoop++)
	{
		ubTeam = gOutOfTurnOrder[ubLoop]->bTeam;
		if (ubTeamHighest[ ubTeam ] > 0)
		{
			// check the other teams to see if any of them are between our last team's mention in
			// the array and this
			for (ubLoop2 = 0; ubLoop2 < MAXTEAMS; ubLoop2++)
			{
				if (ubLoop2 == ubTeam)
				{
					continue;
				}
				else
				{
					if (ubTeamHighest[ ubLoop2 ] > ubTeamHighest[ ubTeam ])
					{
						// there's a loop!! delete it!
						const SOLDIERTYPE* const NextInArrayOnTeam = gOutOfTurnOrder[ubLoop];
						ubNextIndex = ubTeamHighest[ ubTeam ] + 1;

						while (gOutOfTurnOrder[ubNextIndex] != NextInArrayOnTeam)
						{
							// Pause them...
							AdjustNoAPToFinishMove(gOutOfTurnOrder[ubNextIndex], TRUE);

							// If they were turning from prone, stop them
							gOutOfTurnOrder[ubNextIndex]->fTurningFromPronePosition = FALSE;

							DeleteFromIntList( ubNextIndex, FALSE );
						}

						fFoundLoop = TRUE;
						break;
					}
				}
			}

			if (fFoundLoop)
			{
				// at this point we should restart our outside loop (ugh)
				fFoundLoop = FALSE;
				for (ubLoop2 = 0; ubLoop2 < MAXTEAMS; ubLoop2++)
				{
					ubTeamHighest[ ubLoop2 ] = 0;
				}
				ubLoop = 0;
				continue;

			}

		}

		ubTeamHighest[ ubTeam ] = ubLoop;
	}

	// Another potential problem: the player is interrupted by the enemy who is interrupted by
	// the militia.  In this situation the enemy should just lose their interrupt.
	// (Or, the militia is interrupted by the enemy who is interrupted by the player.)

	// Check for 3+ teams in the interrupt queue.  If three exist then abort all interrupts (return
	// control to the first team)
	ubTeamsInList = 0;
	for ( ubLoop = 0; ubLoop < MAXTEAMS; ubLoop++ )
	{
		if ( ubTeamHighest[ ubLoop ] > 0 )
		{
			ubTeamsInList++;
		}
	}
	if ( ubTeamsInList >= 3 )
	{
		// This is bad.  Loop through everyone but the first person in the INT list and remove 'em
		for (ubLoop = 2; ubLoop <= gubOutOfTurnPersons; )
		{
			if (gOutOfTurnOrder[ubLoop]->bTeam != gOutOfTurnOrder[1]->bTeam)
			{
				// remove!

				// Pause them...
				AdjustNoAPToFinishMove(gOutOfTurnOrder[ubLoop], TRUE);

				// If they were turning from prone, stop them
				gOutOfTurnOrder[ubLoop]->fTurningFromPronePosition = FALSE;

				DeleteFromIntList( ubLoop, FALSE );

				// since we deleted someone from the list, we want to check the same index in the
				// array again, hence we DON'T increment.
			}
			else
			{
				ubLoop++;
			}
		}
	}
}


void DoneAddingToIntList(void)
{
	VerifyOutOfTurnOrderArray();
	if (EveryoneInInterruptListOnSameTeam())
	{
		EndInterrupt(TRUE);
	}
	else
	{
		StartInterrupt();
	}
}


void ResolveInterruptsVs( SOLDIERTYPE * pSoldier, UINT8 ubInterruptType)
{
	UINT8 ubIntCnt;
	SOLDIERTYPE* IntList[MAXMERCS];
	UINT8 ubIntDiff[MAXMERCS];
	UINT8 ubSmallestDiff;
	UINT8 ubSlot, ubSmallestSlot;
	BOOLEAN fIntOccurs;

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		ubIntCnt = 0;

		for (UINT8 ubTeam = 0; ubTeam < MAXTEAMS; ++ubTeam)
		{
			if (IsTeamActive(ubTeam) && gTacticalStatus.Team[ubTeam].bSide != pSoldier->bSide && ubTeam != CIV_TEAM)
			{
				FOR_EACH_IN_TEAM(pOpponent, ubTeam)
				{
					if (pOpponent->bInSector && pOpponent->bLife >= OKLIFE && !pOpponent->bCollapsed)
					{
						if ( ubInterruptType == NOISEINTERRUPT )
						{
							// don't grant noise interrupts at greater than max. visible distance
							if ( PythSpacesAway( pSoldier->sGridNo, pOpponent->sGridNo ) > MaxDistanceVisible() )
							{
								pOpponent->bInterruptDuelPts = NO_INTERRUPT;
								SLOGD("Resetting int pts for {} - NOISE BEYOND SIGHT DISTANCE!?",
											pOpponent->ubID);
								continue;
							}
						}
						else if ( pOpponent->bOppList[pSoldier->ubID] != SEEN_CURRENTLY )
						{
							pOpponent->bInterruptDuelPts = NO_INTERRUPT;
							SLOGD("Resetting int pts for {} - DOESN'T SEE ON SIGHT INTERRUPT!?",
										pOpponent->ubID);
							continue;
						}

						switch (pOpponent->bInterruptDuelPts)
						{
							case NO_INTERRUPT:		// no interrupt possible, no duel necessary
								fIntOccurs = FALSE;
								break;

							case AUTOMATIC_INTERRUPT:	// interrupts occurs automatically
								pSoldier->bInterruptDuelPts = 0;	// just to have a valid intDiff later
								fIntOccurs = TRUE;
								SLOGD("INTERRUPT: automatic interrupt on {} by {}",
											pSoldier->ubID, pOpponent->ubID);
								break;

							default:		// interrupt is possible, run a duel
								SLOGD("Calculating int duel pts for onlooker in ResolveInterruptsVs");
								pSoldier->bInterruptDuelPts = CalcInterruptDuelPts(pSoldier, pOpponent, TRUE);
								fIntOccurs = InterruptDuel(pOpponent,pSoldier);
								if (fIntOccurs)
								{
									SLOGD("INTERRUPT: standard interrupt on {} ({} pts) by {} ({} pts)",
												pSoldier->ubID, pSoldier->bInterruptDuelPts, pOpponent->ubID, pOpponent->bInterruptDuelPts);
								}
								break;
						}

						if (fIntOccurs)
						{
							// remember that this opponent's scheduled to interrupt us
							IntList[ubIntCnt] = pOpponent;

							// and by how much he beat us in the duel
							ubIntDiff[ubIntCnt] = pOpponent->bInterruptDuelPts - pSoldier->bInterruptDuelPts;

							// increment counter of interrupts lost
							ubIntCnt++;
						}
						else
						{
						/*
							if (pOpponent->bInterruptDuelPts != NO_INTERRUPT)
							{
								ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, ST::format("{} fails to interrupt {} ({} vs {} pts)", pOpponent->ubID, pSoldier->ubID, pOpponent->bInterruptDuelPts, pSoldier->bInterruptDuelPts));
							}
							*/
						}

						// either way, clear out both sides' bInterruptDuelPts field to prepare next one
						if (pSoldier->bInterruptDuelPts != NO_INTERRUPT)
						{
							SLOGD("Resetting int pts for {} and {}",
										pSoldier->ubID, pOpponent->ubID);
						}
						pSoldier->bInterruptDuelPts = NO_INTERRUPT;
						pOpponent->bInterruptDuelPts = NO_INTERRUPT;
					}
				}
			}
		}

		// if any interrupts are scheduled to occur (ie. I lost at least once)
		if (ubIntCnt)
		{
			// First add currently active character to the interrupt queue.  This is
			// USUALLY pSoldier->guynum, but NOT always, because one enemy can
			// "interrupt" on another enemy's turn if he hears another team's wound
			// victim's screaming...  the guy screaming is pSoldier here, it's not his turn!
			//AddToIntList(GetSelectedMan(), FALSE, TRUE);

			if ( (gTacticalStatus.ubCurrentTeam != pSoldier->bTeam) && !(gTacticalStatus.Team[ gTacticalStatus.ubCurrentTeam ].bHuman) )
			{
				// if anyone on this team is under AI control, remove
				// their AI control flag and put them on the queue instead of this guy
				FOR_EACH_IN_TEAM(s, gTacticalStatus.ubCurrentTeam)
				{
					if (s->uiStatusFlags & SOLDIER_UNDERAICONTROL)
					{
						// this guy lost control
						s->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
						AddToIntList(s, FALSE, TRUE);
						break;
					}
				}

			}
			else
			{
				// this guy lost control
				AddToIntList(pSoldier, FALSE, TRUE);
			}

			// loop once for each opponent who interrupted
			for (UINT8 ubLoop = 0; ubLoop < ubIntCnt; ++ubLoop)
			{
				// find the smallest intDiff still remaining in the list
				ubSmallestDiff = NO_INTERRUPT;
				ubSmallestSlot = NOBODY;

				for (ubSlot = 0; ubSlot < ubIntCnt; ubSlot++)
				{
					if (ubIntDiff[ubSlot] < ubSmallestDiff)
					{
						ubSmallestDiff = ubIntDiff[ubSlot];
						ubSmallestSlot = ubSlot;
					}
				}

				if (ubSmallestSlot < MAX_NUM_SOLDIERS)
				{
					// add this guy to everyone's interrupt queue
					AddToIntList(IntList[ubSmallestSlot], TRUE, TRUE);
					if (INTERRUPTS_OVER)
					{
						// a loop was created which removed all the people in the interrupt queue!
						EndInterrupt( TRUE );
						return;
					}

					ubIntDiff[ubSmallestSlot] = NO_INTERRUPT;      // mark slot as been handled
				}
			}

			// sends off an end-of-list msg telling everyone whether to switch control,
			// unless it's a MOVEMENT interrupt, in which case that is delayed til later
			DoneAddingToIntList();
		}
	}
}


void SaveTeamTurnsToTheSaveGameFile(HWFILE const f)
{
	BYTE  data[174];
	DataWriter d{data};
	for (size_t i = 0; i != lengthof(gOutOfTurnOrder); ++i)
	{
		INJ_SOLDIER(d, gOutOfTurnOrder[i])
	}
	INJ_U8(     d, gubOutOfTurnPersons)
	INJ_SKIP(   d, 3)
	INJ_SOLDIER(d, gWhoThrewRock)
	INJ_SKIP(   d, 2)
	INJ_BOOL(   d, gfHiddenInterrupt)
	INJ_SOLDIER(d, gLastInterruptedGuy)
	INJ_SKIP(   d, 17)
	Assert(d.getConsumed() == lengthof(data));

	f->write(data, sizeof(data));
}


void LoadTeamTurnsFromTheSavedGameFile(HWFILE const f)
{
	BYTE data[174];
	f->read(data, sizeof(data));

	DataReader d{data};
	EXTR_SKIP(d, 1)
	for (size_t i = 1; i != lengthof(gOutOfTurnOrder); ++i)
	{
		EXTR_SOLDIER(d, gOutOfTurnOrder[i])
	}
	EXTR_U8(     d, gubOutOfTurnPersons)
	EXTR_SKIP(   d, 3)
	EXTR_SOLDIER(d, gWhoThrewRock)
	EXTR_SKIP(   d, 2)
	EXTR_BOOL(   d, gfHiddenInterrupt)
	EXTR_SOLDIER(d, gLastInterruptedGuy)
	EXTR_SKIP(   d, 17)
	Assert(d.getConsumed() == lengthof(data));
}


BOOLEAN NPCFirstDraw( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pTargetSoldier )
{
	// if attacking an NPC check to see who draws first!

	if ( pTargetSoldier->ubProfile != NO_PROFILE && pTargetSoldier->ubProfile != SLAY && pTargetSoldier->bNeutral && pTargetSoldier->bOppList[ pSoldier->ubID ] == SEEN_CURRENTLY && (	FindAIUsableObjClass( pTargetSoldier, IC_WEAPON ) != NO_SLOT ) )
	{
		UINT8	ubLargerHalf, ubSmallerHalf, ubTargetLargerHalf, ubTargetSmallerHalf;

		// roll the dice!
		// e.g. if level 5, roll Random( 3 + 1 ) + 2 for result from 2 to 5
		// if level 4, roll Random( 2 + 1 ) + 2 for result from 2 to 4
		ubSmallerHalf = EffectiveExpLevel( pSoldier ) / 2;
		ubLargerHalf = EffectiveExpLevel( pSoldier ) - ubSmallerHalf;

		ubTargetSmallerHalf = EffectiveExpLevel( pTargetSoldier ) / 2;
		ubTargetLargerHalf = EffectiveExpLevel( pTargetSoldier ) - ubTargetSmallerHalf;
		if ( gMercProfiles[ pTargetSoldier->ubProfile ].bApproached & gbFirstApproachFlags[ APPROACH_THREATEN - 1 ] )
		{
			// gains 1 to 2 points
			ubTargetSmallerHalf += 1;
			ubTargetLargerHalf += 1;
		}
		if ( Random( ubTargetSmallerHalf + 1) + ubTargetLargerHalf > Random( ubSmallerHalf + 1) + ubLargerHalf )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}
