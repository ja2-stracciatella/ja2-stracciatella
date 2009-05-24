#include <stdexcept>

#include "Buffer.h"
#include "Font.h"
#include "Font_Control.h"
#include "GameLoop.h"
#include "LoadSaveData.h"
#include "LoadSaveEMail.h"
#include "LoadSaveMercProfile.h"
#include "LoadSaveSoldierType.h"
#include "LoadSaveTacticalStatusType.h"
#include "Local.h"
#include "MapScreen.h"
#include "Timer_Control.h"
#include "Types.h"
#include "Soldier_Profile.h"
#include "FileMan.h"
#include "Debug.h"
#include "Overhead.h"
#include "Keys.h"
#include "Finances.h"
#include "History.h"
#include "Files.h"
#include "Laptop.h"
#include "StrategicMap.h"
#include "Game_Events.h"
#include "Game_Clock.h"
#include "Soldier_Create.h"
#include "WorldDef.h"
#include "LaptopSave.h"
#include "Queen_Command.h"
#include "SaveLoadGame.h"
#include "Tactical_Save.h"
#include "Squads.h"
#include "Environment.h"
#include "Lighting.h"
#include "Strategic_Movement.h"
#include "Quests.h"
#include "OppList.h"
#include "Message.h"
#include "NPC.h"
#include "Merc_Hiring.h"
#include "SaveLoadScreen.h"
#include "GameVersion.h"
#include "GameSettings.h"
#include "Music_Control.h"
#include "Options_Screen.h"
#include "AI.h"
#include "RenderWorld.h"
#include "SmokeEffects.h"
#include "Random.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Interface.h"
#include "Map_Screen_Helicopter.h"
#include "Arms_Dealer_Init.h"
#include "Tactical_Placement_GUI.h"
#include "Strategic_Mines.h"
#include "Strategic_Town_Loyalty.h"
#include "Vehicles.h"
#include "Merc_Contract.h"
#include "Bullets.h"
#include "Air_Raid.h"
#include "Physics.h"
#include "Strategic_Pathing.h"
#include "TeamTurns.h"
#include "Explosion_Control.h"
#include "Creature_Spreading.h"
#include "Strategic_Status.h"
#include "PreBattle_Interface.h"
#include "Boxing.h"
#include "Strategic_AI.h"
#include "Map_Screen_Interface_Map.h"
#include "Meanwhile.h"
#include "Dialogue_Control.h"
#include "Text.h"
#include "LightEffects.h"
#include "HelpScreen.h"
#include "Animated_ProgressBar.h"
#include "MercTextBox.h"
#include "Map_Information.h"
#include "Interface_Items.h"
#include "Civ_Quotes.h"
#include "Scheduling.h"
#include "Animation_Data.h"
#include "Cheats.h"
#include "Render_Dirty.h"
#include "Strategic_Event_Handler.h"
#include "Interface_Panels.h"
#include "Interface_Dialogue.h"
#include "Assignments.h"
#include "Video.h"
#include "VSurface.h"
#include "MemMan.h"
#include "JAScreens.h"
#include "BobbyR.h"
#include "IMP_Portraits.h"
#include "Loading_Screen.h"
#include "Interface_Utils.h"
#include "IMP_Confirm.h"
#include "Enemy_Soldier_Save.h"
#include "BobbyRMailOrder.h"
#include "Mercs.h"


static const char g_quicksave_name[] = "QuickSave";
static const char g_savegame_name[]  = "SaveGame";
static const char g_savegame_ext[]   = "sav";
static const char g_savegame_dir[]   = "../SavedGames";


//Global variable used
#ifdef JA2BETAVERSION
static UINT32 guiNumberOfMapTempFiles; // Test purposes
static UINT32 guiSizeOfTempFiles;
static char   gzNameOfMapTempFile[128];
#endif

extern		INT32					giSortStateForMapScreenList;
extern		INT16					sDeadMercs[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];
extern		INT32					giRTAILastUpdateTime;
extern		BOOLEAN				gfRedrawSaveLoadScreen;
extern		UINT8					gubScreenCount;
extern		INT16					sWorldSectorLocationOfFirstBattle;
extern		BOOLEAN				gfLoadedGame;
extern		UINT8					gubDesertTemperature;
extern		UINT8					gubGlobalTemperature;
extern		BOOLEAN				gfCreatureMeanwhileScenePlayed;
#ifdef JA2BETAVERSION
	extern		UINT8					gubReportMapscreenLock;
#endif

static UINT8 gMusicModeToPlay;

BOOLEAN	gfUseConsecutiveQuickSaveSlots = FALSE;
#ifdef JA2BETAVERSION
static UINT32 guiCurrentQuickSaveNumber = 0;
#endif
UINT32	guiLastSaveGameNum;

UINT32	guiJA2EncryptionSet = 0;


ScreenID guiScreenToGotoAfterLoadingSavedGame = ERROR_SCREEN; // XXX TODO001A was not properly initialised (0)

extern		UINT32		guiCurrentUniqueSoldierId;


static BYTE const* ExtractGameOptions(BYTE const* const data, GAME_OPTIONS& g)
{
	BYTE const* d = data;
	EXTR_BOOL( d, g.fGunNut)
	EXTR_BOOL( d, g.fSciFi)
	EXTR_U8(   d, g.ubDifficultyLevel)
	EXTR_BOOL( d, g.fTurnTimeLimit)
	EXTR_BOOL( d, g.fIronManMode)
	EXTR_SKIP( d, 7)
	Assert(d == data + 12);
	return d;
}


static BYTE* InjectGameOptions(BYTE* const data, GAME_OPTIONS const& g)
{
	BYTE* d = data;
	INJ_BOOL( d, g.fGunNut)
	INJ_BOOL( d, g.fSciFi)
	INJ_U8(   d, g.ubDifficultyLevel)
	INJ_BOOL( d, g.fTurnTimeLimit)
	INJ_BOOL( d, g.fIronManMode)
	INJ_SKIP( d, 7)
	Assert(d == data + 12);
	return d;
}


static UINT32 CalcJA2EncryptionSet(SAVED_GAME_HEADER* pSaveGameHeader);
static void PauseBeforeSaveGame(void);
static void UnPauseAfterSaveGame(void);
static void SaveGeneralInfo(HWFILE);
static void SaveMeanwhileDefsFromSaveGameFile(HWFILE);
static void SaveMercProfiles(HWFILE);
static void SaveOppListInfoToSavedGame(HWFILE);
static void SavePreRandomNumbersToSaveGameFile(HWFILE);
static void SaveSoldierStructure(HWFILE hFile);
static void SaveTacticalStatusToSavedGame(HWFILE);
static void SaveWatchedLocsToSavedGame(HWFILE);

#ifdef JA2BETAVERSION
static void InitSaveGameFilePosition(UINT8 slot);
static void InitShutDownMapTempFileTest(BOOLEAN fInit, const char* pNameOfFile, UINT8 ubSaveGameID);
static void SaveGameFilePosition(UINT8 slot, const HWFILE save, const char* pMsg);
#else
#	define InitSaveGameFilePosition(slot)              ((void)0)
#	define InitShutDownMapTempFileTest(init, name, id) ((void)0)
#	define SaveGameFilePosition(slot, file, msg)       ((void)0)
#endif


