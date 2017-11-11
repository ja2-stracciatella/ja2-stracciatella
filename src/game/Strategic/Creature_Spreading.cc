#include <stdexcept>

#include "Campaign_Init.h"
#include "Overhead.h"
#include "FileMan.h"
#include "Creature_Spreading.h"
#include "Campaign_Types.h"
#include "Queen_Command.h"
#include "Strategic_Movement.h"
#include "Game_Event_Hook.h"
#include "GameSettings.h"
#include "Random.h"
#include "Message.h"
#include "Font_Control.h"
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
#include "Strategic_AI.h"
#include "Map_Information.h"
#include "MemMan.h"
#include "Debug.h"
#include "ScreenIDs.h"

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
enum
{
	QUEEN_LAIR,		//where the queen lives.  Highly protected
	LAIR,			//part of the queen's lair -- lots of babies and defending mothers
	LAIR_ENTRANCE,		//where the creatures access the mine.
	INNER_MINE,		//parts of the mines that aren't close to the outside world
	OUTER_MINE,		//area's where miners work, close to towns, creatures love to eat :)
	FEEDING_GROUNDS,	//creatures love to populate these sectors :)
	MINE_EXIT,		//the area that creatures can initiate town attacks if lots of monsters.
};

struct CREATURE_DIRECTIVE
{
	CREATURE_DIRECTIVE*     next;
	UNDERGROUND_SECTORINFO *pLevel;
};

CREATURE_DIRECTIVE *lair;
INT32 giHabitatedDistance = 0;
INT32 giPopulationModifier = 0;
INT32 giLairID = 0;
INT32 giDestroyedLairID = 0;

//various information required for keeping track of the battle sector involved for
//prebattle interface, autoresolve, etc.
INT16 gsCreatureInsertionCode = 0;
INT16 gsCreatureInsertionGridNo = 0;
UINT8 gubNumCreaturesAttackingTown = 0;
UINT8 gubYoungMalesAttackingTown = 0;
UINT8 gubYoungFemalesAttackingTown = 0;
UINT8 gubAdultMalesAttackingTown = 0;
UINT8 gubAdultFemalesAttackingTown = 0;
UINT8 gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;
UINT8 gubSectorIDOfCreatureAttack = 0;


static CREATURE_DIRECTIVE* NewDirective(UINT8 ubSectorID, UINT8 ubSectorZ, UINT8 ubCreatureHabitat)
{
	UINT8 ubSectorX, ubSectorY;
	CREATURE_DIRECTIVE* const curr = MALLOC(CREATURE_DIRECTIVE);
	ubSectorX = (UINT8)((ubSectorID % 16) + 1);
	ubSectorY = (UINT8)((ubSectorID / 16) + 1);
	curr->pLevel = FindUnderGroundSector( ubSectorX, ubSectorY, ubSectorZ );
	if( !curr->pLevel )
	{
		SLOGE(DEBUG_TAG_ASSERTS, "Could not find underground sector node (%c%db_%d) that should exist.",
			ubSectorY + 'A' - 1, ubSectorX, ubSectorZ);
		return 0;
	}

	curr->pLevel->ubCreatureHabitat = ubCreatureHabitat;
	Assert( curr->pLevel );
	curr->next = NULL;
	return curr;
}


static void InitLairDrassen(void)
{
	CREATURE_DIRECTIVE *curr;
	giLairID = 1;
	//initialize the linked list of lairs
	lair = NewDirective( SEC_F13, 3, QUEEN_LAIR );
	curr = lair;
	if( !curr->pLevel->ubNumCreatures )
	{
		curr->pLevel->ubNumCreatures = 1;	//for the queen.
	}
	curr->next = NewDirective( SEC_G13, 3, LAIR );
	curr = curr->next;
	curr->next = NewDirective( SEC_G13, 2, LAIR_ENTRANCE );
	curr = curr->next;
	curr->next = NewDirective( SEC_F13, 2, INNER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_E13, 2, INNER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_E13, 1, OUTER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_D13, 1, MINE_EXIT );
}


