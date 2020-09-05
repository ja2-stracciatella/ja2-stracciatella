#include "Air_Raid.h"
#include "AI.h"
#include "Animated_ProgressBar.h"
#include "Animation_Data.h"
#include "Arms_Dealer_Init.h"
#include "Assignments.h"
#include "BobbyR.h"
#include "BobbyRMailOrder.h"
#include "Boxing.h"
#include "Buffer.h"
#include "Bullets.h"
#include "Cheats.h"
#include "Civ_Quotes.h"
#include "ContentManager.h"
#include "ContentMusic.h"
#include "Creature_Spreading.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Enemy_Soldier_Save.h"
#include "Environment.h"
#include "Explosion_Control.h"
#include "FileMan.h"
#include "Files.h"
#include "Finances.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Events.h"
#include "Game_Clock.h"
#include "GameInstance.h"
#include "GameLoop.h"
#include "GameRes.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "HelpScreen.h"
#include "History.h"
#include "IMP_Compile_Character.h"
#include "IMP_Confirm.h"
#include "IMP_Portraits.h"
#include "Interface_Dialogue.h"
#include "Interface_Items.h"
#include "Interface_Panels.h"
#include "Interface_Utils.h"
#include "Interface.h"
#include "JAScreens.h"
#include "Keys.h"
#include "Laptop.h"
#include "Lighting.h"
#include "LightEffects.h"
#include "Loading_Screen.h"
#include "LoadSaveData.h"
#include "LoadSaveEMail.h"
#include "LoadSaveMercProfile.h"
#include "LoadSaveSoldierType.h"
#include "LoadSaveTacticalStatusType.h"
#include "Local.h"
#include "Logger.h"
#include "LaptopSave.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Information.h"
#include "MapScreen.h"
#include "Meanwhile.h"
#include "Merc_Contract.h"
#include "Merc_Hiring.h"
#include "Mercs.h"
#include "MercTextBox.h"
#include "MemMan.h"
#include "Message.h"
#include "Music_Control.h"
#include "NPC.h"
#include "OppList.h"
#include "Options_Screen.h"
#include "Overhead.h"
#include "Physics.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Random.h"
#include "Render_Dirty.h"
#include "RenderWorld.h"
#include "SaveLoadGame.h"
#include "SaveLoadScreen.h"
#include "Scheduling.h"
#include "ShippingDestinationModel.h"
#include "SmokeEffects.h"
#include "Soldier_Create.h"
#include "Soldier_Profile.h"
#include "Squads.h"
#include "Strategic_AI.h"
#include "Strategic_Event_Handler.h"
#include "Strategic_Mines.h"
#include "Strategic_Movement.h"
#include "Strategic_Status.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Pathing.h"
#include "StrategicMap.h"
#include "Tactical_Placement_GUI.h"
#include "Tactical_Save.h"
#include "TeamTurns.h"
#include "Text.h"
#include "Timer_Control.h"
#include "Types.h"
#include "UILayout.h"
#include "Vehicles.h"
#include "Video.h"
#include "VSurface.h"
#include "WorldDef.h"

#include "policy/GamePolicy.h"
#include "sgp/StrUtils.h"

#include <string_theory/string>

#include <algorithm>
#include <stdexcept>

static const char g_quicksave_name[] = "QuickSave";
static const char g_savegame_name[]  = "SaveGame";
static const char g_savegame_ext[]   = "sav";

//Global variable used

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

static MusicMode gMusicModeToPlay;

BOOLEAN	gfUseConsecutiveQuickSaveSlots = FALSE;
UINT32	guiLastSaveGameNum;

UINT32	guiJA2EncryptionSet = 0;


ScreenID guiScreenToGotoAfterLoadingSavedGame = ERROR_SCREEN; // XXX TODO001A was not properly initialised (0)

extern		UINT32		guiCurrentUniqueSoldierId;

static void SaveTempFileToSavedGame(const char* fileName, HWFILE const hFile);
static void LoadTempFileFromSavedGame(const char* tempFileName, HWFILE const hFile);

static void ExtractGameOptions(DataReader& d, GAME_OPTIONS& g)
{
	size_t start = d.getConsumed();
	EXTR_BOOL( d, g.fGunNut)
	EXTR_BOOL( d, g.fSciFi)
	EXTR_U8(   d, g.ubDifficultyLevel)
	EXTR_BOOL( d, g.fTurnTimeLimit)
	EXTR_U8(   d, g.ubGameSaveMode)
	EXTR_SKIP( d, 7)
	Assert(d.getConsumed() == start + 12);
}


static void InjectGameOptions(DataWriter& d, GAME_OPTIONS const& g)
{
	size_t start = d.getConsumed();
	INJ_BOOL( d, g.fGunNut)
	INJ_BOOL( d, g.fSciFi)
	INJ_U8(   d, g.ubDifficultyLevel)
	INJ_BOOL( d, g.fTurnTimeLimit)
	INJ_U8(   d, g.ubGameSaveMode)
	INJ_SKIP( d, 7)
	Assert(d.getConsumed() == start + 12);
}


static void CalcJA2EncryptionSet(SAVED_GAME_HEADER const&);
static void PauseBeforeSaveGame(void);
static void UnPauseAfterSaveGame(void);
static void SaveGeneralInfo(HWFILE);
static void SaveMeanwhileDefsToSaveGameFile(HWFILE);
static void SaveMercProfiles(HWFILE);
static void SaveOppListInfoToSavedGame(HWFILE);
static void SavePreRandomNumbersToSaveGameFile(HWFILE);
static void SaveSoldierStructure(HWFILE hFile);
static void SaveTacticalStatusToSavedGame(HWFILE);
static void SaveWatchedLocsToSavedGame(HWFILE);

static void SaveIMPPlayerProfiles();

