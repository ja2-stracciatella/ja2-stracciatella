#include "Font.h"
#include "HImage.h"
#include "Interface.h"
#include "LoadSaveSectorInfo.h"
#include "LoadSaveStrategicMapElement.h"
#include "Local.h"
#include "MapScreen.h"
#include "Merc_Contract.h"
#include "Merc_Hiring.h"
#include "StrategicMap.h"
#include "Strategic.h"
#include "Strategic_Mines.h"
#include "Tactical_Turns.h"
#include "Timer.h"
#include "Timer_Control.h"
#include "Types.h"
#include "JAScreens.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Interface_Panels.h"
#include "Isometric_Utils.h"
#include "Ambient_Control.h"
#include "VObject.h"
#include "WorldDef.h"
#include "WorldDat.h"
#include "Text.h"
#include "Soldier_Add.h"
#include "Soldier_Macros.h"
#include "Strategic_Pathing.h"
#include "Soldier_Create.h"
#include "Handle_UI.h"
#include "Faces.h"
#include "RenderWorld.h"
#include "GameScreen.h"
#include "Game_Clock.h"
#include "Soldier_Init_List.h"
#include "Strategic_Turns.h"
#include "Merc_Entering.h"
#include "Map_Information.h"
#include "Assignments.h"
#include "Message.h"
#include "Font_Control.h"
#include "Environment.h"
#include "Game_Events.h"
#include "Quests.h"
#include "FileMan.h"
#include "Animated_ProgressBar.h"
#include "Music_Control.h"
#include "Fade_Screen.h"
#include "Strategic_Movement.h"
#include "Campaign_Types.h"
#include "Sys_Globals.h"
#include "Exit_Grids.h"
#include "Tactical_Save.h"
#include "PathAI.h"
#include "Animation_Control.h"
#include "Squads.h"
#include "WorldMan.h"
#include "Strategic_Town_Loyalty.h"
#include "Queen_Command.h"
#include "Cursor_Control.h"
#include "PreBattle_Interface.h"
#include "Scheduling.h"
#include "GameLoop.h"
#include "Random.h"
#include "AI.h"
#include "OppList.h"
#include "Keys.h"
#include "Tactical_Placement_GUI.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Edgepoints.h"
#include "Player_Command.h"
#include "Event_Pump.h"
#include "Meanwhile.h"
#include "Creature_Spreading.h"
#include "SaveLoadMap.h"
#include "Militia_Control.h"
#include "GameSettings.h"
#include "Dialogue_Control.h"
#include "Town_Militia.h"
#include "SysUtil.h"
#include "Sound_Control.h"
#include "Points.h"
#include "Render_Dirty.h"
#include "Loading_Screen.h"
#include "Enemy_Soldier_Save.h"
#include "Boxing.h"
#include "NPC.h"
#include "Strategic_Event_Handler.h"
#include "MessageBoxScreen.h"
#include "Interface_Dialogue.h"
#include "History.h"
#include "Bullets.h"
#include "Physics.h"
#include "Explosion_Control.h"
#include "Auto_Resolve.h"
#include "Cursors.h"
#include "Video.h"
#include "VSurface.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "Items.h"


//Used by PickGridNoToWalkIn
#define MAX_ATTEMPTS	200

#define		QUEST_CHECK_EVENT_TIME							( 8 * 60 )
#define		BOBBYRAY_UPDATE_TIME							  ( 9 * 60 )
#define		INSURANCE_UPDATE_TIME								0
#define		EARLY_MORNING_TIME									( 4 * 60 )
#define   ENRICO_MAIL_TIME                    ( 7 * 60 )


extern INT16		gsRobotGridNo;

BOOLEAN		gfGettingNameFromSaveLoadScreen;

INT16			gWorldSectorX = 0;
INT16			gWorldSectorY = 0;
INT8			gbWorldSectorZ = -1;

INT16			gsAdjacentSectorX, gsAdjacentSectorY;
INT8			gbAdjacentSectorZ;
GROUP			*gpAdjacentGroup = NULL;
UINT8			gubAdjacentJumpCode;
UINT32		guiAdjacentTraverseTime;
UINT8			gubTacticalDirection;
INT16			gsAdditionalData;
UINT16		gusDestExitGridNo;

BOOLEAN		fUsingEdgePointsForStrategicEntry = FALSE;
BOOLEAN		gfInvalidTraversal = FALSE;
BOOLEAN		gfLoneEPCAttemptingTraversal = FALSE;
BOOLEAN		gfRobotWithoutControllerAttemptingTraversal = FALSE;
BOOLEAN   gubLoneMercAttemptingToAbandonEPCs = 0;
const SOLDIERTYPE* gPotentiallyAbandonedEPC = NULL;

INT8 gbGreenToElitePromotions = 0;
INT8 gbGreenToRegPromotions = 0;
INT8 gbRegToElitePromotions = 0;
INT8 gbMilitiaPromotions = 0;


BOOLEAN		gfUseAlternateMap = FALSE;
// whether or not we have found Orta yet
BOOLEAN fFoundOrta = FALSE;

// have any of the sam sites been found
BOOLEAN fSamSiteFound[ NUMBER_OF_SAMS ]={
	FALSE,
	FALSE,
	FALSE,
	FALSE,
};

INT16 pSamList[ NUMBER_OF_SAMS ]={
	SECTOR( SAM_1_X, SAM_1_Y ),
	SECTOR( SAM_2_X, SAM_2_Y ),
	SECTOR( SAM_3_X, SAM_3_Y ),
	SECTOR( SAM_4_X, SAM_4_Y ),
};

INT16 pSamGridNoAList[ NUMBER_OF_SAMS ]={
	10196,
	11295,
	16080,
	11913,
};

INT16 pSamGridNoBList[ NUMBER_OF_SAMS ]={
	10195,
	11135,
	15920,
	11912,
};

// ATE: Update this w/ graphic used
// Use 3 if / orientation, 4 if \ orientation
INT8 gbSAMGraphicList[ NUMBER_OF_SAMS ]={
	4,
	3,
	3,
  3,
};


// the amount of time that a soldier will wait to return to desired/old squad
#define DESIRE_SQUAD_RESET_DELAY 12 * 60




UINT8 ubSAMControlledSectors[ MAP_WORLD_Y ][ MAP_WORLD_X ] = {
//       1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16
	{ 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0 },

	{ 0,   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 2, 2, 2, 2, 2,   0 }, // A
	{ 0,   1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,   0 }, // B
	{ 0,   1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2,   0 }, // C
	{ 0,   1,01, 1, 1, 1, 1, 1, 3, 3, 2, 2, 2, 2, 2,02, 2,   0 }, // D
	{ 0,   1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2,   0 }, // E
	{ 0,   1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2,   0 }, // F
	{ 0,   1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2,   0 }, // G
	{ 0,   1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,   0 }, // H
	{ 0,   1, 1, 3, 3, 3, 3, 3,03, 3, 3, 3, 3, 3, 2, 2, 2,   0 }, // I
	{ 0,   1, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,   0 }, // J
	{ 0,   4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,   0 }, // K
	{ 0,   4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,   0 }, // L
	{ 0,   4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 2, 2, 2,   0 }, // M
	{ 0,   4, 4, 4,04, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2,   0 }, // N
	{ 0,   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2,   0 }, // O
	{ 0,   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2,   0 }, // P

	{ 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0 }
};

INT16 DirXIncrementer[8] =
 {
  0,        //N
	1,        //NE
	1,        //E
	1,        //SE
	0,         //S
	-1,       //SW
	-1,       //W
	-1       //NW
 };

INT16 DirYIncrementer[8] =
 {
  -1,        //N
	-1,        //NE
	0,        //E
	1,        //SE
	1,         //S
	1,       //SW
	0,       //W
	-1       //NW
 };


const char *pVertStrings[]={
"X",
"A",
"B",
"C",
"D",
"E",
"F",
"G",
"H",
"I",
"J",
"K",
"L",
"M",
"N",
"O",
"P",
"Q",
"R",
};

const char *pHortStrings[]={
"X",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8",
"9",
"10",
"11",
"12",
"13",
"14",
"15",
"16",
"17",
};


extern BOOLEAN gfOverrideSector;


StrategicMapElement StrategicMap[MAP_WORLD_X * MAP_WORLD_Y];


//temp timer stuff -- to measure the time it takes to load a map.
#ifdef JA2TESTVERSION
	extern INT16	 gsAINumAdmins;
	extern INT16	 gsAINumTroops;
	extern INT16	 gsAINumElites;
	extern INT16	 gsAINumCreatures;
	//The wrapper time for EnterSector
	BOOLEAN fStartNewFile = TRUE;
	UINT32 uiEnterSectorStartTime;
	UINT32 uiEnterSectorEndTime;
	//The grand total time for loading a map
	UINT32 uiLoadWorldStartTime;
	UINT32 uiLoadWorldEndTime;
	//The time spent in FileRead
	UINT32 uiTotalFileReadTime;
	UINT32 uiTotalFileReadCalls;
	//LoadWorld and parts
	UINT32 uiLoadWorldTime;
	UINT32 uiTrashWorldTime;
	UINT32 uiLoadMapTilesetTime;
	UINT32 uiLoadMapLightsTime;
	UINT32 uiBuildShadeTableTime;
	UINT32 uiNumImagesReloaded;
	#include "TileDat.h"
#endif


static UINT32 UndergroundTacticalTraversalTime(INT8 bExitDirection)
{ //We are attempting to traverse in an underground environment.  We need to use a complete different
	//method.  When underground, all sectors are instantly adjacent.
	switch( bExitDirection )
	{
		case NORTH_STRATEGIC_MOVE:
			if( gMapInformation.sNorthGridNo != -1 )
				return 0;
			break;
		case EAST_STRATEGIC_MOVE:
			if( gMapInformation.sEastGridNo != -1 )
				return 0;
			break;
		case SOUTH_STRATEGIC_MOVE:
			if( gMapInformation.sSouthGridNo != -1 )
				return 0;
			break;
		case WEST_STRATEGIC_MOVE:
			if( gMapInformation.sWestGridNo != -1 )
				return 0;
			break;
	}
	return 0xffffffff;
}


void BeginLoadScreen( )
{
	UINT32 uiStartTime, uiCurrTime;
	INT32 iPercentage, iFactor;
	UINT32 uiTimeRange;
	INT32 iLastShadePercentage;

#ifdef JA2DEMOADS
	DoDemoIntroduction();
#endif

	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	if( guiCurrentScreen == MAP_SCREEN && !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && !AreInMeanwhile() )
	{
		SGPBox const DstRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		uiTimeRange = 2000;
		iPercentage = 0;
		iLastShadePercentage = 0;
		uiStartTime = GetClock();
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
		PlayJA2SampleFromFile("SOUNDS/Final Psionic Blast 01 (16-44).wav", HIGHVOLUME, 1, MIDDLEPAN);
		while( iPercentage < 100  )
		{
			uiCurrTime = GetClock();
			iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
			iPercentage = MIN( iPercentage, 100 );

			//Factor the percentage so that it is modified by a gravity falling acceleration effect.
			iFactor = (iPercentage - 50) * 2;
			if( iPercentage < 50 )
				iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
			else
				iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

			if( iPercentage > 50 )
			{
				//iFactor = (iPercentage - 50) * 2;
				//if( iFactor > iLastShadePercentage )
			//	{
					//Calculate the difference from last shade % to the new one.  Ex:  Going from
					//50% shade value to 60% shade value requires applying 20% to the 50% to achieve 60%.
					//if( iLastShadePercentage )
					//	iReqShadePercentage = 100 - (iFactor * 100 / iLastShadePercentage);
					//else
					//	iReqShadePercentage = iFactor;
					//Record the new final shade percentage.
					//iLastShadePercentage = iFactor;
					guiSAVEBUFFER->ShadowRectUsingLowPercentTable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			//	}
			}

			SGPBox const SrcRect =
			{
				536 * iPercentage / 100,
				367 * iPercentage / 100,
				SCREEN_WIDTH  - 541 * iPercentage / 100,
				SCREEN_HEIGHT - 406 * iPercentage / 100
			};
			BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &SrcRect, &DstRect);
			InvalidateScreen();
			RefreshScreen();
		}
	}
	FRAME_BUFFER->Fill(Get16BPPColor(FROMRGB(0, 0, 0)));
	InvalidateScreen( );
	RefreshScreen();

	//If we are loading a saved game, use the Loading screen we saved into the SavedGameHeader file
	// ( which gets reloaded into gubLastLoadingScreenID )
	if( !gfGotoSectorTransition )
	{
		LoadingScreenID const id = gTacticalStatus.uiFlags & LOADING_SAVED_GAME ?
			gubLastLoadingScreenID :
			GetLoadScreenID(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
		DisplayLoadScreenWithID(id);
	}

#ifdef JA2TESTVERSION
	uiEnterSectorStartTime = 0;
	uiEnterSectorEndTime = 0;
	//The grand total time for loading a map
	uiLoadWorldStartTime = 0;
	uiLoadWorldEndTime = 0;
	//The time spent in FileRead
	uiTotalFileReadTime = 0;
	uiTotalFileReadCalls = 0;
	//Sections of LoadWorld
	uiLoadWorldTime = 0;
	uiTrashWorldTime = 0;
	uiLoadMapTilesetTime = 0;
	uiLoadMapLightsTime = 0;
	uiBuildShadeTableTime = 0;
	uiEnterSectorStartTime = GetJA2Clock();
#endif

}


static void EndLoadScreen(void)
{
#ifdef JA2TESTVERSION
	//Report the time it took to load the map.  This is temporary until we are satisfied with the time
	//it takes to load the map.
	wchar_t str[60];
	FILE *fp;
	UINT32 uiSeconds;
	UINT32 uiHundreths;
	UINT32 uiUnaccounted;
	UINT32 uiPercentage;
	uiEnterSectorEndTime = GetJA2Clock();
	uiSeconds = (uiEnterSectorEndTime - uiEnterSectorStartTime) / 1000;
	uiHundreths = ((uiEnterSectorEndTime - uiEnterSectorStartTime) / 10) % 100;
	if( !gbWorldSectorZ )
	{
		swprintf(str, lengthof(str), L"%c%d ENTER SECTOR TIME:  %d.%02d seconds.",
							'A' + gWorldSectorY - 1, gWorldSectorX, uiSeconds, uiHundreths );
	}
	else
	{
		swprintf(str, lengthof(str), L"%c%d_b%d ENTER SECTOR TIME:  %d.%02d seconds.",
							'A' + gWorldSectorY - 1, gWorldSectorX, gbWorldSectorZ, uiSeconds, uiHundreths );
	}
	ScreenMsg( FONT_YELLOW, MSG_TESTVERSION, str );
	if( fStartNewFile )
	{ //start new file
		fp = fopen( "TimeResults.txt", "w" );
		ScreenMsg( FONT_YELLOW, MSG_TESTVERSION, L"See JA2/Data/TimeResults.txt for more detailed timings." );
		fStartNewFile = FALSE;
	}
	else
	{ //append to end of file
		fp = fopen( "TimeResults.txt", "a" );

		if ( fp )
		{
			fprintf( fp, "\n\n--------------------------------------------------------------------\n\n" );
		}
	}
	if( fp )
	{
		//Record all of the timings.
		fprintf( fp, "%ls\n", str );
		fprintf( fp, "EnterSector() supersets LoadWorld().  This includes other external sections.\n");
		//FileRead()
		fprintf( fp, "\n\nVARIOUS FUNCTION TIMINGS (exclusive of actual function timings in second heading)\n" );
		uiSeconds = uiTotalFileReadTime / 1000;
		uiHundreths = (uiTotalFileReadTime / 10) % 100;
		fprintf( fp, "FileRead:  %d.%02d (called %d times)\n", uiSeconds, uiHundreths, uiTotalFileReadCalls );

		fprintf( fp, "\n\nSECTIONS OF LOADWORLD (all parts should add up to 100%)\n" );
		//TrashWorld()
		uiSeconds = uiTrashWorldTime / 1000;
		uiHundreths = (uiTrashWorldTime / 10) % 100;
		fprintf( fp, "TrashWorld: %d.%02d\n", uiSeconds, uiHundreths );
		//LoadMapTilesets()
		uiSeconds = uiLoadMapTilesetTime / 1000;
		uiHundreths = (uiLoadMapTilesetTime / 10) % 100;
		fprintf( fp, "LoadMapTileset: %d.%02d\n", uiSeconds, uiHundreths );
		//LoadMapLights()
		uiSeconds = uiLoadMapLightsTime / 1000;
		uiHundreths = (uiLoadMapLightsTime / 10) % 100;
		fprintf( fp, "LoadMapLights: %d.%02d\n", uiSeconds, uiHundreths );
		uiSeconds = uiBuildShadeTableTime / 1000;
		uiHundreths = (uiBuildShadeTableTime / 10) % 100;
		fprintf( fp, "  1)  BuildShadeTables: %d.%02d\n", uiSeconds, uiHundreths );

		uiPercentage = uiNumImagesReloaded * 100 / NUMBEROFTILETYPES;
		fprintf( fp, "  2)  %d%% of the tileset images were actually reloaded.\n", uiPercentage );

		//Unaccounted
		uiUnaccounted = uiLoadWorldTime - uiTrashWorldTime - uiLoadMapTilesetTime - uiLoadMapLightsTime;
		uiSeconds = uiUnaccounted / 1000;
		uiHundreths = (uiUnaccounted / 10) % 100;
		fprintf( fp, "Unaccounted: %d.%02d\n", uiSeconds, uiHundreths );
		//LoadWorld()
		uiSeconds = uiLoadWorldTime / 1000;
		uiHundreths = (uiLoadWorldTime / 10) % 100;
		fprintf( fp, "\nTotal: %d.%02d\n", uiSeconds, uiHundreths );

		fclose( fp );
	}
#endif
}


