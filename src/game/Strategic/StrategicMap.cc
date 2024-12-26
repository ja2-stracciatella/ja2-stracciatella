#include "StrategicMap.h"

#include "AI.h"
#include "Ambient_Control.h"
#include "Animated_ProgressBar.h"
#include "Animation_Control.h"
#include "Assignments.h"
#include "Auto_Resolve.h"
#include "Boxing.h"
#include "Bullets.h"
#include "Campaign_Types.h"
#include "ContentManager.h"
#include "ContentMusic.h"
#include "Creature_Spreading.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Enemy_Soldier_Save.h"
#include "Environment.h"
#include "Event_Pump.h"
#include "Exit_Grids.h"
#include "Explosion_Control.h"
#include "Faces.h"
#include "Fade_Screen.h"
#include "GameLoop.h"
#include "GameInstance.h"
#include "GamePolicy.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "HImage.h"
#include "Handle_UI.h"
#include "History.h"
#include "Interface.h"
#include "Interface_Dialogue.h"
#include "Interface_Panels.h"
#include "Isometric_Utils.h"
#include "Items.h"
#include "JAScreens.h"
#include "Keys.h"
#include "LoadSaveSectorInfo.h"
#include "LoadSaveStrategicMapElement.h"
#include "Loading_Screen.h"
#include "Logger.h"
#include "MapScreen.h"
#include "Map_Edgepoints.h"
#include "Map_Information.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface_Map.h"
#include "Meanwhile.h"
#include "Merc_Contract.h"
#include "Merc_Entering.h"
#include "Merc_Hiring.h"
#include "MessageBoxScreen.h"
#include "Militia_Control.h"
#include "MineModel.h"
#include "Music_Control.h"
#include "NPC.h"
#include "OppList.h"
#include "Overhead.h"
#include "PathAI.h"
#include "Physics.h"
#include "Points.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Radar_Screen.h"
#include "Random.h"
#include "RenderWorld.h"
#include "SAM_Sites.h"
#include "SamSiteModel.h"
#include "SaveLoadMap.h"
#include "Scheduling.h"
#include "ScreenIDs.h"
#include "Soldier_Add.h"
#include "Soldier_Control.h"
#include "Soldier_Init_List.h"
#include "Soldier_Macros.h"
#include "Sound_Control.h"
#include "Squads.h"
#include "Strategic.h"
#include "StrategicMap_Secrets.h"
#include "StrategicMapSecretModel.h"
#include "Strategic_Event_Handler.h"
#include "Strategic_Mines.h"
#include "Strategic_Movement.h"
#include "Strategic_Pathing.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Turns.h"
#include "SysUtil.h"
#include "Sys_Globals.h"
#include "Tactical_Placement_GUI.h"
#include "Tactical_Save.h"
#include "Tactical_Turns.h"
#include "Text.h"
#include "Timer.h"
#include "TownModel.h"
#include "Town_Militia.h"
#include "Types.h"
#include "UILayout.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WorldDat.h"
#include "WorldDef.h"
#include "WorldMan.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>
#include <map>
#include <stdexcept>

//Used by PickGridNoToWalkIn
#define MAX_ATTEMPTS 200

#define QUEST_CHECK_EVENT_TIME ( 8 * 60 )
#define BOBBYRAY_UPDATE_TIME   ( 9 * 60 )
#define INSURANCE_UPDATE_TIME  0
#define EARLY_MORNING_TIME     ( 4 * 60 )
#define ENRICO_MAIL_TIME       ( 7 * 60 )


extern INT16 gsRobotGridNo;

BOOLEAN gfGettingNameFromSaveLoadScreen;

SGPSector gWorldSector(0, 0, -1);

static SGPSector gsAdjacentSector;
static GROUP* gpAdjacentGroup = 0;
static UINT8  gubAdjacentJumpCode;
static UINT32 guiAdjacentTraverseTime;
UINT8			gubTacticalDirection;
static INT16  gsAdditionalData;

static BOOLEAN fUsingEdgePointsForStrategicEntry = false;
BOOLEAN		gfInvalidTraversal = false;
BOOLEAN		gfLoneEPCAttemptingTraversal = false;
BOOLEAN		gfRobotWithoutControllerAttemptingTraversal = false;
BOOLEAN   gubLoneMercAttemptingToAbandonEPCs = 0;
const SOLDIERTYPE* gPotentiallyAbandonedEPC = NULL;

INT8 gbGreenToElitePromotions = 0;
INT8 gbGreenToRegPromotions = 0;
INT8 gbRegToElitePromotions = 0;
INT8 gbMilitiaPromotions = 0;


BOOLEAN		gfUseAlternateMap = false;

static const SGPSector SectorIncrementer[8] =
{
	{ 0, -1 }, // N
	{ 1, -1 }, // NE
	{ 1, 0 },  // E
	{ 1, 1 },  // SE
	{ 0, 1 },  // S
	{ -1, 1 }, // SW
	{ -1, 0 }, // W
	{ -1, -1 } // NW
};

Observable<> BeforePrepareSector = {};

extern BOOLEAN gfOverrideSector;


StrategicMapElement StrategicMap[MAP_WORLD_X * MAP_WORLD_Y];


//temp timer stuff -- to measure the time it takes to load a map.

static UINT32 UndergroundTacticalTraversalTime(INT8 const exit_direction)
{ /* We are attempting to traverse in an underground environment. We need to use
	 * a complete different method.  When underground, all sectors are instantly
	 * adjacent. */
	GridNo gridno;
	switch (exit_direction)
	{
		case NORTH_STRATEGIC_MOVE: gridno = gMapInformation.sNorthGridNo; break;
		case EAST_STRATEGIC_MOVE:  gridno = gMapInformation.sEastGridNo;  break;
		case SOUTH_STRATEGIC_MOVE: gridno = gMapInformation.sSouthGridNo; break;
		case WEST_STRATEGIC_MOVE:  gridno = gMapInformation.sWestGridNo;  break;
		default: throw std::logic_error("invalid exit direction");
	}
	return gridno != -1 ? 0 : TRAVERSE_TIME_IMPOSSIBLE;
}


