#include "Overhead.h"
#include "Creature_Spreading.h"
#include "Campaign_Types.h"
#include "Queen_Command.h"
#include "Strategic_Movement.h"
#include "Game_Event_Hook.h"
#include "GameSettings.h"
#include "Random.h"
#include "Soldier_Init_List.h"
#include "Lighting.h"
#include "StrategicMap.h"
#include "Game_Clock.h"
#include "Strategic_Mines.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Strategic.h"
#include "JAScreens.h"
#include "Town_Militia.h"
#include "Strategic_Town_Loyalty.h"
#include "PreBattle_Interface.h"
#include "Map_Edgepoints.h"
#include "Animation_Data.h"
#include "OppList.h"
#include "Meanwhile.h"
#include "Map_Information.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "MineModel.h"
#include "CreatureLairModel.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "MineModel.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

//GAME BALANCING DEFINITIONS FOR CREATURE SPREADING
//Hopefully, adjusting these following definitions will ease the balancing of the
//creature spreading.
//The one note here is that for any definitions that have a XXX_BONUS at the end of a definition,
//it gets added on to it's counterpart via:
//								XXX_VALUE + Random( 1 + XXX_BONUS )

//This is how often the creatures spread, once the quest begins.  The smaller the gap,
//the faster the creatures will advance.  This is also directly related to the reproduction
//rates which are applied each time the creatures spread.
#define EASY_SPREAD_TIME_IN_MINUTES		510	//easy spreads every 8.5 hours
#define NORMAL_SPREAD_TIME_IN_MINUTES		450	//normal spreads every 7.5 hours
#define HARD_SPREAD_TIME_IN_MINUTES		390	//hard spreads every 6.5 hours

//Once the queen is added to the game, we can instantly let her spread x number of times
//to give her a head start.  This can also be a useful tool for having slow reproduction rates
//but quicker head start to compensate to make the creatures less aggressive overall.
#define EASY_QUEEN_INIT_BONUS_SPREADS		1
#define NORMAL_QUEEN_INIT_BONUS_SPREADS		2
#define HARD_QUEEN_INIT_BONUS_SPREADS		3

//This value modifies the chance to populate a given sector.  This is different from the previous definition.
//This value gets applied to a potentially complicated formula, using the creature habitat to modify
//chance to populate, along with factoring in the relative distance to the hive range (to promote deeper lair
//population increases), etc.  I would recommend not tweaking the value too much in either direction from
//zero due to the fact that this can greatly effect spread times and maximum populations.  Basically, if the
//creatures are spreading too quickly, increase the value, otherwise decrease it to a negative value
#define EASY_POPULATION_MODIFIER		0
#define NORMAL_POPULATION_MODIFIER		0
#define HARD_POPULATION_MODIFIER		0

//Augments the chance that the creatures will attack a town.  The conditions for attacking a town
//are based strictly on the occupation of the creatures in each of the four mine exits.  For each creature
//there is a base chance of 10% that the creatures will feed sometime during the night.
#define EASY_CREATURE_TOWN_AGGRESSIVENESS	-10
#define NORMAL_CREATURE_TOWN_AGGRESSIVENESS	0
#define HARD_CREATURE_TOWN_AGGRESSIVENESS	10


//This is how many creatures the queen produces for each cycle of spreading.  The higher
//the numbers the faster the creatures will advance.
#define EASY_QUEEN_REPRODUCTION_BASE		6 //6-7
#define EASY_QUEEN_REPRODUCTION_BONUS		1
#define NORMAL_QUEEN_REPRODUCTION_BASE		7 //7-9
#define NORMAL_QUEEN_REPRODUCTION_BONUS		2
#define HARD_QUEEN_REPRODUCTION_BASE		9 //9-12
#define HARD_QUEEN_REPRODUCTION_BONUS		3

//When either in a cave level with blue lights or there is a creature presence, then
//we override the normal music with the creature music.  The conditions are maintained
//inside the function PrepareCreaturesForBattle() in this module.
BOOLEAN gfUseCreatureMusic = FALSE;
BOOLEAN gfCreatureMeanwhileScenePlayed = FALSE;

struct CREATURE_DIRECTIVE
{
	CREATURE_DIRECTIVE*     next;
	UNDERGROUND_SECTORINFO *pLevel;
};

CREATURE_DIRECTIVE *gLair;
const CreatureLairModel* gLairModel;

INT32 giHabitatedDistance = 0;
INT32 giPopulationModifier = 0;
INT32 giLairID = 0;
INT32 giDestroyedLairID = 0;

//various information required for keeping track of the battle sector involved for
//prebattle interface, autoresolve, etc.
INT16 gsCreatureInsertionCode = 0;
INT16 gsCreatureInsertionGridNo = 0;
UINT8 gubNumCreaturesAttackingTown = 0;
UINT8 gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;
UINT8 gubSectorIDOfCreatureAttack = 0;