static void InitializeMapStructure(void);


void InitStrategicEngine()
{
	// this runs every time we start the application, so don't put anything in here that's only supposed to run when a new
	// *game* is started!  Those belong in InitStrategicLayer() instead.

	InitializeMapStructure();

	// set up town stuff
	BuildListOfTownSectors( );
}


UINT8 GetTownIdForSector( INT16 sMapX, INT16 sMapY )
{
	// return the name value of the town in this sector

	return( StrategicMap[CALCULATE_STRATEGIC_INDEX( sMapX, sMapY )].bNameId );
}


// return number of sectors this town takes up
UINT8 GetTownSectorSize( INT8 bTownId )
{
	UINT8 ubSectorSize = 0;
	INT32 iCounterA =0, iCounterB = 0;

	for( iCounterA = 0; iCounterA < ( INT32 )( MAP_WORLD_X - 1 ); iCounterA++ )
	{
		for( iCounterB = 0; iCounterB < ( INT32 )( MAP_WORLD_Y - 1 ); iCounterB++ )
		{
			if(  StrategicMap[CALCULATE_STRATEGIC_INDEX( iCounterA, iCounterB )].bNameId == bTownId )
			{
				ubSectorSize++;
			}
		}
	}

	return( ubSectorSize );
}


static UINT8 GetMilitiaCountAtLevelAnywhereInTown(UINT8 ubTownValue, UINT8 ubLevelValue)
{
	INT32 iCounter = 0;
	UINT8 ubCount =0;

	while( pTownNamesList[ iCounter ] != 0 )
	{
		if( StrategicMap[ pTownLocationsList[ iCounter ] ].bNameId == ubTownValue )
		{
			// match.  Add the number of civs at this level
			ubCount += SectorInfo[ STRATEGIC_INDEX_TO_SECTOR_INFO( pTownLocationsList[ iCounter ] ) ].ubNumberOfCivsAtLevel[ ubLevelValue ];
		}

		iCounter++;
	}
	return( ubCount );
}


// return number of sectors under player control for this town
UINT8 GetTownSectorsUnderControl( INT8 bTownId )
{
	INT8 ubSectorsControlled = 0;
	INT32 iCounterA =0, iCounterB = 0;
	UINT16 usSector = 0;

	for( iCounterA = 0; iCounterA < ( INT32 )( MAP_WORLD_X - 1 ); iCounterA++ )
	{
		for( iCounterB = 0; iCounterB < ( INT32 )( MAP_WORLD_Y - 1 ); iCounterB++ )
		{
			usSector = (UINT16)CALCULATE_STRATEGIC_INDEX( iCounterA, iCounterB );

			if( ( StrategicMap[ usSector ].bNameId == bTownId ) &&
					!StrategicMap[usSector].fEnemyControlled &&
					( NumEnemiesInSector( ( INT16 )iCounterA, ( INT16 )iCounterB ) == 0 ) )
			{
				ubSectorsControlled++;
			}
		}
	}

	return( ubSectorsControlled );
}


static void InitializeStrategicMapSectorTownNames(void);


static void InitializeMapStructure(void)
{
	memset(StrategicMap, 0, sizeof(StrategicMap));

	InitializeStrategicMapSectorTownNames( );
}


void InitializeSAMSites( void )
{
	// move the landing zone over to Omerta
	gsMercArriveSectorX = 9;
	gsMercArriveSectorY = 1;

	// all SAM sites start game in perfect working condition
	StrategicMap[(SAM_1_X)+(MAP_WORLD_X*(SAM_1_Y))].bSAMCondition = 100;
	StrategicMap[(SAM_2_X)+(MAP_WORLD_X*(SAM_2_Y))].bSAMCondition = 100;
	StrategicMap[(SAM_3_X)+(MAP_WORLD_X*(SAM_3_Y))].bSAMCondition = 100;
	StrategicMap[(SAM_4_X)+(MAP_WORLD_X*(SAM_4_Y))].bSAMCondition = 100;

	UpdateAirspaceControl( );
}


// get short sector name without town name
void GetShortSectorString(const INT16 sMapX, const INT16 sMapY, wchar_t* const sString, const size_t Length)
{
	// OK, build string id like J11
	swprintf(sString, Length, L"%hs%hs", pVertStrings[sMapY], pHortStrings[sMapX]);
}


void GetMapFileName(INT16 sMapX, INT16 sMapY, INT8 bSectorZ, char* bString, BOOLEAN fUsePlaceholder, BOOLEAN fAddAlternateMapLetter)
{
	CHAR8	 bTestString[ 150 ];
	CHAR8	 bExtensionString[ 15 ];

	if ( bSectorZ != 0 )
	{
		sprintf( bExtensionString, "_b%d", bSectorZ );
	}
	else
	{
		strcpy( bExtensionString, "" );
	}

	// the gfUseAlternateMap flag is set in the loading saved games.  When starting a new game the underground sector
	//info has not been initialized, so we need the flag to load an alternate sector.
	if( gfUseAlternateMap | GetSectorFlagStatus( sMapX, sMapY, bSectorZ, SF_USE_ALTERNATE_MAP ) )
	{
		gfUseAlternateMap = FALSE;

		//if we ARE to use the a map, or if we are saving AND the save game version is before 80, add the a
		if( fAddAlternateMapLetter )
		{
			strcat( bExtensionString, "_a" );
		}
	}

	// If we are in a meanwhile...
	if ( AreInMeanwhile( ) && sMapX == 3 && sMapY == 16 && !bSectorZ )//GetMeanwhileID() != INTERROGATION )
	{
		if( fAddAlternateMapLetter )
		{
			strcat( bExtensionString, "_m" );
		}
	}

	// This is the string to return, but...
	sprintf( bString, "%s%s%s.DAT", pVertStrings[sMapY], pHortStrings[sMapX], bExtensionString );

	// We will test against this string
	sprintf( bTestString, "MAPS/%s", bString );

	if( fUsePlaceholder && !FileExists( bTestString ) )
	{
		// Debug str
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Map does not exist for %s, using default.", bTestString ) );
		// Set to a string we know!
		sprintf(bString, "H10.DAT",pVertStrings[sMapY],pHortStrings[sMapX]);
		ScreenMsg( FONT_YELLOW, MSG_DEBUG, L"Using PLACEHOLDER map!");
	}
}

void GetCurrentWorldSector( INT16 *psMapX, INT16 *psMapY )
{
	*psMapX = gWorldSectorX;
	*psMapY = gWorldSectorY;
}


static void HandleRPCDescriptionOfSector(INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ)
{
	UINT32	cnt;
	UINT8		ubSectorDescription[33][3] =
	{
		 // row (letter), column, quote #
		 { 2, 13,  0 },	// b13	Drassen
		 { 3, 13,  1 },	// c13	Drassen
		 { 4, 13,  2 },	// d13	Drassen
		 { 8, 13,  3 },	// h13	Alma
		 { 8, 14,  4 },	// h14	Alma
		 { 9, 13,  5 },	// i13	Alma * (extra quote 6 if Sci-fi ) *
		 { 9, 14,  7 },	// i14	Alma
		 { 6,  8,  8 },	// f8	Cambria
		 { 6,  9,  9 }, 	// f9	Cambria
		 { 7,  8, 10 },	// g8	Cambria

		 { 7,  9, 11 },	// g9	Cambria
		 { 3,  6, 12 },	// c6	San Mona
		 { 3,  5, 13 },	// c5	San Mona
		 { 4,  5, 14 },	// d5	San Mona
		 { 2,  2, 15 },	// b2	Chitzena
		 { 1,  2, 16 },	// a2	Chitzena
		 { 7,  1, 17 },	// g1	Grumm
		 { 8,  1, 18 },	// h1	Grumm
		 { 7,  2, 19 },	// g2 	Grumm
		 { 8,  2, 20 },	// h2	Grumm

		 { 9,  6, 21 },	// i6	Estoni
		 {11,	 4, 22 },	// k4	Orta
		 {12,	 11, 23 },	// l11	Balime
		 {12,	 12, 24 },	// l12	Balime
		 {15,  3, 25 },	// o3	Meduna
		 {16,  3, 26 },	// p3	Meduna
		 {14,  4, 27 },	// n4	Meduna
		 {14,  3, 28 },	// n3	Meduna
		 {15,  4, 30 },	// o4	Meduna
		 {10,  9, 31 },	// j9	Tixa

		 {4,	15, 32 },	// d15	NE SAM
		 {4,  2, 33 },	// d2	NW SAM
		 {9,  8, 34 }	// i8	CENTRAL SAM
	};

  // Default to false
	gTacticalStatus.fCountingDownForGuideDescription = FALSE;


	// OK, if the first time in...
	if (!GetSectorFlagStatus(sSectorX, sSectorY, sSectorZ, SF_HAVE_USED_GUIDE_QUOTE))
	{
		if ( sSectorZ != 0 )
		{
			return;
		}

		// OK, check if we are in a good sector....
		for ( cnt = 0; cnt < 33; cnt++ )
		{
			if ( sSectorX == ubSectorDescription[ cnt ][ 1 ] && sSectorY == ubSectorDescription[ cnt ][ 0 ] )
			{
				// If we're not scifi, skip some
				if ( !gGameOptions.fSciFi && cnt == 3 )
				{
					continue;
				}

				SetSectorFlag( sSectorX, sSectorY, ( UINT8 )sSectorZ, SF_HAVE_USED_GUIDE_QUOTE );

				gTacticalStatus.fCountingDownForGuideDescription = TRUE;
				gTacticalStatus.bGuideDescriptionCountDown			 = (INT8)( 4 + Random( 5 ) ); // 4 to 8 tactical turns...
				gTacticalStatus.ubGuideDescriptionToUse					 = ubSectorDescription[ cnt ][ 2 ];
				gTacticalStatus.bGuideDescriptionSectorX				 =  (INT8)sSectorX;
				gTacticalStatus.bGuideDescriptionSectorY				 =  (INT8)sSectorY;
			}
		}
	}

  // Handle guide description ( will be needed if a SAM one )
	HandleRPCDescription( );

}


static BOOLEAN EnterSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
enum
{
	ABOUT_TO_LOAD_NEW_MAP,
	ABOUT_TO_TRASH_WORLD,
};
static void HandleDefiniteUnloadingOfWorld(UINT8 ubUnloadCode);


bool SetCurrentWorldSector(INT16 const x, INT16 const y, INT8 const z)
{
	SyncStrategicTurnTimes();

#ifdef JA2BETAVERSION
	if (gfOverrideSector)
	{
		/* Skip the cancel, and force load the sector.  This is used by the AIViewer
		 * to "reset" a level with different numbers of various types of enemies. */
	}
	else
#endif
	// is the sector already loaded?
	if (gWorldSectorX == x && y == gWorldSectorY && z == gbWorldSectorZ)
	{
		/* Insert the enemies into the newly loaded map based on the strategic
		 * information. Note, the flag will return TRUE only if enemies were added.
		 * The game may wish to do something else in a case where no enemies are
		 * present. */

		SetPendingNewScreen(GAME_SCREEN);
		if (NumEnemyInSector() == 0)
		{
			PrepareEnemyForSectorBattle();
		}
		if (gubNumCreaturesAttackingTown != 0 &&
				z                            == 0 &&
				gubSectorIDOfCreatureAttack  == SECTOR(x, y))
		{
			PrepareCreaturesForBattle();
		}

		if (gfGotoSectorTransition)
		{
			BeginLoadScreen();
			gfGotoSectorTransition = FALSE;
		}

		HandleHelicopterOnGroundGraphic();

		ResetMilitia();
		AllTeamsLookForAll(TRUE);
		return true;
	}

	if (gWorldSectorX != 0 && gWorldSectorY != 0 && gbWorldSectorZ != -1)
	{
		HandleDefiniteUnloadingOfWorld(ABOUT_TO_LOAD_NEW_MAP);
	}

	// make this the currently loaded sector
	gWorldSectorX  = x;
	gWorldSectorY  = y;
	gbWorldSectorZ = z;

	// update currently selected map sector to match
	ChangeSelectedMapSector(x, y, z);

	bool const loading_savegame = gTacticalStatus.uiFlags & LOADING_SAVED_GAME;
	if (loading_savegame)
	{
		SetMusicMode(MUSIC_MAIN_MENU);
	}
	else
	{
    StopAnyCurrentlyTalkingSpeech();

		/* Check to see if the sector we are loading is the cave sector under Tixa.
		 * If so then we will set up the meanwhile scene to start the creature
		 * quest. */
		if (x == 9 && y == 10 && z == 2)
		{
			InitCreatureQuest(); // Ignored if already active.
		}

		gTacticalStatus.uiTimeSinceLastInTactical = GetWorldTotalMin();
		InitializeTacticalStatusAtBattleStart();
	  HandleHelicopterOnGroundSkyriderProfile();
	}

	if (!EnterSector(x, y, z)) return false;

	if (!loading_savegame)
	{
		InitAI();
		ExamineDoorsOnEnteringSector();
	}

	/* Update all the doors in the sector according to the temp file previously
	 * loaded, and any changes made by the schedules */
	UpdateDoorGraphicsFromStatus();

	// Set the fact we have visited the  sector
	SetSectorFlag(x, y, z, SF_ALREADY_LOADED);

	// Check for helicopter being on the ground in this sector
	HandleHelicopterOnGroundGraphic();

	if (!loading_savegame)
	{
		if (gubMusicMode == MUSIC_TACTICAL_ENEMYPRESENT ?
					NumHostilesInSector(x, y, z) == 0 :
					gubMusicMode != MUSIC_TACTICAL_BATTLE)
		{
			// ATE: Fade FAST
			SetMusicFadeSpeed(5);
			SetMusicMode(MUSIC_TACTICAL_NOTHING);
		}

		// ATE: Check what sector we are in, to show description if we have an RPC
		HandleRPCDescriptionOfSector(x, y, z);

		// ATE: Set Flag for being visited
		SetSectorFlag(x, y, z, SF_HAS_ENTERED_TACTICAL);

		ResetMultiSelection();

    gTacticalStatus.fHasEnteredCombatModeSinceEntering = FALSE;
    gTacticalStatus.fDontAddNewCrows                   = FALSE;

    // Adjust delay for tense quote
    gTacticalStatus.sCreatureTenseQuoteDelay = 10 + Random(20);

		INT16 sWarpWorldX;
		INT16 sWarpWorldY;
		INT8  bWarpWorldZ;
		INT16 sWarpGridNo;
		if (z >= 2 && GetWarpOutOfMineCodes(&sWarpWorldX, &sWarpWorldY, &bWarpWorldZ, &sWarpGridNo))
		{
			gTacticalStatus.uiFlags |= IN_CREATURE_LAIR;
		}
		else
		{
			gTacticalStatus.uiFlags &= ~IN_CREATURE_LAIR;
		}

		gTacticalStatus.fGoodToAllowCrows  = TRUE;
		gTacticalStatus.ubNumCrowsPossible = 5 + Random(5);
	}

	return true;
}


void RemoveMercsInSector( )
{
	// ATE: only for OUR guys.. the rest is taken care of in TrashWorld() when a new sector is added...
	FOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		RemoveSoldierFromGridNo(s);
	}
}