BOOLEAN SaveGame( UINT8 ubSaveGameID, const wchar_t *GameDesc)
{
	CHAR8		zSaveGameName[ 512 ];
	BOOLEAN	fPausedStateBeforeSaving = gfGamePaused;
	BOOLEAN	fLockPauseStateBeforeSaving = gfLockPauseState;
	UINT16	usPosX, usActualWidth, usActualHeight;
	BOOLEAN fWePausedIt = FALSE;

	if( ubSaveGameID >= NUM_SAVE_GAMES && ubSaveGameID != SAVE__ERROR_NUM && ubSaveGameID != SAVE__END_TURN_NUM )
		return( FALSE );

	if ( !GamePaused() )
	{
		PauseBeforeSaveGame();
		fWePausedIt = TRUE;
	}

	InitShutDownMapTempFileTest(TRUE, "SaveMapTempFile", ubSaveGameID);

	//Place a message on the screen telling the user that we are saving the game
	{ AutoMercPopUpBox const save_load_game_message_box(PrepareMercPopupBox(0, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, zSaveLoadText[SLG_SAVING_GAME_MESSAGE], 300, 0, 0, 0, &usActualWidth, &usActualHeight));
		usPosX = (SCREEN_WIDTH - usActualWidth) / 2;
		RenderMercPopUpBox(save_load_game_message_box, usPosX, 160, FRAME_BUFFER);
	}

	InvalidateScreen();

	ExecuteBaseDirtyRectQueue( );
	RefreshScreen();

	//
	// make sure we redraw the screen when we are done
	//

	//if we are in the game screen
	if( guiCurrentScreen == GAME_SCREEN )
	{
		SetRenderFlags( RENDER_FLAG_FULL );
	}

	else if( guiCurrentScreen == MAP_SCREEN )
	{
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}

	else if( guiCurrentScreen == SAVE_LOAD_SCREEN )
	{
		gfRedrawSaveLoadScreen = TRUE;
	}

	InitSaveGameFilePosition(ubSaveGameID);

	//Set the fact that we are saving a game
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;


	try
	{
		//Save the current sectors open temp files to the disk
		SaveCurrentSectorsInformationToTempItemFile();

		SAVED_GAME_HEADER SaveGameHeader;
		memset(&SaveGameHeader, 0, sizeof(SaveGameHeader));

		//if we are saving the quick save,
		if (ubSaveGameID == 0)
		{
#ifdef JA2BETAVERSION
			//Increment the quicksave counter
			guiCurrentQuickSaveNumber++;

			if (gfUseConsecutiveQuickSaveSlots)
				swprintf(SaveGameHeader.sSavedGameDesc, lengthof(SaveGameHeader.sSavedGameDesc), L"%hs%03d", g_quicksave_name, guiCurrentQuickSaveNumber);
			else
#endif
				swprintf(SaveGameHeader.sSavedGameDesc, lengthof(SaveGameHeader.sSavedGameDesc), L"%hs", g_quicksave_name);
		}
		else
		{
			if (GameDesc[0] == L'\0') GameDesc = pMessageStrings[MSG_NODESC];
			wcslcpy(SaveGameHeader.sSavedGameDesc, GameDesc, lengthof(SaveGameHeader.sSavedGameDesc));
		}

		MakeFileManDirectory(g_savegame_dir);

		//Create the name of the file
		CreateSavedGameFileNameFromNumber(ubSaveGameID, zSaveGameName);

		// create the save game file
		AutoSGPFile f(FileOpen(zSaveGameName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS));
		SaveGameFilePosition(ubSaveGameID, f, "Just Opened File");

		//
		// If there are no enemy or civilians to save, we have to check BEFORE savinf the sector info struct because
		// the NewWayOfSavingEnemyAndCivliansToTempFile will RESET the civ or enemy flag AFTER they have been saved.
		//
		NewWayOfSavingEnemyAndCivliansToTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, TRUE, TRUE );
		NewWayOfSavingEnemyAndCivliansToTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, FALSE, TRUE );







		//
		// Setup the save game header
		//

		SaveGameHeader.uiSavedGameVersion = guiSavedGameVersion;
		strcpy(SaveGameHeader.zGameVersionNumber, g_version_number);

		//The following will be used to quickly access info to display in the save/load screen
		SaveGameHeader.uiDay = GetWorldDay();
		SaveGameHeader.ubHour = (UINT8)GetWorldHour();
		SaveGameHeader.ubMin = (UINT8)guiMin;

		//copy over the initial game options
		SaveGameHeader.sInitialGameOptions = gGameOptions;

		//Get the sector value to save.
		GetBestPossibleSectorXYZValues(&SaveGameHeader.sSectorX, &SaveGameHeader.sSectorY, &SaveGameHeader.bSectorZ);

		SaveGameHeader.ubNumOfMercsOnPlayersTeam = NumberOfMercsOnPlayerTeam();
		SaveGameHeader.iCurrentBalance = LaptopSaveInfo.iCurrentBalance;


		SaveGameHeader.uiCurrentScreen = guiPreviousOptionScreen;

		SaveGameHeader.fAlternateSector = GetSectorFlagStatus( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, SF_USE_ALTERNATE_MAP );

		if( gfWorldLoaded )
		{
			SaveGameHeader.fWorldLoaded = TRUE;
			SaveGameHeader.ubLoadScreenID = GetLoadScreenID( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		}
		else
		{
			SaveGameHeader.fWorldLoaded = FALSE;
			SaveGameHeader.ubLoadScreenID = 0;
		}

		SaveGameHeader.uiRandom = Random( RAND_MAX );

		//
		// Save the Save Game header file
		//
		BYTE                     data[688];
		BYTE*                    d = data;
		SAVED_GAME_HEADER const& h = SaveGameHeader;
		INJ_U32(   d, h.uiSavedGameVersion)
		INJ_STR(   d, h.zGameVersionNumber, lengthof(h.zGameVersionNumber))
	#ifdef _WIN32 // XXX HACK000A
		INJ_WSTR16(d, h.sSavedGameDesc, lengthof(h.sSavedGameDesc))
	#else
		INJ_WSTR32(d, h.sSavedGameDesc, lengthof(h.sSavedGameDesc))
	#endif
		INJ_SKIP(  d, 4)
		INJ_U32(   d, h.uiDay)
		INJ_U8(    d, h.ubHour)
		INJ_U8(    d, h.ubMin)
		INJ_I16(   d, h.sSectorX)
		INJ_I16(   d, h.sSectorY)
		INJ_I8(    d, h.bSectorZ)
		INJ_U8(    d, h.ubNumOfMercsOnPlayersTeam)
		INJ_I32(   d, h.iCurrentBalance)
		INJ_U32(   d, h.uiCurrentScreen)
		INJ_BOOL(  d, h.fAlternateSector)
		INJ_BOOL(  d, h.fWorldLoaded)
		INJ_U8(    d, h.ubLoadScreenID)
		d = InjectGameOptions(d, h.sInitialGameOptions);
		INJ_SKIP(  d, 1)
		INJ_U32(   d, h.uiRandom)
		INJ_SKIP(  d, 112)
		Assert(d == endof(data));

		FileWrite(f, data, sizeof(data));
		SaveGameFilePosition(ubSaveGameID, f, "Save Game Header");

		guiJA2EncryptionSet = CalcJA2EncryptionSet( &SaveGameHeader );

		//
		//Save the gTactical Status array, plus the curent secotr location
		//
		SaveTacticalStatusToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "Tactical Status");

		// save the game clock info
		SaveGameClock(f, fPausedStateBeforeSaving, fLockPauseStateBeforeSaving);
		SaveGameFilePosition(ubSaveGameID, f, "Game Clock");

		// save the strategic events
		SaveStrategicEventsToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "Strategic Events");

		SaveLaptopInfoToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "Laptop Info");

		//
		// Save the merc profiles
		//
		SaveMercProfiles(f);
		SaveGameFilePosition(ubSaveGameID, f, "Merc Profiles");

		//
		// Save the soldier structure
		//
		SaveSoldierStructure(f);
		SaveGameFilePosition(ubSaveGameID, f, "Soldier Structure");

		//Save the Finaces Data file
		SaveFilesToSavedGame(FINANCES_DATA_FILE, f);
		SaveGameFilePosition(ubSaveGameID, f, "Finances Data File");

		//Save the history file
		SaveFilesToSavedGame(HISTORY_DATA_FILE, f);
		SaveGameFilePosition(ubSaveGameID, f, "History file");

		//Save the Laptop File file
		SaveFilesToSavedGame(FILES_DAT_FILE, f);
		SaveGameFilePosition(ubSaveGameID, f, "The Laptop FILES file");

		//Save email stuff to save file
		SaveEmailToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "Email");

		//Save the strategic information
		SaveStrategicInfoToSavedFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Strategic Information");

		//save the underground information
		SaveUnderGroundSectorInfoToSaveGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "Underground Information");

		//save the squad info
		SaveSquadInfoToSavedGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Squad Info");

		SaveStrategicMovementGroupsToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Strategic Movement Groups");

		//Save all the map temp files from the maps\temp directory into the saved game file
		SaveMapTempFilesToSavedGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "All the Map Temp files");

		SaveQuestInfoToSavedGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Quest Info");

		SaveOppListInfoToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "OppList info");

		SaveMapScreenMessagesToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "MapScreen Messages");

		SaveNPCInfoToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "NPC Info");

		SaveKeyTableToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "KeyTable");

		SaveTempNpcQuoteArrayToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "NPC Temp Quote File");

		SavePreRandomNumbersToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "PreGenerated Random Files");

		SaveArmsDealerInventoryToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Arms Dealers Inventory");

		SaveGeneralInfo(f);
		SaveGameFilePosition(ubSaveGameID, f, "Misc. Info");

		SaveMineStatusToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Mine Status");

		SaveStrategicTownLoyaltyToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Town Loyalty");

		SaveVehicleInformationToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Vehicle Information");

		SaveBulletStructureToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Bullet Information");

		SavePhysicsTableToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Physics Table");

		SaveAirRaidInfoToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Air Raid Info");

		SaveTeamTurnsToTheSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Team Turn Info");

		SaveExplosionTableToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Explosion Table");

		SaveCreatureDirectives(f);
		SaveGameFilePosition(ubSaveGameID, f, "Creature Spreading");

		SaveStrategicStatusToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Strategic Status");

		SaveStrategicAI(f);
		SaveGameFilePosition(ubSaveGameID, f, "Strategic AI");

		SaveWatchedLocsToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "Watched Locs Info");

		SaveItemCursorToSavedGame(f);
		SaveGameFilePosition(ubSaveGameID, f, "ItemCursor Info");

		SaveCivQuotesToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Civ Quote System");

		SaveBackupNPCInfoToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Backed up NPC Info");

		SaveMeanwhileDefsFromSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Meanwhile Definitions");

		// save meanwhiledefs

		SaveSchedules(f);
		SaveGameFilePosition(ubSaveGameID, f, "Schedules");

		// Save extra vehicle info
		NewSaveVehicleMovementInfoToSavedGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Vehicle Movement Stuff");

		// Save contract renewal sequence stuff
		SaveContractRenewalDataToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "Contract Renewal Data");

		// Save leave list stuff
		SaveLeaveItemList(f);
		SaveGameFilePosition(ubSaveGameID, f, "leave list");

		//do the new way of saving bobbyr mail order items
		NewWayOfSavingBobbyRMailOrdersToSaveGameFile(f);
		SaveGameFilePosition(ubSaveGameID, f, "New way of saving Bobby R mailorders");
	}
	catch (...)
	{
		if (fWePausedIt) UnPauseAfterSaveGame();

		//Delete the failed attempt at saving
		DeleteSaveGameNumber(ubSaveGameID);

		//Put out an error message
		ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, zSaveLoadText[SLG_SAVE_GAME_ERROR]);

		InitShutDownMapTempFileTest(FALSE, "SaveMapTempFile", ubSaveGameID);

		//Check for enough free hard drive space
		NextLoopCheckForEnoughFreeHardDriveSpace();

#ifdef JA2BETAVERSION
		if (fDisableDueToBattleRoster || fDisableMapInterfaceDueToBattle)
		{
			gubReportMapscreenLock = 2;
		}
#endif
		gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;
		return FALSE;
	}

	//if we succesfully saved the game, mark this entry as the last saved game file
	if( ubSaveGameID != SAVE__ERROR_NUM && ubSaveGameID != SAVE__END_TURN_NUM )
	{
		gGameSettings.bLastSavedGameSlot = ubSaveGameID;
	}

	//Save the save game settings
	SaveGameSettings();

	// Display a screen message that the save was succesful
	if (ubSaveGameID != SAVE__END_TURN_NUM)
	{
		ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[MSG_SAVESUCCESS]);
	}
//#ifdef JA2BETAVERSION
	else
	{
//		ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[ MSG_END_TURN_AUTO_SAVE ] );
	}
//#endif

	//restore the music mode
	SetMusicMode( gubMusicMode );

	//Unset the fact that we are saving a game
	gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

	UnPauseAfterSaveGame();

	InitShutDownMapTempFileTest(FALSE, "SaveMapTempFile", ubSaveGameID);

	#ifdef JA2BETAVERSION
		ValidateSoldierInitLinks( 2 );
	#endif

	//Check for enough free hard drive space
	NextLoopCheckForEnoughFreeHardDriveSpace();

	return( TRUE );
}


void ExtractSavedGameHeaderFromFile(HWFILE const f, SAVED_GAME_HEADER& h)
{
	BYTE data[688];
	FileRead(f, data, sizeof(data));

	BYTE const* d = data;
	EXTR_U32(   d, h.uiSavedGameVersion)
	EXTR_STR(   d, h.zGameVersionNumber, lengthof(h.zGameVersionNumber))
#ifdef _WIN32 // XXX HACK000A
	EXTR_WSTR16(d, h.sSavedGameDesc, lengthof(h.sSavedGameDesc))
#else
	EXTR_WSTR32(d, h.sSavedGameDesc, lengthof(h.sSavedGameDesc))
#endif
	EXTR_SKIP(  d, 4)
	EXTR_U32(   d, h.uiDay)
	EXTR_U8(    d, h.ubHour)
	EXTR_U8(    d, h.ubMin)
	EXTR_I16(   d, h.sSectorX)
	EXTR_I16(   d, h.sSectorY)
	EXTR_I8(    d, h.bSectorZ)
	EXTR_U8(    d, h.ubNumOfMercsOnPlayersTeam)
	EXTR_I32(   d, h.iCurrentBalance)
	EXTR_U32(   d, h.uiCurrentScreen)
	EXTR_BOOL(  d, h.fAlternateSector)
	EXTR_BOOL(  d, h.fWorldLoaded)
	EXTR_U8(    d, h.ubLoadScreenID)
	d = ExtractGameOptions(d, h.sInitialGameOptions);
	EXTR_SKIP(  d, 1)
	EXTR_U32(   d, h.uiRandom)
	EXTR_SKIP(  d, 112)
	Assert(d == endof(data));
}


static void HandleOldBobbyRMailOrders(void);
static void LoadGeneralInfo(HWFILE, UINT32 savegame_version);
static void LoadMeanwhileDefsFromSaveGameFile(HWFILE, UINT32 savegame_version);
static void LoadOppListInfoFromSavedGame(HWFILE);
static void LoadPreRandomNumbersFromSaveGameFile(HWFILE);
static void LoadSavedMercProfiles(HWFILE, UINT32 savegame_version);
static void LoadSoldierStructure(HWFILE, UINT32 savegame_version);
static void LoadTacticalStatusFromSavedGame(HWFILE);
static void LoadWatchedLocsFromSavedGame(HWFILE);
static void TruncateStrategicGroupSizes(void);
static void UpdateMercMercContractInfo(void);

#ifdef JA2BETAVERSION
static void InitLoadGameFilePosition(UINT8 slot);
static void LoadGameFilePosition(UINT8 slot, HWFILE load, const char* pMsg);
#else
#	define InitLoadGameFilePosition(slot)        ((void)0)
#	define LoadGameFilePosition(slot, file, msg) ((void)0)
#endif


