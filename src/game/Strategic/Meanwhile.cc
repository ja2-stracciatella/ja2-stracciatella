#include "MapScreen.h"
#include "Meanwhile.h"
#include "PreBattle_Interface.h"
#include "MessageBoxScreen.h"
#include "StrategicMap.h"
#include "Fade_Screen.h"
#include "ScreenIDs.h"
#include "JAScreens.h"
#include "NPC.h"
#include "Game_Event_Hook.h"
#include "Game_Clock.h"
#include "Tactical_Save.h"
#include "Soldier_Profile.h"
#include "Overhead.h"
#include "Dialogue_Control.h"
#include "Assignments.h"
#include "Text.h"
#include "GameSettings.h"
#include "Interface_Control.h"
#include "Interface_Items.h"
#include "Map_Information.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Interface.h"
#include "Game_Events.h"
#include "Strategic_AI.h"
#include "Interface_Dialogue.h"
#include "Quests.h"
#include "Campaign_Types.h"
#include "Squads.h"
#include "Random.h"


#define MAX_MEANWHILE_PROFILES	10

// the snap to grid nos for meanwhile scenes
static const UINT16 gusMeanWhileGridNo[] =
{
	12248,
	12248,
	12248,
	12248,
	12248,
	12248,
	12248,
	12248,
	12248,
	12248,
	12248,
	8075,
	12248,
	12248,
	12248,
	12248,
	12248,
};


struct NPC_SAVE_INFO
{
	UINT8 ubProfile;
	INT16 sX;
	INT16 sY;
	INT16 sZ;
	INT16 sGridNo;
};


// BEGIN SERALIZATION
MEANWHILE_DEFINITION gCurrentMeanwhileDef;
MEANWHILE_DEFINITION gMeanwhileDef[NUM_MEANWHILES];
BOOLEAN              gfMeanwhileTryingToStart = FALSE;
BOOLEAN              gfInMeanwhile = FALSE;
// END SERIALIZATION
static INT16 gsOldSectorX;
static INT16 gsOldSectorY;
static INT16 gsOldSectorZ;
static INT16 gsOldSelectedSectorX;
static INT16 gsOldSelectedSectorY;
static INT16 gsOldSelectedSectorZ;

static UINT32        guiOldScreen;
static NPC_SAVE_INFO gNPCSaveData[MAX_MEANWHILE_PROFILES];
static UINT32        guiNumNPCSaves = 0;
static BOOLEAN       gfReloadingScreenFromMeanwhile = FALSE;
static BOOLEAN       gfWorldWasLoaded = FALSE;
static UINT8         ubCurrentMeanWhileId = 0;

UINT32 uiMeanWhileFlags = 0;

// meanwhile flag defines
#define END_OF_PLAYERS_FIRST_BATTLE_FLAG	0x00000001
#define DRASSEN_LIBERATED_FLAG			0x00000002
#define CAMBRIA_LIBERATED_FLAG			0x00000004
#define ALMA_LIBERATED_FLAG			0x00000008
#define GRUMM_LIBERATED_FLAG			0x00000010
#define CHITZENA_LIBERATED_FLAG			0x00000020
#define NW_SAM_FLAG				0x00000040
#define NE_SAM_FLAG				0x00000080
#define CENTRAL_SAM_FLAG			0x00000100
#define FLOWERS_FLAG				0x00000200
#define LOST_TOWN_FLAG				0x00000400
#define CREATURES_FLAG				0x00000800
#define KILL_CHOPPER_FLAG			0x00001000
#define AWOL_SCIENTIST_FLAG			0x00002000
#define OUTSKIRTS_MEDUNA_FLAG			0x00004000
#define INTERROGATION_FLAG			0x00008000
#define BALIME_LIBERATED_FLAG			0x00010000