void PrepareLoadedSector()
{
	BOOLEAN fEnemyPresenceInThisSector = FALSE;
	BOOLEAN fAddCivs = TRUE;
	INT8 bMineIndex = -1;

	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		UpdateMercsInSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
	}

	// Reset ambients!
	HandleNewSectorAmbience( gTilesets[ giCurrentTilesetID ].ubAmbientID );

	//if we are loading a 'pristine' map ( ie, not loading a saved game )
	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ))
	{
		if ( !AreReloadingFromMeanwhile( ) )
		{
			SetPendingNewScreen(GAME_SCREEN);

      // Make interface the team panel always...
			SetCurrentInterfacePanel(TEAM_PANEL);
		}


		//Check to see if civilians should be added.  Always add civs to maps unless they are
		//in a mine that is shutdown.
		if( gbWorldSectorZ )
		{
			bMineIndex = GetIdOfMineForSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
			if( bMineIndex != -1 )
			{
				if( !AreThereMinersInsideThisMine( (UINT8)bMineIndex ) )
				{
					fAddCivs = FALSE;
				}
			}
		}
		if( fAddCivs )
		{
			AddSoldierInitListTeamToWorld( CIV_TEAM, 255 );
		}

		AddSoldierInitListTeamToWorld( MILITIA_TEAM, 255 );
		AddSoldierInitListBloodcats();
		//Creatures are only added if there are actually some of them.  It has to go through some
		//additional checking.

		#ifdef JA2TESTVERSION
		//Override the sector with the populations specified in the AIViewer
		if( gfOverrideSector )
		{
			if( gbWorldSectorZ > 0 )
			{
				UNDERGROUND_SECTORINFO *pSector;
				pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
				pSector->ubNumAdmins = (UINT8)( gsAINumAdmins > 0 ? gsAINumAdmins : 0 );
				pSector->ubNumTroops = (UINT8)( gsAINumTroops > 0 ? gsAINumTroops : 0 );
				pSector->ubNumElites = (UINT8)( gsAINumElites > 0 ? gsAINumElites : 0 );
				pSector->ubNumCreatures = (UINT8)( gsAINumCreatures > 0 ? gsAINumCreatures : 0 );
			}
			else if( !gbWorldSectorZ )
			{
				SECTORINFO *pSector;
				pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
				pSector->ubNumAdmins = (UINT8)( gsAINumAdmins > 0 ? gsAINumAdmins : 0 );
				pSector->ubNumTroops = (UINT8)( gsAINumTroops > 0 ? gsAINumTroops : 0 );
				pSector->ubNumElites = (UINT8)( gsAINumElites > 0 ? gsAINumElites : 0 );
				pSector->ubNumCreatures = (UINT8)( gsAINumCreatures > 0 ? gsAINumCreatures : 0 );
			}
		}
		#endif

		PrepareCreaturesForBattle();

		PrepareMilitiaForTactical();

		// OK, set varibles for entring this new sector...
		gTacticalStatus.fVirginSector = TRUE;

		//Inserts the enemies into the newly loaded map based on the strategic information.
		//Note, the flag will return TRUE only if enemies were added.  The game may wish to
		//do something else in a case where no enemies are present.
		if( !gfRestoringEnemySoldiersFromTempFile )
		{
			//AddSoldierInitListTeamToWorld( CIV_TEAM, 255 );
//			fEnemyPresenceInThisSector = PrepareEnemyForSectorBattle();
		}
		AddProfilesNotUsingProfileInsertionData();

		if( !AreInMeanwhile() || GetMeanwhileID() == INTERROGATION )
		{
			fEnemyPresenceInThisSector = PrepareEnemyForSectorBattle();
		}


		//Regardless whether or not this was set, clear it now.
		gfRestoringEnemySoldiersFromTempFile = FALSE;

		//KM:  FEB 8, 99 -- This call is no longer required!  Done already when group arrives in sector.
		//if (gbWorldSectorZ == 0 && !fEnemyPresenceInThisSector)
		//{
		//	SetThisSectorAsPlayerControlled( gWorldSectorX, gWorldSectorY, 0 );
		//}

		//@@@Evaluate
		//Add profiles to world using strategic info, not editor placements.
		AddProfilesUsingProfileInsertionData();

		PostSchedules();
	}

	if( gubEnemyEncounterCode == ENEMY_AMBUSH_CODE || gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE )
	{
		if( gMapInformation.sCenterGridNo != -1 )
		{
			CallAvailableEnemiesTo( gMapInformation.sCenterGridNo );
		}
		else
		{
			#ifdef JA2BETAVERSION
				ScreenMsg( FONT_RED, MSG_ERROR, L"Ambush aborted in sector %c%d -- no center point in map.  LC:1",
					gWorldSectorY + 'A' - 1, gWorldSectorX );
			#endif
		}
	}

	EndLoadScreen( );

	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// unpause game
		UnPauseGame( );
	}

	gpBattleGroup = NULL;

	if( gfTacticalTraversal )
	{
		CalculateNonPersistantPBIInfo();
	}

	ScreenMsg( FONT_YELLOW, MSG_DEBUG, L"Current Time is: %d", GetWorldTotalMin() );

	AllTeamsLookForAll( TRUE );
}

#define RANDOM_HEAD_MINERS 4
void HandleQuestCodeOnSectorEntry( INT16 sNewSectorX, INT16 sNewSectorY, INT8 bNewSectorZ )
{
	UINT8		ubRandomMiner[RANDOM_HEAD_MINERS] = { 106, 156, 157, 158 };
	UINT8		ubMiner, ubMinersPlaced;
	UINT8		ubMine, ubThisMine;

	if ( CheckFact( FACT_ALL_TERRORISTS_KILLED, 0 ) )
	{
		// end terrorist quest
		EndQuest( QUEST_KILL_TERRORISTS, gMercProfiles[ CARMEN ].sSectorX, gMercProfiles[ CARMEN ].sSectorY );
		// remove Carmen
		gMercProfiles[ CARMEN ].sSectorX = 0;
		gMercProfiles[ CARMEN ].sSectorY = 0;
		gMercProfiles[ CARMEN ].bSectorZ = 0;
	}

	// are we in a mine sector, on the surface?
	if ( IsThereAMineInThisSector( sNewSectorX, sNewSectorY ) && ( bNewSectorZ == 0 ))
	{
		if (!CheckFact(FACT_MINERS_PLACED, 0))
		{
			// SET HEAD MINER LOCATIONS

			ubThisMine = GetMineIndexForSector( sNewSectorX, sNewSectorY );

			if (ubThisMine != MINE_SAN_MONA) // San Mona is abandoned
			{
				ubMinersPlaced = 0;

				if (ubThisMine != MINE_ALMA)
				{
					// Fred Morris is always in the first mine sector we enter, unless that's Alma (then he's randomized, too)
					MERCPROFILESTRUCT& fred = GetProfile(FRED);
					fred.sSectorX = sNewSectorX;
					fred.sSectorY = sNewSectorY;
					fred.bSectorZ = 0;
					fred.bTown    = gMineLocation[ubThisMine].bAssociatedTown;

					// mark miner as placed
					ubRandomMiner[ 0 ] = 0;
					ubMinersPlaced++;
				}

				// assign the remaining (3) miners randomly
				for ( ubMine = 0; ubMine < MAX_NUMBER_OF_MINES; ubMine++ )
				{
					if ( ubMine == ubThisMine || ubMine == MINE_ALMA || ubMine == MINE_SAN_MONA )
					{
						// Alma always has Matt as a miner, and we have assigned Fred to the current mine
						// and San Mona is abandoned
						continue;
					}

					do
					{
						ubMiner = (UINT8) Random( RANDOM_HEAD_MINERS );
					}
					while( ubRandomMiner[ ubMiner ] == 0 );

					MERCPROFILESTRUCT& p = GetProfile(ubRandomMiner[ubMiner]);
					GetMineSector(ubMine, &p.sSectorX, &p.sSectorY);
					p.bSectorZ = 0;
					p.bTown = gMineLocation[ ubMine ].bAssociatedTown;

					// mark miner as placed
					ubRandomMiner[ ubMiner ] = 0;
					ubMinersPlaced++;

					if (ubMinersPlaced == RANDOM_HEAD_MINERS)
					{
						break;
					}
				}

				SetFactTrue( FACT_MINERS_PLACED );
			}
		}
	}

	if (!CheckFact(FACT_ROBOT_RECRUITED_AND_MOVED, 0))
	{
		const SOLDIERTYPE* const pRobot = FindSoldierByProfileIDOnPlayerTeam(ROBOT);
		if (pRobot)
		{
			// robot is on our team and we have changed sectors, so we can
			// replace the robot-under-construction in Madlab's sector
			RemoveGraphicFromTempFile( gsRobotGridNo, SEVENTHISTRUCT1, gMercProfiles[MADLAB].sSectorX, gMercProfiles[MADLAB].sSectorY, gMercProfiles[MADLAB].bSectorZ );
			SetFactTrue( FACT_ROBOT_RECRUITED_AND_MOVED );
		}
	}

	// Check to see if any player merc has the Chalice; if so,
	// note it as stolen
	CFOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		if (FindObj(s, CHALICE) != ITEM_NOT_FOUND)
		{
			SetFactTrue(FACT_CHALICE_STOLEN);
		}
	}

#ifdef JA2DEMO
	// special stuff to make NPCs talk as if the next day, after going down
	// into mines
	if (bNewSectorZ > 0)
	{
		if ( gMercProfiles[ GABBY ].ubLastDateSpokenTo != 0 )
		{
			gMercProfiles[ GABBY ].ubLastDateSpokenTo = 199;
		}
		if ( gMercProfiles[ JAKE ].ubLastDateSpokenTo != 0 )
		{
			gMercProfiles[ JAKE ].ubLastDateSpokenTo = 199;
		}
	}
#endif

	if ( (gubQuest[ QUEST_KINGPIN_MONEY ] == QUESTINPROGRESS) && CheckFact( FACT_KINGPIN_CAN_SEND_ASSASSINS, 0 ) && (GetTownIdForSector( sNewSectorX, sNewSectorY ) != BLANK_SECTOR) && Random( 10 + GetNumberOfMilitiaInSector( sNewSectorX, sNewSectorY, bNewSectorZ ) ) < 3 )
	{
		DecideOnAssassin();
	}

/*
	if ( sNewSectorX == 5 && sNewSectorY == MAP_ROW_C )
	{
		// reset Madame Layla counters
		gMercProfiles[ MADAME ].bNPCData = 0;
		gMercProfiles[ MADAME ].bNPCData2 = 0;
	}
	*/

	if ( sNewSectorX == 6 && sNewSectorY == MAP_ROW_C && gubQuest[ QUEST_RESCUE_MARIA ] == QUESTDONE )
	{
		// make sure Maria and Angel are gone
		gMercProfiles[ MARIA ].sSectorX = 0;
		gMercProfiles[ ANGEL ].sSectorY = 0;
		gMercProfiles[ MARIA ].sSectorX = 0;
		gMercProfiles[ ANGEL ].sSectorY = 0;
	}

	if ( sNewSectorX == 5 && sNewSectorY == MAP_ROW_D )
	{
		gBoxer[0] = NULL;
		gBoxer[1] = NULL;
		gBoxer[2] = NULL;
	}

	if ( sNewSectorX == 3 && sNewSectorY == MAP_ROW_P )
	{
		// heal up Elliot if he's been hurt
		if ( gMercProfiles[ ELLIOT ].bLife < gMercProfiles[ ELLIOT ].bLifeMax )
		{
			gMercProfiles[ ELLIOT ].bLife = gMercProfiles[ ELLIOT ].bLifeMax;
		}
	}

	ResetOncePerConvoRecordsForAllNPCsInLoadedSector();
}


static void HandleQuestCodeOnSectorExit(INT16 sOldSectorX, INT16 sOldSectorY, INT8 bOldSectorZ)
{
	if ( sOldSectorX == KINGPIN_MONEY_SECTOR_X && sOldSectorY == KINGPIN_MONEY_SECTOR_Y && bOldSectorZ == KINGPIN_MONEY_SECTOR_Z )
	{
		CheckForKingpinsMoneyMissing( TRUE );
	}

	if ( sOldSectorX == 13 && sOldSectorY == MAP_ROW_H && bOldSectorZ == 0 && CheckFact( FACT_CONRAD_SHOULD_GO, 0 ) )
	{
		// remove Conrad from the map
		gMercProfiles[ CONRAD ].sSectorX = 0;
		gMercProfiles[ CONRAD ].sSectorY = 0;
	}

	if ( sOldSectorX == HOSPITAL_SECTOR_X && sOldSectorY == HOSPITAL_SECTOR_Y && bOldSectorZ == HOSPITAL_SECTOR_Z )
	{
		CheckForMissingHospitalSupplies();
	}

	// reset the state of the museum alarm for Eldin's quotes
	SetFactFalse( FACT_MUSEUM_ALARM_WENT_OFF );
}


static void SetupProfileInsertionDataForCivilians(void)
{
	FOR_ALL_IN_TEAM(s, CIV_TEAM)
	{
		if (s->bInSector) SetupProfileInsertionDataForSoldier(s);
	}
}


static BOOLEAN EnterSector(INT16 const x, INT16 const y, INT8 const z)
{
	PauseGame();
	// Stop time for this frame
	InterruptTime();

	/* Setup the tactical existance of RPCs and CIVs in the last sector before
	 * moving on to a new sector. */
	//@@@Evaluate
	if (gfWorldLoaded) SetupProfileInsertionDataForCivilians();

	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		// Handle NPC stuff related to changing sectors
		HandleQuestCodeOnSectorEntry(x, y, z);
	}

	BeginLoadScreen();

	/* This has to be done before loadworld, as it will remmove old gridnos if
	 * present */
	RemoveMercsInSector();

	if (!AreInMeanwhile())
	{
		SetSectorFlag(x, y, z, SF_ALREADY_VISITED);
	}

	CreateLoadingScreenProgressBar();
#ifdef JA2BETAVERSION
	SetProgressBarMsgAttributes(0, FONT12ARIAL, FONT_MCOLOR_WHITE, 0);
	// Set the tile so we do not see the text come up
	SetProgressBarTextDisplayFlag(0, TRUE, TRUE, TRUE);
#endif

	char filename[50];
	GetMapFileName(x, y, z, filename, TRUE, TRUE);
	if (!LoadWorld(filename)) return FALSE;

	/* ATE: Moved this form above, so that we can have the benefit of changing the
	 * world BEFORE adding guys to it. */
	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		try
		{ // Load the current sectors Information From the temporary files
			LoadCurrentSectorsInformationFromTempItemsFile();
		}
		catch (...)
		{ /* The integrity of the temp files have been compromised.  Boot out of the
			 * game after warning message. */
			InitExitGameDialogBecauseFileHackDetected();
			return TRUE;
		}
	}

	RemoveLoadingScreenProgressBar();

	if (gfEnterTacticalPlacementGUI)
	{
		SetPendingNewScreen(GAME_SCREEN);
		InitTacticalPlacementGUI();
	}
	else
	{
		EndMapScreen(FALSE);
		PrepareLoadedSector();
	}

	/* This function will either hide or display the tree tops, depending on the
	 * game setting */
	SetTreeTopStateForMap();

	return TRUE;
}


void UpdateMercsInSector(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	// Remove from interface slot
	RemoveAllPlayersFromSlot();

	// Remove tactical interface stuff
	guiPendingOverrideEvent = I_CHANGE_TO_IDLE;

	//If we are in this function during the loading of a sector
	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		//DONT set these values
		SetSelectedMan(NULL);
		gfGameScreenLocateToSoldier = TRUE;
	}

	SetAllAutoFacesInactive();

	if (fUsingEdgePointsForStrategicEntry)
	{
		BeginMapEdgepointSearch();
	}

	UINT8       pow_squad   = NO_CURRENT_SQUAD;
	UINT8 const first_enemy = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
	UINT8 const last_enemy  = gTacticalStatus.Team[CREATURE_TEAM].bLastID;
	for (INT32 i = 0; i != MAX_NUM_SOLDIERS; ++i)
	{
		if (gfRestoringEnemySoldiersFromTempFile &&
				first_enemy <= i && i <= last_enemy)
		{ /* Don't update enemies/creatures (consec. teams) if they were just
			 * restored via the temp map files */
			continue;
		}

		SOLDIERTYPE* const s = GetMan(i);
		RemoveMercSlot(s);

		s->bInSector = FALSE;

		if (!s->bActive)             continue;
		if (s->sSectorX != sSectorX) continue;
		if (s->sSectorY != sSectorY) continue;
		if (s->bSectorZ != bSectorZ) continue;
		if (s->fBetweenSectors)      continue;

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
		{
			if (gMapInformation.sCenterGridNo != -1 &&
					gfBlitBattleSectorLocator           &&
					s->bTeam != CIV_TEAM                &&
					(
						gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
						gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE
					))
			{
				s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				s->usStrategicInsertionData = gMapInformation.sCenterGridNo;
			}
			else if (gfOverrideInsertionWithExitGrid)
			{
				s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				s->usStrategicInsertionData = gExitGrid.usGridNo;
			}
		}

		UpdateMercInSector(s, sSectorX, sSectorY, bSectorZ);

		if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) continue;
		if (s->bAssignment != ASSIGNMENT_POW)             continue;

		if (pow_squad == NO_CURRENT_SQUAD)
		{
			// ATE: If we are in i13 - pop up message!
			if (sSectorY == MAP_ROW_I && sSectorX == 13)
			{
				DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[POW_MERCS_ARE_HERE], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
			}
			else
			{
				AddCharacterToUniqueSquad(s);
				pow_squad   = s->bAssignment;
				s->bNeutral = FALSE;
			}
		}
		else
		{
			if (sSectorY != MAP_ROW_I && sSectorX != 13)
			{
				AddCharacterToSquad(s, pow_squad);
			}
		}

		// ATE: Call actions based on what POW we are on...
		if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTINPROGRESS)
		{
			EndQuest(QUEST_HELD_IN_ALMA, sSectorX, sSectorY);
			HandleNPCDoAction(0, NPC_ACTION_GRANT_EXPERIENCE_3, 0);
		}
	}

	if (fUsingEdgePointsForStrategicEntry)
	{
		EndMapEdgepointSearch();
		fUsingEdgePointsForStrategicEntry = FALSE;
	}
}


static void GetLoadedSectorString(wchar_t* pString, size_t Length);