void BeginLoadScreen( )
{
	UINT32 uiStartTime, uiCurrTime;
	INT32 iPercentage, iFactor;
	UINT32 uiTimeRange;

	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	if( guiCurrentScreen == MAP_SCREEN && !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME) && !AreInMeanwhile() )
	{
		SGPBox const DstRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		uiTimeRange = 2000;
		iPercentage = 0;
		uiStartTime = GetClock();
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
		PlayJA2SampleFromFile(SOUNDSDIR "/final psionic blast 01 (16-44).wav", HIGHVOLUME, 1, MIDDLEPAN);
		while( iPercentage < 100  )
		{
			uiCurrTime = GetClock();
			iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
			iPercentage = std::min(iPercentage, 100);

			//Factor the percentage so that it is modified by a gravity falling acceleration effect.
			iFactor = (iPercentage - 50) * 2;
			if( iPercentage < 50 )
				iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
			else
				iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

			if( iPercentage > 50 )
			{
				guiSAVEBUFFER->ShadowRectUsingLowPercentTable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			}

			SGPBox const SrcRect =
			{
				(UINT16) (536 * iPercentage / 100),
				(UINT16) (367 * iPercentage / 100),
				(UINT16) (SCREEN_WIDTH  - 541 * iPercentage / 100),
				(UINT16) (SCREEN_HEIGHT - 406 * iPercentage / 100)
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
		UINT8 const id = gTacticalStatus.uiFlags & LOADING_SAVED_GAME ?
			gubLastLoadingScreenID :
			GetLoadScreenID(gWorldSector);
		DisplayLoadScreenWithID(id);
	}
}

static void InitializeMapStructure(void);

static void HandleAirspaceControlUpdated();

void InitStrategicEngine()
{
	// this runs every time we start the application, so don't put anything in here that's only supposed to run when a new
	// *game* is started!  Those belong in InitStrategicLayer() instead.

	InitializeMapStructure();

	// set up town stuff
	BuildListOfTownSectors( );

	OnAirspaceControlUpdated.addListener("default", HandleAirspaceControlUpdated);
}


UINT8 GetTownIdForSector(const SGPSector& sSector)
{
	return StrategicMap[sSector.AsStrategicIndex()].bNameId;
}

UINT8 GetTownSectorSize(INT8 const town_id)
{
	UINT8 n = 0;
	FOR_EACH_SECTOR_IN_TOWN(i, town_id) ++n;
	return n;
}


UINT8 GetTownSectorsUnderControl(INT8 const town_id)
{
	UINT8 n = 0;
	FOR_EACH_SECTOR_IN_TOWN(i, town_id)
	{
		SGPSector sSector(i->sector);
		if (StrategicMap[sSector.AsStrategicIndex()].fEnemyControlled) continue;
		if (NumEnemiesInSector(sSector) != 0)                          continue;
		++n;
	}
	return n;
}


static void InitializeStrategicMapSectorTownNames(void);


static void InitializeMapStructure(void)
{
	std::fill(std::begin(StrategicMap), std::end(StrategicMap), StrategicMapElement{});

	InitializeStrategicMapSectorTownNames( );
}

ST::string GetMapFileName(const SGPSector& sector, BOOLEAN const add_alternate_map_letter)
{
	ST::string fileName;
	if (sector.z == 0)
	{
		fileName = sector.AsShortString();
	}
	else
	{
		fileName = sector.AsLongString(true);
	}

	/* The gfUseAlternateMap flag is set while loading saved games. When starting
	 * a new game the underground sector info has not been initialized, so we need
	 * the flag to load an alternate sector. */
	if (GetSectorFlagStatus(sector, SF_USE_ALTERNATE_MAP) || gfUseAlternateMap)
	{
		gfUseAlternateMap = false;
		if (add_alternate_map_letter) fileName += "_a";
	}

	static const SGPSector MedunaHQ(3, 16, 0);
	if (AreInMeanwhile() && sector == MedunaHQ)
	{
		if (add_alternate_map_letter) fileName += "_m";
	}

	fileName += ".dat";
	return fileName;
}


static void HandleRPCDescriptionOfSector(const SGPSector& sector)
{
	struct SectorDescriptionInfo
	{
		UINT8 y;
		UINT8 x;
		UINT8 quote;
	};

	SectorDescriptionInfo const sector_description[] =
	{
		{  2, 13,  0 }, // B13 Drassen
		{  3, 13,  1 }, // C13 Drassen
		{  4, 13,  2 }, // D13 Drassen
		{  8, 13,  3 }, // H13 Alma
		{  8, 14,  4 }, // H14 Alma
		{  9, 13,  5 }, // I13 Alma (extra quote 6 if Sci-fi)
		{  9, 14,  7 }, // I14 Alma
		{  6,  8,  8 }, // F8  Cambria
		{  6,  9,  9 }, // F9  Cambria
		{  7,  8, 10 }, // G8  Cambria

		{  7,  9, 11 }, // G9  Cambria
		{  3,  6, 12 }, // C6  San Mona
		{  3,  5, 13 }, // C5  San Mona
		{  4,  5, 14 }, // D5  San Mona
		{  2,  2, 15 }, // B2  Chitzena
		{  1,  2, 16 }, // A2  Chitzena
		{  7,  1, 17 }, // G1  Grumm
		{  8,  1, 18 }, // H1  Grumm
		{  7,  2, 19 }, // G2  Grumm
		{  8,  2, 20 }, // H2  Grumm

		{  9,  6, 21 }, // I6  Estoni
		{ 11,  4, 22 }, // K4  Orta
		{ 12, 11, 23 }, // L11 Balime
		{ 12, 12, 24 }, // L12 Balime
		{ 15,  3, 25 }, // O3  Meduna
		{ 16,  3, 26 }, // P3  Meduna
		{ 14,  4, 27 }, // N4  Meduna
		{ 14,  3, 28 }, // N3  Meduna
		{ 15,  4, 30 }, // O4  Meduna
		{ 10,  9, 31 }, // J9  Tixa

		{  4, 15, 32 }, // D15 NE SAM
		{  4,  2, 33 }, // D2  NW SAM
		{  9,  8, 34 }  // I8  CENTRAL SAM
	};

	TacticalStatusType& ts = gTacticalStatus;
	// Default to false
	ts.fCountingDownForGuideDescription = false;

	if (GetSectorFlagStatus(sector, SF_HAVE_USED_GUIDE_QUOTE)) return;
	if (sector.z != 0) return;

	// Check if we are in a good sector
	FOR_EACH(SectorDescriptionInfo const, i, sector_description)
	{
		if (sector.x != i->x || sector.y != i->y) continue;

		// If we're not scifi, skip some
		if (i == &sector_description[3] && !gGameOptions.fSciFi) continue;

		SetSectorFlag(sector, SF_HAVE_USED_GUIDE_QUOTE);

		ts.fCountingDownForGuideDescription = true;
		ts.bGuideDescriptionCountDown       = 4 + Random(5); // 4 to 8 tactical turns
		ts.ubGuideDescriptionToUse          = i->quote;
		ts.bGuideDescriptionSectorX         = sector.x;
		ts.bGuideDescriptionSectorY         = sector.y;

		// Handle guide description (will be needed if a SAM one)
		HandleRPCDescription();
		break;
	}
}


static void EnterSector(const SGPSector& sector);
enum
{
	ABOUT_TO_LOAD_NEW_MAP,
	ABOUT_TO_TRASH_WORLD,
};
static void HandleDefiniteUnloadingOfWorld(UINT8 ubUnloadCode);

void SetCurrentWorldSector(const SGPSector& sector)
{
	SyncStrategicTurnTimes();

	// is the sector already loaded?
	if (gWorldSector == sector)
	{
		/* Insert the enemies into the newly loaded map based on the strategic
		 * information. Note, the flag will return true only if enemies were added.
		 * The game may wish to do something else in a case where no enemies are
		 * present. */

		SetPendingNewScreen(GAME_SCREEN);
		if (NumEnemyInSector() == 0)
		{
			PrepareEnemyForSectorBattle();
		}
		if (gubNumCreaturesAttackingTown != 0 &&
				sector.z                    == 0 &&
				gubSectorIDOfCreatureAttack == sector.AsByte())
		{
			PrepareCreaturesForBattle();
		}

		if (gfGotoSectorTransition)
		{
			BeginLoadScreen();
			gfGotoSectorTransition = false;
		}

		HandleHelicopterOnGround(true);

		ResetMilitia();
		AllTeamsLookForAll(true);
		return;
	}

	if (gWorldSector.IsValid())
	{
		HandleDefiniteUnloadingOfWorld(ABOUT_TO_LOAD_NEW_MAP);
	}

	// make this the currently loaded sector
	gWorldSector = sector;

	// update currently selected map sector to match
	ChangeSelectedMapSector(sector);

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
		static const SGPSector TixaCave = SGPSector(9, 10, 2);
		if (sector == TixaCave)
		{
			InitCreatureQuest(); // Ignored if already active.
		}

		gTacticalStatus.uiTimeSinceLastInTactical = GetWorldTotalMin();
		InitializeTacticalStatusAtBattleStart();
		HandleHelicopterOnGround(false);
	}

	EnterSector(sector);

	if (!loading_savegame)
	{
		InitAI();
		ExamineDoorsOnEnteringSector();
	}

	/* Update all the doors in the sector according to the temp file previously
	 * loaded, and any changes made by the schedules */
	UpdateDoorGraphicsFromStatus();

	// Set the fact we have visited the  sector
	SetSectorFlag(sector, SF_ALREADY_LOADED);

	// Check for helicopter being on the ground in this sector
	HandleHelicopterOnGround(true);

	if (!loading_savegame)
	{
		if (gubMusicMode == MUSIC_TACTICAL_ENEMYPRESENT ?
			NumHostilesInSector(sector) == 0 :
			gubMusicMode != MUSIC_TACTICAL_BATTLE)
		{
			// ATE: Fade FAST
			SetMusicFadeSpeed(5);
			SetMusicMode(MUSIC_TACTICAL_NOTHING);
		}

		// ATE: Check what sector we are in, to show description if we have an RPC
		HandleRPCDescriptionOfSector(sector);

		ResetMultiSelection();

		gTacticalStatus.fHasEnteredCombatModeSinceEntering = false;
		gTacticalStatus.fDontAddNewCrows                   = false;

		// Adjust delay for tense quote
		gTacticalStatus.sCreatureTenseQuoteDelay = 10 + Random(20);

		SGPSector sWarpWorld;
		INT16 sWarpGridNo;
		if (sector.z >= 2 && GetWarpOutOfMineCodes(sWarpWorld, &sWarpGridNo))
		{
			gTacticalStatus.uiFlags |= IN_CREATURE_LAIR;
		}
		else
		{
			gTacticalStatus.uiFlags &= ~IN_CREATURE_LAIR;
		}

		gTacticalStatus.ubNumCrowsPossible = 5 + Random(5);
	}
}


void RemoveMercsInSector( )
{
	// ATE: only for OUR guys.. the rest is taken care of in TrashWorld() when a new sector is added...
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		RemoveSoldierFromGridNo(*i);
	}
}


void PrepareLoadedSector()
{
	BOOLEAN fAddCivs = true;
	INT8 bMineIndex = -1;

	BeforePrepareSector();

	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		UpdateMercsInSector();
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
		if (gWorldSector.z)
		{
			bMineIndex = GetIdOfMineForSector(gWorldSector);
			if( bMineIndex != -1 )
			{
				if( !AreThereMinersInsideThisMine( (UINT8)bMineIndex ) )
				{
					fAddCivs = false;
				}
			}
		}
		if( fAddCivs )
		{
			AddSoldierInitListTeamToWorld(CIV_TEAM);
		}

		AddSoldierInitListTeamToWorld(MILITIA_TEAM);
		AddSoldierInitListBloodcats();
		//Creatures are only added if there are actually some of them.  It has to go through some
		//additional checking.

		PrepareCreaturesForBattle();

		PrepareMilitiaForTactical();

		// OK, set varibles for entring this new sector...
		gTacticalStatus.fVirginSector = true;

		AddProfilesNotUsingProfileInsertionData();

		if( !AreInMeanwhile() || GetMeanwhileID() == INTERROGATION )
		{ // Insert the enemies into the newly loaded map based on the strategic information.
			PrepareEnemyForSectorBattle();
		}


		//Regardless whether or not this was set, clear it now.
		gfRestoringEnemySoldiersFromTempFile = false;

		//@@@Evaluate
		//Add profiles to world using strategic info, not editor placements.
		AddProfilesUsingProfileInsertionData();

		PostSchedules();
	}

	CallAvailableTeamEnemiesToAmbush(gMapInformation.sCenterGridNo);

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

	SLOGD("Current Time is: {}", GetWorldTotalMin());

	AllTeamsLookForAll( true );
}