void LoadSavedGame(UINT8 const save_slot_id)
{
	TrashAllSoldiers();
	RemoveAllGroups();

	// Empty the dialogue Queue cause someone could still have a quote in waiting
	EmptyDialogueQueue();

	// If there is someone talking, stop them
	StopAnyCurrentlyTalkingSpeech();

	ZeroAnimSurfaceCounts();

	ShutdownNPCQuotes();

	InitShutDownMapTempFileTest(TRUE, "LoadMapTempFile", save_slot_id);

	//Used in mapescreen to disable the annoying 'swoosh' transitions
	gfDontStartTransitionFromLaptop = TRUE;

	// Reset timer callbacks
	gpCustomizableTimerCallback = NULL;

	InitLoadGameFilePosition(save_slot_id);

	//Set the fact that we are loading a saved game
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

	/* Trash the existing world.  This is done to ensure that if we are loading a
	 * game that doesn't have a world loaded, that we trash it beforehand -- else
	 * the player could theoretically enter that sector where it would be in a
	 * pre-load state. */
	TrashWorld();

	// Deletes all the Temp files in the Maps/Temp directory
	InitTacticalSave(TRUE);

	// ATE: Added to empty dialogue q
	EmptyDialogueQueue();

	char zSaveGameName[512];
	CreateSavedGameFileNameFromNumber(save_slot_id, zSaveGameName);
	AutoSGPFile f(FileOpen(zSaveGameName, FILE_ACCESS_READ));
	LoadGameFilePosition(save_slot_id, f, "Just Opened File");

	SAVED_GAME_HEADER SaveGameHeader;
	ExtractSavedGameHeaderFromFile(f, SaveGameHeader);
	LoadGameFilePosition(save_slot_id, f, "Save Game Header");

	guiJA2EncryptionSet = CalcJA2EncryptionSet(&SaveGameHeader);

	UINT32 const version = SaveGameHeader.uiSavedGameVersion;

	/* If the player is loading up an older version of the game and the person
	 * DOESN'T have the cheats on. */
	if (version < 65 && !CHEATER_CHEAT_LEVEL()) throw std::runtime_error("Savegame too old");

	//Store the loading screenID that was saved
	gubLastLoadingScreenID = static_cast<LoadingScreenID>(SaveGameHeader.ubLoadScreenID);

#if 0 // XXX was commented out
	LoadGeneralInfo(f, version);
	LoadGameFilePosition(save_slot_id, f, "Misc info");
#endif

	//Load the gtactical status structure plus the current sector x,y,z
	LoadTacticalStatusFromSavedGame(f);
	LoadGameFilePosition(save_slot_id, f, "Tactical Status");

	//This gets reset by the above function
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;


	//Load the game clock ingo
	LoadGameClock(f);
	LoadGameFilePosition(save_slot_id, f, "Game Clock");

	//if we are suppose to use the alternate sector
	if (SaveGameHeader.fAlternateSector)
	{
		SetSectorFlag(gWorldSectorX, gWorldSectorY, gbWorldSectorZ, SF_USE_ALTERNATE_MAP);
		gfUseAlternateMap = TRUE;
	}

	//if the world was loaded when saved, reload it, otherwise dont
	if (SaveGameHeader.fWorldLoaded || version < 50)
	{
		//Get the current world sector coordinates
		INT16 const sLoadSectorX = gWorldSectorX;
		INT16 const sLoadSectorY = gWorldSectorY;
		INT8  const bLoadSectorZ = gbWorldSectorZ;

		// This will guarantee that the sector will be loaded
		gbWorldSectorZ = -1;

		//if we should load a sector (if the person didnt just start the game game)
		if (gWorldSectorX != 0 && gWorldSectorY != 0)
		{
			//Load the sector
			SetCurrentWorldSector(sLoadSectorX, sLoadSectorY, bLoadSectorZ);
		}
	}
	else
	{ //By clearing these values, we can avoid "in sector" checks -- at least, that's the theory.
		gWorldSectorX = 0;
		gWorldSectorY = 0;

		INT16 const x = SaveGameHeader.sSectorX;
		INT16 const y = SaveGameHeader.sSectorY;
		INT8  const z = SaveGameHeader.bSectorZ;
		gubLastLoadingScreenID = x == -1 || y == -1 || z == -1 ?
			LOADINGSCREEN_HELI : GetLoadScreenID(x, y, z);

		BeginLoadScreen();
	}

	CreateLoadingScreenProgressBar();
	SetProgressBarColor(0, 0, 0, 150);

#ifdef JA2BETAVERSION
	SetProgressBarMsgAttributes(0, FONT12ARIAL, FONT_MCOLOR_WHITE, 0);

	//
	// Set the tile so we don see the text come up
	//

	//if the world is unloaded, we must use the save buffer for the text
	SetProgressBarTextDisplayFlag(0, TRUE, TRUE, !SaveGameHeader.fWorldLoaded);
#endif

	UINT32 uiRelStartPerc = 0;
	UINT32 uiRelEndPerc   = 0;

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Events...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	//load the game events
	LoadStrategicEventsFromSavedGame(f);
	LoadGameFilePosition(save_slot_id, f, "Strategic Events");

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Laptop Info");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;


	LoadLaptopInfoFromSavedGame(f);
	LoadGameFilePosition(save_slot_id, f, "Laptop Info");

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Merc Profiles...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load all the saved Merc profiles
	LoadSavedMercProfiles(f, version);
	LoadGameFilePosition(save_slot_id, f, "Merc Profiles");

	uiRelEndPerc += 30;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Soldier Structure...");
	uiRelStartPerc = uiRelEndPerc;

	// Load the soldier structure info
	LoadSoldierStructure(f, version);
	LoadGameFilePosition(save_slot_id, f, "Soldier Structure");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Finances Data File...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the Finances Data and write it to a new file
	LoadFilesFromSavedGame(FINANCES_DATA_FILE, f);
	LoadGameFilePosition(save_slot_id, f, "Finances Data File");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"History File...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the History Data and write it to a new file
	LoadFilesFromSavedGame(HISTORY_DATA_FILE, f);
	LoadGameFilePosition(save_slot_id, f, "History File");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"The Laptop FILES file...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the Files Data and write it to a new file
	LoadFilesFromSavedGame(FILES_DAT_FILE, f);
	LoadGameFilePosition(save_slot_id, f, "The Laptop FILES file");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Email...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the data for the emails
	LoadEmailFromSavedGame(f);
	LoadGameFilePosition(save_slot_id, f, "Email");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Information...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the strategic Information
	LoadStrategicInfoFromSavedFile(f);
	LoadGameFilePosition(save_slot_id, f, "Strategic Information");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"UnderGround Information...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the underground information
	LoadUnderGroundSectorInfoFromSavedGame(f);
	LoadGameFilePosition(save_slot_id, f, "UnderGround Information");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Squad Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load all the squad info from the saved game file
	LoadSquadInfoFromSavedGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "Squad Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Movement Groups...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Load the group linked list
	LoadStrategicMovementGroupsFromSavedGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "Strategic Movement Groups");

	uiRelEndPerc += 30;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"All the Map Temp files...");
	uiRelStartPerc = uiRelEndPerc;

	// Load all the map temp files from the saved game file into the maps/temp directory
	LoadMapTempFilesFromSavedGameFile(f, version);
	LoadGameFilePosition(save_slot_id, f, "All the Map Temp files");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Quest Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadQuestInfoFromSavedGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "Quest Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"OppList Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadOppListInfoFromSavedGame(f);
	LoadGameFilePosition(save_slot_id, f, "OppList Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"MapScreen Messages...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadMapScreenMessagesFromSaveGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "MapScreen Messages");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"NPC Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadNPCInfoFromSavedGameFile(f, version);
	LoadGameFilePosition(save_slot_id, f, "NPC Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"KeyTable...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadKeyTableFromSaveedGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "KeyTable");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Npc Temp Quote File...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadTempNpcQuoteArrayToSaveGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "Npc Temp Quote File");

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"PreGenerated Random Files...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadPreRandomNumbersFromSaveGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "PreGenerated Random Files");

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Smoke Effect Structures...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// No longer need to load smoke effects.  They are now in temp files
	if (version < 75) LoadSmokeEffectsFromLoadGameFile(f, version);
	LoadGameFilePosition(save_slot_id, f, "Smoke Effect Structures");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Arms Dealers Inventory...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadArmsDealerInventoryFromSavedGameFile(f, version);
	LoadGameFilePosition(save_slot_id, f, "Arms Dealers Inventory");

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Misc info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadGeneralInfo(f, version);
	LoadGameFilePosition(save_slot_id, f, "Misc info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Mine Status...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	LoadMineStatusFromSavedGameFile(f);
	LoadGameFilePosition(save_slot_id, f, "Mine Status");

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Town Loyalty...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 21)
	{
		LoadStrategicTownLoyaltyFromSavedGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Town Loyalty");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Vehicle Information...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 22)
	{
		LoadVehicleInformationFromSavedGameFile(f, version);
		LoadGameFilePosition(save_slot_id, f, "Vehicle Information");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Bullet Information...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 24)
	{
		LoadBulletStructureFromSavedGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Bullet Information");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Physics table...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 24)
	{
		LoadPhysicsTableFromSavedGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Physics table");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Air Raid Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 24)
	{
		LoadAirRaidInfoFromSaveGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Air Raid Info");
	}

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Team Turn Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 24)
	{
		LoadTeamTurnsFromTheSavedGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Team Turn Info");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Explosion Table...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 25)
	{
		LoadExplosionTableFromSavedGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Explosion Table");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Creature Spreading...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 27)
	{
		LoadCreatureDirectives(f, version);
		LoadGameFilePosition(save_slot_id, f, "Creature Spreading");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic Status...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;


	if (version	>= 28)
	{
		LoadStrategicStatusFromSaveGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Strategic Status");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Strategic AI...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 31)
	{
		LoadStrategicAI(f);
		LoadGameFilePosition(save_slot_id, f, "Strategic AI");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Lighting Effects...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// No longer need to load Light effects.  They are now in temp files
	if (37 <= version && version < 76)
	{
		LoadLightEffectsFromLoadGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Lighting Effects");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Watched Locs Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 38)
	{
		LoadWatchedLocsFromSavedGame(f);
	}
	LoadGameFilePosition(save_slot_id, f, "Watched Locs Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Item cursor Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version	>= 39)
	{
		LoadItemCursorFromSavedGame(f);
	}
	LoadGameFilePosition(save_slot_id, f, "Item cursor Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Civ Quote System...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version >= 51)
	{
		LoadCivQuotesFromLoadGameFile(f);
	}
	LoadGameFilePosition(save_slot_id, f, "Civ Quote System");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Backed up NPC Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version >= 53)
	{
		LoadBackupNPCInfoFromSavedGameFile(f);
	}
	LoadGameFilePosition(save_slot_id, f, "Backed up NPC Info");

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Meanwhile definitions...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version >= 58)
	{
		LoadMeanwhileDefsFromSaveGameFile(f, version);
		LoadGameFilePosition(save_slot_id, f, "Meanwhile definitions");
	}
	else
	{
		gMeanwhileDef[gCurrentMeanwhileDef.ubMeanwhileID] = gCurrentMeanwhileDef;
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Schedules...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version >= 59)
	{
		// trash schedules loaded from map
		DestroyAllSchedulesWithoutDestroyingEvents();
		LoadSchedulesFromSave(f);
		LoadGameFilePosition(save_slot_id, f, "Schedules");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Extra Vehicle Info...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version >= 61)
	{
		if (version < 84)
		{
			LoadVehicleMovementInfoFromSavedGameFile(f);
		}
		else
		{
			NewLoadVehicleMovementInfoFromSavedGameFile(f);
		}
		LoadGameFilePosition(save_slot_id, f, "Extra Vehicle Info");
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Contract renweal sequence stuff...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	if (version < 62)
	{ // The older games had a bug where this flag was never being set
		gfResetAllPlayerKnowsEnemiesFlags = TRUE;
	}

	if (version >= 67)
	{
		LoadContractRenewalDataFromSaveGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "Contract renweal sequence stuff");
	}

	if (version >= 70)
	{
		LoadLeaveItemList(f);
		LoadGameFilePosition(save_slot_id, f, "Leave List");
	}

	if (version <= 73)
	{ // Patch vehicle fuel
		AddVehicleFuelToSave();
	}

	if (version >= 85)
	{
		NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(f);
		LoadGameFilePosition(save_slot_id, f, "New way of loading Bobby R mailorders");
	}

	// If there are any old Bobby R Mail orders, tranfer them to the new system
	if (version < 85)
	{
		HandleOldBobbyRMailOrders();
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// ATE: Patch? Patch up groups (will only do for old saves)
	UpdatePersistantGroupsFromOldSave(version);

	if (version	<= 40)
	{ /* Cancel all pending purchase orders for BobbyRay's.  Starting with version
		 * 41, the BR orders events are posted with the usItemIndex itself as the
		 * parameter, rather than the inventory slot index.  This was done to make
		 * it easier to modify BR's traded inventory lists later on without
		 * breaking saves. */
		CancelAllPendingBRPurchaseOrders();
	}

	// if the world is loaded, apply the temp files to the loaded map
	if (SaveGameHeader.fWorldLoaded || version < 50)
	{
		try
		{ // Load the current sectors Information From the temporary files
			LoadCurrentSectorsInformationFromTempItemsFile();
		}
		catch (...)
		{
			InitExitGameDialogBecauseFileHackDetected();
			return;
		}
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	InitAI();

	// Update the mercs in the sector with the new soldier info
	UpdateMercsInSector(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);

	PostSchedules();

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Reset the lighting level if we are outside
	if (gbWorldSectorZ == 0)
		LightSetBaseLevel(GetTimeOfDayAmbientLightLevel());

	//if we have been to this sector before
//	if (SectorInfo[SECTOR(gWorldSectorX, gWorldSectorY)].uiFlags & SF_ALREADY_VISITED)
	{
		//Reset the fact that we are loading a saved game
		gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;
	}

	/* CJC January 13: we can't do this because (a) it resets militia IN THE
	 * MIDDLE OF COMBAT, and (b) if we add militia to the teams while
	 * LOADING_SAVED_GAME is set, the team counters will not be updated properly!
	 */
//	ResetMilitia();

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// if the UI was locked in the saved game file
	if (gTacticalStatus.ubAttackBusyCount > 1)
	{ // Lock the ui
		SetUIBusy(GetSelectedMan());
	}

	// if we succesfully LOADED! the game, mark this entry as the last saved game file
	gGameSettings.bLastSavedGameSlot = save_slot_id;

	//Save the save game settings
	SaveGameSettings();

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	// Reset the AI Timer clock
	giRTAILastUpdateTime = 0;

	// If we are in tactical
	if (guiScreenToGotoAfterLoadingSavedGame == GAME_SCREEN)
	{ //Initialize the current panel
		InitializeCurrentPanel();
		SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel) SelectSoldier(sel, SELSOLDIER_FORCE_RESELECT);
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Final Checks...");
	RenderProgressBar(0, 100);
	uiRelStartPerc = uiRelEndPerc;

	InitalizeStaticExternalNPCFaces();
	LoadCarPortraitValues();

	// OK, turn OFF show all enemies....
	gTacticalStatus.uiFlags &= ~(SHOW_ALL_MERCS | SHOW_ALL_ITEMS);

	InitShutDownMapTempFileTest(FALSE, "LoadMapTempFile", save_slot_id);

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Setting attack busy count to 0 from load");
		gTacticalStatus.ubAttackBusyCount = 0;
	}

	if (version	< 64)
	{ /* Militia/enemies/creature team sizes have been changed from 32 to 20.
		 * This function will simply kill off the excess.  This will allow the
		 * ability to load previous saves, though there will still be problems,
		 * though a LOT less than there would be without this call. */
		TruncateStrategicGroupSizes();
	}

	// ATE: if we are within this window where skyridder was foobared, fix!
	if (61 <= version && version <= 65 && !fSkyRiderSetUp)
	{
		// see if we can find him and remove him if so....
		SOLDIERTYPE* const s = FindSoldierByProfileID(SKYRIDER);
		if (s) TacticalRemoveSoldier(s);

		// add the pilot at a random location!
		INT16 x;
		INT16 y;
		switch (Random(4))
		{
			case 0: x = 15; y = MAP_ROW_B; break;
			case 1: x = 14; y = MAP_ROW_E; break;
			case 2: x = 12; y = MAP_ROW_D; break;
			case 3: x = 16; y = MAP_ROW_C; break;
			default: abort(); // HACK000E
		}
		MERCPROFILESTRUCT& p = GetProfile(SKYRIDER);
		p.sSectorX = x;
		p.sSectorY = y;
		p.bSectorZ = 0;
	}

	if (version < 68)
	{
		// correct bVehicleUnderRepairID for all mercs
		FOR_ALL_SOLDIERS(s) s->bVehicleUnderRepairID = -1;
	}

	if (version < 73 && LaptopSaveInfo.fMercSiteHasGoneDownYet)
	{
		LaptopSaveInfo.fFirstVisitSinceServerWentDown = 2;
	}

	/* Update the MERC merc contract length.  Before save version 77 the data was
	 * stored in the SOLDIERTYPE, after 77 the data is stored in the profile */
	if (version < 77)
	{
		UpdateMercMercContractInfo();
	}

	if (version <= 89)
	{ /* ARM: A change was made in version 89 where refuel site availability now
		 * also depends on whether the player has airspace control over that sector.
		 * To update the settings immediately, must call it here. */
		UpdateRefuelSiteAvailability();
	}

	if (version < 91)
	{ // Update the amount of money that has been paid to speck
		CalcAproximateAmountPaidToSpeck();
	}

	gfLoadedGame = TRUE;

	uiRelEndPerc = 100;
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, L"Done!");
	RenderProgressBar(0, 100);

	RemoveLoadingScreenProgressBar();

	SetMusicMode(gMusicModeToPlay);

#ifndef JA2TESTVERSION
	RESET_CHEAT_LEVEL();
#endif

#ifdef JA2BETAVERSION
	if (fDisableDueToBattleRoster || fDisableMapInterfaceDueToBattle)
	{
		gubReportMapscreenLock = 1;
	}
#endif

	// reset once-per-convo records for everyone in the loaded sector
	ResetOncePerConvoRecordsForAllNPCsInLoadedSector();

	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{ /* Fix lingering attack busy count problem on loading saved game by
		 * resetting a.b.c if we're not in combat. */
		gTacticalStatus.ubAttackBusyCount = 0;
	}

	// fix squads
	CheckSquadMovementGroups();

	/* The above function LightSetBaseLevel adjusts ALL the level node light
	 * values including the merc node, we must reset the values */
	HandlePlayerTogglingLightEffects(FALSE);
}


static void SaveMercProfiles(HWFILE const f)
{
	// Loop through all the profiles to save
	void (&writer)(HWFILE, BYTE const*, UINT32) = guiSavedGameVersion < 87 ?
		JA2EncryptedFileWrite : NewJA2EncryptedFileWrite;
	for (MERCPROFILESTRUCT* i = gMercProfiles; i != endof(gMercProfiles); ++i)
	{
#ifdef _WIN32 // XXX HACK000A
		BYTE data[716];
#else
		BYTE data[796];
#endif
		InjectMercProfile(data, *i);
		writer(f, data, sizeof(data));
	}
}


static void LoadSavedMercProfiles(HWFILE const f, UINT32 const savegame_version)
{
	// Loop through all the profiles to load
	void (&reader)(HWFILE, BYTE*, UINT32) = savegame_version < 87 ?
		JA2EncryptedFileRead : NewJA2EncryptedFileRead;
	for (MERCPROFILESTRUCT* i = gMercProfiles; i != endof(gMercProfiles); ++i)
	{
#ifdef _WIN32 // XXX HACK000A
		BYTE data[716];
#else
		BYTE data[796];
#endif
		reader(f, data, sizeof(data));
		ExtractMercProfile(data, *i);
	}
}


static void SaveSoldierStructure(HWFILE const f)
{
	// Loop through all the soldier structs to save
	void (&writer)(HWFILE, BYTE const*, UINT32) = guiSavedGameVersion < 87 ?
		JA2EncryptedFileWrite : NewJA2EncryptedFileWrite;
	for (UINT16 i = 0; i < TOTAL_SOLDIERS; ++i)
	{
		SOLDIERTYPE const* const s = GetMan(i);

		// If the soldier isn't active, don't add them to the saved game file.
		FileWrite(f, &s->bActive, 1);
		if (!s->bActive) continue;

		// Save the soldier structure
#ifdef _WIN32 // XXX HACK000A
		BYTE data[2328];
#else
		BYTE data[2352];
#endif
		InjectSoldierType(data, s);
		writer(f, data, sizeof(data));

		// Save all the pointer info from the structure
		SaveMercPath(f, s->pMercPath);

		// Save the key ring
		UINT8 const has_keyring = s->pKeyRing != 0;
		FileWrite(f, &has_keyring, sizeof(has_keyring));
		if (!has_keyring) continue;
		FileWrite(f, s->pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING));
	}
}


static void LoadSoldierStructure(HWFILE const f, UINT32 savegame_version)
{
	// Loop through all the soldier and delete them all
	FOR_ALL_SOLDIERS(s) TacticalRemoveSoldier(s);

	// Loop through all the soldier structs to load
	void (&reader)(HWFILE, BYTE*, UINT32) = savegame_version < 87 ?
		JA2EncryptedFileRead : NewJA2EncryptedFileRead;
	for (UINT16 i = 0; i < TOTAL_SOLDIERS; ++i)
	{
		// Update the progress bar
		UINT32 const percentage = (i * 100) / (TOTAL_SOLDIERS - 1);
		RenderProgressBar(0, percentage);

		// Read in a byte to tell us whether or not there is a soldier loaded here.
		UINT8 active;
		FileRead(f, &active, 1);
		if (!active) continue;

#ifdef _WIN32 // XXX HACK000A
		BYTE Data[2328];
#else
		BYTE Data[2352];
#endif
		//Read in the saved soldier info into a Temp structure
		reader(f, Data, sizeof(Data));
		SOLDIERTYPE SavedSoldierInfo;
		ExtractSoldierType(Data, &SavedSoldierInfo);

		SOLDIERTYPE* const s = TacticalCreateSoldierFromExisting(&SavedSoldierInfo);
		Assert(s->ubID == i);

		LoadMercPath(f, &s->pMercPath);

		// Read the file to see if we have to load the keys
		UINT8 has_keyring;
		FileRead(f, &has_keyring, 1);
		if (has_keyring)
		{
			// Now Load the ....
			FileRead(f, s->pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING));
		}
		else
		{
			Assert(s->pKeyRing == NULL);
		}

		//if the soldier is an IMP character
		if (s->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER && s->bTeam == gbPlayerNum)
		{
			ResetIMPCharactersEyesAndMouthOffsets(s->ubProfile);
		}

		// If the saved game version is before x, calculate the amount of money paid to mercs
		if (savegame_version < 83 && s->ubProfile != NO_PROFILE)
		{
			MERCPROFILESTRUCT& p = GetProfile(s->ubProfile);
			if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			{
				p.uiTotalCostToDate = p.sSalary * p.iMercMercContractLength;
			}
			else
			{
				p.uiTotalCostToDate = p.sSalary * s->iTotalContractLength;
			}
		}

#ifdef GERMAN
		// Fix neutral flags
		if (savegame_version < 94 &&
				s->bTeam == OUR_TEAM  &&
				s->bNeutral           &&
				s->bAssignment != ASSIGNMENT_POW)
		{
			// turn off neutral flag
			s->bNeutral = FALSE;
		}
#endif
		// JA2Gold: fix next-to-previous attacker value
		if (savegame_version < 99)
		{
			s->next_to_previous_attacker = NULL;
		}
	}

	// Fix robot
	if (savegame_version <= 87)
	{
		MERCPROFILESTRUCT& robot_p = GetProfile(ROBOT);
		if (robot_p.inv[VESTPOS] == SPECTRA_VEST)
		{
			// update this
			robot_p.inv[VESTPOS]   = SPECTRA_VEST_18;
			robot_p.inv[HELMETPOS] = SPECTRA_HELMET_18;
			robot_p.inv[LEGPOS]    = SPECTRA_LEGGINGS_18;
			robot_p.bAgility = 50;
			SOLDIERTYPE* const robot_s = FindSoldierByProfileID(ROBOT);
			if (robot_s)
			{
				robot_s->inv[VESTPOS].usItem   = SPECTRA_VEST_18;
				robot_s->inv[HELMETPOS].usItem = SPECTRA_HELMET_18;
				robot_s->inv[LEGPOS].usItem    = SPECTRA_LEGGINGS_18;
				robot_s->bAgility = 50;
			}
		}
	}
}


#ifdef JA2BETAVERSION
static void WriteTempFileNameToFile(const char* pFileName, UINT32 uiSizeOfFile, HWFILE hSaveFile);
#endif


void SaveFilesToSavedGame(char const* const pSrcFileName, HWFILE const hFile)
{
	AutoSGPFile hSrcFile(FileOpen(pSrcFileName, FILE_ACCESS_READ));

#ifdef JA2BETAVERSION
	guiNumberOfMapTempFiles++;		//Increment counter:  To determine where the temp files are crashing
#endif

	//Get the file size of the source data file
	UINT32 uiFileSize = FileGetSize( hSrcFile );

	// Write the the size of the file to the saved game file
	FileWrite(hFile, &uiFileSize, sizeof(UINT32));

	if (uiFileSize == 0) return;

	// Read the saource file into the buffer
	SGP::Buffer<UINT8> pData(uiFileSize);
	FileRead(hSrcFile, pData, uiFileSize);

	// Write the buffer to the saved game file
	FileWrite(hFile, pData, uiFileSize);

#ifdef JA2BETAVERSION
	//Write out the name of the temp file so we can track whcih ones get loaded, and saved
	WriteTempFileNameToFile(pSrcFileName, uiFileSize, hFile);
#endif
}


void LoadFilesFromSavedGame(char const* const pSrcFileName, HWFILE const hFile)
{
#ifdef JA2BETAVERSION
	++guiNumberOfMapTempFiles; //Increment counter:  To determine where the temp files are crashing
#endif

	AutoSGPFile hSrcFile(FileOpen(pSrcFileName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS));

	// Read the size of the data
	UINT32 uiFileSize;
	FileRead(hFile, &uiFileSize, sizeof(UINT32));

	if (uiFileSize == 0) return;

	// Read into the buffer
	SGP::Buffer<UINT8> pData(uiFileSize);
	FileRead(hFile, pData, uiFileSize);

	// Write the buffer to the new file
	FileWrite(hSrcFile, pData, uiFileSize);

#ifdef JA2BETAVERSION
	WriteTempFileNameToFile(pSrcFileName, uiFileSize, hFile);
#endif
}


static void SaveTacticalStatusToSavedGame(HWFILE const f)
{
	InjectTacticalStatusTypeIntoFile(f);

	// Save the current sector location
	BYTE  data[5];
	BYTE* d = data;
	INJ_I16(d, gWorldSectorX)
	INJ_I16(d, gWorldSectorY)
	INJ_I8( d, gbWorldSectorZ)
	Assert(d == endof(data));

	FileWrite(f, data, sizeof(data));
}


static void LoadTacticalStatusFromSavedGame(HWFILE const f)
{
	ExtractTacticalStatusTypeFromFile(f);

	// Load the current sector location
	BYTE data[5];
	FileRead(f, data, sizeof(data));

	BYTE const* d = data;
	EXTR_I16(d, gWorldSectorX)
	EXTR_I16(d, gWorldSectorY)
	EXTR_I8( d, gbWorldSectorZ)
	Assert(d == endof(data));
}


static void SaveOppListInfoToSavedGame(HWFILE const hFile)
{
	UINT32	uiSaveSize=0;

	// Save the Public Opplist
	uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
	FileWrite(hFile, gbPublicOpplist, uiSaveSize);

	// Save the Seen Oppenents
	uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	FileWrite(hFile, gbSeenOpponents, uiSaveSize);

	// Save the Last Known Opp Locations
	uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS; // XXX TODO000F
	FileWrite(hFile, gsLastKnownOppLoc, uiSaveSize);

	// Save the Last Known Opp Level
	uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	FileWrite(hFile, gbLastKnownOppLevel, uiSaveSize);

	// Save the Public Last Known Opp Locations
	uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS; // XXX TODO000F
	FileWrite(hFile, gsPublicLastKnownOppLoc, uiSaveSize);

	// Save the Public Last Known Opp Level
	uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
	FileWrite(hFile, gbPublicLastKnownOppLevel, uiSaveSize);

	// Save the Public Noise Volume
	uiSaveSize = MAXTEAMS;
	FileWrite(hFile, gubPublicNoiseVolume, uiSaveSize);

	// Save the Public Last Noise Gridno
	uiSaveSize = MAXTEAMS; // XXX TODO000F
	FileWrite(hFile, gsPublicNoiseGridno, uiSaveSize);
}


static void LoadOppListInfoFromSavedGame(HWFILE const hFile)
{
	UINT32	uiLoadSize=0;

	// Load the Public Opplist
	uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
	FileRead(hFile, gbPublicOpplist, uiLoadSize);

	// Load the Seen Oppenents
	uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	FileRead(hFile, gbSeenOpponents, uiLoadSize);

	// Load the Last Known Opp Locations
	uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS; // XXX TODO000F
	FileRead(hFile, gsLastKnownOppLoc, uiLoadSize);

	// Load the Last Known Opp Level
	uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	FileRead(hFile, gbLastKnownOppLevel, uiLoadSize);

	// Load the Public Last Known Opp Locations
	uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS; // XXX TODO000F
	FileRead(hFile, gsPublicLastKnownOppLoc, uiLoadSize);

	// Load the Public Last Known Opp Level
	uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
	FileRead(hFile, gbPublicLastKnownOppLevel, uiLoadSize);

	// Load the Public Noise Volume
	uiLoadSize = MAXTEAMS;
	FileRead(hFile, gubPublicNoiseVolume, uiLoadSize);

	// Load the Public Last Noise Gridno
	uiLoadSize = MAXTEAMS; // XXX TODO000F
	FileRead(hFile, gsPublicNoiseGridno, uiLoadSize);
}


static void SaveWatchedLocsToSavedGame(HWFILE const hFile)
{
	UINT32	uiArraySize;
	UINT32	uiSaveSize=0;

	uiArraySize = TOTAL_SOLDIERS * NUM_WATCHED_LOCS;

	// save locations of watched points
	uiSaveSize = uiArraySize * sizeof( INT16 );
	FileWrite(hFile, gsWatchedLoc, uiSaveSize);

	uiSaveSize = uiArraySize * sizeof( INT8 );

	FileWrite(hFile, gbWatchedLocLevel, uiSaveSize);

	FileWrite(hFile, gubWatchedLocPoints, uiSaveSize);

	FileWrite(hFile, gfWatchedLocReset, uiSaveSize);
}


static void LoadWatchedLocsFromSavedGame(HWFILE const hFile)
{
	UINT32	uiArraySize;
	UINT32	uiLoadSize=0;

	uiArraySize = TOTAL_SOLDIERS * NUM_WATCHED_LOCS;

	uiLoadSize = uiArraySize * sizeof( INT16 );
	FileRead(hFile, gsWatchedLoc, uiLoadSize);

	uiLoadSize = uiArraySize * sizeof( INT8 );
	FileRead(hFile, gbWatchedLocLevel, uiLoadSize);

	FileRead(hFile, gubWatchedLocPoints, uiLoadSize);

	FileRead(hFile, gfWatchedLocReset, uiLoadSize);
}


void CreateSavedGameFileNameFromNumber(const UINT8 ubSaveGameID, char* const pzNewFileName)
{
	char const* const dir = g_savegame_dir;
	char const* const ext = g_savegame_ext;

	switch (ubSaveGameID)
	{
		case 0: // we are creating the QuickSave file
		{
			char const* const quick = g_quicksave_name;
#ifdef JA2BETAVERSION
			if (gfUseConsecutiveQuickSaveSlots &&
					guiCurrentQuickSaveNumber != 0)
			{
				sprintf(pzNewFileName, "%s/%s%02d.%s", dir, quick, guiCurrentQuickSaveNumber, ext);
			}
			else
#endif
			{
				sprintf(pzNewFileName, "%s/%s.%s", dir, quick, ext);
			}
			break;
		}

		case SAVE__END_TURN_NUM:
			sprintf(pzNewFileName, "%s/Auto%02d.%s", dir, guiLastSaveGameNum, ext);
			guiLastSaveGameNum = (guiLastSaveGameNum + 1) % 2;
			break;

		case SAVE__ERROR_NUM:
			sprintf(pzNewFileName, "%s/error.%s", dir, ext);
			break;

		default:
			sprintf(pzNewFileName, "%s/%s%02d.%s", dir, g_savegame_name, ubSaveGameID, ext);
			break;
	}
}


void SaveMercPath(HWFILE const f, PathSt const* const head)
{
	UINT32 n_nodes = 0;
	for (const PathSt* p = head; p != NULL; p = p->pNext) ++n_nodes;
	FileWrite(f, &n_nodes, sizeof(UINT32));

	for (const PathSt* p = head; p != NULL; p = p->pNext)
	{
		BYTE  data[20];
		BYTE* d = data;
		INJ_U32(d, p->uiSectorId)
		INJ_SKIP(d, 16)
		Assert(d == endof(data));

		FileWrite(f, data, sizeof(data));
	}
}


void LoadMercPath(HWFILE const hFile, PathSt** const head)
{
	//Load the number of the nodes
	UINT32 uiNumOfNodes = 0;
	FileRead(hFile, &uiNumOfNodes, sizeof(UINT32));

	//load all the nodes
	PathSt* path = NULL;
	for (UINT32 cnt = 0; cnt < uiNumOfNodes; ++cnt)
	{
		PathSt* const n = MALLOC(PathSt);

		BYTE data[20];
		FileRead(hFile, data, sizeof(data));

		const BYTE* d = data;
		EXTR_U32(d, n->uiSectorId)
		EXTR_SKIP(d, 16)
		Assert(d == endof(data));

		n->pPrev = path;
		n->pNext = NULL;

		//Put the node into the list
		if (path == NULL)
		{
			*head = n;
		}
		else
		{
			path->pNext = n;
		}
		path = n;
	}
}


#ifdef JA2BETAVERSION
static void InitSaveGameFilePosition(UINT8 const slot)
{
	CHAR8		zFileName[128];
	sprintf(zFileName, "%s/SaveGameFilePos%2d.txt", g_savegame_dir, slot);
	FileDelete( zFileName );
}


static void SaveGameFilePosition(UINT8 const slot, const HWFILE save, const char* const pMsg)
{
	CHAR8		zTempString[512];
	UINT32	uiStrLen=0;
	CHAR8		zFileName[128];

	sprintf(zFileName, "%s/SaveGameFilePos%2d.txt", g_savegame_dir, slot);

	// create the save game file
	AutoSGPFile hFile(FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS));

	const INT32 pos = FileGetPos(save);
	sprintf(zTempString, "%8d     %s\n", pos, pMsg);
	uiStrLen = strlen( zTempString );
	FileWrite(hFile, zTempString, uiStrLen);
}


static void InitLoadGameFilePosition(UINT8 const slot)
{
	CHAR8		zFileName[128];
	sprintf(zFileName, "%s/LoadGameFilePos%2d.txt", g_savegame_dir, slot);
	FileDelete( zFileName );
}


static void LoadGameFilePosition(UINT8 const slot, const HWFILE load, const char* const pMsg)
{
	CHAR8		zTempString[512];
	UINT32	uiStrLen=0;

	CHAR8		zFileName[128];
	sprintf(zFileName, "%s/LoadGameFilePos%2d.txt", g_savegame_dir, slot);

	// create the save game file
	AutoSGPFile hFile(FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS));

	const INT32 pos = FileGetPos(load);
	sprintf(zTempString, "%8d     %s\n", pos, pMsg);
	uiStrLen = strlen( zTempString );

	FileWrite(hFile, zTempString, uiStrLen);
}
#endif


static BYTE* InjectMeanwhileDefinition(BYTE* const data, MEANWHILE_DEFINITION const& m)
{
	BYTE* d = data;
	INJ_I16(d, m.sSectorX)
	INJ_I16(d, m.sSectorY)
	INJ_U16(d, m.usTriggerEvent)
	INJ_U8( d, m.ubMeanwhileID)
	INJ_U8( d, m.ubNPCNumber)
	Assert(d == data + 8);
	return d;
}


static BYTE const* ExtractMeanwhileDefinition(BYTE const* const data, MEANWHILE_DEFINITION& m)
{
	BYTE const* d = data;
	EXTR_I16(d, m.sSectorX)
	EXTR_I16(d, m.sSectorY)
	EXTR_U16(d, m.usTriggerEvent)
	EXTR_U8( d, m.ubMeanwhileID)
	EXTR_U8( d, m.ubNPCNumber)
	Assert(d == data + 8);
	return d;
}


static void SaveGeneralInfo(HWFILE const f)
{
	BYTE  data[1024];
	BYTE* d = data;
	INJ_U32(  d, guiPreviousOptionScreen)
	INJ_U32(  d, guiCurrentUniqueSoldierId)
	INJ_U8(   d, gubMusicMode)
	INJ_SKIP( d, 1)
	INJ_U16(  d, Soldier2ID(GetSelectedMan()))
	INJ_I16(  d, gsRenderCenterX)
	INJ_I16(  d, gsRenderCenterY)
	INJ_BOOL( d, gfAtLeastOneMercWasHired)
	INJ_BOOL( d, fShowItemsFlag)
	INJ_BOOL( d, fShowTownFlag)
	INJ_BOOL( d, fShowTeamFlag)
	INJ_BOOL( d, fShowMineFlag)
	INJ_BOOL( d, fShowAircraftFlag)
	INJ_BOOL( d, iHelicopterVehicleId != -1) // XXX HACK000B
	INJ_SKIP( d, 1)
	INJ_I32(  d, iHelicopterVehicleId)
	INJ_SKIP( d, 4)
	INJ_I32(  d, iTotalAccumulatedCostByPlayer)
	INJ_BOOL( d, iHelicopterVehicleId != -1) // XXX HACK000B
	INJ_SKIP( d, 9)
	INJ_BOOL( d, fHelicopterIsAirBorne)
	INJ_BOOL( d, fHeliReturnStraightToBase)
	INJ_BOOL( d, fHoveringHelicopter)
	INJ_SKIP( d, 3)
	INJ_U32(  d, uiStartHoverTime)
	INJ_U32(  d, guiHelicopterSkyriderTalkState)
	INJ_BOOL( d, fShowEstoniRefuelHighLight)
	INJ_BOOL( d, fShowOtherSAMHighLight)
	INJ_BOOL( d, fShowDrassenSAMHighLight)
	INJ_SKIP( d, 1)
	INJ_U32(  d, guiEnvWeather)
	INJ_U8(   d, gubDefaultButton)
	INJ_BOOL( d, gfSkyriderEmptyHelpGiven)
	INJ_BOOL( d, fEnterMapDueToContract)
	INJ_U8(   d, gubHelicopterHitsTaken)
	INJ_SKIP( d, 1)
	INJ_BOOL( d, gfSkyriderSaidCongratsOnTakingSAM)
	INJ_I16(  d, pContractReHireSoldier ? pContractReHireSoldier->ubID : -1)
	d = InjectGameOptions(d, gGameOptions);
#ifdef JA2BETAVERSION
	// Everytime we save get, and set a seed value, when reload, seed again
	UINT32 const seed = GetJA2Clock();
	INJ_U32(  d, seed)
	srand(seed);
#else
	INJ_SKIP( d, 4)
#endif
	INJ_U32(  d, guiBaseJA2Clock)
	INJ_I16(  d, gsCurInterfacePanel)
	INJ_U8(   d, gpSMCurrentMerc ? gpSMCurrentMerc->ubID : 255)
	INJ_SKIP( d, 1)
	INJ_BOOL( d, fDisableDueToBattleRoster)
	INJ_BOOL( d, fDisableMapInterfaceDueToBattle)
	INJ_I16A( d, gsBoxerGridNo, lengthof(gsBoxerGridNo))
	for (SOLDIERTYPE* const* i = gBoxer; i != endof(gBoxer); ++i)
	{
		INJ_SOLDIER(d, *i)
	}
	INJ_BOOLA(d, gfBoxerFought, lengthof(gfBoxerFought))
	INJ_BOOL( d, fHelicopterDestroyed)
	INJ_SKIP( d, 1)
	INJ_I32(  d, giSortStateForMapScreenList)
	INJ_BOOL( d, fFoundTixa)
	INJ_SKIP( d, 3)
	INJ_U32(  d, guiTimeOfLastSkyriderMonologue)
	INJ_BOOL( d, fShowCambriaHospitalHighLight)
	INJ_BOOL( d, fSkyRiderSetUp)
	INJ_BOOLA(d, fRefuelingSiteAvailable, lengthof(fRefuelingSiteAvailable))
	d = InjectMeanwhileDefinition(d, gCurrentMeanwhileDef);
	INJ_BOOL( d, gubPlayerProgressSkyriderLastCommentedOn)
	INJ_BOOL( d, gfMeanwhileTryingToStart)
	INJ_BOOL( d, gfInMeanwhile)
	INJ_SKIP( d, 1)
	for (INT16 (* i)[NUMBER_OF_SOLDIERS_PER_SQUAD] = sDeadMercs; i != endof(sDeadMercs); ++i)
	{
		INJ_I16A(d, *i, lengthof(*i))
	}
	INJ_I8A(  d, gbPublicNoiseLevel, lengthof(gbPublicNoiseLevel))
	INJ_U8(   d, gubScreenCount)
	INJ_SKIP( d, 5)
	INJ_I32(  d, iPortraitNumber)
	INJ_I16(  d, sWorldSectorLocationOfFirstBattle)
	INJ_BOOL( d, fUnReadMailFlag)
	INJ_BOOL( d, fNewMailFlag)
	INJ_BOOL( d, FALSE) // XXX HACK000B
	INJ_BOOL( d, fOldNewMailFlag)
	INJ_BOOL( d, fShowMilitia)
	INJ_BOOL( d, fNewFilesInFileViewer)
	INJ_BOOL( d, gfLastBoxingMatchWonByPlayer)
	INJ_SKIP( d, 7)
	INJ_BOOLA(d, fSamSiteFound, lengthof(fSamSiteFound))
	INJ_U8(   d, gubNumTerrorists)
	INJ_U8(   d, gubCambriaMedicalObjects)
	INJ_BOOL( d, gfDisableTacticalPanelButtons)
	INJ_SKIP( d, 1)
	INJ_I16(  d, sSelMapX)
	INJ_I16(  d, sSelMapY)
	INJ_I32(  d, iCurrentMapSectorZ)
	INJ_U16(  d, gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen)
	INJ_SKIP( d, 1)
	INJ_U8(   d, gubBoxingMatchesWon)
	INJ_U8(   d, gubBoxersRests)
	INJ_BOOL( d, gfBoxersResting)
	INJ_U8(   d, gubDesertTemperature)
	INJ_U8(   d, gubGlobalTemperature)
	INJ_I16(  d, gsMercArriveSectorX)
	INJ_I16(  d, gsMercArriveSectorY)
	INJ_BOOL( d, gfCreatureMeanwhileScenePlayed)
	INJ_U8(   d, gbPlayerNum)
	INJ_BOOL( d, gfPersistantPBI)
	INJ_U8(   d, gubEnemyEncounterCode)
	INJ_BOOL( d, gubExplicitEnemyEncounterCode)
	INJ_BOOL( d, gfBlitBattleSectorLocator)
	INJ_U8(   d, gubPBSectorX)
	INJ_U8(   d, gubPBSectorY)
	INJ_U8(   d, gubPBSectorZ)
	INJ_BOOL( d, gfCantRetreatInPBI)
	INJ_BOOL( d, gfExplosionQueueActive)
	INJ_SKIP( d, 1)
	INJ_U32(  d, uiMeanWhileFlags)
	INJ_I8(   d, bSelectedInfoChar)
	INJ_I8(   d, gbHospitalPriceModifier)
	INJ_SKIP( d, 2)
	INJ_I32(  d, giHospitalTempBalance)
	INJ_I32(  d, giHospitalRefund)
	INJ_I8(   d, gfPlayerTeamSawJoey)
	INJ_I8(   d, gfMikeShouldSayHi)
	INJ_SKIP( d, 550)
	Assert(d == endof(data));

	FileWrite(f, data, sizeof(data));
}


static void LoadGeneralInfo(HWFILE const f, UINT32 const savegame_version)
{
	BYTE data[1024];
	FileRead(f, data, sizeof(data));

	BYTE const* d = data;
	UINT32 screen_after_loading;
	EXTR_U32(  d, screen_after_loading)
	guiScreenToGotoAfterLoadingSavedGame = static_cast<ScreenID>(screen_after_loading); // XXX TODO001A unchecked conversion
	EXTR_U32(  d, guiCurrentUniqueSoldierId)
	EXTR_U8(   d, gMusicModeToPlay)
	EXTR_SKIP( d, 1)
	UINT16 sel;
	EXTR_U16(  d, sel)
	SetSelectedMan(ID2Soldier(sel));
	EXTR_I16(  d, gsRenderCenterX)
	EXTR_I16(  d, gsRenderCenterY)
	EXTR_BOOL( d, gfAtLeastOneMercWasHired)
	EXTR_BOOL( d, fShowItemsFlag)
	EXTR_BOOL( d, fShowTownFlag)
	EXTR_BOOL( d, fShowTeamFlag)
	EXTR_BOOL( d, fShowMineFlag)
	EXTR_BOOL( d, fShowAircraftFlag)
	EXTR_SKIP( d, 2)
	EXTR_I32(  d, iHelicopterVehicleId)
	EXTR_SKIP( d, 4)
	EXTR_I32(  d, iTotalAccumulatedCostByPlayer)
	EXTR_SKIP( d, 10)
	EXTR_BOOL( d, fHelicopterIsAirBorne)
	EXTR_BOOL( d, fHeliReturnStraightToBase)
	EXTR_BOOL( d, fHoveringHelicopter)
	EXTR_SKIP( d, 3)
	EXTR_U32(  d, uiStartHoverTime)
	EXTR_U32(  d, guiHelicopterSkyriderTalkState)
	EXTR_BOOL( d, fShowEstoniRefuelHighLight)
	EXTR_BOOL( d, fShowOtherSAMHighLight)
	EXTR_BOOL( d, fShowDrassenSAMHighLight)
	EXTR_SKIP( d, 1)
	EXTR_U32(  d, guiEnvWeather)
	EXTR_U8(   d, gubDefaultButton)
	EXTR_BOOL( d, gfSkyriderEmptyHelpGiven)
	EXTR_BOOL( d, fEnterMapDueToContract)
	EXTR_U8(   d, gubHelicopterHitsTaken)
	EXTR_SKIP( d, 1)
	EXTR_BOOL( d, gfSkyriderSaidCongratsOnTakingSAM)
	INT16 contract_rehire_soldier;
	EXTR_I16(  d, contract_rehire_soldier)
	pContractReHireSoldier = contract_rehire_soldier != -1 ? GetMan(contract_rehire_soldier) : 0;
	d = ExtractGameOptions(d, gGameOptions);
#ifdef JA2BETAVERSION
	// Everytime we save get, and set a seed value, when reload, seed again
	UINT32 seed;
	EXTR_U32(  d, seed)
	srand(seed);
#else
	EXTR_SKIP( d, 4)
#endif
	EXTR_U32(  d, guiBaseJA2Clock)
	ResetJA2ClockGlobalTimers();
	INT16 cur_interface_panel;
	EXTR_I16(  d, cur_interface_panel)
	UINT8 sm_current_merc;
	EXTR_U8(   d, sm_current_merc)
	gpSMCurrentMerc = sm_current_merc != 255 ? GetMan(sm_current_merc) : 0;
	// Set the interface panel to the team panel
	ShutdownCurrentPanel();
	gsCurInterfacePanel = static_cast<InterfacePanelKind>(cur_interface_panel); // XXX TODO001A unchecked conversion
	EXTR_SKIP( d, 1)
	EXTR_BOOL( d, fDisableDueToBattleRoster)
	EXTR_BOOL( d, fDisableMapInterfaceDueToBattle)
	EXTR_I16A( d, gsBoxerGridNo, lengthof(gsBoxerGridNo))
	for (SOLDIERTYPE** i = gBoxer; i != endof(gBoxer); ++i)
	{
		EXTR_SOLDIER(d, *i)
	}
	EXTR_BOOLA(d, gfBoxerFought, lengthof(gfBoxerFought))
	EXTR_BOOL( d, fHelicopterDestroyed)
	EXTR_SKIP( d, 1)
	EXTR_I32(  d, giSortStateForMapScreenList)
	EXTR_BOOL( d, fFoundTixa)
	EXTR_SKIP( d, 3)
	EXTR_U32(  d, guiTimeOfLastSkyriderMonologue)
	EXTR_BOOL( d, fShowCambriaHospitalHighLight)
	EXTR_BOOL( d, fSkyRiderSetUp)
	EXTR_BOOLA(d, fRefuelingSiteAvailable, lengthof(fRefuelingSiteAvailable))
	d = ExtractMeanwhileDefinition(d, gCurrentMeanwhileDef);
	EXTR_BOOL( d, gubPlayerProgressSkyriderLastCommentedOn)
	EXTR_BOOL( d, gfMeanwhileTryingToStart)
	EXTR_BOOL( d, gfInMeanwhile)
	EXTR_SKIP( d, 1)
	for (INT16 (* i)[NUMBER_OF_SOLDIERS_PER_SQUAD] = sDeadMercs; i != endof(sDeadMercs); ++i)
	{
		EXTR_I16A(d, *i, lengthof(*i))
	}
	EXTR_I8A(  d, gbPublicNoiseLevel, lengthof(gbPublicNoiseLevel))
	EXTR_U8(   d, gubScreenCount)
	EXTR_SKIP(d, 1)
	UINT16 old_meanwhile_flags;
	EXTR_U16(d, old_meanwhile_flags)
	EXTR_SKIP(d, 2)
	EXTR_I32(  d, iPortraitNumber)
	EXTR_I16(  d, sWorldSectorLocationOfFirstBattle)
	EXTR_BOOL( d, fUnReadMailFlag)
	EXTR_BOOL( d, fNewMailFlag)
	EXTR_SKIP( d, 1)
	EXTR_BOOL( d, fOldNewMailFlag)
	EXTR_BOOL( d, fShowMilitia)
	EXTR_BOOL( d, fNewFilesInFileViewer)
	EXTR_BOOL( d, gfLastBoxingMatchWonByPlayer)
	EXTR_SKIP( d, 7)
	EXTR_BOOLA(d, fSamSiteFound, lengthof(fSamSiteFound))
	EXTR_U8(   d, gubNumTerrorists)
	EXTR_U8(   d, gubCambriaMedicalObjects)
	EXTR_BOOL( d, gfDisableTacticalPanelButtons)
	EXTR_SKIP( d, 1)
	EXTR_I16(  d, sSelMapX)
	EXTR_I16(  d, sSelMapY)
	EXTR_I32(  d, iCurrentMapSectorZ)
	EXTR_U16(  d, gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen)
	EXTR_SKIP( d, 1)
	EXTR_U8(   d, gubBoxingMatchesWon)
	EXTR_U8(   d, gubBoxersRests)
	EXTR_BOOL( d, gfBoxersResting)
	EXTR_U8(   d, gubDesertTemperature)
	EXTR_U8(   d, gubGlobalTemperature)
	EXTR_I16(  d, gsMercArriveSectorX)
	EXTR_I16(  d, gsMercArriveSectorY)
	EXTR_BOOL( d, gfCreatureMeanwhileScenePlayed)
	EXTR_U8(   d, gbPlayerNum)
	EXTR_BOOL( d, gfPersistantPBI)
	EXTR_U8(   d, gubEnemyEncounterCode)
	EXTR_BOOL( d, gubExplicitEnemyEncounterCode)
	EXTR_BOOL( d, gfBlitBattleSectorLocator)
	EXTR_U8(   d, gubPBSectorX)
	EXTR_U8(   d, gubPBSectorY)
	EXTR_U8(   d, gubPBSectorZ)
	EXTR_BOOL( d, gfCantRetreatInPBI)
	EXTR_BOOL( d, gfExplosionQueueActive)
	EXTR_SKIP( d, 1)
	UINT32 meanwhile_flags;
	EXTR_U32(  d, meanwhile_flags)
	uiMeanWhileFlags = savegame_version < 71 ? old_meanwhile_flags : meanwhile_flags;
	EXTR_I8(   d, bSelectedInfoChar)
	EXTR_I8(   d, gbHospitalPriceModifier)
	EXTR_SKIP( d, 2)
	EXTR_I32(  d, giHospitalTempBalance)
	EXTR_I32(  d, giHospitalRefund)
	EXTR_I8(   d, gfPlayerTeamSawJoey)
	EXTR_I8(   d, gfMikeShouldSayHi)
	EXTR_SKIP( d, 550)
	Assert(d == endof(data));
}


static void SavePreRandomNumbersToSaveGameFile(HWFILE const hFile)
{
	//Save the Prerandom number index
	FileWrite(hFile, &guiPreRandomIndex, sizeof(UINT32));

	//Save the Prerandom number index
	FileWrite(hFile, guiPreRandomNums, sizeof(UINT32) * MAX_PREGENERATED_NUMS);
}


static void LoadPreRandomNumbersFromSaveGameFile(HWFILE const hFile)
{
	//Load the Prerandom number index
	FileRead(hFile, &guiPreRandomIndex, sizeof(UINT32));

	//Load the Prerandom number index
	FileRead(hFile, guiPreRandomNums, sizeof(UINT32) * MAX_PREGENERATED_NUMS);
}


static void LoadMeanwhileDefsFromSaveGameFile(HWFILE const f, UINT32 const savegame_version)
{
	MEANWHILE_DEFINITION const* end;
	if (savegame_version < 72)
	{
		memset(&gMeanwhileDef[NUM_MEANWHILES - 1], 0, sizeof(gMeanwhileDef[NUM_MEANWHILES - 1]));
		end = gMeanwhileDef + NUM_MEANWHILES - 1;
	}
	else
	{
		end = endof(gMeanwhileDef);
	}
	for (MEANWHILE_DEFINITION* i = gMeanwhileDef; i != end; ++i)
	{
		BYTE data[8];
		FileRead(f, data, sizeof(data));
		ExtractMeanwhileDefinition(data, *i);
	}
}


static void SaveMeanwhileDefsFromSaveGameFile(HWFILE const f)
{
	for (MEANWHILE_DEFINITION* i = gMeanwhileDef; i != endof(gMeanwhileDef); ++i)
	{
		BYTE data[8];
		InjectMeanwhileDefinition(data, *i);
		FileWrite(f, data, sizeof(data));
	}
}


BOOLEAN DoesUserHaveEnoughHardDriveSpace()
{
	UINT32			uiBytesFree=0;

	uiBytesFree = GetFreeSpaceOnHardDriveWhereGameIsRunningFrom( );

	//check to see if there is enough hard drive space
	if( uiBytesFree < REQUIRED_FREE_SPACE )
	{
		return( FALSE );
	}

	return( TRUE );
}

#ifdef JA2BETAVERSION

static void InitShutDownMapTempFileTest(BOOLEAN fInit, const char* pNameOfFile, UINT8 ubSaveGameID)
{
	CHAR8		zFileName[128];
	CHAR8		zTempString[512];
	UINT32	uiStrLen;

	//strcpy( gzNameOfMapTempFile, pNameOfFile);
	sprintf( gzNameOfMapTempFile, "%s%d", pNameOfFile, ubSaveGameID );

	sprintf(zFileName, "%s/%s.txt", g_savegame_dir, gzNameOfMapTempFile);

	if( fInit )
	{
		guiNumberOfMapTempFiles = 0;		//Test:  To determine where the temp files are crashing
		guiSizeOfTempFiles = 0;

		FileDelete(zFileName);
	}
	else
	{
		// create the save game file
		AutoSGPFile hFile(FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS));

		sprintf( zTempString, "Number Of Files: %6d.  Size of all files: %6d.\n", guiNumberOfMapTempFiles, guiSizeOfTempFiles );
		uiStrLen = strlen( zTempString );
		FileWrite(hFile, zTempString, uiStrLen);
	}
}


static void WriteTempFileNameToFile(const char* pFileName, UINT32 uiSizeOfFile, HWFILE hSaveFile)
{
	CHAR8		zTempString[512];
	UINT32	uiStrLen=0;

	CHAR8		zFileName[128];

	guiSizeOfTempFiles += uiSizeOfFile;

	sprintf(zFileName, "%s/%s.txt", g_savegame_dir, gzNameOfMapTempFile);

	// create the save game file
	AutoSGPFile hFile(FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS));

	sprintf( zTempString, "%8d   %6d   %s\n", FileGetPos( hSaveFile ), uiSizeOfFile, pFileName );
	uiStrLen = strlen( zTempString );

	FileWrite(hFile, zTempString, uiStrLen);
}

#endif


void GetBestPossibleSectorXYZValues(INT16* const psSectorX, INT16* const psSectorY, INT8* const pbSectorZ)
{
	//if the current sector is valid
	if (gfWorldLoaded)
	{
		*psSectorX = gWorldSectorX;
		*psSectorY = gWorldSectorY;
		*pbSectorZ = gbWorldSectorZ;
		return;
	}

	if (iCurrentTacticalSquad != NO_CURRENT_SQUAD)
	{
		const SOLDIERTYPE* const s = Squad[iCurrentTacticalSquad][0];
		if (s != NULL && s->bAssignment != IN_TRANSIT)
		{
			*psSectorX = s->sSectorX;
			*psSectorY = s->sSectorY;
			*pbSectorZ = s->bSectorZ;
			return;
		}
	}

	//loop through all the mercs on the players team to find the one that is not moving
	CFOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		if (s->bAssignment != IN_TRANSIT && !s->fBetweenSectors)
		{
			//we found an alive, merc that is not moving
			*psSectorX = s->sSectorX;
			*psSectorY = s->sSectorY;
			*pbSectorZ = s->bSectorZ;
			return;
		}
	}

	// loop through all the mercs and find one that is moving
	CFOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		//we found an alive, merc that is not moving
		*psSectorX = s->sSectorX;
		*psSectorY = s->sSectorY;
		*pbSectorZ = s->bSectorZ;
		return;
	}

	// if we STILL havent found a merc, give up and use the -1, -1, -1
	*psSectorX = gWorldSectorX;
	*psSectorY = gWorldSectorY;
	*pbSectorZ = gbWorldSectorZ;
}


