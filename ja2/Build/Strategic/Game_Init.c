#ifdef PRECOMPILEDHEADERS
	#include "Strategic All.h"
	#include "Language_Defines.h"
	#include "HelpScreen.h"
#else
	#include "SGP.h"
	#include "JAScreens.h"
	#include "Laptop.h"
	#include "WorldDef.h"
	#include "Soldier_Control.h"
	#include "Overhead.h"
	#include "Fade_Screen.h"
	#include "GameScreen.h"
	#include "EMail.h"
	#include "Game_Clock.h"
	#include "Soldier_Profile.h"
	#include "StrategicMap.h"
	#include "Game_Init.h"
	#include "Animation_Data.h"
	#include "Finances.h"
	#include "Soldier_Create.h"
	#include "Soldier_Init_List.h"
	#include "Music_Control.h"
	#include "Strategic.h"
	#include "History.h"
	#include "Merc_Entering.h"
	#include "Squads.h"
	#include "Campaign_Init.h"
	#include "Strategic_Town_Loyalty.h"
	#include "Strategic_Mines.h"
	#include "Gameloop.h"
	#include "Random.h"
	#include "Map_Screen_Interface.h"
	#include "Tactical_Save.h"
	#include "Campaign_Types.h"
	#include "Message.h"
	#include "Game_Event_Hook.h"
	#include "Strategic_Movement.h"
	#include "Creature_Spreading.h"
	#include "Quests.h"
	#include "Strategic_AI.h"
	#include "Sound_Control.h"
	#include "LaptopSave.h"
	#include "AIMMembers.h"
	#include "Dialogue_Control.h"
	#include "NPC.h"
	#include "OppList.h"
	#include "GameSettings.h"
	#include "Interface_Dialogue.h"
	#include "Map_Screen_Interface_Border.h"
	#include "Map_Screen_Helicopter.h"
	#include "Vehicles.h"
	#include "Map_Screen_Interface_Map.h"
	#include "PreBattle_Interface.h"
	#include "Arms_Dealer_Init.h"
	#include "BobbyR.h"
	#include "HelpScreen.h"
	#include "Air_Raid.h"
	#include "Interface.h"
	#include "Cheats.h"
#endif

// Temp function
void QuickSetupOfMercProfileItems( UINT32 uiCount, UINT8 ubProfileIndex );
BOOLEAN QuickGameMemberHireMerc( UINT8 ubCurrentSoldier );
extern UINT32 guiExitScreen;
extern UINT32 uiMeanWhileFlags;
extern BOOLEAN gfGamePaused;

extern UNDERGROUND_SECTORINFO* FindUnderGroundSector( INT16 sMapX, INT16 sMapY, UINT8 bMapZ );
void InitVehicles(void); /* XXX shouldn't be here */


UINT8			gubScreenCount=0;