#define RANDOM_HEAD_MINERS 4
void HandleQuestCodeOnSectorEntry(const SGPSector& sNewSector)
{
	UINT8		ubRandomMiner[RANDOM_HEAD_MINERS] = { 106, 156, 157, 158 };
	UINT8		ubMiner, ubMinersPlaced;

	if ( CheckFact( FACT_ALL_TERRORISTS_KILLED, 0 ) )
	{
		// end terrorist quest
		EndQuest(QUEST_KILL_TERRORISTS, gMercProfiles[CARMEN].sSector);
		// remove Carmen
		gMercProfiles[ CARMEN ].sSector = SGPSector();
	}

	UINT8 const sector = sNewSector.AsByte();
	// are we in a mine sector, on the surface?
	if (sNewSector.z == 0)
	{
		INT8 const ubThisMine = GetMineIndexForSector(sector);
		if (ubThisMine != -1 && !CheckFact(FACT_MINERS_PLACED, 0))
		{
			auto thisMine = GCM->getMine(ubThisMine);
			// SET HEAD MINER LOCATIONS, unless mine is abandoned
			if (!thisMine->isAbandoned())
			{
				ubMinersPlaced = 0;

				if (!thisMine->headMinerAssigned)
				{
					// Fred Morris is always in the first mine sector we enter, unless head miner here has been pre-determined (then he's randomized, too)
					MERCPROFILESTRUCT& fred = GetProfile(FRED);
					fred.sSector  = sNewSector;
					fred.bTown    = thisMine->associatedTownId;

					// mark miner as placed
					ubRandomMiner[ 0 ] = 0;
					ubMinersPlaced++;
				}

				// assign the remaining (3) miners randomly
				for (const MineModel* ubMine : GCM->getMines())
				{
					if ( ubMine->mineId == ubThisMine || ubMine->headMinerAssigned || ubMine->isAbandoned() )
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

					MERCPROFILESTRUCT& p      = GetProfile(ubRandomMiner[ubMiner]);
					UINT8 const        sector = ubMine->entranceSector;
					p.sSector = SGPSector(sector);
					p.bTown = ubMine->associatedTownId;

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
			RemoveGraphicFromTempFile( gsRobotGridNo, SEVENTHISTRUCT1, gMercProfiles[MADLAB].sSector);
			SetFactTrue( FACT_ROBOT_RECRUITED_AND_MOVED );
		}
	}

	// Check to see if any player merc has the Chalice; if so,
	// note it as stolen
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (FindObj(s, CHALICE) != ITEM_NOT_FOUND)
		{
			SetFactTrue(FACT_CHALICE_STOLEN);
		}
	}

	if (gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS &&
			CheckFact(FACT_KINGPIN_CAN_SEND_ASSASSINS, 0)    &&
			GetTownIdForSector(sector) != BLANK_SECTOR       &&
			Random(10 + GetNumberOfMilitiaInSector(sNewSector)) < 3)
	{
		DecideOnAssassin();
	}

/*
	if (sector == SEC_C5)
	{
		// reset Madame Layla counters
		gMercProfiles[ MADAME ].bNPCData = 0;
		gMercProfiles[ MADAME ].bNPCData2 = 0;
	}
	*/

	if (sector == SEC_D5)
	{
		gBoxer[0] = NULL;
		gBoxer[1] = NULL;
		gBoxer[2] = NULL;
	}

	if (sector == SEC_P3)
	{
		// heal up Elliot if he's been hurt
		if ( gMercProfiles[ ELLIOT ].bMercStatus != MERC_IS_DEAD )
		{
			gMercProfiles[ ELLIOT ].bLife = gMercProfiles[ ELLIOT ].bLifeMax;
		}
	}

	ResetOncePerConvoRecordsForAllNPCsInLoadedSector();
}


static void HandleQuestCodeOnSectorExit(const SGPSector& oldSector)
{
	SGPSector sector(KINGPIN_MONEY_SECTOR_X, KINGPIN_MONEY_SECTOR_Y, KINGPIN_MONEY_SECTOR_Z);
	if (oldSector == sector)
	{
		CheckForKingpinsMoneyMissing( true );
	}

	sector = SGPSector(13, MAP_ROW_H, 0);
	if (oldSector == sector && CheckFact(FACT_CONRAD_SHOULD_GO, 0))
	{
		// remove Conrad from the map
		gMercProfiles[ CONRAD ].sSector = SGPSector();
	}

	sector = SGPSector(HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y, HOSPITAL_SECTOR_Z);
	if (oldSector == sector)
	{
		CheckForMissingHospitalSupplies();
	}

	// reset the state of the museum alarm for Eldin's quotes
	SetFactFalse( FACT_MUSEUM_ALARM_WENT_OFF );
}


static void SetupProfileInsertionDataForCivilians(void)
{
	FOR_EACH_IN_TEAM(s, CIV_TEAM)
	{
		if (s->bInSector) SetupProfileInsertionDataForSoldier(s);
	}
}


static void EnterSector(const SGPSector& sector)
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
		HandleQuestCodeOnSectorEntry(sector);
	}

	BeginLoadScreen();

	/* This has to be done before loadworld, as it will remmove old gridnos if
	 * present */
	RemoveMercsInSector();

	if (!AreInMeanwhile())
	{
		SetSectorFlag(sector, SF_ALREADY_VISITED);
	}

	CreateLoadingScreenProgressBar();

	ST::string filename = GetMapFileName(sector, true);
	LoadWorld(filename);
	LoadRadarScreenBitmap(filename);
	// We have to add the helicopter after the sector is fully loaded
	// to prevent that the pathfinding doenst consider its collission-grids
	HandleHelicopterOnGround(true);

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
			return;
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
		EndMapScreen(false);
		PrepareLoadedSector();
	}

	/* This function will either hide or display the tree tops, depending on the
	 * game setting */
	SetTreeTopStateForMap();
}


void UpdateMercsInSector()
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
		gfGameScreenLocateToSoldier = true;
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

		SOLDIERTYPE& s = GetMan(i);
		RemoveMercSlot(&s);

		s.bInSector = false;

		if (!s.bActive)             continue;
		if (s.sSector != gWorldSector) continue;
		if (s.fBetweenSectors)      continue;

		if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
		{
			if (gMapInformation.sCenterGridNo != -1 &&
					gfBlitBattleSectorLocator           &&
					s.bTeam != CIV_TEAM                 &&
					(
						gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
						gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE
					))
			{
				s.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				s.usStrategicInsertionData = gMapInformation.sCenterGridNo;
			}
			else if (gfOverrideInsertionWithExitGrid)
			{
				s.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				s.usStrategicInsertionData = gExitGrid.usGridNo;
			}
		}

		UpdateMercInSector(s, gWorldSector);

		if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) continue;
		if (s.bAssignment != ASSIGNMENT_POW)              continue;

		static const SGPSector secI13(13, MAP_ROW_I);
		if (pow_squad == NO_CURRENT_SQUAD)
		{
			// ATE: If we are in i13 - pop up message!
			if (gWorldSector == secI13)
			{
				DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[POW_MERCS_ARE_HERE], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
			}
			else
			{
				AddCharacterToUniqueSquad(&s);
				pow_squad  = s.bAssignment;
				s.bNeutral = false;
			}
		}
		else
		{
			if (gWorldSector != secI13)
			{
				AddCharacterToSquad(&s, pow_squad);
			}
		}

		// ATE: Call actions based on what POW we are on...
		if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTINPROGRESS)
		{
			EndQuest(QUEST_HELD_IN_ALMA, gWorldSector);
			HandleNPCDoAction(0, NPC_ACTION_GRANT_EXPERIENCE_3, 0);
		}
	}

	if (fUsingEdgePointsForStrategicEntry)
	{
		EndMapEdgepointSearch();
		fUsingEdgePointsForStrategicEntry = false;
	}
}