static void PauseBeforeSaveGame(void)
{
	//if we are not in the save load screen
	if( guiCurrentScreen != SAVE_LOAD_SCREEN )
	{
		//Pause the game
		PauseGame();
	}
}


static void UnPauseAfterSaveGame(void)
{
	//if we are not in the save load screen
	if( guiCurrentScreen != SAVE_LOAD_SCREEN )
	{
		//UnPause time compression
		UnPauseGame();
	}
}


static void TruncateStrategicGroupSizes(void)
{
	SECTORINFO *pSector;
	INT32 i;
	for( i = SEC_A1; i < SEC_P16; i++ )
	{
		pSector = &SectorInfo[ i ];
		if( pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites > MAX_STRATEGIC_TEAM_SIZE )
		{
			if( pSector->ubNumAdmins > pSector->ubNumTroops )
			{
				if( pSector->ubNumAdmins > pSector->ubNumElites )
				{
					pSector->ubNumAdmins = 20;
					pSector->ubNumTroops = 0;
					pSector->ubNumElites = 0;
				}
				else
				{
					pSector->ubNumAdmins = 0;
					pSector->ubNumTroops = 0;
					pSector->ubNumElites = 20;
				}
			}
			else if( pSector->ubNumTroops > pSector->ubNumElites )
			{
				if( pSector->ubNumTroops > pSector->ubNumAdmins )
				{
					pSector->ubNumAdmins = 0;
					pSector->ubNumTroops = 20;
					pSector->ubNumElites = 0;
				}
				else
				{
					pSector->ubNumAdmins = 20;
					pSector->ubNumTroops = 0;
					pSector->ubNumElites = 0;
				}
			}
			else
			{
				if( pSector->ubNumElites > pSector->ubNumTroops )
				{
					pSector->ubNumAdmins = 0;
					pSector->ubNumTroops = 0;
					pSector->ubNumElites = 20;
				}
				else
				{
					pSector->ubNumAdmins = 0;
					pSector->ubNumTroops = 20;
					pSector->ubNumElites = 0;
				}
			}
		}
		//militia
		if( pSector->ubNumberOfCivsAtLevel[0] + pSector->ubNumberOfCivsAtLevel[1] + pSector->ubNumberOfCivsAtLevel[2] > MAX_STRATEGIC_TEAM_SIZE )
		{
			if( pSector->ubNumberOfCivsAtLevel[0] > pSector->ubNumberOfCivsAtLevel[1] )
			{
				if( pSector->ubNumberOfCivsAtLevel[0] > pSector->ubNumberOfCivsAtLevel[2] )
				{
					pSector->ubNumberOfCivsAtLevel[0] = 20;
					pSector->ubNumberOfCivsAtLevel[1] = 0;
					pSector->ubNumberOfCivsAtLevel[2] = 0;
				}
				else
				{
					pSector->ubNumberOfCivsAtLevel[0] = 0;
					pSector->ubNumberOfCivsAtLevel[1] = 0;
					pSector->ubNumberOfCivsAtLevel[2] = 20;
				}
			}
			else if( pSector->ubNumberOfCivsAtLevel[1] > pSector->ubNumberOfCivsAtLevel[2] )
			{
				if( pSector->ubNumberOfCivsAtLevel[1] > pSector->ubNumberOfCivsAtLevel[0] )
				{
					pSector->ubNumberOfCivsAtLevel[0] = 0;
					pSector->ubNumberOfCivsAtLevel[1] = 20;
					pSector->ubNumberOfCivsAtLevel[2] = 0;
				}
				else
				{
					pSector->ubNumberOfCivsAtLevel[0] = 20;
					pSector->ubNumberOfCivsAtLevel[1] = 0;
					pSector->ubNumberOfCivsAtLevel[2] = 0;
				}
			}
			else
			{
				if( pSector->ubNumberOfCivsAtLevel[2] > pSector->ubNumberOfCivsAtLevel[1] )
				{
					pSector->ubNumberOfCivsAtLevel[0] = 0;
					pSector->ubNumberOfCivsAtLevel[1] = 0;
					pSector->ubNumberOfCivsAtLevel[2] = 20;
				}
				else
				{
					pSector->ubNumberOfCivsAtLevel[0] = 0;
					pSector->ubNumberOfCivsAtLevel[1] = 20;
					pSector->ubNumberOfCivsAtLevel[2] = 0;
				}
			}
		}
	}
	//Enemy groups
	FOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if( pGroup->pEnemyGroup->ubNumAdmins + pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites > MAX_STRATEGIC_TEAM_SIZE )
		{
			pGroup->ubGroupSize = 20;
			if( pGroup->pEnemyGroup->ubNumAdmins > pGroup->pEnemyGroup->ubNumTroops )
			{
				if( pGroup->pEnemyGroup->ubNumAdmins > pGroup->pEnemyGroup->ubNumElites )
				{
					pGroup->pEnemyGroup->ubNumAdmins = 20;
					pGroup->pEnemyGroup->ubNumTroops = 0;
					pGroup->pEnemyGroup->ubNumElites = 0;
				}
				else
				{
					pGroup->pEnemyGroup->ubNumAdmins = 0;
					pGroup->pEnemyGroup->ubNumTroops = 0;
					pGroup->pEnemyGroup->ubNumElites = 20;
				}
			}
			else if( pGroup->pEnemyGroup->ubNumTroops > pGroup->pEnemyGroup->ubNumElites )
			{
				if( pGroup->pEnemyGroup->ubNumTroops > pGroup->pEnemyGroup->ubNumAdmins )
				{
					pGroup->pEnemyGroup->ubNumAdmins = 0;
					pGroup->pEnemyGroup->ubNumTroops = 20;
					pGroup->pEnemyGroup->ubNumElites = 0;
				}
				else
				{
					pGroup->pEnemyGroup->ubNumAdmins = 20;
					pGroup->pEnemyGroup->ubNumTroops = 0;
					pGroup->pEnemyGroup->ubNumElites = 0;
				}
			}
			else
			{
				if( pGroup->pEnemyGroup->ubNumElites > pGroup->pEnemyGroup->ubNumTroops )
				{
					pGroup->pEnemyGroup->ubNumAdmins = 0;
					pGroup->pEnemyGroup->ubNumTroops = 0;
					pGroup->pEnemyGroup->ubNumElites = 20;
				}
				else
				{
					pGroup->pEnemyGroup->ubNumAdmins = 0;
					pGroup->pEnemyGroup->ubNumTroops = 20;
					pGroup->pEnemyGroup->ubNumElites = 0;
				}
			}
		}
	}
}