static CREATURE_DIRECTIVE* NewDirective(UINT8 ubSectorID, UINT8 ubSectorZ, UINT8 ubCreatureHabitat)
{
	CREATURE_DIRECTIVE* const curr = new CREATURE_DIRECTIVE{};
	SGPSector ubSector = SGPSector::FromSectorID(ubSectorID, ubSectorZ);
	curr->pLevel = FindUnderGroundSector(ubSector);
	if( !curr->pLevel )
	{
		SLOGA("Could not find underground sector node ({}) that should exist.", ubSector);
		delete curr;
		return 0;
	}

	curr->pLevel->ubCreatureHabitat = ubCreatureHabitat;
	curr->next = NULL;
	return curr;
}

static void InitCreatureLair(const CreatureLairModel* lairModel)
{
	gLairModel = lairModel;

	CREATURE_DIRECTIVE* curr = NULL;
	giLairID = lairModel->lairId;

	//initialize the linked list of lairs
	for (auto const& sec : lairModel->lairSectors)
	{
		auto next = NewDirective(sec.sectorId, sec.sectorLevel, sec.habitatType);
		if (sec.habitatType == QUEEN_LAIR && !next->pLevel->ubNumCreatures)
		{
			next->pLevel->ubNumCreatures = 1;	//for the queen.
		}

		if (curr == NULL)
		{ // first node, set gLair to the start of list
			gLair = next;
		}
		else
		{ // append to list
			curr->next = next;
		}
		curr = next;
	}
}

static bool IsMineInfestible(const MineModel* mine)
{ // If neither head miner was attacked, ore will/has run out nor enemy controlled
	UINT8 id = mine->mineId;
	MINE_STATUS_TYPE const& m = gMineStatus[id];
	return  !m.fAttackedHeadMiner       &&
		m.uiOreRunningOutPoint == 0 &&
		!StrategicMap[SGPSector(mine->entranceSector).AsStrategicIndex()].fEnemyControlled;
}


void InitCreatureQuest()
{
	INT32 i=-1;
	UINT8 ubChosenMineId;
	INT32 iRandom;
	INT32 iNumMinesInfestible;
	std::vector<UINT8> ubMinesInfestible;

	if( giLairID )
	{
		return; //already active!
	}

	if( !gfCreatureMeanwhileScenePlayed )
	{
		//Start the meanwhile scene for the queen ordering the release of the creatures.
		HandleCreatureRelease();
		gfCreatureMeanwhileScenePlayed = TRUE;
	}

	giHabitatedDistance = 0;
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			giPopulationModifier = EASY_POPULATION_MODIFIER;
			break;
		case DIF_LEVEL_MEDIUM:
			giPopulationModifier = NORMAL_POPULATION_MODIFIER;
			break;
		case DIF_LEVEL_HARD:
			giPopulationModifier = HARD_POPULATION_MODIFIER;
			break;
	}

	/* Determine which of the four mines are infectible by creatures. Infectible
	 * mines are those that are player controlled and unlimited. We don't want the
	 * creatures to infect the mine that runs out. */
	for (auto lair : GCM->getCreatureLairs())
	{
		auto mine = GCM->getMine(lair->associatedMineId);
		if (IsMineInfestible(mine))
		{
			ubMinesInfestible.push_back(mine->mineId);
		}
	}

	iNumMinesInfestible = ubMinesInfestible.size();
	if( !iNumMinesInfestible )
	{
		return;
	}

	//Choose one of the infectible mines randomly
	iRandom = Random( iNumMinesInfestible );
	ubChosenMineId = ubMinesInfestible[iRandom];

	//Now, choose a start location for the queen.
	auto lairModel = GCM->getCreatureLairByMineId(ubChosenMineId);
	InitCreatureLair(lairModel);

	// enable the lair entrance
	UINT8 entranceSector = lairModel->entranceSector;
	UNDERGROUND_SECTORINFO* lairEntrance = FindUnderGroundSector(SGPSector::FromSectorID(entranceSector, lairModel->entranceSectorLevel));
	if (lairEntrance == NULL)
	{
		throw std::runtime_error("Lair entrance sector is not defined as an underground sector");
	}
	lairEntrance->uiFlags |= SF_PENDING_ALTERNATE_MAP;

	//Now determine how often we will spread the creatures.
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			i = EASY_QUEEN_INIT_BONUS_SPREADS;
			AddPeriodStrategicEvent( EVENT_CREATURE_SPREAD, EASY_SPREAD_TIME_IN_MINUTES, 0 );
			break;
		case DIF_LEVEL_MEDIUM:
			i = NORMAL_QUEEN_INIT_BONUS_SPREADS;
			AddPeriodStrategicEvent( EVENT_CREATURE_SPREAD, NORMAL_SPREAD_TIME_IN_MINUTES, 0 );
			break;
		case DIF_LEVEL_HARD:
			i = HARD_QUEEN_INIT_BONUS_SPREADS;
			AddPeriodStrategicEvent( EVENT_CREATURE_SPREAD, HARD_SPREAD_TIME_IN_MINUTES, 0 );
			break;
	}

	//Set things up so that the creatures can plan attacks on helpless miners and civilians while
	//they are sleeping.  They do their planning at 10PM every day, and decide to attack sometime
	//during the night.
	AddEveryDayStrategicEvent( EVENT_CREATURE_NIGHT_PLANNING, 1320, 0 );

	//Got to give the queen some early protection, so do some creature spreading.
	while( i-- )
	{ //# times spread is based on difficulty, and the values in the defines.
		SpreadCreatures();
	}
}