void UpdateMercInSector(SOLDIERTYPE& s, const SGPSector& sSector)
{
	// Determine entrance direction and get sweetspot
	// Some checks here must be fleshed out

	if (!s.bActive) return;

	if (s.bAssignment == IN_TRANSIT) return;

	if (s.ubProfile != NO_PROFILE && GetProfile(s.ubProfile).ubMiscFlags3 & PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE)
	{ // Override orders
		s.bOrders = STATIONARY;
	}

	if (s.ubStrategicInsertionCode == INSERTION_CODE_PRIMARY_EDGEINDEX ||
			s.ubStrategicInsertionCode == INSERTION_CODE_SECONDARY_EDGEINDEX)
	{
		if (!fUsingEdgePointsForStrategicEntry)
		{ // If we are not supposed to use this now, pick something better
			s.ubStrategicInsertionCode = (UINT8)s.usStrategicInsertionData;
		}
	}

MAPEDGEPOINT_SEARCH_FAILED:
	// Use insertion direction from loaded map
	GridNo gridno;
	static const SGPSector startSector(gamepolicy(start_sector));
	switch (s.ubStrategicInsertionCode)
	{
		case INSERTION_CODE_NORTH:  gridno = gMapInformation.sNorthGridNo;  goto check_entry;
		case INSERTION_CODE_SOUTH:  gridno = gMapInformation.sSouthGridNo;  goto check_entry;
		case INSERTION_CODE_EAST:   gridno = gMapInformation.sEastGridNo;   goto check_entry;
		case INSERTION_CODE_WEST:   gridno = gMapInformation.sWestGridNo;   goto check_entry;
		case INSERTION_CODE_CENTER: gridno = gMapInformation.sCenterGridNo; goto check_entry;
check_entry:
			if (gridno == -1 && !gfEditMode)
			{ /* Strategic insertion failed because it expected to find an entry
				 * point. This is likely a missing part of the map or possible fault in
				 * strategic movement costs, traversal logic, etc. */
				ST::string sector;
				if (gfWorldLoaded)
				{
					if (gWorldSector.z == 0)
					{
						 sector = gWorldSector.AsShortString();
					}
					else
					{
						 sector = gWorldSector.AsLongString();
					}
				}

				ST::string entry;
				if (gMapInformation.sNorthGridNo != -1)
				{
					entry  = "north";
					gridno = gMapInformation.sNorthGridNo;
				}
				else if (gMapInformation.sEastGridNo != -1)
				{
					entry  = "east";
					gridno = gMapInformation.sEastGridNo;
				}
				else if (gMapInformation.sSouthGridNo != -1)
				{
					entry  = "south";
					gridno = gMapInformation.sSouthGridNo;
				}
				else if (gMapInformation.sWestGridNo != -1)
				{
					entry  = "west";
					gridno = gMapInformation.sWestGridNo;
				}
				else if (gMapInformation.sCenterGridNo != -1)
				{
					entry  = "center";
					gridno = gMapInformation.sCenterGridNo;
				}
				else
				{
					SLOGD("Sector {} has NO entrypoints -- using precise center of map for {}.", sector, s.name);
					goto place_in_center;
				}
				ST::string no_entry;
				switch (s.ubStrategicInsertionCode)
				{
					case INSERTION_CODE_NORTH:  no_entry = "north";  break;
					case INSERTION_CODE_EAST:   no_entry = "east";   break;
					case INSERTION_CODE_SOUTH:  no_entry = "south";  break;
					case INSERTION_CODE_WEST:   no_entry = "west";   break;
					case INSERTION_CODE_CENTER: no_entry = "center"; break;
				}
				if (!no_entry.empty())
				{
					SLOGD("Sector {} doesn't have a {} entrypoint -- substituting {} entrypoint for {}.", sector, no_entry, entry, s.name);
				}
			}
			break;

		case INSERTION_CODE_GRIDNO:
			gridno = s.usStrategicInsertionData;
			break;

		case INSERTION_CODE_PRIMARY_EDGEINDEX:
		{
			gridno = SearchForClosestPrimaryMapEdgepoint(s.sPendingActionData2, (UINT8)s.usStrategicInsertionData);
			SLOGD("{}'s primary insertion gridno is {} using {} as initial search gridno and {} insertion code.",
						s.name, gridno, s.sPendingActionData2, s.usStrategicInsertionData);
			if (gridno == NOWHERE)
			{
				SLOGE("Main edgepoint search failed for {} -- substituting entrypoint.", s.name);
				s.ubStrategicInsertionCode = (UINT8)s.usStrategicInsertionData;
				goto MAPEDGEPOINT_SEARCH_FAILED;
			}
			break;
		}

		case INSERTION_CODE_SECONDARY_EDGEINDEX:
		{
			gridno = SearchForClosestSecondaryMapEdgepoint(s.sPendingActionData2, (UINT8)s.usStrategicInsertionData);
			SLOGD("{}'s isolated insertion gridno is {} using {} as initial search gridno and {} insertion code.",
						s.name, gridno, s.sPendingActionData2, s.usStrategicInsertionData);
			if (gridno == NOWHERE)
			{
				SLOGE("Isolated edgepoint search failed for {} -- substituting entrypoint.", s.name);
				s.ubStrategicInsertionCode = (UINT8)s.usStrategicInsertionData;
				goto MAPEDGEPOINT_SEARCH_FAILED;
			}
			break;
		}

		case INSERTION_CODE_ARRIVING_GAME:
			// Are we in the start sector?
			if (sSector == startSector && gWorldSector == startSector)
			{ // Try another location and walk into map
				gridno = 4379;
			}
			else
			{
				s.ubStrategicInsertionCode = INSERTION_CODE_NORTH;
				gridno                     = gMapInformation.sNorthGridNo;
			}
			break;

		case INSERTION_CODE_CHOPPER:
			AddMercToHeli(&s);
			return;

		default:
			SLOGD("Improper insertion code {} given to UpdateMercsInSector", s.ubStrategicInsertionCode);
			goto place_in_center;
	}

	// If no insertion direction exists, this is bad!
	if (gridno == -1)
	{
		SLOGW("Insertion gridno for direction {} not added to map sector {}", s.ubStrategicInsertionCode, sSector);
place_in_center:
		gridno = WORLD_ROWS / 2 * WORLD_COLS + WORLD_COLS / 2;
	}

	s.sInsertionGridNo = gridno;
	AddSoldierToSector(&s);
}


static void InitializeStrategicMapSectorTownNames(void)
{
	for (auto& element: GCM->getTowns())
	{
		auto town = element.second;
		for (auto sector : town->sectorIDs)
		{
			StrategicMap[ SGPSector(sector).AsStrategicIndex() ].bNameId = town->townId;
		}
	}
}

ST::string GetSectorLandTypeString(UINT8 const ubSectorID, UINT8 const ubSectorZ, bool const fDetailed)
{
	// first consider map secrets and SAM sites
	auto secret = GetMapSecretBySectorID(ubSectorID);
	if (ubSectorZ == 0 && secret)
	{
		UINT8 ubLandType = secret->getLandType(IsSecretFoundAt(ubSectorID));
		if (ubLandType != TOWN) // we will handle town sectors separately
		{
			return (secret->isSAMSite && !fDetailed)
					? GCM->getLandTypeString(SAM_SITE)
					: GCM->getLandTypeString(ubLandType);
		}
	}

	// special facilities
	if (ubSectorZ > 0 || fDetailed)
	{
		int16_t landType = GCM->getSectorLandType(ubSectorID, ubSectorZ);
		if (landType >= 0)
		{
			return GCM->getLandTypeString(landType);
		}
	}

	INT8 const town_name_id = StrategicMap[SGPSector(ubSectorID).AsStrategicIndex()].bNameId;
	if (town_name_id != BLANK_SECTOR)
	{	// show town name
		return GCM->getTownName(town_name_id);
	}

	if (ubSectorZ > 0)
	{	// any other underground sectors (not facility, not part of a mine) are creature lair
		return GCM->getLandTypeString(CREATURE_LAIR);
	}

	// finally consider the sector traversibility
	UINT8 ubTraversibility = SectorInfo[ubSectorID].ubTraversability[THROUGH_STRATEGIC_MOVE];
	return GCM->getLandTypeString(ubTraversibility);
}

ST::string GetSectorIDString(const SGPSector& sector, BOOLEAN detailed)
{
	if (!sector.IsValid())
	{
		return {};
	}

	if (sector.z != 0)
	{
		UNDERGROUND_SECTORINFO const* const u = FindUnderGroundSector(sector);
		if (!u || (!(u->uiFlags & SF_ALREADY_VISITED) && !gfGettingNameFromSaveLoadScreen))
		{ // Display nothing
			return {};
		}
	}

	INT8    const  mine_index = GetIdOfMineForSector(sector);
	ST::string add;
	if (mine_index != -1)
	{
		add = GCM->getTownName(GetTownAssociatedWithMine(mine_index));
		if (detailed && mine_index != -1)
		{	// Append "Mine"
			add += ST::format(" {}", pwMineStrings[0]);
		}
	}

	if (add.empty())
	{
		UINT8 const sector_id = sector.AsByte();
		add = GetSectorLandTypeString(sector_id, sector.z, detailed);
	}

	return ST::format("{c}{}: {}", 'A' + sector.y - 1, sector.x, add);
}