static UINT32 MeanwhileIDToFlag(UINT8 const meanwhile_id)
{
	switch (meanwhile_id)
	{
		case END_OF_PLAYERS_FIRST_BATTLE: return END_OF_PLAYERS_FIRST_BATTLE_FLAG;
		case DRASSEN_LIBERATED:           return DRASSEN_LIBERATED_FLAG;
		case CAMBRIA_LIBERATED:           return CAMBRIA_LIBERATED_FLAG;
		case ALMA_LIBERATED:              return ALMA_LIBERATED_FLAG;
		case GRUMM_LIBERATED:             return GRUMM_LIBERATED_FLAG;
		case CHITZENA_LIBERATED:          return CHITZENA_LIBERATED_FLAG;
		case BALIME_LIBERATED:            return BALIME_LIBERATED_FLAG;
		case NW_SAM:                      return NW_SAM_FLAG;
		case NE_SAM:                      return NE_SAM_FLAG;
		case CENTRAL_SAM:                 return CENTRAL_SAM_FLAG;
		case FLOWERS:                     return FLOWERS_FLAG;
		case LOST_TOWN:                   return LOST_TOWN_FLAG;
		case CREATURES:                   return CREATURES_FLAG;
		case KILL_CHOPPER:                return KILL_CHOPPER_FLAG;
		case AWOL_SCIENTIST:              return AWOL_SCIENTIST_FLAG;
		case OUTSKIRTS_MEDUNA:            return OUTSKIRTS_MEDUNA_FLAG;
		case INTERROGATION:               return INTERROGATION_FLAG;
		default:                          return 0;
	}
}


// set flag for this event
static void SetMeanWhileFlag(UINT8 const meanwhile_id)
{
	uiMeanWhileFlags |= MeanwhileIDToFlag(meanwhile_id);
}


// is this flag set?
static bool GetMeanWhileFlag(UINT8 const meanwhile_id)
{
	return uiMeanWhileFlags & MeanwhileIDToFlag(meanwhile_id);
}


static NPC_SAVE_INFO* GetFreeNPCSave(void)
{
	for (NPC_SAVE_INFO* si = gNPCSaveData; si != gNPCSaveData + guiNumNPCSaves; ++si)
	{
		if (si->ubProfile == NO_PROFILE) return si;
	}
	if (guiNumNPCSaves < MAX_MEANWHILE_PROFILES)
	{
		return &gNPCSaveData[guiNumNPCSaves++];
	}
	return NULL;
}


void ScheduleMeanwhileEvent(INT16 const x, INT16 const y, UINT16 const trigger_event, UINT8 const meanwhile_id, UINT8 const npc, UINT32 const time)
{
	// event scheduled to happen before, ignore
	if (GetMeanWhileFlag(meanwhile_id)) return;

	// set the meanwhile flag for this event
	SetMeanWhileFlag(meanwhile_id);

	// set the id value
	ubCurrentMeanWhileId = meanwhile_id;

	// Copy definiaiotn structure into position in global array....
	MEANWHILE_DEFINITION& m = gMeanwhileDef[meanwhile_id];
	m.sSectorX       = x;
	m.sSectorY       = y;
	m.usTriggerEvent = trigger_event;
	m.ubMeanwhileID  = meanwhile_id;
	m.ubNPCNumber    = npc;

	// A meanwhile.. poor elliot!
	// increment his slapped count...

	// We need to do it here 'cause they may skip it...
	if ( gMercProfiles[ ELLIOT ].bNPCData != 17 )
	{
		gMercProfiles[ ELLIOT ].bNPCData++;
	}

	AddStrategicEvent(EVENT_MEANWHILE, time, meanwhile_id);
}


void BeginMeanwhile(UINT8 ubMeanwhileID)
{
	INT32 cnt;

	// Save if we are in Dead is Dead Mode
	DoDeadIsDeadSaveIfNecessary();

	// copy meanwhile data from array to structure for current
	gCurrentMeanwhileDef = gMeanwhileDef[ubMeanwhileID];

	gfMeanwhileTryingToStart = TRUE;
	PauseGame();
	// prevent anyone from messing with the pause!
	LockPauseState(LOCK_PAUSE_06);

	// Set NO_PROFILE info....
	for ( cnt = 0; cnt < MAX_MEANWHILE_PROFILES; cnt++ )
	{
		gNPCSaveData[ cnt ].ubProfile = NO_PROFILE;
	}
}