void UpdateMercInSector( SOLDIERTYPE *pSoldier, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	BOOLEAN fError = FALSE;
	// OK, determine entrence direction and get sweetspot
	// Only if we are an OK guy to control....
	// SOME CHECKS HERE MUST BE FLESHED OUT......
	if ( pSoldier->bActive )		// This was in the if, removed by DEF:  pSoldier->bLife >= OKLIFE &&
	{
		// If we are not in transit...
		if ( pSoldier->bAssignment != IN_TRANSIT )
		{
			// CHECK UBINSERTION CODE..
			if( pSoldier->ubStrategicInsertionCode == INSERTION_CODE_PRIMARY_EDGEINDEX ||
					pSoldier->ubStrategicInsertionCode == INSERTION_CODE_SECONDARY_EDGEINDEX )
			{
				if ( !fUsingEdgePointsForStrategicEntry )
				{
					// If we are not supposed to use this now, pick something better...
					pSoldier->ubStrategicInsertionCode = (UINT8)pSoldier->usStrategicInsertionData;
				}
			}

			MAPEDGEPOINT_SEARCH_FAILED:

			if ( pSoldier->ubProfile != NO_PROFILE && gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE )
			{
				// override orders
				pSoldier->bOrders = STATIONARY;
			}


			// Use insertion direction from loaded map!
			switch( pSoldier->ubStrategicInsertionCode )
			{
				case INSERTION_CODE_NORTH:
					pSoldier->sInsertionGridNo = gMapInformation.sNorthGridNo;
					if ( !gfEditMode && gMapInformation.sNorthGridNo == -1 )
						fError = TRUE;
					break;
				case INSERTION_CODE_SOUTH:
					pSoldier->sInsertionGridNo = gMapInformation.sSouthGridNo;
					if ( !gfEditMode && gMapInformation.sSouthGridNo == -1)
						fError = TRUE;
					break;
				case INSERTION_CODE_EAST:
					pSoldier->sInsertionGridNo = gMapInformation.sEastGridNo;
					if ( !gfEditMode && gMapInformation.sEastGridNo == -1)
						fError = TRUE;
					break;
				case INSERTION_CODE_WEST:
					pSoldier->sInsertionGridNo = gMapInformation.sWestGridNo;
					if ( !gfEditMode && gMapInformation.sWestGridNo == -1)
						fError = TRUE;
					break;
				case INSERTION_CODE_CENTER:
					pSoldier->sInsertionGridNo = gMapInformation.sCenterGridNo;
					if ( !gfEditMode && gMapInformation.sCenterGridNo == -1)
						fError = TRUE;
					break;
				case INSERTION_CODE_GRIDNO:
					pSoldier->sInsertionGridNo = pSoldier->usStrategicInsertionData;
					break;

				case INSERTION_CODE_PRIMARY_EDGEINDEX:
					pSoldier->sInsertionGridNo = SearchForClosestPrimaryMapEdgepoint( pSoldier->sPendingActionData2, (UINT8)pSoldier->usStrategicInsertionData );
					#ifdef JA2BETAVERSION
					{
						char str[256];
						sprintf( str, "%ls's primary insertion gridno is %d using %d as initial search gridno and %d insertion code.",
													pSoldier->name, pSoldier->sInsertionGridNo, pSoldier->sPendingActionData2, pSoldier->usStrategicInsertionData );
						DebugMsg( TOPIC_JA2, DBG_LEVEL_3, str );
					}
					#endif
					if( pSoldier->sInsertionGridNo == NOWHERE )
					{
						ScreenMsg(FONT_RED, MSG_ERROR, L"Main edgepoint search failed for %ls -- substituting entrypoint.", pSoldier->name);
						pSoldier->ubStrategicInsertionCode = (UINT8)pSoldier->usStrategicInsertionData;
						goto MAPEDGEPOINT_SEARCH_FAILED;
					}
					break;
				case INSERTION_CODE_SECONDARY_EDGEINDEX:
					pSoldier->sInsertionGridNo = SearchForClosestSecondaryMapEdgepoint( pSoldier->sPendingActionData2, (UINT8)pSoldier->usStrategicInsertionData );
					#ifdef JA2BETAVERSION
					{
						char str[256];
						sprintf( str, "%ls's isolated insertion gridno is %d using %d as initial search gridno and %d insertion code.",
													pSoldier->name, pSoldier->sInsertionGridNo, pSoldier->sPendingActionData2, pSoldier->usStrategicInsertionData );
						DebugMsg( TOPIC_JA2, DBG_LEVEL_3, str );
					}
					#endif
					if( pSoldier->sInsertionGridNo == NOWHERE )
					{
						ScreenMsg(FONT_RED, MSG_ERROR, L"Isolated edgepont search failed for %ls -- substituting entrypoint.", pSoldier->name);
						pSoldier->ubStrategicInsertionCode = (UINT8)pSoldier->usStrategicInsertionData;
						goto MAPEDGEPOINT_SEARCH_FAILED;
					}
					break;

				case INSERTION_CODE_ARRIVING_GAME:
					// Are we in Omerta!
					if ( sSectorX == gWorldSectorX && gWorldSectorX == 9 && sSectorY == gWorldSectorY && gWorldSectorY == 1 && bSectorZ == gbWorldSectorZ && gbWorldSectorZ == 0 )
					{
						// Try another location and walk into map
						pSoldier->sInsertionGridNo = 4379;
					}
					else
					{
						pSoldier->ubStrategicInsertionCode = INSERTION_CODE_NORTH;
						pSoldier->sInsertionGridNo				 = gMapInformation.sNorthGridNo;
					}
					break;
				case INSERTION_CODE_CHOPPER:
					// Try another location and walk into map
					// Add merc to chopper....
					//pSoldier->sInsertionGridNo = 4058;
					AddMercToHeli(pSoldier);
					return;

				default:
					pSoldier->sInsertionGridNo = 12880;
					DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Improper insertion code %d given to UpdateMercsInSector", pSoldier->ubStrategicInsertionCode ) );
					break;
			}

			if( fError )
			{ //strategic insertion failed because it expected to find an entry point.  This is likely
				//a missing part of the map or possible fault in strategic movement costs, traversal logic, etc.
				const wchar_t* Entry;
				wchar_t szSector[10];
				INT16 sGridNo;
				GetLoadedSectorString(szSector, lengthof(szSector));
				if( gMapInformation.sNorthGridNo != -1 )
				{
					Entry = L"north";
					sGridNo = gMapInformation.sNorthGridNo;
				}
				else if( gMapInformation.sEastGridNo != -1 )
				{
					Entry = L"east";
					sGridNo = gMapInformation.sEastGridNo;
				}
				else if( gMapInformation.sSouthGridNo != -1 )
				{
					Entry = L"south";
					sGridNo = gMapInformation.sSouthGridNo;
				}
				else if( gMapInformation.sWestGridNo != -1 )
				{
					Entry = L"west";
					sGridNo = gMapInformation.sWestGridNo;
				}
				else if( gMapInformation.sCenterGridNo != -1 )
				{
					Entry = L"center";
					sGridNo = gMapInformation.sCenterGridNo;
				}
				else
				{
					ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Sector %ls has NO entrypoints -- using precise center of map for %ls.", szSector, pSoldier->name);
					pSoldier->sInsertionGridNo = 12880;
					AddSoldierToSector(pSoldier);
					return;
				}
				pSoldier->sInsertionGridNo = sGridNo;
				switch( pSoldier->ubStrategicInsertionCode )
				{
					case INSERTION_CODE_NORTH:
						ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Sector %ls doesn't have a north entrypoint -- substituting  %ls entrypoint for %ls.", szSector, Entry, pSoldier->name);
						break;
					case INSERTION_CODE_EAST:
						ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Sector %ls doesn't have a east entrypoint -- substituting  %ls entrypoint for %ls.", szSector, Entry, pSoldier->name);
						break;
					case INSERTION_CODE_SOUTH:
						ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Sector %ls doesn't have a south entrypoint -- substituting  %ls entrypoint for %ls.", szSector, Entry, pSoldier->name);
						break;
					case INSERTION_CODE_WEST:
						ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Sector %ls doesn't have a west entrypoint -- substituting  %ls entrypoint for %ls.", szSector, Entry, pSoldier->name);
						break;
					case INSERTION_CODE_CENTER:
						ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Sector %ls doesn't have a center entrypoint -- substituting  %ls entrypoint for %ls.", szSector, Entry, pSoldier->name);
						break;
				}
			}
			// If no insertion direction exists, this is bad!
			if ( pSoldier->sInsertionGridNo == -1 )
			{
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Insertion gridno for direction %d not added to map sector %d %d", pSoldier->ubStrategicInsertionCode, sSectorX, sSectorY ) );
				pSoldier->sInsertionGridNo = 12880;
			}

			AddSoldierToSector(pSoldier);
		}
	}
}


static void InitializeStrategicMapSectorTownNames(void)
{
  StrategicMap[2+2*MAP_WORLD_X].bNameId= StrategicMap[2+1*MAP_WORLD_X].bNameId= CHITZENA;
	StrategicMap[5+3*MAP_WORLD_X].bNameId=StrategicMap[6+3*MAP_WORLD_X].bNameId=StrategicMap[5+4*MAP_WORLD_X].bNameId = StrategicMap[4+4*MAP_WORLD_X].bNameId =SAN_MONA;
	StrategicMap[9+1*MAP_WORLD_X].bNameId=StrategicMap[10+1*MAP_WORLD_X].bNameId=OMERTA;
  StrategicMap[13+2*MAP_WORLD_X].bNameId=StrategicMap[13+3*MAP_WORLD_X].bNameId=StrategicMap[13+4*MAP_WORLD_X].bNameId=DRASSEN;
	StrategicMap[1+7*MAP_WORLD_X].bNameId=StrategicMap[1+8*MAP_WORLD_X].bNameId=StrategicMap[2+7*MAP_WORLD_X].bNameId= StrategicMap[2+8*MAP_WORLD_X].bNameId = StrategicMap[3+8*MAP_WORLD_X].bNameId = GRUMM;
	StrategicMap[6+9*MAP_WORLD_X].bNameId=ESTONI;
  StrategicMap[9+10*MAP_WORLD_X].bNameId=TIXA;
	StrategicMap[8+6*MAP_WORLD_X].bNameId=StrategicMap[9+6*MAP_WORLD_X].bNameId=StrategicMap[8+7*MAP_WORLD_X].bNameId=StrategicMap[9+7*MAP_WORLD_X].bNameId= StrategicMap[8+8*MAP_WORLD_X].bNameId = CAMBRIA;
	StrategicMap[13+9*MAP_WORLD_X].bNameId=StrategicMap[14+9*MAP_WORLD_X].bNameId=StrategicMap[13+8*MAP_WORLD_X].bNameId=StrategicMap[14+8*MAP_WORLD_X].bNameId=ALMA;
	StrategicMap[4+11*MAP_WORLD_X].bNameId=ORTA;
	StrategicMap[11+12*MAP_WORLD_X].bNameId= 	StrategicMap[12+12*MAP_WORLD_X].bNameId = BALIME;
	StrategicMap[3+14*MAP_WORLD_X].bNameId=StrategicMap[4+14*MAP_WORLD_X].bNameId=StrategicMap[5+14*MAP_WORLD_X].bNameId=StrategicMap[3+15*MAP_WORLD_X].bNameId=StrategicMap[4+15*MAP_WORLD_X].bNameId= StrategicMap[3+16*MAP_WORLD_X].bNameId = MEDUNA;
	//StrategicMap[3+16*MAP_WORLD_X].bNameId=PALACE;
}


// Get sector ID string makes a string like 'A9 - OMERTA', or just J11 if no town....
void GetSectorIDString( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ , CHAR16 *zString, size_t Length, BOOLEAN fDetailed )
{
#ifdef JA2DEMO
	wcslcpy(zString, L"Demoville", Length);
#else
	SECTORINFO *pSector = NULL;
	UNDERGROUND_SECTORINFO *pUnderground;
	INT8		bTownNameID;
	INT8		bMineIndex;
	UINT8 ubSectorID = 0;
	UINT8 ubLandType = 0;

	if( sSectorX <= 0 || sSectorY <= 0 || bSectorZ < 0 ) /* Empty? */
	{
		//swprintf(zString, L"%ls", pErrorStrings);
	}
	else if( bSectorZ != 0 )
	{
		pUnderground = FindUnderGroundSector( sSectorX, sSectorY, bSectorZ );
		if (pUnderground && (pUnderground->uiFlags & SF_ALREADY_VISITED || gfGettingNameFromSaveLoadScreen))
		{
			bMineIndex = GetIdOfMineForSector( sSectorX, sSectorY, bSectorZ );
			if( bMineIndex != -1 )
			{
				swprintf( zString, Length, L"%c%d: %ls %ls", 'A' + sSectorY - 1, sSectorX, pTownNames[ GetTownAssociatedWithMine( bMineIndex ) ], pwMineStrings[ 0 ] );
			}
			else switch( SECTOR( sSectorX, sSectorY ) )
			{
				case SEC_A10:
					swprintf( zString, Length, L"A10: %ls", pLandTypeStrings[ REBEL_HIDEOUT ] );
					break;
				case SEC_J9:
					swprintf( zString, Length, L"J9: %ls", pLandTypeStrings[ TIXA_DUNGEON ] );
					break;
				case SEC_K4:
					swprintf( zString, Length, L"K4: %ls", pLandTypeStrings[ ORTA_BASEMENT ] );
					break;
				case SEC_O3:
					swprintf( zString, Length, L"O3: %ls", pLandTypeStrings[ TUNNEL ] );
					break;
				case SEC_P3:
					swprintf( zString, Length, L"P3: %ls", pLandTypeStrings[ SHELTER ] );
					break;
				default:
					swprintf( zString, Length, L"%c%d: %ls", 'A' + sSectorY - 1, sSectorX, pLandTypeStrings[ CREATURE_LAIR ] );
					break;
			}
		}
		else
		{ //Display nothing
			wcscpy( zString, L"" );
		}
	}
	else
	{
		bTownNameID = StrategicMap[ CALCULATE_STRATEGIC_INDEX( sSectorX, sSectorY ) ].bNameId;
		ubSectorID = (UINT8)SECTOR( sSectorX, sSectorY );
		pSector = &SectorInfo[ ubSectorID ];
		ubLandType = pSector->ubTraversability[ 4 ];
		swprintf( zString, Length, L"%c%d: ", 'A' + sSectorY - 1, sSectorX );

		if ( bTownNameID == BLANK_SECTOR )
		{
			// OK, build string id like J11
			// are we dealing with the unfound towns?
			switch( ubSectorID )
			{
				case SEC_D2: //Chitzena SAM
					if( !fSamSiteFound[ SAM_SITE_ONE ] )
						wcscat( zString, pLandTypeStrings[ TROPICS ] );
					else if( fDetailed )
						wcscat( zString, pLandTypeStrings[ TROPICS_SAM_SITE ] );
					else
						wcscat( zString, pLandTypeStrings[ SAM_SITE ] );
					break;
				case SEC_D15: //Drassen SAM
					if( !fSamSiteFound[ SAM_SITE_TWO ] )
						wcscat( zString, pLandTypeStrings[ SPARSE ] );
					else if( fDetailed )
						wcscat( zString, pLandTypeStrings[ SPARSE_SAM_SITE ] );
					else
						wcscat( zString, pLandTypeStrings[ SAM_SITE ] );
					break;
				case SEC_I8: //Cambria SAM
					if( !fSamSiteFound[ SAM_SITE_THREE ] )
						wcscat( zString, pLandTypeStrings[ SAND ] );
					else if( fDetailed )
						wcscat( zString, pLandTypeStrings[ SAND_SAM_SITE ] );
					else
						wcscat( zString, pLandTypeStrings[ SAM_SITE ] );
					break;
				default:
					wcscat( zString, pLandTypeStrings[ ubLandType ] );
					break;
			}
		}
		else
		{
			switch( ubSectorID )
			{
				case SEC_B13:
					if( fDetailed )
						wcscat( zString, pLandTypeStrings[ DRASSEN_AIRPORT_SITE ] );
					else
						wcscat( zString, pTownNames[ DRASSEN ] );
					break;
				case SEC_F8:
					if( fDetailed )
						wcscat( zString, pLandTypeStrings[ CAMBRIA_HOSPITAL_SITE ] );
					else
						wcscat( zString, pTownNames[ CAMBRIA ] );
					break;
				case SEC_J9: //Tixa
					if( !fFoundTixa )
						wcscat( zString, pLandTypeStrings[ SAND ] );
					else
						wcscat( zString, pTownNames[ TIXA ] );
					break;
				case SEC_K4: //Orta
					if( !fFoundOrta )
						wcscat( zString, pLandTypeStrings[ SWAMP ] );
					else
						wcscat( zString, pTownNames[ ORTA ] );
					break;
				case SEC_N3:
					if( fDetailed )
						wcscat( zString, pLandTypeStrings[ MEDUNA_AIRPORT_SITE ] );
					else
						wcscat( zString, pTownNames[ MEDUNA ] );
					break;
				default:
					if( ubSectorID == SEC_N4 && fSamSiteFound[ SAM_SITE_FOUR ] )
					{	//Meduna's SAM site
						if( fDetailed )
							wcscat( zString, pLandTypeStrings[ MEDUNA_SAM_SITE ] );
						else
							wcscat( zString, pLandTypeStrings[ SAM_SITE ] );
					}
					else
					{	//All other towns that are known since beginning of the game.
						wcscat( zString, pTownNames[ bTownNameID ] );
						if( fDetailed )
						{
							switch( ubSectorID )
							{ //Append the word, "mine" for town sectors containing a mine.
								case SEC_B2:
								case SEC_D4:
								case SEC_D13:
								case SEC_H3:
								case SEC_H8:
								case SEC_I14:
									wcscat( zString, L" " ); //space
									wcscat( zString, pwMineStrings[ 0 ] ); //then "Mine"
									break;
							}
						}
					}
					break;
			}
		}
	}
#endif
}