static void UpdateMercMercContractInfo(void)
{
	UINT8	ubCnt;

	for( ubCnt=BIFF; ubCnt<= BUBBA; ubCnt++ )
	{
		SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(ubCnt);

		//if the merc is on the team
		if( pSoldier == NULL )
			continue;

		gMercProfiles[ ubCnt ].iMercMercContractLength = pSoldier->iTotalContractLength;

		pSoldier->iTotalContractLength = 0;
	}
}

INT8 GetNumberForAutoSave( BOOLEAN fLatestAutoSave )
{
	BOOLEAN	fFile1Exist, fFile2Exist;
	SGP_FILETIME	CreationTime1, LastAccessedTime1, LastWriteTime1;
	SGP_FILETIME	CreationTime2, LastAccessedTime2, LastWriteTime2;

	fFile1Exist = FALSE;
	fFile2Exist = FALSE;

	//The name of the file
	char zFileName1[256];
	sprintf(zFileName1, "%s/Auto%02d.%s", g_savegame_dir, 0, g_savegame_ext);
	char zFileName2[256];
	sprintf(zFileName2, "%s/Auto%02d.%s", g_savegame_dir, 1, g_savegame_ext);

	if( FileExists( zFileName1 ) )
	{
		AutoSGPFile hFile(FileOpen(zFileName1, FILE_ACCESS_READ));
		GetFileManFileTime( hFile, &CreationTime1, &LastAccessedTime1, &LastWriteTime1 );
		fFile1Exist = TRUE;
	}

	if( FileExists( zFileName2 ) )
	{
		AutoSGPFile hFile(FileOpen(zFileName2, FILE_ACCESS_READ));
		GetFileManFileTime( hFile, &CreationTime2, &LastAccessedTime2, &LastWriteTime2 );
		fFile2Exist = TRUE;
	}

	if( !fFile1Exist && !fFile2Exist )
		return( -1 );
	else if( fFile1Exist && !fFile2Exist )
	{
		if( fLatestAutoSave )
			return( 0 );
		else
			return( -1 );
	}
	else if( !fFile1Exist && fFile2Exist )
	{
		if( fLatestAutoSave )
			return( 1 );
		else
			return( -1 );
	}
	else
	{
		if( CompareSGPFileTimes( &LastWriteTime1, &LastWriteTime2 ) > 0 )
			return( 0 );
		else
			return( 1 );
	}
}