static void AddCreatureToNode(CREATURE_DIRECTIVE* node)
{
	node->pLevel->ubNumCreatures++;

	if( node->pLevel->uiFlags & SF_PENDING_ALTERNATE_MAP )
	{ //there is an alternate map meaning that there is a dynamic opening.  From now on
		//we substitute this map.
		node->pLevel->uiFlags &= ~SF_PENDING_ALTERNATE_MAP;
		node->pLevel->uiFlags |= SF_USE_ALTERNATE_MAP;
	}
}


static BOOLEAN PlaceNewCreature(CREATURE_DIRECTIVE* node, INT32 iDistance)
{
	if( !node )
		return FALSE;
	//check to see if the creatures are permitted to spread into certain areas.  There are 4 mines (human perspective), and
	//creatures won't spread to them until the player controls them.  Additionally, if the player has recently cleared the
	//mine, then temporarily prevent the spreading of creatures.

	if( giHabitatedDistance == iDistance )
	{	//FRONT-LINE CONDITIONS -- consider expansion or frontline fortification.  The formulae used
		//in this sector are geared towards outer expansion.
		//we have reached the distance limitation for the spreading.  We will determine if
		//the area is populated enough to spread further.  The minimum population must be 4 before
		//spreading is even considered.
		if( node->pLevel->ubNumCreatures*10 - 10 <= (INT32)Random( 60 ) )
		{
			// x<=1 100%
			// x==2  83%
			// x==3  67%
			// x==4  50%
			// x==5  33%
			// x==6  17%
			// x>=7   0%
			AddCreatureToNode( node );
			return TRUE;
		}
	}
	else if( giHabitatedDistance > iDistance )
	{ //we are within the "safe" habitated area of the creature's area of influence.  The chance of
		//increasing the population inside this sector depends on how deep we are within the sector.
		if( node->pLevel->ubNumCreatures < MAX_STRATEGIC_TEAM_SIZE ||
			(node->pLevel->ubNumCreatures < 32 && node->pLevel->ubCreatureHabitat == QUEEN_LAIR) )
		{ //there is ALWAYS a chance to habitate an interior sector, though the chances are slim for
			//highly occupied sectors.  This chance is modified by the type of area we are in.
			INT32 iAbsoluteMaxPopulation;
			INT32 iMaxPopulation=-1;
			INT32 iChanceToPopulate;
			switch( node->pLevel->ubCreatureHabitat )
			{
				case QUEEN_LAIR: //Defend the queen bonus
					iAbsoluteMaxPopulation = 32;
					break;
				case LAIR: //Smaller defend the queen bonus
					iAbsoluteMaxPopulation = 18;
					break;
				case LAIR_ENTRANCE: //Smallest defend the queen bonus
					iAbsoluteMaxPopulation = 15;
					break;
				case INNER_MINE: //neg bonus -- actually promotes expansion over population, and decrease max pop here.
					iAbsoluteMaxPopulation = 12;
					break;
				case OUTER_MINE: //neg bonus -- actually promotes expansion over population, and decrease max pop here.
					iAbsoluteMaxPopulation = 10;
					break;
				case FEEDING_GROUNDS: //get free food bonus!  yummy humans :)
					iAbsoluteMaxPopulation = 15;
					break;
				case MINE_EXIT:	//close access to humans (don't want to overwhelm them)
					iAbsoluteMaxPopulation = 10;
					break;
				default:
					SLOGA("PlaceNewCreature: invalid habitat type");
					return FALSE;
			}

			switch( gGameOptions.ubDifficultyLevel )
			{
				case DIF_LEVEL_EASY: //50%
					iAbsoluteMaxPopulation /= 2; //Half
					break;
				case DIF_LEVEL_MEDIUM: //80%
					iAbsoluteMaxPopulation = iAbsoluteMaxPopulation * 4 / 5;
					break;
				case DIF_LEVEL_HARD: //100%
					break;
			}

			//Calculate the desired max population percentage based purely on current distant to creature range.
			//The closer we are to the lair, the closer this value will be to 100.
			iMaxPopulation = 100 - iDistance * 100 / giHabitatedDistance;
			iMaxPopulation = std::max(iMaxPopulation, 25);
			//Now, convert the previous value into a numeric population.
			iMaxPopulation = iAbsoluteMaxPopulation * iMaxPopulation / 100;
			iMaxPopulation = std::max(iMaxPopulation, 4);


			//The chance to populate a sector is higher for lower populations.  This is calculated on
			//the ratio of current population to the max population.
			iChanceToPopulate = 100 - node->pLevel->ubNumCreatures * 100 / iMaxPopulation;

			if( !node->pLevel->ubNumCreatures || (iChanceToPopulate > (INT32)Random( 100 )
					&& iMaxPopulation > node->pLevel->ubNumCreatures) )
			{
				AddCreatureToNode( node );
				return TRUE;
			}
		}
	}
	else
	{ //we are in a new area, so we will populate it
		AddCreatureToNode( node );
		giHabitatedDistance++;
		return TRUE;
	}
	if( PlaceNewCreature( node->next, iDistance + 1 ) )
		return TRUE;
	return FALSE;
}