static UINT8 SetInsertionDataFromAdjacentMoveDirection(SOLDIERTYPE* pSoldier, UINT8 ubTacticalDirection, INT16 sAdditionalData)
{
	UINT8				ubDirection;
	EXITGRID		ExitGrid;


	// Set insertion code
	switch( ubTacticalDirection )
	{
		// OK, we are using an exit grid - set insertion values...

		case 255:
			if ( !GetExitGrid( sAdditionalData, &ExitGrid ) )
			{
				AssertMsg(0, "No valid Exit grid can be found when one was expected: SetInsertionDataFromAdjacentMoveDirection.");
			}
			ubDirection = 255;
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			pSoldier->usStrategicInsertionData = ExitGrid.usGridNo;
			pSoldier->bUseExitGridForReentryDirection = TRUE;
			break;

		case NORTH:
			ubDirection = NORTH_STRATEGIC_MOVE;
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SOUTH:
			ubDirection = SOUTH_STRATEGIC_MOVE;
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_NORTH;
			break;
		case EAST:
			ubDirection = EAST_STRATEGIC_MOVE;
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_WEST;
			break;
		case WEST:
			ubDirection = WEST_STRATEGIC_MOVE;
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_EAST;
			break;
		default:
			// Wrong direction given!
			#ifdef JA2BETAVERSION
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Improper insertion direction %d given to SetInsertionDataFromAdjacentMoveDirection", ubTacticalDirection ) );
				ScreenMsg( FONT_RED, MSG_ERROR, L"Improper insertion direction %d given to SetInsertionDataFromAdjacentMoveDirection", ubTacticalDirection );
			#endif
			ubDirection = EAST_STRATEGIC_MOVE;
			pSoldier->ubStrategicInsertionCode = INSERTION_CODE_WEST;
	}

	return( ubDirection );

}


static UINT8 GetInsertionDataFromAdjacentMoveDirection(UINT8 ubTacticalDirection, INT16 sAdditionalData)
{
	UINT8				ubDirection;


	// Set insertion code
	switch( ubTacticalDirection )
	{
		// OK, we are using an exit grid - set insertion values...

		case 255:

			ubDirection = 255;
			break;

		case NORTH:
			ubDirection = NORTH_STRATEGIC_MOVE;
			break;
		case SOUTH:
			ubDirection = SOUTH_STRATEGIC_MOVE;
			break;
		case EAST:
			ubDirection = EAST_STRATEGIC_MOVE;
			break;
		case WEST:
			ubDirection = WEST_STRATEGIC_MOVE;
			break;
		default:
			// Wrong direction given!
			#ifdef JA2BETAVERSION
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Improper insertion direction %d given to GetInsertionDataFromAdjacentMoveDirection", ubTacticalDirection ) );
				ScreenMsg( FONT_RED, MSG_ERROR, L"Improper insertion direction %d given to GetInsertionDataFromAdjacentMoveDirection", ubTacticalDirection );
			#endif
			ubDirection = EAST_STRATEGIC_MOVE;
	}

	return( ubDirection );

}


static UINT8 GetStrategicInsertionDataFromAdjacentMoveDirection(UINT8 ubTacticalDirection, INT16 sAdditionalData)
{
	UINT8				ubDirection;


	// Set insertion code
	switch( ubTacticalDirection )
	{
		// OK, we are using an exit grid - set insertion values...

		case 255:

			ubDirection = 255;
			break;

		case NORTH:
			ubDirection = INSERTION_CODE_SOUTH;
			break;
		case SOUTH:
			ubDirection = INSERTION_CODE_NORTH;
			break;
		case EAST:
			ubDirection = INSERTION_CODE_WEST;
			break;
		case WEST:
			ubDirection = INSERTION_CODE_EAST;
			break;
		default:
			// Wrong direction given!
			#ifdef JA2BETAVERSION
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Improper insertion direction %d given to SetInsertionDataFromAdjacentMoveDirection", ubTacticalDirection ) );
				ScreenMsg( FONT_RED, MSG_ERROR, L"Improper insertion direction %d given to GetStrategicInsertionDataFromAdjacentMoveDirection", ubTacticalDirection );
			#endif
			ubDirection = EAST_STRATEGIC_MOVE;
	}

	return( ubDirection );

}


static INT16 PickGridNoNearestEdge(SOLDIERTYPE* pSoldier, UINT8 ubTacticalDirection);


void JumpIntoAdjacentSector( UINT8 ubTacticalDirection, UINT8 ubJumpCode, INT16 sAdditionalData )
{
	SOLDIERTYPE *pValidSoldier = NULL;
	UINT32 uiTraverseTime=0;
	UINT8 ubDirection = (UINT8)-1; // XXX HACK000E
	EXITGRID ExitGrid;

	// Set initial selected
	// ATE: moved this towards top...
	SOLDIERTYPE* const sel = GetSelectedMan();
	gPreferredInitialSelectedGuy = sel;

	if ( ubJumpCode == JUMP_ALL_LOAD_NEW || ubJumpCode == JUMP_ALL_NO_LOAD )
	{
		// TODO: Check flags to see if we can jump!
		// Move controllable mercs!
		FOR_ALL_IN_TEAM(s, gbPlayerNum)
		{
			// If we are controllable
			if (OkControllableMerc(s) && s->bAssignment == CurrentSquad())
			{
				pValidSoldier = s;
				//This now gets handled by strategic movement.  It is possible that the
				//group won't move instantaneously.
				//s->sSectorX = sNewX;
				//s->sSectorY = sNewY;

				ubDirection = GetInsertionDataFromAdjacentMoveDirection( ubTacticalDirection, sAdditionalData );
				break;
			}
		}
	}
	else if ( ( ubJumpCode == JUMP_SINGLE_LOAD_NEW || ubJumpCode == JUMP_SINGLE_NO_LOAD ) )
	{
		// Use selected soldier...
		// This guy should always be 1 ) selected and 2 ) close enough to exit sector to leave
		if (sel != NULL)
		{
			pValidSoldier = sel;
			ubDirection = GetInsertionDataFromAdjacentMoveDirection( ubTacticalDirection, sAdditionalData );
		}

		if( ubJumpCode == JUMP_SINGLE_NO_LOAD )
		{ // handle soldier moving by themselves
			HandleSoldierLeavingSectorByThemSelf( pValidSoldier );
		}
		else
		{ // now add char to a squad all their own
			AddCharacterToUniqueSquad( pValidSoldier );
		}
	}
	else
	{
		// OK, no jump code here given...
		DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Improper jump code %d given to JumpIntoAdjacentSector", ubJumpCode ) );
	}

	Assert( pValidSoldier );

	//Now, determine the traversal time.
	GROUP* const pGroup = GetGroup(pValidSoldier->ubGroupID);
	AssertMsg( pGroup, String( "%ls is not in a valid group (pSoldier->ubGroupID is %d)",pValidSoldier->name, pValidSoldier->ubGroupID) );

	// If we are going through an exit grid, don't get traversal direction!
	if ( ubTacticalDirection != 255 )
	{
		if( !gbWorldSectorZ )
		{
			uiTraverseTime = GetSectorMvtTimeForGroup( (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ), ubDirection, pGroup );
		}
		else if( gbWorldSectorZ > 0 )
		{ //We are attempting to traverse in an underground environment.  We need to use a complete different
			//method.  When underground, all sectors are instantly adjacent.
			uiTraverseTime = UndergroundTacticalTraversalTime( ubDirection );
		}
		AssertMsg(uiTraverseTime != 0xFFFFFFFF, "Attempting to tactically traverse to adjacent sector, despite being unable to do so.");
	}

	// Alrighty, we want to do whatever our omnipotent player asked us to do
	// this is what the ubJumpCode is for.
	// Regardless of that we were asked to do, we MUST walk OFF ( Ian loves this... )
	// So..... let's setup our people to walk off...
	// We deal with a pGroup here... if an all move or a group...

	// Setup some globals so our callback that deals when guys go off screen is handled....
	// Look in the handler function AllMercsHaveWalkedOffSector() below...
	gpAdjacentGroup				= pGroup;
	gubAdjacentJumpCode		= ubJumpCode;
	guiAdjacentTraverseTime	= uiTraverseTime;
	gubTacticalDirection  = ubTacticalDirection;
	gsAdditionalData			= sAdditionalData;

	// If normal direction, use it!
	if ( ubTacticalDirection != 255 )
	{
		gsAdjacentSectorX				= (INT16)(gWorldSectorX + DirXIncrementer[ ubTacticalDirection ]);
		gsAdjacentSectorY				= (INT16)(gWorldSectorY + DirYIncrementer[ ubTacticalDirection ]);
		gbAdjacentSectorZ				= pValidSoldier->bSectorZ;
	}
	else
	{
		// Take directions from exit grid info!
		if ( !GetExitGrid( sAdditionalData, &ExitGrid ) )
		{
			AssertMsg( 0, String( "Told to use exit grid at %d but one does not exist", sAdditionalData ) );
		}

		gsAdjacentSectorX				= ExitGrid.ubGotoSectorX;
		gsAdjacentSectorY				= ExitGrid.ubGotoSectorY;
		gbAdjacentSectorZ				= ExitGrid.ubGotoSectorZ;
		gusDestExitGridNo				= ExitGrid.usGridNo;
	}

	// Give guy(s) orders to walk off sector...
	if( pGroup->fPlayer )
	{	//For player groups, update the soldier information
		UINT8				ubNum = 0;

		CFOR_ALL_PLAYERS_IN_GROUP(curr, pGroup)
		{
			if ( OK_CONTROLLABLE_MERC( curr->pSoldier) )
			{
				if ( ubTacticalDirection != 255 )
				{
					const INT16 sGridNo = PickGridNoNearestEdge(curr->pSoldier, ubTacticalDirection);

					curr->pSoldier->sPreTraversalGridNo = curr->pSoldier->sGridNo;

					if ( sGridNo != NOWHERE )
					{
						// Save wait code - this will make buddy walk off screen into oblivion
						curr->pSoldier->ubWaitActionToDo = 2;
						// This will set the direction so we know now to move into oblivion
						curr->pSoldier->uiPendingActionData1		 = ubTacticalDirection;
					}
					else
					{
						AssertMsg(0, "Failed to get good exit location for adjacentmove");
					}

					EVENT_GetNewSoldierPath( curr->pSoldier, sGridNo, WALKING );

				}
				else
				{
						// Here, get closest location for exit grid....
						const INT16 sGridNo = FindGridNoFromSweetSpotCloseToExitGrid(curr->pSoldier, sAdditionalData, 10);

						//curr->pSoldier->
						if ( sGridNo != NOWHERE )
						{
							// Save wait code - this will make buddy walk off screen into oblivion
						//	curr->pSoldier->ubWaitActionToDo = 2;
						}
						else
						{
							AssertMsg(0, "Failed to get good exit location for adjacentmove");
						}

						// Don't worry about walk off screen, just stay at gridno...
						curr->pSoldier->ubWaitActionToDo = 1;

						// Set buddy go!
						gfPlotPathToExitGrid = TRUE;
						EVENT_GetNewSoldierPath( curr->pSoldier, sGridNo, WALKING );
						gfPlotPathToExitGrid = FALSE;

				}
				ubNum++;
			}
			else
			{
				// We will remove them later....
			}
		}

		// ATE: Do another round, removing guys from group that can't go on...
BEGINNING_LOOP:
		CFOR_ALL_PLAYERS_IN_GROUP(curr, pGroup)
		{
			if ( !OK_CONTROLLABLE_MERC( curr->pSoldier ) )
			{
				RemoveCharacterFromSquads( curr->pSoldier );
				goto BEGINNING_LOOP;
			}
		}

		// OK, setup TacticalOverhead polling system that will notify us once everybody
		// has made it to our destination.
		const UINT8 action = (ubTacticalDirection == 255 ?
			WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO : WAIT_FOR_MERCS_TO_WALKOFF_SCREEN);
		SetActionToDoOnceMercsGetToLocation(action, ubNum);

		// Lock UI!
		guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI( );
	}
}


void HandleSoldierLeavingSectorByThemSelf( SOLDIERTYPE *pSoldier )
{
	// soldier leaving thier squad behind, will rejoin later
	// if soldier in a squad, set the fact they want to return here

	if( pSoldier->bAssignment < ON_DUTY )
	{
			RemoveCharacterFromSquads( pSoldier ); // REDUNDANT AddCharacterToUniqueSquad()

		// are they in a group?..remove from group
		if( pSoldier->ubGroupID != 0 )
		{
			// remove from group
			RemovePlayerFromGroup(pSoldier);
			pSoldier->ubGroupID = 0;
		}
	}
	else
	{
		// otherwise, they are on thier own, not in a squad, simply remove mvt group
		if( pSoldier->ubGroupID && pSoldier->bAssignment != VEHICLE )
		{ //Can only remove groups if they aren't persistant (not in a squad or vehicle)
			// delete group
			RemoveGroup( pSoldier->ubGroupID );
			pSoldier->ubGroupID = 0;
		}
	}

	// set to guard
	AddCharacterToUniqueSquad( pSoldier );

	if( pSoldier->ubGroupID == 0 )
	{
	  // create independant group
	  GROUP* const g = CreateNewPlayerGroupDepartingFromSector(pSoldier->sSectorX, pSoldier->sSectorY);
	  AddPlayerToGroup(g, pSoldier);
	}
}


static void DoneFadeOutExitGridSector(void);
static void HandlePotentialMoraleHitForSkimmingSectors(GROUP* pGroup);


void AllMercsWalkedToExitGrid()
{
	BOOLEAN fDone;

  HandlePotentialMoraleHitForSkimmingSectors( gpAdjacentGroup );

	if( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD )
	{
		Assert( gpAdjacentGroup );
		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, gpAdjacentGroup)
		{
			SetInsertionDataFromAdjacentMoveDirection( pPlayer->pSoldier, gubTacticalDirection, gsAdditionalData );

			RemoveSoldierFromTacticalSector(pPlayer->pSoldier);
		}

		SetGroupSectorValue(gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ, gpAdjacentGroup);

		SetDefaultSquadOnSectorEntry( TRUE );

	}
	else
	{
		//Because we are actually loading the new map, and we are physically traversing, we don't want
		//to bring up the prebattle interface when we arrive if there are enemies there.  This flag
		//ignores the initialization of the prebattle interface and clears the flag.
		gfTacticalTraversal = TRUE;
		gpTacticalTraversalGroup = gpAdjacentGroup;

		//Check for any unconcious and/or dead merc and remove them from the current squad, so that they
		//don't get moved to the new sector.
		fDone = FALSE;
		while( !fDone )
		{
			fDone = FALSE;
			const PLAYERGROUP* pPlayer = gpAdjacentGroup->pPlayerList;
			while( pPlayer )
			{
				if( pPlayer->pSoldier->bLife < OKLIFE )
				{
					AddCharacterToUniqueSquad( pPlayer->pSoldier );
					break;
				}
				pPlayer = pPlayer->next;
			}
			if( !pPlayer )
			{
				fDone = TRUE;
			}
		}

		// OK, Set insertion direction for all these guys....
		Assert( gpAdjacentGroup );
		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, gpAdjacentGroup)
		{
			SetInsertionDataFromAdjacentMoveDirection( pPlayer->pSoldier, gubTacticalDirection, gsAdditionalData );
		}
		SetGroupSectorValue(gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ, gpAdjacentGroup);

		gFadeOutDoneCallback = DoneFadeOutExitGridSector;
		FadeOutGameScreen( );
	}
	if( !PlayerMercsInSector( (UINT8)gsAdjacentSectorX, (UINT8)gsAdjacentSectorY, (UINT8)gbAdjacentSectorZ ) )
	{
		HandleLoyaltyImplicationsOfMercRetreat( RETREAT_TACTICAL_TRAVERSAL, gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ );
	}
	if( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD )
	{
		gfTacticalTraversal = FALSE;
		gpTacticalTraversalGroup = NULL;
		gpTacticalTraversalChosenSoldier = NULL;
	}
}