static void SetInsertionDataFromAdjacentMoveDirection(SOLDIERTYPE& s, UINT8 const tactical_direction, INT16 const additional_data)
{
	EXITGRID ExitGrid;
	// Set insertion code
	switch (tactical_direction)
	{
		case 255:
			// We are using an exit grid, set insertion values
			if (!GetExitGrid(additional_data, &ExitGrid))
			{
				SLOGA("No valid Exit grid can be found when one was expected: SetInsertionDataFromAdjacentMoveDirection.");
			}
			s.ubStrategicInsertionCode        = INSERTION_CODE_GRIDNO;
			s.usStrategicInsertionData        = ExitGrid.usGridNo;
			s.bUseExitGridForReentryDirection = true;
			break;

		case NORTH: s.ubStrategicInsertionCode = INSERTION_CODE_SOUTH; break;
		case SOUTH: s.ubStrategicInsertionCode = INSERTION_CODE_NORTH; break;
		case EAST:  s.ubStrategicInsertionCode = INSERTION_CODE_WEST;  break;
		case WEST:  s.ubStrategicInsertionCode = INSERTION_CODE_EAST;  break;

		default: // Wrong direction given
			SLOGD("Improper insertion direction {} given to SetInsertionDataFromAdjacentMoveDirection", tactical_direction);
			s.ubStrategicInsertionCode = INSERTION_CODE_WEST;
			break;
	}
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
			SLOGD("Improper insertion direction {} given to GetInsertionDataFromAdjacentMoveDirection", ubTacticalDirection);
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
			SLOGD("Improper insertion direction {} given to GetStrategicInsertionDataFromAdjacentMoveDirection", ubTacticalDirection);
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
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
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
		SLOGD("Improper jump code {} given to JumpIntoAdjacentSector", ubJumpCode);
	}

	Assert( pValidSoldier );

	//Now, determine the traversal time.
	GROUP* const pGroup = GetGroup(pValidSoldier->ubGroupID);
	AssertMsg(pGroup, ST::format("{} is not in a valid group(pSoldier->ubGroupID is {})", pValidSoldier->name, pValidSoldier->ubGroupID));

	// If we are going through an exit grid, don't get traversal direction!
	if ( ubTacticalDirection != 255 )
	{
		if (!gWorldSector.z)
		{
			uiTraverseTime = GetSectorMvtTimeForGroup(pGroup->ubSector.AsByte(), ubDirection, pGroup);
		}
		else if (gWorldSector.z > 0)
		{ //We are attempting to traverse in an underground environment.  We need to use a complete different
			//method.  When underground, all sectors are instantly adjacent.
			uiTraverseTime = UndergroundTacticalTraversalTime( ubDirection );
		}
		AssertMsg(uiTraverseTime != TRAVERSE_TIME_IMPOSSIBLE, "Attempting to tactically traverse to adjacent sector, despite being unable to do so.");
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
		gsAdjacentSector = gWorldSector + SectorIncrementer[ubTacticalDirection];
		gsAdjacentSector.z = pValidSoldier->sSector.z;
	}
	else
	{
		// Take directions from exit grid info!
		if ( !GetExitGrid( sAdditionalData, &ExitGrid ) )
		{
			SLOGA("Told to use exit grid at {} but one does not exist", sAdditionalData);
		}

		gsAdjacentSector = ExitGrid.ubGotoSector;
	}

	// Give guy(s) orders to walk off sector...
	if( pGroup->fPlayer )
	{	//For player groups, update the soldier information
		UINT8				ubNum = 0;

		CFOR_EACH_PLAYER_IN_GROUP(curr, pGroup)
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
						curr->pSoldier->uiPendingActionData1 = ubTacticalDirection;
					}
					else
					{
						SLOGA("Failed to get good exit location for adjacentmove");
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
						//curr->pSoldier->ubWaitActionToDo = 2;
					}
					else
					{
						SLOGA("Failed to get good exit location for adjacentmove");
					}

					// Don't worry about walk off screen, just stay at gridno...
					curr->pSoldier->ubWaitActionToDo = 1;

					// Set buddy go!
					gfPlotPathToExitGrid = true;
					EVENT_GetNewSoldierPath( curr->pSoldier, sGridNo, WALKING );
					gfPlotPathToExitGrid = false;

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
		CFOR_EACH_PLAYER_IN_GROUP(curr, pGroup)
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
			RemovePlayerFromGroup(*pSoldier);
			pSoldier->ubGroupID = 0;
		}
	}
	else
	{
		// otherwise, they are on thier own, not in a squad, simply remove mvt group
		if( pSoldier->ubGroupID && pSoldier->bAssignment != VEHICLE )
		{ //Can only remove groups if they aren't persistant (not in a squad or vehicle)
			// delete group
			RemoveGroup(*GetGroup(pSoldier->ubGroupID));
			pSoldier->ubGroupID = 0;
		}
	}

	// set to guard
	AddCharacterToUniqueSquad( pSoldier );

	if( pSoldier->ubGroupID == 0 )
	{
		// create independant group
		GROUP& g = *CreateNewPlayerGroupDepartingFromSector(pSoldier->sSector);
		AddPlayerToGroup(g, *pSoldier);
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
		CFOR_EACH_PLAYER_IN_GROUP(pPlayer, gpAdjacentGroup)
		{
			SOLDIERTYPE& s = *pPlayer->pSoldier;
			SetInsertionDataFromAdjacentMoveDirection(s, gubTacticalDirection, gsAdditionalData);
			RemoveSoldierFromTacticalSector(s);
		}

		SetGroupSectorValue(gsAdjacentSector, *gpAdjacentGroup);

		SetDefaultSquadOnSectorEntry( true );

	}
	else
	{
		//Because we are actually loading the new map, and we are physically traversing, we don't want
		//to bring up the prebattle interface when we arrive if there are enemies there.  This flag
		//ignores the initialization of the prebattle interface and clears the flag.
		gfTacticalTraversal = true;
		gpTacticalTraversalGroup = gpAdjacentGroup;

		//Check for any unconcious and/or dead merc and remove them from the current squad, so that they
		//don't get moved to the new sector.
		fDone = false;
		while( !fDone )
		{
			fDone = false;
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
				fDone = true;
			}
		}

		// OK, Set insertion direction for all these guys....
		Assert( gpAdjacentGroup );
		CFOR_EACH_PLAYER_IN_GROUP(pPlayer, gpAdjacentGroup)
		{
			SetInsertionDataFromAdjacentMoveDirection(*pPlayer->pSoldier, gubTacticalDirection, gsAdditionalData);
		}
		SetGroupSectorValue(gsAdjacentSector, *gpAdjacentGroup);

		gFadeOutDoneCallback = DoneFadeOutExitGridSector;
		FadeOutGameScreen( );
	}
	if (!PlayerMercsInSector(gsAdjacentSector))
	{
		HandleLoyaltyImplicationsOfMercRetreat(RETREAT_TACTICAL_TRAVERSAL, gsAdjacentSector);
	}
	if( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD )
	{
		gfTacticalTraversal = false;
		gpTacticalTraversalGroup = NULL;
		gpTacticalTraversalChosenSoldier = NULL;
	}
}


static void SetupTacticalTraversalInformation(void)
{
	INT16 sScreenX, sScreenY;

	Assert( gpAdjacentGroup );
	CFOR_EACH_PLAYER_IN_GROUP(pPlayer, gpAdjacentGroup)
	{
		SOLDIERTYPE& s = *pPlayer->pSoldier;

		SetInsertionDataFromAdjacentMoveDirection(s, gubTacticalDirection, gsAdditionalData);

		// pass flag that this is a tactical traversal, the path built MUST go in the traversed direction even if longer!
		PlotPathForCharacter(s, gsAdjacentSector, true);

		if( guiAdjacentTraverseTime <= 5 )
		{
			// Determine 'mirror' gridno...
			// Convert to absolute xy
			GetAbsoluteScreenXYFromMapPos(GETWORLDINDEXFROMWORLDCOORDS(s.sY, s.sX), &sScreenX, &sScreenY);

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
			s.sPendingActionData2      = sNewGridNo;
			// Copy CODe computed earlier into data
			s.usStrategicInsertionData = s.ubStrategicInsertionCode;
			// Now use NEW code....

			s.ubStrategicInsertionCode = CalcMapEdgepointClassInsertionCode(s.sPreTraversalGridNo);

			if( gubAdjacentJumpCode == JUMP_SINGLE_LOAD_NEW || gubAdjacentJumpCode == JUMP_ALL_LOAD_NEW )
			{
				fUsingEdgePointsForStrategicEntry = true;
			}
		}
	}
	if( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD )
	{
		gfTacticalTraversal = false;
		gpTacticalTraversalGroup = NULL;
		gpTacticalTraversalChosenSoldier = NULL;
	}
}


static void DoneFadeOutAdjacentSector(void);


void AllMercsHaveWalkedOffSector( )
{
	BOOLEAN fEnemiesInLoadedSector = false;

	if (NumEnemiesInAnySector(gWorldSector))
	{
		fEnemiesInLoadedSector = true;
	}

	if (fEnemiesInLoadedSector)
	{
		HandleLoyaltyImplicationsOfMercRetreat(RETREAT_TACTICAL_TRAVERSAL, gWorldSector);
	}

	//Setup strategic traversal information
	if( guiAdjacentTraverseTime <= 5 )
	{
		gfTacticalTraversal = true;
		gpTacticalTraversalGroup = gpAdjacentGroup;

		if (gsAdjacentSector.z > 0 && guiAdjacentTraverseTime <= 5)
		{	//Nasty strategic movement logic doesn't like underground sectors!
			gfUndergroundTacticalTraversal = true;
		}
	}
	ClearMercPathsAndWaypointsForAllInGroup(*gpAdjacentGroup);
	AddWaypointToPGroup(gpAdjacentGroup, gsAdjacentSector);
	if (gsAdjacentSector.z > 0 && guiAdjacentTraverseTime <= 5)
	{	//Nasty strategic movement logic doesn't like underground sectors!
		gfUndergroundTacticalTraversal = true;
	}

	SetupTacticalTraversalInformation();

	// ATE: Added here: donot load another screen if we were told not to....
	if( ( gubAdjacentJumpCode == JUMP_ALL_NO_LOAD || gubAdjacentJumpCode == JUMP_SINGLE_NO_LOAD ) )
	{ //Case 1:  Group is leaving sector, but there are other mercs in sector and player wants to stay, or
		//         there are other mercs in sector while a battle is in progress.
		CFOR_EACH_PLAYER_IN_GROUP(pPlayer, gpAdjacentGroup)
		{
			RemoveSoldierFromTacticalSector(*pPlayer->pSoldier);
		}
		SetDefaultSquadOnSectorEntry( true );
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
			ResetMovementForEnemyGroupsInLocation();

			if( guiAdjacentTraverseTime > 5 )
			{
				//Because this final group is retreating, simulate extra time to retreat, so they can't immediately come back.
				WarpGameTime( 300, WARPTIME_NO_PROCESSING_OF_EVENTS );
			}
		}
		if( guiAdjacentTraverseTime <= 5 )
		{
			//Case 2:  Immediatly loading the next sector
			if (!gsAdjacentSector.z)
			{
				UINT32 uiWarpTime;
				uiWarpTime = (GetWorldTotalMin() + 5) * 60 - GetWorldTotalSeconds();
				WarpGameTime( uiWarpTime, WARPTIME_PROCESS_TARGET_TIME_FIRST );
			}
			else if (gsAdjacentSector.z > 0)
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
			gfEnteringMapScreen = true;

			// ATE; Fade FAST....
			SetMusicFadeSpeed( 5 );
			SetMusicMode( MUSIC_TACTICAL_NOTHING );
		}
	}
}