void SpreadCreatures()
{
	UINT16 usNewCreatures=0;

	if (giLairID == -1) return;

	//queen just produced a litter of creature larvae.  Let's do some spreading now.
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			usNewCreatures = (UINT16)(EASY_QUEEN_REPRODUCTION_BASE + Random( 1 + EASY_QUEEN_REPRODUCTION_BONUS ));
			break;
		case DIF_LEVEL_MEDIUM:
			usNewCreatures = (UINT16)(NORMAL_QUEEN_REPRODUCTION_BASE + Random( 1 + NORMAL_QUEEN_REPRODUCTION_BONUS ));
			break;
		case DIF_LEVEL_HARD:
			usNewCreatures = (UINT16)(HARD_QUEEN_REPRODUCTION_BASE + Random( 1 + HARD_QUEEN_REPRODUCTION_BONUS ));
			break;
	}

	while( usNewCreatures-- )
	{
		//Note, this function can and will fail if the population gets dense.  This is a necessary
		//feature.  Otherwise, the queen would fill all the cave levels with MAX_STRATEGIC_TEAM_SIZE monsters, and that would
		//be bad.
		PlaceNewCreature( gLair, 0 );
	}
}


static void AddCreaturesToBattle(UINT8 n_young_males, UINT8 n_young_females, UINT8 n_adult_males, UINT8 n_adult_females)
{
	INT16 const insertion_code = gsCreatureInsertionCode;
	UINT8       desired_direction;
	switch (insertion_code)
	{
		case INSERTION_CODE_NORTH:  desired_direction = SOUTHEAST; break;
		case INSERTION_CODE_EAST:   desired_direction = SOUTHWEST; break;
		case INSERTION_CODE_SOUTH:  desired_direction = NORTHWEST; break;
		case INSERTION_CODE_WEST:   desired_direction = NORTHEAST; break;
		case INSERTION_CODE_GRIDNO: desired_direction = 0;         break;
		default: throw std::logic_error("Invalid direction passed to AddCreaturesToBattle()");
	}

	MAPEDGEPOINTINFO edgepoint_info;
	if (insertion_code != INSERTION_CODE_GRIDNO)
	{
		ChooseMapEdgepoints(&edgepoint_info, insertion_code, n_young_males + n_young_females + n_adult_males + n_adult_females);
	}

	std::vector<SoldierBodyType> bodies;
	bodies.insert(bodies.end(), n_young_males, YAM_MONSTER);
	bodies.insert(bodies.end(), n_young_females, YAF_MONSTER);
	bodies.insert(bodies.end(), n_adult_males, AM_MONSTER);
	bodies.insert(bodies.end(), n_adult_females, ADULTFEMALEMONSTER);
	std::shuffle(bodies.begin(), bodies.end(), gRandomEngine);

	UINT8 slot = 0;
	for (SoldierBodyType const body : bodies)
	{
		SOLDIERTYPE* const s = TacticalCreateCreature(body);
		s->bHunting                 = TRUE;
		s->ubInsertionDirection     = desired_direction;
		s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		if (insertion_code == INSERTION_CODE_GRIDNO)
		{
			s->usStrategicInsertionData = gsCreatureInsertionGridNo;
		}
		else if (slot < edgepoint_info.ubNumPoints)
		{ // Use an edgepoint
			s->usStrategicInsertionData = edgepoint_info.sGridNo[slot++];
		}
		else
		{ // No edgepoints left, so put him at the entrypoint
			s->ubStrategicInsertionCode = insertion_code;
		}
		UpdateMercInSector(*s, gWorldSector);
	}

	gsCreatureInsertionCode      = 0;
	gsCreatureInsertionGridNo    = 0;
	gubNumCreaturesAttackingTown = 0;
	gubCreatureBattleCode        = CREATURE_BATTLE_CODE_NONE;
	gubSectorIDOfCreatureAttack  = 0;
	AllTeamsLookForAll(FALSE);
}