static void HandleOldBobbyRMailOrders(void)
{
	INT32 iCnt;
	INT32	iNewListCnt=0;

	if( LaptopSaveInfo.usNumberOfBobbyRayOrderUsed != 0 )
	{
		gpNewBobbyrShipments = MALLOCN(NewBobbyRayOrderStruct, LaptopSaveInfo.usNumberOfBobbyRayOrderUsed);

		giNumberOfNewBobbyRShipment = LaptopSaveInfo.usNumberOfBobbyRayOrderUsed;

		//loop through and add the old items to the new list
		for( iCnt=0; iCnt< LaptopSaveInfo.usNumberOfBobbyRayOrderItems; iCnt++ )
		{
			//if this slot is used
			if( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].fActive )
			{
				//copy over the purchase info
				memcpy( gpNewBobbyrShipments[ iNewListCnt ].BobbyRayPurchase,
								LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].BobbyRayPurchase,
								sizeof( BobbyRayPurchaseStruct ) * MAX_PURCHASE_AMOUNT );

				gpNewBobbyrShipments[ iNewListCnt ].fActive = TRUE;
				gpNewBobbyrShipments[ iNewListCnt ].ubDeliveryLoc = BR_DRASSEN;
				gpNewBobbyrShipments[ iNewListCnt ].ubDeliveryMethod = 0;
				gpNewBobbyrShipments[ iNewListCnt ].ubNumberPurchases = LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].ubNumberPurchases;
				gpNewBobbyrShipments[ iNewListCnt ].uiPackageWeight = 1;
				gpNewBobbyrShipments[ iNewListCnt ].uiOrderedOnDayNum = GetWorldDay();
				gpNewBobbyrShipments[ iNewListCnt ].fDisplayedInShipmentPage = TRUE;

				iNewListCnt++;
			}
		}

		//Clear out the old list
		LaptopSaveInfo.usNumberOfBobbyRayOrderUsed = 0;
		MemFree( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray );
		LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
	}
}