static void SetupTacticalTraversalInformation(void)
{
	SOLDIERTYPE *pSoldier;
	INT16 sScreenX, sScreenY;

	Assert( gpAdjacentGroup );
	CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, gpAdjacentGroup)
	{
		pSoldier = pPlayer->pSoldier;

		SetInsertionDataFromAdjacentMoveDirection( pSoldier, gubTacticalDirection, gsAdditionalData );

		// pass flag that this is a tactical traversal, the path built MUST go in the traversed direction even if longer!
		PlotPathForCharacter( pSoldier, gsAdjacentSectorX, gsAdjacentSectorY, TRUE );

		if( guiAdjacentTraverseTime <= 5 )
		{
			// Determine 'mirror' gridno...
			// Convert to absolute xy
			GetAbsoluteScreenXYFromMapPos(GETWORLDINDEXFROMWORLDCOORDS(pSoldier->sY, pSoldier->sX), &sScreenX, &sScreenY);

			// Get 'mirror', depending on what direction...
			switch( gubTacticalDirection )
			{
				case NORTH:			sScreenY = 1520;				break;
				case SOUTH:			sScreenY = 0;						break;
				case EAST:			sScreenX = 0;						break;
				case WEST:			sScreenX = 3160;				break;
			}

			// Convert into a gridno again.....
			const GridNo sNewGridNo = GetMapPosFromAbsoluteScreenXY(sScreenX, sScreenY);

			// Save this gridNo....
			pSoldier->sPendingActionData2				= sNewGridNo;
			// Copy CODe computed earlier into data
			pSoldier->usStrategicInsertionData  = pSoldier->ubStrategicInsertionCode;
			// Now use NEW code....

			pSoldier->ubStrategicInsertionCode = CalcMapEdgepointClassInsertionCode( pSoldier->sPreTraversalGridNo );

			if( gubAdjacentJumpCode == JUMP_SINGLE_LOAD_NEW || gubAdjacentJumpCode == JUMP_ALL_LOAD_NEW )
			{
				fUsingEdgePointsForStrategicEntry = TRUE;
			}
		}
	}
	if( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD )
	{
		gfTacticalTraversal = FALSE;
		gpTacticalTraversalGroup = NULL;
		gpTacticalTraversalChosenSoldier = NULL;
	}
}


static void DoneFadeOutAdjacentSector(void);


void AllMercsHaveWalkedOffSector( )
{
	BOOLEAN fEnemiesInLoadedSector = FALSE;

	if( NumEnemiesInAnySector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ ) )
	{
		fEnemiesInLoadedSector = TRUE;
	}

	HandleLoyaltyImplicationsOfMercRetreat( RETREAT_TACTICAL_TRAVERSAL, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//Setup strategic traversal information
	if( guiAdjacentTraverseTime <= 5 )
	{
		gfTacticalTraversal = TRUE;
		gpTacticalTraversalGroup = gpAdjacentGroup;

		if( gbAdjacentSectorZ > 0 && guiAdjacentTraverseTime <= 5 )
		{	//Nasty strategic movement logic doesn't like underground sectors!
			gfUndergroundTacticalTraversal = TRUE;
		}
	}
	ClearMercPathsAndWaypointsForAllInGroup( gpAdjacentGroup );
	AddWaypointToPGroup( gpAdjacentGroup, (UINT8)gsAdjacentSectorX, (UINT8)gsAdjacentSectorY );
	if( gbAdjacentSectorZ > 0 && guiAdjacentTraverseTime <= 5 )
	{	//Nasty strategic movement logic doesn't like underground sectors!
		gfUndergroundTacticalTraversal = TRUE;
	}

	SetupTacticalTraversalInformation();

	// ATE: Added here: donot load another screen if we were told not to....
	if( ( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD ) )
	{ //Case 1:  Group is leaving sector, but there are other mercs in sector and player wants to stay, or
		//         there are other mercs in sector while a battle is in progress.
		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, gpAdjacentGroup)
		{
			RemoveSoldierFromTacticalSector(pPlayer->pSoldier);
		}
		SetDefaultSquadOnSectorEntry( TRUE );
	}
	else
	{
		if( fEnemiesInLoadedSector )
		{ //We are retreating from a sector with enemies in it and there are no mercs left  so
			//warp the game time by 5 minutes to simulate the actual retreat.  This restricts the
			//player from immediately coming back to the same sector they left to perhaps take advantage
			//of the tactical placement gui to get into better position.  Additionally, if there are any
			//enemies in this sector that are part of a movement group, reset that movement group so that they
			//are "in" the sector rather than 75% of the way to the next sector if that is the case.
			ResetMovementForEnemyGroupsInLocation( (UINT8)gWorldSectorX, (UINT8)gWorldSectorY );

			if( guiAdjacentTraverseTime > 5 )
			{
				//Because this final group is retreating, simulate extra time to retreat, so they can't immediately come back.
				WarpGameTime( 300, WARPTIME_NO_PROCESSING_OF_EVENTS );
			}
		}
		if( guiAdjacentTraverseTime <= 5 )
		{
			//Case 2:  Immediatly loading the next sector
			if( !gbAdjacentSectorZ )
			{
				UINT32 uiWarpTime;
				uiWarpTime = (GetWorldTotalMin() + 5) * 60 - GetWorldTotalSeconds();
				WarpGameTime( uiWarpTime, WARPTIME_PROCESS_TARGET_TIME_FIRST );
			}
			else if( gbAdjacentSectorZ > 0 )
			{
				UINT32 uiWarpTime;
				uiWarpTime = (GetWorldTotalMin() + 1) * 60 - GetWorldTotalSeconds();
				WarpGameTime( uiWarpTime, WARPTIME_PROCESS_TARGET_TIME_FIRST );
			}

			//Because we are actually loading the new map, and we are physically traversing, we don't want
			//to bring up the prebattle interface when we arrive if there are enemies there.  This flag
			//ignores the initialization of the prebattle interface and clears the flag.
			gFadeOutDoneCallback = DoneFadeOutAdjacentSector;
			FadeOutGameScreen( );
		}
		else
		{ //Case 3:  Going directly to mapscreen

			//Lock game into mapscreen mode, but after the fade is done.
			gfEnteringMapScreen = TRUE;

			// ATE; Fade FAST....
			SetMusicFadeSpeed( 5 );
			SetMusicMode( MUSIC_TACTICAL_NOTHING );
		}
	}
}


static void DoneFadeOutExitGridSector(void)
{
	SetCurrentWorldSector( gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ );
	if( gfTacticalTraversal && gpTacticalTraversalGroup && gpTacticalTraversalChosenSoldier )
	{
		if( gTacticalStatus.fEnemyInSector )
		{
			TacticalCharacterDialogue(gpTacticalTraversalChosenSoldier, QUOTE_ENEMY_PRESENCE);
		}
	}
	gfTacticalTraversal = FALSE;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;
	FadeInGameScreen( );
}


static INT16 PickGridNoToWalkIn(SOLDIERTYPE* pSoldier, UINT8 ubInsertionDirection, UINT32* puiNumAttempts);


static void DoneFadeOutAdjacentSector(void)
{
	UINT8 ubDirection;
	SetCurrentWorldSector( gsAdjacentSectorX, gsAdjacentSectorY, gbAdjacentSectorZ );

	ubDirection = GetStrategicInsertionDataFromAdjacentMoveDirection( gubTacticalDirection, gsAdditionalData );
	if( gfTacticalTraversal && gpTacticalTraversalGroup && gpTacticalTraversalChosenSoldier )
	{
		if( gTacticalStatus.fEnemyInSector )
		{
			TacticalCharacterDialogue(gpTacticalTraversalChosenSoldier, QUOTE_ENEMY_PRESENCE);
		}
	}
	gfTacticalTraversal = FALSE;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;

	if ( gfCaves )
	{
		// ATE; Set tactical status flag...
		gTacticalStatus.uiFlags |= IGNORE_ALL_OBSTACLES;
		// Set pathing flag to path through anything....
		gfPathAroundObstacles = FALSE;
	}

	// OK, give our guys new orders...
	if( gpAdjacentGroup->fPlayer )
	{
		//For player groups, update the soldier information
		UINT32 uiAttempts;
		INT16				sGridNo, sOldGridNo;
		UINT8				ubNum = 0;
		CFOR_ALL_PLAYERS_IN_GROUP(curr, gpAdjacentGroup)
		{
			if (curr->pSoldier->sGridNo != NOWHERE)
			{
				sGridNo = PickGridNoToWalkIn(curr->pSoldier, ubDirection, &uiAttempts);

				//If the search algorithm failed due to too many attempts, simply reset the
				//the gridno as the destination is a reserved gridno and we will place the
				//merc there without walking into the sector.
				if (sGridNo == NOWHERE && uiAttempts == MAX_ATTEMPTS)
				{
					sGridNo = curr->pSoldier->sGridNo;
				}

				if (sGridNo != NOWHERE)
				{
					curr->pSoldier->ubWaitActionToDo = 1;
					// OK, here we have been given a position, a gridno has been given to use as well....
					sOldGridNo = curr->pSoldier->sGridNo;
					EVENT_SetSoldierPosition(curr->pSoldier, sGridNo, SSP_NONE);
					if (sGridNo != sOldGridNo)
					{
						EVENT_GetNewSoldierPath(curr->pSoldier, sOldGridNo, WALKING);
					}
					ubNum++;
				}
			}
			else
			{
#ifdef JA2BETAVERSION
				char str[256];
				sprintf(str, "%ls's gridno is NOWHERE, and is attempting to walk into sector.", curr->pSoldier->name);
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, str);
#endif
			}
		}
		SetActionToDoOnceMercsGetToLocation(WAIT_FOR_MERCS_TO_WALKON_SCREEN, ubNum);
		guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI( );

		// Unset flag here.....
		gfPathAroundObstacles = TRUE;

	}
	FadeInGameScreen( );
}


static BOOLEAN SoldierOKForSectorExit(SOLDIERTYPE* pSoldier, INT8 bExitDirection, UINT16 usAdditionalData)
{
	INT16 sWorldX;
	INT16 sWorldY;

	// if the soldiers gridno is not NOWHERE
	if( pSoldier->sGridNo == NOWHERE )
		return( FALSE );

	// OK, anyone on roofs cannot!
	if ( pSoldier->bLevel > 0 )
		return( FALSE );

	// Get screen coordinates for current position of soldier
	GetAbsoluteScreenXYFromMapPos(pSoldier->sGridNo, &sWorldX, &sWorldY);

	// Check direction
	switch( bExitDirection )
	{
		case EAST_STRATEGIC_MOVE:

			if ( sWorldX < ( ( gsTRX - gsTLX ) - CHECK_DIR_X_DELTA ) )
			{
				// NOT OK, return FALSE
				return( FALSE );
			}
			break;

		case WEST_STRATEGIC_MOVE:

			if ( sWorldX > CHECK_DIR_X_DELTA )
			{
				// NOT OK, return FALSE
				return( FALSE );
			}
			break;

		case SOUTH_STRATEGIC_MOVE:

			if ( sWorldY < ( ( gsBLY - gsTRY ) - CHECK_DIR_Y_DELTA ) )
			{
				// NOT OK, return FALSE
				return( FALSE );
			}
			break;

		case NORTH_STRATEGIC_MOVE:

			if ( sWorldY > CHECK_DIR_Y_DELTA )
			{
				// NOT OK, return FALSE
				return( FALSE );
			}
			break;

			// This case is for an exit grid....
			// check if we are close enough.....

		case -1:


			// FOR REALTIME - DO MOVEMENT BASED ON STANCE!
			if ( ( gTacticalStatus.uiFlags & REALTIME ) || !( gTacticalStatus.uiFlags & INCOMBAT ) )
			{
				pSoldier->usUIMovementMode =  GetMoveStateBasedOnStance( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight );
			}

			const INT16 sGridNo = FindGridNoFromSweetSpotCloseToExitGrid(pSoldier, usAdditionalData, 10);
			if ( sGridNo == NOWHERE )
			{
				return( FALSE );
			}

			// ATE: if we are in combat, get cost to move here....
			if ( gTacticalStatus.uiFlags & INCOMBAT )
			{
				// Turn off at end of function...
				const INT16 sAPs = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
				if ( !EnoughPoints( pSoldier, sAPs, 0, FALSE ) )
				{
					return( FALSE );
				}
			}
			break;

	}
	return( TRUE );
}

//ATE: Returns FALSE if NOBODY is close enough, 1 if ONLY selected guy is and 2 if all on squad are...
BOOLEAN OKForSectorExit( INT8 bExitDirection, UINT16 usAdditionalData, UINT32 *puiTraverseTimeInMinutes )
{
	BOOLEAN		fAtLeastOneMercControllable = FALSE;
	BOOLEAN		fOnlySelectedGuy = FALSE;
	SOLDIERTYPE *pValidSoldier = NULL;
	UINT8			ubReturnVal = FALSE;
	UINT8			ubNumControllableMercs = 0;
	UINT8		  ubNumMercs = 0, ubNumEPCs = 0;
	UINT8     ubPlayerControllableMercsInSquad = 0;

	const SOLDIERTYPE* const sel = GetSelectedMan();
	// must have a selected soldier to be allowed to tactically traverse.
	if (sel == NULL) return FALSE;

	/*
	//Exception code for the two sectors in San Mona that are separated by a cliff.  We want to allow strategic
	//traversal, but NOT tactical traversal.  The only way to tactically go from D4 to D5 (or viceversa) is to enter
	//the cave entrance.
	if( gWorldSectorX == 4 && gWorldSectorY == 4 && !gbWorldSectorZ && bExitDirection == EAST_STRATEGIC_MOVE )
	{
		gfInvalidTraversal = TRUE;
		return FALSE;
	}
	if( gWorldSectorX == 5 && gWorldSectorY == 4 && !gbWorldSectorZ && bExitDirection == WEST_STRATEGIC_MOVE )
	{
		gfInvalidTraversal = TRUE;
		return FALSE;
	}
	*/

	gfInvalidTraversal = FALSE;
	gfLoneEPCAttemptingTraversal = FALSE;
	gubLoneMercAttemptingToAbandonEPCs = 0;
	gPotentiallyAbandonedEPC = NULL;

	// Look through all mercs and check if they are within range of east end....
	FOR_ALL_IN_TEAM(pSoldier, gbPlayerNum)
	{
		// If we are controllable
		if (OkControllableMerc(pSoldier) && pSoldier->bAssignment == CurrentSquad())
		{
			//Need to keep a copy of a good soldier, so we can access it later, and
			//not more than once.
			pValidSoldier = pSoldier;

			ubNumControllableMercs++;

			//We need to keep track of the number of EPCs and mercs in this squad.  If we have
			//only one merc and one or more EPCs, then we can't allow the merc to tactically traverse,
			//if he is the only merc near enough to traverse.
			if( AM_AN_EPC( pSoldier ) )
			{
				ubNumEPCs++;
				//Also record the EPC's slot ID incase we later build a string using the EPC's name.
				gPotentiallyAbandonedEPC = pSoldier;
				if( AM_A_ROBOT( pSoldier ) && !CanRobotBeControlled( pSoldier ) )
				{
					gfRobotWithoutControllerAttemptingTraversal = TRUE;
					ubNumControllableMercs--;
					continue;
				}
			}
			else
			{
				ubNumMercs++;
			}

			if ( SoldierOKForSectorExit( pSoldier, bExitDirection, usAdditionalData ) )
			{
				fAtLeastOneMercControllable++;

				if (pSoldier == sel) fOnlySelectedGuy = TRUE;
			}
			else
			{
				GROUP *pGroup;

				// ATE: Dont's assume exit grids here...
				if ( bExitDirection != -1 )
				{
					//Now, determine if this is a valid path.
					pGroup = GetGroup( pValidSoldier->ubGroupID );
					AssertMsg( pGroup, String( "%ls is not in a valid group (pSoldier->ubGroupID is %d)",pValidSoldier->name, pValidSoldier->ubGroupID) );
					if( !gbWorldSectorZ )
					{
						*puiTraverseTimeInMinutes = GetSectorMvtTimeForGroup( (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ), bExitDirection, pGroup );
					}
					else if( gbWorldSectorZ > 1 )
					{ //We are attempting to traverse in an underground environment.  We need to use a complete different
						//method.  When underground, all sectors are instantly adjacent.
						*puiTraverseTimeInMinutes = UndergroundTacticalTraversalTime( bExitDirection );
					}
					if( *puiTraverseTimeInMinutes == 0xffffffff )
					{
						gfInvalidTraversal = TRUE;
						return FALSE;
					}
				}
				else
				{
					*puiTraverseTimeInMinutes = 0; //exit grid travel is instantaneous
				}
			}
		}
	}

	// If we are here, at least one guy is controllable in this sector, at least he can go!
	if( fAtLeastOneMercControllable )
	{
		ubPlayerControllableMercsInSquad = (UINT8)NumberOfPlayerControllableMercsInSquad(sel->bAssignment);
		if( fAtLeastOneMercControllable <= ubPlayerControllableMercsInSquad )
		{ //if the selected merc is an EPC and we can only leave with that merc, then prevent it
			//as EPCs aren't allowed to leave by themselves.  Instead of restricting this in the
			//exiting sector gui, we restrict it by explaining it with a message box.
			if (AM_AN_EPC(sel))
			{
				if (fAtLeastOneMercControllable < ubPlayerControllableMercsInSquad || fAtLeastOneMercControllable == 1)
				{
					gfLoneEPCAttemptingTraversal = TRUE;
					return FALSE;
				}
			}
			else
			{	//We previously counted the number of EPCs and mercs, and if the selected merc is not an EPC and there are no
				//other mercs in the squad able to escort the EPCs, we will prohibit this merc from tactically traversing.
				if( ubNumEPCs && ubNumMercs == 1 && fAtLeastOneMercControllable < ubPlayerControllableMercsInSquad )
				{
					gubLoneMercAttemptingToAbandonEPCs = ubNumEPCs;
					return FALSE;
				}
			}
		}
		if ( bExitDirection != -1 )
		{
			GROUP *pGroup;
			//Now, determine if this is a valid path.
			pGroup = GetGroup( pValidSoldier->ubGroupID );
			AssertMsg( pGroup, String( "%ls is not in a valid group (pSoldier->ubGroupID is %d)",pValidSoldier->name, pValidSoldier->ubGroupID) );
			if( !gbWorldSectorZ )
			{
				*puiTraverseTimeInMinutes = GetSectorMvtTimeForGroup( (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ), bExitDirection, pGroup );
			}
			else if( gbWorldSectorZ > 0 )
			{ //We are attempting to traverse in an underground environment.  We need to use a complete different
				//method.  When underground, all sectors are instantly adjacent.
				*puiTraverseTimeInMinutes = UndergroundTacticalTraversalTime( bExitDirection );
			}
			if( *puiTraverseTimeInMinutes == 0xffffffff )
			{
				gfInvalidTraversal = TRUE;
				ubReturnVal = FALSE;
			}
			else
			{
				ubReturnVal = TRUE;
			}
		}
		else
		{
			ubReturnVal = TRUE;
			*puiTraverseTimeInMinutes = 0; //exit grid travel is instantaneous
		}
	}

	if ( ubReturnVal )
	{
		// Default to FALSE again, until we see that we have
		ubReturnVal = FALSE;

		if ( fAtLeastOneMercControllable )
		{
			// Do we contain the selected guy?
			if ( fOnlySelectedGuy )
			{
				ubReturnVal = 1;
			}
			// Is the whole squad able to go here?
			if ( fAtLeastOneMercControllable == ubPlayerControllableMercsInSquad )
			{
				ubReturnVal = 2;
			}
		}
	}

	return( ubReturnVal );
}