static void ChooseTownSectorToAttack(UINT8 ubSectorID, BOOLEAN fSpecificSector)
{
	if (gLairModel == NULL)
	{
		SLOGA("gLairModel is NULL. Something wrong!");
		return;
	}

	// determine town sector to attack
	auto const * const attackDetails = (fSpecificSector) ?
		gLairModel->getTownAttackDetails(ubSectorID) : // attack the given sector
		gLairModel->chooseTownSectorToAttack()         // pick a sector to attack
	;
	if (!attackDetails)
	{
		SLOGA("ChooseTownSectorToAttack: invalid SectorID");
		return;
	}

	// determine how the enemies enter the sector
	gubSectorIDOfCreatureAttack = attackDetails->sectorId;
	gsCreatureInsertionCode = attackDetails->insertionCode;
	if (gsCreatureInsertionCode == INSERTION_CODE_GRIDNO)
	{
		gsCreatureInsertionGridNo = attackDetails->insertionGridNo;
	}
}

void CreatureAttackTown(UINT8 ubSectorID, BOOLEAN fSpecificSector)
{ //This is the launching point of the creature attack.
	UNDERGROUND_SECTORINFO *pSector;

	if( gfWorldLoaded && gTacticalStatus.fEnemyInSector )
	{ //Battle currently in progress, repost the event
		AddStrategicEvent( EVENT_CREATURE_ATTACK, GetWorldTotalMin() + Random( 10 ), ubSectorID );
		return;
	}

	gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;
	SGPSector ubSector = SGPSector::FromSectorID(ubSectorID, 1);

	if (!fSpecificSector)
	{
		//Record the number of creatures in the sector.
		pSector = FindUnderGroundSector(ubSector);
		if( !pSector )
		{
			CreatureAttackTown(ubSectorID, TRUE);
			return;
		}
		gubNumCreaturesAttackingTown = pSector->ubNumCreatures;
		if( !gubNumCreaturesAttackingTown )
		{
			CreatureAttackTown(ubSectorID, TRUE);
			return;
		}

		pSector->ubNumCreatures = 0;

		//Choose one of the town sectors to attack.  Sectors closer to
		//the mine entrance have a greater chance of being chosen.
		ChooseTownSectorToAttack( ubSectorID, FALSE );
		ubSector = SGPSector(gubSectorIDOfCreatureAttack);
	}
	else
	{
		ChooseTownSectorToAttack(ubSectorID, TRUE);
		gubNumCreaturesAttackingTown = 5;
	}

	//Now that the sector has been chosen, attack it!
	SGPSector sector(ubSector.x, ubSector.y, 0);
	if (PlayerGroupsInSector(sector))
	{ //we have players in the sector
		if (sector == gWorldSector)
		{ //This is the currently loaded sector.  All we have to do is change the music and insert
			//the creatures tactically.
			if( guiCurrentScreen == GAME_SCREEN )
			{
				gubCreatureBattleCode = CREATURE_BATTLE_CODE_TACTICALLYADD;
			}
			else
			{
				gubCreatureBattleCode = CREATURE_BATTLE_CODE_PREBATTLEINTERFACE;
			}
		}
		else
		{
			gubCreatureBattleCode = CREATURE_BATTLE_CODE_PREBATTLEINTERFACE;
		}
	}
	else if (CountAllMilitiaInSector(sector))
	{ //we have militia in the sector
		gubCreatureBattleCode = CREATURE_BATTLE_CODE_AUTORESOLVE;
	}
	else if (!StrategicMap[sector.AsStrategicIndex()].fEnemyControlled)
	{ //player controlled sector -- eat some civilians
		AdjustLoyaltyForCivsEatenByMonsters(sector, gubNumCreaturesAttackingTown);
		SectorInfo[ ubSectorID ].ubDayOfLastCreatureAttack = GetWorldDay();
		return;
	}
	else
	{ //enemy controlled sectors don't get attacked.
		return;
	}

	SectorInfo[ ubSectorID ].ubDayOfLastCreatureAttack = GetWorldDay();
	switch( gubCreatureBattleCode )
	{
		case CREATURE_BATTLE_CODE_AUTORESOLVE:
			gfAutomaticallyStartAutoResolve = TRUE;
			/* FALLTHROUGH */
		case CREATURE_BATTLE_CODE_PREBATTLEINTERFACE:
			InitPreBattleInterface(0, true);
			break;
		case CREATURE_BATTLE_CODE_TACTICALLYADD:
			PrepareCreaturesForBattle();
			break;
	}
	InterruptTime();
	PauseGame();
	LockPauseState(LOCK_PAUSE_CREATURE_ATTACK);
}


static void DeleteDirectiveNode(CREATURE_DIRECTIVE** node)
{
	if( (*node)->next )
		DeleteDirectiveNode( &((*node)->next) );
	delete *node;
	*node = NULL;
}

