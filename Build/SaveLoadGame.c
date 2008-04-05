#include "Font.h"
#include "Font_Control.h"
#include "LoadSaveData.h"
#include "LoadSaveEMail.h"
#include "LoadSaveMercProfile.h"
#include "LoadSaveSoldierType.h"
#include "LoadSaveTacticalStatusType.h"
#include "Local.h"
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
#include "ScreenIDs.h"
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


extern void NextLoopCheckForEnoughFreeHardDriveSpace();
extern void UpdatePersistantGroupsFromOldSave( UINT32 uiSavedGameVersion );
extern void TrashAllSoldiers( );
extern void ResetJA2ClockGlobalTimers( void );

extern void BeginLoadScreen();

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
extern		BOOLEAN				gfGamePaused;
extern		BOOLEAN				gfLockPauseState;
extern		BOOLEAN				gfLoadedGame;
extern		UINT8					gubDesertTemperature;
extern		UINT8					gubGlobalTemperature;
extern		BOOLEAN				gfCreatureMeanwhileScenePlayed;
#ifdef JA2BETAVERSION
	extern		UINT8					gubReportMapscreenLock;
#endif

static BOOLEAN gMusicModeToPlay = FALSE;

BOOLEAN	gfUseConsecutiveQuickSaveSlots = FALSE;
static UINT32 guiCurrentQuickSaveNumber = 0;
UINT32	guiLastSaveGameNum;

UINT32	guiJA2EncryptionSet = 0;

typedef struct
{
	//The screen that the gaem was saved from
	UINT32	uiCurrentScreen;

	UINT32	uiCurrentUniqueSoldierId;

	//The music that was playing when the game was saved
	UINT8		ubMusicMode;

	//Flag indicating that we have purchased something from Tony
	BOOLEAN	fHavePurchasedItemsFromTony_UNUSED; // XXX HACK000B

	//The selected soldier in tactical
	UINT16	usSelectedSoldier;

	// The x and y scroll position
	INT16		sRenderCenterX;
	INT16		sRenderCenterY;

	BOOLEAN	fAtLeastOneMercWasHired;

	//General Map screen state flags
	BOOLEAN	fShowItemsFlag;
	BOOLEAN	fShowTownFlag;
	BOOLEAN	fShowTeamFlag;
	BOOLEAN	fShowMineFlag;
	BOOLEAN	fShowAircraftFlag;

	// is the helicopter available to player?
	BOOLEAN fHelicopterAvailable;

	// helicopter vehicle id
	INT32 iHelicopterVehicleId;

	// total distance travelled
	INT32 UNUSEDiTotalHeliDistanceSinceRefuel; // XXX HACK000B

	// total owed to player
	INT32 iTotalAccumulatedCostByPlayer;

	// whether or not skyrider is alive and well? and on our side yet?
	BOOLEAN fSkyRiderAvailable;

	// skyrider engaging in a monologue
	BOOLEAN UNUSEDfSkyriderMonologue;

	// list of sector locations
	INT16 UNUSED[ 2 ][ 2 ];

	// is the heli in the air?
	BOOLEAN fHelicopterIsAirBorne;

	// is the pilot returning straight to base?
	BOOLEAN fHeliReturnStraightToBase;

	// heli hovering
	BOOLEAN fHoveringHelicopter;

	// time started hovering
	UINT32 uiStartHoverTime;

	// what state is skyrider's dialogue in in?
	UINT32 uiHelicopterSkyriderTalkState;

	// the flags for skyrider events
	BOOLEAN fShowEstoniRefuelHighLight;
	BOOLEAN fShowOtherSAMHighLight;
	BOOLEAN fShowDrassenSAMHighLight;

	UINT32	uiEnvWeather;

	UINT8		ubDefaultButton;




	BOOLEAN	fSkyriderEmptyHelpGiven;
	BOOLEAN	fEnterMapDueToContract;
	UINT8		ubHelicopterHitsTaken;
	UINT8		ubQuitType; // XXX HACK000B
	BOOLEAN fSkyriderSaidCongratsOnTakingSAM;
	INT16		sContractRehireSoldierID;


	GAME_OPTIONS	GameOptions;

	UINT32	uiSeedNumber;

	//The GetJA2Clock() value
	UINT32	uiBaseJA2Clock;

	INT16		sCurInterfacePanel;

	UINT8		ubSMCurrentMercID;

	BOOLEAN	fFirstTimeInMapScreen; // XXX HACK000B

	BOOLEAN	fDisableDueToBattleRoster;

	BOOLEAN fDisableMapInterfaceDueToBattle;

	INT16		sBoxerGridNo[ NUM_BOXERS ];
	UINT8		ubBoxerID[ NUM_BOXERS ];
	BOOLEAN	fBoxerFought[ NUM_BOXERS ];

	BOOLEAN	fHelicopterDestroyed;								//if the chopper is destroyed
	BOOLEAN	fShowMapScreenHelpText; // If true, displays help in mapscreen // XXX HACK000B

	INT32		iSortStateForMapScreenList;
	BOOLEAN	fFoundTixa;

	UINT32	uiTimeOfLastSkyriderMonologue;
	BOOLEAN fShowCambriaHospitalHighLight;
	BOOLEAN fSkyRiderSetUp;
	BOOLEAN fRefuelingSiteAvailable[ NUMBER_OF_REFUEL_SITES ];


	//Meanwhile stuff
	MEANWHILE_DEFINITION	gCurrentMeanwhileDef;

	BOOLEAN ubPlayerProgressSkyriderLastCommentedOn;

	BOOLEAN								gfMeanwhileTryingToStart;
	BOOLEAN								gfInMeanwhile;

	// list of dead guys for squads...in id values -> -1 means no one home
	INT16 sDeadMercs[ NUMBER_OF_SQUADS ][ NUMBER_OF_SOLDIERS_PER_SQUAD ];

	// levels of publicly known noises
	INT8	gbPublicNoiseLevel[MAXTEAMS];

	UINT8		gubScreenCount;

	UINT16	usOldMeanWhileFlags;

	INT32		iPortraitNumber;

	INT16		sWorldSectorLocationOfFirstBattle;

	BOOLEAN	fUnReadMailFlag;
	BOOLEAN fNewMailFlag;
	BOOLEAN	fOldUnReadFlag; // XXX HACK000B
	BOOLEAN	fOldNewMailFlag;

	BOOLEAN	fShowMilitia;

	BOOLEAN	fNewFilesInFileViewer;

	BOOLEAN	fLastBoxingMatchWonByPlayer;

	UINT32	uiUNUSED;

	BOOLEAN fSamSiteFound[ NUMBER_OF_SAMS ];

	UINT8		ubNumTerrorists;
	UINT8		ubCambriaMedicalObjects;

	BOOLEAN	fDisableTacticalPanelButtons;

	INT16		sSelMapX;
	INT16		sSelMapY;
	INT32		iCurrentMapSectorZ;

	UINT16	usHasPlayerSeenHelpScreenInCurrentScreen;
	BOOLEAN	fHideHelpInAllScreens;
	UINT8		ubBoxingMatchesWon;

	UINT8		ubBoxersRests;
	BOOLEAN	fBoxersResting;
	UINT8		ubDesertTemperature;
	UINT8		ubGlobalTemperature;

	INT16		sMercArriveSectorX;
	INT16		sMercArriveSectorY;

	BOOLEAN fCreatureMeanwhileScenePlayed;
	UINT8		ubPlayerNum;
	//New stuff for the Prebattle interface / autoresolve
	BOOLEAN fPersistantPBI;
	UINT8 ubEnemyEncounterCode;

	BOOLEAN ubExplicitEnemyEncounterCode;
	BOOLEAN fBlitBattleSectorLocator;
	UINT8 ubPBSectorX;
	UINT8 ubPBSectorY;

	UINT8 ubPBSectorZ;
	BOOLEAN fCantRetreatInPBI;
	BOOLEAN fExplosionQueueActive;
	UINT8	ubUnused[1];

	UINT32	uiMeanWhileFlags;

	INT8 bSelectedInfoChar;
	INT8 bHospitalPriceModifier;
	INT8 bUnused2[ 2 ];

	INT32 iHospitalTempBalance;
	INT32 iHospitalRefund;

  INT8  fPlayerTeamSawJoey;
	INT8	fMikeShouldSayHi;

	UINT8		ubFiller[550];		//This structure should be 1024 bytes

} GENERAL_SAVE_INFO;
CASSERT(sizeof(GENERAL_SAVE_INFO) == 1024)


UINT32	guiSaveGameVersion=0;


static UINT8 gubSaveGameLoc = 0;

UINT32		guiScreenToGotoAfterLoadingSavedGame = 0;

extern		UINT32		guiCurrentUniqueSoldierId;


static UINT32 CalcJA2EncryptionSet(SAVED_GAME_HEADER* pSaveGameHeader);
static void PauseBeforeSaveGame(void);
static void UnPauseAfterSaveGame(void);
static BOOLEAN SaveGeneralInfo(HWFILE hFile);
static BOOLEAN SaveMeanwhileDefsFromSaveGameFile(HWFILE hFile);
static BOOLEAN SaveMercProfiles(HWFILE hFile);
static BOOLEAN SaveOppListInfoToSavedGame(HWFILE hFile);
static BOOLEAN SavePreRandomNumbersToSaveGameFile(HWFILE hFile);
static BOOLEAN SaveSoldierStructure(HWFILE hFile);
static BOOLEAN SaveTacticalStatusToSavedGame(HWFILE hFile);
static BOOLEAN SaveWatchedLocsToSavedGame(HWFILE hFile);

#ifdef JA2BETAVERSION
static void InitSaveGameFilePosition(void);
static void InitShutDownMapTempFileTest(BOOLEAN fInit, const char* pNameOfFile, UINT8 ubSaveGameID);
static void SaveGameFilePosition(const HWFILE save, const char* pMsg);
#endif