void InitNPCs( void )
{
	MERCPROFILESTRUCT * pProfile;

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

	#ifdef JA2TESTVERSION
		ScreenMsg( MSG_FONT_RED, MSG_DEBUG, L"Skyrider in %c %d", 'A' + pProfile->sSectorY - 1, pProfile->sSectorX );
	#endif
	// use alternate map, with Skyrider's shack, in this sector
	SectorInfo[ SECTOR( pProfile->sSectorX, pProfile->sSectorY ) ].uiFlags |= SF_USE_ALTERNATE_MAP;


	// set up Madlab's secret lab (he'll be added when the meanwhile scene occurs)

	switch( Random( 4 ) )
	{
		case 0:
			// use alternate map in this sector
			SectorInfo[ SECTOR( 7, MAP_ROW_H ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
			break;
		case 1:
			SectorInfo[ SECTOR( 16, MAP_ROW_H ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
			break;
		case 2:
			SectorInfo[ SECTOR( 11, MAP_ROW_I ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
			break;
		case 3:
			SectorInfo[ SECTOR( 4, MAP_ROW_E ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
			break;
	}

	// add Micky in random location

	pProfile = &(gMercProfiles[MICKY]);
	switch( Random( 5 ) )
	{
		case 0:
			pProfile->sSectorX = 9;
			pProfile->sSectorY = MAP_ROW_G;
			pProfile->bSectorZ = 0;
			break;
		case 1:
			pProfile->sSectorX = 13;
			pProfile->sSectorY = MAP_ROW_D;
			pProfile->bSectorZ = 0;
			break;
		case 2:
			pProfile->sSectorX = 5;
			pProfile->sSectorY = MAP_ROW_C;
			pProfile->bSectorZ = 0;
			break;
		case 3:
			pProfile->sSectorX = 2;
			pProfile->sSectorY = MAP_ROW_H;
			pProfile->bSectorZ = 0;
			break;
		case 4:
			pProfile->sSectorX = 6;
			pProfile->sSectorY = MAP_ROW_C;
			pProfile->bSectorZ = 0;
			break;
	}

	#ifdef JA2TESTVERSION
		ScreenMsg( MSG_FONT_RED, MSG_DEBUG, L"%s in %c %d", pProfile->zNickname, 'A' + pProfile->sSectorY - 1, pProfile->sSectorX );
	#endif

	// use alternate map in this sector
	//SectorInfo[ SECTOR( pProfile->sSectorX, pProfile->sSectorY ) ].uiFlags |= SF_USE_ALTERNATE_MAP;

  gfPlayerTeamSawJoey = FALSE;


	if ( gGameOptions.fSciFi )
	{
		// add Bob
		pProfile = &(gMercProfiles[BOB]);
		pProfile->sSectorX = 8;
		pProfile->sSectorY = MAP_ROW_F;
		pProfile->bSectorZ = 0;

		// add Gabby in random location
		pProfile = &(gMercProfiles[GABBY]);
		switch( Random( 2 ) )
		{
			case 0:
				pProfile->sSectorX = 11;
				pProfile->sSectorY = MAP_ROW_H;
				pProfile->bSectorZ = 0;
				break;
			case 1:
				pProfile->sSectorX = 4;
				pProfile->sSectorY = MAP_ROW_I;
				pProfile->bSectorZ = 0;
				break;
		}

		#ifdef JA2TESTVERSION
			ScreenMsg( MSG_FONT_RED, MSG_DEBUG, L"%s in %c %d", pProfile->zNickname, 'A' + pProfile->sSectorY - 1, pProfile->sSectorX );
		#endif

		// use alternate map in this sector
		SectorInfo[ SECTOR( pProfile->sSectorX, pProfile->sSectorY ) ].uiFlags |= SF_USE_ALTERNATE_MAP;
	}
	else
	{ //not scifi, so use alternate map in Tixa's b1 level that doesn't have the stairs going down to the caves.
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( 9, 10, 1 ); //j9_b1
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
	//Hard coded table of bloodcat populations.  We don't have
	//access to the real population (if different) until we physically
	//load the map.  If the real population is different, then an error
	//will be reported.
	for( i = 0; i < 255; i++ )
	{
		SectorInfo[ i ].bBloodCats = -1;
	}
	SectorInfo[ SEC_A15	].bBloodCatPlacements = 9;
	SectorInfo[ SEC_B4	].bBloodCatPlacements = 9;
	SectorInfo[ SEC_B16	].bBloodCatPlacements = 8;
	SectorInfo[ SEC_C3	].bBloodCatPlacements = 12;
	SectorInfo[ SEC_C8	].bBloodCatPlacements = 13;
	SectorInfo[ SEC_C11	].bBloodCatPlacements = 7;
	SectorInfo[ SEC_D4	].bBloodCatPlacements = 8;
	SectorInfo[ SEC_D9	].bBloodCatPlacements = 12;
	SectorInfo[ SEC_E11	].bBloodCatPlacements = 10;
	SectorInfo[ SEC_E13	].bBloodCatPlacements = 14;
	SectorInfo[ SEC_F3	].bBloodCatPlacements = 13;
	SectorInfo[ SEC_F5	].bBloodCatPlacements = 7;
	SectorInfo[ SEC_F7	].bBloodCatPlacements = 12;
	SectorInfo[ SEC_F12	].bBloodCatPlacements = 9;
	SectorInfo[ SEC_F14	].bBloodCatPlacements = 14;
	SectorInfo[ SEC_F15	].bBloodCatPlacements = 8;
	SectorInfo[ SEC_G6	].bBloodCatPlacements = 7;
	SectorInfo[ SEC_G10	].bBloodCatPlacements = 12;
	SectorInfo[ SEC_G12	].bBloodCatPlacements = 11;
	SectorInfo[ SEC_H5	].bBloodCatPlacements = 9;
	SectorInfo[ SEC_I4	].bBloodCatPlacements = 8;
	SectorInfo[ SEC_I15	].bBloodCatPlacements = 8;
	SectorInfo[ SEC_J6	].bBloodCatPlacements = 11;
	SectorInfo[ SEC_K3	].bBloodCatPlacements = 12;
	SectorInfo[ SEC_K6	].bBloodCatPlacements = 14;
	SectorInfo[ SEC_K10	].bBloodCatPlacements = 12;
	SectorInfo[ SEC_K14	].bBloodCatPlacements = 14;

	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY: //50%
			SectorInfo[ SEC_I16	].bBloodCatPlacements = 14;
			SectorInfo[ SEC_I16	].bBloodCats = 14;
			SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
			SectorInfo[ SEC_N5	].bBloodCats = 8;
			break;
		case DIF_LEVEL_MEDIUM: //75%
			SectorInfo[ SEC_I16	].bBloodCatPlacements = 19;
			SectorInfo[ SEC_I16	].bBloodCats = 19;
			SectorInfo[ SEC_N5	].bBloodCatPlacements = 10;
			SectorInfo[ SEC_N5	].bBloodCats = 10;
			break;
		case DIF_LEVEL_HARD: //100%
			SectorInfo[ SEC_I16	].bBloodCatPlacements = 26;
			SectorInfo[ SEC_I16	].bBloodCats = 26;
			SectorInfo[ SEC_N5	].bBloodCatPlacements = 12;
			SectorInfo[ SEC_N5	].bBloodCats = 12;
			break;
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

	// select A9 Omerta as the initial selected sector
	ChangeSelectedMapSector( 9, 1, 0 );

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

BOOLEAN InitNewGame( BOOLEAN fReset )
{
	INT32		iStartingCash;

//	static fScreenCount = 0;

	if( fReset )
	{
		gubScreenCount = 0;
		return( TRUE );
	}

	// reset meanwhile flags
	uiMeanWhileFlags = 0;

	// Reset the selected soldier
	gusSelectedSoldier = NOBODY;

	if( gubScreenCount == 0 )
	{
		if( !LoadMercProfiles() )
			return(FALSE);
	}

	//Initialize the Arms Dealers and Bobby Rays inventory
	if( gubScreenCount == 0 )
	{
		//Init all the arms dealers inventory
		InitAllArmsDealers();
		InitBobbyRayInventory();
	}

	// clear tactical
	ClearTacticalMessageQueue( );

	// clear mapscreen messages
	FreeGlobalMessageList();

#ifdef JA2DEMO

	// IF our first time, go into laptop!
	InitStrategicLayer();

	// Hire demo mercs....
#if defined ( JA2TESTVERSION ) || defined ( JA2DEMO )
	DemoHiringOfMercs( );
#endif

	// Setup initial money
 	AddTransactionToPlayersBook( ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), 20500 );
	#ifdef GERMAN
	  //The different mercs are slightly more expensive.  This adds that difference.
		AddTransactionToPlayersBook( ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), 1075 );
	#endif


	if ( !SetCurrentWorldSector( 1, 16, 0 ) )
	{

	}


	SetLaptopExitScreen( MAP_SCREEN );
	FadeInGameScreen( );
	EnterTacticalScreen( );

#else

	// IF our first time, go into laptop!
	if ( gubScreenCount == 0 )
	{
		//Init the laptop here
		InitLaptopAndLaptopScreens();

		InitStrategicLayer();

		// Set new game flag
		SetLaptopNewGameFlag( );

		// this is for the "mercs climbing down from a rope" animation, NOT Skyrider!!
		ResetHeliSeats( );

		// Setup two new messages!
		AddPreReadEmail(OLD_ENRICO_1,OLD_ENRICO_1_LENGTH,MAIL_ENRICO,  GetWorldTotalMin() );
		AddPreReadEmail(OLD_ENRICO_2,OLD_ENRICO_2_LENGTH,MAIL_ENRICO,  GetWorldTotalMin() );
		AddPreReadEmail(RIS_REPORT,RIS_REPORT_LENGTH,RIS_EMAIL,  GetWorldTotalMin() );
		AddPreReadEmail(OLD_ENRICO_3,OLD_ENRICO_3_LENGTH,MAIL_ENRICO,  GetWorldTotalMin() );
		AddEmail(IMP_EMAIL_INTRO,IMP_EMAIL_INTRO_LENGTH,CHAR_PROFILE_SITE,  GetWorldTotalMin() );
		//AddEmail(ENRICO_CONGRATS,ENRICO_CONGRATS_LENGTH,MAIL_ENRICO, GetWorldTotalMin() );

		// ATE: Set starting cash....
		switch( gGameOptions.ubDifficultyLevel )
		{
			case DIF_LEVEL_EASY:

				iStartingCash	= 45000;
				break;

			case DIF_LEVEL_MEDIUM:

				iStartingCash	= 35000;
				break;

			case DIF_LEVEL_HARD:

				iStartingCash	= 30000;
				break;

			default:
				Assert(0);
				return( FALSE );
		}

		// Setup initial money
 		AddTransactionToPlayersBook( ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), iStartingCash );


		{
			UINT32	uiDaysTimeMercSiteAvailable = Random( 2 ) + 1;

			// schedule email for message from spec at 7am 3 days in the future
			AddFutureDayStrategicEvent( EVENT_DAY3_ADD_EMAIL_FROM_SPECK, 60*7, 0, uiDaysTimeMercSiteAvailable );
		}

#ifdef CRIPPLED_VERSION
		{
			UINT32 cnt;

			//loop through the first 20 AIM mercs and set them to be away
			for( cnt = 0; cnt < 20; cnt++)
			{
				gMercProfiles[ cnt ].bMercStatus = MERC_WORKING_ELSEWHERE;
				gMercProfiles[ cnt ].uiDayBecomesAvailable = 14;		// 14 days should be ok considering crippled version only goes to day 7
			}
		}

		//Add an event to check for the end of the crippled version
		AddEveryDayStrategicEvent( EVENT_CRIPPLED_VERSION_END_GAME_CHECK, 0, 0 );
#endif


		SetLaptopExitScreen( INIT_SCREEN );
		SetPendingNewScreen(LAPTOP_SCREEN);
		gubScreenCount = 1;

		//Set the fact the game is in progress
		gTacticalStatus.fHasAGameBeenStarted = TRUE;

		return( TRUE );
	}

	/*
	if( ( guiExitScreen == MAP_SCREEN ) && ( LaptopSaveInfo.gfNewGameLaptop ) )
	{
		SetLaptopExitScreen( GAME_SCREEN );
		return( TRUE );
	}
*/
	if ( gubScreenCount == 1 )
	{
		// OK , FADE HERE
		//BeginFade( INIT_SCREEN, 35, FADE_OUT_REALFADE, 5 );
		//BeginFade( INIT_SCREEN, 35, FADE_OUT_VERSION_FASTER, 25 );
		//BeginFade( INIT_SCREEN, 35, FADE_OUT_VERSION_SIDE, 0 );


		gubScreenCount = 2;
		return( TRUE );
	}

/*
	if ( gubScreenCount == 2 )
	{

		if ( !SetCurrentWorldSector( 9, 1, 0 ) )
		{

		}

		SetLaptopExitScreen( MAP_SCREEN );

		FadeInGameScreen( );

		EnterTacticalScreen( );

		if( gfAtLeastOneMercWasHired == TRUE )
		{
			gubScreenCount = 3;
		}
		else
		{

		}

		return( TRUE );
	}

	*/
#endif

	return( TRUE );
}


BOOLEAN AnyMercsHired( )
{
	INT32 cnt;
	SOLDIERTYPE		*pTeamSoldier;
	INT16				  bLastTeamID;

	// Find first guy availible in team
	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

	bLastTeamID = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

  // look for all mercs on the same team,
  for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pTeamSoldier++)
	{
		if ( pTeamSoldier->bActive )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


void QuickStartGame( )
{
	INT32		cnt;
	UINT16	usVal;
	UINT8 ub1 = 0, ub2 = 0;

	for ( cnt = 0; cnt < 3; cnt++ )
	{
		if ( cnt == 0 )
		{
			usVal = (UINT16)Random( 40 );

			QuickSetupOfMercProfileItems( cnt, (UINT8)usVal );
			QuickGameMemberHireMerc( (UINT8)usVal );
		}
		else if ( cnt == 1 )
		{
			do
			{
				usVal = (UINT16)Random( 40 );
			}
			while( usVal != ub1 );

			QuickSetupOfMercProfileItems( cnt, (UINT8)usVal );
			QuickGameMemberHireMerc( (UINT8)usVal );
		}
		else if ( cnt == 2 )
		{
			do
			{
				usVal = (UINT16)Random( 40 );
			}
			while( usVal != ub1 && usVal != ub2 );

			QuickSetupOfMercProfileItems( cnt, (UINT8)usVal );
			QuickGameMemberHireMerc( (UINT8)usVal );
		}

	}
}


// TEMP FUNCTION!
void QuickSetupOfMercProfileItems( UINT32 uiCount, UINT8 ubProfileIndex )
{
	// Quickly give some guys we hire some items

	if ( uiCount == 0 )
	{
		//CreateGun( GLOCK_17, &(pSoldier->inv[ HANDPOS ] ) );
		//gMercProfiles[ ubProfileIndex ].inv[ HANDPOS ] = HAND_GRENADE;
		//gMercProfiles[ ubProfileIndex ].bInvStatus[ HANDPOS ] = 100;
		//gMercProfiles[ ubProfileIndex ].bInvNumber[ HANDPOS ] = 3;
		gMercProfiles[ ubProfileIndex ].inv[ HANDPOS ] = C7;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ HANDPOS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ HANDPOS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ BIGPOCK1POS ] = CAWS;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ BIGPOCK1POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ BIGPOCK1POS ] = 1;

		gMercProfiles[ ubProfileIndex ].bSkillTrait = MARTIALARTS;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK3POS ] = KEY_2;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK3POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK3POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK5POS ] = LOCKSMITHKIT;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK5POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK5POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ BIGPOCK3POS ] = MEDICKIT;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ BIGPOCK3POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ BIGPOCK3POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ BIGPOCK4POS ] = SHAPED_CHARGE;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ BIGPOCK4POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ BIGPOCK4POS ] = 1;


		// TEMP!
		// make carman's opinion of us high!
		gMercProfiles[ 78 ].bMercOpinion[ ubProfileIndex ] = 25;

	}
	else if ( uiCount == 1 )
	{
		gMercProfiles[ ubProfileIndex ].inv[ HANDPOS ] = CAWS;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ HANDPOS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ HANDPOS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK3POS ] = KEY_1;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK3POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK3POS ] = 1;


	}
	else if ( uiCount == 2 )
	{
		gMercProfiles[ ubProfileIndex ].inv[ HANDPOS ] = GLOCK_17;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ HANDPOS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ HANDPOS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SECONDHANDPOS ] = 5;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SECONDHANDPOS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SECONDHANDPOS ] = 1;


		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK1POS ] = SILENCER;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK1POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK1POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK2POS ] = SNIPERSCOPE;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK2POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK2POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK3POS ] = LASERSCOPE;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK3POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK3POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK5POS ] = BIPOD;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK5POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK5POS ] = 1;

		gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK6POS ] = LOCKSMITHKIT;
		gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK6POS ] = 100;
		gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK6POS ] = 1;

	}
	else
	{
		gMercProfiles[ ubProfileIndex ].inv[ HANDPOS ] = (UINT8)Random(30);
		gMercProfiles[ ubProfileIndex ].bInvNumber[ HANDPOS ] = 1;


	}

	gMercProfiles[ ubProfileIndex ].inv[ HELMETPOS ] = KEVLAR_HELMET;
	gMercProfiles[ ubProfileIndex ].bInvStatus[ HELMETPOS ] = 100;
	gMercProfiles[ ubProfileIndex ].bInvNumber[ HELMETPOS ] = 1;

	gMercProfiles[ ubProfileIndex ].inv[ VESTPOS ] = KEVLAR_VEST;
	gMercProfiles[ ubProfileIndex ].bInvStatus[ VESTPOS ] = 100;
	gMercProfiles[ ubProfileIndex ].bInvNumber[ VESTPOS ] = 1;

	gMercProfiles[ ubProfileIndex ].inv[ BIGPOCK2POS ] = RDX;
	gMercProfiles[ ubProfileIndex ].bInvStatus[ BIGPOCK2POS ] = 10;
	gMercProfiles[ ubProfileIndex ].bInvNumber[ BIGPOCK2POS ] = 1;

	gMercProfiles[ ubProfileIndex ].inv[ SMALLPOCK4POS ] = HAND_GRENADE;
	gMercProfiles[ ubProfileIndex ].bInvStatus[ SMALLPOCK4POS ] = 100;
	gMercProfiles[ ubProfileIndex ].bInvNumber[ SMALLPOCK4POS ] = 4;

	// Give special items to some NPCs
	//gMercProfiles[ 78 ].inv[ SMALLPOCK4POS ] = TERRORIST_INFO;
	//gMercProfiles[ 78 ].bInvStatus[ SMALLPOCK4POS ] = 100;
	//gMercProfiles[ 78 ].bInvNumber[ SMALLPOCK4POS ] = 1;

}