static void DoneFadeOutExitGridSector(void)
{
	SetCurrentWorldSector(gsAdjacentSector);
	if( gfTacticalTraversal && gpTacticalTraversalGroup && gpTacticalTraversalChosenSoldier )
	{
		if( gTacticalStatus.fEnemyInSector )
		{
			TacticalCharacterDialogue(gpTacticalTraversalChosenSoldier, QUOTE_ENEMY_PRESENCE);
		}
	}
	gfTacticalTraversal = false;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;
	FadeInGameScreen( );
}


static INT16 PickGridNoToWalkIn(SOLDIERTYPE* pSoldier, UINT8 ubInsertionDirection, UINT32* puiNumAttempts);


static void DoneFadeOutAdjacentSector(void)
{
	UINT8 ubDirection;
	SetCurrentWorldSector(gsAdjacentSector);

	ubDirection = GetStrategicInsertionDataFromAdjacentMoveDirection( gubTacticalDirection, gsAdditionalData );
	if( gfTacticalTraversal && gpTacticalTraversalGroup && gpTacticalTraversalChosenSoldier )
	{
		if( gTacticalStatus.fEnemyInSector )
		{
			TacticalCharacterDialogue(gpTacticalTraversalChosenSoldier, QUOTE_ENEMY_PRESENCE);
		}
	}
	gfTacticalTraversal = false;
	gpTacticalTraversalGroup = NULL;
	gpTacticalTraversalChosenSoldier = NULL;

	if ( gfCaves )
	{
		// ATE; Set tactical status flag...
		gTacticalStatus.uiFlags |= IGNORE_ALL_OBSTACLES;
		// Set pathing flag to path through anything....
		gfPathAroundObstacles = false;
	}

	// OK, give our guys new orders...
	if( gpAdjacentGroup->fPlayer )
	{
		//For player groups, update the soldier information
		UINT32 uiAttempts;
		INT16				sGridNo, sOldGridNo;
		UINT8				ubNum = 0;
		CFOR_EACH_PLAYER_IN_GROUP(curr, gpAdjacentGroup)
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
				SLOGW("{}'s gridno is NOWHERE, and is attempting to walk into sector.", curr->pSoldier->name);
			}
		}
		SetActionToDoOnceMercsGetToLocation(WAIT_FOR_MERCS_TO_WALKON_SCREEN, ubNum);
		guiPendingOverrideEvent = LU_BEGINUILOCK;
		HandleTacticalUI( );

		// Unset flag here.....
		gfPathAroundObstacles = true;

	}
	FadeInGameScreen( );
}


static BOOLEAN SoldierOKForSectorExit(SOLDIERTYPE* pSoldier, INT8 bExitDirection, UINT16 usAdditionalData)
{
	INT16 sWorldX;
	INT16 sWorldY;

	// if the soldiers gridno is not NOWHERE
	if( pSoldier->sGridNo == NOWHERE )
		return false;

	// OK, anyone on roofs cannot!
	if ( pSoldier->bLevel > 0 )
		return false;

	// Get screen coordinates for current position of soldier
	GetAbsoluteScreenXYFromMapPos(pSoldier->sGridNo, &sWorldX, &sWorldY);

	// Check direction
	switch( bExitDirection )
	{
		case EAST_STRATEGIC_MOVE:

			if ( sWorldX < ( ( gsRightX - gsLeftX ) - CHECK_DIR_X_DELTA ) )
			{
				// NOT OK, return false
				return false;
			}
			break;

		case WEST_STRATEGIC_MOVE:

			if ( sWorldX > CHECK_DIR_X_DELTA )
			{
				// NOT OK, return false
				return false;
			}
			break;

		case SOUTH_STRATEGIC_MOVE:

			if ( sWorldY < ( ( gsBottomY - gsTopY ) - CHECK_DIR_Y_DELTA ) )
			{
				// NOT OK, return false
				return false;
			}
			break;

		case NORTH_STRATEGIC_MOVE:

			if ( sWorldY > CHECK_DIR_Y_DELTA )
			{
				// NOT OK, return false
				return false;
			}
			break;

			// This case is for an exit grid....
			// check if we are close enough.....

		case -1:


			// FOR REALTIME - DO MOVEMENT BASED ON STANCE!
			if (!(gTacticalStatus.uiFlags & INCOMBAT))
			{
				pSoldier->usUIMovementMode =  GetMoveStateBasedOnStance( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight );
			}

			const INT16 sGridNo = FindGridNoFromSweetSpotCloseToExitGrid(pSoldier, usAdditionalData, 10);
			if ( sGridNo == NOWHERE )
			{
				return false;
			}

			// ATE: if we are in combat, get cost to move here....
			if ( gTacticalStatus.uiFlags & INCOMBAT )
			{
				// Turn off at end of function...
				const INT16 sAPs = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
				if ( !EnoughPoints( pSoldier, sAPs, 0, false ) )
				{
					return false;
				}
			}
			break;

	}
	return true;
}