static void BeginMeanwhileCallBack(MessageBoxReturnValue);


static void BringupMeanwhileBox(void)
{
	wchar_t zStr[256];

	swprintf( zStr, lengthof(zStr), L"%ls.....", pMessageStrings[ MSG_MEANWHILE ] );
	MessageBoxFlags const flags =
		gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION
		&& MeanwhileSceneSeen(gCurrentMeanwhileDef.ubMeanwhileID)
		? MSG_BOX_FLAG_OKSKIP : MSG_BOX_FLAG_OK;
	DoMessageBox(MSG_BOX_BASIC_STYLE, zStr, guiCurrentScreen, flags, BeginMeanwhileCallBack, NULL);
}

void CheckForMeanwhileOKStart( )
{
	if ( gfMeanwhileTryingToStart )
	{
		// Are we in prebattle interface?
		if ( gfPreBattleInterfaceActive )
		{
			return;
		}

		if ( !InterfaceOKForMeanwhilePopup() )
		{
			return;
		}

		if ( !DialogueQueueIsEmptyOrSomebodyTalkingNow( ) )
		{
			return;
		}

		gfMeanwhileTryingToStart = FALSE;

		guiOldScreen = guiCurrentScreen;

		if ( guiCurrentScreen == GAME_SCREEN )
		{
			LeaveTacticalScreen( GAME_SCREEN );
		}



		// We need to make sure we have no item - at least in tactical
		// In mapscreen, time is paused when manipulating items...
		CancelItemPointer( );

		BringupMeanwhileBox( );
	}
}


static void SetNPCMeanwhile(const ProfileID pid, const INT16 sector_x, const INT16 sector_y)
{
	NPC_SAVE_INFO* const si = GetFreeNPCSave();
	if (si == NULL) return;

	MERCPROFILESTRUCT& p = GetProfile(pid);
	si->ubProfile = pid;
	si->sX        = p.sSectorX;
	si->sY        = p.sSectorY;
	si->sZ        = p.bSectorZ;
	si->sGridNo   = p.sGridNo;

	ReloadQuoteFile(pid);
	ChangeNpcToDifferentSector(p, sector_x, sector_y, 0);
}


static void DoneFadeOutMeanwhile(void);


static void StartMeanwhile(void)
{
	// OK, save old position...
	if ( gfWorldLoaded )
	{
		gsOldSectorX = gWorldSectorX;
		gsOldSectorY = gWorldSectorY;
		gsOldSectorZ = gbWorldSectorZ;
	}

	gsOldSelectedSectorX = sSelMapX;
	gsOldSelectedSectorY = sSelMapY;
	gsOldSelectedSectorZ = (INT16) iCurrentMapSectorZ;

	gfInMeanwhile = TRUE;

	// ATE: Change music before load
	SetMusicMode( MUSIC_MAIN_MENU );


	gfWorldWasLoaded = gfWorldLoaded;

	// Setup NPC locations, depending on meanwhile type...
	switch( gCurrentMeanwhileDef.ubMeanwhileID )
	{
		case	END_OF_PLAYERS_FIRST_BATTLE:
		case	DRASSEN_LIBERATED:
		case	CAMBRIA_LIBERATED:
		case	ALMA_LIBERATED:
		case	GRUMM_LIBERATED:
		case	CHITZENA_LIBERATED:
		case  BALIME_LIBERATED:
		case	NW_SAM:
		case	NE_SAM:
		case	CENTRAL_SAM:
		case	FLOWERS:
		case	LOST_TOWN:
		case	CREATURES:
		case	KILL_CHOPPER:
		case	AWOL_SCIENTIST:
		case	OUTSKIRTS_MEDUNA:
			SetNPCMeanwhile(QUEEN,  3, 16);
			SetNPCMeanwhile(ELLIOT, 3, 16);
			if (gCurrentMeanwhileDef.ubMeanwhileID == OUTSKIRTS_MEDUNA)
			{
				SetNPCMeanwhile(JOE, 3, 16);
			}
			break;

		case	INTERROGATION:
			SetNPCMeanwhile(QUEEN,  7, 14);
			SetNPCMeanwhile(ELLIOT, 7, 14);
			SetNPCMeanwhile(JOE,    7, 14);
			break;
	}

	// fade out old screen....
	FadeOutNextFrame( );

	// Load new map....
	gFadeOutDoneCallback = DoneFadeOutMeanwhile;


}


