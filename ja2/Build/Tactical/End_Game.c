#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
	#include "PreBattle_Interface.h"
	#include "Intro.h"
#else
	#include "Soldier_Control.h"
	#include "Overhead.h"
	#include "Boxing.h"
	#include "Render_Fun.h"
	#include "Random.h"
	#include "WorldMan.h"
	#include "Soldier_Profile.h"
	#include "NPC.h"
	#include "OppList.h"
	#include "AI.h"
	#include "Dialogue_Control.h"
	#include "Handle_UI.h"
	#include "Points.h"
	#include "Interface.h"
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
#endif

INT16 sStatueGridNos[] = { 13829, 13830, 13669, 13670 };

SOLDIERTYPE *gpKillerSoldier = NULL;
INT16				 gsGridNo;
INT8				 gbLevel;


// This function checks if our statue exists in the current sector at given gridno
BOOLEAN DoesO3SectorStatueExistHere( INT16 sGridNo )
{
	INT32 cnt;
	EXITGRID								ExitGrid;

	// First check current sector......
	if ( gWorldSectorX == 3 && gWorldSectorY == MAP_ROW_O && gbWorldSectorZ == 0 )
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
	EXITGRID								ExitGrid;
	UINT16									usTileIndex;
	INT16 sX, sY;

	// Remove old graphic
	ApplyMapChangesToMapTempFile( TRUE );
	// Remove it!
	// Get index for it...
	GetTileIndexFromTypeSubIndex( EIGHTOSTRUCT, (INT8)( 5 ), &usTileIndex );
	RemoveStruct( 13830, usTileIndex );

	// Add new one...
	if ( fFromExplosion )
	{
		// Use damaged peice
		GetTileIndexFromTypeSubIndex( EIGHTOSTRUCT, (INT8)( 7 ), &usTileIndex );
	}
	else
	{
		GetTileIndexFromTypeSubIndex( EIGHTOSTRUCT, (INT8)( 8 ), &usTileIndex );
		// Play sound...

    PlayJA2Sample( OPEN_STATUE, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN );

	}
	AddStructToHead( 13830, usTileIndex );

	// Add exit grid
	ExitGrid.ubGotoSectorX = 3;
	ExitGrid.ubGotoSectorY = MAP_ROW_O;
	ExitGrid.ubGotoSectorZ = 1;
	ExitGrid.usGridNo = 13037;

	AddExitGridToWorld( 13669, &ExitGrid );
	gpWorldLevelData[ 13669 ].uiFlags |= MAPELEMENT_REVEALED;

	// Turn off permenant changes....
	ApplyMapChangesToMapTempFile( FALSE );

	// Re-render the world!
	gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
	// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
	InvalidateWorldRedundency( );
	SetRenderFlags(RENDER_FLAG_FULL);

	// Redo movement costs....
	ConvertGridNoToXY( 13830, &sX, &sY );

	RecompileLocalMovementCostsFromRadius( 13830, 5 );

}

void DeidrannaTimerCallback( void )
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

void HandleDeidrannaDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;
	INT16		sDistVisible = FALSE;
	UINT8		ubKillerSoldierID = NOBODY;

	// Start victory music here...
	SetMusicMode( MUSIC_TACTICAL_VICTORY );


	if ( pKillerSoldier )
	{
		TacticalCharacterDialogue( pKillerSoldier, QUOTE_KILLING_DEIDRANNA );
		ubKillerSoldierID = pKillerSoldier->ubID;
	}

	// STEP 1 ) START ALL QUOTES GOING!
	// OK - loop through all witnesses and see if they want to say something abou this...
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		if ( cnt != ubKillerSoldierID )
		{
			if ( OK_INSECTOR_MERC( pTeamSoldier ) && !( pTeamSoldier->uiStatusFlags & SOLDIER_GASSED ) && !AM_AN_EPC( pTeamSoldier ) )
			{
					if ( QuoteExp_WitnessDeidrannaDeath[ pTeamSoldier->ubProfile ] )
					{
						// Can we see location?
						sDistVisible = DistanceVisible( pTeamSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, bLevel );

						if ( SoldierTo3DLocationLineOfSightTest( pTeamSoldier, sGridNo,  bLevel, 3, (UINT8) sDistVisible, TRUE ) )
						{
							TacticalCharacterDialogue( pTeamSoldier, QUOTE_KILLING_DEIDRANNA );
						}
					}
			}
		}
	}

	// Set fact that she is dead!
	SetFactTrue( FACT_QUEEN_DEAD );

	ExecuteStrategicAIAction( STRATEGIC_AI_ACTION_QUEEN_DEAD, 0, 0 );

	// AFTER LAST ONE IS DONE - PUT SPECIAL EVENT ON QUEUE TO BEGIN FADE< ETC
	SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_MULTIPURPOSE, MULTIPURPOSE_SPECIAL_EVENT_DONE_KILLING_DEIDRANNA, 0,0,0,0 );

}