//ATE: Returns false if NOBODY is close enough, 1 if ONLY selected guy is and 2 if all on squad are...
BOOLEAN OKForSectorExit( INT8 bExitDirection, UINT16 usAdditionalData, UINT32 *puiTraverseTimeInMinutes )
{
	BOOLEAN     fAtLeastOneMercControllable = false;
	BOOLEAN     fOnlySelectedGuy = false;
	SOLDIERTYPE *pValidSoldier = NULL;
	UINT8       ubReturnVal = false;
	UINT8       ubNumMercs = 0, ubNumEPCs = 0;
	UINT8       ubPlayerControllableMercsInSquad = 0;

	const SOLDIERTYPE* const sel = GetSelectedMan();
	// must have a selected soldier to be allowed to tactically traverse.
	if (sel == NULL) return false;

	/*
	//Exception code for the two sectors in San Mona that are separated by a cliff.  We want to allow strategic
	//traversal, but NOT tactical traversal.  The only way to tactically go from D4 to D5 (or viceversa) is to enter
	//the cave entrance.
	if (gWorldSector.x == 4 && gWorldSector.y == 4 && !gWorldSector.z && bExitDirection == EAST_STRATEGIC_MOVE )
	{
		gfInvalidTraversal = true;
		return false;
	}
	if (gWorldSector.x == 5 && gWorldSector.y == 4 && !gWorldSector.z && bExitDirection == WEST_STRATEGIC_MOVE )
	{
		gfInvalidTraversal = true;
		return false;
	}
	*/

	gfInvalidTraversal = false;
	gfLoneEPCAttemptingTraversal = false;
	gubLoneMercAttemptingToAbandonEPCs = 0;
	gPotentiallyAbandonedEPC = NULL;

	// Look through all mercs and check if they are within range of east end....
	FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// If we are controllable
		if (OkControllableMerc(pSoldier) && pSoldier->bAssignment == CurrentSquad())
		{
			//Need to keep a copy of a good soldier, so we can access it later, and
			//not more than once.
			pValidSoldier = pSoldier;

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
					gfRobotWithoutControllerAttemptingTraversal = true;
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

				if (pSoldier == sel) fOnlySelectedGuy = true;
			}
			else
			{
				GROUP *pGroup;

				// ATE: Dont's assume exit grids here...
				if ( bExitDirection != -1 )
				{
					//Now, determine if this is a valid path.
					pGroup = GetGroup( pValidSoldier->ubGroupID );
					AssertMsg(pGroup, ST::format("{} is not in a valid group (pSoldier->ubGroupID is {})", pValidSoldier->name, pValidSoldier->ubGroupID));
					UINT32 traverse_time = TRAVERSE_TIME_IMPOSSIBLE;
					if (!gWorldSector.z)
					{
						traverse_time = GetSectorMvtTimeForGroup(pGroup->ubSector.AsByte(), bExitDirection, pGroup);
					}
					else if (gWorldSector.z > 1)
					{ //We are attempting to traverse in an underground environment.  We need to use a complete different
						//method.  When underground, all sectors are instantly adjacent.
						traverse_time = UndergroundTacticalTraversalTime(bExitDirection);
					}
					if (puiTraverseTimeInMinutes) *puiTraverseTimeInMinutes = traverse_time;
					if (traverse_time == TRAVERSE_TIME_IMPOSSIBLE)
					{
						gfInvalidTraversal = true;
						return false;
					}
				}
				else
				{
					// Exit grid travel is instantaneous
					if (puiTraverseTimeInMinutes) *puiTraverseTimeInMinutes = 0;
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
					gfLoneEPCAttemptingTraversal = true;
					return false;
				}
			}
			else
			{	//We previously counted the number of EPCs and mercs, and if the selected merc is not an EPC and there are no
				//other mercs in the squad able to escort the EPCs, we will prohibit this merc from tactically traversing.
				if( ubNumEPCs && ubNumMercs == 1 && fAtLeastOneMercControllable < ubPlayerControllableMercsInSquad )
				{
					gubLoneMercAttemptingToAbandonEPCs = ubNumEPCs;
					return false;
				}
			}
		}
		if ( bExitDirection != -1 )
		{
			GROUP *pGroup;
			//Now, determine if this is a valid path.
			pGroup = GetGroup( pValidSoldier->ubGroupID );
			AssertMsg(pGroup, ST::format("{} is not in a valid group (pSoldier->ubGroupID is {})", pValidSoldier->name, pValidSoldier->ubGroupID));
			UINT32 traverse_time = TRAVERSE_TIME_IMPOSSIBLE; // -XXX Wmaybe-uninitialized : valid default?
			if (!gWorldSector.z)
			{
				traverse_time = GetSectorMvtTimeForGroup(pGroup->ubSector.AsByte(), bExitDirection, pGroup);
			}
			else if (gWorldSector.z > 0)
			{ //We are attempting to traverse in an underground environment.  We need to use a complete different
				//method.  When underground, all sectors are instantly adjacent.
				traverse_time = UndergroundTacticalTraversalTime(bExitDirection);
			}
			if (puiTraverseTimeInMinutes) *puiTraverseTimeInMinutes = traverse_time;
			if (traverse_time == TRAVERSE_TIME_IMPOSSIBLE)
			{
				gfInvalidTraversal = true;
				ubReturnVal = false;
			}
			else
			{
				ubReturnVal = true;
			}
		}
		else
		{
			ubReturnVal = true;
			// Exit grid travel is instantaneous
			if (puiTraverseTimeInMinutes) *puiTraverseTimeInMinutes = 0;
		}
	}

	if ( ubReturnVal )
	{
		// Default to false again, until we see that we have
		ubReturnVal = false;

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


void SetupNewStrategicGame()
{
	// Set all sectors as enemy controlled.
	FOR_EACH(StrategicMapElement, i, StrategicMap)
	{
		i->fEnemyControlled = true;
	}

	InitNewGameClock();
	DeleteAllStrategicEvents();

	// Set up all events that get processed daily.
	BuildDayLightLevels();
	// Check for quests each morning.
	AddEveryDayStrategicEvent(EVENT_CHECKFORQUESTS,                   QUEST_CHECK_EVENT_TIME, 0);
	// Some things get updated in the very early morning.
	AddEveryDayStrategicEvent(EVENT_DAILY_EARLY_MORNING_EVENTS,       EARLY_MORNING_TIME,     0);
	// Daily update BobbyRay Inventory.
	AddEveryDayStrategicEvent(EVENT_DAILY_UPDATE_BOBBY_RAY_INVENTORY, BOBBYRAY_UPDATE_TIME,   0);
	// Daily update of the M.E.R.C. site..
	AddEveryDayStrategicEvent(EVENT_DAILY_UPDATE_OF_MERC_SITE,        0,                      0);
	// Daily update of insured mercs.
	AddEveryDayStrategicEvent(EVENT_HANDLE_INSURED_MERCS,             INSURANCE_UPDATE_TIME,  0);
	// Daily update of mercs.
	AddEveryDayStrategicEvent(EVENT_MERC_DAILY_UPDATE,                0,                      0);
	// Daily mine production processing events.
	AddEveryDayStrategicEvent(EVENT_SETUP_MINE_INCOME,                0,                      0);
	// Daily checks for E-mail from Enrico.
	AddEveryDayStrategicEvent(EVENT_ENRICO_MAIL,                      ENRICO_MAIL_TIME,       0);

	// Hourly update of all sorts of things
	AddPeriodStrategicEvent(EVENT_HOURLY_UPDATE,       60, 0);
	AddPeriodStrategicEvent(EVENT_QUARTER_HOUR_UPDATE, 15, 0);

	// Clear any possible battle locator.
	gfBlitBattleSectorLocator = false;

	StrategicTurnsNewGame();

	// Move the landing zone over to the start sector.
	g_merc_arrive_sector = SGPSector(gamepolicy(start_sector));
}


bool CanGoToTacticalInSector(const SGPSector& sector)
{
	// If not a valid sector
	if (!sector.IsValid()) return false;

	/* Look for all living, fighting mercs on player's team. Robot and EPCs
	 * qualify! */
	CFOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE const& s = *i;
		/* ARM: Now allows loading of sector with all mercs below OKLIFE as long as
		 * they're alive */
		if (s.bLife == 0)                      continue;
		if (s.uiStatusFlags & SOLDIER_VEHICLE) continue;
		if (s.bAssignment == IN_TRANSIT)       continue;
		if (s.bAssignment == ASSIGNMENT_POW)   continue;
		if (s.bAssignment == ASSIGNMENT_DEAD)  continue;
		if (SoldierAboardAirborneHeli(s))      continue;
		if (s.fBetweenSectors)                 continue;
		if (s.sSector != sector)               continue;
		return true;
	}

	return false;
}

static void HandleAirspaceControlUpdated()
{
	// check if currently selected arrival sector still has secure airspace

	// if it's not enemy air controlled
	if (StrategicMap[g_merc_arrive_sector.AsStrategicIndex()].fEnemyAirControlled)
	{
		// get the name of the old sector
		ST::string sMsgSubString1 = GetSectorIDString(g_merc_arrive_sector, false);

		// Move the landing zone over to the start sector.
		g_merc_arrive_sector = SGPSector(gamepolicy(start_sector));

		// get the name of the new sector
		ST::string sMsgSubString2 = GetSectorIDString(g_merc_arrive_sector, false);

		// now build the string
		ST::string sMsgString = st_format_printf(pBullseyeStrings[ 4 ], sMsgSubString1, sMsgSubString2);

		// confirm the change with overlay message
		DoScreenIndependantMessageBox(sMsgString, MSG_BOX_FLAG_OK, NULL);

		// update position of bullseye
		fMapPanelDirty = true;

		// update destination column for any mercs in transit
		fTeamPanelDirty = true;
	}


	// ARM: airspace control now affects refueling site availability, so update that too with every change!
	UpdateRefuelSiteAvailability();
}

void SaveStrategicInfoToSavedFile(HWFILE const f)
{
	// Save the strategic map information
	FOR_EACH(StrategicMapElement const, i, StrategicMap)
	{
		InjectStrategicMapElementIntoFile(f, *i);
	}

	// Save the Sector Info
	FOR_EACH(SECTORINFO const, i, SectorInfo)
	{
		InjectSectorInfoIntoFile(f, *i);
	}

	// Skip the SAM controlled sector information
	f->seek(MAP_WORLD_X * MAP_WORLD_Y, FILE_SEEK_FROM_CURRENT);

	// Save the state of the 2nd map secret (fFoundOrta)
	BOOLEAN fFound = GetMapSecretStateForSave(1);
	f->write(&fFound, sizeof(BOOLEAN));
}


void LoadStrategicInfoFromSavedFile(HWFILE const f)
{
	// Load the strategic map information
	FOR_EACH (StrategicMapElement, i, StrategicMap)
	{
		ExtractStrategicMapElementFromFile(f, *i);
	}

	// Load the Sector Info
	FOR_EACH(SECTORINFO, i, SectorInfo)
	{
		ExtractSectorInfoFromFile(f, *i);
	}

	// Skip the SAM controlled sector information
	f->seek(MAP_WORLD_X * MAP_WORLD_Y, FILE_SEEK_FROM_CURRENT);

	// Load state of the 2nd map secret (fFoundOrta)
	BOOLEAN fFound;
	f->read(&fFound, sizeof(BOOLEAN));
	SetMapSecretStateFromSave(1, fFound);
}


static INT16 PickGridNoNearestEdge(SOLDIERTYPE* pSoldier, UINT8 ubTacticalDirection)
{
	INT16  sGridNo, sStartGridNo, sOldGridNo;
	INT8   bOdd = 1, bOdd2 = 1;
	UINT8  bAdjustedDist = 0;
	UINT32 cnt;

	switch( ubTacticalDirection )
	{

		case EAST:

			sGridNo      = pSoldier->sGridNo;
			sStartGridNo = pSoldier->sGridNo;
			sOldGridNo   = pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
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

			} while( true );

		case WEST:

			sGridNo      = pSoldier->sGridNo;
			sStartGridNo = pSoldier->sGridNo;
			sOldGridNo   = pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
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

			} while( true );

		case NORTH:

			sGridNo      = pSoldier->sGridNo;
			sStartGridNo = pSoldier->sGridNo;
			sOldGridNo   = pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
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

			} while( true );

		case SOUTH:

			sGridNo      = pSoldier->sGridNo;
			sStartGridNo = pSoldier->sGridNo;
			sOldGridNo   = pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			do
			{
				// OK, here we go back one, check for OK destination...
				if ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
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

			} while( true );
	}

	return( NOWHERE );
}