static void DoneFadeInMeanwhile(void);
static void LocateMeanWhileGrid(void);


static void DoneFadeOutMeanwhile(void)
{
	// OK, insertion data found, enter sector!

	SetCurrentWorldSector( gCurrentMeanwhileDef.sSectorX, gCurrentMeanwhileDef.sSectorY, 0 );

	//LocateToMeanwhileCharacter( );
	LocateMeanWhileGrid( );

	gFadeInDoneCallback = DoneFadeInMeanwhile;

	FadeInNextFrame( );
}


static void DoneFadeInMeanwhile(void)
{
	// ATE: double check that we are in meanwhile
	// this is if we cancel right away.....
	if ( gfInMeanwhile )
	{
		giNPCReferenceCount = 1;

		if ( gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION )
		{
			gTacticalStatus.uiFlags |= SHOW_ALL_MERCS;
		}

		TriggerNPCRecordImmediately( gCurrentMeanwhileDef.ubNPCNumber, (UINT8)gCurrentMeanwhileDef.usTriggerEvent );
	}
}


static void ProcessImplicationsOfMeanwhile(void);


static void BeginMeanwhileCallBack(MessageBoxReturnValue const bExitValue)
{
	if ( bExitValue == MSG_BOX_RETURN_OK || bExitValue == MSG_BOX_RETURN_YES )
	{
		gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
		// Increment reference count...
		giNPCReferenceCount = 1;

		StartMeanwhile( );
	}
	else
	{
		// skipped scene!
		gfInMeanwhile = FALSE;
		ProcessImplicationsOfMeanwhile();
		UnLockPauseState();
		UnPauseGame();
	}
}


bool AreInMeanwhile()
{
	/* KM: April 6, 1999
	 * Tactical traversal needs to take precedence over meanwhile events. When
	 * tactically traversing, we expect to make it to the other side without
	 * interruption. */
	if (gfTacticalTraversal) return false;

	if (gfInMeanwhile) return true;

	/* Check to make sure a meanwhile scene isn't in the event list occurring at
	 * the exact same time as this call. Meanwhile scenes have precedence over a
	 * new battle if they occur in the same second. */
	for (STRATEGICEVENT const* i = gpEventList; i; i = i->next)
	{
		if (i->uiTimeStamp != GetWorldTotalSeconds()) return false;
		if (i->ubCallbackID == EVENT_MEANWHILE)       return true;
	}

	return false;
}