//Recursively delete all nodes (from the top down).
void DeleteCreatureDirectives()
{
	if( gLair )
		DeleteDirectiveNode( &gLair );
	giLairID = 0;
}

void EndCreatureQuest()
{
	CREATURE_DIRECTIVE *curr;
	UNDERGROUND_SECTORINFO *pSector;
	INT32 i;

	//By setting the lairID to -1, when it comes time to spread creatures,
	//They will get subtracted instead.
	giDestroyedLairID = giLairID;
	giLairID = -1;

	//Also nuke all of the creatures in all of the other mine sectors.  This
	//is keyed on the fact that the queen monster is killed.
	curr = gLair;
	if( curr )
	{ //skip first node (there could be other creatures around.
		curr = curr->next;
	}
	while( curr )
	{
		curr->pLevel->ubNumCreatures = 0;
		curr = curr->next;
	}

	//Remove the creatures that are trapped underneath Tixa
	pSector = FindUnderGroundSector(SGPSector(9, 10, 2));
	if( pSector )
	{
		pSector->ubNumCreatures = 0;
	}

	//Also find and nuke all creatures on any surface levels!!!
	//KM: Sept 3, 1999 patch
	for( i = 0; i < 255; i++ )
	{
		SectorInfo[ i ].ubNumCreatures = 0;
		SectorInfo[ i ].ubCreaturesInBattle = 0;
	}
}


static UINT8 CreaturesInUndergroundSector(UINT8 ubSectorID, UINT8 ubSectorZ)
{
	UNDERGROUND_SECTORINFO *pSector;
	SGPSector ubSector = SGPSector::FromSectorID(ubSectorID, ubSectorZ);
	pSector = FindUnderGroundSector(ubSector);
	if( pSector )
		return pSector->ubNumCreatures;
	return 0;
}

BOOLEAN MineClearOfMonsters( UINT8 ubMineIndex )
{
	Assert(ubMineIndex < MAX_NUMBER_OF_MINES);

	if( !gMineStatus[ ubMineIndex ].fPrevInvadedByMonsters )
	{
		auto mine = GCM->getMine(ubMineIndex);
		if (mine == NULL)
		{
			SLOGE("Attempting to check if mine is clear but mine index is invalid ({}).", ubMineIndex);
			return true;
		}
		for (auto const& sector : mine->mineSectors)
		{
			if (CreaturesInUndergroundSector(sector[0], sector[1]))
			{
				return false;
			}
		}
	}
	else
	{ //mine was previously invaded by creatures.  Don't allow mine production until queen is dead.
		if( giLairID != -1 )
		{
			return FALSE;
		}
	}
	return TRUE;
}

namespace {
using Weights = std::array<UINT8, 6>;
constexpr std::array CreatureWeightsByHabitat
{
	//        Larvae, Infant, YoungMale, YoungFemale, AdultMale, AdultFemale
	// QUEEN_LAIR
	Weights{      20,     40,         0,           0,        30,          10 },
	// LAIR
	Weights{      15,     35,        10,           5,        25,          10 },
	// LAIR_ENTRANCE
	Weights{       0,     15,        30,          10,        35,          10 },
	// INNER_MINE
	Weights{       0,      0,        20,          40,        10,          30 },
	// OUTER_MINE
	Weights{       0,      0,        10,          65,         5,          20 },
	// FEEDING GROUNDS (currently set in Editor, not randomly generated)
	Weights{      25,     25,        25,          25,         0,           0 },
	// MINE_EXIT
	Weights{       0,      0,        10,          65,         5,          20 },
};
static_assert(CreatureWeightsByHabitat.size() == CreatureHabitat::MINE_EXIT + 1);


Weights DistributeCreaturesByWeight(UINT8 const numOfCreatures,
	Weights const& weights)
{
	std::discrete_distribution<size_t> distribution{ weights.begin(), weights.end() };
	Weights result{};

	for (int i{ 0 }; i != numOfCreatures; ++i)
	{
		++result[distribution(gRandomEngine)];
	}

	return result;
}
}

void DetermineCreatureTownComposition(UINT8 ubNumCreatures,
					UINT8 *pubNumYoungMales, UINT8 *pubNumYoungFemales,
					UINT8 *pubNumAdultMales, UINT8 *pubNumAdultFemales)
{
	auto const distributedCreatures{ DistributeCreaturesByWeight(
		ubNumCreatures, CreatureWeightsByHabitat[CreatureHabitat::MINE_EXIT]) };

	// There are no larvae or infants in town attacks.
	*pubNumYoungMales   = distributedCreatures[2];
	*pubNumYoungFemales = distributedCreatures[3];
	*pubNumAdultMales   = distributedCreatures[4];
	*pubNumAdultFemales = distributedCreatures[5];
}

