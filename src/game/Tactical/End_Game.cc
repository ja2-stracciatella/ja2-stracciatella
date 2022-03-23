#include "Soldier_Control.h"
#include "Overhead.h"
#include "Boxing.h"
#include "Render_Fun.h"
#include "Random.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "WorldMan.h"
#include "Soldier_Profile.h"
#include "NPC.h"
#include "OppList.h"
#include "AI.h"
#include "Dialogue_Control.h"
#include "Handle_UI.h"
#include "Points.h"
#include "End_Game.h"
#include "Intro.h"
#include "Exit_Grids.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "SaveLoadMap.h"
#include "Sound_Control.h"
#include "RenderWorld.h"
#include "Isometric_Utils.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Soldier_Macros.h"
#include "QArray.h"
#include "LOS.h"
#include "Strategic_AI.h"
#include "Squads.h"
#include "PreBattle_Interface.h"
#include "Strategic.h"
#include "Queen_Command.h"
#include "Morale.h"
#include "Strategic_Town_Loyalty.h"
#include "Player_Command.h"
#include "Tactical_Save.h"
#include "Fade_Screen.h"
#include "ScreenIDs.h"


INT16 sStatueGridNos[] = { 13829, 13830, 13669, 13670 };

SOLDIERTYPE *gpKillerSoldier = NULL;
INT16 gsGridNo;
INT8 gbLevel;


// This function checks if our statue exists in the current sector at given gridno
static BOOLEAN DoesO3SectorStatueExistHere( INT16 sGridNo )
{
	INT32 cnt;
	EXITGRID ExitGrid;

	// First check current sector......
	static const SGPSector sectorO3(3, MAP_ROW_O);
	if (gWorldSector == sectorO3)
	{
		// Check for exitence of and exit grid here...
		// ( if it doesn't then the change has already taken place )
		if ( !GetExitGrid( 13669, &ExitGrid ) )
		{
			for ( cnt = 0; cnt < 4; cnt++ )
			{
				if ( sStatueGridNos[ cnt ] == sGridNo )
				{
					return( TRUE );
				}
			}
		}
	}

	return( FALSE );
}

// This function changes the graphic of the statue and adds the exit grid...
void ChangeO3SectorStatue( BOOLEAN fFromExplosion )
{
	EXITGRID ExitGrid;
	UINT16 usTileIndex;

	// Remove old graphic
	{
		ApplyMapChangesToMapTempFile app;
		// Remove it!
		// Get index for it...
		usTileIndex = GetTileIndexFromTypeSubIndex(EIGHTOSTRUCT, 5);
		RemoveStruct( 13830, usTileIndex );

		// Add new one...
		if ( fFromExplosion )
		{
			// Use damaged peice
			usTileIndex = GetTileIndexFromTypeSubIndex(EIGHTOSTRUCT, 7);
		}
		else
		{
			usTileIndex = GetTileIndexFromTypeSubIndex(EIGHTOSTRUCT, 8);
			// Play sound...

			PlayJA2Sample(OPEN_STATUE, HIGHVOLUME, 1, MIDDLEPAN);

		}
		AddStructToHead( 13830, usTileIndex );

		// Add exit grid
		ExitGrid.ubGotoSectorX = 3;
		ExitGrid.ubGotoSectorY = MAP_ROW_O;
		ExitGrid.ubGotoSectorZ = 1;
		ExitGrid.usGridNo = 13037;

		AddExitGridToWorld( 13669, &ExitGrid );
		gpWorldLevelData[ 13669 ].uiFlags |= MAPELEMENT_REVEALED;
	}

	// Re-render the world!
	gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
	// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
	InvalidateWorldRedundency( );
	SetRenderFlags(RENDER_FLAG_FULL);

	RecompileLocalMovementCostsFromRadius( 13830, 5 );
}

void HandleStatueDamaged(INT16 sectorX, INT16 sectorY, INT8 sectorZ, INT16 sGridNo, STRUCTURE *s, UINT32 uiDist, BOOLEAN_S *skipDamage)
{
	/* ATE: Check for O3 statue for special damage
	 * Note, we do this check every time explosion goes off in game, but it's an
	 * efficient check */
	if (DoesO3SectorStatueExistHere(sGridNo) && uiDist <= 1)
	{
		ChangeO3SectorStatue(TRUE);
		*skipDamage = true;
	}
}

static void HandleDeidrannaDeath(SOLDIERTYPE* pKillerSoldier, INT16 sGridNo, INT8 bLevel);


static void DeidrannaTimerCallback(void)
{
	HandleDeidrannaDeath( gpKillerSoldier, gsGridNo, gbLevel );
}


void BeginHandleDeidrannaDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel )
{
	gpKillerSoldier = pKillerSoldier;
	gsGridNo = sGridNo;
	gbLevel  = bLevel;

	// Lock the UI.....
	gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
	// Increment refrence count...
	giNPCReferenceCount = 1;

	gTacticalStatus.uiFlags |= IN_DEIDRANNA_ENDGAME;

	SetCustomizableTimerCallbackAndDelay( 2000, DeidrannaTimerCallback, FALSE );

}