BOOLEAN SaveGame( UINT8 ubSaveGameID, const wchar_t *GameDesc)
{
	HWFILE	hFile=0;
	SAVED_GAME_HEADER SaveGameHeader;
	CHAR8		zSaveGameName[ 512 ];
	UINT32	uiSizeOfGeneralInfo = sizeof( GENERAL_SAVE_INFO );
	BOOLEAN	fPausedStateBeforeSaving = gfGamePaused;
	BOOLEAN	fLockPauseStateBeforeSaving = gfLockPauseState;
	INT32		iSaveLoadGameMessageBoxID = -1;
	UINT16	usPosX, usActualWidth, usActualHeight;
	BOOLEAN fWePausedIt = FALSE;

	wchar_t pGameDesc[256];
	wcslcpy(pGameDesc, GameDesc, lengthof(pGameDesc));

#ifdef JA2BETAVERSION
	AssertMsg( uiSizeOfGeneralInfo == 1024, String( "Saved General info is NOT 1024, it is %d.  DF 1.", uiSizeOfGeneralInfo ) );
	AssertMsg( sizeof( LaptopSaveInfoStruct ) == 7440, String( "LaptopSaveStruct is NOT 7440, it is %d.  DF 1.", sizeof( LaptopSaveInfoStruct ) ) );
#endif

	if( ubSaveGameID >= NUM_SAVE_GAMES && ubSaveGameID != SAVE__ERROR_NUM && ubSaveGameID != SAVE__END_TURN_NUM )
		return( FALSE );

	//clear out the save game header
	memset( &SaveGameHeader, 0, sizeof( SAVED_GAME_HEADER ) );

	if ( !GamePaused() )
	{
		PauseBeforeSaveGame();
		fWePausedIt = TRUE;
	}


	#ifdef JA2BETAVERSION
		InitShutDownMapTempFileTest( TRUE, "SaveMapTempFile", ubSaveGameID );
	#endif


	//Place a message on the screen telling the user that we are saving the game
	iSaveLoadGameMessageBoxID = PrepareMercPopupBox( iSaveLoadGameMessageBoxID, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, zSaveLoadText[ SLG_SAVING_GAME_MESSAGE ], 300, 0, 0, 0, &usActualWidth, &usActualHeight);
	usPosX = (SCREEN_WIDTH - usActualWidth) / 2;

	RenderMercPopUpBoxFromIndex( iSaveLoadGameMessageBoxID, usPosX, 160, FRAME_BUFFER );

	InvalidateScreen();

	ExecuteBaseDirtyRectQueue( );
	EndFrameBufferRender( );
	RefreshScreen();

	if( RemoveMercPopupBoxFromIndex( iSaveLoadGameMessageBoxID ) )
	{
		iSaveLoadGameMessageBoxID = -1;
	}

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




	gubSaveGameLoc = ubSaveGameID;

#ifdef JA2BETAVERSION
	InitSaveGameFilePosition();
#endif


	//Set the fact that we are saving a game
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;


	//Save the current sectors open temp files to the disk
	if( !SaveCurrentSectorsInformationToTempItemFile() )
	{
		ScreenMsg( FONT_MCOLOR_WHITE, MSG_TESTVERSION, L"ERROR in SaveCurrentSectorsInformationToTempItemFile()");
		goto FAILED_TO_SAVE_NO_CLOSE;
	}

	//if we are saving the quick save,
	if( ubSaveGameID == 0 )
	{
#ifdef JA2BETAVERSION
		//Increment the quicksave counter
		guiCurrentQuickSaveNumber++;

		if( gfUseConsecutiveQuickSaveSlots )
			swprintf(pGameDesc, lengthof(pGameDesc), L"%hs%03d", g_quicksave_name, guiCurrentQuickSaveNumber);
		else
#endif
			swprintf(pGameDesc, lengthof(pGameDesc), L"%hs", g_quicksave_name);
	}

	//If there was no string, add one
	if( pGameDesc[0] == '\0' )
		wcscpy( pGameDesc, pMessageStrings[ MSG_NODESC ] );

	//Check to see if the save directory exists
	const char* const dir = g_savegame_dir;
	if (FileGetAttributes(dir) == 0xFFFFFFFF && // if the directory does not exist
			!MakeFileManDirectory(dir))
	{
		goto FAILED_TO_SAVE_NO_CLOSE;
	}

	//Create the name of the file
	CreateSavedGameFileNameFromNumber( ubSaveGameID, zSaveGameName );

	// create the save game file
	hFile = FileOpen(zSaveGameName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
	if( !hFile )
	{
		goto FAILED_TO_SAVE_NO_CLOSE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Just Opened File");
#endif

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
	wcscpy( SaveGameHeader.sSavedGameDesc, pGameDesc );
	strcpy(SaveGameHeader.zGameVersionNumber, g_version_number);

	SaveGameHeader.uiFlags;

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


	if (!FileWrite(hFile, &SaveGameHeader, sizeof(SAVED_GAME_HEADER))) goto FAILED_TO_SAVE;
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Save Game Header");
#endif

	guiJA2EncryptionSet = CalcJA2EncryptionSet( &SaveGameHeader );

	//
	//Save the gTactical Status array, plus the curent secotr location
	//
	if( !SaveTacticalStatusToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Tactical Status");
#endif

	// save the game clock info
	if( !SaveGameClock( hFile, fPausedStateBeforeSaving, fLockPauseStateBeforeSaving ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Game Clock");
#endif

	// save the strategic events
	if( !SaveStrategicEventsToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Strategic Events");
#endif

	if( !SaveLaptopInfoToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Laptop Info");
#endif

	//
	// Save the merc profiles
	//
	if( !SaveMercProfiles( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Merc Profiles");
#endif

	//
	// Save the soldier structure
	//
	if( !SaveSoldierStructure( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Soldier Structure");
#endif

	//Save the Finaces Data file
	if( !SaveFilesToSavedGame( FINANCES_DATA_FILE, hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Finances Data File");
#endif

	//Save the history file
	if( !SaveFilesToSavedGame( HISTORY_DATA_FILE, hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "History file");
#endif

	//Save the Laptop File file
	if( !SaveFilesToSavedGame( FILES_DAT_FILE, hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "The Laptop FILES file");
#endif

	//Save email stuff to save file
	if( !SaveEmailToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Email");
#endif

	//Save the strategic information
	if( !SaveStrategicInfoToSavedFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Strategic Information");
#endif

	//save the underground information
	if( !SaveUnderGroundSectorInfoToSaveGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Underground Information");
#endif

	//save the squad info
	if( !SaveSquadInfoToSavedGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Squad Info");
#endif

	if( !SaveStrategicMovementGroupsToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Strategic Movement Groups");
#endif

	//Save all the map temp files from the maps\temp directory into the saved game file
	if( !SaveMapTempFilesToSavedGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "All the Map Temp files");
#endif

	if( !SaveQuestInfoToSavedGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Quest Info");
#endif

	if( !SaveOppListInfoToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "OppList info");
#endif

	if( !SaveMapScreenMessagesToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "MapScreen Messages");
#endif

	if( !SaveNPCInfoToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "NPC Info");
#endif

	if( !SaveKeyTableToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "KeyTable");
#endif

	if( !SaveTempNpcQuoteArrayToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "NPC Temp Quote File");
#endif

	if( !SavePreRandomNumbersToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "PreGenerated Random Files");
#endif

	if( !SaveArmsDealerInventoryToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Arms Dealers Inventory");
#endif

	if( !SaveGeneralInfo( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Misc. Info");
#endif

	if( !SaveMineStatusToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Mine Status");
#endif

	if( !SaveStrategicTownLoyaltyToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Town Loyalty");
#endif

	if( !SaveVehicleInformationToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Vehicle Information");
#endif

	if( !SaveBulletStructureToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Bullet Information");
#endif

	if( !SavePhysicsTableToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Physics Table");
#endif

	if( !SaveAirRaidInfoToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Air Raid Info");
#endif

	if( !SaveTeamTurnsToTheSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Team Turn Info");
#endif

	if( !SaveExplosionTableToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Explosion Table");
#endif

	if( !SaveCreatureDirectives( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Creature Spreading");
#endif

	if( !SaveStrategicStatusToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Strategic Status");
#endif

	if( !SaveStrategicAI( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Strategic AI");
#endif

	if( !SaveWatchedLocsToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Watched Locs Info");
#endif

	if( !SaveItemCursorToSavedGame( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "ItemCursor Info");
#endif

	if( !SaveCivQuotesToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Civ Quote System");
#endif

	if( !SaveBackupNPCInfoToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Backed up NPC Info");
#endif

	if ( !SaveMeanwhileDefsFromSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Meanwhile Definitions");
#endif

	// save meanwhiledefs

	if ( !SaveSchedules( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Schedules");
#endif

		// Save extra vehicle info
	if ( !NewSaveVehicleMovementInfoToSavedGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Vehicle Movement Stuff");
#endif

	// Save contract renewal sequence stuff
	if ( !SaveContractRenewalDataToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Contract Renewal Data");
#endif

	// Save leave list stuff
	if ( !SaveLeaveItemList( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "leave list");
#endif

	//do the new way of saving bobbyr mail order items
	if( !NewWayOfSavingBobbyRMailOrdersToSaveGameFile( hFile ) )
	{
		goto FAILED_TO_SAVE;
	}
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "New way of saving Bobby R mailorders");
#endif

	//Close the saved game file
	FileClose( hFile );


	//if we succesfully saved the game, mark this entry as the last saved game file
	if( ubSaveGameID != SAVE__ERROR_NUM && ubSaveGameID != SAVE__END_TURN_NUM )
	{
		gGameSettings.bLastSavedGameSlot = ubSaveGameID;
	}

	//Save the save game settings
	SaveGameSettings();

	//
	// Display a screen message that the save was succesful
	//

	//if its the quick save slot
	if( ubSaveGameID == 0 )
	{
		ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[ MSG_SAVESUCCESS ] );
	}
//#ifdef JA2BETAVERSION
	else if( ubSaveGameID == SAVE__END_TURN_NUM )
	{
//		ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[ MSG_END_TURN_AUTO_SAVE ] );
	}
//#endif
	else
	{
		ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[ MSG_SAVESLOTSUCCESS ] );
	}

	//restore the music mode
	SetMusicMode( gubMusicMode );

	//Unset the fact that we are saving a game
	gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;

	UnPauseAfterSaveGame();

	#ifdef JA2BETAVERSION
		InitShutDownMapTempFileTest( FALSE, "SaveMapTempFile", ubSaveGameID );
	#endif

	#ifdef JA2BETAVERSION
		ValidateSoldierInitLinks( 2 );
	#endif

	//Check for enough free hard drive space
	NextLoopCheckForEnoughFreeHardDriveSpace();

	return( TRUE );

	//if there is an error saving the game
FAILED_TO_SAVE:
#ifdef JA2BETAVERSION
	SaveGameFilePosition(hFile, "Failed to Save!!!");
#endif

	FileClose( hFile );

FAILED_TO_SAVE_NO_CLOSE:
	if ( fWePausedIt )
	{
		UnPauseAfterSaveGame();
	}

	//Delete the failed attempt at saving
	DeleteSaveGameNumber( ubSaveGameID );

	//Put out an error message
	ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, zSaveLoadText[SLG_SAVE_GAME_ERROR] );

	#ifdef JA2BETAVERSION
		InitShutDownMapTempFileTest( FALSE, "SaveMapTempFile", ubSaveGameID );
	#endif

	//Check for enough free hard drive space
	NextLoopCheckForEnoughFreeHardDriveSpace();

#ifdef JA2BETAVERSION
	if( fDisableDueToBattleRoster || fDisableMapInterfaceDueToBattle )
	{
		gubReportMapscreenLock = 2;
	}
#endif

	return( FALSE );
}


UINT32 guiBrokenSaveGameVersion = 0;


static void HandleOldBobbyRMailOrders(void);
static BOOLEAN LoadGeneralInfo(HWFILE hFile);
static BOOLEAN LoadMeanwhileDefsFromSaveGameFile(HWFILE hFile);
static BOOLEAN LoadOppListInfoFromSavedGame(HWFILE hFile);
static BOOLEAN LoadPreRandomNumbersFromSaveGameFile(HWFILE hFile);
static BOOLEAN LoadSavedMercProfiles(HWFILE hFile);
static BOOLEAN LoadSoldierStructure(HWFILE hFile);
static BOOLEAN LoadTacticalStatusFromSavedGame(HWFILE hFile);
static BOOLEAN LoadWatchedLocsFromSavedGame(HWFILE hFile);
static void TruncateStrategicGroupSizes(void);
static void UpdateMercMercContractInfo(void);

#ifdef JA2BETAVERSION
static void InitLoadGameFilePosition(void);
static void LoadGameFilePosition(HWFILE load, const char* pMsg);
#endif


BOOLEAN LoadSavedGame( UINT8 ubSavedGameID )
{
	HWFILE	hFile;
	SAVED_GAME_HEADER SaveGameHeader;

	INT16 sLoadSectorX;
	INT16 sLoadSectorY;
	INT8 bLoadSectorZ;
	CHAR8		zSaveGameName[ 512 ];
	UINT32	uiSizeOfGeneralInfo = sizeof( GENERAL_SAVE_INFO );

	UINT32 uiRelStartPerc;
	UINT32 uiRelEndPerc;

	uiRelStartPerc = uiRelEndPerc =0;

  TrashAllSoldiers( );

	//Empty the dialogue Queue cause someone could still have a quote in waiting
	EmptyDialogueQueue( );

	//If there is someone talking, stop them
	StopAnyCurrentlyTalkingSpeech( );

  ZeroAnimSurfaceCounts( );

	ShutdownNPCQuotes();


#ifdef JA2BETAVERSION
	AssertMsg( uiSizeOfGeneralInfo == 1024, String( "Saved General info is NOT 1024, it is %d.  DF 1.", uiSizeOfGeneralInfo ) );
#endif

	//is it a valid save number
	if( ubSavedGameID >= NUM_SAVE_GAMES )
	{
		if( ubSavedGameID != SAVE__END_TURN_NUM )
			return( FALSE );
	}
	else if( !gbSaveGameArray[ ubSavedGameID ] )
		return( FALSE );


	#ifdef JA2BETAVERSION
		InitShutDownMapTempFileTest( TRUE, "LoadMapTempFile", ubSavedGameID );
	#endif


	//Used in mapescreen to disable the annoying 'swoosh' transitions
	gfDontStartTransitionFromLaptop = TRUE;

	// Reset timer callbacks
	gpCustomizableTimerCallback = NULL;

	gubSaveGameLoc = ubSavedGameID;
#ifdef JA2BETAVERSION
	InitLoadGameFilePosition();
#endif

	//Set the fact that we are loading a saved game
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;

	//Trash the existing world.  This is done to ensure that if we are loading a game that doesn't have
	//a world loaded, that we trash it beforehand -- else the player could theoretically enter that sector
	//where it would be in a pre-load state.
	TrashWorld();


	//Deletes all the Temp files in the Maps\Temp directory
	InitTacticalSave( TRUE );

	// ATE; Added to empry dialogue q
	EmptyDialogueQueue( );

	//Create the name of the file
	CreateSavedGameFileNameFromNumber( ubSavedGameID, zSaveGameName );

	// open the save game file
	hFile = FileOpen(zSaveGameName, FILE_ACCESS_READ);
	if (!hFile) goto load_failed_no_close;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Just Opened File");
#endif

	//Load the Save Game header file
	if (!FileRead(hFile, &SaveGameHeader, sizeof(SAVED_GAME_HEADER))) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Save Game Header");
#endif

	guiJA2EncryptionSet = CalcJA2EncryptionSet( &SaveGameHeader );

	guiBrokenSaveGameVersion = SaveGameHeader.uiSavedGameVersion;

	//if the player is loading up an older version of the game, and the person DOESNT have the cheats on,
	if (SaveGameHeader.uiSavedGameVersion < 65 && !CHEATER_CHEAT_LEVEL()) goto load_failed;

	//Store the loading screenID that was saved
	gubLastLoadingScreenID = SaveGameHeader.ubLoadScreenID;

	//HACK
	guiSaveGameVersion = SaveGameHeader.uiSavedGameVersion;

/*
	if (!LoadGeneralInfo(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Misc info");
#endif
*/

	//Load the gtactical status structure plus the current sector x,y,z
	if (!LoadTacticalStatusFromSavedGame(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Tactical Status");
#endif

	//This gets reset by the above function
	gTacticalStatus.uiFlags |= LOADING_SAVED_GAME;


	//Load the game clock ingo
	if (!LoadGameClock(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Game Clock");
#endif

	//if we are suppose to use the alternate sector
	if( SaveGameHeader.fAlternateSector )
	{
		SetSectorFlag( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, SF_USE_ALTERNATE_MAP );
		gfUseAlternateMap = TRUE;
	}


	//if the world was loaded when saved, reload it, otherwise dont
	if( SaveGameHeader.fWorldLoaded || SaveGameHeader.uiSavedGameVersion < 50 )
	{
		//Get the current world sector coordinates
		sLoadSectorX = gWorldSectorX;
		sLoadSectorY = gWorldSectorY;
		bLoadSectorZ = gbWorldSectorZ;

		// This will guarantee that the sector will be loaded
		gbWorldSectorZ = -1;


		//if we should load a sector ( if the person didnt just start the game game )
		if( ( gWorldSectorX != 0 ) && ( gWorldSectorY != 0 ) )
		{
			//Load the sector
			SetCurrentWorldSector( sLoadSectorX, sLoadSectorY, bLoadSectorZ );
		}
	}
	else
	{ //By clearing these values, we can avoid "in sector" checks -- at least, that's the theory.
		gWorldSectorX = gWorldSectorY = 0;

		//Since there is no
		if( SaveGameHeader.sSectorX == -1 || SaveGameHeader.sSectorY == -1 || SaveGameHeader.bSectorZ == -1 )
			gubLastLoadingScreenID = LOADINGSCREEN_HELI;
		else
			gubLastLoadingScreenID = GetLoadScreenID( SaveGameHeader.sSectorX, SaveGameHeader.sSectorY, SaveGameHeader.bSectorZ );

		BeginLoadScreen();
	}

	CreateLoadingScreenProgressBar();
	SetProgressBarColor( 0, 0, 0, 150 );

#ifdef JA2BETAVERSION
	//set the font
	SetProgressBarMsgAttributes( 0, FONT12ARIAL, FONT_MCOLOR_WHITE, 0 );

	//
	// Set the tile so we don see the text come up
	//

	//if the world is unloaded, we must use the save buffer for the text
	if( SaveGameHeader.fWorldLoaded )
		SetProgressBarTextDisplayFlag( 0, TRUE, TRUE, FALSE );
	else
		SetProgressBarTextDisplayFlag( 0, TRUE, TRUE, TRUE );
#endif

	uiRelStartPerc = 0;

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Strategic Events..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;

	//load the game events
	if (!LoadStrategicEventsFromSavedGame(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Strategic Events");
#endif

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Laptop Info" );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if (!LoadLaptopInfoFromSavedGame(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Laptop Info");
#endif

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Merc Profiles..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;

	//
	// Load all the saved Merc profiles
	//
	if (!LoadSavedMercProfiles(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Merc Profiles");
#endif

	uiRelEndPerc += 30;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Soldier Structure..." );
	uiRelStartPerc = uiRelEndPerc;


	//
	// Load the soldier structure info
	//
	if (!LoadSoldierStructure(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Soldier Structure");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Finances Data File..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//
	// Load the Finances Data and write it to a new file
	//
	if (!LoadFilesFromSavedGame(FINANCES_DATA_FILE, hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Finances Data File");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"History File..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//
	// Load the History Data and write it to a new file
	//
	if (!LoadFilesFromSavedGame(HISTORY_DATA_FILE, hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "History File");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"The Laptop FILES file..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//
	// Load the Files Data and write it to a new file
	//
	if (!LoadFilesFromSavedGame(FILES_DAT_FILE, hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "The Laptop FILES file");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Email..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	// Load the data for the emails
	if (!LoadEmailFromSavedGame(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Email");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Strategic Information..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	//Load the strategic Information
	if (!LoadStrategicInfoFromSavedFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Strategic Information");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"UnderGround Information..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//Load the underground information
	if (!LoadUnderGroundSectorInfoFromSavedGame(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "UnderGround Information");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Squad Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	// Load all the squad info from the saved game file
	if (!LoadSquadInfoFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Squad Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Strategic Movement Groups..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//Load the group linked list
	if (!LoadStrategicMovementGroupsFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Strategic Movement Groups");
#endif

	uiRelEndPerc += 30;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"All the Map Temp files..." );
	uiRelStartPerc = uiRelEndPerc;


	// Load all the map temp files from the saved game file into the maps\temp directory
	if (!LoadMapTempFilesFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "All the Map Temp files");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Quest Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	if (!LoadQuestInfoFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Quest Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"OppList Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	if (!LoadOppListInfoFromSavedGame(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "OppList Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"MapScreen Messages..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if (!LoadMapScreenMessagesFromSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "MapScreen Messages");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"NPC Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if (!LoadNPCInfoFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "NPC Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"KeyTable..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if (!LoadKeyTableFromSaveedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "KeyTable");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Npc Temp Quote File..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	if (!LoadTempNpcQuoteArrayToSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Npc Temp Quote File");
#endif

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"PreGenerated Random Files..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	if (!LoadPreRandomNumbersFromSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "PreGenerated Random Files");
#endif

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Smoke Effect Structures..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	if (!LoadSmokeEffectsFromLoadGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Smoke Effect Structures");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Arms Dealers Inventory..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if (!LoadArmsDealerInventoryFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion >= 54, SaveGameHeader.uiSavedGameVersion >= 55)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Arms Dealers Inventory");
#endif

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Misc info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;




	if (!LoadGeneralInfo(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Misc info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Mine Status..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	if (!LoadMineStatusFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Mine Status");
#endif

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Town Loyalty..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;




	if( SaveGameHeader.uiSavedGameVersion	>= 21 )
	{
		if (!LoadStrategicTownLoyaltyFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Town Loyalty");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Vehicle Information..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 22 )
	{
		if (!LoadVehicleInformationFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Vehicle Information");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Bullet Information..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 24 )
	{
		if (!LoadBulletStructureFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Bullet Information");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Physics table..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;




	if( SaveGameHeader.uiSavedGameVersion	>= 24 )
	{
		if (!LoadPhysicsTableFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Physics table");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Air Raid Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 24 )
	{
		if (!LoadAirRaidInfoFromSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Air Raid Info");
#endif
	}

	uiRelEndPerc += 0;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Team Turn Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 24 )
	{
		if (!LoadTeamTurnsFromTheSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Team Turn Info");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Explosion Table..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 25 )
	{
		if (!LoadExplosionTableFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Explosion Table");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Creature Spreading..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;




	if( SaveGameHeader.uiSavedGameVersion	>= 27 )
	{
		if (!LoadCreatureDirectives(hFile, SaveGameHeader.uiSavedGameVersion)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Creature Spreading");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Strategic Status..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;




	if( SaveGameHeader.uiSavedGameVersion	>= 28 )
	{
		if (!LoadStrategicStatusFromSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Strategic Status");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Strategic AI..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 31 )
	{
		if (!LoadStrategicAI(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Strategic AI");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Lighting Effects..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 37 )
	{
		if (!LoadLightEffectsFromLoadGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Lighting Effects");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Watched Locs Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 38 )
	{
		if (!LoadWatchedLocsFromSavedGame(hFile)) goto load_failed;
	}
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Watched Locs Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Item cursor Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion	>= 39 )
	{
		if (!LoadItemCursorFromSavedGame(hFile)) goto load_failed;
	}
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Item cursor Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Civ Quote System..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion >= 51 )
	{
		if (!LoadCivQuotesFromLoadGameFile(hFile)) goto load_failed;
	}
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Civ Quote System");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Backed up NPC Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion >= 53 )
	{
		if (!LoadBackupNPCInfoFromSavedGameFile(hFile, SaveGameHeader.uiSavedGameVersion)) goto load_failed;
	}
#ifdef JA2BETAVERSION
	LoadGameFilePosition(hFile, "Backed up NPC Info");
#endif

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Meanwhile definitions..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion >= 58 )
	{
		if (!LoadMeanwhileDefsFromSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Meanwhile definitions");
#endif
	}
	else
	{
		gMeanwhileDef[gCurrentMeanwhileDef.ubMeanwhileID] = gCurrentMeanwhileDef;
	}




	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Schedules..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion >= 59 )
	{
		// trash schedules loaded from map
		DestroyAllSchedulesWithoutDestroyingEvents();
		if (!LoadSchedulesFromSave(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Schedules");
#endif
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Extra Vehicle Info..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion >= 61 )
	{
	  if( SaveGameHeader.uiSavedGameVersion < 84 )
	  {
		  if (!LoadVehicleMovementInfoFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
			LoadGameFilePosition(hFile, "Extra Vehicle Info");
#endif
    }
    else
    {
		  if (!NewLoadVehicleMovementInfoFromSavedGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
			LoadGameFilePosition(hFile, "Extra Vehicle Info");
#endif
    }
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Contract renweal sequence stuff..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;



	if( SaveGameHeader.uiSavedGameVersion < 62 )
	{
		// the older games had a bug where this flag was never being set
		gfResetAllPlayerKnowsEnemiesFlags = TRUE;
	}

	if( SaveGameHeader.uiSavedGameVersion >= 67 )
	{
		if (!LoadContractRenewalDataFromSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Contract renweal sequence stuff");
#endif
	}

	if( SaveGameHeader.uiSavedGameVersion >= 70 )
	{
		if (!LoadLeaveItemList(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "Leave List");
#endif
	}

	if( SaveGameHeader.uiSavedGameVersion <= 73 )
	{
    // Patch vehicle fuel
    AddVehicleFuelToSave( );
	}


	if( SaveGameHeader.uiSavedGameVersion >= 85 )
	{
		if (!NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(hFile)) goto load_failed;
#ifdef JA2BETAVERSION
		LoadGameFilePosition(hFile, "New way of loading Bobby R mailorders");
#endif
	}

	//If there are any old Bobby R Mail orders, tranfer them to the new system
	if( SaveGameHeader.uiSavedGameVersion < 85 )
	{
		HandleOldBobbyRMailOrders();
	}


	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Final Checks..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;




	//
	//Close the saved game file
	//
	FileClose( hFile );

	// ATE: Patch? Patch up groups.....( will only do for old saves.. )
	UpdatePersistantGroupsFromOldSave( SaveGameHeader.uiSavedGameVersion );


	if( SaveGameHeader.uiSavedGameVersion	<= 40 )
	{
		// Cancel all pending purchase orders for BobbyRay's.  Starting with version 41, the BR orders events are
		// posted with the usItemIndex itself as the parameter, rather than the inventory slot index.  This was
		// done to make it easier to modify BR's traded inventory lists later on without breaking saves.
		CancelAllPendingBRPurchaseOrders();
	}


	//if the world is loaded, apply the temp files to the loaded map
	if( SaveGameHeader.fWorldLoaded || SaveGameHeader.uiSavedGameVersion < 50 )
	{
		// Load the current sectors Information From the temporary files
		if( !LoadCurrentSectorsInformationFromTempItemsFile() )
		{
			InitExitGameDialogBecauseFileHackDetected();
			guiSaveGameVersion=0;
			return( TRUE );
		}
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Final Checks..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;

	InitAI();

	//Update the mercs in the sector with the new soldier info
	UpdateMercsInSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	PostSchedules();


	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Final Checks..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//Reset the lighting level if we are outside
	if( gbWorldSectorZ == 0 )
		LightSetBaseLevel( GetTimeOfDayAmbientLightLevel() );

	//if we have been to this sector before
//	if( SectorInfo[ SECTOR( gWorldSectorX,gWorldSectorY) ].uiFlags & SF_ALREADY_VISITED )
	{
		//Reset the fact that we are loading a saved game
		gTacticalStatus.uiFlags &= ~LOADING_SAVED_GAME;
	}

	// CJC January 13: we can't do this because (a) it resets militia IN THE MIDDLE OF
	// COMBAT, and (b) if we add militia to the teams while LOADING_SAVED_GAME is set,
	// the team counters will not be updated properly!!!
//	ResetMilitia();


	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Final Checks..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;

	//if the UI was locked in the saved game file
	if( gTacticalStatus.ubAttackBusyCount > 1 )
	{
		//Lock the ui
		SetUIBusy(GetSelectedMan());
	}

	//Reset the shadow
  SetFontShadow( DEFAULT_SHADOW );

#ifdef JA2BETAVERSION
	AssertMsg( uiSizeOfGeneralInfo == 1024, String( "Saved General info is NOT 1024, it is %d.  DF 1.", uiSizeOfGeneralInfo ) );
#endif

	//if we succesfully LOADED! the game, mark this entry as the last saved game file
	gGameSettings.bLastSavedGameSlot		= ubSavedGameID;

	//Save the save game settings
	SaveGameSettings();


	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Final Checks..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	//Reset the Ai Timer clock
	giRTAILastUpdateTime = 0;

	//if we are in tactical
	if( guiScreenToGotoAfterLoadingSavedGame == GAME_SCREEN )
	{
		//Initialize the current panel
		InitializeCurrentPanel( );

		SelectSoldier(GetSelectedMan(), SELSOLDIER_FORCE_RESELECT);
	}

	uiRelEndPerc += 1;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Final Checks..." );
	RenderProgressBar( 0, 100 );
	uiRelStartPerc = uiRelEndPerc;


	// init extern faces
	InitalizeStaticExternalNPCFaces( );

	// load portraits
	LoadCarPortraitValues( );

	// OK, turn OFF show all enemies....
	gTacticalStatus.uiFlags&= (~SHOW_ALL_MERCS );
	gTacticalStatus.uiFlags &= ~SHOW_ALL_ITEMS ;

	#ifdef JA2BETAVERSION
		InitShutDownMapTempFileTest( FALSE, "LoadMapTempFile", ubSavedGameID );
	#endif

	if ( (gTacticalStatus.uiFlags & INCOMBAT) )
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Setting attack busy count to 0 from load");
		gTacticalStatus.ubAttackBusyCount = 0;
	}

	if( SaveGameHeader.uiSavedGameVersion	< 64 )
	{ //Militia/enemies/creature team sizes have been changed from 32 to 20.  This function
		//will simply kill off the excess.  This will allow the ability to load previous saves, though
		//there will still be problems, though a LOT less than there would be without this call.
		TruncateStrategicGroupSizes();
	}

	// ATE: if we are within this window where skyridder was foobared, fix!
	if ( SaveGameHeader.uiSavedGameVersion >= 61 && SaveGameHeader.uiSavedGameVersion <= 65 )
	{
		MERCPROFILESTRUCT *pProfile;

		if ( !fSkyRiderSetUp )
		{
			// see if we can find him and remove him if so....
			SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(SKYRIDER);
			if (pSoldier != NULL) TacticalRemoveSoldier(pSoldier);

			// add the pilot at a random location!
			pProfile = &(gMercProfiles[ SKYRIDER ]);
			switch( Random( 4 ) )
			{
				case 0:
					pProfile->sSectorX = 15;
					pProfile->sSectorY = MAP_ROW_B;
					pProfile->bSectorZ = 0;
					break;
				case 1:
					pProfile->sSectorX = 14;
					pProfile->sSectorY = MAP_ROW_E;
					pProfile->bSectorZ = 0;
					break;
				case 2:
					pProfile->sSectorX = 12;
					pProfile->sSectorY = MAP_ROW_D;
					pProfile->bSectorZ = 0;
					break;
				case 3:
					pProfile->sSectorX = 16;
					pProfile->sSectorY = MAP_ROW_C;
					pProfile->bSectorZ = 0;
					break;
			}
		}
	}

	if ( SaveGameHeader.uiSavedGameVersion < 68 )
	{
		// correct bVehicleUnderRepairID for all mercs
		FOR_ALL_NON_PLANNING_SOLDIERS(s) s->bVehicleUnderRepairID = -1;
	}

	if ( SaveGameHeader.uiSavedGameVersion < 73 )
	{
		if( LaptopSaveInfo.fMercSiteHasGoneDownYet )
			LaptopSaveInfo.fFirstVisitSinceServerWentDown = 2;
	}


	//Update the MERC merc contract lenght.  Before save version 77 the data was stored in the SOLDIERTYPE,
	//after 77 the data is stored in the profile
	if ( SaveGameHeader.uiSavedGameVersion < 77 )
	{
		UpdateMercMercContractInfo();
	}


	if ( SaveGameHeader.uiSavedGameVersion <= 89 )
	{
		// ARM: A change was made in version 89 where refuel site availability now also depends on whether the player has
		// airspace control over that sector.  To update the settings immediately, must call it here.
		UpdateRefuelSiteAvailability();
	}

	if( SaveGameHeader.uiSavedGameVersion < 91 )
	{
		//update the amount of money that has been paid to speck
		CalcAproximateAmountPaidToSpeck();
	}

	gfLoadedGame = TRUE;

	uiRelEndPerc = 100;
	SetRelativeStartAndEndPercentage( 0, uiRelStartPerc, uiRelEndPerc, L"Done!" );
	RenderProgressBar( 0, 100 );

	RemoveLoadingScreenProgressBar();

	SetMusicMode( gMusicModeToPlay );

#ifndef JA2TESTVERSION
	RESET_CHEAT_LEVEL( );
#endif

#ifdef JA2BETAVERSION
	if( fDisableDueToBattleRoster || fDisableMapInterfaceDueToBattle )
	{
		gubReportMapscreenLock = 1;
	}
#endif

	// reset to 0
	guiSaveGameVersion=0;

	// reset once-per-convo records for everyone in the loaded sector
	ResetOncePerConvoRecordsForAllNPCsInLoadedSector();

	if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
	{
		// fix lingering attack busy count problem on loading saved game by resetting a.b.c
		// if we're not in combat.
		gTacticalStatus.ubAttackBusyCount = 0;
	}

	// fix squads
	CheckSquadMovementGroups();

	//The above function LightSetBaseLevel adjusts ALL the level node light values including the merc node,
	//we must reset the values
	HandlePlayerTogglingLightEffects( FALSE );


	return( TRUE );

load_failed:
	FileClose(hFile);
load_failed_no_close:
	guiSaveGameVersion = 0;
	return FALSE;
}


static BOOLEAN SaveMercProfiles(HWFILE hFile)
{
	UINT16	cnt;

	//Lopp through all the profiles to save
	for( cnt=0; cnt< NUM_PROFILES; cnt++)
	{
		gMercProfiles[ cnt ].uiProfileChecksum = ProfileChecksum( &(gMercProfiles[ cnt ]) );
		BOOLEAN Ret;
#ifdef _WIN32 // XXX HACK000A
		BYTE Data[716];
#else
		BYTE Data[796];
#endif
		InjectMercProfile(Data,  &gMercProfiles[cnt]);
		if ( guiSavedGameVersion < 87 )
		{
			Ret = JA2EncryptedFileWrite(hFile, Data, sizeof(Data));
		}
		else
		{
			Ret = NewJA2EncryptedFileWrite(hFile, Data, sizeof(Data));
		}
		if (!Ret) return FALSE;
	}

	return( TRUE );
}


static BOOLEAN LoadSavedMercProfiles(HWFILE hFile)
{
	UINT16	cnt;

	//Lopp through all the profiles to Load
	for( cnt=0; cnt< NUM_PROFILES; cnt++)
	{
#ifdef _WIN32 // XXX HACK000A
		BYTE Data[716];
#else
		BYTE Data[796];
#endif
		BOOLEAN Ret;
		if ( guiSaveGameVersion < 87 )
		{
			Ret = JA2EncryptedFileRead(hFile, Data, sizeof(Data));
		}
		else
		{
			Ret = NewJA2EncryptedFileRead(hFile, Data, sizeof(Data));
		}
		if (!Ret) return FALSE;
		ExtractMercProfile(Data,  &gMercProfiles[cnt]);
		if ( gMercProfiles[ cnt ].uiProfileChecksum != ProfileChecksum( &(gMercProfiles[ cnt ]) ) )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}


		//Not saving any of these in the soldier struct
		//	LEVELNODE*                   pLevelNode;
		//	UINT16											*pForcedShade;
		//
		// 	UINT16											*pEffectShades[ NUM_SOLDIER_EFFECTSHADES ]; // Shading tables for effects
		//  THROW_PARAMS								*pThrowParams;
		//	UINT16											*pGlowShades[ 20 ]; //
		//	UINT16											*pShades[ NUM_SOLDIER_SHADES ]; // Shading tables
		//	OBJECTTYPE									*pTempObject;
static BOOLEAN SaveSoldierStructure(HWFILE hFile)
{
	UINT16	cnt;
	UINT8		ubOne = 1;
	UINT8		ubZero = 0;

	//Loop through all the soldier structs to save
	for( cnt=0; cnt< TOTAL_SOLDIERS; cnt++)
	{
		SOLDIERTYPE* const s = GetMan(cnt);
		//if the soldier isnt active, dont add them to the saved game file.
		if (!s->bActive)
		{
			// Save the byte specifing to NOT load the soldiers
			if (!FileWrite(hFile, &ubZero, 1)) return FALSE;
		}
		else
		{
			// Save the byte specifing to load the soldiers
			if (!FileWrite(hFile, &ubOne, 1)) return FALSE;

			// calculate checksum for soldier
			s->uiMercChecksum = MercChecksum(s);
			// Save the soldier structure
#ifdef _WIN32 // XXX HACK000A
			BYTE Data[2328];
#else
			BYTE Data[2352];
#endif
			InjectSoldierType(Data, s);
			BOOLEAN Ret;
			if ( guiSavedGameVersion < 87 )
			{
				Ret = JA2EncryptedFileWrite(hFile, Data, sizeof(Data));
			}
			else
			{
				Ret = NewJA2EncryptedFileWrite(hFile, Data, sizeof(Data));
			}
			if (!Ret) return FALSE;

			// Save all the pointer info from the structure

			if (!SaveMercPath(hFile, s->pMercPath)) return FALSE;

			//do we have a 	KEY_ON_RING									*pKeyRing;
			if (s->pKeyRing != NULL)
			{
				// write to the file saying we have the ....
				if (!FileWrite(hFile, &ubOne, 1)) return FALSE;

				// Now save the ....
				if (!FileWrite(hFile, s->pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING))) return FALSE;
			}
			else
			{
				// write to the file saying we DO NOT have the Key ring
				if (!FileWrite(hFile, &ubZero, 1)) return FALSE;
			}
		}
	}

	return( TRUE );
}


static BOOLEAN LoadSoldierStructure(HWFILE hFile)
{
	UINT16	cnt;
	SOLDIERTYPE SavedSoldierInfo;
	UINT8		ubOne = 1;
	UINT8		ubActive = 1;
	UINT32	uiPercentage;

	//Loop through all the soldier and delete them all
	FOR_ALL_SOLDIERS(s) TacticalRemoveSoldier(s);

	//Loop through all the soldier structs to load
	for( cnt=0; cnt< TOTAL_SOLDIERS; cnt++)
	{

		//update the progress bar
		uiPercentage = (cnt * 100) / (TOTAL_SOLDIERS-1);

		RenderProgressBar( 0, uiPercentage );


		//Read in a byte to tell us whether or not there is a soldier loaded here.
		if (!FileRead(hFile, &ubActive, 1)) return FALSE;

		// if the soldier is not active, continue
		if( !ubActive )
		{
			continue;
		}

		// else if there is a soldier
		else
		{
#ifdef _WIN32 // XXX HACK000A
			BYTE Data[2328];
#else
			BYTE Data[2352];
#endif
			//Read in the saved soldier info into a Temp structure
			BOOLEAN Ret;
			if ( guiSaveGameVersion < 87 )
			{
				Ret = JA2EncryptedFileRead(hFile, &Data, sizeof(Data));
			}
			else
			{
				Ret = NewJA2EncryptedFileRead(hFile, &Data, sizeof(Data));
			}
			if (!Ret) return FALSE;
			ExtractSoldierType(Data, &SavedSoldierInfo);
			// check checksum
			if ( MercChecksum( &SavedSoldierInfo ) != SavedSoldierInfo.uiMercChecksum )
			{
				return( FALSE );
			}

			//Make sure all the pointer references are NULL'ed out.
			SavedSoldierInfo.pTempObject	 = NULL;
			SavedSoldierInfo.pKeyRing	 = NULL;
			memset( SavedSoldierInfo.pShades, 0, sizeof( UINT16* ) * NUM_SOLDIER_SHADES );
			memset( SavedSoldierInfo.pGlowShades, 0, sizeof( UINT16* ) * 20 );
			SavedSoldierInfo.pThrowParams	 = NULL;
			SavedSoldierInfo.pLevelNode	 = NULL;
			SavedSoldierInfo.pForcedShade	 = NULL;
			SavedSoldierInfo.pMercPath	 = NULL;
			memset( SavedSoldierInfo.pEffectShades, 0, sizeof( UINT16* ) * NUM_SOLDIER_EFFECTSHADES );

			SOLDIERTYPE* const s = TacticalCreateSoldierFromExisting(&SavedSoldierInfo);
			if (s == NULL) return FALSE;
			Assert(s->ubID == cnt);

			// Load the pMercPath
			if (!LoadMercPath(hFile, &s->pMercPath)) return FALSE;

			//do we have a 	KEY_ON_RING									*pKeyRing;

			// Read the file to see if we have to load the keys
			if (!FileRead(hFile, &ubOne, 1)) return FALSE;

			if( ubOne )
			{
				// Now Load the ....
				if (!FileRead(hFile, s->pKeyRing, NUM_KEYS * sizeof(KEY_ON_RING)))
				{
					return(FALSE);
				}

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

			//if the saved game version is before x, calculate the amount of money paid to mercs
			if( guiSaveGameVersion < 83 )
			{
				//if the soldier is someone
				if (s->ubProfile != NO_PROFILE)
				{
					MERCPROFILESTRUCT* const p = &gMercProfiles[s->ubProfile];
					if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
					{
						p->uiTotalCostToDate = p->sSalary * p->iMercMercContractLength;
					}
					else
					{
						p->uiTotalCostToDate = p->sSalary * s->iTotalContractLength;
					}
				}
			}

#ifdef GERMAN
			// Fix neutral flags
			if ( guiSaveGameVersion < 94 )
			{
				if (s->bTeam == OUR_TEAM && s->bNeutral && s->bAssignment != ASSIGNMENT_POW)
				{
					// turn off neutral flag
					s->bNeutral = FALSE;
				}
			}
#endif
			// JA2Gold: fix next-to-previous attacker value
			if ( guiSaveGameVersion < 99 )
			{
				s->next_to_previous_attacker = NULL;
			}
		}
	}

	// Fix robot
	if ( guiSaveGameVersion <= 87 )
	{
		MERCPROFILESTRUCT* const robot_p = &gMercProfiles[ROBOT];
		if (robot_p->inv[VESTPOS] == SPECTRA_VEST)
		{
			// update this
			robot_p->inv[VESTPOS]   = SPECTRA_VEST_18;
			robot_p->inv[HELMETPOS] = SPECTRA_HELMET_18;
			robot_p->inv[LEGPOS]    = SPECTRA_LEGGINGS_18;
			robot_p->bAgility = 50;
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

	return( TRUE );
}


#ifdef JA2BETAVERSION
static void WriteTempFileNameToFile(const char* pFileName, UINT32 uiSizeOfFile, HWFILE hSaveFile);
#endif


BOOLEAN SaveFilesToSavedGame( const char *pSrcFileName, HWFILE hFile )
{
	BOOLEAN Ret = FALSE;

	HWFILE hSrcFile = FileOpen(pSrcFileName, FILE_ACCESS_READ);
	if (!hSrcFile) goto ret_only;

	#ifdef JA2BETAVERSION
	guiNumberOfMapTempFiles++;		//Increment counter:  To determine where the temp files are crashing
	#endif


	//Get the file size of the source data file
	UINT32 uiFileSize = FileGetSize( hSrcFile );
	if (uiFileSize == 0) goto ret_close;

	// Write the the size of the file to the saved game file
	if (!FileWrite(hFile, &uiFileSize, sizeof(UINT32))) goto ret_close;

	//Allocate a buffer to read the data into
	UINT8* const pData = MALLOCN(UINT8, uiFileSize);
	if (pData == NULL) goto ret_close;

	// Read the saource file into the buffer
	if (!FileRead(hSrcFile, pData, uiFileSize)) goto ret_free;

	// Write the buffer to the saved game file
	if (!FileWrite(hFile, pData, uiFileSize)) goto ret_free;

#ifdef JA2BETAVERSION
	//Write out the name of the temp file so we can track whcih ones get loaded, and saved
	WriteTempFileNameToFile(pSrcFileName, uiFileSize, hFile);
#endif

	Ret = TRUE;

ret_free:
	MemFree(pData);
ret_close:
	FileClose(hSrcFile);
ret_only:
	return Ret;
}


BOOLEAN LoadFilesFromSavedGame( const char *pSrcFileName, HWFILE hFile )
{
	UINT32	uiFileSize;
	HWFILE	hSrcFile;

	#ifdef JA2BETAVERSION
	guiNumberOfMapTempFiles++;		//Increment counter:  To determine where the temp files are crashing
	#endif

	//open the destination file to write to
	hSrcFile = FileOpen(pSrcFileName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
	if( !hSrcFile )
	{
		//error, we cant open the saved game file
		return( FALSE );
	}


	// Read the size of the data
	if (!FileRead(hFile, &uiFileSize, sizeof(UINT32)))
	{
		FileClose( hSrcFile );

		return(FALSE);
	}


	//if there is nothing in the file, return;
	if( uiFileSize == 0 )
	{
		FileClose( hSrcFile );
		return( TRUE );
	}

	//Allocate a buffer to read the data into
	UINT8* pData = MALLOCN(UINT8, uiFileSize);
	if( pData == NULL )
	{
		FileClose( hSrcFile );
		return( FALSE );
	}


	// Read into the buffer
	if (!FileRead(hFile, pData, uiFileSize))
	{
		FileClose( hSrcFile );

		//Free the buffer
		MemFree( pData );

		return(FALSE);
	}



	// Write the buffer to the new file
	if (!FileWrite(hSrcFile, pData, uiFileSize))
	{
		FileClose( hSrcFile );
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("FAILED to Write to the %s File", pSrcFileName ) );
		//Free the buffer
		MemFree( pData );

		return(FALSE);
	}

	//Free the buffer
	MemFree( pData );

	//Close the source data file
	FileClose( hSrcFile );

	#ifdef JA2BETAVERSION
		WriteTempFileNameToFile( pSrcFileName, uiFileSize, hFile );
	#endif
	return( TRUE );
}


static BOOLEAN SaveTacticalStatusToSavedGame(HWFILE hFile)
{
	if (!InjectTacticalStatusTypeIntoFile(hFile)) return FALSE;

	//
	//Save the current sector location to the saved game file
	//

	// save gWorldSectorX
	if (!FileWrite(hFile, &gWorldSectorX, sizeof(gWorldSectorX))) return FALSE;

	// save gWorldSectorY
	if (!FileWrite(hFile, &gWorldSectorY, sizeof(gWorldSectorY))) return FALSE;

	// save gbWorldSectorZ
	if (!FileWrite(hFile, &gbWorldSectorZ, sizeof(gbWorldSectorZ))) return FALSE;

	return( TRUE );
}


static BOOLEAN LoadTacticalStatusFromSavedGame(HWFILE hFile)
{
	if (!ExtractTacticalStatusTypeFromFile(hFile)) return FALSE;

	//
	//Load the current sector location to the saved game file
	//

	// Load gWorldSectorX
	if (!FileRead(hFile, &gWorldSectorX, sizeof(gWorldSectorX))) return FALSE;

	// Load gWorldSectorY
	if (!FileRead(hFile, &gWorldSectorY, sizeof(gWorldSectorY))) return FALSE;


	// Load gbWorldSectorZ
	if (!FileRead(hFile, &gbWorldSectorZ, sizeof(gbWorldSectorZ))) return FALSE;

	return( TRUE );
}


static BOOLEAN SaveOppListInfoToSavedGame(HWFILE hFile)
{
	UINT32	uiSaveSize=0;

	// Save the Public Opplist
	uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
	if (!FileWrite(hFile, gbPublicOpplist, uiSaveSize)) return FALSE;

	// Save the Seen Oppenents
	uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	if (!FileWrite(hFile, gbSeenOpponents, uiSaveSize)) return FALSE;

	// Save the Last Known Opp Locations
	uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS; // XXX TODO000F
	if (!FileWrite(hFile, gsLastKnownOppLoc, uiSaveSize)) return FALSE;

	// Save the Last Known Opp Level
	uiSaveSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	if (!FileWrite(hFile, gbLastKnownOppLevel, uiSaveSize)) return FALSE;

	// Save the Public Last Known Opp Locations
	uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS; // XXX TODO000F
	if (!FileWrite(hFile, gsPublicLastKnownOppLoc, uiSaveSize)) return FALSE;

	// Save the Public Last Known Opp Level
	uiSaveSize = MAXTEAMS * TOTAL_SOLDIERS;
	if (!FileWrite(hFile, gbPublicLastKnownOppLevel, uiSaveSize)) return FALSE;

	// Save the Public Noise Volume
	uiSaveSize = MAXTEAMS;
	if (!FileWrite(hFile, gubPublicNoiseVolume, uiSaveSize)) return FALSE;

	// Save the Public Last Noise Gridno
	uiSaveSize = MAXTEAMS; // XXX TODO000F
	if (!FileWrite(hFile, gsPublicNoiseGridno, uiSaveSize)) return FALSE;

	return( TRUE );
}


static BOOLEAN LoadOppListInfoFromSavedGame(HWFILE hFile)
{
	UINT32	uiLoadSize=0;

	// Load the Public Opplist
	uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
	if (!FileRead(hFile, gbPublicOpplist, uiLoadSize)) return FALSE;

	// Load the Seen Oppenents
	uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	if (!FileRead(hFile, gbSeenOpponents, uiLoadSize)) return FALSE;

	// Load the Last Known Opp Locations
	uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS; // XXX TODO000F
	if (!FileRead(hFile, gsLastKnownOppLoc, uiLoadSize)) return FALSE;

	// Load the Last Known Opp Level
	uiLoadSize = TOTAL_SOLDIERS * TOTAL_SOLDIERS;
	if (!FileRead(hFile, gbLastKnownOppLevel, uiLoadSize)) return FALSE;

	// Load the Public Last Known Opp Locations
	uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS; // XXX TODO000F
	if (!FileRead(hFile, gsPublicLastKnownOppLoc, uiLoadSize)) return FALSE;

	// Load the Public Last Known Opp Level
	uiLoadSize = MAXTEAMS * TOTAL_SOLDIERS;
	if (!FileRead(hFile, gbPublicLastKnownOppLevel, uiLoadSize)) return FALSE;

	// Load the Public Noise Volume
	uiLoadSize = MAXTEAMS;
	if (!FileRead(hFile, gubPublicNoiseVolume, uiLoadSize)) return FALSE;

	// Load the Public Last Noise Gridno
	uiLoadSize = MAXTEAMS; // XXX TODO000F
	if (!FileRead(hFile, gsPublicNoiseGridno, uiLoadSize)) return FALSE;

	return( TRUE );
}


static BOOLEAN SaveWatchedLocsToSavedGame(HWFILE hFile)
{
	UINT32	uiArraySize;
	UINT32	uiSaveSize=0;

	uiArraySize = TOTAL_SOLDIERS * NUM_WATCHED_LOCS;

	// save locations of watched points
	uiSaveSize = uiArraySize * sizeof( INT16 );
	if (!FileWrite(hFile, gsWatchedLoc, uiSaveSize)) return FALSE;

	uiSaveSize = uiArraySize * sizeof( INT8 );

	if (!FileWrite(hFile, gbWatchedLocLevel, uiSaveSize)) return FALSE;

	if (!FileWrite(hFile, gubWatchedLocPoints, uiSaveSize)) return FALSE;

	if (!FileWrite(hFile, gfWatchedLocReset, uiSaveSize)) return FALSE;

	return( TRUE );
}


static BOOLEAN LoadWatchedLocsFromSavedGame(HWFILE hFile)
{
	UINT32	uiArraySize;
	UINT32	uiLoadSize=0;

	uiArraySize = TOTAL_SOLDIERS * NUM_WATCHED_LOCS;

	uiLoadSize = uiArraySize * sizeof( INT16 );
	if (!FileRead(hFile, gsWatchedLoc, uiLoadSize)) return FALSE;

	uiLoadSize = uiArraySize * sizeof( INT8 );
	if (!FileRead(hFile, gbWatchedLocLevel, uiLoadSize)) return FALSE;

	if (!FileRead(hFile, gubWatchedLocPoints, uiLoadSize)) return FALSE;

	if (!FileRead(hFile, gfWatchedLocReset, uiLoadSize)) return FALSE;

	return( TRUE );
}


void CreateSavedGameFileNameFromNumber(const UINT8 ubSaveGameID, char* const pzNewFileName)
{
	const char* const dir = g_savegame_dir;
	const char* const ext = g_savegame_ext;

	//if we are creating the QuickSave file
	if (ubSaveGameID == 0)
	{
		const char* const quick = g_quicksave_name;
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
	}
	else if (ubSaveGameID == SAVE__END_TURN_NUM)
	{
		//The name of the file
		sprintf(pzNewFileName, "%s/Auto%02d.%s", dir, guiLastSaveGameNum, ext);

		guiLastSaveGameNum = (guiLastSaveGameNum + 1) % 2;
	}
	else
	{
		sprintf(pzNewFileName, "%s/%s%02d.%s", dir, g_savegame_name, ubSaveGameID, ext);
	}
}


BOOLEAN SaveMercPath(const HWFILE f, const PathSt* const head)
{
	UINT32 n_nodes = 0;
	for (const PathSt* p = head; p != NULL; p = p->pNext) ++n_nodes;
	if (!FileWrite(f, &n_nodes, sizeof(UINT32))) return FALSE;

	for (const PathSt* p = head; p != NULL; p = p->pNext)
	{
		BYTE  data[20];
		BYTE* d = data;
		INJ_U32(d, p->uiSectorId)
		INJ_U32(d, p->uiEta)
		INJ_BOOL(d, p->fSpeed)
		INJ_SKIP(d, 11)
		Assert(d == endof(data));

		if (!FileWrite(f, data, sizeof(data))) return FALSE;
	}
	return TRUE;
}


BOOLEAN LoadMercPath(const HWFILE hFile, PathSt** const head)
{
	//Load the number of the nodes
	UINT32 uiNumOfNodes = 0;
	if (!FileRead(hFile, &uiNumOfNodes, sizeof(UINT32))) return FALSE;

	//load all the nodes
	PathSt* path = NULL;
	for (UINT32 cnt = 0; cnt < uiNumOfNodes; ++cnt)
	{
		PathSt* const n = MALLOC(PathSt);
		if (n == NULL) return FALSE;

		BYTE data[20];
		if (!FileRead(hFile, data, sizeof(data))) return FALSE;

		const BYTE* d = data;
		EXTR_U32(d, n->uiSectorId)
		EXTR_U32(d, n->uiEta)
		EXTR_BOOL(d, n->fSpeed)
		EXTR_SKIP(d, 11)
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
	return TRUE;
}


#ifdef JA2BETAVERSION
static void InitSaveGameFilePosition(void)
{
	CHAR8		zFileName[128];
	sprintf(zFileName, "%s/SaveGameFilePos%2d.txt", g_savegame_dir, gubSaveGameLoc);
	FileDelete( zFileName );
}


static void SaveGameFilePosition(const HWFILE save, const char* const pMsg)
{
	CHAR8		zTempString[512];
	UINT32	uiStrLen=0;
	CHAR8		zFileName[128];

	sprintf(zFileName, "%s/SaveGameFilePos%2d.txt", g_savegame_dir, gubSaveGameLoc);

	// create the save game file
	const HWFILE hFile = FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS);
	if( !hFile )
	{
		return;
	}

	const INT32 pos = FileGetPos(save);
	sprintf(zTempString, "%8d     %s\n", pos, pMsg);
	uiStrLen = strlen( zTempString );

	if (!FileWrite(hFile, zTempString, uiStrLen))
	{
		FileClose( hFile );
		return;
	}

	FileClose( hFile );
}


static void InitLoadGameFilePosition(void)
{
	CHAR8		zFileName[128];
	sprintf(zFileName, "%s/LoadGameFilePos%2d.txt", g_savegame_dir, gubSaveGameLoc);
	FileDelete( zFileName );
}


static void LoadGameFilePosition(const HWFILE load, const char* const pMsg)
{
	CHAR8		zTempString[512];
	UINT32	uiStrLen=0;

	CHAR8		zFileName[128];
	sprintf(zFileName, "%s/LoadGameFilePos%2d.txt", g_savegame_dir, gubSaveGameLoc);

	// create the save game file
	const HWFILE hFile = FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS);
	if( !hFile )
	{
		return;
	}

	const INT32 pos = FileGetPos(load);
	sprintf(zTempString, "%8d     %s\n", pos, pMsg);
	uiStrLen = strlen( zTempString );

	if (!FileWrite(hFile, zTempString, uiStrLen))
	{
		FileClose( hFile );
		return;
	}

	FileClose( hFile );


}
#endif


static BOOLEAN SaveGeneralInfo(HWFILE hFile)
{
	GENERAL_SAVE_INFO sGeneralInfo;
	memset( &sGeneralInfo, 0, sizeof( GENERAL_SAVE_INFO ) );

	sGeneralInfo.ubMusicMode = gubMusicMode;
	sGeneralInfo.uiCurrentUniqueSoldierId = guiCurrentUniqueSoldierId;
	sGeneralInfo.uiCurrentScreen = guiPreviousOptionScreen;

	sGeneralInfo.usSelectedSoldier = Soldier2ID(GetSelectedMan());
	sGeneralInfo.sRenderCenterX = gsRenderCenterX;
	sGeneralInfo.sRenderCenterY = gsRenderCenterY;
	sGeneralInfo.fAtLeastOneMercWasHired = gfAtLeastOneMercWasHired;
	sGeneralInfo.fHavePurchasedItemsFromTony_UNUSED = FALSE;

	sGeneralInfo.fShowItemsFlag			= fShowItemsFlag;
	sGeneralInfo.fShowTownFlag			= fShowTownFlag;
	sGeneralInfo.fShowMineFlag			= fShowMineFlag;
	sGeneralInfo.fShowAircraftFlag	= fShowAircraftFlag;
	sGeneralInfo.fShowTeamFlag			= fShowTeamFlag;

	sGeneralInfo.fHelicopterAvailable = fHelicopterAvailable;

	// helicopter vehicle id
	sGeneralInfo.iHelicopterVehicleId = iHelicopterVehicleId;

	// total owed by player
	sGeneralInfo.iTotalAccumulatedCostByPlayer = iTotalAccumulatedCostByPlayer;

	// whether or not skyrider is alive and well? and on our side yet?
	sGeneralInfo.fSkyRiderAvailable = fSkyRiderAvailable;

	// is the heli in the air?
	sGeneralInfo.fHelicopterIsAirBorne = fHelicopterIsAirBorne;

	// is the pilot returning straight to base?
	sGeneralInfo.fHeliReturnStraightToBase = fHeliReturnStraightToBase;

	// heli hovering
	sGeneralInfo.fHoveringHelicopter = fHoveringHelicopter;

	// time started hovering
	sGeneralInfo.uiStartHoverTime = uiStartHoverTime;

	// what state is skyrider's dialogue in in?
	sGeneralInfo.uiHelicopterSkyriderTalkState = guiHelicopterSkyriderTalkState;

	// the flags for skyrider events
	sGeneralInfo.fShowEstoniRefuelHighLight = fShowEstoniRefuelHighLight;
	sGeneralInfo.fShowOtherSAMHighLight = fShowOtherSAMHighLight;
	sGeneralInfo.fShowDrassenSAMHighLight = fShowDrassenSAMHighLight;
	sGeneralInfo.fShowCambriaHospitalHighLight = fShowCambriaHospitalHighLight;

	//The current state of the weather
	sGeneralInfo.uiEnvWeather = guiEnvWeather;

	sGeneralInfo.ubDefaultButton = gubDefaultButton;

	sGeneralInfo.fSkyriderEmptyHelpGiven = gfSkyriderEmptyHelpGiven;
	sGeneralInfo.ubHelicopterHitsTaken = gubHelicopterHitsTaken;
	sGeneralInfo.fSkyriderSaidCongratsOnTakingSAM = gfSkyriderSaidCongratsOnTakingSAM;
	sGeneralInfo.ubPlayerProgressSkyriderLastCommentedOn = gubPlayerProgressSkyriderLastCommentedOn;

	sGeneralInfo.fEnterMapDueToContract = fEnterMapDueToContract;
	sGeneralInfo.ubQuitType = 0;

	if( pContractReHireSoldier != NULL )
		sGeneralInfo.sContractRehireSoldierID = pContractReHireSoldier->ubID;
	else
		sGeneralInfo.sContractRehireSoldierID = -1;

	sGeneralInfo.GameOptions = gGameOptions;


	#ifdef JA2BETAVERSION
	//Everytime we save get, and set a seed value, when reload, seed again
	sGeneralInfo.uiSeedNumber = GetJA2Clock();
	srand( sGeneralInfo.uiSeedNumber );
	#endif

	//Save the Ja2Clock()
	sGeneralInfo.uiBaseJA2Clock = guiBaseJA2Clock;

	// Save the current tactical panel mode
	sGeneralInfo.sCurInterfacePanel = gsCurInterfacePanel;

	// Save the selected merc
	if( gpSMCurrentMerc )
		sGeneralInfo.ubSMCurrentMercID = gpSMCurrentMerc->ubID;
	else
		sGeneralInfo.ubSMCurrentMercID = 255;

	//save map screen disabling buttons
	sGeneralInfo.fDisableDueToBattleRoster = fDisableDueToBattleRoster;
	sGeneralInfo.fDisableMapInterfaceDueToBattle = fDisableMapInterfaceDueToBattle;

	// Save boxing info
	memcpy( &sGeneralInfo.sBoxerGridNo, &gsBoxerGridNo, NUM_BOXERS * sizeof( INT16 ) );
	for (UINT i = 0; i < lengthof(gBoxer); ++i)
	{
		sGeneralInfo.ubBoxerID[i] = Soldier2ID(gBoxer[i]);
	}
	memcpy( &sGeneralInfo.fBoxerFought, &gfBoxerFought, NUM_BOXERS * sizeof( BOOLEAN ) );

	//Save the helicopter status
	sGeneralInfo.fHelicopterDestroyed = fHelicopterDestroyed;

	sGeneralInfo.iSortStateForMapScreenList = giSortStateForMapScreenList;
	sGeneralInfo.fFoundTixa = fFoundTixa;

	sGeneralInfo.uiTimeOfLastSkyriderMonologue = guiTimeOfLastSkyriderMonologue;
	sGeneralInfo.fSkyRiderSetUp = fSkyRiderSetUp;

	memcpy( &sGeneralInfo.fRefuelingSiteAvailable, &fRefuelingSiteAvailable, NUMBER_OF_REFUEL_SITES * sizeof( BOOLEAN ) );


	//Meanwhile stuff
	sGeneralInfo.gCurrentMeanwhileDef = gCurrentMeanwhileDef;
	sGeneralInfo.gfMeanwhileTryingToStart = gfMeanwhileTryingToStart;
	sGeneralInfo.gfInMeanwhile = gfInMeanwhile;


	// list of dead guys for squads...in id values -> -1 means no one home
	memcpy( &sGeneralInfo.sDeadMercs, &sDeadMercs, sizeof( INT16 ) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD );

	// level of public noises
	memcpy( &sGeneralInfo.gbPublicNoiseLevel, &gbPublicNoiseLevel, sizeof( INT8 ) * MAXTEAMS );

	//The screen count for the initscreen
	sGeneralInfo.gubScreenCount = gubScreenCount;


	//used for the mean while screen
	sGeneralInfo.uiMeanWhileFlags = uiMeanWhileFlags;

	//Imp portrait number
	sGeneralInfo.iPortraitNumber = iPortraitNumber;

	// location of first enocunter with enemy
	sGeneralInfo.sWorldSectorLocationOfFirstBattle = sWorldSectorLocationOfFirstBattle;


	//State of email flags
	sGeneralInfo.fUnReadMailFlag = fUnReadMailFlag;
	sGeneralInfo.fNewMailFlag = fNewMailFlag;
	sGeneralInfo.fOldUnReadFlag = FALSE; // XXX HACK000B
	sGeneralInfo.fOldNewMailFlag = fOldNewMailFlag;

	sGeneralInfo.fShowMilitia	= fShowMilitia;

	sGeneralInfo.fNewFilesInFileViewer = fNewFilesInFileViewer;

	sGeneralInfo.fLastBoxingMatchWonByPlayer = gfLastBoxingMatchWonByPlayer;

	memcpy( &sGeneralInfo.fSamSiteFound, &fSamSiteFound, NUMBER_OF_SAMS * sizeof( BOOLEAN ) );

	sGeneralInfo.ubNumTerrorists = gubNumTerrorists;
	sGeneralInfo.ubCambriaMedicalObjects = gubCambriaMedicalObjects;

	sGeneralInfo.fDisableTacticalPanelButtons = gfDisableTacticalPanelButtons;

	sGeneralInfo.sSelMapX						= sSelMapX;
	sGeneralInfo.sSelMapY						= sSelMapY;
	sGeneralInfo.iCurrentMapSectorZ	= iCurrentMapSectorZ;

	//Save the current status of the help screens flag that say wether or not the user has been there before
	sGeneralInfo.usHasPlayerSeenHelpScreenInCurrentScreen = gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen;

	sGeneralInfo.ubBoxingMatchesWon = gubBoxingMatchesWon;
	sGeneralInfo.ubBoxersRests = gubBoxersRests;
	sGeneralInfo.fBoxersResting = gfBoxersResting;

	sGeneralInfo.ubDesertTemperature = gubDesertTemperature;
	sGeneralInfo.ubGlobalTemperature = gubGlobalTemperature;

	sGeneralInfo.sMercArriveSectorX	 = gsMercArriveSectorX;
	sGeneralInfo.sMercArriveSectorY	 = gsMercArriveSectorY;

	sGeneralInfo.fCreatureMeanwhileScenePlayed = gfCreatureMeanwhileScenePlayed;

	//save the global player num
	sGeneralInfo.ubPlayerNum = gbPlayerNum;

	//New stuff for the Prebattle interface / autoresolve
	sGeneralInfo.fPersistantPBI									= gfPersistantPBI;
	sGeneralInfo.ubEnemyEncounterCode						= gubEnemyEncounterCode;
	sGeneralInfo.ubExplicitEnemyEncounterCode		= gubExplicitEnemyEncounterCode;
	sGeneralInfo.fBlitBattleSectorLocator				= gfBlitBattleSectorLocator;
	sGeneralInfo.ubPBSectorX										= gubPBSectorX;
	sGeneralInfo.ubPBSectorY										= gubPBSectorY;
	sGeneralInfo.ubPBSectorZ										= gubPBSectorZ;
	sGeneralInfo.fCantRetreatInPBI							= gfCantRetreatInPBI;
	sGeneralInfo.fExplosionQueueActive					= gfExplosionQueueActive;

	sGeneralInfo.bSelectedInfoChar							= bSelectedInfoChar;

	sGeneralInfo.iHospitalTempBalance						= giHospitalTempBalance;
	sGeneralInfo.iHospitalRefund								= giHospitalRefund;
	sGeneralInfo.bHospitalPriceModifier					= gbHospitalPriceModifier;
  sGeneralInfo.fPlayerTeamSawJoey             = gfPlayerTeamSawJoey;
	sGeneralInfo.fMikeShouldSayHi								= gfMikeShouldSayHi;

	return FileWrite(hFile, &sGeneralInfo, sizeof(GENERAL_SAVE_INFO));
}


static BOOLEAN LoadGeneralInfo(HWFILE hFile)
{
	GENERAL_SAVE_INFO sGeneralInfo;
	memset( &sGeneralInfo, 0, sizeof( GENERAL_SAVE_INFO ) );


	//Load the current music mode
	if (!FileRead(hFile, &sGeneralInfo, sizeof(GENERAL_SAVE_INFO))) return FALSE;

	gMusicModeToPlay = sGeneralInfo.ubMusicMode;

	guiCurrentUniqueSoldierId = sGeneralInfo.uiCurrentUniqueSoldierId;

	guiScreenToGotoAfterLoadingSavedGame = sGeneralInfo.uiCurrentScreen;

	SetSelectedMan(ID2Soldier(sGeneralInfo.usSelectedSoldier));

	gsRenderCenterX = sGeneralInfo.sRenderCenterX;
	gsRenderCenterY = sGeneralInfo.sRenderCenterY;

	gfAtLeastOneMercWasHired = sGeneralInfo.fAtLeastOneMercWasHired;


	fShowItemsFlag		= sGeneralInfo.fShowItemsFlag;
	fShowTownFlag			= sGeneralInfo.fShowTownFlag;
	fShowMineFlag			= sGeneralInfo.fShowMineFlag;
	fShowAircraftFlag	= sGeneralInfo.fShowAircraftFlag;
	fShowTeamFlag			= sGeneralInfo.fShowTeamFlag;

	fHelicopterAvailable = sGeneralInfo.fHelicopterAvailable;

	// helicopter vehicle id
	iHelicopterVehicleId = sGeneralInfo.iHelicopterVehicleId;

	// total owed to player
	iTotalAccumulatedCostByPlayer = sGeneralInfo.iTotalAccumulatedCostByPlayer;

	// whether or not skyrider is alive and well? and on our side yet?
	fSkyRiderAvailable = sGeneralInfo.fSkyRiderAvailable;

	// is the heli in the air?
	fHelicopterIsAirBorne = sGeneralInfo.fHelicopterIsAirBorne;

	// is the pilot returning straight to base?
	fHeliReturnStraightToBase = sGeneralInfo.fHeliReturnStraightToBase;

	// heli hovering
	fHoveringHelicopter = sGeneralInfo.fHoveringHelicopter;

	// time started hovering
	uiStartHoverTime = sGeneralInfo.uiStartHoverTime;

	// what state is skyrider's dialogue in in?
	guiHelicopterSkyriderTalkState = sGeneralInfo.uiHelicopterSkyriderTalkState;

	// the flags for skyrider events
	fShowEstoniRefuelHighLight = sGeneralInfo.fShowEstoniRefuelHighLight;
	fShowOtherSAMHighLight = sGeneralInfo.fShowOtherSAMHighLight;
	fShowDrassenSAMHighLight = sGeneralInfo.fShowDrassenSAMHighLight;
	fShowCambriaHospitalHighLight = sGeneralInfo.fShowCambriaHospitalHighLight;

	//The current state of the weather
	guiEnvWeather = sGeneralInfo.uiEnvWeather;

	gubDefaultButton = sGeneralInfo.ubDefaultButton;

	gfSkyriderEmptyHelpGiven = sGeneralInfo.fSkyriderEmptyHelpGiven;
	gubHelicopterHitsTaken = sGeneralInfo.ubHelicopterHitsTaken;
	gfSkyriderSaidCongratsOnTakingSAM = sGeneralInfo.fSkyriderSaidCongratsOnTakingSAM;
	gubPlayerProgressSkyriderLastCommentedOn = sGeneralInfo.ubPlayerProgressSkyriderLastCommentedOn;

	fEnterMapDueToContract = sGeneralInfo.fEnterMapDueToContract;

	//if the soldier id is valid
	if( sGeneralInfo.sContractRehireSoldierID == -1 )
		pContractReHireSoldier = NULL;
	else
		pContractReHireSoldier = GetMan(sGeneralInfo.sContractRehireSoldierID);

	gGameOptions = sGeneralInfo.GameOptions;

	#ifdef JA2BETAVERSION
	//Reset the random 'seed' number
	srand( sGeneralInfo.uiSeedNumber );
	#endif

	//Restore the JA2 Clock
	guiBaseJA2Clock = sGeneralInfo.uiBaseJA2Clock;

	// whenever guiBaseJA2Clock changes, we must reset all the timer variables that use it as a reference
	ResetJA2ClockGlobalTimers();


	// Restore the selected merc
	if( sGeneralInfo.ubSMCurrentMercID == 255 )
		gpSMCurrentMerc = NULL;
	else
		gpSMCurrentMerc = GetMan(sGeneralInfo.ubSMCurrentMercID);

	//Set the interface panel to the team panel
	ShutdownCurrentPanel( );

	//Restore the current tactical panel mode
	gsCurInterfacePanel = sGeneralInfo.sCurInterfacePanel;

	/*
	//moved to last stage in the LoadSaveGame() function
	//if we are in tactical
	if( guiScreenToGotoAfterLoadingSavedGame == GAME_SCREEN )
	{
		//Initialize the current panel
		InitializeCurrentPanel( );

		SelectSoldier(GetSelectedMan(), SELSOLDIER_FORCE_RESELECT);
	}
	*/

	//Load map screen disabling buttons
	fDisableDueToBattleRoster = sGeneralInfo.fDisableDueToBattleRoster;
	fDisableMapInterfaceDueToBattle = sGeneralInfo.fDisableMapInterfaceDueToBattle;

	memcpy( &gsBoxerGridNo, &sGeneralInfo.sBoxerGridNo, NUM_BOXERS * sizeof( INT16 ) );
	for (UINT i = 0; i < lengthof(gBoxer); ++i)
	{
		gBoxer[i] = ID2Soldier(sGeneralInfo.ubBoxerID[i]);
	}
	memcpy( &gfBoxerFought, &sGeneralInfo.fBoxerFought, NUM_BOXERS * sizeof( BOOLEAN ) );

	//Load the helicopter status
	fHelicopterDestroyed = sGeneralInfo.fHelicopterDestroyed;

	giSortStateForMapScreenList = sGeneralInfo.iSortStateForMapScreenList;
	fFoundTixa = sGeneralInfo.fFoundTixa;

	guiTimeOfLastSkyriderMonologue = sGeneralInfo.uiTimeOfLastSkyriderMonologue;
	fSkyRiderSetUp = sGeneralInfo.fSkyRiderSetUp;

	memcpy( &fRefuelingSiteAvailable, &sGeneralInfo.fRefuelingSiteAvailable, NUMBER_OF_REFUEL_SITES * sizeof( BOOLEAN ) );


	//Meanwhile stuff
	gCurrentMeanwhileDef = sGeneralInfo.gCurrentMeanwhileDef;
	gfMeanwhileTryingToStart = sGeneralInfo.gfMeanwhileTryingToStart;
	gfInMeanwhile = sGeneralInfo.gfInMeanwhile;

	// list of dead guys for squads...in id values -> -1 means no one home
	memcpy( &sDeadMercs, &sGeneralInfo.sDeadMercs, sizeof( INT16 ) * NUMBER_OF_SQUADS * NUMBER_OF_SOLDIERS_PER_SQUAD );

	// level of public noises
	memcpy( &gbPublicNoiseLevel, &sGeneralInfo.gbPublicNoiseLevel, sizeof( INT8 ) * MAXTEAMS );

	//the screen count for the init screen
	gubScreenCount = sGeneralInfo.gubScreenCount;

	//used for the mean while screen
	if ( guiSaveGameVersion < 71 )
	{
		uiMeanWhileFlags = sGeneralInfo.usOldMeanWhileFlags;
	}
	else
	{
		uiMeanWhileFlags = sGeneralInfo.uiMeanWhileFlags;
	}

	//Imp portrait number
	iPortraitNumber = sGeneralInfo.iPortraitNumber;

	// location of first enocunter with enemy
	sWorldSectorLocationOfFirstBattle = sGeneralInfo.sWorldSectorLocationOfFirstBattle;

	fShowMilitia	= sGeneralInfo.fShowMilitia;

	fNewFilesInFileViewer = sGeneralInfo.fNewFilesInFileViewer;

	gfLastBoxingMatchWonByPlayer = sGeneralInfo.fLastBoxingMatchWonByPlayer;

	memcpy( &fSamSiteFound, &sGeneralInfo.fSamSiteFound, NUMBER_OF_SAMS * sizeof( BOOLEAN ) );

	gubNumTerrorists = sGeneralInfo.ubNumTerrorists;
	gubCambriaMedicalObjects = sGeneralInfo.ubCambriaMedicalObjects;

	gfDisableTacticalPanelButtons = sGeneralInfo.fDisableTacticalPanelButtons;

	sSelMapX						= sGeneralInfo.sSelMapX;
	sSelMapY						= sGeneralInfo.sSelMapY;
	iCurrentMapSectorZ	= sGeneralInfo.iCurrentMapSectorZ;

	//State of email flags
	fUnReadMailFlag = sGeneralInfo.fUnReadMailFlag;
	fNewMailFlag = sGeneralInfo.fNewMailFlag;
	fOldNewMailFlag = sGeneralInfo.fOldNewMailFlag;

	//Save the current status of the help screens flag that say wether or not the user has been there before
	gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen = sGeneralInfo.usHasPlayerSeenHelpScreenInCurrentScreen;

	gubBoxingMatchesWon = sGeneralInfo.ubBoxingMatchesWon;
	gubBoxersRests = sGeneralInfo.ubBoxersRests;
	gfBoxersResting = sGeneralInfo.fBoxersResting;

	gubDesertTemperature = sGeneralInfo.ubDesertTemperature;
	gubGlobalTemperature = sGeneralInfo.ubGlobalTemperature;

	gsMercArriveSectorX = sGeneralInfo.sMercArriveSectorX;
	gsMercArriveSectorY = sGeneralInfo.sMercArriveSectorY;

	gfCreatureMeanwhileScenePlayed = sGeneralInfo.fCreatureMeanwhileScenePlayed;

	//load the global player num
	gbPlayerNum = sGeneralInfo.ubPlayerNum;

	//New stuff for the Prebattle interface / autoresolve
	gfPersistantPBI									= sGeneralInfo.fPersistantPBI;
	gubEnemyEncounterCode						= sGeneralInfo.ubEnemyEncounterCode;
	gubExplicitEnemyEncounterCode		= sGeneralInfo.ubExplicitEnemyEncounterCode;
	gfBlitBattleSectorLocator				= sGeneralInfo.fBlitBattleSectorLocator;
	gubPBSectorX										= sGeneralInfo.ubPBSectorX;
	gubPBSectorY										= sGeneralInfo.ubPBSectorY;
	gubPBSectorZ										= sGeneralInfo.ubPBSectorZ;
	gfCantRetreatInPBI							= sGeneralInfo.fCantRetreatInPBI;
	gfExplosionQueueActive					= sGeneralInfo.fExplosionQueueActive;

	bSelectedInfoChar	= sGeneralInfo.bSelectedInfoChar;

	giHospitalTempBalance		= sGeneralInfo.iHospitalTempBalance;
	giHospitalRefund				= sGeneralInfo.iHospitalRefund;
	gbHospitalPriceModifier = sGeneralInfo.bHospitalPriceModifier;
  gfPlayerTeamSawJoey     = sGeneralInfo.fPlayerTeamSawJoey;
	gfMikeShouldSayHi				= sGeneralInfo.fMikeShouldSayHi;

	return( TRUE );
}


static BOOLEAN SavePreRandomNumbersToSaveGameFile(HWFILE hFile)
{
	//Save the Prerandom number index
	if (!FileWrite(hFile, &guiPreRandomIndex, sizeof(UINT32))) return FALSE;

	//Save the Prerandom number index
	if (!FileWrite(hFile, guiPreRandomNums, sizeof(UINT32) * MAX_PREGENERATED_NUMS)) return FALSE;

	return( TRUE );
}


static BOOLEAN LoadPreRandomNumbersFromSaveGameFile(HWFILE hFile)
{
	//Load the Prerandom number index
	if (!FileRead(hFile, &guiPreRandomIndex, sizeof(UINT32))) return FALSE;

	//Load the Prerandom number index
	if (!FileRead(hFile, guiPreRandomNums, sizeof(UINT32) * MAX_PREGENERATED_NUMS)) return FALSE;

	return( TRUE );
}


static BOOLEAN LoadMeanwhileDefsFromSaveGameFile(HWFILE hFile)
{
	if ( guiSaveGameVersion < 72 )
	{
		//Load the array of meanwhile defs
		if (!FileRead(hFile, gMeanwhileDef, sizeof(MEANWHILE_DEFINITION) * (NUM_MEANWHILES - 1))) return FALSE;
		// and set the last one
		memset( &(gMeanwhileDef[ NUM_MEANWHILES - 1]), 0, sizeof( MEANWHILE_DEFINITION ) );
	}
	else
	{
		//Load the array of meanwhile defs
		if (!FileRead(hFile, gMeanwhileDef, sizeof(MEANWHILE_DEFINITION) * NUM_MEANWHILES)) return FALSE;
	}

	return( TRUE );
}


static BOOLEAN SaveMeanwhileDefsFromSaveGameFile(HWFILE hFile)
{
	//Save the array of meanwhile defs
	if (!FileWrite(hFile, &gMeanwhileDef, sizeof(MEANWHILE_DEFINITION) * NUM_MEANWHILES)) return FALSE;

	return( TRUE );
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
		const HWFILE hFile = FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS);
		if( !hFile )
		{
			return;
		}

		sprintf( zTempString, "Number Of Files: %6d.  Size of all files: %6d.\n", guiNumberOfMapTempFiles, guiSizeOfTempFiles );
		uiStrLen = strlen( zTempString );

		if (!FileWrite(hFile, zTempString, uiStrLen))
		{
			FileClose( hFile );
			return;
		}

		FileClose( hFile );

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
	const HWFILE hFile = FileOpen(zFileName, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS);
	if( !hFile )
	{
		return;
	}

	sprintf( zTempString, "%8d   %6d   %s\n", FileGetPos( hSaveFile ), uiSizeOfFile, pFileName );
	uiStrLen = strlen( zTempString );

	if (!FileWrite(hFile, zTempString, uiStrLen))
	{
		FileClose( hFile );
		return;
	}

	FileClose( hFile );
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
	HWFILE	hFile;
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
		hFile = FileOpen(zFileName1, FILE_ACCESS_READ);

		GetFileManFileTime( hFile, &CreationTime1, &LastAccessedTime1, &LastWriteTime1 );

		FileClose( hFile );

		fFile1Exist = TRUE;
	}

	if( FileExists( zFileName2 ) )
	{
		hFile = FileOpen(zFileName2, FILE_ACCESS_READ);

		GetFileManFileTime( hFile, &CreationTime2, &LastAccessedTime2, &LastWriteTime2 );

		FileClose( hFile );

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
		//Allocate memory for the list
		gpNewBobbyrShipments = MALLOCN(NewBobbyRayOrderStruct, LaptopSaveInfo.usNumberOfBobbyRayOrderUsed);
		if( gpNewBobbyrShipments == NULL )
		{
			Assert(0);
			return;
		}

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

	uiEncryptionSet = pSaveGameHeader->uiSavedGameVersion;
	uiEncryptionSet *= pSaveGameHeader->uiFlags;
	uiEncryptionSet += pSaveGameHeader->iCurrentBalance;
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