void DetermineCreatureTownCompositionBasedOnTacticalInformation(UINT8 *pubNumCreatures,
								UINT8 *pubNumYoungMales, UINT8 *pubNumYoungFemales,
								UINT8 *pubNumAdultMales, UINT8 *pubNumAdultFemales)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[gWorldSector.AsByte()];
	*pubNumCreatures = 0;
	pSector->ubNumCreatures = 0;
	pSector->ubCreaturesInBattle = 0;
	CFOR_EACH_IN_TEAM(s, CREATURE_TEAM)
	{
		if (s->bInSector && s->bLife)
		{
			switch (s->ubBodyType)
			{
				case ADULTFEMALEMONSTER:
					(*pubNumCreatures)++;
					(*pubNumAdultFemales)++;
					break;
				case AM_MONSTER:
					(*pubNumCreatures)++;
					(*pubNumAdultMales)++;
					break;
				case YAF_MONSTER:
					(*pubNumCreatures)++;
					(*pubNumYoungFemales)++;
					break;
				case YAM_MONSTER:
					(*pubNumCreatures)++;
					(*pubNumYoungMales)++;
					break;
			}
		}
	}
}



BOOLEAN PrepareCreaturesForBattle()
{
	UNDERGROUND_SECTORINFO *pSector;
	UINT8 ubCreatureHabitat;
	UINT8 ubNumCreatures;

	if( !gubCreatureBattleCode )
	{
		//By default, we only play creature music in the cave levels (the creature levels all consistently
		//have blue lights while human occupied mines have red lights.  We always play creature music
		//when creatures are in the level.
		gfUseCreatureMusic = LightGetColor()->b != 0;

		if (!gWorldSector.z)
			return FALSE;  //Creatures don't attack overworld with this battle code.
		pSector = FindUnderGroundSector(gWorldSector);
		if( !pSector )
		{
			return FALSE;
		}
		if( !pSector->ubNumCreatures )
		{
			return FALSE;
		}
		gfUseCreatureMusic = TRUE; //creatures are here, so play creature music
		ubCreatureHabitat = pSector->ubCreatureHabitat;
		ubNumCreatures = pSector->ubNumCreatures;
	}
	else
	{ //creatures are attacking a town sector
		gfUseCreatureMusic = TRUE;
		SetMusicMode( MUSIC_TACTICAL_NOTHING );
		ubCreatureHabitat = MINE_EXIT;
		ubNumCreatures = gubNumCreaturesAttackingTown;
	}

	bool const fQueen{ ubCreatureHabitat == QUEEN_LAIR };
	if (fQueen)
	{
		// Subtract queen but ensure at least one other creature.
		ubNumCreatures = std::max(1U, ubNumCreatures - 1U);
	}

	auto const [
		ubNumLarvae,
		ubNumInfants,
		ubNumYoungMales,
		ubNumYoungFemales,
		ubNumAdultMales,
		ubNumAdultFemales ] { DistributeCreaturesByWeight(
			ubNumCreatures, CreatureWeightsByHabitat[ubCreatureHabitat]) };

	if (gWorldSector.z)
	{
		UNDERGROUND_SECTORINFO *pUndergroundSector;
		pUndergroundSector = FindUnderGroundSector(gWorldSector);
		if( !pUndergroundSector )
		{ //No info?!!!!!
			SLOGA("Please report underground sector you are in or going to and send save if possible." );
			return FALSE;
		}
		pUndergroundSector->ubCreaturesInBattle = pUndergroundSector->ubNumCreatures;
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[gWorldSector.AsByte()];
		pSector->ubNumCreatures = ubNumCreatures;
		pSector->ubCreaturesInBattle = ubNumCreatures;
	}

	switch( gubCreatureBattleCode )
	{
		case CREATURE_BATTLE_CODE_NONE: //in the mines
			AddSoldierInitListCreatures( fQueen, ubNumLarvae, ubNumInfants,
				ubNumYoungMales, ubNumYoungFemales, ubNumAdultMales, ubNumAdultFemales );
			break;
		case CREATURE_BATTLE_CODE_TACTICALLYADD: //creature attacking a town sector
		case CREATURE_BATTLE_CODE_PREBATTLEINTERFACE:
			AddCreaturesToBattle( ubNumYoungMales, ubNumYoungFemales, ubNumAdultMales, ubNumAdultFemales );
			break;
		case CREATURE_BATTLE_CODE_AUTORESOLVE:
			return FALSE;
	}
	return TRUE;
}

void CreatureNightPlanning()
{ //Check the populations of the mine exits, and factor a chance for them to attack at night.
	for (auto lair: GCM->getCreatureLairs())
	{
		auto mine = GCM->getMine(lair->associatedMineId);
		UINT8 ubNumCreatures = CreaturesInUndergroundSector(mine->entranceSector, 1);
		if (ubNumCreatures > 1 && ubNumCreatures * 10 > (INT32)PreRandom(100))
		{ //10% chance for each creature to decide it's time to attack.
			AddStrategicEvent(EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom(429), mine->entranceSector);
		}
	}
}