void AdjustSoldierPathToGoOffEdge( SOLDIERTYPE *pSoldier, INT16 sEndGridNo, UINT8 ubTacticalDirection )
{
	INT16 sNewGridNo, sTempGridNo;
	INT32 iLoop;

	// will this path segment actually take us to our desired destination in the first place?
	if (pSoldier->ubPathDataSize + 2 > MAX_PATH_LIST_SIZE)
	{

		sTempGridNo = pSoldier->sGridNo;

		for (iLoop = 0; iLoop < pSoldier->ubPathDataSize; iLoop++)
		{
			sTempGridNo += DirectionInc( pSoldier->ubPathingData[ iLoop ] );
		}

		if (sTempGridNo == sEndGridNo)
		{
			// we can make it, but there isn't enough path room for the two steps required.
			// truncate our path so there's guaranteed the merc will have to generate another
			// path later on...
			pSoldier->ubPathDataSize -= 4;
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

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, DirectionInc( NORTHEAST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = NORTHEAST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, DirectionInc( NORTHEAST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = NORTHEAST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			break;

		case WEST:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, DirectionInc( SOUTHWEST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = SOUTHWEST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, DirectionInc( SOUTHWEST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = SOUTHWEST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;
			break;

		case NORTH:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, (UINT16)DirectionInc( (UINT8)NORTHWEST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = NORTHWEST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, DirectionInc( NORTHWEST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = NORTHWEST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			break;

		case SOUTH:

			sNewGridNo = NewGridNo( (UINT16)sEndGridNo, DirectionInc( SOUTHEAST ) );

			if ( OutOfBounds( sEndGridNo, sNewGridNo ) )
			{
				return;
			}

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = SOUTHEAST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;

			sTempGridNo = NewGridNo( (UINT16)sNewGridNo, DirectionInc( SOUTHEAST ) );

			if ( OutOfBounds( sNewGridNo, sTempGridNo ) )
			{
				return;
			}
			sNewGridNo = sTempGridNo;

			pSoldier->ubPathingData[ pSoldier->ubPathDataSize ] = SOUTHEAST;
			pSoldier->ubPathDataSize++;
			pSoldier->sFinalDestination = sNewGridNo;
			pSoldier->usActionData = sNewGridNo;
			break;

	}
}


static INT16 PickGridNoToWalkIn(SOLDIERTYPE* pSoldier, UINT8 ubInsertionDirection, UINT32* puiNumAttempts)
{
	INT16  sGridNo, sStartGridNo, sOldGridNo;
	INT8   bOdd = 1, bOdd2 = 1;
	UINT8  bAdjustedDist = 0;
	UINT32 cnt;

	*puiNumAttempts = 0;

	switch( ubInsertionDirection )
	{
		// OK, we're given a direction on visible map, let's look for the first oone
		// we find that is just on the start of visible map...
		case INSERTION_CODE_WEST:

			sGridNo      = (INT16)pSoldier->sGridNo;
			sStartGridNo = (INT16)pSoldier->sGridNo;
			sOldGridNo   = (INT16)pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
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

			sGridNo      = (INT16)pSoldier->sGridNo;
			sStartGridNo = (INT16)pSoldier->sGridNo;
			sOldGridNo   = (INT16)pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
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

			sGridNo        = (INT16)pSoldier->sGridNo;
			sStartGridNo   = (INT16)pSoldier->sGridNo;
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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
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

			sGridNo       = (INT16)pSoldier->sGridNo;
			sStartGridNo  = (INT16)pSoldier->sGridNo;
			sOldGridNo    = (INT16)pSoldier->sGridNo;

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

			sGridNo      = sOldGridNo;
			sStartGridNo = sOldGridNo;

			while( *puiNumAttempts < MAX_ATTEMPTS )
			{
				(*puiNumAttempts)++;
				// OK, here we go back one, check for OK destination...
				if ( ( gTacticalStatus.uiFlags & IGNORE_ALL_OBSTACLES ) || ( NewOKDestination( pSoldier, sGridNo, true, pSoldier->bLevel ) && FindBestPath( pSoldier, sGridNo, pSoldier->bLevel, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) ) )
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


bool IsSectorDesert(const SGPSector& sector)
{
	return SectorInfo[sector.AsByte()].ubTraversability[THROUGH_STRATEGIC_MOVE] == SAND;
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
	HandleQuestCodeOnSectorExit(gWorldSector);

	//if we arent loading a saved game
	if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		//Clear any potential battle flags.  They will be set if necessary.
		gTacticalStatus.fEnemyInSector = false;
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
			UpdateSoldierPointerDataIntoProfile();
		}
	}
	else if( ubUnloadCode == ABOUT_TO_TRASH_WORLD )
	{
		//Save the current sectors open temp files to the disk
		SaveCurrentSectorsInformationToTempItemFile();

		//Setup the tactical existance of the current soldier.
		//@@@Evaluate
		SetupProfileInsertionDataForCivilians();

		gfBlitBattleSectorLocator = false;
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
		SOLDIERTYPE const& creature = GetMan(i);
		if (creature.bActive &&
				creature.bLife != 0 &&
				creature.sSector == gWorldSector)
		{ //We have enemies, now look for militia!
			for( i = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID; i <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; i++ )
			{
				SOLDIERTYPE const& milita = GetMan(i);
				if (milita.bActive &&
						milita.bLife != 0 &&
						milita.bSide    == OUR_TEAM &&
						milita.sSector == gWorldSector)
				{ //We have militia and enemies and no mercs!  Let's finish this battle in autoresolve.
					gfEnteringMapScreen = true;
					gfEnteringMapScreenToEnterPreBattleInterface = true;
					gfAutomaticallyStartAutoResolve = true;
					gfUsePersistantPBI = false;
					gubPBSector = gWorldSector;
					gfBlitBattleSectorLocator = true;
					gfTransferTacticalOppositionToAutoResolve = true;
					if( gubEnemyEncounterCode != CREATURE_ATTACK_CODE )
					{
						gubEnemyEncounterCode = ENEMY_INVASION_CODE; //has to be, if militia are here.
					}
					else
					{
						//DoScreenIndependantMessageBox(gzLateLocalizedString[STR_LATE_39], MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
					}

					return true;
				}
			}
		}
	}

	return false;
}


BOOLEAN CheckAndHandleUnloadingOfCurrentWorld()
try
{
	//Don't bother checking this if we don't have a world loaded.
	if( !gfWorldLoaded )
	{
		return false;
	}

	if (DidGameJustStart() && gWorldSector.AsByte() == gamepolicy(start_sector) && gWorldSector.z == 0)
	{
		return false;
	}

	SGPSector sBattleSector;
	GetCurrentBattleSectorXYZ(sBattleSector);

	if( guiCurrentScreen == AUTORESOLVE_SCREEN )
	{ //The user has decided to let the game autoresolve the current battle.
		if (gWorldSector == sBattleSector)
		{
			FOR_EACH_IN_TEAM(i, OUR_TEAM)
			{ //If we have a live and valid soldier
				SOLDIERTYPE& s = *i;
				if (s.bLife == 0)                 continue;
				if (s.fBetweenSectors)            continue;
				if (IsMechanical(s))              continue;
				if (AM_AN_EPC(&s))                continue;
				if (s.sSector != gWorldSector)    continue;
				RemoveSoldierFromGridNo(s);
				InitSoldierOppList(s);
			}
		}
	}
	else
	{	//Check and see if we have any live mercs in the sector.
		CFOR_EACH_IN_TEAM(s, OUR_TEAM)
		{ //If we have a live and valid soldier
			if (s->bLife != 0 &&
					!s->fBetweenSectors &&
					!IsMechanical(*s) &&
					!AM_AN_EPC(s) &&
					s->sSector == gWorldSector)
			{
				return false;
			}
		}
		//KM : August 6, 1999 Patch fix
		//     Added logic to prevent a crash when player mercs would retreat from a battle involving militia and enemies.
		//     Without the return here, it would proceed to trash the world, and then when autoresolve would come up to
		//     finish the tactical battle, it would fail to find the existing soldier information (because it was trashed).
		if( HandlePotentialBringUpAutoresolveToFinishBattle( ) )
		{
			return false;
		}
		//end

		//HandlePotentialBringUpAutoresolveToFinishBattle( ); //prior patch logic
	}


	CheckForEndOfCombatMode( false );
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
			gfEnteringMapScreen = true;
		}
		else
		{ //The trashing of the world will be handled automatically.
			return false;
		}
	}

	//We have passed all the checks and can Trash the world.
	HandleDefiniteUnloadingOfWorld(ABOUT_TO_TRASH_WORLD);

	if( guiCurrentScreen == AUTORESOLVE_SCREEN )
	{
		if (gWorldSector == sBattleSector)
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
	gTacticalStatus.fEnemyInSector = false;
	gTacticalStatus.uiFlags &= ~INCOMBAT;
	EndTopMessage( );


	//Clear the world sector values.
	SetWorldSectorInvalid();

	//Clear the flags regarding.
	gfCaves = false;
	gfBasement = false;

	return true;
}
catch (...) { return false; }


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
				p.fUseProfileInsertionInfo = false;
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

				p.fUseProfileInsertionInfo = true;
				p.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
				p.ubQuoteActionID          = s->ubQuoteActionID;
				p.ubQuoteRecord            = s->ubQuoteActionID;
			}
		}
		else
		{
			p.fUseProfileInsertionInfo = false;
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
		p.fUseProfileInsertionInfo = true;
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

		CFOR_EACH_PLAYER_IN_GROUP(pPlayer, pGroup)
		{
			// Do morale hit...
			// CC look here!
			// pPlayer->pSoldier
		}
	}
}


UINT GetWorldSector()
{
	if (!gWorldSector.IsValid()) return NO_SECTOR;
	return gWorldSector.AsByte();
}