BOOLEAN SaveGame(UINT8 ubSaveGameID, const ST::string& gameDesc)
{
	if (gamepolicy(imp_load_saved_merc_by_nickname)) SaveIMPPlayerProfiles();

	BOOLEAN	fPausedStateBeforeSaving    = gfGamePaused;
	BOOLEAN	fLockPauseStateBeforeSaving = gfLockPauseState;

	bool fWePausedIt = false;
	if (!GamePaused())
	{
		PauseBeforeSaveGame();
		fWePausedIt = true;
	}

	// Place a message on the screen telling the user that we are saving the game
	if (gGameOptions.ubGameSaveMode != DIF_DEAD_IS_DEAD)
	{
		UINT16 actual_w;
		UINT16 actual_h;
		AutoMercPopUpBox const save_load_game_message_box(PrepareMercPopupBox(0, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, zSaveLoadText[SLG_SAVING_GAME_MESSAGE], 300, 0, 0, 0, &actual_w, &actual_h));
		UINT16 const x = (SCREEN_WIDTH - actual_w) / 2;
		RenderMercPopUpBox(save_load_game_message_box, x, STD_SCREEN_Y + 160, FRAME_BUFFER);
	}

	InvalidateScreen();
	ExecuteBaseDirtyRectQueue();
	RefreshScreen();

	// Make sure we redraw the screen when we are done
	switch (guiCurrentScreen)
	{
		case GAME_SCREEN:
			SetRenderFlags(RENDER_FLAG_FULL);
			break;

		case MAP_SCREEN:
			fMapPanelDirty           = TRUE;
			fTeamPanelDirty          = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			break;

		case SAVE_LOAD_SCREEN:
			gfRedrawSaveLoadScreen = TRUE;
			break;
		default:
			break;
	}

	// Set the fact that we are saving a game
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

	try
	{
		//Save the current sectors open temp files to the disk
		SaveCurrentSectorsInformationToTempItemFile();

		SAVED_GAME_HEADER header;
		header = SAVED_GAME_HEADER{};

		if (ubSaveGameID == 0)
		{ // We are saving the quick save
			header.sSavedGameDesc = g_quicksave_name;
		}
		else if (gameDesc.empty())
		{
			header.sSavedGameDesc = pMessageStrings[MSG_NODESC];
		}
		else
		{
			header.sSavedGameDesc = gameDesc;
		}

		FileMan::createDir(GCM->getSavedGamesFolder().c_str());

		// Create the save game file
		char savegame_name[512];
		CreateSavedGameFileNameFromNumber(ubSaveGameID, savegame_name);
		AutoSGPFile f(FileMan::openForWriting(savegame_name));

		/* If there are no enemy or civilians to save, we have to check BEFORE
		 * saving the sector info struct because the
		 * NewWayOfSavingEnemyAndCivliansToTempFile() will RESET the civ or enemy
		 * flag AFTER they have been saved. */
		NewWayOfSavingEnemyAndCivliansToTempFile(gWorldSectorX, gWorldSectorY, gbWorldSectorZ, TRUE,  TRUE);
		NewWayOfSavingEnemyAndCivliansToTempFile(gWorldSectorX, gWorldSectorY, gbWorldSectorZ, FALSE, TRUE);

		// Setup the save game header
		header.uiSavedGameVersion = guiSavedGameVersion;
		strcpy(header.zGameVersionNumber, g_version_number);

		// The following will be used to quickly access info to display in the save/load screen
		header.uiDay                     = GetWorldDay();
		header.ubHour                    = (UINT8)GetWorldHour();
		header.ubMin                     = (UINT8)guiMin;
		header.sInitialGameOptions       = gGameOptions;
		GetBestPossibleSectorXYZValues(&header.sSectorX, &header.sSectorY, &header.bSectorZ);
		header.ubNumOfMercsOnPlayersTeam = NumberOfMercsOnPlayerTeam();
		header.iCurrentBalance           = LaptopSaveInfo.iCurrentBalance;
		header.uiCurrentScreen           = guiPreviousOptionScreen;
		header.fAlternateSector          = GetSectorFlagStatus(gWorldSectorX, gWorldSectorY, gbWorldSectorZ, SF_USE_ALTERNATE_MAP);

		if (gfWorldLoaded)
		{
			header.fWorldLoaded   = TRUE;
			header.ubLoadScreenID = GetLoadScreenID(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
		}
		else
		{
			header.fWorldLoaded   = FALSE;
			header.ubLoadScreenID = 0;
		}

		header.uiRandom = Random(RAND_MAX);

		// Save the savegame header
		BYTE  data[432];
		DataWriter d{data};
		INJ_U32(   d, header.uiSavedGameVersion)
		INJ_STR(   d, header.zGameVersionNumber, lengthof(header.zGameVersionNumber))
		d.writeUTF16(header.sSavedGameDesc, SIZE_OF_SAVE_GAME_DESC);
		INJ_SKIP(  d, 4)
		INJ_U32(   d, header.uiDay)
		INJ_U8(    d, header.ubHour)
		INJ_U8(    d, header.ubMin)
		INJ_I16(   d, header.sSectorX)
		INJ_I16(   d, header.sSectorY)
		INJ_I8(    d, header.bSectorZ)
		INJ_U8(    d, header.ubNumOfMercsOnPlayersTeam)
		INJ_I32(   d, header.iCurrentBalance)
		INJ_U32(   d, header.uiCurrentScreen)
		INJ_BOOL(  d, header.fAlternateSector)
		INJ_BOOL(  d, header.fWorldLoaded)
		INJ_U8(    d, header.ubLoadScreenID)
		InjectGameOptions(d, header.sInitialGameOptions);
		INJ_SKIP(  d, 1)
		INJ_U32(   d, header.uiRandom)
		INJ_SKIP(  d, 112)
		Assert(d.getConsumed() == lengthof(data));

		FileWrite(f, data, sizeof(data));

		CalcJA2EncryptionSet(header);

		// Save the gTactical Status array, plus the curent sector location
		SaveTacticalStatusToSavedGame(f);

		SaveGameClock(f, fPausedStateBeforeSaving, fLockPauseStateBeforeSaving);

		SaveStrategicEventsToSavedGame(f);

		SaveLaptopInfoToSavedGame(f);

		SaveMercProfiles(f);

		SaveSoldierStructure(f);

		SaveTempFileToSavedGame(NEWTMP_FINANCES_DATA_FILE, f);

		SaveFilesToSavedGame(HISTORY_DATA_FILE, f);

		SaveFilesToSavedGame(FILES_DAT_FILE, f);

		SaveEmailToSavedGame(f);

		SaveStrategicInfoToSavedFile(f);

		SaveUnderGroundSectorInfoToSaveGame(f);

		SaveSquadInfoToSavedGameFile(f);

		SaveStrategicMovementGroupsToSaveGameFile(f);

		SaveMapTempFilesToSavedGameFile(f);

		SaveQuestInfoToSavedGameFile(f);

		SaveOppListInfoToSavedGame(f);

		SaveMapScreenMessagesToSaveGameFile(f);

		SaveNPCInfoToSaveGameFile(f);

		SaveKeyTableToSaveGameFile(f);

		SaveTempNpcQuoteArrayToSaveGameFile(f);

		SavePreRandomNumbersToSaveGameFile(f);

		SaveArmsDealerInventoryToSaveGameFile(f);

		SaveGeneralInfo(f);

		SaveMineStatusToSaveGameFile(f);

		SaveStrategicTownLoyaltyToSaveGameFile(f);

		SaveVehicleInformationToSaveGameFile(f);

		SaveBulletStructureToSaveGameFile(f);

		SavePhysicsTableToSaveGameFile(f);

		SaveAirRaidInfoToSaveGameFile(f);

		SaveTeamTurnsToTheSaveGameFile(f);

		SaveExplosionTableToSaveGameFile(f);

		SaveCreatureDirectives(f);

		SaveStrategicStatusToSaveGameFile(f);

		SaveStrategicAI(f);

		SaveWatchedLocsToSavedGame(f);

		SaveItemCursorToSavedGame(f);

		SaveCivQuotesToSaveGameFile(f);

		SaveBackupNPCInfoToSaveGameFile(f);

		SaveMeanwhileDefsToSaveGameFile(f);

		SaveSchedules(f);

		NewSaveVehicleMovementInfoToSavedGameFile(f);

		SaveContractRenewalDataToSaveGameFile(f);

		SaveLeaveItemList(f);

		NewWayOfSavingBobbyRMailOrdersToSaveGameFile(f);
	}
	catch (...)
	{
		if (fWePausedIt) UnPauseAfterSaveGame();

		// Delete the failed attempt at saving
		DeleteSaveGameNumber(ubSaveGameID);

		//Put out an error message
		ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, zSaveLoadText[SLG_SAVE_GAME_ERROR]);

		NextLoopCheckForEnoughFreeHardDriveSpace();

		gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;
		return FALSE;
	}

	// If we succesfully saved the game, mark this entry as the last saved game file
	if (ubSaveGameID != SAVE__ERROR_NUM && ubSaveGameID != SAVE__END_TURN_NUM)
	{
		gGameSettings.bLastSavedGameSlot = ubSaveGameID;
	}

	SaveGameSettings();

	// Display a screen message that the save was succesful (unless we are in Dead is Dead Mode to prevent message spamming)
	if (ubSaveGameID != SAVE__END_TURN_NUM && gGameOptions.ubGameSaveMode != DIF_DEAD_IS_DEAD)
	{
		ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[MSG_SAVESUCCESS]);
	}

	// Restore the music mode
	SetMusicMode(gubMusicMode);

	// Unset the fact that we are saving a game
	gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

	UnPauseAfterSaveGame();

	NextLoopCheckForEnoughFreeHardDriveSpace();
	return TRUE;
}