static void ProcessImplicationsOfMeanwhile(void)
{
	switch( gCurrentMeanwhileDef.ubMeanwhileID )
	{
		case END_OF_PLAYERS_FIRST_BATTLE:
			if( gGameOptions.ubDifficultyLevel == DIF_LEVEL_HARD )
			{ //Wake up the queen earlier to punish the good players!
				ExecuteStrategicAIAction( STRATEGIC_AI_ACTION_WAKE_QUEEN, 0, 0 );
			}
			HandleNPCDoAction( QUEEN, NPC_ACTION_SEND_SOLDIERS_TO_BATTLE_LOCATION, 0 );
			break;
		case CAMBRIA_LIBERATED:
		case ALMA_LIBERATED:
		case GRUMM_LIBERATED:
		case CHITZENA_LIBERATED:
		case BALIME_LIBERATED:
			ExecuteStrategicAIAction( STRATEGIC_AI_ACTION_WAKE_QUEEN, 0, 0 );
			break;
		case DRASSEN_LIBERATED:
			ExecuteStrategicAIAction( STRATEGIC_AI_ACTION_WAKE_QUEEN, 0, 0 );
			HandleNPCDoAction( QUEEN, NPC_ACTION_SEND_SOLDIERS_TO_DRASSEN, 0 );
			break;
		case CREATURES:
			// add Rat
			HandleNPCDoAction( QUEEN, NPC_ACTION_ADD_RAT, 0 );
			break;
		case AWOL_SCIENTIST:
			{
				INT16	sSectorX, sSectorY; // XXX HACK000E

				StartQuest( QUEST_FIND_SCIENTIST, -1, -1 );
				// place Madlab and robot!
				if ( SectorInfo[ SECTOR( 7, MAP_ROW_H ) ].uiFlags & SF_USE_ALTERNATE_MAP )
				{
					sSectorX = 7;
					sSectorY = MAP_ROW_H;
				}
				else if ( SectorInfo[ SECTOR( 16, MAP_ROW_H ) ].uiFlags & SF_USE_ALTERNATE_MAP )
				{
					sSectorX = 16;
					sSectorY = MAP_ROW_H;
				}
				else if ( SectorInfo[ SECTOR( 11, MAP_ROW_I ) ].uiFlags & SF_USE_ALTERNATE_MAP )
				{
					sSectorX = 11;
					sSectorY = MAP_ROW_I;
				}
				else if ( SectorInfo[ SECTOR( 4, MAP_ROW_E ) ].uiFlags & SF_USE_ALTERNATE_MAP )
				{
					sSectorX = 4;
					sSectorY = MAP_ROW_E;
				}
				else
				{
					abort(); // HACK000E
				}
				gMercProfiles[ MADLAB ].sSectorX = sSectorX;
				gMercProfiles[ MADLAB ].sSectorY = sSectorY;
				gMercProfiles[ MADLAB ].bSectorZ = 0;

				gMercProfiles[ ROBOT ].sSectorX = sSectorX;
				gMercProfiles[ ROBOT ].sSectorY = sSectorY;
				gMercProfiles[ ROBOT ].bSectorZ = 0;
			}
			break;

		{
			UINT8 sector;
		case NW_SAM:      sector = pSamList[0]; goto send_troops_to_sam;
		case NE_SAM:      sector = pSamList[1]; goto send_troops_to_sam;
		case CENTRAL_SAM: sector = pSamList[2]; goto send_troops_to_sam;
send_troops_to_sam:
			ExecuteStrategicAIAction(NPC_ACTION_SEND_TROOPS_TO_SAM, SECTORX(sector), SECTORY(sector));
			break;
		}

		default:
			break;
	}
}


static void RestoreNPCMeanwhile(void)
{
	// ATE: Restore people to saved positions...
	// OK, restore NPC save info...
	for (const NPC_SAVE_INFO* si = gNPCSaveData, *const end = gNPCSaveData + guiNumNPCSaves; si != end; ++si)
	{
		const ProfileID pid = si->ubProfile;
		if (pid == NO_PROFILE) continue;

		MERCPROFILESTRUCT& p = GetProfile(pid);
		p.sSectorX = si->sX;
		p.sSectorY = si->sY;
		p.bSectorZ = (INT8)si->sZ;
		p.sGridNo  = (INT8)si->sGridNo;

		// Ensure NPC files loaded...
		ReloadQuoteFile(pid);
	}
}


static void DoneFadeOutMeanwhileOnceDone(void);


void EndMeanwhile( )
{
	EmptyDialogueQueue();
	ProcessImplicationsOfMeanwhile();
	SetMeanwhileSceneSeen( gCurrentMeanwhileDef.ubMeanwhileID );

	gfInMeanwhile = FALSE;
	giNPCReferenceCount = 0;

	gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV );

	UnLockPauseState();
	UnPauseGame();

	// ATE: Make sure!
	TurnOffSectorLocator();

	if ( gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION )
	{
		gTacticalStatus.uiFlags &= (~SHOW_ALL_MERCS);

		// OK, load old sector again.....
		FadeOutNextFrame( );

		// Load new map....
		gFadeOutDoneCallback = DoneFadeOutMeanwhileOnceDone;
	}
	else
	{
		// We leave this sector open for our POWs to escape!
		// Set music mode to enemy present!
		SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

		RestoreNPCMeanwhile();
	}
}


