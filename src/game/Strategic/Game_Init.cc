#include "Game_Init.h"
#include "Arms_Dealer_Init.h"
#include "BloodCatPlacementsModel.h"
#include "BloodCatSpawnsModel.h"
#include "BobbyR.h"
#include "Campaign_Init.h"
#include "Campaign_Types.h"
#include "Cheats.h"
#include "ContentManager.h"
#include "ContentMusic.h"
#include "Creature_Spreading.h"
#include "Dialogue_Control.h"
#include "EMail.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "GameInstance.h"
#include "GameLoop.h"
#include "GamePolicy.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "Interface.h"
#include "Interface_Dialogue.h"
#include "Laptop.h"
#include "Logger.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Map.h"
#include "MapScreen.h"
#include "Meanwhile.h"
#include "Merc_Entering.h"
#include "Message.h"
#include "Music_Control.h"
#include "NPC.h"
#include "NpcPlacementModel.h"
#include "OppList.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Random.h"
#include "SAM_Sites.h"
#include "SaveLoadGameStates.h"
#include "ScreenIDs.h"
#include "ShopKeeper_Interface.h"
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"
#include "SoundMan.h"
#include "Squads.h"
#include "Strategic_AI.h"
#include "Strategic_Mines.h"
#include "Strategic_Movement.h"
#include "Strategic_Town_Loyalty.h"
#include "StrategicMap.h"
#include "StrategicMap_Secrets.h"
#include "Tactical_Save.h"
#include "Timer_Control.h"
#include "Vehicles.h"
#include "WorldDef.h"
#include <stdexcept>
#include <string_theory/format>

void InitScriptingEngine();

UINT8			gubScreenCount=0;


static void InitNPCs()
{
	for (auto p : GCM->listNpcPlacements())
	{
		const NpcPlacementModel* placement = p.second;
		if (placement->isSciFiOnly && !gGameOptions.fSciFi)
		{
			continue;
		}

		SGPSector sMap(placement->pickPlacementSector());
		if (placement->useAlternateMap)
		{
			SectorInfo[sMap.AsByte()].uiFlags |= SF_USE_ALTERNATE_MAP;
			SLOGD("Alternate map in {}", sMap.AsShortString());
		}
		if (placement->isPlacedAtStart)
		{
			MERCPROFILESTRUCT& merc = GetProfile(placement->profileId);
			merc.sSector = sMap;
			SLOGD("{} in {}", merc.zNickname, sMap.AsShortString());
		}
	}

	gfPlayerTeamSawJoey = FALSE;

	if (!gGameOptions.fSciFi)
	{ //not scifi, so use alternate map in Tixa's b1 level that doesn't have the stairs going down to the caves.
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector(SGPSector(TIXA_SECTOR_X, TIXA_SECTOR_Y, 1)); //j9_b1
		if( pSector )
		{
			pSector->uiFlags |= SF_USE_ALTERNATE_MAP;
		}
	}

	// init hospital variables
	giHospitalTempBalance = 0;
	giHospitalRefund = 0;
	gbHospitalPriceModifier = 0;

	// set up Devin so he will be placed ASAP
	gMercProfiles[ DEVIN ].bNPCData = 3;
}

void InitBloodCatSectors()
{
	INT32 i;
	for( i = 0; i < 255; i++ )
	{
		SectorInfo[ i ].bBloodCats = -1;
	};
	for (auto placements : GCM->getBloodCatPlacements())
	{
		UINT8 sectorId = placements->sectorId;
		SectorInfo[ sectorId ].bBloodCatPlacements = placements->bloodCatPlacements;
	}
	for (auto spawns : GCM->getBloodCatSpawns())
	{
		UINT8 sectorId = spawns->sectorId;
		INT8 spawnsCount = spawns->getSpawnsByDifficulty(gGameOptions.ubDifficultyLevel);
		SectorInfo[ sectorId ].bBloodCatPlacements = spawnsCount;
		SectorInfo[ sectorId ].bBloodCats = spawnsCount;
	}
}


void InitStrategicLayer( void )
{
	// Clear starategic layer!
	SetupNewStrategicGame();
	InitQuestEngine();

	//Setup a new campaign via the enemy perspective.
	InitNewCampaign();
	// Init Squad Lists
	InitSquads();
	// Init vehicles
	InitVehicles( );
	// init town loyalty
	InitTownLoyalty();
	// init the mine management system
	InitializeMines();
	// initialize map screen flags
	InitMapScreenFlags();
	// initialize NPCs, select alternate maps, etc
	InitNPCs();
	// init Skyrider and his helicopter
	InitializeHelicopter();
	//Clear out the vehicle list
	ClearOutVehicleList();

	InitBloodCatSectors();

	InitializeSAMSites();

	// make Orta, Tixa, SAM sites not found
	InitMapSecrets();


	// free up any leave list arrays that were left allocated
	ShutDownLeaveList( );
	// re-set up leave list arrays for dismissed mercs
	InitLeaveList( );

	// reset time compression mode to X0 (this will also pause it)
	SetGameTimeCompressionLevel( TIME_COMPRESS_X0 );

	// Select the start sector as the initial selected sector
	static const SGPSector startSector(gamepolicy(start_sector));
	ChangeSelectedMapSector(startSector);

	// Reset these flags or mapscreen could be disabled and cause major headache.
	fDisableDueToBattleRoster = FALSE;
	fDisableMapInterfaceDueToBattle = FALSE;
}