void DoneFadeInKilledQueen( void )
{
	SOLDIERTYPE *pNPCSoldier;

	// Locate gridno.....

	// Run NPC script
	pNPCSoldier = FindSoldierByProfileID( 136, FALSE );
	if ( !pNPCSoldier )
	{
		return;
	}

	// Converse!
	//InitiateConversation( pNPCSoldier, pSoldier, 0, 1 );
	TriggerNPCRecordImmediately( pNPCSoldier->ubProfile, 6 );

}

void DoneFadeOutKilledQueen( void )
{
	INT32 cnt;
	SOLDIERTYPE *pSoldier, *pTeamSoldier;

	// For one, loop through our current squad and move them over
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
	{
		// Are we in this sector, On the current squad?
		if ( pSoldier->bActive && pSoldier->bLife >= OKLIFE && pSoldier->bInSector && pSoldier->bAssignment == CurrentSquad( ) )
		{
			gfTacticalTraversal = TRUE;
			SetGroupSectorValue( 3, MAP_ROW_P, 0, pSoldier->ubGroupID );

			// Set next sectore
			pSoldier->sSectorX = 3;
			pSoldier->sSectorY = MAP_ROW_P;
			pSoldier->bSectorZ = 0;

			// Set gridno
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			pSoldier->usStrategicInsertionData = 5687;
			// Set direction to face....
			pSoldier->ubInsertionDirection		 = 100 + NORTHWEST;
		}
	}

	// Kill all enemies in world.....
	cnt = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ ENEMY_TEAM ].bLastID; cnt++,pTeamSoldier++)
	{
		// Are we active and in sector.....
		if ( pTeamSoldier->bActive  )
		{
			// For sure for flag thet they are dead is not set
			// Check for any more badguys
			// ON THE STRAGETY LAYER KILL BAD GUYS!
			if ( !pTeamSoldier->bNeutral && (pTeamSoldier->bSide != gbPlayerNum ) )
			{
				ProcessQueenCmdImplicationsOfDeath( pSoldier );
			}
		}
	}

	// 'End' battle
	ExitCombatMode();
	gTacticalStatus.fLastBattleWon = TRUE;
	// Set enemy presence to false
	gTacticalStatus.fEnemyInSector = FALSE;

	SetMusicMode( MUSIC_TACTICAL_VICTORY );

	HandleMoraleEvent( NULL, MORALE_QUEEN_BATTLE_WON, 3, MAP_ROW_P, 0 );
	HandleGlobalLoyaltyEvent( GLOBAL_LOYALTY_QUEEN_BATTLE_WON, 3, MAP_ROW_P, 0 );

	SetMusicMode( MUSIC_TACTICAL_VICTORY );

	SetThisSectorAsPlayerControlled( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, TRUE );

	// ATE: Force change of level set z to 1
	gbWorldSectorZ = 1;

	// Clear out dudes.......
	SectorInfo[ SEC_P3 ].ubNumAdmins = 0;
	SectorInfo[ SEC_P3 ].ubNumTroops = 0;
	SectorInfo[ SEC_P3 ].ubNumElites = 0;
	SectorInfo[ SEC_P3 ].ubAdminsInBattle = 0;
	SectorInfo[ SEC_P3 ].ubTroopsInBattle = 0;
	SectorInfo[ SEC_P3 ].ubElitesInBattle = 0;

  // ATE: GEt rid of elliot in P3...
  gMercProfiles[ ELLIOT ].sSectorX = 1;

	ChangeNpcToDifferentSector( DEREK, 3, MAP_ROW_P, 0 );
	ChangeNpcToDifferentSector( OLIVER, 3, MAP_ROW_P, 0 );


	// OK, insertion data found, enter sector!
	SetCurrentWorldSector( 3, MAP_ROW_P, 0 );

	// OK, once down here, adjust the above map with crate info....
	gfTacticalTraversal = FALSE;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;

	gFadeInDoneCallback = DoneFadeInKilledQueen;

	FadeInGameScreen( );
}

// Called after all player quotes are done....
void HandleDoneLastKilledQueenQuote( )
{
	gFadeOutDoneCallback = DoneFadeOutKilledQueen;

	FadeOutGameScreen( );
}