BOOLEAN QuickGameMemberHireMerc( UINT8 ubCurrentSoldier )
{
	MERC_HIRE_STRUCT HireMercStruct;

	memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

	HireMercStruct.ubProfileID = ubCurrentSoldier;

	HireMercStruct.sSectorX = gsMercArriveSectorX;
	HireMercStruct.sSectorY = gsMercArriveSectorY;
	HireMercStruct.fUseLandingZoneForArrival = TRUE;

	HireMercStruct.fCopyProfileItemsOver =	TRUE;
	HireMercStruct.ubInsertionCode				= INSERTION_CODE_CHOPPER;

	HireMercStruct.iTotalContractLength = 7;

	//specify when the merc should arrive
	HireMercStruct.uiTimeTillMercArrives = 0;

	//if we succesfully hired the merc
	if( !HireMerc( &HireMercStruct ) )
	{
		return(FALSE);
	}

	//add an entry in the finacial page for the hiring of the merc
	AddTransactionToPlayersBook(HIRED_MERC, ubCurrentSoldier, GetWorldTotalMin(), -(INT32) gMercProfiles[ubCurrentSoldier].uiWeeklySalary );

	if( gMercProfiles[ ubCurrentSoldier ].bMedicalDeposit )
	{
		//add an entry in the finacial page for the medical deposit
		AddTransactionToPlayersBook(MEDICAL_DEPOSIT, ubCurrentSoldier, GetWorldTotalMin(), -(gMercProfiles[ubCurrentSoldier].sMedicalDepositAmount) );
	}

	//add an entry in the history page for the hiring of the merc
	AddHistoryToPlayersLog( HISTORY_HIRED_MERC_FROM_AIM, ubCurrentSoldier, GetWorldTotalMin(), -1, -1 );

	return(TRUE);
}