static void DoneFadeInMeanwhileOnceDone(void);


static void DoneFadeOutMeanwhileOnceDone(void)
{
	// OK, insertion data found, enter sector!
	gfReloadingScreenFromMeanwhile = TRUE;


	if( gfWorldWasLoaded )
	{
		SetCurrentWorldSector( gsOldSectorX, gsOldSectorY, (INT8)gsOldSectorZ );

		ExamineCurrentSquadLights( );
	}
	else
	{
		TrashWorld( );
		// NB no world is loaded!
		SetWorldSectorInvalid();
	}

	ChangeSelectedMapSector( gsOldSelectedSectorX, gsOldSelectedSectorY, (INT8) gsOldSelectedSectorZ );

	gfReloadingScreenFromMeanwhile = FALSE;

	RestoreNPCMeanwhile();

	gFadeInDoneCallback = DoneFadeInMeanwhileOnceDone;

	// OK, based on screen we were in....
	switch( guiOldScreen )
	{
		case MAP_SCREEN:
			InternalLeaveTacticalScreen( MAP_SCREEN );
			//gfEnteringMapScreen = TRUE;
			break;

		case GAME_SCREEN:
			// restore old interface panel flag
			SetCurrentInterfacePanel(TEAM_PANEL);
			break;
	}

	FadeInNextFrame( );

}


static void DoneFadeInMeanwhileOnceDone(void)
{

}


static void LocateMeanWhileGrid(void)
{
	INT16 sGridNo = 0;

	// go to the approp. gridno
	sGridNo = gusMeanWhileGridNo[ ubCurrentMeanWhileId ];

	InternalLocateGridNo( sGridNo, TRUE );
}

void LocateToMeanwhileCharacter( )
{
	if ( gfInMeanwhile )
	{
		SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(gCurrentMeanwhileDef.ubNPCNumber);
		if (pSoldier != NULL) LocateSoldier(pSoldier, FALSE);
	}
}


BOOLEAN AreReloadingFromMeanwhile( )
{
	return( gfReloadingScreenFromMeanwhile );
}

UINT8 GetMeanwhileID( )
{
	return( gCurrentMeanwhileDef.ubMeanwhileID );
}


void HandleCreatureRelease( void )
{
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, CREATURES, QUEEN, uiTime);
}


void HandleMeanWhileEventPostingForTownLiberation( UINT8 bTownId )
{
	UINT32 const uiTime = GetWorldTotalMin() + 5;

	UINT8 ubId;
	switch (bTownId) // which town liberated?
	{
		case DRASSEN:  ubId = DRASSEN_LIBERATED;  break;
		case CAMBRIA:  ubId = CAMBRIA_LIBERATED;  break;
		case ALMA:     ubId = ALMA_LIBERATED;     break;
		case GRUMM:    ubId = GRUMM_LIBERATED;    break;
		case CHITZENA: ubId = CHITZENA_LIBERATED; break;
		case BALIME:   ubId = BALIME_LIBERATED;   break;
		default: return;
	}
	ScheduleMeanwhileEvent(3, 16, 0, ubId, QUEEN, uiTime);
}


void HandleMeanWhileEventPostingForTownLoss()
{
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, LOST_TOWN, QUEEN, uiTime);
}


void HandleMeanWhileEventPostingForSAMLiberation( INT8 bSamId )
{
	UINT8 ubId;
	switch (bSamId) // which SAM liberated?
	{
		case 0:  ubId = NW_SAM;      break;
		case 1:  ubId = NE_SAM;      break;
		case 2:  ubId = CENTRAL_SAM; break;
		case 3:  return; // no meanwhile scene for this SAM site
		default: return; // invalid parameter
	}
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, ubId, QUEEN, uiTime);
}