/** Parse binary data and fill SAVED_GAME_HEADER structure.
 * @param data Data to be parsed.
 * @param h Header structure to be filled.
 * @param stracLinuxFormat Flag, telling to use "Stracciatella Linux" format. */
void ParseSavedGameHeader(const BYTE *data, SAVED_GAME_HEADER& h, bool stracLinuxFormat)
{
	DataReader d{data};
	EXTR_U32(   d, h.uiSavedGameVersion);
	EXTR_STR(   d, h.zGameVersionNumber, lengthof(h.zGameVersionNumber));
	if(stracLinuxFormat)
	{
		h.sSavedGameDesc = d.readUTF32(SIZE_OF_SAVE_GAME_DESC);
	}
	else
	{
		h.sSavedGameDesc = d.readUTF16(SIZE_OF_SAVE_GAME_DESC);
	}
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
	ExtractGameOptions(d, h.sInitialGameOptions);
	EXTR_SKIP(  d, 1)
	EXTR_U32(   d, h.uiRandom)
	EXTR_SKIP(  d, 112)
	// XXX: this assert doesn't work anymore
	// Assert(d.getConsumed() == lengthof(data));
}

/** @brief Check if SAVED_GAME_HEADER structure contains valid data.
 * This function does only a basic check.  It might not detect all problems. */
bool isValidSavedGameHeader(SAVED_GAME_HEADER& h)
{
	if((h.sSectorX == 0) && (h.sSectorY == 0) && (h.bSectorZ == -1))
	{
		// Special case: sector N/A at the game start
		if((h.uiDay == 0)
			|| (h.iCurrentBalance < 0))
		{
			// invalid for sure
			return false;
		}
	}
	else
	{
		if((h.uiDay == 0)
			|| (h.sSectorX <= 0) || (h.sSectorX > 16)
			|| (h.sSectorY <= 0) || (h.sSectorY > 16)
			|| (h.bSectorZ  < 0) || (h.bSectorZ > 3)
			|| (h.iCurrentBalance < 0))
		{
			// invalid for sure
			return false;
		}
	}
	return true;
}


void ExtractSavedGameHeaderFromFile(HWFILE const f, SAVED_GAME_HEADER& h, bool *stracLinuxFormat)
{
	// first try Strac Linux format
	try
	{
		BYTE data[SAVED_GAME_HEADER_ON_DISK_SIZE_STRAC_LIN];
		FileRead(f, data, sizeof(data));
		ParseSavedGameHeader(data, h, true);
		if(isValidSavedGameHeader(h))
		{
			*stracLinuxFormat = true;
			return;
		}
	}
	catch (...) {}

	{
		// trying vanilla format
		BYTE data[SAVED_GAME_HEADER_ON_DISK_SIZE];
		FileSeek(f, 0, FILE_SEEK_FROM_START);
		FileRead(f, data, sizeof(data));
		ParseSavedGameHeader(data, h, false);
		*stracLinuxFormat = false;
	}
}

static void HandleOldBobbyRMailOrders(void);
static void LoadGeneralInfo(HWFILE, UINT32 savegame_version);
static void LoadMeanwhileDefsFromSaveGameFile(HWFILE, UINT32 savegame_version);
static void LoadOppListInfoFromSavedGame(HWFILE);
static void LoadPreRandomNumbersFromSaveGameFile(HWFILE);
static void LoadSavedMercProfiles(HWFILE, UINT32 savegame_version, bool stracLinuxFormat);
static void LoadSoldierStructure(HWFILE, UINT32 savegame_version, bool stracLinuxFormat);
static void LoadTacticalStatusFromSavedGame(HWFILE, bool stracLinuxFormat);
static void LoadWatchedLocsFromSavedGame(HWFILE);
static void TruncateStrategicGroupSizes(void);
static void UpdateMercMercContractInfo(void);
void InitScriptingEngine();