static void DoneFadeOutKilledQueen();


static void HandleDeidrannaDeath(SOLDIERTYPE* const pKillerSoldier, const INT16 sGridNo, const INT8 bLevel)
{
	// Start victory music here...
	SetMusicMode( MUSIC_TACTICAL_VICTORY );


	if ( pKillerSoldier )
	{
		TacticalCharacterDialogue( pKillerSoldier, QUOTE_KILLING_DEIDRANNA );
	}

	// STEP 1 ) START ALL QUOTES GOING!
	// OK - loop through all witnesses and see if they want to say something abou this...
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s != pKillerSoldier &&
			OkControllableMerc(s) &&
			!(s->uiStatusFlags & SOLDIER_GASSED) &&
			!AM_AN_EPC(s) &&
			QuoteExp_WitnessDeidrannaDeath[s->ubProfile])
		{
			// Can we see location?
			const INT16 sDistVisible = DistanceVisible(s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, bLevel);
			if (SoldierTo3DLocationLineOfSightTest(s, sGridNo, bLevel, 3, sDistVisible, TRUE))
			{
				TacticalCharacterDialogue(s, QUOTE_KILLING_DEIDRANNA);
			}
		}
	}

	// Set fact that she is dead!
	SetFactTrue( FACT_QUEEN_DEAD );

	ExecuteStrategicAIAction( STRATEGIC_AI_ACTION_QUEEN_DEAD, 0, 0 );

	class DialogueEventDoneKillingDeidranna : public DialogueEvent
	{
		public:
			bool Execute()
			{
				// Called after all player quotes are done
				gFadeOutDoneCallback = DoneFadeOutKilledQueen;
				FadeOutGameScreen();
				return false;
			}
	};

	// AFTER LAST ONE IS DONE - PUT SPECIAL EVENT ON QUEUE TO BEGIN FADE< ETC
	DialogueEvent::Add(new DialogueEventDoneKillingDeidranna());
}


static void DoneFadeInKilledQueen(void)
{
	// Run NPC script
	const SOLDIERTYPE* const pNPCSoldier = FindSoldierByProfileID(DEREK);
	if ( !pNPCSoldier )
	{
		return;
	}

	TriggerNPCRecordImmediately( pNPCSoldier->ubProfile, 6 );
}


static void DoneFadeOutKilledQueen()
{
	// Move current squad over
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE& s = *i;
		// Are we in this sector, on the current squad?
		if (s.bLife < OKLIFE)                continue;
		if (!s.bInSector)                    continue;
		if (s.bAssignment != CurrentSquad()) continue;

		gfTacticalTraversal = TRUE;
		SetGroupSectorValue(3, MAP_ROW_P, 0, *GetGroup(s.ubGroupID));

		// XXX redundant, SetGroupSectorValue() handles this
		s.sSectorX                 = 3;
		s.sSectorY                 = MAP_ROW_P;
		s.bSectorZ                 = 0;
		// Set gridno
		s.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		s.usStrategicInsertionData = 5687;
		// Set direction to face
		s.ubInsertionDirection     = 100 + NORTHWEST;
	}

	// Kill all enemies in world
	CFOR_EACH_IN_TEAM(i, ENEMY_TEAM)
	{
		SOLDIERTYPE const& s = *i;
		// For sure for flag thet they are dead is not set
		// Check for any more badguys
		// ON THE STRAGETY LAYER KILL BAD GUYS!
		if (s.bNeutral)             continue;
		if (s.bSide == OUR_TEAM) continue;
		ProcessQueenCmdImplicationsOfDeath(&s);
	}

	// 'End' battle
	ExitCombatMode();
	gTacticalStatus.fLastBattleWon = TRUE;
	// Set enemy presence to false
	gTacticalStatus.fEnemyInSector = FALSE;

	SetMusicMode(MUSIC_TACTICAL_VICTORY);

	const SGPSector upstairs(3, MAP_ROW_P, 0);
	HandleMoraleEvent(0, MORALE_QUEEN_BATTLE_WON, upstairs.x, upstairs.y, upstairs.z);
	HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_QUEEN_BATTLE_WON, upstairs);

	SetMusicMode(MUSIC_TACTICAL_VICTORY);

	SetThisSectorAsPlayerControlled(gWorldSector, TRUE);

	// ATE: Force change of level set z to 1
	gWorldSector.z = 1;

	// Clear out dudes
	SECTORINFO& sector = SectorInfo[SEC_P3];
	sector.ubNumAdmins      = 0;
	sector.ubNumTroops      = 0;
	sector.ubNumElites      = 0;
	sector.ubAdminsInBattle = 0;
	sector.ubTroopsInBattle = 0;
	sector.ubElitesInBattle = 0;

	// ATE: Get rid of Elliot in P3
	GetProfile(ELLIOT).sSectorX = 1;

	ChangeNpcToDifferentSector(GetProfile(DEREK), upstairs);
	ChangeNpcToDifferentSector(GetProfile(OLIVER), upstairs);

	SetCurrentWorldSector(upstairs);

	gfTacticalTraversal              = FALSE;
	gpTacticalTraversalGroup         = 0;
	gpTacticalTraversalChosenSoldier = 0;

	gFadeInDoneCallback = DoneFadeInKilledQueen;
	FadeInGameScreen();
}