void EndQueenDeathEndgameBeginEndCimenatic( )
{
	INT32 cnt;
	SOLDIERTYPE *pSoldier;

	// Start end cimimatic....
  gTacticalStatus.uiFlags |= IN_ENDGAME_SEQUENCE;

	// first thing is to loop through team and say end quote...
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// look for all mercs on the same team,
	for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier++)
	{
		// Are we in this sector, On the current squad?
		if ( pSoldier->bActive && pSoldier->bLife >= OKLIFE && !AM_AN_EPC( pSoldier ) )
		{
			TacticalCharacterDialogue( pSoldier, QUOTE_END_GAME_COMMENT );
		}
	}

	// Add queue event to proceed w/ smacker cimimatic
	SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_MULTIPURPOSE, MULTIPURPOSE_SPECIAL_EVENT_TEAM_MEMBERS_DONE_TALKING, 0,0,0,0 );

}

void EndQueenDeathEndgame( )
{
	// Unset flags...
	gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV );
	// Increment refrence count...
	giNPCReferenceCount = 0;

	gTacticalStatus.uiFlags &= (~IN_DEIDRANNA_ENDGAME);
}


void DoneFadeOutEndCinematic( void )
{
	// DAVE PUT SMAKER STUFF HERE!!!!!!!!!!!!
	// :)
  gTacticalStatus.uiFlags &= (~IN_ENDGAME_SEQUENCE);


	// For now, just quit the freaken game...
//	InternalLeaveTacticalScreen( MAINMENU_SCREEN );

	InternalLeaveTacticalScreen( INTRO_SCREEN );
//	guiCurrentScreen = INTRO_SCREEN;


	SetIntroType( INTRO_ENDING );
}

// OK, end death UI - fade to smaker....
void HandleDoneLastEndGameQuote( )
{
	EndQueenDeathEndgame( );

	gFadeOutDoneCallback = DoneFadeOutEndCinematic;

	FadeOutGameScreen( );
}



void QueenBitchTimerCallback( void )
{
	HandleQueenBitchDeath( gpKillerSoldier, gsGridNo, gbLevel );
}


void BeginHandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;


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
	cnt = gTacticalStatus.Team[ CREATURE_TEAM ].bFirstID;

	// look for all mercs on the same team,
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ CREATURE_TEAM ].bLastID; cnt++,pTeamSoldier++)
	{
		// Are we active and ALIVE and in sector.....
		if ( pTeamSoldier->bActive && pTeamSoldier->bLife > 0 )
		{
			// For sure for flag thet they are dead is not set
			// Check for any more badguys
			// ON THE STRAGETY LAYER KILL BAD GUYS!

			// HELLO!  THESE ARE CREATURES!  THEY CAN'T BE NEUTRAL!
			//if ( !pTeamSoldier->bNeutral && (pTeamSoldier->bSide != gbPlayerNum ) )
			{
    		gTacticalStatus.ubAttackBusyCount++;
				EVENT_SoldierGotHit( pTeamSoldier, 0, 10000, 0, pTeamSoldier->bDirection, 320, NOBODY , FIRE_WEAPON_NO_SPECIAL, pTeamSoldier->bAimShotLocation, 0, NOWHERE );
			}
		}
	}


}

void HandleQueenBitchDeath( SOLDIERTYPE *pKillerSoldier, INT16 sGridNo, INT8 bLevel )
{
	SOLDIERTYPE *pTeamSoldier;
	INT32 cnt;
	INT16		sDistVisible = FALSE;
	UINT8		ubKillerSoldierID = NOBODY;

	// Start victory music here...
	SetMusicMode( MUSIC_TACTICAL_VICTORY );

	if ( pKillerSoldier )
	{
		TacticalCharacterDialogue( pKillerSoldier, QUOTE_KILLING_QUEEN );
		ubKillerSoldierID = pKillerSoldier->ubID;
	}

	// STEP 1 ) START ALL QUOTES GOING!
	// OK - loop through all witnesses and see if they want to say something abou this...
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	// run through list
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pTeamSoldier++ )
	{
		if ( cnt != ubKillerSoldierID )
		{
			if ( OK_INSECTOR_MERC( pTeamSoldier ) && !( pTeamSoldier->uiStatusFlags & SOLDIER_GASSED ) && !AM_AN_EPC( pTeamSoldier ) )
			{
					if ( QuoteExp_WitnessQueenBugDeath[ pTeamSoldier->ubProfile ] )
					{
						// Can we see location?
						sDistVisible = DistanceVisible( pTeamSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, bLevel );

						if ( SoldierTo3DLocationLineOfSightTest( pTeamSoldier, sGridNo,  bLevel, 3, (UINT8) sDistVisible, TRUE ) )
						{
							TacticalCharacterDialogue( pTeamSoldier, QUOTE_KILLING_QUEEN );
						}
					}
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