void ShutdownStrategicLayer()
{
	DeleteAllStrategicEvents();
	RemoveAllGroups();
	TrashUndergroundSectorInfo();
	DeleteCreatureDirectives();
	KillStrategicAI();
}


void InitNewGame()
{
	uiMeanWhileFlags = 0;
	SetSelectedMan(0);

	RESET_CHEAT_LEVEL();

	if (gubScreenCount == 0)
	{
		LoadMercProfiles();
		InitAllArmsDealers();
		InitBobbyRayInventory();
	}

	ClearTacticalMessageQueue();
	FreeGlobalMessageList(); // Clear mapscreen messages

	ResetGameStates();
	InitScriptingEngine();

	if (gubScreenCount == 0)
	{ // Our first time, go into laptop
		InitLaptopAndLaptopScreens();
		InitStrategicLayer();
		SetLaptopNewGameFlag();

		// This is for the "mercs climbing down from a rope" animation, NOT Skyrider
		ResetHeliSeats();

		UINT32 const now = GetWorldTotalMin();
		AddPreReadEmail(OLD_ENRICO_1, OLD_ENRICO_1_LENGTH, MAIL_ENRICO, now);
		AddPreReadEmail(OLD_ENRICO_2, OLD_ENRICO_2_LENGTH, MAIL_ENRICO, now);
		AddPreReadEmail(RIS_REPORT,   RIS_REPORT_LENGTH,   RIS_EMAIL,   now);
		AddPreReadEmail(OLD_ENRICO_3, OLD_ENRICO_3_LENGTH, MAIL_ENRICO, now);
		AddEmail(IMP_EMAIL_INTRO, IMP_EMAIL_INTRO_LENGTH, CHAR_PROFILE_SITE, now);

		// ATE: Set starting cash
		INT32 starting_cash;
		switch (gGameOptions.ubDifficultyLevel)
		{
			case DIF_LEVEL_EASY:   starting_cash = gamepolicy(starting_cash_easy); break;
			case DIF_LEVEL_MEDIUM: starting_cash = gamepolicy(starting_cash_medium); break;
			case DIF_LEVEL_HARD:   starting_cash = gamepolicy(starting_cash_hard); break;
			default: throw std::logic_error("invalid difficulty level");
		}
		AddTransactionToPlayersBook(ANONYMOUS_DEPOSIT, 0, now, starting_cash);

		// random day between min and max days, inclusive
		UINT8 ubMin = gamepolicy(merc_online_min_days);
		UINT8 ubMax = gamepolicy(merc_online_max_days);
		UINT32 const days_time_merc_site_available = Random(ubMax - ubMin + 1) + ubMin;
		if (days_time_merc_site_available == 0)
		{
			// M.E.R.C. is already online
			AddEmail(MERC_INTRO, MERC_INTRO_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());
		}
		else
		{
			// Schedule email for message from Speck at 7am on a random day in the future
			AddFutureDayStrategicEvent(EVENT_DAY3_ADD_EMAIL_FROM_SPECK, 60 * 7, 0, days_time_merc_site_available);
		}

		SetLaptopExitScreen(INIT_SCREEN);
		SetPendingNewScreen(LAPTOP_SCREEN);
		gubScreenCount = 1;

		// Set the fact the game is in progress
		gTacticalStatus.fHasAGameBeenStarted = TRUE;
	}
	else if (gubScreenCount == 1)
	{
		gubScreenCount = 2;
	}
}


BOOLEAN AnyMercsHired( )
{
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		return TRUE;
	}
	return FALSE;
}


//This function is called when the game is REstarted.  Things that need to be reinited are placed in here
void ReStartingGame()
{
	//Pause the game
	gfGamePaused = TRUE;

	//Reset the sectors
	SetWorldSectorInvalid();

	SetMusicMode(MUSIC_NONE);
	SoundStopAll();

	//we are going to restart a game so initialize the variable so we can initialize a new game
	gubScreenCount = 0;

	InitTacticalSave();

	//Loop through all the soldier and delete them all
	FOR_EACH_SOLDIER(i) TacticalRemoveSoldier(*i);

	// Re-init overhead...
	InitOverhead( );

	//Reset the email list
	ShutDownEmailList();

	//Reinit the laptopn screen variables
	InitLaptopAndLaptopScreens();
	LaptopScreenInit();

	//Reload the Merc profiles
	LoadMercProfiles( );

	// Reload quote files
	ReloadAllQuoteFiles();

	//Initialize the ShopKeeper Interface ( arms dealer inventory, etc. )
	ShopKeeperScreenInit();

	//Delete the world info
	TrashWorld();

	//Init the help screen system
	InitHelpScreenSystem();

	EmptyDialogueQueue( );


	//Make sure the game starts in the TEAM panel ( it wasnt being reset )
	gsCurInterfacePanel = TEAM_PANEL;

	//Delete all the strategic events
	DeleteAllStrategicEvents();

	//Delete creature lair, if any
	DeleteCreatureDirectives();

	//This function gets called when ur in a game a click the quit to main menu button, therefore no game is in progress
	gTacticalStatus.fHasAGameBeenStarted = FALSE;

	// Reset timer callbacks
	gpCustomizableTimerCallback = NULL;

	RESET_CHEAT_LEVEL();
}