//This function is called when the game is REstarted.  Things that need to be reinited are placed in here
void ReStartingGame()
{
	UINT16	cnt;

	//Pause the game
	gfGamePaused = TRUE;

	//Reset the sectors
	gWorldSectorX = gWorldSectorY = 0;
	gbWorldSectorZ = -1;

	SoundStopAll( );

	//we are going to restart a game so initialize the variable so we can initialize a new game
	InitNewGame( TRUE );

	//Deletes all the Temp files in the Maps\Temp directory
	InitTacticalSave( TRUE );

	//Loop through all the soldier and delete them all
	for( cnt=0; cnt< TOTAL_SOLDIERS; cnt++)
	{
		TacticalRemoveSoldier( cnt );
	}

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

	if ( InAirRaid( ) )
	{
		EndAirRaid( );
	}

#ifdef JA2TESTVERSION
	//Reset so we can use the 'cheat key' to start with mercs
	TempHiringOfMercs( 0, TRUE );
#endif

	//Make sure the game starts in the TEAM panel ( it wasnt being reset )
	gsCurInterfacePanel = TEAM_PANEL;

	//Delete all the strategic events
	DeleteAllStrategicEvents();

	//This function gets called when ur in a game a click the quit to main menu button, therefore no game is in progress
	gTacticalStatus.fHasAGameBeenStarted = FALSE;

	// Reset timer callbacks
	gpCustomizableTimerCallback = NULL;

	gubCheatLevel = STARTING_CHEAT_LEVEL;
}