static void DoneFadeOutEndCinematic(void);


void EndQueenDeathEndgameBeginEndCimenatic( )
{
	// Start end cimimatic....
	gTacticalStatus.uiFlags |= IN_ENDGAME_SEQUENCE;

	// first thing is to loop through team and say end quote...
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bLife >= OKLIFE && !AM_AN_EPC(s))
		{
			TacticalCharacterDialogue(s, QUOTE_END_GAME_COMMENT);
		}
	}

	class DialogueEventTeamMembersDoneTalking : public DialogueEvent
	{
		public:
			bool Execute()
			{
				// End death UI - fade to smaker
				EndQueenDeathEndgame();
				gFadeOutDoneCallback = DoneFadeOutEndCinematic;
				FadeOutGameScreen();
				return false;
			}
	};

	// Add queue event to proceed w/ smacker cimimatic
	DialogueEvent::Add(new DialogueEventTeamMembersDoneTalking());
}


void EndQueenDeathEndgame( )
{
	// Unset flags...
	gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV );
	// Increment refrence count...
	giNPCReferenceCount = 0;

	gTacticalStatus.uiFlags &= (~IN_DEIDRANNA_ENDGAME);
}


static void DoneFadeOutEndCinematic(void)
{
	// DAVE PUT SMAKER STUFF HERE!!!!!!!!!!!!
	// :)
	gTacticalStatus.uiFlags &= (~IN_ENDGAME_SEQUENCE);


	// For now, just quit the freaken game...
	//InternalLeaveTacticalScreen( MAINMENU_SCREEN );

	InternalLeaveTacticalScreen( INTRO_SCREEN );
	//guiCurrentScreen = INTRO_SCREEN;


	SetIntroType( INTRO_ENDING );
}


static void HandleQueenBitchDeath(SOLDIERTYPE* pKillerSoldier, INT16 sGridNo, INT8 bLevel);


static void QueenBitchTimerCallback(void)
{
	HandleQueenBitchDeath( gpKillerSoldier, gsGridNo, gbLevel );
}


void BeginHandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel )
{
	gpKillerSoldier = pKillerSoldier;
	gsGridNo = sGridNo;
	gbLevel  = bLevel;

	// Lock the UI.....
	gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
	// Increment refrence count...
	giNPCReferenceCount = 1;

	// gTacticalStatus.uiFlags |= IN_DEIDRANNA_ENDGAME;

	SetCustomizableTimerCallbackAndDelay( 3000, QueenBitchTimerCallback, FALSE );


	// Kill all enemies in creature team.....
	FOR_EACH_IN_TEAM(s, CREATURE_TEAM)
	{
		// Are we ALIVE.....
		if (s->bLife > 0)
		{
			// For sure for flag thet they are dead is not set
			// Check for any more badguys
			// ON THE STRAGETY LAYER KILL BAD GUYS!

			// HELLO!  THESE ARE CREATURES!  THEY CAN'T BE NEUTRAL!
			//if (!s->bNeutral && s->bSide != OUR_TEAM)
			{
				gTacticalStatus.ubAttackBusyCount++;
				EVENT_SoldierGotHit(s, 0, 10000, 0, s->bDirection, 320, NULL, FIRE_WEAPON_NO_SPECIAL, s->bAimShotLocation, NOWHERE);
			}
		}
	}


}


static void HandleQueenBitchDeath(SOLDIERTYPE* const pKillerSoldier, const INT16 sGridNo, const INT8 bLevel)
{
	// Start victory music here...
	SetMusicMode( MUSIC_TACTICAL_VICTORY );

	if ( pKillerSoldier )
	{
		TacticalCharacterDialogue( pKillerSoldier, QUOTE_KILLING_QUEEN );
	}

	// STEP 1 ) START ALL QUOTES GOING!
	// OK - loop through all witnesses and see if they want to say something abou this...
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s != pKillerSoldier &&
			OkControllableMerc(s) &&
			!(s->uiStatusFlags & SOLDIER_GASSED) &&
			!AM_AN_EPC(s) &&
			QuoteExp_WitnessQueenBugDeath[s->ubProfile])
		{
			// Can we see location?
			const INT16 sDistVisible = DistanceVisible(s, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, bLevel);
			if (SoldierTo3DLocationLineOfSightTest(s, sGridNo, bLevel, 3, sDistVisible, TRUE))
			{
				TacticalCharacterDialogue(s, QUOTE_KILLING_QUEEN);
			}
		}
	}


	// Set fact that she is dead!
	if ( CheckFact( FACT_QUEEN_DEAD, 0 ) )
	{
		EndQueenDeathEndgameBeginEndCimenatic( );
	}
	else
	{
		// Unset flags...
		gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV );
		// Increment refrence count...
		giNPCReferenceCount = 0;
	}
}