void SetupNewStrategicGame( )
{
	INT16 sSectorX, sSectorY;

	// Set all sectors as enemy controlled
	for ( sSectorX = 0; sSectorX < MAP_WORLD_X; sSectorX++ )
	{
		for ( sSectorY = 0; sSectorY < MAP_WORLD_Y; sSectorY++ )
		{
			StrategicMap[ CALCULATE_STRATEGIC_INDEX( sSectorX, sSectorY ) ].fEnemyControlled = TRUE;
		}
	}

	//Initialize the game time
	InitNewGameClock();
	//Remove all events
	DeleteAllStrategicEvents();

	//Set up all events that get processed daily...
	//.............................................
	BuildDayLightLevels();
	// Check for quests each morning
	AddEveryDayStrategicEvent( EVENT_CHECKFORQUESTS, QUEST_CHECK_EVENT_TIME, 0 );
	// Some things get updated in the very early morning
	AddEveryDayStrategicEvent( EVENT_DAILY_EARLY_MORNING_EVENTS, EARLY_MORNING_TIME, 0 );
	//Daily Update BobbyRay Inventory
	AddEveryDayStrategicEvent( EVENT_DAILY_UPDATE_BOBBY_RAY_INVENTORY, BOBBYRAY_UPDATE_TIME, 0 );
	//Daily Update of the M.E.R.C. site.
	AddEveryDayStrategicEvent( EVENT_DAILY_UPDATE_OF_MERC_SITE, 0, 0 );
	//Daily update of insured mercs
	AddEveryDayStrategicEvent( EVENT_HANDLE_INSURED_MERCS, INSURANCE_UPDATE_TIME,	0 );
	//Daily update of mercs
	AddEveryDayStrategicEvent( EVENT_MERC_DAILY_UPDATE, 0, 0 );
	// Daily mine production processing events
	AddEveryDayStrategicEvent( EVENT_SETUP_MINE_INCOME, 0, 0 );
	// Daily checks for E-mail from Enrico
	AddEveryDayStrategicEvent( EVENT_ENRICO_MAIL, ENRICO_MAIL_TIME , 0 );

	// Hourly update of all sorts of things
	AddPeriodStrategicEvent( EVENT_HOURLY_UPDATE, 60, 0 );
	AddPeriodStrategicEvent( EVENT_QUARTER_HOUR_UPDATE, 15, 0 );

	#ifdef JA2DEMO
		AddPeriodStrategicEventWithOffset( EVENT_MINUTE_UPDATE, 60, 475, 0 );
	#endif

	//Clear any possible battle locator
	gfBlitBattleSectorLocator = FALSE;




	StrategicTurnsNewGame( );
}


// a -1 will be returned upon failure
INT8 GetSAMIdFromSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	INT8 bCounter = 0;
	INT16 sSectorValue = 0;

	// check if valid sector
	if( bSectorZ != 0 )
	{
		return( -1 );
	}

	// get the sector value
	sSectorValue = SECTOR( sSectorX, sSectorY );

	// run through list of sam sites
	for( bCounter = 0; bCounter < 4; bCounter++ )
	{
		if( pSamList[ bCounter ] == sSectorValue )
		{
			return( bCounter );
		}
	}


	return( -1 );
}


BOOLEAN CanGoToTacticalInSector( INT16 sX, INT16 sY, UINT8 ubZ )
{
	// if not a valid sector
	if( ( sX < 1 ) || ( sX > 16 ) || ( sY < 1 ) || ( sY > 16 ) || ( ubZ > 3 ) )
	{
		return( FALSE );
	}

  // look for all living, fighting mercs on player's team.  Robot and EPCs qualify!
	CFOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		// ARM: now allows loading of sector with all mercs below OKLIFE as long as they're alive
		if (s->bLife != 0 &&
				!(s->uiStatusFlags & SOLDIER_VEHICLE) &&
				s->bAssignment != IN_TRANSIT &&
				s->bAssignment != ASSIGNMENT_POW &&
				s->bAssignment != ASSIGNMENT_DEAD &&
				!SoldierAboardAirborneHeli(s) &&
				!s->fBetweenSectors &&
				s->sSectorX == sX &&
				s->sSectorY == sY &&
				s->bSectorZ == ubZ)
		{
			return TRUE;
		}
	}

	return( FALSE );
}


INT32 GetNumberOfSAMSitesUnderPlayerControl( void )
{
	INT32 iNumber = 0, iCounter = 0;

	// if the sam site is under player control, up the number
	for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
	{
		if (!StrategicMap[SECTOR_INFO_TO_STRATEGIC_INDEX(pSamList[iCounter])].fEnemyControlled)
		{
			iNumber++;
		}
	}

	return( iNumber );
}

INT32 SAMSitesUnderPlayerControl( INT16 sX, INT16 sY )
{
	BOOLEAN fSamSiteUnderControl = FALSE;

	// is this sector a SAM sector?
	if (IsThisSectorASAMSector(sX, sY, 0))
	{
		// is it under control by the player
		if (!StrategicMap[CALCULATE_STRATEGIC_INDEX(sX, sY)].fEnemyControlled)
		{
			// yes
			fSamSiteUnderControl = TRUE;
		}
	}

	return( fSamSiteUnderControl );
}


void UpdateAirspaceControl( void )
{
	INT32 iCounterA = 0, iCounterB = 0;
	UINT8 ubControllingSAM;
	StrategicMapElement *pSAMStrategicMap = NULL;
	BOOLEAN fEnemyControlsAir;


	for( iCounterA = 1; iCounterA < ( INT32 )( MAP_WORLD_X - 1 ); iCounterA++ )
	{
		for( iCounterB = 1; iCounterB < ( INT32 )( MAP_WORLD_Y - 1 ); iCounterB++ )
		{
			// IMPORTANT: B and A are reverse here, since the table is stored transposed
			ubControllingSAM = ubSAMControlledSectors[ iCounterB ][ iCounterA ];

			if ( ( ubControllingSAM >= 1 ) && ( ubControllingSAM <= NUMBER_OF_SAMS ) )
			{
				pSAMStrategicMap = &( StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( pSamList[ ubControllingSAM - 1 ] ) ] );

				// if the enemies own the controlling SAM site, and it's in working condition
				if( ( pSAMStrategicMap->fEnemyControlled ) && ( pSAMStrategicMap->bSAMCondition >= MIN_CONDITION_FOR_SAM_SITE_TO_WORK ) )
				{
					fEnemyControlsAir = TRUE;
				}
				else
				{
					fEnemyControlsAir = FALSE;
				}
			}
			else
			{
				// no controlling SAM site
				fEnemyControlsAir = FALSE;
			}

			StrategicMap[ CALCULATE_STRATEGIC_INDEX( iCounterA, iCounterB ) ].fEnemyAirControlled = fEnemyControlsAir;
		}
	}


	// check if currently selected arrival sector still has secure airspace

	// if it's not enemy air controlled
	if (StrategicMap[CALCULATE_STRATEGIC_INDEX(gsMercArriveSectorX, gsMercArriveSectorY)].fEnemyAirControlled)
	{
		// NOPE!
		CHAR16 sMsgString[ 256 ], sMsgSubString1[ 64 ], sMsgSubString2[ 64 ];

		// get the name of the old sector
		GetSectorIDString( gsMercArriveSectorX, gsMercArriveSectorY, 0, sMsgSubString1, lengthof(sMsgSubString1), FALSE );

		// move the landing zone over to Omerta
		gsMercArriveSectorX = 9;
		gsMercArriveSectorY = 1;

		// get the name of the new sector
		GetSectorIDString( gsMercArriveSectorX, gsMercArriveSectorY, 0, sMsgSubString2, lengthof(sMsgSubString2), FALSE );

		// now build the string
		swprintf( sMsgString, lengthof(sMsgString), pBullseyeStrings[ 4 ], sMsgSubString1, sMsgSubString2 );

		// confirm the change with overlay message
		DoScreenIndependantMessageBox( sMsgString, MSG_BOX_FLAG_OK, NULL );

		// update position of bullseye
		fMapPanelDirty = TRUE;

		// update destination column for any mercs in transit
		fTeamPanelDirty = TRUE;
	}


	// ARM: airspace control now affects refueling site availability, so update that too with every change!
	UpdateRefuelSiteAvailability( );
}


BOOLEAN IsThereAFunctionalSAMSiteInSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	if (!IsThisSectorASAMSector(sSectorX, sSectorY, bSectorZ)) return FALSE;

	if( StrategicMap[ CALCULATE_STRATEGIC_INDEX( sSectorX, sSectorY ) ].bSAMCondition < MIN_CONDITION_FOR_SAM_SITE_TO_WORK )
	{
		return( FALSE );
	}

	return( TRUE );
}

BOOLEAN IsThisSectorASAMSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{

	// is the sector above ground?
	if( bSectorZ != 0 )
	{
		return( FALSE );
	}

	if( ( SAM_1_X == sSectorX ) && ( SAM_1_Y == sSectorY ) )
	{
		return( TRUE );
	}
	else 	if( ( SAM_2_X == sSectorX ) && ( SAM_2_Y == sSectorY ) )
	{
		return( TRUE );
	}
	else 	if( ( SAM_3_X == sSectorX ) && ( SAM_3_Y == sSectorY ) )
	{
		return( TRUE );
	}
	else 	if( ( SAM_4_X == sSectorX ) && ( SAM_4_Y == sSectorY ) )
	{
		return( TRUE );
	}

	return ( FALSE );
}


// is this sector part of the town?
static BOOLEAN SectorIsPartOfTown(INT8 bTownId, INT16 sSectorX, INT16 sSectorY)
{
	if( StrategicMap[CALCULATE_STRATEGIC_INDEX( sSectorX, sSectorY ) ].bNameId == bTownId )
	{
		// is in the town
		return ( TRUE );
	}

	// not in the town
	return ( FALSE );
}


void SaveStrategicInfoToSavedFile(HWFILE const hFile)
{
	// Save the strategic map information
	for (const StrategicMapElement* i = StrategicMap; i != endof(StrategicMap); ++i)
	{
		InjectStrategicMapElementIntoFile(hFile, i);
	}

	// Save the Sector Info
	for (const SECTORINFO* i = SectorInfo; i != endof(SectorInfo); ++i)
	{
		InjectSectorInfoIntoFile(hFile, i);
	}

	// Save the SAM Controlled Sector Information
	const UINT32 uiSize = MAP_WORLD_X * MAP_WORLD_Y;
/*
	FileWrite(hFile, ubSAMControlledSectors, uiSize);
*/
	FileSeek( hFile, uiSize, FILE_SEEK_FROM_CURRENT );

	// Save the fFoundOrta
	FileWrite(hFile, &fFoundOrta, sizeof(BOOLEAN));
}


void LoadStrategicInfoFromSavedFile(HWFILE const hFile)
{
	// Load the strategic map information
	for (StrategicMapElement* i = StrategicMap; i != endof(StrategicMap); ++i)
	{
		ExtractStrategicMapElementFromFile(hFile, i);
	}

	// Load the Sector Info
	for (SECTORINFO* i = SectorInfo; i != endof(SectorInfo); ++i)
	{
		ExtractSectorInfoFromFile(hFile, i);
	}

	// Load the SAM Controlled Sector Information
	const UINT32 uiSize = MAP_WORLD_X * MAP_WORLD_Y;
/*
	FileRead(hFile, ubSAMControlledSectors, uiSize);
*/
	FileSeek( hFile, uiSize, FILE_SEEK_FROM_CURRENT );

	// Load the fFoundOrta
	FileRead(hFile, &fFoundOrta, sizeof(BOOLEAN));
}


static INT16 PickGridNoNearestEdge(SOLDIERTYPE* pSoldier, UINT8 ubTacticalDirection)
{
	INT16			sGridNo, sStartGridNo, sOldGridNo;
	INT8			bOdd = 1, bOdd2 = 1;
	UINT8			bAdjustedDist = 0;
	UINT32		cnt;

	switch( ubTacticalDirection )
	{

		case EAST:

			sGridNo				 = pSoldier->sGridNo;
			sStartGridNo	 = pSoldier->sGridNo;
			sOldGridNo     = pSoldier->sGridNo;

			// Move directly to the right!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo -= WORLD_COLS;
				}
				else
				{
					sGridNo++;
				}

				bOdd = (INT8)!bOdd;
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate up/down )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS - 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS + 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			} while( TRUE );

		case WEST:

			sGridNo				 = pSoldier->sGridNo;
			sStartGridNo	 = pSoldier->sGridNo;
			sOldGridNo     = pSoldier->sGridNo;

			// Move directly to the left!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo += WORLD_COLS;
				}
				else
				{
					sGridNo--;
				}

				bOdd = (INT8)!bOdd;
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate up/down )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS - 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS + 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			} while( TRUE );

		case NORTH:

			sGridNo				 = pSoldier->sGridNo;
			sStartGridNo	 = pSoldier->sGridNo;
			sOldGridNo     = pSoldier->sGridNo;

			// Move directly to the left!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo -= WORLD_COLS;
				}
				else
				{
					sGridNo--;
				}

				bOdd = (INT8)(!bOdd);
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate left/right )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS - 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS + 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			} while( TRUE );

		case SOUTH:

			sGridNo				 = pSoldier->sGridNo;
			sStartGridNo	 = pSoldier->sGridNo;
			sOldGridNo     = pSoldier->sGridNo;

			// Move directly to the left!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo += WORLD_COLS;
				}
				else
				{
					sGridNo++;
				}

				bOdd = (INT8)(!bOdd);
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate left/right )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS - 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS + 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			} while( TRUE );
	}

	return( NOWHERE );
}