void LoadSavedGame(UINT8 const save_slot_id)
{
	// Save the game before if we are in Dead is Dead Mode
	if (gGameOptions.ubGameSaveMode == DIF_DEAD_IS_DEAD) {
		// The previous options screen may be the main menu if we use quicksave/load
		if (guiCurrentScreen != SAVE_LOAD_SCREEN)
		{
			guiPreviousOptionScreen = guiCurrentScreen;
		}
		DoDeadIsDeadSave();
	}
	TrashAllSoldiers();
	RemoveAllGroups();

	// Empty the dialogue Queue cause someone could still have a quote in waiting
	EmptyDialogueQueue();

	// If there is someone talking, stop them
	StopAnyCurrentlyTalkingSpeech();

	ZeroAnimSurfaceCounts();

	ShutdownNPCQuotes();

	//Used in mapescreen to disable the annoying 'swoosh' transitions
	gfDontStartTransitionFromLaptop = TRUE;

	// Reset timer callbacks
	gpCustomizableTimerCallback = NULL;

	//Set the fact that we are loading a saved game
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

	/* Trash the existing world.  This is done to ensure that if we are loading a
	 * game that doesn't have a world loaded, that we trash it beforehand -- else
	 * the player could theoretically enter that sector where it would be in a
	 * pre-load state. */
	TrashWorld();

	InitScriptingEngine();

	InitTacticalSave();

	// ATE: Added to empty dialogue q
	EmptyDialogueQueue();

	char zSaveGameName[512];
	CreateSavedGameFileNameFromNumber(save_slot_id, zSaveGameName);
	AutoSGPFile f(GCM->openUserPrivateFileForReading(zSaveGameName));

	SAVED_GAME_HEADER SaveGameHeader;
	bool stracLinuxFormat;
	ExtractSavedGameHeaderFromFile(f, SaveGameHeader, &stracLinuxFormat);

	CalcJA2EncryptionSet(SaveGameHeader);

	UINT32 const version = SaveGameHeader.uiSavedGameVersion;
	// Load the savegame name, only relevant for Dead is Dead games
	gGameSettings.sCurrentSavedGameName = SaveGameHeader.sSavedGameDesc;

	/* If the player is loading up an older version of the game and the person
	 * DOESN'T have the cheats on. */
	if (version < 65 && !CHEATER_CHEAT_LEVEL()) throw std::runtime_error("Savegame too old");

	//Store the loading screenID that was saved
	gubLastLoadingScreenID = static_cast<LoadingScreenID>(SaveGameHeader.ubLoadScreenID);

#if 0 // XXX was commented out
	LoadGeneralInfo(f, version);
#endif

	//Load the gtactical status structure plus the current sector x,y,z
	LoadTacticalStatusFromSavedGame(f, stracLinuxFormat);

	//This gets reset by the above function
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;


	//Load the game clock ingo
	LoadGameClock(f);

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
		SetWorldSectorInvalid();

		//if we should load a sector (if the person didnt just start the game game)
		if (sLoadSectorX != 0 && sLoadSectorY != 0)
		{
			//Load the sector
			SetCurrentWorldSector(sLoadSectorX, sLoadSectorY, bLoadSectorZ);
		}
	}
	else
	{ //By clearing these values, we can avoid "in sector" checks -- at least, that's the theory.
		SetWorldSectorInvalid();

		INT16 const x = SaveGameHeader.sSectorX;
		INT16 const y = SaveGameHeader.sSectorY;
		INT8  const z = SaveGameHeader.bSectorZ;
		gubLastLoadingScreenID = x == -1 || y == -1 || z == -1 ?
			LOADINGSCREEN_HELI : GetLoadScreenID(x, y, z);

		BeginLoadScreen();
	}

	CreateLoadingScreenProgressBar();
	SetProgressBarColor(0, 0, 0, 150);

	UINT32 uiRelStartPerc = 0;
	UINT32 uiRelEndPerc   = 0;

#define BAR(delta, text)                                                     \
	(uiRelEndPerc += (delta),                                                  \
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, (text)), \
	RenderProgressBar(0, 100),                                                 \
	uiRelStartPerc = uiRelEndPerc)                                             \

	BAR(1, "Strategic Events...");
	LoadStrategicEventsFromSavedGame(f);

	BAR(0, "Laptop Info");
	LoadLaptopInfoFromSavedGame(f);

	BAR(0, "Merc Profiles...");
	LoadSavedMercProfiles(f, version, stracLinuxFormat);

	BAR(30, "Soldier Structure...");
	LoadSoldierStructure(f, version, stracLinuxFormat);

	BAR(1, "Finances Data File...");
	LoadTempFileFromSavedGame(NEWTMP_FINANCES_DATA_FILE, f);

	BAR(1, "History File...");
	LoadFilesFromSavedGame(HISTORY_DATA_FILE, f);

	BAR(1, "The Laptop FILES file...");
	LoadFilesFromSavedGame(FILES_DAT_FILE, f);

	BAR(1, "Email...");
	LoadEmailFromSavedGame(f);

	BAR(1, "Strategic Information...");
	LoadStrategicInfoFromSavedFile(f);

	BAR(1, "UnderGround Information...");
	LoadUnderGroundSectorInfoFromSavedGame(f);

	BAR(1, "Squad Info...");
	LoadSquadInfoFromSavedGameFile(f);

	BAR(1, "Strategic Movement Groups...");
	LoadStrategicMovementGroupsFromSavedGameFile(f);

	BAR(30, "All the Map Temp files...");
	LoadMapTempFilesFromSavedGameFile(f, version);

	BAR(1, "Quest Info...");
	LoadQuestInfoFromSavedGameFile(f);

	BAR(1, "OppList Info...");
	LoadOppListInfoFromSavedGame(f);

	BAR(1, "MapScreen Messages...");
	LoadMapScreenMessagesFromSaveGameFile(f, stracLinuxFormat);

	BAR(1, "NPC Info...");
	LoadNPCInfoFromSavedGameFile(f, version);

	BAR(1, "KeyTable...");
	LoadKeyTableFromSaveedGameFile(f);

	BAR(1, "Npc Temp Quote File...");
	LoadTempNpcQuoteArrayToSaveGameFile(f);

	BAR(0, "PreGenerated Random Files...");
	LoadPreRandomNumbersFromSaveGameFile(f);

	BAR(0, "Smoke Effect Structures...");
	// No longer need to load smoke effects.  They are now in temp files
	if (version < 75) LoadSmokeEffectsFromLoadGameFile(f, version);

	BAR(1, "Arms Dealers Inventory...");
	LoadArmsDealerInventoryFromSavedGameFile(f, version);

	BAR(0, "Misc info...");
	LoadGeneralInfo(f, version);

	BAR(1, "Mine Status...");
	LoadMineStatusFromSavedGameFile(f);

	BAR(0, "Town Loyalty...");
	if (version	>= 21)
	{
		LoadStrategicTownLoyaltyFromSavedGameFile(f);
	}

	BAR(1, "Vehicle Information...");
	if (version	>= 22)
	{
		LoadVehicleInformationFromSavedGameFile(f, version);
	}

	BAR(1, "Bullet Information...");
	if (version	>= 24)
	{
		LoadBulletStructureFromSavedGameFile(f);
	}

	BAR(1, "Physics table...");
	if (version	>= 24)
	{
		LoadPhysicsTableFromSavedGameFile(f);
	}

	BAR(1, "Air Raid Info...");
	if (version	>= 24)
	{
		LoadAirRaidInfoFromSaveGameFile(f);
	}

	BAR(0, "Team Turn Info...");
	if (version	>= 24)
	{
		LoadTeamTurnsFromTheSavedGameFile(f);
	}

	BAR(1, "Explosion Table...");
	if (version	>= 25)
	{
		LoadExplosionTableFromSavedGameFile(f);
	}

	BAR(1, "Creature Spreading...");
	if (version	>= 27)
	{
		LoadCreatureDirectives(f, version);
	}

	BAR(1, "Strategic Status...");
	if (version	>= 28)
	{
		LoadStrategicStatusFromSaveGameFile(f);
	}

	BAR(1, "Strategic AI...");
	if (version	>= 31)
	{
		LoadStrategicAI(f);
	}

	BAR(1, "Lighting Effects...");
	// No longer need to load Light effects.  They are now in temp files
	if (37 <= version && version < 76)
	{
		LoadLightEffectsFromLoadGameFile(f);
	}

	BAR(1, "Watched Locs Info...");
	if (version	>= 38)
	{
		LoadWatchedLocsFromSavedGame(f);
	}

	BAR(1, "Item cursor Info...");
	if (version	>= 39)
	{
		LoadItemCursorFromSavedGame(f);
	}

	BAR(1, "Civ Quote System...");
	if (version >= 51)
	{
		LoadCivQuotesFromLoadGameFile(f);
	}

	BAR(1, "Backed up NPC Info...");
	if (version >= 53)
	{
		LoadBackupNPCInfoFromSavedGameFile(f);
	}

	BAR(1, "Meanwhile definitions...");
	if (version >= 58)
	{
		LoadMeanwhileDefsFromSaveGameFile(f, version);
	}
	else
	{
		gMeanwhileDef[gCurrentMeanwhileDef.ubMeanwhileID] = gCurrentMeanwhileDef;
	}

	BAR(1, "Schedules...");
	if (version >= 59)
	{
		// trash schedules loaded from map
		DestroyAllSchedulesWithoutDestroyingEvents();
		LoadSchedulesFromSave(f);
	}

	BAR(1, "Extra Vehicle Info...");
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
	}

	BAR(1, "Contract renweal sequence stuff...");
	if (version < 62)
	{ // The older games had a bug where this flag was never being set
		gfResetAllPlayerKnowsEnemiesFlags = TRUE;
	}

	if (version >= 67)
	{
		LoadContractRenewalDataFromSaveGameFile(f);
	}

	if (version >= 70)
	{
		LoadLeaveItemList(f);
	}

	if (version <= 73)
	{ // Patch vehicle fuel
		AddVehicleFuelToSave();
	}

	if (version >= 85)
	{
		NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(f);
	}

	// If there are any old Bobby R Mail orders, tranfer them to the new system
	if (version < 85)
	{
		HandleOldBobbyRMailOrders();
	}

	BAR(1, "Final Checks...");

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

	BAR(1, "Final Checks...");

	InitAI();

	// Update the mercs in the sector with the new soldier info
	UpdateMercsInSector();

	PostSchedules();

	BAR(1, "Final Checks...");

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

	BAR(1, "Final Checks...");

	// if the UI was locked in the saved game file
	if (gTacticalStatus.ubAttackBusyCount > 1)
	{ // Lock the ui
		SetUIBusy(GetSelectedMan());
	}

	// if we succesfully LOADED! the game, mark this entry as the last saved game file
	gGameSettings.bLastSavedGameSlot = save_slot_id;

	//Save the save game settings
	SaveGameSettings();

	BAR(1, "Final Checks...");

	// Reset the AI Timer clock
	giRTAILastUpdateTime = 0;

	// If we are in tactical
	if (guiScreenToGotoAfterLoadingSavedGame == GAME_SCREEN)
	{ //Initialize the current panel
		InitializeCurrentPanel();
		SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel) SelectSoldier(sel, SELSOLDIER_FORCE_RESELECT);
	}

	BAR(1, "Final Checks...");