void HandleFlowersMeanwhileScene( INT8 bTimeCode )
{
	UINT32 uiTime = 0;

	// make sure scene hasn't been used before
	if ( GetMeanWhileFlag( FLOWERS ) )
	{
		return;
	}

	// time delay should be based on time code, 0 next day, 1 seeral days (random)
	if ( bTimeCode == 0 )
	{
		// 20-24 hours later
		uiTime = GetWorldTotalMin() + 60 * ( 20 + Random( 5 ) );
	}
	else
	{
		// 2-4 days later
		uiTime = GetWorldTotalMin() + 60 * ( 24 + Random( 48 ) );
	}

	ScheduleMeanwhileEvent(3, 16, 0, FLOWERS, QUEEN, uiTime);
}


void HandleOutskirtsOfMedunaMeanwhileScene( void )
{
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, OUTSKIRTS_MEDUNA, QUEEN, uiTime);
}


void HandleKillChopperMeanwhileScene( void )
{
	// make sure scene hasn't been used before
	if ( GetMeanWhileFlag( KILL_CHOPPER ) )
	{
		return;
	}

	UINT32 const uiTime = GetWorldTotalMin() + 55 + Random(10);
	ScheduleMeanwhileEvent(3, 16, 0, KILL_CHOPPER, QUEEN, uiTime);
}


void HandleScientistAWOLMeanwhileScene( void )
{
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, AWOL_SCIENTIST, QUEEN, uiTime);
}


static void HandleFirstBattleVictory(void)
{
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, END_OF_PLAYERS_FIRST_BATTLE, QUEEN, uiTime);
}


static void HandleDelayedFirstBattleVictory(void)
{
	/*
	//It is theoretically impossible to liberate a town within 60 minutes of the first battle (which is supposed to
	//occur outside of a town in this scenario).  The delay is attributed to the info taking longer to reach the queen.
	UINT32 const uiTime = GetWorldTotalMin() + 60;
	*/
	UINT32 const uiTime = GetWorldTotalMin() + 5;
	ScheduleMeanwhileEvent(3, 16, 0, END_OF_PLAYERS_FIRST_BATTLE, QUEEN, uiTime);
}


void HandleFirstBattleEndingWhileInTown( INT16 sSectorX, INT16 sSectorY, INT16 bSectorZ, BOOLEAN fFromAutoResolve )
{
	INT8 bTownId = 0;
	INT16 sSector = 0;

	if ( GetMeanWhileFlag( END_OF_PLAYERS_FIRST_BATTLE ) )
	{
		return;
	}

	// if this is in fact a town and it is the first battle, then set gfFirstBattleMeanwhileScenePending true
	// if  is true then this is the end of the second battle, post the first meanwhile OR, on call to trash world, that
	// means player is leaving sector

	// grab sector value
	sSector = sSectorX + sSectorY * MAP_WORLD_X;

	// get town name id
	bTownId = StrategicMap[ sSector ].bNameId;

	if ( bTownId == BLANK_SECTOR )
	{
		// invalid town
		HandleDelayedFirstBattleVictory( );
		gfFirstBattleMeanwhileScenePending = FALSE;
	}
	else if ( gfFirstBattleMeanwhileScenePending || fFromAutoResolve )
	{
		HandleFirstBattleVictory( );
		gfFirstBattleMeanwhileScenePending = FALSE;
	}
	else
	{
		gfFirstBattleMeanwhileScenePending = TRUE;
	}
}


void HandleFirstMeanWhileSetUpWithTrashWorld( void )
{

	// exiting sector after first battle fought
	if( gfFirstBattleMeanwhileScenePending )
	{
		HandleFirstBattleVictory( );
		gfFirstBattleMeanwhileScenePending = FALSE;
	}

}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Meanwhile, asserts)
{
	EXPECT_EQ(sizeof(MEANWHILE_DEFINITION), 8);
}

#endif
