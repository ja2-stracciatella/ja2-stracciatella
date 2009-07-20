#include "Game_Clock.h"
#include "Render_Dirty.h"
#include "Overhead.h"
#include "Environment.h"
#include "WorldDef.h"
#include "Rotting_Corpses.h"
#include "Soldier_Create.h"
#include "Soldier_Add.h"
#include "Strategic_Turns.h"
#include "Animation_Data.h"
#include "Tactical_Turns.h"
#include "Points.h"
#include "Smell.h"
#include "OppList.h"
#include "Queen_Command.h"
#include "Dialogue_Control.h"
#include "SmokeEffects.h"
#include "LightEffects.h"
#include "Campaign.h"
#include "Soldier_Macros.h"
#include "StrategicMap.h"
#include "Random.h"
#include "Explosion_Control.h"
#include "JAScreens.h"
#include "ScreenIDs.h"
#include "Items.h"


void HandleRPCDescription()
{
	TacticalStatusType& ts = gTacticalStatus;
	if (!ts.fCountingDownForGuideDescription) return;

	// ATE: postpone if we are not in tactical
	if (guiCurrentScreen != GAME_SCREEN) return;

	if (ts.uiFlags & ENGAGED_IN_CONV) return;

	// Are we a SAM site?
	if (ts.ubGuideDescriptionToUse == 25 ||
			ts.ubGuideDescriptionToUse == 27 ||
			ts.ubGuideDescriptionToUse == 30 ||
			ts.ubGuideDescriptionToUse == 31 ||
			ts.ubGuideDescriptionToUse == 32)
	{
		ts.bGuideDescriptionCountDown = 1;
	}
	else
	{
		// ATE; Don't do in combat
		if (ts.uiFlags & INCOMBAT) return;

		// Don't do if enemy in sector
		if (NumEnemyInSector()) return;
	}

	if (--ts.bGuideDescriptionCountDown != 0) return;
	ts.fCountingDownForGuideDescription = FALSE;

	// Count how many RPC guys we have
	UINT8        n_mercs = 0;
	SOLDIERTYPE* mercs_in_sector[20];
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// Add guy if he's a candidate
		if (!RPC_RECRUITED(s))                          continue;
		if (s->bLife < OKLIFE)                          continue;
		if (s->sSectorX != ts.bGuideDescriptionSectorX) continue;
		if (s->sSectorY != ts.bGuideDescriptionSectorY) continue;
		if (s->bSectorZ != gbWorldSectorZ)              continue;
		if (s->fBetweenSectors)                         continue;

		if (s->ubProfile == IRA    ||
				s->ubProfile == MIGUEL ||
				s->ubProfile == CARLOS ||
				s->ubProfile == DIMITRI)
		{
			mercs_in_sector[n_mercs++] = s;
		}
	}

	if (n_mercs == 0) return;

	SOLDIERTYPE& chosen = *mercs_in_sector[Random(n_mercs)];
	CharacterDialogueUsingAlternateFile(chosen, ts.ubGuideDescriptionToUse, DIALOGUE_TACTICAL_UI);
}


void HandleTacticalEndTurn(void)
{
	UINT32				uiTime;
  static UINT32 uiTimeSinceLastStrategicUpdate = 0;

	// OK, Do a number of things here....
	// Every few turns......

	// Get time elasped
	uiTime = GetWorldTotalSeconds( );

  if ( ( uiTimeSinceLastStrategicUpdate - uiTime ) > 1200 )
  {
		HandleRottingCorpses( );
  	//DecayTacticalMoraleModifiers();

    uiTimeSinceLastStrategicUpdate = uiTime;
  }

	DecayBombTimers( );

	DecaySmokeEffects( uiTime );

	DecayLightEffects( uiTime );

	// Decay smells
	//DecaySmells();

	// Decay blood
	DecayBloodAndSmells( uiTime );

	// decay AI warning values from corpses
	DecayRottingCorpseAIWarnings();

	//Check for enemy pooling (add enemies if there happens to be more than the max in the
	//current battle.  If one or more slots have freed up, we can add them now.
	AddPossiblePendingEnemiesToBattle();

	// Loop through each active team and decay public opplist...
	// May want this done every few times too
	NonCombatDecayPublicOpplist( uiTime );
	/*
	for (UINT32 cnt = 0; cnt < MAXTEAMS; ++cnt)
	{
		if (IsTeamActive(cnt))
		{
			// decay team's public opplist
			DecayPublicOpplist( (INT8)cnt );
		}
	}
*/

	// First pass:
	// Loop through our own mercs:
	//	Check things like ( even if not in our sector )
	//		1 ) All updates of breath, shock, bleeding, etc
	//    2 ) Updating First AID, etc
	//  ( If in our sector: )
	//		3 ) Update things like decayed opplist, etc

	// Second pass:
	//  Loop through all mercs in tactical engine
	//  If not a player merc ( ubTeam ) , do things like 1 , 2 , 3 above


	// First exit if we are not in realtime combat or realtime noncombat
	if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		BeginLoggingForBleedMeToos( TRUE );

		FOR_ALL_IN_TEAM(s, gbPlayerNum)
		{
			if (s->bLife > 0 && !(s->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(s))
			{
				// Handle everything from getting breath back, to bleeding, etc
				EVENT_BeginMercTurn(s);

				// Handle Player services
				HandlePlayerServices(*s);

				// if time is up, turn off xray
				if (s->uiXRayActivatedTime && uiTime > s->uiXRayActivatedTime + XRAY_TIME)
				{
					TurnOffXRayEffects(s);
				}
			}
		}

		BeginLoggingForBleedMeToos( FALSE );

		// OK, loop through the mercs to perform 'end turn' events on each...
		// We're looping through only mercs in tactical engine, ignoring our mercs
		// because they were done earilier...
		FOR_ALL_MERCS(i)
		{
			SOLDIERTYPE* const pSoldier = *i;
			if ( pSoldier->bTeam != gbPlayerNum )
			{
				// Handle everything from getting breath back, to bleeding, etc
				EVENT_BeginMercTurn(pSoldier);

				// Handle Player services
				HandlePlayerServices(*pSoldier);
			}
		}
	}

	HandleRPCDescription( );

}