#undef BAR

	InitalizeStaticExternalNPCFaces();
	LoadCarPortraitValues();

	// OK, turn OFF show all enemies....
	gTacticalStatus.uiFlags &= ~(SHOW_ALL_MERCS | SHOW_ALL_ITEMS);

	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		SLOGD("Setting attack busy count to 0 from load");
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
		if (s) TacticalRemoveSoldier(*s);

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
		FOR_EACH_SOLDIER(s) s->bVehicleUnderRepairID = -1;
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
	SetRelativeStartAndEndPercentage(0, uiRelStartPerc, uiRelEndPerc, "Done!");
	RenderProgressBar(0, 100);

	RemoveLoadingScreenProgressBar();

	SetMusicMode(gMusicModeToPlay);

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

	// on loading a gamestate or getting ambushed from the StrategicMap
	// We have to call this right after loading a saved game for several reasons:
	// 1. the gubEnemyEncounterCode may be analyzed later so we cant check the
	// 		ambush code properly
	// 2. the ai may ignoring the CallAvailableEnemiesTo(...) because it wasn't
	//		fully analyzed before
	CallAvailableTeamEnemiesToAmbush(gMapInformation.sCenterGridNo);
}


static void SaveMercProfiles(HWFILE const f)
{
	// Loop through all the profiles to save
	void (&writer)(HWFILE, BYTE const*, UINT32) = guiSavedGameVersion < 87 ?
		JA2EncryptedFileWrite : NewJA2EncryptedFileWrite;
	FOR_EACH(MERCPROFILESTRUCT, i, gMercProfiles)
	{
		BYTE data[716];
		InjectMercProfile(data, *i);
		writer(f, data, sizeof(data));
	}
}

ST::string IMPSavedProfileCreateFilename(ST::string nickname)
{
	return GCM->getSavedGamesFolder() + "/mercprofile." + nickname;;
}

bool IMPSavedProfileDoesFileExist(ST::string nickname)
{
	ST::string profile_filename = IMPSavedProfileCreateFilename(nickname);
	bool fexists = Fs_exists(profile_filename.c_str());
	return fexists;
}

SGPFile* const IMPSavedProfileOpenFileForRead(ST::string nickname)
{
	if (!IMPSavedProfileDoesFileExist(nickname)) {
		throw std::runtime_error(FormattedString("Lost IMP with nickname '%s'!", nickname.c_str()).to_std_string());
	}
	SGPFile *f = FileMan::openForReading(IMPSavedProfileCreateFilename(nickname).c_str());
	return f;
}

SGPFile* const IMPSavedProfileOpenFileForWrite(ST::string nickname)
{
	ST::string profile_filename = IMPSavedProfileCreateFilename(nickname);
	SGPFile *f = FileMan::openForWriting(profile_filename.c_str(), true);
	return f;
}

int IMPSavedProfileLoadMercProfile(ST::string nickname)
{
	if (!IMPSavedProfileDoesFileExist(nickname)) {
		throw std::runtime_error(FormattedString("Lost IMP with nickname '%s'!", nickname.c_str()).to_std_string());
	}
	SGPFile *f = IMPSavedProfileOpenFileForRead(nickname);
	MERCPROFILESTRUCT profile_saved;
	FileRead(f, &profile_saved, sizeof(MERCPROFILESTRUCT));
	FileClose(f);
	int voiceid = profile_saved.ubSuspiciousDeath;
	MERCPROFILESTRUCT& profile_new = gMercProfiles[PLAYER_GENERATED_CHARACTER_ID + voiceid];
	profile_new = profile_saved;
	profile_new.bMercStatus = MERC_OK;
	return voiceid;
}