static void InitLairCambria(void)
{
	CREATURE_DIRECTIVE *curr;
	giLairID = 2;
	//initialize the linked list of lairs
	lair = NewDirective( SEC_J8, 3, QUEEN_LAIR );
	curr = lair;
	if( !curr->pLevel->ubNumCreatures )
	{
		curr->pLevel->ubNumCreatures = 1;	//for the queen.
	}
	curr->next = NewDirective( SEC_I8, 3, LAIR );
	curr = curr->next;
	curr->next = NewDirective( SEC_H8, 3, LAIR );
	curr = curr->next;
	curr->next = NewDirective( SEC_H8, 2, LAIR_ENTRANCE );
	curr = curr->next;
	curr->next = NewDirective( SEC_H9, 2, INNER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_H9, 1, OUTER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_H8, 1, MINE_EXIT );
}


static void InitLairAlma(void)
{
	CREATURE_DIRECTIVE *curr;
	giLairID = 3;
	//initialize the linked list of lairs
	lair = NewDirective( SEC_K13, 3, QUEEN_LAIR );
	curr = lair;
	if( !curr->pLevel->ubNumCreatures )
	{
		curr->pLevel->ubNumCreatures = 1;	//for the queen.
	}
	curr->next = NewDirective( SEC_J13, 3, LAIR );
	curr = curr->next;
	curr->next = NewDirective( SEC_J13, 2, LAIR_ENTRANCE );
	curr = curr->next;
	curr->next = NewDirective( SEC_J14, 2, INNER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_J14, 1, OUTER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_I14, 1, MINE_EXIT );
}


static void InitLairGrumm(void)
{
	CREATURE_DIRECTIVE *curr;
	giLairID = 4;
	//initialize the linked list of lairs
	lair = NewDirective( SEC_G4, 3, QUEEN_LAIR );
	curr = lair;
	if( !curr->pLevel->ubNumCreatures )
	{
		curr->pLevel->ubNumCreatures = 1;	//for the queen.
	}
	curr->next = NewDirective( SEC_H4, 3, LAIR );
	curr = curr->next;
	curr->next = NewDirective( SEC_H4, 2, LAIR_ENTRANCE );
	curr = curr->next;
	curr->next = NewDirective( SEC_H3, 2, INNER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_I3, 2, INNER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_I3, 1, OUTER_MINE );
	curr = curr->next;
	curr->next = NewDirective( SEC_H3, 1, MINE_EXIT );
}


static bool IsMineInfectible(MineID const id)
{ // If neither head miner was attacked, ore will/has run out nor enemy controlled
	MINE_STATUS_TYPE const& m = gMineStatus[id];
	return
		!m.fAttackedHeadMiner       &&
		m.uiOreRunningOutPoint == 0 &&
		!StrategicMap[SECTOR_INFO_TO_STRATEGIC_INDEX(gMineLocation[id].sector)].fEnemyControlled;
}