void CheckConditionsForTriggeringCreatureQuest()
{
	UINT8 ubValidMines = 0;
	if( !gGameOptions.fSciFi )
		return; //No scifi, no creatures...
	if( giLairID )
		return;	//Creature quest already begun

	//Count the number of "infectible mines" the player occupies
	for (auto lair : GCM->getCreatureLairs())
	{
		auto mine = GCM->getMine(lair->associatedMineId);
		auto sectorIndex = SGPSector(mine->entranceSector).AsStrategicIndex();
		if (!StrategicMap[sectorIndex].fEnemyControlled)
		{
			ubValidMines++;
		}
	}

	if( ubValidMines >= 3 )
	{
		InitCreatureQuest();
	}
}


void SaveCreatureDirectives(HWFILE const hFile)
{
	hFile->write(&giHabitatedDistance,  4);
	hFile->write(&giPopulationModifier, 4);
	hFile->write(&giLairID,             4);
	hFile->write(&gfUseCreatureMusic,   1);
	hFile->write(&giDestroyedLairID,    4);
}


void LoadCreatureDirectives(HWFILE const hFile, UINT32 const uiSavedGameVersion)
{
	hFile->read(&giHabitatedDistance,  4);
	hFile->read(&giPopulationModifier, 4);
	hFile->read(&giLairID,             4);
	hFile->read(&gfUseCreatureMusic,   1);

	if( uiSavedGameVersion >= 82 )
	{
		hFile->read(&giDestroyedLairID, 4);
	}
	else
	{
		giDestroyedLairID = 0;
	}

	switch( giLairID )
	{
	case -1: //creature quest finished -- it's okay
	case 0:  //lair doesn't exist yet -- it's okay
		break;
	default:
		auto lair = GCM->getCreatureLair(giLairID);
		if (!lair)
		{
			SLOGE("Invalid restoration of creature lair ID of {}.  Save game potentially hosed.", giLairID);
			break;
		}
		InitCreatureLair(lair);
		break;
	}
}


BOOLEAN PlayerGroupIsInACreatureInfestedMine()
{
	if( giLairID <= 0 )
	{ //Creature quest inactive
		return FALSE;
	}

	//Lair is active, so look for live soldier in any creature level
	CREATURE_DIRECTIVE *curr = gLair;
	while( curr )
	{
		//Loop through all the creature directives (mine sectors that are infectible) and
		//see if players are there.
		CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
		{
			if (pSoldier->bLife    != 0 &&
					pSoldier->sSector == curr->pLevel->ubSector &&
					!pSoldier->fBetweenSectors )
			{
				return TRUE;
			}
		}
		curr = curr->next;
	}

	//Lair is active, but no mercs are in these sectors
	return FALSE;
}


bool GetWarpOutOfMineCodes(SGPSector& sector, INT16* const insertion_grid_no)
{
	if (!gfWorldLoaded)      return false;
	if (gWorldSector.z == 0) return false;

	auto lair = gLairModel;
	if (lair == NULL && giLairID == -1)
	{
		// Quest is finished
		lair = GCM->getCreatureLair(giDestroyedLairID);
	}

	// Now make sure the mercs are in the previously infested mine
	if (lair && lair->isSectorInLair(gWorldSector))
	{
		sector = SGPSector(lair->warpExitSector);
		*insertion_grid_no = lair->warpExitGridNo;

		return true;
	}
	return false;
}

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#include <numeric>

TEST(CreatureSpreading, CreatureWeightsAddUpTo100Percent)
{
	for (Weights const& w : CreatureWeightsByHabitat)
	{
		EXPECT_EQ(100, std::accumulate(w.begin(), w.end(), 0));
	}
}

TEST(CreatureSpreading, DetermineCreatureTownCompositionProducesCorrectDistribution)
{
	gRandomEngine.seed(12345);

	UINT8 ym, yf, am, af;

	ym = yf = am = af = 0;
	DetermineCreatureTownComposition(250, &ym, &yf, &am, &af);
	EXPECT_EQ(250, ym + yf + am + af);
	EXPECT_EQ(26, ym);  // 250 * 10% = 25
	EXPECT_EQ(152, yf); // 250 * 65% = 162.5
	EXPECT_EQ(16, am);  // 250 *  5% = 12.5
	EXPECT_EQ(56, af);  // 250 * 20% = 50

	// Again to verify that the distribution is different.
	ym = yf = am = af = 0;
	DetermineCreatureTownComposition(250, &ym, &yf, &am, &af);
	EXPECT_EQ(250, ym + yf + am + af);
	EXPECT_EQ(24, ym);
	EXPECT_EQ(166, yf);
	EXPECT_EQ(11, am);
	EXPECT_EQ(49, af);
}
#endif