void IMPSavedProfileLoadInventory(ST::string nickname, SOLDIERTYPE *pSoldier)
{
	if (!IMPSavedProfileDoesFileExist(nickname)) return;
	if (!pSoldier) return;

	SGPFile *f = IMPSavedProfileOpenFileForRead(nickname);
	FileSeek(f, sizeof(MERCPROFILESTRUCT), FILE_SEEK_FROM_START);
	FileRead(f, pSoldier->inv, sizeof(OBJECTTYPE) * NUM_INV_SLOTS);
	FileClose(f);
}

void SaveIMPPlayerProfiles()
{
	for (int i = PLAYER_GENERATED_CHARACTER_ID; i <= LAST_IMP_MERC; i++)
	{
		MERCPROFILESTRUCT* const mercprofile = &gMercProfiles[i];
		if (mercprofile->bLife == 0) continue;
		SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(i);
		if (!pSoldier) continue;
		if (pSoldier->bTeam != OUR_TEAM) continue;
		if (pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__PLAYER_CHARACTER) continue;

		SGPFile *f = IMPSavedProfileOpenFileForWrite(mercprofile->zNickname.c_str());
		if (!f) continue;

		mercprofile->ubSuspiciousDeath = i - PLAYER_GENERATED_CHARACTER_ID; // save voice_id, field not used for resuscitated merc
		FileWrite(f, mercprofile, sizeof(MERCPROFILESTRUCT));
		FileWrite(f, pSoldier->inv, sizeof(OBJECTTYPE) * NUM_INV_SLOTS);
		FileClose(f);
	}
}

static void LoadSavedMercProfiles(HWFILE const f, UINT32 const savegame_version, bool stracLinuxFormat)
{
	// Loop through all the profiles to load
	void (&reader)(HWFILE, BYTE*, UINT32) = savegame_version < 87 ?
		JA2EncryptedFileRead : NewJA2EncryptedFileRead;
	FOR_EACH(MERCPROFILESTRUCT, i, gMercProfiles)
	{
		UINT32 checksum;
		UINT32 dataSize = stracLinuxFormat ? MERC_PROFILE_SIZE_STRAC_LINUX : MERC_PROFILE_SIZE;
		std::vector<BYTE> data(dataSize);
		reader(f, data.data(), dataSize);
		ExtractMercProfile(data.data(), *i, stracLinuxFormat, &checksum, NULL);
		if (checksum != SoldierProfileChecksum(*i))
		{
			throw std::runtime_error("Merc profile checksum mismatch");
		}
	}
}


static void SaveSoldierStructure(HWFILE const f)
{
	// Loop through all the soldier structs to save
	void (&writer)(HWFILE, BYTE const*, UINT32) = guiSavedGameVersion < 87 ?
		JA2EncryptedFileWrite : NewJA2EncryptedFileWrite;
	for (UINT16 i = 0; i < TOTAL_SOLDIERS; ++i)
	{
		SOLDIERTYPE const& s = GetMan(i);

		// If the soldier isn't active, don't add them to the saved game file.
		FileWrite(f, &s.bActive, 1);
		if (!s.bActive) continue;

		// Save the soldier structure
		BYTE data[2328];
		std::fill_n(data, 2328, 0);
		InjectSoldierType(data, &s);
		writer(f, data, sizeof(data));

		// Save all the pointer info from the structure
		SaveMercPath(f, s.pMercPath);

		// Save the key ring
		UINT8 const has_keyring = s.pKeyRing != 0;
		FileWrite(f, &has_keyring, sizeof(has_keyring));
		if (!has_keyring) continue;
		FileWrite(f, s.pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING));
	}
}