void InitCreatureQuest()
{
	UNDERGROUND_SECTORINFO *curr;
	BOOLEAN fPlayMeanwhile = FALSE;
	INT32 i=-1;
	INT32 iChosenMine;
	INT32 iRandom;
	INT32 iNumMinesInfectible;
	BOOLEAN fMineInfectible[4];

	if( giLairID )
	{
		return; //already active!
	}

	fPlayMeanwhile = TRUE;

	if( fPlayMeanwhile && !gfCreatureMeanwhileScenePlayed )
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
	fMineInfectible[0] = IsMineInfectible(MINE_DRASSEN);
	fMineInfectible[1] = IsMineInfectible(MINE_CAMBRIA);
	fMineInfectible[2] = IsMineInfectible(MINE_ALMA);
	fMineInfectible[3] = IsMineInfectible(MINE_GRUMM);

	iNumMinesInfectible = fMineInfectible[0] + fMineInfectible[1] + fMineInfectible[2] + fMineInfectible[3];

	if( !iNumMinesInfectible )
	{
		return;
	}

	//Choose one of the infectible mines randomly
	iRandom = Random( iNumMinesInfectible ) + 1;

	iChosenMine = 0;

	for( i = 0; i < 4; i++ )
	{
		if( iRandom )
		{
			iChosenMine++;
			if( fMineInfectible[i] )
			{
				iRandom--;
			}
		}
	}

	//Now, choose a start location for the queen.
	switch( iChosenMine )
	{
		case 1: //Drassen
			InitLairDrassen();
			curr = FindUnderGroundSector( 13, 5, 1 );
			curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
			break;
		case 2: //Cambria
			InitLairCambria();
			curr = FindUnderGroundSector( 9, 8, 1 );
			curr->uiFlags |= SF_PENDING_ALTERNATE_MAP; //entrance
			break;
		case 3: //Alma's mine
			InitLairAlma();
			curr = FindUnderGroundSector( 14, 10, 1 );
			curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
			break;
		case 4: //Grumm's mine
			InitLairGrumm();
			curr = FindUnderGroundSector( 4, 8, 2 );
			curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
			break;
		default:
			return;
	}

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
					SLOGE(DEBUG_TAG_ASSERTS, "PlaceNewCreature: invalid habitat type");
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
			iMaxPopulation = MAX( iMaxPopulation, 25 );
			//Now, convert the previous value into a numeric population.
			iMaxPopulation = iAbsoluteMaxPopulation * iMaxPopulation / 100;
			iMaxPopulation = MAX( iMaxPopulation, 4 );


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
		PlaceNewCreature( lair, 0 );
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

	UINT8 slot = 0;
	while (n_young_males + n_young_females + n_adult_males + n_adult_females != 0)
	{
		UINT32          const roll = Random(n_young_males + n_young_females + n_adult_males + n_adult_females);
		SoldierBodyType const body =
			roll < n_young_males                                   ? --n_young_males,   YAM_MONSTER :
			roll < n_young_males + n_young_females                 ? --n_young_females, YAF_MONSTER :
			roll < n_young_males + n_young_females + n_adult_males ? --n_adult_males,   AM_MONSTER  :
			(--n_adult_females, ADULTFEMALEMONSTER);

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
		UpdateMercInSector(*s, gWorldSectorX, gWorldSectorY, 0);
	}

	gsCreatureInsertionCode      = 0;
	gsCreatureInsertionGridNo    = 0;
	gubNumCreaturesAttackingTown = 0;
	gubYoungMalesAttackingTown   = 0;
	gubYoungFemalesAttackingTown = 0;
	gubAdultMalesAttackingTown   = 0;
	gubAdultFemalesAttackingTown = 0;
	gubCreatureBattleCode        = CREATURE_BATTLE_CODE_NONE;
	gubSectorIDOfCreatureAttack  = 0;
	AllTeamsLookForAll(FALSE);
}


static void ChooseTownSectorToAttack(UINT8 ubSectorID, BOOLEAN fOverrideTest)
{
	INT32 iRandom;

	if( !fOverrideTest )
	{
		iRandom = PreRandom( 100 );
		switch( ubSectorID )
		{
			case SEC_D13: //DRASSEN
				if( iRandom < 45 )
					ubSectorID = SEC_D13;
				else if( iRandom < 70 )
					ubSectorID = SEC_C13;
				else
					ubSectorID = SEC_B13;
				break;
			case SEC_H3: //GRUMM
				if( iRandom < 35 )
					ubSectorID = SEC_H3;
				else if( iRandom < 55 )
					ubSectorID = SEC_H2;
				else if( iRandom < 70 )
					ubSectorID = SEC_G2;
				else if( iRandom < 85 )
					ubSectorID = SEC_H1;
				else
					ubSectorID = SEC_G1;
				break;
			case SEC_H8: //CAMBRIA
				if( iRandom < 35 )
					ubSectorID = SEC_H8;
				else if( iRandom < 55 )
					ubSectorID = SEC_G8;
				else if( iRandom < 70 )
					ubSectorID = SEC_F8;
				else if( iRandom < 85 )
					ubSectorID = SEC_G9;
				else
					ubSectorID = SEC_F9;
				break;
			case SEC_I14: //ALMA
				if( iRandom < 45 )
					ubSectorID = SEC_I14;
				else if( iRandom < 65 )
					ubSectorID = SEC_I13;
				else if( iRandom < 85 )
					ubSectorID = SEC_H14;
				else
					ubSectorID = SEC_H13;
				break;
			default:
				SLOGE(DEBUG_TAG_ASSERTS, "ChooseTownSectorToAttack: invalid SectorID");
				return;
		}
	}
	switch( ubSectorID )
	{
		case SEC_D13: //DRASSEN
			gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
			gsCreatureInsertionGridNo = 20703;
			break;
		case SEC_C13:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_B13:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_H3: //GRUMM
			gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
			gsCreatureInsertionGridNo = 10303;
			break;
		case SEC_H2:
			gsCreatureInsertionCode = INSERTION_CODE_EAST;
			break;
		case SEC_G2:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_H1:
			gsCreatureInsertionCode = INSERTION_CODE_EAST;
			break;
		case SEC_G1:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_H8: //CAMBRIA
			gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
			gsCreatureInsertionGridNo = 13005;
			break;
		case SEC_G8:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_F8:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_G9:
			gsCreatureInsertionCode = INSERTION_CODE_WEST;
			break;
		case SEC_F9:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_I14: //ALMA
			gsCreatureInsertionCode = INSERTION_CODE_GRIDNO;
			gsCreatureInsertionGridNo = 9726;
			break;
		case SEC_I13:
			gsCreatureInsertionCode = INSERTION_CODE_EAST;
			break;
		case SEC_H14:
			gsCreatureInsertionCode = INSERTION_CODE_SOUTH;
			break;
		case SEC_H13:
			gsCreatureInsertionCode = INSERTION_CODE_EAST;
			break;
		default:
			return;
	}
	gubSectorIDOfCreatureAttack = ubSectorID;
}