void AdjustSoldierPathToGoOffEdge( SOLDIERTYPE *pSoldier, INT16 sEndGridNo, UINT8 ubTacticalDirection )
{
	INT16 sNewGridNo, sTempGridNo;
	INT32	iLoop;

	// will this path segment actually take us to our desired destination in the first place?
	if (pSoldier->usPathDataSize + 2 > MAX_PATH_LIST_SIZE)
	{

		sTempGridNo = pSoldier->sGridNo;

		for (iLoop = 0; iLoop < pSoldier->usPathDataSize; iLoop++)
		{
			sTempGridNo += (INT16)DirectionInc( pSoldier->usPathingData[ iLoop ] );
		}

		if (sTempGridNo == sEndGridNo)
		{
			// we can make it, but there isn't enough path room for the two steps required.
			// truncate our path so there's guaranteed the merc will have to generate another
			// path later on...
			pSoldier->usPathDataSize -= 4;
			return;
		}
		else
		{
			// can't even make it there with these 30 tiles of path, abort...
			return;
		}
	}

	switch( ubTacticalDirection )
	{
		case EAST:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, (UINT16)DirectionInc( (UINT8)NORTHEAST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = NORTHEAST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, (UINT16)DirectionInc( (UINT8)NORTHEAST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = NORTHEAST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			break;

		case WEST:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, (UINT16)DirectionInc( (UINT8)SOUTHWEST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = SOUTHWEST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, (UINT16)DirectionInc( (UINT8)SOUTHWEST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = SOUTHWEST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;
			break;

		case NORTH:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, (UINT16)DirectionInc( (UINT8)NORTHWEST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = NORTHWEST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, (UINT16)DirectionInc( (UINT8)NORTHWEST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = NORTHWEST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			break;

		case SOUTH:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, (UINT16)DirectionInc( (UINT8)SOUTHEAST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = SOUTHEAST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, (UINT16)DirectionInc( (UINT8)SOUTHEAST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->usPathingData[ pSoldier->usPathDataSize ] = SOUTHEAST;
			pSoldier->usPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;
			break;

	}
}


static INT16 PickGridNoToWalkIn(SOLDIERTYPE* pSoldier, UINT8 ubInsertionDirection, UINT32* puiNumAttempts)
{
	INT16			sGridNo, sStartGridNo, sOldGridNo;
	INT8			bOdd = 1, bOdd2 = 1;
	UINT8			bAdjustedDist = 0;
	UINT32		cnt;

	*puiNumAttempts = 0;

	switch( ubInsertionDirection )
	{
		// OK, we're given a direction on visible map, let's look for the first oone
		// we find that is just on the start of visible map...
		case INSERTION_CODE_WEST:

			sGridNo				 = (INT16)pSoldier->sGridNo;
			sStartGridNo	 = (INT16)pSoldier->sGridNo;
			sOldGridNo     = (INT16)pSoldier->sGridNo;

			// Move directly to the left!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo += WORLD_COLS;
				}
				else
				{
					sGridNo--;
				}

				bOdd = (INT8)(!bOdd);
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate up/down )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS - 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS + 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			}
			return NOWHERE;

		case INSERTION_CODE_EAST:

			sGridNo				 = (INT16)pSoldier->sGridNo;
			sStartGridNo	 = (INT16)pSoldier->sGridNo;
			sOldGridNo     = (INT16)pSoldier->sGridNo;

			// Move directly to the right!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo -= WORLD_COLS;
				}
				else
				{
					sGridNo++;
				}

				bOdd = (INT8)(!bOdd);
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate up/down )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS - 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS + 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			}
			return NOWHERE;

		case INSERTION_CODE_NORTH:

			sGridNo				 = (INT16)pSoldier->sGridNo;
			sStartGridNo	 = (INT16)pSoldier->sGridNo;
			sOldGridNo     = (INT16)pSoldier->sGridNo;

			// Move directly to the up!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo -= WORLD_COLS;
				}
				else
				{
					sGridNo--;
				}

				bOdd = (INT8)(!bOdd);
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate left/right )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS + 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS - 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			}
			return NOWHERE;

		case INSERTION_CODE_SOUTH:

			sGridNo				 = (INT16)pSoldier->sGridNo;
			sStartGridNo	 = (INT16)pSoldier->sGridNo;
			sOldGridNo     = (INT16)pSoldier->sGridNo;

			// Move directly to the down!
			while( GridNoOnVisibleWorldTile( sGridNo ) )
			{
				sOldGridNo = sGridNo;

				if ( bOdd )
				{
					sGridNo += WORLD_COLS;
				}
				else
				{
					sGridNo++;
				}

				bOdd = (INT8)(!bOdd);
			}

			sGridNo			 = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
				{
					return( sGridNo );
				}

				// If here, try another place!
				// ( alternate left/right )
				if ( bOdd2 )
				{
					bAdjustedDist++;

					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo - WORLD_COLS + 1);
					}
				}
				else
				{
					sGridNo = sStartGridNo;

					for ( cnt = 0; cnt < bAdjustedDist; cnt++ )
					{
						sGridNo = (INT16)(sGridNo + WORLD_COLS - 1);
					}
				}

				bOdd2 = (INT8)(!bOdd2);

			}
			return NOWHERE;

	}

	//Unhandled exit
	*puiNumAttempts = 0;

	return( NOWHERE );
}


//NEW!
//Calculates the name of the sector based on the loaded sector values.
//Examples:		A9
//						A10_B1
//						J9_B2_A ( >= BETAVERSION ) else J9_B2 (release equivalent)
static void GetLoadedSectorString(wchar_t* const pString, const size_t Length)
{
	if (!gfWorldLoaded)
	{
		swprintf(pString, Length, L"");
	}
	else if (gbWorldSectorZ == 0)
	{
		swprintf(pString, Length, L"%c%d", gWorldSectorY + 'A' - 1, gWorldSectorX);
	}
	else
	{
		swprintf(pString, Length, L"%c%d_b%d", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ);
	}
}


void HandleSlayDailyEvent( void )
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileIDOnPlayerTeam(SLAY);
	if( pSoldier == NULL )
	{
		return;
	}

	// valid soldier?
	if (pSoldier->bLife == 0 || pSoldier->bAssignment == IN_TRANSIT || pSoldier->bAssignment == ASSIGNMENT_POW)
	{
		// no
		return;
	}

	// ATE: This function is used to check for the ultimate last day SLAY can stay for
	// he may decide to leave randomly while asleep...
	//if the user hasnt renewed yet, and is still leaving today
	if( ( pSoldier->iEndofContractTime /1440 ) <= (INT32)GetWorldDay( ) )
	{
		pSoldier->ubLeaveHistoryCode = HISTORY_SLAY_MYSTERIOUSLY_LEFT;
		MakeCharacterDialogueEventContractEndingNoAskEquip(*pSoldier);
	}
}



BOOLEAN IsSectorDesert( INT16 sSectorX, INT16 sSectorY )
{
	if ( SectorInfo[ SECTOR(sSectorX, sSectorY) ].ubTraversability[ THROUGH_STRATEGIC_MOVE ] == SAND )
	{
		// desert
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static void HandleDefiniteUnloadingOfWorld(UINT8 const ubUnloadCode)
{
	// clear tactical queue
	ClearEventQueue();

	// ATE: End all bullets....
	DeleteAllBullets();

	// End all physics objects...
	RemoveAllPhysicsObjects();

	RemoveAllActiveTimedBombs();

	// handle any quest stuff here so world items can be affected
	HandleQuestCodeOnSectorExit( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//if we arent loading a saved game
	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		//Clear any potential battle flags.  They will be set if necessary.
		gTacticalStatus.fEnemyInSector = FALSE;
		gTacticalStatus.uiFlags &= ~INCOMBAT;
	}

	if ( ubUnloadCode == ABOUT_TO_LOAD_NEW_MAP )
	{
		//if we arent loading a saved game
		if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
		{

			// Save the current sectors Item list to a temporary file, if its not the first time in
			SaveCurrentSectorsInformationToTempItemFile();

			// Update any mercs currently in sector, their profile info...
			UpdateSoldierPointerDataIntoProfile( FALSE );
		}
	}
	else if( ubUnloadCode == ABOUT_TO_TRASH_WORLD )
	{
		//Save the current sectors open temp files to the disk
		SaveCurrentSectorsInformationToTempItemFile();

		//Setup the tactical existance of the current soldier.
		//@@@Evaluate
		SetupProfileInsertionDataForCivilians();

		gfBlitBattleSectorLocator = FALSE;
	}

	//Handle cases for both types of unloading
	HandleMilitiaStatusInCurrentMapBeforeLoadingNewMap();
}


BOOLEAN HandlePotentialBringUpAutoresolveToFinishBattle( )
{
	INT32 i;

	//We don't have mercs in the sector.  Now, we check to see if there are BOTH enemies and militia.  If both
	//co-exist in the sector, then make them fight for control of the sector via autoresolve.
	for( i = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; i <= gTacticalStatus.Team[ CREATURE_TEAM ].bLastID; i++ )
	{
		const SOLDIERTYPE* const creature = GetMan(i);
		if (creature->bActive &&
				creature->bLife != 0 &&
				creature->sSectorX == gWorldSectorX &&
				creature->sSectorY == gWorldSectorY &&
				creature->bSectorZ == gbWorldSectorZ)
		{ //We have enemies, now look for militia!
			for( i = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID; i <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; i++ )
			{
				const SOLDIERTYPE* const milita = GetMan(i);
				if (milita->bActive &&
						milita->bLife != 0 &&
						milita->bSide    == OUR_TEAM &&
						milita->sSectorX == gWorldSectorX &&
						milita->sSectorY == gWorldSectorY &&
						milita->bSectorZ == gbWorldSectorZ)
				{ //We have militia and enemies and no mercs!  Let's finish this battle in autoresolve.
					gfEnteringMapScreen = TRUE;
					gfEnteringMapScreenToEnterPreBattleInterface = TRUE;
					gfAutomaticallyStartAutoResolve = TRUE;
					gfUsePersistantPBI = FALSE;
					gubPBSectorX = (UINT8)gWorldSectorX;
					gubPBSectorY = (UINT8)gWorldSectorY;
					gubPBSectorZ = (UINT8)gbWorldSectorZ;
					gfBlitBattleSectorLocator = TRUE;
					gfTransferTacticalOppositionToAutoResolve = TRUE;
					if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
					{
						gubEnemyEncounterCode = ENEMY_INVASION_CODE; //has to be, if militia are here.
					}
					else
					{
						//DoScreenIndependantMessageBox( gzLateLocalizedString[ 39 ], MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
					}

					return( TRUE );
				}
			}
		}
  }

  return( FALSE );
}


BOOLEAN CheckAndHandleUnloadingOfCurrentWorld()
try
{
	INT16 sBattleSectorX, sBattleSectorY, sBattleSectorZ;

	//Don't bother checking this if we don't have a world loaded.
	if( !gfWorldLoaded )
	{
		return FALSE;
	}

	if (DidGameJustStart() && gWorldSectorX == 9 && gWorldSectorY == 1 && !gbWorldSectorZ)
	{
		return FALSE;
	}

	GetCurrentBattleSectorXYZ( &sBattleSectorX, &sBattleSectorY, &sBattleSectorZ );

	if( guiCurrentScreen == AUTORESOLVE_SCREEN )
	{ //The user has decided to let the game autoresolve the current battle.
		if( gWorldSectorX == sBattleSectorX && gWorldSectorY == sBattleSectorY && gbWorldSectorZ == sBattleSectorZ )
		{
			FOR_ALL_IN_TEAM(s, OUR_TEAM)
			{ //If we have a live and valid soldier
				if (s->bLife != 0 &&
						!s->fBetweenSectors &&
						!(s->uiStatusFlags & SOLDIER_VEHICLE) &&
						!AM_A_ROBOT(s) &&
						!AM_AN_EPC(s) &&
						s->sSectorX == gWorldSectorX &&
						s->sSectorY == gWorldSectorY &&
						s->bSectorZ == gbWorldSectorZ)
				{
					RemoveSoldierFromGridNo(s);
					InitSoldierOppList(s);
				}
			}
		}
	}
	else
	{	//Check and see if we have any live mercs in the sector.
		CFOR_ALL_IN_TEAM(s, OUR_TEAM)
		{ //If we have a live and valid soldier
			if (s->bLife != 0 &&
					!s->fBetweenSectors &&
					!(s->uiStatusFlags & SOLDIER_VEHICLE) &&
					!AM_A_ROBOT(s) &&
					!AM_AN_EPC(s) &&
					s->sSectorX == gWorldSectorX &&
					s->sSectorY == gWorldSectorY &&
					s->bSectorZ == gbWorldSectorZ)
			{
				return FALSE;
			}
		}
		//KM : August 6, 1999 Patch fix
		//     Added logic to prevent a crash when player mercs would retreat from a battle involving militia and enemies.
		//		 Without the return here, it would proceed to trash the world, and then when autoresolve would come up to
		//     finish the tactical battle, it would fail to find the existing soldier information (because it was trashed).
    if( HandlePotentialBringUpAutoresolveToFinishBattle( ) )
		{
			return FALSE;
		}
		//end

		//HandlePotentialBringUpAutoresolveToFinishBattle( ); //prior patch logic
	}


	CheckForEndOfCombatMode( FALSE );
	EndTacticalBattleForEnemy();

	// ATE: Change cursor to wait cursor for duration of frame.....
	// save old cursor ID....
	SetCurrentCursorFromDatabase( CURSOR_WAIT_NODELAY );
	RefreshScreen();

	// JA2Gold: Leaving sector, so get rid of ambients!
	DeleteAllAmbients();

	if( guiCurrentScreen == GAME_SCREEN )
	{
		if( !gfTacticalTraversal )
		{ //if we are in tactical and don't intend on going to another sector immediately, then
			gfEnteringMapScreen = TRUE;
		}
		else
		{ //The trashing of the world will be handled automatically.
			return FALSE;
		}
	}

	//We have passed all the checks and can Trash the world.
	HandleDefiniteUnloadingOfWorld(ABOUT_TO_TRASH_WORLD);

	if( guiCurrentScreen == AUTORESOLVE_SCREEN )
	{
		if( gWorldSectorX == sBattleSectorX && gWorldSectorY == sBattleSectorY && gbWorldSectorZ == sBattleSectorZ )
		{
			/* Yes, this is and looks like a hack.  The conditions of this if
			 * statement doesn't work inside TrashWorld() or more specifically,
			 * TacticalRemoveSoldier() from within TrashWorld().  Because we are in
			 * the autoresolve screen, soldiers are internally created different (from
			 * pointers instead of Menptr[]).  It keys on the fact that we are in the
			 * autoresolve screen.  So, by switching the screen, it'll delete the
			 * soldiers in the loaded world properly, then later on, once autoresolve
			 * is complete, it'll delete the autoresolve soldiers properly.  As you
			 * can now see, the above if conditions don't change throughout this whole
			 * process which makes it necessary to do it this way. */
			guiCurrentScreen = MAP_SCREEN;
			TrashWorld();
			guiCurrentScreen = AUTORESOLVE_SCREEN;
		}
	}
	else
	{
		TrashWorld();
	}

	//Clear all combat related flags.
	gTacticalStatus.fEnemyInSector = FALSE;
	gTacticalStatus.uiFlags &= ~INCOMBAT;
  EndTopMessage( );


	//Clear the world sector values.
	gWorldSectorX = gWorldSectorY = 0;
	gbWorldSectorZ = -1;

	//Clear the flags regarding.
	gfCaves = FALSE;
	gfBasement = FALSE;

	return TRUE;
}
catch (...) { return FALSE; }


/* This is called just before the world is unloaded to preserve location
 * information for RPCs and NPCs either in the sector or strategically in the
 * sector (such as firing an NPC in a sector that isn't yet loaded.)  When
 * loading that sector, the RPC would be added. */
//@@@Evaluate
void SetupProfileInsertionDataForSoldier(const SOLDIERTYPE* const s)
{
	if (s->ubProfile == NO_PROFILE) return;
	MERCPROFILESTRUCT& p = GetProfile(s->ubProfile);

	// can't be changed?
	if (p.ubMiscFlags3 & PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE) return;

	if (gfWorldLoaded && s->bActive && s->bInSector)
	{
		// This soldier is currently in the sector

		//@@@Evaluate -- insert code here
		//SAMPLE CODE:  There are multiple situations that I didn't code.  The gridno should be the final destination
		//or reset???

		if (s->ubQuoteRecord && s->ubQuoteActionID)
		{
			// if moving to traverse
			if (s->ubQuoteActionID >= QUOTE_ACTION_ID_TRAVERSE_EAST && s->ubQuoteActionID <= QUOTE_ACTION_ID_TRAVERSE_NORTH)
			{
				// Handle traversal.  This NPC's sector will NOT already be set correctly, so we have to call for that too
				HandleNPCChangesForTacticalTraversal(s);
				p.fUseProfileInsertionInfo = FALSE;
				if (s->ubProfile != NO_PROFILE && NPCHasUnusedRecordWithGivenApproach(s->ubProfile, APPROACH_DONE_TRAVERSAL))
				{
					p.ubMiscFlags3 |= PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL;
				}
			}
			else
			{
				if (s->sFinalDestination == s->sGridNo)
				{
					p.usStrategicInsertionData = s->sGridNo;
				}
				else if (s->sAbsoluteFinalDestination != NOWHERE)
				{
					p.usStrategicInsertionData = s->sAbsoluteFinalDestination;
				}
				else
				{
					p.usStrategicInsertionData = s->sFinalDestination;
				}

				p.fUseProfileInsertionInfo = TRUE;
				p.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				p.ubQuoteActionID          = s->ubQuoteActionID;
				p.ubQuoteRecord            = s->ubQuoteActionID;
			}
		}
		else
		{
			p.fUseProfileInsertionInfo = FALSE;
		}
	}
	else
	{
		//use strategic information
		/* It appears to set the soldier's strategic insertion code everytime a
		 * group arrives in a new sector.  The insertion data isn't needed for these
		 * cases as the code is a direction only. */
		p.ubStrategicInsertionCode = s->ubStrategicInsertionCode;
		p.usStrategicInsertionData = 0;

		//Strategic system should now work.
		p.fUseProfileInsertionInfo = TRUE;
	}
}


static void HandlePotentialMoraleHitForSkimmingSectors(GROUP* pGroup)
{
  if ( !gTacticalStatus.fHasEnteredCombatModeSinceEntering && gTacticalStatus.fEnemyInSector )
  {
		//Flag is set so if "wilderness" enemies are in the adjacent sector of this group, the group has
		//a 90% chance of ambush.  Because this typically doesn't happen very often, the chance is high.
		//This reflects the enemies radioing ahead to other enemies of the group's arrival, so they have
		//time to setup a good ambush!
		pGroup->uiFlags |= GROUPFLAG_HIGH_POTENTIAL_FOR_AMBUSH;

		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
	  {
      // Do morale hit...
      // CC look here!
      // pPlayer->pSoldier
	  }
  }
}