static void LoadSoldierStructure(HWFILE const f, UINT32 savegame_version, bool stracLinuxFormat)
{
	// Loop through all the soldier and delete them all
	FOR_EACH_SOLDIER(i) TacticalRemoveSoldier(*i);

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

		//Read in the saved soldier info into a Temp structure
		SOLDIERTYPE SavedSoldierInfo;
		if(stracLinuxFormat)
		{
			BYTE Data[2352];
			reader(f, Data, sizeof(Data));
			ExtractSoldierType(Data, &SavedSoldierInfo, stracLinuxFormat, savegame_version);
		}
		else
		{
			BYTE Data[2328];
			reader(f, Data, sizeof(Data));
			ExtractSoldierType(Data, &SavedSoldierInfo, stracLinuxFormat, savegame_version);
		}

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
		if (s->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER && s->bTeam == OUR_TEAM)
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

		if(isGermanVersion())
		{
			// Fix neutral flags
			if (savegame_version < 94 &&
					s->bTeam == OUR_TEAM  &&
					s->bNeutral           &&
					s->bAssignment != ASSIGNMENT_POW)
			{
				// turn off neutral flag
				s->bNeutral = FALSE;
			}
		}
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


void BackupSavedGame(UINT8 const ubSaveGameID)
{
	ST::string backupdir = FileMan::joinPaths(GCM->getSavedGamesFolder(), "Backup");
	FileMan::createDir(backupdir.c_str());
	char zSourceSaveGameName[512];
	char zSourceBackupSaveGameName[515];
	char zTargetSaveGameName[515];
	sprintf(zSourceSaveGameName, "%s%02d.%s", g_savegame_name, ubSaveGameID, g_savegame_ext);
	for (int i = NUM_SAVE_GAME_BACKUPS - 1; i >= 0; i--)
	{
		if (i==0)
		{
			strcpy(zSourceBackupSaveGameName, zSourceSaveGameName);
		}
		else
		{
			sprintf(zSourceBackupSaveGameName, "%s.%01d", zSourceSaveGameName, i);
		}
		sprintf(zTargetSaveGameName, "%s.%01d", zSourceSaveGameName, i+1);
		// Only backup existing savegames
		if (FileMan::checkFileExistance(i==0 ? GCM->getSavedGamesFolder() : backupdir, zSourceBackupSaveGameName))
		{
			FileMan::moveFile(FileMan::joinPaths(i==0 ? GCM->getSavedGamesFolder() : backupdir, zSourceBackupSaveGameName).c_str(),
												FileMan::joinPaths(backupdir,zTargetSaveGameName).c_str());
		}
	}
}

static void SaveFileToSavedGame(SGPFile* fileToSave, HWFILE const hFile)
{
	//Get the file size of the source data file
	UINT32 uiFileSize = FileGetSize( fileToSave );

	// Write the the size of the file to the saved game file
	FileWrite(hFile, &uiFileSize, sizeof(UINT32));

	if (uiFileSize == 0) return;

	// Read the saource file into the buffer
	SGP::Buffer<UINT8> pData(uiFileSize);
	FileRead(fileToSave, pData, uiFileSize);

	// Write the buffer to the saved game file
	FileWrite(hFile, pData, uiFileSize);
}

static void SaveTempFileToSavedGame(const char* fileName, HWFILE const hFile)
{
	AutoSGPFile fileToSave(GCM->openTempFileForReading(fileName));
	SaveFileToSavedGame(fileToSave, hFile);
}

void SaveFilesToSavedGame(char const* const pSrcFileName, HWFILE const hFile)
{
	AutoSGPFile hSrcFile(GCM->openGameResForReading(pSrcFileName));
	SaveFileToSavedGame(hSrcFile, hFile);
}


static void LoadFileFromSavedGame(SGPFile* fileToWrite, HWFILE const hFile)
{
	// Read the size of the data
	UINT32 uiFileSize;
	FileRead(hFile, &uiFileSize, sizeof(UINT32));

	if (uiFileSize == 0) return;

	// Read into the buffer
	SGP::Buffer<UINT8> pData(uiFileSize);
	FileRead(hFile, pData, uiFileSize);

	// Write the buffer to the new file
	FileWrite(fileToWrite, pData, uiFileSize);
}

void LoadTempFileFromSavedGame(const char* tempFileName, HWFILE const hFile)
{
	AutoSGPFile fileToWrite(GCM->openTempFileForWriting(tempFileName, true));
	LoadFileFromSavedGame(fileToWrite, hFile);
}

void LoadFilesFromSavedGame(char const* const pSrcFileName, HWFILE const hFile)
{
	AutoSGPFile hSrcFile(FileMan::openForWriting(pSrcFileName));
	LoadFileFromSavedGame(hSrcFile, hFile);
}

static void SaveTacticalStatusToSavedGame(HWFILE const f)
{
	InjectTacticalStatusTypeIntoFile(f);

	// Save the current sector location
	BYTE  data[5];
	DataWriter d{data};
	INJ_I16(d, gWorldSectorX)
	INJ_I16(d, gWorldSectorY)
	INJ_I8( d, gbWorldSectorZ)
	Assert(d.getConsumed() == lengthof(data));

	FileWrite(f, data, sizeof(data));
}


static void LoadTacticalStatusFromSavedGame(HWFILE const f, bool stracLinuxFormat)
{
	ExtractTacticalStatusTypeFromFile(f, stracLinuxFormat);

	// Load the current sector location
	BYTE data[5];
	FileRead(f, data, sizeof(data));

	DataReader d{data};
	EXTR_I16(d, gWorldSectorX)
	EXTR_I16(d, gWorldSectorY)
	EXTR_I8( d, gbWorldSectorZ)
	Assert(d.getConsumed() == lengthof(data));
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
	ST::string dir = GCM->getSavedGamesFolder();
	char const* const ext = g_savegame_ext;

	switch (ubSaveGameID)
	{
		case 0: // we are creating the QuickSave file
		{
			char const* const quick = g_quicksave_name;
			{
				sprintf(pzNewFileName, "%s/%s.%s", dir.c_str(), quick, ext);
			}
			break;
		}

		case SAVE__END_TURN_NUM:
			sprintf(pzNewFileName, "%s/Auto%02d.%s", dir.c_str(), guiLastSaveGameNum, ext);
			guiLastSaveGameNum = (guiLastSaveGameNum + 1) % 2;
			break;

		case SAVE__ERROR_NUM:
			sprintf(pzNewFileName, "%s/error.%s", dir.c_str(), ext);
			break;

		default:
			sprintf(pzNewFileName, "%s/%s%02d.%s", dir.c_str(), g_savegame_name, ubSaveGameID, ext);
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
		DataWriter d{data};
		INJ_U32(d, p->uiSectorId)
		INJ_SKIP(d, 16)
		Assert(d.getConsumed() == lengthof(data));

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
		PathSt* const n = new PathSt{};

		BYTE data[20];
		FileRead(hFile, data, sizeof(data));

		DataReader d{data};
		EXTR_U32(d, n->uiSectorId)
		EXTR_SKIP(d, 16)
		Assert(d.getConsumed() == lengthof(data));

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


static void InjectMeanwhileDefinition(DataWriter& d, MEANWHILE_DEFINITION const& m)
{
	size_t start = d.getConsumed();
	INJ_I16(d, m.sSectorX)
	INJ_I16(d, m.sSectorY)
	INJ_U16(d, m.usTriggerEvent)
	INJ_U8( d, m.ubMeanwhileID)
	INJ_U8( d, m.ubNPCNumber)
	Assert(d.getConsumed() == start + 8);
}


static void ExtractMeanwhileDefinition(DataReader& d, MEANWHILE_DEFINITION& m)
{
	size_t start = d.getConsumed();
	EXTR_I16(d, m.sSectorX)
	EXTR_I16(d, m.sSectorY)
	EXTR_U16(d, m.usTriggerEvent)
	EXTR_U8( d, m.ubMeanwhileID)
	EXTR_U8( d, m.ubNPCNumber)
	Assert(d.getConsumed() == start + 8);
}


static void SaveGeneralInfo(HWFILE const f)
{
	BYTE  data[1024];
	DataWriter d{data};
	INJ_U32(  d, guiPreviousOptionScreen)
	INJ_U32(  d, guiCurrentUniqueSoldierId)
	INJ_U8(   d, (UINT8)gubMusicMode)
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
	InjectGameOptions(d, gGameOptions);
	INJ_SKIP( d, 4)
	INJ_U32(  d, guiBaseJA2Clock)
	INJ_I16(  d, gsCurInterfacePanel)
	INJ_U8(   d, gpSMCurrentMerc ? gpSMCurrentMerc->ubID : 255)
	INJ_SKIP( d, 1)
	INJ_BOOL( d, fDisableDueToBattleRoster)
	INJ_BOOL( d, fDisableMapInterfaceDueToBattle)
	INJ_I16A( d, gsBoxerGridNo, lengthof(gsBoxerGridNo))
	FOR_EACH(SOLDIERTYPE* const, i, gBoxer)
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
	InjectMeanwhileDefinition(d, gCurrentMeanwhileDef);
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
	INJ_I16(  d, SECTORX(g_merc_arrive_sector))
	INJ_I16(  d, SECTORY(g_merc_arrive_sector))
	INJ_BOOL( d, gfCreatureMeanwhileScenePlayed)
	INJ_SKIP_U8(d)
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
	Assert(d.getConsumed() == lengthof(data));

	FileWrite(f, data, sizeof(data));
}


static void LoadGeneralInfo(HWFILE const f, UINT32 const savegame_version)
{
	BYTE data[1024];
	FileRead(f, data, sizeof(data));
	UINT8 ubMusicModeToPlay = 0;

	DataReader d{data};
	UINT32 screen_after_loading;
	EXTR_U32(  d, screen_after_loading)
	guiScreenToGotoAfterLoadingSavedGame = static_cast<ScreenID>(screen_after_loading); // XXX TODO001A unchecked conversion
	EXTR_U32(  d, guiCurrentUniqueSoldierId)
	EXTR_U8(   d, ubMusicModeToPlay)
	gMusicModeToPlay = (MusicMode)ubMusicModeToPlay;
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
	pContractReHireSoldier = contract_rehire_soldier != -1 ? &GetMan(contract_rehire_soldier) : 0;
	ExtractGameOptions(d, gGameOptions);
	EXTR_SKIP( d, 4)
	EXTR_U32(  d, guiBaseJA2Clock)
	ResetJA2ClockGlobalTimers();
	INT16 cur_interface_panel;
	EXTR_I16(  d, cur_interface_panel)
	UINT8 sm_current_merc;
	EXTR_U8(   d, sm_current_merc)
	gpSMCurrentMerc = sm_current_merc != 255 ? &GetMan(sm_current_merc) : 0;
	// Set the interface panel to the team panel
	ShutdownCurrentPanel();
	gsCurInterfacePanel = static_cast<InterfacePanelKind>(cur_interface_panel); // XXX TODO001A unchecked conversion
	EXTR_SKIP( d, 1)
	EXTR_BOOL( d, fDisableDueToBattleRoster)
	EXTR_BOOL( d, fDisableMapInterfaceDueToBattle)
	EXTR_I16A( d, gsBoxerGridNo, lengthof(gsBoxerGridNo))
	FOR_EACH(SOLDIERTYPE*, i, gBoxer)
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
	ExtractMeanwhileDefinition(d, gCurrentMeanwhileDef);
	EXTR_BOOL( d, gubPlayerProgressSkyriderLastCommentedOn)
	EXTR_BOOL( d, gfMeanwhileTryingToStart)
	EXTR_BOOL( d, gfInMeanwhile)
	// Always set gfInMeanwhile to false for Dead is Dead. This must be done because it is saved as true if a Meanwhile event is in the event pipe
	// Preventing the value to be saved in the first place leads to odd behaviour during the commencing cutscene
	if (gGameOptions.ubGameSaveMode == DIF_DEAD_IS_DEAD) gfInMeanwhile = FALSE;
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
	INT16 merc_arrive_x;
	INT16 merc_arrive_y;
	EXTR_I16(  d, merc_arrive_x)
	EXTR_I16(  d, merc_arrive_y)
	g_merc_arrive_sector =
		1 <= merc_arrive_x && merc_arrive_x <= 16 &&
		1 <= merc_arrive_y && merc_arrive_y <= 16 ? SECTOR(merc_arrive_x, merc_arrive_y) :
		START_SECTOR;
	EXTR_BOOL( d, gfCreatureMeanwhileScenePlayed)
	EXTR_SKIP_U8(d)
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
	Assert(d.getConsumed() == lengthof(data));
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
		gMeanwhileDef[NUM_MEANWHILES - 1] = MEANWHILE_DEFINITION{};
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
		DataReader d{data};
		ExtractMeanwhileDefinition(d, *i);
		Assert(d.getConsumed() == lengthof(data));
	}
}


static void SaveMeanwhileDefsToSaveGameFile(HWFILE const f)
{
	FOR_EACH(MEANWHILE_DEFINITION, i, gMeanwhileDef)
	{
		BYTE data[8];
		DataWriter d{data};
		InjectMeanwhileDefinition(d, *i);
		FileWrite(f, data, sizeof(data));
	}
}


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
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
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
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
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
	FOR_EACH_ENEMY_GROUP(pGroup)
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
	double	LastWriteTime1;
	double	LastWriteTime2;

	fFile1Exist = FALSE;
	fFile2Exist = FALSE;

	//The name of the file
	char zFileName1[256];
	sprintf(zFileName1, "%s/Auto%02d.%s", GCM->getSavedGamesFolder().c_str(), 0, g_savegame_ext);
	char zFileName2[256];
	sprintf(zFileName2, "%s/Auto%02d.%s", GCM->getSavedGamesFolder().c_str(), 1, g_savegame_ext);

	if( GCM->doesGameResExists( zFileName1 ) )
	{
		Fs_modifiedSecs( zFileName1, &LastWriteTime1 );
		fFile1Exist = TRUE;
	}

	if( GCM->doesGameResExists( zFileName2 ) )
	{
		Fs_modifiedSecs( zFileName2, &LastWriteTime2 );
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
		if( LastWriteTime1 > LastWriteTime2 )
			return( 0 );
		else
			return( 1 );
	}
}


static void HandleOldBobbyRMailOrders(void)
{
	INT32	iNewListCnt=0;

	if( LaptopSaveInfo.usNumberOfBobbyRayOrderUsed != 0 )
	{
		gpNewBobbyrShipments.assign(LaptopSaveInfo.usNumberOfBobbyRayOrderUsed, NewBobbyRayOrderStruct{});

		//loop through and add the old items to the new list
		for (size_t iCnt = 0; iCnt < LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.size(); iCnt++)
		{
			//if this slot is used
			if( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].fActive )
			{
				//copy over the purchase info
				std::copy_n(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[iCnt].BobbyRayPurchase,
					MAX_PURCHASE_AMOUNT,
					gpNewBobbyrShipments[ iNewListCnt ].BobbyRayPurchase);

				gpNewBobbyrShipments[ iNewListCnt ].fActive = TRUE;
				gpNewBobbyrShipments[ iNewListCnt ].ubDeliveryLoc = GCM->getPrimaryShippingDestination()->locationId;
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
		LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.clear();
	}
}


static void CalcJA2EncryptionSet(SAVED_GAME_HEADER const& h)
{
	UINT32 set;

	set  = h.iCurrentBalance;
	set *= h.ubNumOfMercsOnPlayersTeam + 1;
	set += h.bSectorZ * 3;
	set += h.ubLoadScreenID;

	if (h.fAlternateSector) set += 7;

	UINT32 const r = h.uiRandom;
	if (r % 2 == 0)
	{
		set++;
		if (r % 7 == 0)
		{
			set++;
			if (r % 23 == 0) set++;
			if (r % 79 == 0) set += 2;
		}
	}

	if(isGermanVersion())
	{
		set *= 11;
	}

	set %= 10;
	set += h.uiDay / 10;
	set %= BASE_NUMBER_OF_ROTATION_ARRAYS;

	// now pick a different set of #s depending on what game options we've chosen
	GAME_OPTIONS const& o = h.sInitialGameOptions;
	if (o.fGunNut) set += BASE_NUMBER_OF_ROTATION_ARRAYS * 6;
	if (o.fSciFi)  set += BASE_NUMBER_OF_ROTATION_ARRAYS * 3;
	switch (o.ubDifficultyLevel)
	{
		case DIF_LEVEL_EASY:   set += 0;                                  break;
		case DIF_LEVEL_MEDIUM: set += BASE_NUMBER_OF_ROTATION_ARRAYS;     break;
		case DIF_LEVEL_HARD:   set += BASE_NUMBER_OF_ROTATION_ARRAYS * 2; break;
	}

	Assert(set < BASE_NUMBER_OF_ROTATION_ARRAYS * 12);
	guiJA2EncryptionSet = set;
}