static UINT32 CalcJA2EncryptionSet(SAVED_GAME_HEADER* pSaveGameHeader)
{
	UINT32	uiEncryptionSet = 0;

	uiEncryptionSet  = pSaveGameHeader->iCurrentBalance;
	uiEncryptionSet *= (pSaveGameHeader->ubNumOfMercsOnPlayersTeam + 1);
	uiEncryptionSet += pSaveGameHeader->bSectorZ * 3;
	uiEncryptionSet += pSaveGameHeader->ubLoadScreenID;

	if ( pSaveGameHeader->fAlternateSector )
	{
		uiEncryptionSet += 7;
	}

	if ( pSaveGameHeader->uiRandom % 2 == 0 )
	{
		uiEncryptionSet++;

		if ( pSaveGameHeader->uiRandom % 7 == 0)
		{
			uiEncryptionSet++;
			if ( pSaveGameHeader->uiRandom % 23 == 0 )
			{
				uiEncryptionSet++;
			}
			if ( pSaveGameHeader->uiRandom % 79 == 0 )
			{
				uiEncryptionSet += 2;
			}
		}
	}

	#ifdef GERMAN
		uiEncryptionSet *= 11;
	#endif

	uiEncryptionSet = uiEncryptionSet % 10;

	uiEncryptionSet += pSaveGameHeader->uiDay / 10;

	uiEncryptionSet = uiEncryptionSet % 19;

	// now pick a different set of #s depending on what game options we've chosen
	if ( pSaveGameHeader->sInitialGameOptions.fGunNut )
	{
		uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS * 6;
	}

	if ( pSaveGameHeader->sInitialGameOptions.fSciFi )
	{
		uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS * 3;
	}

	switch( pSaveGameHeader->sInitialGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			uiEncryptionSet += 0;
			break;
		case DIF_LEVEL_MEDIUM:
			uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS;
			break;
		case DIF_LEVEL_HARD:
			uiEncryptionSet += BASE_NUMBER_OF_ROTATION_ARRAYS * 2;
			break;
	}

	return( uiEncryptionSet );
}