void CreatureAttackTown( UINT8 ubSectorID, BOOLEAN fOverrideTest )
{ //This is the launching point of the creature attack.
	UNDERGROUND_SECTORINFO *pSector;
	UINT8 ubSectorX, ubSectorY;

	if( gfWorldLoaded && gTacticalStatus.fEnemyInSector )
	{ //Battle currently in progress, repost the event
		AddStrategicEvent( EVENT_CREATURE_ATTACK, GetWorldTotalMin() + Random( 10 ), ubSectorID );
		return;
	}

	gubCreatureBattleCode = CREATURE_BATTLE_CODE_NONE;

	ubSectorX = (UINT8)((ubSectorID % 16) + 1);
	ubSectorY = (UINT8)((ubSectorID / 16) + 1);

	if( !fOverrideTest )
	{
		//Record the number of creatures in the sector.
		pSector = FindUnderGroundSector( ubSectorX, ubSectorY, 1 );
		if( !pSector )
		{
			CreatureAttackTown( ubSectorID, TRUE );
			return;
		}
		gubNumCreaturesAttackingTown = pSector->ubNumCreatures;
		if( !gubNumCreaturesAttackingTown )
		{
			CreatureAttackTown( ubSectorID, TRUE );
			return;
		}

		pSector->ubNumCreatures = 0;

		//Choose one of the town sectors to attack.  Sectors closer to
		//the mine entrance have a greater chance of being chosen.
		ChooseTownSectorToAttack( ubSectorID, FALSE );
		ubSectorX = (UINT8)((gubSectorIDOfCreatureAttack % 16) + 1);
		ubSectorY = (UINT8)((gubSectorIDOfCreatureAttack / 16) + 1);
	}
	else
	{
		ChooseTownSectorToAttack( ubSectorID, TRUE );
		gubNumCreaturesAttackingTown = 5;
	}

	//Now that the sector has been chosen, attack it!
	if( PlayerGroupsInSector( ubSectorX, ubSectorY, 0 ) )
	{ //we have players in the sector
		if( ubSectorX == gWorldSectorX && ubSectorY == gWorldSectorY && !gbWorldSectorZ )
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
	else if( CountAllMilitiaInSector( ubSectorX, ubSectorY ) )
	{ //we have militia in the sector
		gubCreatureBattleCode = CREATURE_BATTLE_CODE_AUTORESOLVE;
	}
	else if( !StrategicMap[ ubSectorX + MAP_WORLD_X * ubSectorY ].fEnemyControlled )
	{ //player controlled sector -- eat some civilians
		AdjustLoyaltyForCivsEatenByMonsters( ubSectorX, ubSectorY, gubNumCreaturesAttackingTown );
		SectorInfo[ ubSectorID ].ubDayOfLastCreatureAttack = (UINT8)GetWorldDay();
		return;
	}
	else
	{ //enemy controlled sectors don't get attacked.
		return;
	}

	SectorInfo[ ubSectorID ].ubDayOfLastCreatureAttack = (UINT8)GetWorldDay();
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
	LockPauseState(LOCK_PAUSE_02);
}


static void DeleteDirectiveNode(CREATURE_DIRECTIVE** node)
{
	if( (*node)->next )
		DeleteDirectiveNode( &((*node)->next) );
	MemFree( *node );
	*node = NULL;
}

//Recursively delete all nodes (from the top down).
void DeleteCreatureDirectives()
{
	if( lair )
		DeleteDirectiveNode( &lair );
	giLairID = 0;
}

void ClearCreatureQuest()
{
	//This will remove all of the underground sector information and reinitialize it.
	//The only part that doesn't get added are the queen's lair.
	BuildUndergroundSectorInfoList();
	DeleteCreatureDirectives();
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
	curr = lair;
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
	pSector = FindUnderGroundSector( 9, 10, 2 );
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
	UINT8 ubSectorX, ubSectorY;
	ubSectorX = (UINT8)SECTORX( ubSectorID );
	ubSectorY = (UINT8)SECTORY( ubSectorID );
	pSector = FindUnderGroundSector( ubSectorX, ubSectorY, ubSectorZ );
	if( pSector )
		return pSector->ubNumCreatures;
	return 0;
}

BOOLEAN MineClearOfMonsters( UINT8 ubMineIndex )
{
	Assert(ubMineIndex < MAX_NUMBER_OF_MINES);

	if( !gMineStatus[ ubMineIndex ].fPrevInvadedByMonsters )
	{
		switch( ubMineIndex )
		{
			case MINE_GRUMM:
				if( CreaturesInUndergroundSector( SEC_H3, 1 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_I3, 1 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_I3, 2 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_H3, 2 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_H4, 2 ) )
					return FALSE;
				break;
			case MINE_CAMBRIA:
				if( CreaturesInUndergroundSector( SEC_H8, 1 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_H9, 1 ) )
					return FALSE;
				break;
			case MINE_ALMA:
				if( CreaturesInUndergroundSector( SEC_I14, 1 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_J14, 1 ) )
					return FALSE;
				break;
			case MINE_DRASSEN:
				if( CreaturesInUndergroundSector( SEC_D13, 1 ) )
					return FALSE;
				if( CreaturesInUndergroundSector( SEC_E13, 1 ) )
					return FALSE;
				break;
			case MINE_CHITZENA:
			case MINE_SAN_MONA:
				// these are never attacked
				break;

			default:
				SLOGE(DEBUG_TAG_SMAP, "Attempting to check if mine is clear but mine index is invalid (%d).", ubMineIndex );
				break;
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

void DetermineCreatureTownComposition(UINT8 ubNumCreatures,
					UINT8 *pubNumYoungMales, UINT8 *pubNumYoungFemales,
					UINT8 *pubNumAdultMales, UINT8 *pubNumAdultFemales)
{
	INT32 i, iRandom;
	UINT8 ubYoungMalePercentage = 10;
	UINT8 ubYoungFemalePercentage = 65;
	UINT8 ubAdultMalePercentage = 5;
	UINT8 ubAdultFemalePercentage = 20;

	//First step is to convert the percentages into the numbers we will use.
	ubYoungFemalePercentage += ubYoungMalePercentage;
	ubAdultMalePercentage += ubYoungFemalePercentage;
	ubAdultFemalePercentage += ubAdultMalePercentage;
	if( ubAdultFemalePercentage != 100 )
	{
		SLOGE(DEBUG_TAG_ASSERTS, "Percentage for adding creatures don't add up to 100." );
	}
	//Second step is to determine the breakdown of the creatures randomly.
	i = ubNumCreatures;
	while( i-- )
	{
		iRandom = Random( 100 );
		if( iRandom < ubYoungMalePercentage )
			(*pubNumYoungMales)++;
		else if( iRandom < ubYoungFemalePercentage )
			(*pubNumYoungFemales)++;
		else if( iRandom < ubAdultMalePercentage )
			(*pubNumAdultMales)++;
		else
			(*pubNumAdultFemales)++;
	}
}

void DetermineCreatureTownCompositionBasedOnTacticalInformation(UINT8 *pubNumCreatures,
								UINT8 *pubNumYoungMales, UINT8 *pubNumYoungFemales,
								UINT8 *pubNumAdultMales, UINT8 *pubNumAdultFemales)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
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
	INT32 i, iRandom;
	BOOLEAN fQueen;
	UINT8 ubLarvaePercentage;
	UINT8 ubInfantPercentage;
	UINT8 ubYoungMalePercentage;
	UINT8 ubYoungFemalePercentage;
	UINT8 ubAdultMalePercentage;
	UINT8 ubAdultFemalePercentage;
	UINT8 ubCreatureHabitat;
	UINT8 ubNumLarvae = 0;
	UINT8 ubNumInfants = 0;
	UINT8 ubNumYoungMales = 0;
	UINT8 ubNumYoungFemales = 0;
	UINT8 ubNumAdultMales = 0;
	UINT8 ubNumAdultFemales = 0;
	UINT8 ubNumCreatures;

	if( !gubCreatureBattleCode )
	{
		//By default, we only play creature music in the cave levels (the creature levels all consistently
		//have blue lights while human occupied mines have red lights.  We always play creature music
		//when creatures are in the level.
		gfUseCreatureMusic = LightGetColor()->b != 0;

		if( !gbWorldSectorZ )
			return FALSE;  //Creatures don't attack overworld with this battle code.
		pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
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

	switch( ubCreatureHabitat )
	{
		case QUEEN_LAIR:
			fQueen = TRUE;
			ubLarvaePercentage = 20;
			ubInfantPercentage = 40;
			ubYoungMalePercentage = 0;
			ubYoungFemalePercentage = 0;
			ubAdultMalePercentage = 30;
			ubAdultFemalePercentage = 10;
			break;
		case LAIR:
			fQueen = FALSE;
			ubLarvaePercentage = 15;
			ubInfantPercentage = 35;
			ubYoungMalePercentage = 10;
			ubYoungFemalePercentage = 5;
			ubAdultMalePercentage = 25;
			ubAdultFemalePercentage = 10;
			break;
		case LAIR_ENTRANCE:
			fQueen = FALSE;
			ubLarvaePercentage = 0;
			ubInfantPercentage = 15;
			ubYoungMalePercentage = 30;
			ubYoungFemalePercentage = 10;
			ubAdultMalePercentage = 35;
			ubAdultFemalePercentage = 10;
			break;
		case INNER_MINE:
			fQueen = FALSE;
			ubLarvaePercentage = 0;
			ubInfantPercentage = 0;
			ubYoungMalePercentage = 20;
			ubYoungFemalePercentage = 40;
			ubAdultMalePercentage = 10;
			ubAdultFemalePercentage = 30;
			break;
		case OUTER_MINE:
		case MINE_EXIT:
			fQueen = FALSE;
			ubLarvaePercentage = 0;
			ubInfantPercentage = 0;
			ubYoungMalePercentage = 10;
			ubYoungFemalePercentage = 65;
			ubAdultMalePercentage = 5;
			ubAdultFemalePercentage = 20;
			break;
		default:
			SLOGE(DEBUG_TAG_SMAP, "Invalid creature habitat ID of %d for PrepareCreaturesForBattle.  Ignoring...", ubCreatureHabitat );
			return FALSE;
	}

	//First step is to convert the percentages into the numbers we will use.
	if( fQueen )
	{
		ubNumCreatures--;
	}
	ubInfantPercentage += ubLarvaePercentage;
	ubYoungMalePercentage += ubInfantPercentage;
	ubYoungFemalePercentage += ubYoungMalePercentage;
	ubAdultMalePercentage += ubYoungFemalePercentage;
	ubAdultFemalePercentage += ubAdultMalePercentage;
	if( ubAdultFemalePercentage != 100 )
	{
		SLOGE(DEBUG_TAG_ASSERTS, "Percentage for adding creatures don't add up to 100." );
	}
	//Second step is to determine the breakdown of the creatures randomly.
	i = ubNumCreatures;
	while( i-- )
	{
		iRandom = Random( 100 );
		if( iRandom < ubLarvaePercentage )
			ubNumLarvae++;
		else if( iRandom < ubInfantPercentage )
			ubNumInfants++;
		else if( iRandom < ubYoungMalePercentage )
			ubNumYoungMales++;
		else if( iRandom < ubYoungFemalePercentage )
			ubNumYoungFemales++;
		else if( iRandom < ubAdultMalePercentage )
			ubNumAdultMales++;
		else
			ubNumAdultFemales++;
	}

	if( gbWorldSectorZ )
	{
		UNDERGROUND_SECTORINFO *pUndergroundSector;
		pUndergroundSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		if( !pUndergroundSector )
		{ //No info?!!!!!
			SLOGE(DEBUG_TAG_ASSERTS, "Please report underground sector you are in or going to and send save if possible." );
			return FALSE;
		}
		pUndergroundSector->ubCreaturesInBattle = pUndergroundSector->ubNumCreatures;
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
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
	UINT8 ubNumCreatures;
	ubNumCreatures = CreaturesInUndergroundSector( SEC_H3, 1 );
	if( ubNumCreatures > 1 && ubNumCreatures * 10 > (INT32)PreRandom( 100 ) )
	{ //10% chance for each creature to decide it's time to attack.
		AddStrategicEvent( EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom( 429 ), SEC_H3 );
	}
	ubNumCreatures = CreaturesInUndergroundSector( SEC_D13, 1 );
	if( ubNumCreatures > 1 && ubNumCreatures * 10 > (INT32)PreRandom( 100 ) )
	{ //10% chance for each creature to decide it's time to attack.
		AddStrategicEvent( EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom( 429 ), SEC_D13 );
	}
	ubNumCreatures = CreaturesInUndergroundSector( SEC_I14, 1 );
	if( ubNumCreatures > 1 && ubNumCreatures * 10 > (INT32)PreRandom( 100 ) )
	{ //10% chance for each creature to decide it's time to attack.
		AddStrategicEvent( EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 + PreRandom( 429 ), SEC_I14 );
	}
	ubNumCreatures = CreaturesInUndergroundSector( SEC_H8, 1 );
	if( ubNumCreatures > 1 && ubNumCreatures * 10 > (INT32)PreRandom( 100 ) )
	{ //10% chance for each creature to decide it's time to attack.
		AddStrategicEvent( EVENT_CREATURE_ATTACK, GetWorldTotalMin() + 1 +PreRandom( 429 ), SEC_H8 );
	}
}


void CheckConditionsForTriggeringCreatureQuest( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	UINT8 ubValidMines = 0;
	if( !gGameOptions.fSciFi )
		return; //No scifi, no creatures...
	if( giLairID )
		return;	//Creature quest already begun

	//Count the number of "infectible mines" the player occupies
	if( !StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( SEC_D13 ) ].fEnemyControlled )
	{
		ubValidMines++;
	}
	if( !StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( SEC_H8 ) ].fEnemyControlled )
	{
		ubValidMines++;
	}
	if( !StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( SEC_I14 ) ].fEnemyControlled )
	{
		ubValidMines++;
	}
	if( !StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( SEC_H3 ) ].fEnemyControlled )
	{
		ubValidMines++;
	}

	if( ubValidMines >= 3 )
	{
		InitCreatureQuest();
	}
}


void SaveCreatureDirectives(HWFILE const hFile)
{
	FileWrite(hFile, &giHabitatedDistance,  4);
	FileWrite(hFile, &giPopulationModifier, 4);
	FileWrite(hFile, &giLairID,             4);
	FileWrite(hFile, &gfUseCreatureMusic,   1);
	FileWrite(hFile, &giDestroyedLairID,    4);
}


void LoadCreatureDirectives(HWFILE const hFile, UINT32 const uiSavedGameVersion)
{
	FileRead(hFile, &giHabitatedDistance,  4);
	FileRead(hFile, &giPopulationModifier, 4);
	FileRead(hFile, &giLairID,             4);
	FileRead(hFile, &gfUseCreatureMusic,   1);

	if( uiSavedGameVersion >= 82 )
	{
		FileRead(hFile, &giDestroyedLairID, 4);
	}
	else
	{
		giDestroyedLairID = 0;
	}

	switch( giLairID )
	{
		case -1:											break; //creature quest finished -- it's okay
		case 0:												break; //lair doesn't exist yet -- it's okay
		case 1:		InitLairDrassen();	break;
		case 2:		InitLairCambria();	break;
		case 3:		InitLairAlma();			break;
		case 4:		InitLairGrumm();		break;
		default:
			SLOGE(DEBUG_TAG_SMAP, "Invalid restoration of creature lair ID of %d.  Save game potentially hosed.", giLairID );
			break;
	}
}


BOOLEAN PlayerGroupIsInACreatureInfestedMine()
{
	CREATURE_DIRECTIVE *curr;
	INT16 sSectorX, sSectorY;
	INT8 bSectorZ;

	if( giLairID <= 0 )
	{ //Creature quest inactive
		return FALSE;
	}

	//Lair is active, so look for live soldier in any creature level
	curr = lair;
	while( curr )
	{
		sSectorX = curr->pLevel->ubSectorX;
		sSectorY = curr->pLevel->ubSectorY;
		bSectorZ = (INT8)curr->pLevel->ubSectorZ;
		//Loop through all the creature directives (mine sectors that are infectible) and
		//see if players are there.
		CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
		{
			if (pSoldier->bLife    != 0 &&
					pSoldier->sSectorX == sSectorX &&
					pSoldier->sSectorY == sSectorY &&
					pSoldier->bSectorZ == bSectorZ &&
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


bool GetWarpOutOfMineCodes(INT16* const sector_x, INT16* const sector_y, INT8* const sector_z, INT16* const insertion_grid_no)
{
	if (!gfWorldLoaded)      return false;
	if (gbWorldSectorZ == 0) return false;

	INT32 lair_id = giLairID;
	if (lair_id == -1) lair_id = giDestroyedLairID;

	//:Now make sure the mercs are in the previously infested mine
	INT16 const x = gWorldSectorX;
	INT16 const y = gWorldSectorY;
	INT8  const z = gbWorldSectorZ;
	switch (lair_id)
	{
		case 1: // Drassen
			if ((x == 13 && y == 6 && z == 3) ||
					(x == 13 && y == 7 && z == 3) ||
					(x == 13 && y == 7 && z == 2) ||
					(x == 13 && y == 6 && z == 2) ||
					(x == 13 && y == 5 && z == 2) ||
					(x == 13 && y == 5 && z == 1) ||
					(x == 13 && y == 4 && z == 1))
			{
				*sector_x          = 13;
				*sector_y          = 4;
				*sector_z          = 0;
				*insertion_grid_no = 20700;
				return true;
			}
			break;

		case 3: // Cambria
			if ((x == 8 && y == 9 && z == 3) ||
					(x == 8 && y == 8 && z == 3) ||
					(x == 8 && y == 8 && z == 2) ||
					(x == 9 && y == 8 && z == 2) ||
					(x == 9 && y == 8 && z == 1) ||
					(x == 8 && y == 8 && z == 1))
			{
				*sector_x          = 8;
				*sector_y          = 8;
				*sector_z          = 0;
				*insertion_grid_no = 13002;
				return true;
			}
			break;

		case 2: // Alma
			if ((x == 13 && y == 11 && z == 3) ||
					(x == 13 && y == 10 && z == 3) ||
					(x == 13 && y == 10 && z == 2) ||
					(x == 14 && y == 10 && z == 2) ||
					(x == 14 && y == 10 && z == 1) ||
					(x == 14 && y ==  9 && z == 1))
			{
				*sector_x          = 14;
				*sector_y          = 9;
				*sector_z          = 0;
				*insertion_grid_no = 9085;
				return true;
			}
			break;

		case 4: // Grumm
			if ((x == 4 && y == 7 && z == 3) ||
					(x == 4 && y == 8 && z == 3) ||
					(x == 3 && y == 8 && z == 2) ||
					(x == 3 && y == 8 && z == 2) ||
					(x == 3 && y == 9 && z == 2) ||
					(x == 3 && y == 9 && z == 1) ||
					(x == 3 && y == 8 && z == 1))
			{
				*sector_x          = 3;
				*sector_y          = 8;
				*sector_z          = 0;
				*insertion_grid_no = 9822;
				return true;
			}
			break;
	}
	return false;
}
