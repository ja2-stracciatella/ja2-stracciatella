#include "Font.h"
#include "Font_Control.h"
#include "Soldier_Tile.h"
#include "Types.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"
#include "Queen_Command.h"
#include "Strategic_AI.h"
#include "Game_Event_Hook.h"
#include "GameSettings.h"
#include "Campaign_Types.h"
#include "Random.h"
#include "Strategic_Pathing.h"
#include "Message.h"
#include "StrategicMap.h"
#include "Game_Clock.h"
#include "Explosion_Control.h"
#include "Town_Militia.h"
#include "Overhead.h"
#include "Quests.h"
#include "Player_Command.h"
#include "Campaign_Init.h"
#include "Game_Init.h"
#include "PreBattle_Interface.h"
#include "Soldier_Profile.h"
#include "Scheduling.h"
#include "Campaign.h"
#include "Map_Information.h"
#include "Interface_Dialogue.h"
#include "MemMan.h"
#include "Debug.h"
#include "FileMan.h"

#ifdef JA2BETAVERSION
#	include "JAScreens.h"
#	include "MessageBoxScreen.h"
#	include "ScreenIDs.h"
#	include "Squads.h"
#	include <stdarg.h>
#endif


#define SAI_VERSION		29

/*
STRATEGIC AI -- UNDERLYING PHILOSOPHY
The most fundamental part of the strategic AI which takes from reality and gives to gameplay is the manner
the queen attempts to take her towns back.  Finances and owning mines are the most important way
to win the game.  As the player takes more mines over, the queen will focus more on quality and defense.  In
the beginning of the game, she will focus more on offense than mid-game or end-game.

REALITY
The queen owns the entire country, and the player starts the game with a small lump of cash, enough to hire
some mercenaries for about a week.  In that week, the queen may not notice what is going on, and the player
would believably take over one of the towns before she could feasibly react.  As soon as her military was
aware of the situation, she would likely proceed to send 300-400 troops to annihilate the opposition, and the
game would be over relatively quickly.  If the player was a prodigy, and managed to hold the town against such
a major assault, he would probably lose in the long run being forced into a defensive position and running out
of money quickly while the queen could continue to pump out the troops.  On the other hand, if the player
somehow managed to take over most of the mines, he would be able to casually walk over the queen eventually
just from the sheer income allowing him to purchase several of the best mercs.  That would have the effect of
making the game impossibly difficult in the beginning of the game, and a joke at the end (this is very much
like Master Of Orion II on the more difficult settings )

GAMEPLAY
Because we want the game to be like a normal game and make it fun, we need to make the game easy in the
beginning and harder at the end.  In order to accomplish this, I feel that pure income shouldn't be the factor
for the queen, because she would likely crucify a would-be leader in his early days.  So, in the beginning of
the game, the forces would already be situated with the majority of forces being the administrators in the towns,
and army troops and elites in the more important sectors.  Restricting the queen's offensive
abilities using a distance penalty would mean that the furthest sectors from the queen's palace would be
much easier to defend because she would only be allowed to send x number of troops.  As you get closer to the
queen, she would be allowed to send larger forces to attack those towns in question.  Also, to further
increase the games difficulty as the campaign progresses in the player's favor, we could also increase the
quality of the queen's troops based purely on the peek progress percentage.  This is calculated using a formula
that determines how well the player is doing by combining loyalty of towns owned, income generated, etc.  So,
in the beginning of the game, the quality is at the worst, but once you capture your first mines/towns, it
permanently  increase the queen's quality rating, effectively bumping up the stakes.  By the time you capture
four or five mines, the queen is going to focus more (but not completely) on quality defense as she prepares
for your final onslaught.  This quality rating will augment the experience level, equipment rating, and/or
attribute ratings of the queen's troops.  I would maintain a table of these enhancements based on the current
quality rating hooking into the difficulty all along.

//EXPLANATION OF THE WEIGHT SYSTEM:
The strategic AI has two types of groups:  garrisons and patrol groups.  Each of these groups contain
information of it's needs, mainly desired population.  If the current population is greater than the
desired population, and the group will get a negative weight assigned to it, which means that it is willing
to give up troops to areas that need them more.  On the other hand, if a group has less than the desired population,
then the weight will be positive, meaning they are requesting reinforcements.

The weight generated will range between -100 and +100.  The calculated weight is modified by the priority
of the group.  If the priority of the group is high, they
*/

//Modifies the number of troops the queen has at the beginning of the game on top
//of all of the garrison and patrol groups.  Additionally, there are a total of
//16 sectors that are LEVEL 1, 2, or 3 garrison groups.  The lower the level, the more
//troops stay in that sector, and the rest will also be used as a secondary pool when
//the primary pool runs dry.  So basically, this number is only part of the equation.
#define EASY_QUEENS_POOL_OF_TROOPS					150
#define NORMAL_QUEENS_POOL_OF_TROOPS				200
#define HARD_QUEENS_POOL_OF_TROOPS					400

//Modifies the starting values as well as the desired values for all of the garrisons.
#define EASY_INITIAL_GARRISON_PERCENTAGES		70
#define NORMAL_INITIAL_GARRISON_PERCENTAGES	100
#define HARD_INITIAL_GARRISON_PERCENTAGES		125

#define EASY_MIN_ENEMY_GROUP_SIZE						3
#define NORMAL_MIN_ENEMY_GROUP_SIZE					4
#define HARD_MIN_ENEMY_GROUP_SIZE						6

//Sets the starting alert chances.  Everytime an enemy arrives in a new sector, or the player,
//this is the chance the enemy will detect the player in adjacent sectors.  This chance is associated
//with each side checked.  Stationary groups do this check periodically.
#define EASY_ENEMY_STARTING_ALERT_LEVEL			5
#define	NORMAL_ENEMY_STARTING_ALERT_LEVEL		20
#define HARD_ENEMY_STARTING_ALERT_LEVEL			60

//When an enemy spots and chases a player group, the alertness value decrements by this value.  The
//higher the value, the less of a chance the enemy will spot and attack subsequent groups.  This
//minimizes the aggressiveness of the enemy.  Ranges from 1-100 (but recommend 20-60).
#define EASY_ENEMY_STARTING_ALERT_DECAY			75
#define	NORMAL_ENEMY_STARTING_ALERT_DECAY		50
#define HARD_ENEMY_STARTING_ALERT_DECAY			25
//The base time that the queen can think about reinforcements for refilling lost patrol groups,
//town garrisons, etc. She only is allowed one action per 'turn'.
#define EASY_TIME_EVALUATE_IN_MINUTES				480
#define NORMAL_TIME_EVALUATE_IN_MINUTES			360
#define HARD_TIME_EVALUATE_IN_MINUTES				180
//The variance added on.
#define EASY_TIME_EVALUATE_VARIANCE					240
#define NORMAL_TIME_EVALUATE_VARIANCE				180
#define HARD_TIME_EVALUATE_VARIANCE					120

//When a player takes control of a sector, don't allow any enemy reinforcements to enter the sector for a
//limited amount of time.  This essentially dumbs down the AI, making it less aggressive.
#define EASY_GRACE_PERIOD_IN_HOURS					144		// 6 days
#define NORMAL_GRACE_PERIOD_IN_HOURS				96		// 4 days
#define HARD_GRACE_PERIOD_IN_HOURS					48		// 2 days

//Defines how many days must pass before the queen is willing to refill a defeated patrol group.
#define EASY_PATROL_GRACE_PERIOD_IN_DAYS		16
#define NORMAL_PATROL_GRACE_PERIOD_IN_DAYS	12
#define HARD_PATROL_GRACE_PERIOD_IN_DAYS		8

//Certain conditions can cause the queen to go into a "full alert" mode.  This means that temporarily, the queen's
//forces will automatically succeed adjacent checks until x number of enemy initiated battles occur.  The same variable
//is what is used to determine the free checks.
#define EASY_NUM_AWARE_BATTLES							1
#define NORMAL_NUM_AWARE_BATTLES						2
#define HARD_NUM_AWARE_BATTLES							3

BOOLEAN gfAutoAIAware = FALSE;

//Saved vars
INT8		gbPadding2[3]						= {0, 0, 0};	//NOT USED // XXX HACK000B
BOOLEAN gfExtraElites						= 0;	//Set when queen compositions are augmented with bonus elites.
static INT32 giGarrisonArraySize = 0;
INT32		giPatrolArraySize				= 0;
INT32		giForcePercentage				= 0;	//Modifies the starting group sizes relative by percentage
INT32		giArmyAlertness					= 0;	//The chance the group will spot an adjacent player/militia
INT32   giArmyAlertnessDecay		= 0;	//How much the spotting chance decreases when spot check succeeds
UINT8		gubNumAwareBattles			= 0;	//When non-zero, this means the queen is very aware and searching for players.  Every time
																			//there is an enemy initiated battle, this counter decrements until zero.  Until that point,
																			//all adjacent sector checks automatically succeed.
BOOLEAN gfQueenAIAwake					= FALSE;	//This flag turns on/off the strategic decisions.  If it's off, no reinforcements
																					//or assaults will happen.
																					//@@@Alex, this flag is ONLY set by the first meanwhile scene which calls an action.  If this
																					//action isn't called, the AI will never turn on.  It is completely dependant on this action.  It can
																					//be toggled at will in the AIViewer for testing purposes.
INT32		giReinforcementPool			= 0;	//How many troops the queen has in reserve in noman's land.  These guys are spawned as needed in P3.
INT32   giReinforcementPoints		= 0;	//the entire army's capacity to provide reinforcements.
INT32		giRequestPoints					= 0;	//the entire army's need for reinforcements.
UINT8   gubSAIVersion						= SAI_VERSION;	//Used for adding new features to be saved.
UINT8		gubQueenPriorityPhase		= 0;	//Defines how far into defence the queen is -- abstractly related to defcon index ranging from 0-10.
																			//10 is the most defensive
//Used for authorizing the use of the first battle meanwhile scene AFTER the battle is complete.  This is the case used when
//the player attacks a town, and is set once militia are sent to investigate.
BOOLEAN gfFirstBattleMeanwhileScenePending = FALSE;

//After the first battle meanwhile scene is finished, this flag is set, and the queen orders patrol groups to immediately fortify all towns.
BOOLEAN gfMassFortificationOrdered = FALSE;

UINT8		gubMinEnemyGroupSize			= 0;
UINT8		gubHoursGracePeriod      = 0;
UINT16	gusPlayerBattleVictories = 0;
BOOLEAN gfUseAlternateQueenPosition = FALSE;

//padding for generic globals
#define SAI_PADDING_BYTES				97
INT8		gbPadding[SAI_PADDING_BYTES]; // XXX HACK000B
//patrol group info plus padding
#define SAVED_PATROL_GROUPS			50
static PATROL_GROUP* gPatrolGroup;
//army composition info plus padding
#define SAVED_ARMY_COMPOSITIONS	60
ARMY_COMPOSITION gArmyComp[ NUM_ARMY_COMPOSITIONS ];
//garrison info plus padding
#define SAVED_GARRISON_GROUPS		100
GARRISON_GROUP *gGarrisonGroup	= NULL;

extern UINT8 gubNumGroupsArrivedSimultaneously;

//This refers to the number of force points that are *saved* for the AI to use.  This is basically an array of each
//group.  When the queen wants to send forces to attack a town that is defended, the initial number of forces that
//she would send would be considered too weak.  So, instead, she will send that force to the sector's adjacent sector,
//and stage, while
UINT8 *gubGarrisonReinforcementsDenied = NULL;
UINT8 *gubPatrolReinforcementsDenied = NULL;

//Unsaved vars
BOOLEAN gfDisplayStrategicAILogs = FALSE;


//The army composition defines attributes for the various garrisons.  The priority reflects how important the sector is
//to the queen, the elite/troop percentages refer to the desired composition of the group.  The admin percentage has recently been
//changed to reflect the starting percentage of the garrison that are administrators.  Note that elite% + troop% = 100, and the admin% is
//not related in this effect.  If the admin% is non-zero, then that garrison is assigned only x% of the force as admins, with NO troops or elites.
//All reinforcements use the composition of the troop/elite for refilling.
//@@@Alex, the send reinforcement composition isn't complete.  Either sends all troops or troops based off of the composition of the source garrison.
//  It is my intention to add this.

//If you change the MAX_STRATEGIC_TEAM_SIZE, then all the garrison sizes (start, desired) will have to be changed accordingly.

#define M(composition, prio, elite, troop, admin, desired, start) { composition, prio, elite, troop, admin, desired, start, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

static const ARMY_COMPOSITION gOrigArmyComp[NUM_ARMY_COMPOSITIONS] =
{	//COMPOSITION          PRIO ELITE% TROOP% ADMIN DESIRED# START#
	//                                              START%
	M(QUEEN_DEFENCE,        100, 100,   0,   0,  32,  32),
	M(MEDUNA_DEFENCE,        95,  55,  45,   0,  16,  20),
	M(MEDUNA_SAMSITE,        96,  65,  35,   0,  20,  20),
	M(LEVEL1_DEFENCE,        40,  20,  80,   0,  12,  20),
	M(LEVEL2_DEFENCE,        30,  10,  90,   0,  10,  20),
	M(LEVEL3_DEFENCE,        20,   5,  95,   0,   8,  20),
	M(ORTA_DEFENCE,          90,  50,  50,   0,  18,  19),
	M(EAST_GRUMM_DEFENCE,    80,  20,  80,   0,  15,  15),
	M(WEST_GRUMM_DEFENCE,    70,   0, 100,  40,  15,  15),
	M(GRUMM_MINE,            85,  25,  75,  45,  15,  15),
	M(OMERTA_WELCOME_WAGON,   0,   0, 100,   0,   0,   3),
	M(BALIME_DEFENCE,        60,  45,  55,  20,  10,   4),
	M(TIXA_PRISON,           80,  10,  90,  15,  15,  15),
	M(TIXA_SAMSITE,          85,  10,  90,   0,  12,  12),
	M(ALMA_DEFENCE,          74,  15,  85,   0,  11,  20),
	M(ALMA_MINE,             80,  20,  80,  45,  15,  20),
	M(CAMBRIA_DEFENCE,       50,   0, 100,  30,  10,   6),
	M(CAMBRIA_MINE,          60,  15,  90,  40,  11,   6),
	M(CHITZENA_DEFENCE,      30,   0, 100,  75,  12,  10),
	M(CHITZENA_MINE,         40,   0, 100,  75,  10,  10),
	M(CHITZENA_SAMSITE,      75,  10,  90,   0,   9,   9),
	M(DRASSEN_AIRPORT,       30,   0, 100,  85,  12,  10),
	M(DRASSEN_DEFENCE,       20,   0, 100,  80,  10,   8),
	M(DRASSEN_MINE,          35,   0, 100,  75,  11,   9),
	M(DRASSEN_SAMSITE,       50,   0, 100,   0,  10,  10),
	M(ROADBLOCK,             20,   2,  98,   0,   8,   0),
	M(SANMONA_SMALL,          0,   0,   0,   0,   0,   0)
};

#undef M

#define M(size, prio, p1, p2, p3, p4) { size, prio, { p1, p2, p3, p4 }, -1, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

//Patrol definitions
//NOTE:	  A point containing 0 is actually the same as SEC_A1, but because nobody is using SEC_A1 in any
//				of the patrol groups, I am coding 0 to be ignored.
//NOTE:		Must have at least two points.
static const PATROL_GROUP gOrigPatrolGroup[] =
{ //SIZE PRIO POINT1    POINT2    POINT3    POINT4
	M( 8,   40, SEC_B1,   SEC_C1,   SEC_C3,   SEC_A3 ),
	M( 6,   35, SEC_B4,   SEC_B7,   SEC_C7,   0      ),
	M( 6,   25, SEC_A8,   SEC_B8,   SEC_B9,   0      ),
	M( 6,   30, SEC_B10,  SEC_B12,  0,        0      ),
	M( 7,   45, SEC_A11,  SEC_A14,  SEC_D14,  0      ),
	//5
	M( 6,   50, SEC_C8,   SEC_C9,   SEC_D9,   0      ),
	M(12,   55, SEC_D3,   SEC_G3,   0,        0      ),
	M(10,   50, SEC_D6,   SEC_D7,   SEC_F7,   0      ),
	M(10,   55, SEC_E8,   SEC_E11,  SEC_F11,  0      ),
	M(10,   60, SEC_E12,  SEC_E15,  0,        0      ),
	//10
	M(12,   60, SEC_G4,   SEC_G7,   0,        0      ),
	M(12,   65, SEC_G10,  SEC_G12,  SEC_F12,  0      ),
	M(12,   65, SEC_G13,  SEC_G15,  0,        0      ),
	M(10,   65, SEC_H15,  SEC_J15,  0,        0      ),
	M(14,   65, SEC_H12,  SEC_J12,  SEC_J13,  0      ),
	//15
	M(13,   70, SEC_H9,   SEC_I9,   SEC_I10,  SEC_J10),
	M(11,   70, SEC_K11,  SEC_K14,  SEC_J14,  0      ),
	M(12,   75, SEC_J2,   SEC_K2,   0,        0      ),
	M(12,   80, SEC_I3,   SEC_J3,   0,        0      ),
	M(12,   80, SEC_J6,   SEC_K6,   0,        0      ),
	//20
	M(13,   85, SEC_K7,   SEC_K10,  0,        0      ),
	M(12,   90, SEC_L10,  SEC_M10,  0,        0      ),
	M(12,   90, SEC_N9,   SEC_N10,  0,        0      ),
	M(12,   80, SEC_L7,   SEC_L8,   SEC_M8,   SEC_M9 ),
	M(14,   80, SEC_H4,   SEC_H5,   SEC_I5,   0      ),
	//25
	M( 7,   40, SEC_D4,   SEC_E4,   SEC_E5,   0      ),
	M( 7,   50, SEC_C10,  SEC_C11,  SEC_D11,  SEC_D12),
	M( 8,   40, SEC_A15,  SEC_C15,  SEC_C16,  0      ),
	M(12,   30, SEC_L13,  SEC_M13,  SEC_M14,  SEC_L14)
	//29
};

#undef M

#define PATROL_GROUPS 29


#define M(sector, composition) { sector, composition, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

static const GARRISON_GROUP gOrigGarrisonGroup[] =
{ //SECTOR   MILITARY COMPOSITION
	M(SEC_P3,  QUEEN_DEFENCE       ),
	M(SEC_O3,  MEDUNA_DEFENCE      ),
	M(SEC_O4,  MEDUNA_DEFENCE      ),
	M(SEC_N3,  MEDUNA_DEFENCE      ),
	M(SEC_N4,  MEDUNA_SAMSITE      ),
	//5
	M(SEC_N5,  MEDUNA_DEFENCE      ),
	M(SEC_M3,  LEVEL1_DEFENCE      ),
	M(SEC_M4,  LEVEL1_DEFENCE      ),
	M(SEC_M5,  LEVEL1_DEFENCE      ),
	M(SEC_N6,  LEVEL1_DEFENCE      ),
	//10
	M(SEC_M2,  LEVEL2_DEFENCE      ),
	M(SEC_L3,  LEVEL2_DEFENCE      ),
	M(SEC_L4,  LEVEL2_DEFENCE      ),
	M(SEC_L5,  LEVEL2_DEFENCE      ),
	M(SEC_M6,  LEVEL2_DEFENCE      ),
	//15
	M(SEC_N7,  LEVEL1_DEFENCE      ),
	M(SEC_L2,  LEVEL3_DEFENCE      ),
	M(SEC_K3,  LEVEL3_DEFENCE      ),
	M(SEC_K5,  LEVEL3_DEFENCE      ),
	M(SEC_L6,  LEVEL3_DEFENCE      ),
	//20
	M(SEC_M7,  LEVEL3_DEFENCE      ),
	M(SEC_N8,  LEVEL3_DEFENCE      ),
	M(SEC_K4,  ORTA_DEFENCE        ),
	M(SEC_G1,  WEST_GRUMM_DEFENCE  ),
	M(SEC_G2,  EAST_GRUMM_DEFENCE  ),
	//25
	M(SEC_H1,  WEST_GRUMM_DEFENCE  ),
	M(SEC_H2,  EAST_GRUMM_DEFENCE  ),
	M(SEC_H3,  GRUMM_MINE          ),
	M(SEC_A9,  OMERTA_WELCOME_WAGON),
	M(SEC_L11, BALIME_DEFENCE      ),
	//30
	M(SEC_L12, BALIME_DEFENCE      ),
	M(SEC_J9,  TIXA_PRISON         ),
	M(SEC_I8,  TIXA_SAMSITE        ),
	M(SEC_H13, ALMA_DEFENCE        ),
	M(SEC_H14, ALMA_DEFENCE        ),
	//35
	M(SEC_I13, ALMA_DEFENCE        ),
	M(SEC_I14, ALMA_MINE           ),
	M(SEC_F8,  CAMBRIA_DEFENCE     ),
	M(SEC_F9,  CAMBRIA_DEFENCE     ),
	M(SEC_G8,  CAMBRIA_DEFENCE     ),
	//40
	M(SEC_G9,  CAMBRIA_DEFENCE     ),
	M(SEC_H8,  CAMBRIA_MINE        ),
	M(SEC_A2,  CHITZENA_DEFENCE    ),
	M(SEC_B2,  CHITZENA_MINE       ),
	M(SEC_D2,  CHITZENA_SAMSITE    ),
	//45
	M(SEC_B13, DRASSEN_AIRPORT     ),
	M(SEC_C13, DRASSEN_DEFENCE     ),
	M(SEC_D13, DRASSEN_MINE        ),
	M(SEC_D15, DRASSEN_SAMSITE     ),
	M(SEC_G12, ROADBLOCK           ),
	//50
	M(SEC_M10, ROADBLOCK           ),
	M(SEC_G6,  ROADBLOCK           ),
	M(SEC_C9,  ROADBLOCK           ),
	M(SEC_K10, ROADBLOCK           ),
	M(SEC_G7,  ROADBLOCK           ),
	//55
	M(SEC_G3,  ROADBLOCK           ),
	M(SEC_C5,  SANMONA_SMALL       )
	//57
};

#undef M


//Records any decisions that I deem important enough into an automatically appending AI log file called
//"Strategic Decisions.txt" in the JA2\Data directory.  This also records the time that each log entry
//was made.
#ifdef JA2BETAVERSION
static void LogStrategicEvent(const char* str, ...); //adds a timestamp.
static void LogStrategicMsg(const char* str, ...); //doesn't use the time stamp
#endif


extern INT16 sWorldSectorLocationOfFirstBattle;


#if !defined JA2BETAVERSION
#	define SAIReportError(a) //define it out
#endif


//returns the number of reinforcements permitted to be sent.  Will increased if the denied counter is non-zero.
static INT32 GarrisonReinforcementsRequested(INT32 iGarrisonID, UINT8* pubExtraReinforcements)
{
	INT32 iReinforcementsRequested;
	INT32 iExistingForces;
	SECTORINFO *pSector;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iExistingForces = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iReinforcementsRequested = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation - iExistingForces;

	//Record how many of the reinforcements are additionally provided due to being denied in the past.  This will grow
	//until it is finally excepted or an absolute max is made.
	*pubExtraReinforcements = (UINT8)(gubGarrisonReinforcementsDenied[ iGarrisonID ] / (6 - gGameOptions.ubDifficultyLevel));
	//Make sure the number of extra reinforcements don't bump the force size past the max of MAX_STRATEGIC_TEAM_SIZE.
	*pubExtraReinforcements = (UINT8)MIN( (INT32)*pubExtraReinforcements, MIN( (INT32)(*pubExtraReinforcements), MAX_STRATEGIC_TEAM_SIZE - iReinforcementsRequested ) );

	iReinforcementsRequested = MIN( MAX_STRATEGIC_TEAM_SIZE, iReinforcementsRequested );

	if( iReinforcementsRequested + *pubExtraReinforcements + iExistingForces > MAX_STRATEGIC_TEAM_SIZE )
	{
		iExistingForces = iExistingForces;
	}

	return iReinforcementsRequested;
}


static INT32 PatrolReinforcementsRequested(PATROL_GROUP const* const pg)
{
	GROUP* const g = GetGroup(pg->ubGroupID);
	INT32 size = pg->bSize;
	if (g) size -= g->ubGroupSize;
	return size;
}


static INT32 ReinforcementsAvailable(INT32 iGarrisonID)
{
	SECTORINFO *pSector;
	INT32 iReinforcementsAvailable;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iReinforcementsAvailable = pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins;
	iReinforcementsAvailable -= gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;

	switch( gGarrisonGroup[ iGarrisonID ].ubComposition )
	{
		case LEVEL1_DEFENCE:
		case LEVEL2_DEFENCE:
		case LEVEL3_DEFENCE:
		case ALMA_DEFENCE:
		case ALMA_MINE:
			//Legal spawning locations
			break;
		default:
			//No other sector permitted to send surplus troops
			return 0;
	}

	return iReinforcementsAvailable;
}


static BOOLEAN PlayerForceTooStrong(UINT8 ubSectorID, UINT16 usOffensePoints, UINT16* pusDefencePoints)
{
	SECTORINFO *pSector;
	UINT8 ubSectorX, ubSectorY;

	ubSectorX = (UINT8)SECTORX( ubSectorID );
	ubSectorY = (UINT8)SECTORY( ubSectorID );
	pSector = &SectorInfo[ ubSectorID ];

	*pusDefencePoints = pSector->ubNumberOfCivsAtLevel[ GREEN_MILITIA ]		* 1 +
											pSector->ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] * 2 +
											pSector->ubNumberOfCivsAtLevel[ ELITE_MILITIA ]		* 3 +
											PlayerMercsInSector( ubSectorX, ubSectorY, 0 )		* 5;
	if( *pusDefencePoints > usOffensePoints )
	{
		return TRUE;
	}
	return FALSE;
}


static void SendReinforcementsForGarrison(INT32 iDstGarrisonID, UINT16 usDefencePoints, GROUP** pOptionalGroup);


static void RequestAttackOnSector(UINT8 ubSectorID, UINT16 usDefencePoints)
{
	INT32 i;
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		if( gGarrisonGroup[ i ].ubSectorID == ubSectorID && !gGarrisonGroup[ i ].ubPendingGroupID )
		{
			#ifdef JA2BETAVERSION
				LogStrategicEvent( "An attack has been requested in sector %c%d.",
					SECTORY( ubSectorID ) + 'A' - 1, SECTORX( ubSectorID ) );
			#endif
			SendReinforcementsForGarrison( i, usDefencePoints, NULL );
			return;
		}
	}
}


static BOOLEAN AdjacentSectorIsImportantAndUndefended(UINT8 ubSectorID)
{
	SECTORINFO *pSector;
	switch( ubSectorID )
	{
		case SEC_A9:	case SEC_A10:								//Omerta
		case SEC_C5:	case SEC_C6:	case SEC_D5:	//San Mona
		case SEC_I6:															//Estoni
			//These sectors aren't important.
			return FALSE;
	}
	pSector = &SectorInfo[ ubSectorID ];
	if( pSector->ubNumTroops || pSector->ubNumElites || pSector->ubNumAdmins )
	{
		return FALSE;
	}
	if( pSector->ubTraversability[ 4 ] == TOWN )
	{
		if( !PlayerSectorDefended( ubSectorID ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}


static void ValidatePendingGroups(void)
{
	#ifdef JA2BETAVERSION
		GROUP *pGroup;
		INT32 i, iErrorsForInvalidPendingGroup = 0;
		UINT8 ubGroupID;
		for( i = 0; i < giPatrolArraySize; i++ )
		{
			ubGroupID = gPatrolGroup[ i ].ubPendingGroupID;
			if( ubGroupID )
			{
				pGroup = GetGroup( ubGroupID );
				if( !pGroup || pGroup->fPlayer )
				{
					iErrorsForInvalidPendingGroup++;
					gPatrolGroup[ i ].ubPendingGroupID = 0;
				}
			}
		}
		for( i = 0; i < giGarrisonArraySize; i++ )
		{
			ubGroupID = gGarrisonGroup[ i ].ubPendingGroupID;
			if( ubGroupID )
			{
				pGroup = GetGroup( ubGroupID );
				if( !pGroup || pGroup->fPlayer )
				{
					iErrorsForInvalidPendingGroup++;
					gGarrisonGroup[ i ].ubPendingGroupID = 0;
				}
			}
		}
		if( iErrorsForInvalidPendingGroup )
		{
			wchar_t str[256];
			swprintf(str, lengthof(str), L"Strategic AI:  Internal error -- %d pending groups were discovered to be invalid.  Please report error and send save."
										 L"You can continue playing, as this has been auto-corrected.  No need to send any debug files.", iErrorsForInvalidPendingGroup );
			SAIReportError( str );
		}
	#endif
}


static void ValidateWeights(INT32 iID)
{
	#ifdef JA2BETAVERSION
		INT32 i;
		INT32 iSumRequestPoints = 0;
		INT32 iSumReinforcementPoints = 0;
		for( i = 0; i < giPatrolArraySize; i++ )
		{
			iSumRequestPoints += gPatrolGroup[ i ].bWeight;
		}
		for( i = 0; i < giGarrisonArraySize; i++ )
		{
			if( gGarrisonGroup[ i ].bWeight > 0 )
			{
				iSumRequestPoints += gGarrisonGroup[ i ].bWeight;
			}
			else if( gGarrisonGroup[ i ].bWeight < 0 )
			{
				iSumReinforcementPoints -= gGarrisonGroup[ i ].bWeight; //double negative is positive!
			}
		}
		if( giReinforcementPoints != iSumReinforcementPoints || giRequestPoints != iSumRequestPoints )
		{
			wchar_t str[256];
			swprintf(str, lengthof(str), L"Strategic AI:  Internal error #%02d (total request/reinforcement points).  Please report error including error#.  "
										 L"You can continue playing, as the points have been auto-corrected.  No need to send any save/debug files.", iID );
			//Correct the misalignment.
			giReinforcementPoints = iSumReinforcementPoints;
			giRequestPoints = iSumRequestPoints;
			SAIReportError( str );
		}
	#endif
}


static void ReassignAIGroup(GROUP** pGroup);


static void ValidateGroup(GROUP* pGroup)
{
	if( !pGroup->ubSectorX || !pGroup->ubSectorY || pGroup->ubSectorX > 16 || pGroup->ubSectorY > 16 )
	{
		if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
		{
			#ifdef JA2BETAVERSION
			{
				wchar_t str[256];
				swprintf(str, lengthof(str), L"Strategic AI:  Internal error (invalid enemy group #%d location at %c%d, destination %c%d).  Please send PRIOR save file and Strategic Decisions.txt.",
											 pGroup->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX, pGroup->ubNextY + 'A' - 1, pGroup->ubNextX );
				SAIReportError( str );
			}
			#endif
			RemoveGroupFromStrategicAILists(*pGroup);
			RemoveGroup(*pGroup);
			return;
		}
	}
	if( !pGroup->ubNextX || !pGroup->ubNextY )
	{
		if( !pGroup->fPlayer && pGroup->pEnemyGroup->ubIntention != STAGING
												 && pGroup->pEnemyGroup->ubIntention != REINFORCEMENTS )
		{
			#ifdef JA2BETAVERSION
				SAIReportError( L"Strategic AI:  Internal error (floating group).  Please send PRIOR save file and Strategic Decisions.txt." );
			#endif
			if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
			{
				RemoveGroupFromStrategicAILists(*pGroup);
				ReassignAIGroup( &pGroup );
				return;
			}
		}
	}
	#ifdef JA2BETAVERSION
		if( pGroup->pEnemyGroup->ubNumAdmins + pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites != pGroup->ubGroupSize ||
				pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE )
		{
			SAIReportError( L"Strategic AI:  Internal error (bad group populations).  Please send PRIOR save file and Strategic Decisions.txt." );
		}
	#endif
}


static void ValidateLargeGroup(GROUP* pGroup)
{
	#ifdef JA2BETAVERSION
		if( pGroup->ubGroupSize > 25 )
		{
			wchar_t str[ 512 ];
			swprintf(str, lengthof(str), L"Strategic AI warning:  Enemy group containing %d soldiers "
									 L"(%d admins, %d troops, %d elites) in sector %c%d.  This message is a temporary test message "
									 L"to evaluate a potential problems with very large enemy groups.",
									 pGroup->ubGroupSize, pGroup->pEnemyGroup->ubNumAdmins, pGroup->pEnemyGroup->ubNumTroops, pGroup->pEnemyGroup->ubNumElites,
									 pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
			SAIReportError( str );
		}
	#endif
}


#ifdef JA2BETAVERSION
static void RemovePlayersFromAllMismatchGroups(SOLDIERTYPE& s)
{
	FOR_ALL_GROUPS_SAFE(i)
	{
		GROUP& g = *i;
		if (!g.fPlayer) continue;
		if (s.ubGroupID == g.ubGroupID) continue;

		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, &g)
		{
			if (pPlayer->pSoldier != &s) continue;
			RemovePlayerFromPGroup(g, s);
			break;
		}
	}
}
#endif


#ifdef JA2BETAVERSION
void ValidatePlayersAreInOneGroupOnly(void)
{
	INT32 iGroups;
	INT32 iMismatches;
	INT32 iNumErrors;
	wchar_t str[1024];
	UINT8 ubGroupID;
	//Go through each merc slot in the player team
	iNumErrors = 0;
	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{ //check to see if the merc has a group ID
		if (!pSoldier->bLife || !pSoldier->ubGroupID)
		{ //non-existant, dead, or in no group (don't care, skip to next merc)
			continue;
		}

		//Record the merc's group ID, as we may have to restore this later if the merc is found to exist
		//in multiple groups.
		ubGroupID = pSoldier->ubGroupID;

		iGroups = 0;
		iMismatches = 0;
		//Go through each group and determine if the player exists in multiple groups
		//iGroups ------ counts the number of groups the merc is in.
		//iMismatches -- counts the cases where the merc's ubGroupID doesn't match the ubGroupID of the group
		//               the merc exists in.
		CFOR_ALL_PLAYER_GROUPS(pGroup)
		{
			CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
			{
				if( pPlayer->pSoldier == pSoldier )
				{
					if( pSoldier->ubGroupID != pGroup->ubGroupID )
					{
						iMismatches++;
					}
					iGroups++;
					break;
				}
			}
		}

		if( iMismatches || !iGroups )
		{ //If we have any mismatches or a merc not in a group, then there is definately an error.
			//We need to record and report the "first" error in detail
			iNumErrors++; //keeps track of the total errors (this number will be reported)

			if( iNumErrors == 1 )
			{ //This is the first error, so we will provide detailed debug information to help fix the bug(s).
				if( iGroups == 1 && iMismatches == 1 )
				{ //We have a very serious problem, as we have no way to know which group this merc is supposed to be in.
					//This problem cannot be corrected (so we will assign the merc to his own unique squad) and definately report it.

					//Get a pointer to the group that contains the merc
					iMismatches = 0;
					iGroups = 0;
					const GROUP* pOtherGroup = NULL;
					CFOR_ALL_PLAYER_GROUPS(pGroup)
					{
						CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
						{
							if( pPlayer->pSoldier == pSoldier )
							{
								if( pSoldier->ubGroupID != pGroup->ubGroupID )
								{
									pOtherGroup = pGroup;
									iMismatches++;
								}
								iGroups++;
								break;
							}
						}
						if( iMismatches == 1 )
						{
							break;
						}
					}
					const GROUP* const pGroup = GetGroup(pSoldier->ubGroupID);
					Assert( pGroup );
					Assert( pOtherGroup );
					swprintf(str, lengthof(str), L"%ls in %c%d thinks he/she is in group %d in %c%d but isn't.  "
												 L"Group %d in %c%d thinks %ls is in the group but isn't.  %ls will be assigned to a unique squad.  "
												 L"Please send screenshot, PRIOR save (corrected by time you read this), and any theories.",
												 pSoldier->name, pSoldier->sSectorY + 'A' - 1, pSoldier->sSectorX,
												 pSoldier->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
												 pOtherGroup->ubGroupID, pOtherGroup->ubSectorY + 'A' - 1, pOtherGroup->ubSectorX, pSoldier->name,
												 pSoldier->name );
				}
				else if( iGroups > 1 && iMismatches == iGroups - 1 )
				{ //This is the error that is being targetted.  This means that the merc exists in multiple groups though the merc
					//knows what group he is supposed to be in.  This error can be corrected, by manually removing the merc from all
					//mismatch groups.  This is indicative of a merc being reassigned to another group without removing him first.

					//Get a pointer to the first mismatch group that contains the merc
					iMismatches = 0;
					iGroups = 0;
					const GROUP* pOtherGroup = NULL;
					CFOR_ALL_PLAYER_GROUPS(pGroup)
					{
						CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
						{
							if( pPlayer->pSoldier == pSoldier )
							{
								if( pSoldier->ubGroupID != pGroup->ubGroupID )
								{
									pOtherGroup = pGroup;
									iMismatches++;
								}
								iGroups++;
								break;
							}
						}
					}
					const GROUP* const pGroup = GetGroup(pSoldier->ubGroupID);
					Assert( pGroup );
					Assert( pOtherGroup );

					swprintf(str, lengthof(str), L"%ls in %c%d has been found in multiple groups.  The group he/she is supposed "
												 L"to be in is group %d in %c%d, but %ls was also found to be in group %d in %c%d.  %ls was found in %d groups "
												 L"total.  Please send screenshot, PRIOR save (corrected by time you read this), and any theories.",
												 pSoldier->name, pSoldier->sSectorY + 'A' - 1, pSoldier->sSectorX,
												 pGroup->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
												 pSoldier->name, pOtherGroup->ubGroupID, pOtherGroup->ubSectorY + 'A' - 1, pOtherGroup->ubSectorX,
												 pSoldier->name, iGroups );
				}
				else if( !iGroups )
				{ //The merc cannot be found in any group!  This should never happen!  We will assign the merc into his
					//own unique squad as a correction.
					swprintf(str, lengthof(str), L"%ls in %c%d cannot be found in any group.  %ls will be assigned to a unique group/squad.  "
												 L"Please provide details on how you think this may have happened.  Send screenshot and PRIOR save.  Do not send a save "
												 L"you create after this point as the info will have been corrected by then.",
												 pSoldier->name, pSoldier->sSectorY + 'A' - 1, pSoldier->sSectorX, pSoldier->name );
				}
			}

			//CORRECT THE ERRORS NOW
			if( iGroups == 1 && iMismatches == 1 )
			{ //We have a very serious problem, as we have no way to know which group this merc is supposed to be in.
				//This problem cannot be corrected (so we will assign the merc to his own unique squad).
				RemovePlayersFromAllMismatchGroups(*pSoldier);
				AddCharacterToUniqueSquad( pSoldier );
			}
			else if( iGroups > 1 && iMismatches == iGroups - 1 )
			{ //This is the error that is being targetted.  This means that the merc exists in multiple groups though the merc
				//knows what group he is supposed to be in.  This error can be corrected, by manually removing the merc from all
				//mismatch groups.  This is indicative of a merc being reassigned to another group without removing him first.
				RemovePlayersFromAllMismatchGroups(*pSoldier);
				pSoldier->ubGroupID = ubGroupID;
			}
			else if( !iGroups )
			{ //The merc cannot be found in any group!  This should never happen!  We will assign the merc into his
				//own unique squad as a correction.
				AddCharacterToUniqueSquad( pSoldier );
			}
		}
	}
	if( iNumErrors )
	{ //The first error to be detected is the one responsible for building the strings.  We will simply append another string containing
		//the total number of detected errors.
		wchar_t tempstr[128];
		swprintf(tempstr, lengthof(tempstr), L"  A total of %d related errors have been detected.", iNumErrors );
		wcscat( str, tempstr );
		SAIReportError( str );
	}
}
#endif


#ifdef JA2BETAVERSION
void SAIReportError(const wchar_t* wErrorString)
{
	// runtime static only, don't save
	#ifdef JA2TESTVERSION
		static BOOLEAN fReportedAlready = FALSE;
	#else
		BOOLEAN fReportedAlready = FALSE; //so it can loop
	#endif

	if ( !fReportedAlready )
	{
		StopTimeCompression();

		// report the error
		if( guiCurrentScreen != SAVE_LOAD_SCREEN )
		{
			DoScreenIndependantMessageBox( wErrorString, MSG_BOX_FLAG_OK, NULL );
		}
		else
		{
			ScreenMsg( FONT_LTBLUE, MSG_BETAVERSION, wErrorString );
		}
		if( guiCurrentScreen == AIVIEWER_SCREEN )
		{
			char str[512];
			sprintf( str, "%ls\n", wErrorString );
			DebugMsg(TOPIC_JA2SAI, DBG_LEVEL_1, str);
		}

		// this should keep it from repeating endlessly and allow player to save/bail
		fReportedAlready = TRUE;
	}
}
#endif


#ifdef JA2BETAVERSION
static void ClearStrategicLog(void);
#endif


void InitStrategicAI()
{
	INT32 i, cnt, iRandom;
	INT32 iEliteChance, iTroopChance, iAdminChance;
	INT32 iWeight;
	INT32 iStartPop, iDesiredPop, iPriority;
	SECTORINFO *pSector = NULL;
	GROUP *pGroup;
	UINT8 ubNumTroops;
	//Initialize the basic variables.

	gbPadding2[0]						= 0;
	gbPadding2[1]						= 0;
	gbPadding2[2]						= 0;
	gfExtraElites						= FALSE;
	giGarrisonArraySize			= 0;
	giPatrolArraySize				= 0;
	giForcePercentage				= 0;
	giArmyAlertness					= 0;
	giArmyAlertnessDecay		= 0;
	gubNumAwareBattles			= 0;
	gfQueenAIAwake					= FALSE;
	giReinforcementPool			= 0;
	giReinforcementPoints		= 0;
	giRequestPoints					= 0;
	gubSAIVersion						= SAI_VERSION;
	gubQueenPriorityPhase		= 0;
	gfFirstBattleMeanwhileScenePending = FALSE;
	gfMassFortificationOrdered = FALSE;
	gubMinEnemyGroupSize		= 0;
	gubHoursGracePeriod			= 0;
	gusPlayerBattleVictories = 0;
	gfUseAlternateQueenPosition = FALSE;

	#ifdef JA2BETAVERSION
		ClearStrategicLog();
	#endif
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			giReinforcementPool		= EASY_QUEENS_POOL_OF_TROOPS;
			giForcePercentage			= EASY_INITIAL_GARRISON_PERCENTAGES;
			giArmyAlertness				= EASY_ENEMY_STARTING_ALERT_LEVEL;
			giArmyAlertnessDecay	= EASY_ENEMY_STARTING_ALERT_DECAY;
			gubMinEnemyGroupSize	= EASY_MIN_ENEMY_GROUP_SIZE;
			gubHoursGracePeriod   = EASY_GRACE_PERIOD_IN_HOURS;
			// 475 is 7:55am in minutes since midnight, the time the game starts on day 1
			AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, 475 + EASY_TIME_EVALUATE_IN_MINUTES + Random( EASY_TIME_EVALUATE_VARIANCE ), 0 );
			break;
		case DIF_LEVEL_MEDIUM:
			giReinforcementPool		= NORMAL_QUEENS_POOL_OF_TROOPS;
			giForcePercentage			= NORMAL_INITIAL_GARRISON_PERCENTAGES;
			giArmyAlertness				= NORMAL_ENEMY_STARTING_ALERT_LEVEL;
			giArmyAlertnessDecay	= NORMAL_ENEMY_STARTING_ALERT_DECAY;
			gubMinEnemyGroupSize	= NORMAL_MIN_ENEMY_GROUP_SIZE;
			gubHoursGracePeriod   = NORMAL_GRACE_PERIOD_IN_HOURS;
			AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, 475 + NORMAL_TIME_EVALUATE_IN_MINUTES + Random( NORMAL_TIME_EVALUATE_VARIANCE ), 0 );
			break;
		case DIF_LEVEL_HARD:
			giReinforcementPool		= HARD_QUEENS_POOL_OF_TROOPS;
			giForcePercentage			= HARD_INITIAL_GARRISON_PERCENTAGES;
			giArmyAlertness				= HARD_ENEMY_STARTING_ALERT_LEVEL;
			giArmyAlertnessDecay	= HARD_ENEMY_STARTING_ALERT_DECAY;
			gubMinEnemyGroupSize	= HARD_MIN_ENEMY_GROUP_SIZE;
			gubHoursGracePeriod   = HARD_GRACE_PERIOD_IN_HOURS;
			AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, 475 + HARD_TIME_EVALUATE_IN_MINUTES + Random( HARD_TIME_EVALUATE_VARIANCE ), 0 );
			break;
	}

	//Initialize the sectorinfo structure so all sectors don't point to a garrisonID.
	for( i = 0; i <= 255; i++ )
	{
		SectorInfo[ i ].ubGarrisonID = NO_GARRISON;
	}

	//copy over the original army composition as it does get modified during the campaign.  This
	//bulletproofs starting the game over again.
	memcpy( gArmyComp, gOrigArmyComp, sizeof( gArmyComp ) );

	//Eliminate more perimeter defenses on the easier levels.
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			gArmyComp[ LEVEL2_DEFENCE ].bDesiredPopulation = 0;
			gArmyComp[ LEVEL2_DEFENCE ].bStartPopulation = 0;
			gArmyComp[ LEVEL3_DEFENCE ].bDesiredPopulation = 0;
			gArmyComp[ LEVEL3_DEFENCE ].bStartPopulation = 0;
			break;
		case DIF_LEVEL_MEDIUM:
			gArmyComp[ LEVEL3_DEFENCE ].bDesiredPopulation = 0;
			gArmyComp[ LEVEL3_DEFENCE ].bStartPopulation = 0;
			break;
	}
	//initialize the patrol group definitions
	giPatrolArraySize = sizeof( gOrigPatrolGroup ) / sizeof( PATROL_GROUP );
	if( !gPatrolGroup )
	{ //Allocate it (otherwise, we just overwrite it because the size never changes)
		gPatrolGroup = MALLOCN(PATROL_GROUP, lengthof(gOrigPatrolGroup));
	}
	memcpy( gPatrolGroup, gOrigPatrolGroup, sizeof( gOrigPatrolGroup ) );

	gubPatrolReinforcementsDenied = MALLOCNZ(UINT8, giPatrolArraySize);

	//initialize the garrison group definitions
	giGarrisonArraySize = sizeof( gOrigGarrisonGroup ) / sizeof( GARRISON_GROUP );
	if( !gGarrisonGroup )
	{
		gGarrisonGroup = MALLOCN(GARRISON_GROUP, lengthof(gOrigGarrisonGroup));
	}
	memcpy( gGarrisonGroup, gOrigGarrisonGroup, sizeof( gOrigGarrisonGroup ) );

	gubGarrisonReinforcementsDenied = MALLOCNZ(UINT8, giGarrisonArraySize);

	//Modify initial force sizes?
	if( giForcePercentage != 100 )
	{ //The initial force sizes are being modified, so go through each of the army compositions
		//and adjust them accordingly.
		for( i = 0; i < NUM_ARMY_COMPOSITIONS; i++ )
		{
			if( i != QUEEN_DEFENCE )
			{
				gArmyComp[ i ].bDesiredPopulation = (INT8)MIN( MAX_STRATEGIC_TEAM_SIZE, (gArmyComp[ i ].bDesiredPopulation * giForcePercentage / 100) );
				if( gArmyComp[ i ].bStartPopulation != MAX_STRATEGIC_TEAM_SIZE )
				{ //if the value is MAX_STRATEGIC_TEAM_SIZE, then that means the particular sector is a spawning location.
					//Don't modify the value if it is MAX_STRATEGIC_TEAM_SIZE.  Everything else is game.
					gArmyComp[ i ].bStartPopulation = (INT8)MIN( MAX_STRATEGIC_TEAM_SIZE, (gArmyComp[ i ].bStartPopulation * giForcePercentage / 100) );
				}
			}
			else
			{
				gArmyComp[ i ].bDesiredPopulation = (INT8)MIN( 32, (gArmyComp[ i ].bDesiredPopulation * giForcePercentage / 100) );
				gArmyComp[ i ].bStartPopulation = gArmyComp[ i ].bDesiredPopulation;
			}
		}
		for( i = 0; i < giPatrolArraySize; i++ )
		{ //force modified range within 1-MAX_STRATEGIC_TEAM_SIZE.
			gPatrolGroup[ i ].bSize = (INT8)MAX( gubMinEnemyGroupSize, MIN( MAX_STRATEGIC_TEAM_SIZE, (gPatrolGroup[ i ].bSize * giForcePercentage / 100 ) ) );
		}
	}

	//Now, initialize the garrisons based on the initial sizes (all variances are plus or minus 1).
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
		pSector->ubGarrisonID = (UINT8)i;
		iStartPop = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bStartPopulation;
		iDesiredPop = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bDesiredPopulation;
		iPriority = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bPriority;
		iEliteChance = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bElitePercentage;
		iTroopChance = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bTroopPercentage + iEliteChance;
		iAdminChance = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bAdminPercentage;

		switch( gGarrisonGroup[ i ].ubComposition )
		{
			case ROADBLOCK:
				pSector->uiFlags |= SF_ENEMY_AMBUSH_LOCATION;
				if( Chance( 20 ) )
					iStartPop = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bDesiredPopulation;
				else
					iStartPop = 0;
				break;
			case SANMONA_SMALL:
				iStartPop = 0; //not appropriate until Kingpin is killed.
				break;
		}

		if( iStartPop )
		{
			if( gGarrisonGroup[ i ].ubSectorID != SEC_P3 )
			{
				// if population is less than maximum
				if( iStartPop != MAX_STRATEGIC_TEAM_SIZE )
				{
					// then vary it a bit (+/- 25%)
					iStartPop = iStartPop * ( 100 + ( Random ( 51 ) - 25 ) ) / 100;
				}

				iStartPop = MAX( gubMinEnemyGroupSize, MIN( MAX_STRATEGIC_TEAM_SIZE, iStartPop ) );
			}
			cnt = iStartPop;

			if( iAdminChance )
			{
				pSector->ubNumAdmins = iAdminChance * iStartPop / 100;
			}
			else while( cnt-- )
			{ //for each person, randomly determine the types of each soldier.
				{
					iRandom = Random( 100 );
					if( iRandom < iEliteChance )
					{
						pSector->ubNumElites++;
					}
					else if( iRandom < iTroopChance )
					{
						pSector->ubNumTroops++;
					}
				}
			}
			switch( gGarrisonGroup[ i ].ubComposition )
			{
				case CAMBRIA_DEFENCE:
				case CAMBRIA_MINE:
				case ALMA_MINE:
				case GRUMM_MINE:
					//Fill up extra start slots with troops
					pSector->ubNumTroops = (UINT8)(iStartPop -= pSector->ubNumAdmins);
					break;
				case DRASSEN_AIRPORT:
				case DRASSEN_DEFENCE:
				case DRASSEN_MINE:
					pSector->ubNumAdmins = (UINT8)MAX( 5, pSector->ubNumAdmins );
					break;
				case TIXA_PRISON:
					pSector->ubNumAdmins = (UINT8)MAX( 8, pSector->ubNumAdmins );
					break;

			}
		}
		if( iAdminChance && pSector->ubNumAdmins < gubMinEnemyGroupSize )
		{
			pSector->ubNumAdmins = gubMinEnemyGroupSize;
		}
		//Calculate weight (range is -20 to +20 before multiplier).
		//The multiplier of 3 brings it to a range of -96 to +96 which is
		//close enough to a plus/minus 100%.  The resultant percentage is then
		//converted based on the priority.
		iWeight = (iDesiredPop - iStartPop) * 3;
		if( iWeight > 0 )
		{ //modify it by it's priority.
			//generates a value between 2 and 100
			iWeight = iWeight * iPriority / 96;
			iWeight = MAX( iWeight, 2 );
			giRequestPoints += iWeight;
		}
		else if( iWeight < 0 )
		{ //modify it by it's reverse priority
			//generates a value between -2 and -100
			iWeight = iWeight * (100-iPriority) / 96;
			iWeight = MIN( iWeight, -2 );
			giReinforcementPoints -= iWeight;
		}
		gGarrisonGroup[ i ].bWeight = (INT8)iWeight;

		//Now post an event which allows them to check adjacent sectors periodically.
		//Spread them out so that they process at different times.
		AddPeriodStrategicEventWithOffset( EVENT_CHECK_ENEMY_CONTROLLED_SECTOR, 140 - 20 * gGameOptions.ubDifficultyLevel + Random( 4 ), 475 + i, gGarrisonGroup[ i ].ubSectorID );
	}
	//Now, initialize each of the patrol groups
	for( i = 0; i < giPatrolArraySize; i++ )
	{	// IGNORE COMMENT, FEATURE REMOVED!
		//Some of the patrol groups aren't there at the beginning of the game.  This is
		//based on the difficulty settings in the above patrol table.
		//if( gPatrolGroup[ i ].ubUNUSEDStartIfDifficulty <= gGameOptions.ubDifficultyLevel )
		{ //Add this patrol group now.
			ubNumTroops = (UINT8)(gPatrolGroup[ i ].bSize + Random( 3 ) - 1);
			ubNumTroops = (UINT8)MAX( gubMinEnemyGroupSize, MIN( MAX_STRATEGIC_TEAM_SIZE, ubNumTroops ) );
			//ubNumTroops = (UINT8)MAX( gubMinEnemyGroupSize, MIN( MAX_STRATEGIC_TEAM_SIZE, gPatrolGroup[ i ].bSize + Random( 3 ) - 1 ) );
			//Note on adding patrol groups...
			//The patrol group can't actually start on the first waypoint, so we set it to the second way
			//point for initialization, and then add the waypoints from 0 up
			pGroup = CreateNewEnemyGroupDepartingFromSector( gPatrolGroup[ i ].ubSectorID[ 1 ], 0, ubNumTroops, 0 );

			if( i == 3 || i == 4 )
			{ //Special case:  Two patrol groups are administrator groups -- rest are troops
				pGroup->pEnemyGroup->ubNumAdmins = pGroup->pEnemyGroup->ubNumTroops;
				pGroup->pEnemyGroup->ubNumTroops = 0;
			}
			gPatrolGroup[ i ].ubGroupID = pGroup->ubGroupID;
			pGroup->pEnemyGroup->ubIntention = PATROL;
			pGroup->ubMoveType = ENDTOEND_FORWARDS;
			AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 0 ] );
			AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 1 ] );
			if( gPatrolGroup[ i ].ubSectorID[ 2 ] )
			{ //Add optional waypoints if included.
				AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 2 ] );
				if( gPatrolGroup[ i ].ubSectorID[ 3 ] )
					AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 3 ] );
			}
			RandomizePatrolGroupLocation( pGroup );
			ValidateGroup( pGroup );
		}
		//else
		//{ //we aren't creating this patrol group at the beginning of the game, so we
			//need to set up the weighting values to prioritize it's reinforcement request so that
			//it gets filled up later in the game.
		//	iWeight = gPatrolGroup[ i ].bSize * 3 * gPatrolGroup[ i ].bPriority / 96;
		//	gPatrolGroup[ i ].bWeight = (INT8)iWeight;
		//	giRequestPoints += iWeight;
		//}
	}

	//Setup the flags for the four sam sites.
	SectorInfo[SEC_D2].uiFlags |= SF_SAM_SITE;
	SectorInfo[SEC_D15].uiFlags |= SF_SAM_SITE;
	SectorInfo[SEC_I8].uiFlags |= SF_SAM_SITE;
	SectorInfo[SEC_N4].uiFlags |= SF_SAM_SITE;

	//final thing to do is choose 1 cache map out of 5 possible maps.  Simply select the sector randomly,
	//set up the flags to use the alternate map, then place 8-12 regular troops there (no ai though).
	//changing MAX_STRATEGIC_TEAM_SIZE may require changes to to the defending force here.
	switch( Random( 5 ) )
	{
		case 0:	pSector = &SectorInfo[ SEC_E11 ]; break;
		case 1:	pSector = &SectorInfo[ SEC_H5 ]; break;
		case 2:	pSector = &SectorInfo[ SEC_H10 ]; break;
		case 3:	pSector = &SectorInfo[ SEC_J12 ]; break;
		case 4:	pSector = &SectorInfo[ SEC_M9 ]; break;
	}
	pSector->uiFlags |= SF_USE_ALTERNATE_MAP;
	pSector->ubNumTroops = (UINT8)(6 + gGameOptions.ubDifficultyLevel * 2);

	ValidateWeights( 1 );
}

void KillStrategicAI()
{
	if( gPatrolGroup )
	{
		MemFree( gPatrolGroup );
		gPatrolGroup = NULL;
	}
	if( gGarrisonGroup )
	{
		MemFree( gGarrisonGroup );
		gGarrisonGroup = NULL;
	}
	if( gubPatrolReinforcementsDenied )
	{
		MemFree( gubPatrolReinforcementsDenied );
		gubPatrolReinforcementsDenied = NULL;
	}
	if( gubGarrisonReinforcementsDenied )
	{
		MemFree( gubGarrisonReinforcementsDenied );
		gubGarrisonReinforcementsDenied = NULL;
	}
	DeleteAllStrategicEventsOfType( EVENT_EVALUATE_QUEEN_SITUATION );
}

BOOLEAN OkayForEnemyToMoveThroughSector( UINT8 ubSectorID )
{
	SECTORINFO *pSector;
	pSector = &SectorInfo[ ubSectorID ];
	if( pSector->uiTimeLastPlayerLiberated && pSector->uiTimeLastPlayerLiberated + (gubHoursGracePeriod * 3600) > GetWorldTotalSeconds() )
	{
		return FALSE;
	}
	return TRUE;
}


static BOOLEAN EnemyPermittedToAttackSector(GROUP** pGroup, UINT8 ubSectorID)
{
	SECTORINFO *pSector;
	BOOLEAN fPermittedToAttack = TRUE;

	pSector = &SectorInfo[ ubSectorID ];
	fPermittedToAttack = OkayForEnemyToMoveThroughSector( ubSectorID );
	if( pGroup && *pGroup && pSector->ubGarrisonID != NO_GARRISON )
	{
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			GROUP *pPendingGroup;
			pPendingGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			if( pPendingGroup == *pGroup )
			{
				if( fPermittedToAttack )
				{
					if( GroupAtFinalDestination( *pGroup ) )
					{ //High priority reinforcements have arrived.  This overrides most other situations.
						return TRUE;
					}
				}
				else
				{ //Reassign the group
					ReassignAIGroup( pGroup );
				}
			}
		}
	}
	if( !fPermittedToAttack )
	{
		return FALSE;
	}
	//If Hill-billies are alive, then enemy won't attack the sector.
	switch( ubSectorID )
	{
		case SEC_F10:
			//Hill-billy farm -- not until hill billies are dead.
			if (CheckFact(FACT_HILLBILLIES_KILLED, FALSE))
				return FALSE;
			break;
		case SEC_A9:
		case SEC_A10:
			//Omerta -- not until Day 2 at 7:45AM.
			if( GetWorldTotalMin() < 3345 )
				return FALSE;
			break;
		case SEC_B13:
		case SEC_C13:
		case SEC_D13:
			//Drassen -- not until Day 3 at 6:30AM.
			if( GetWorldTotalMin() < 4710 )
				return FALSE;
			break;
		case SEC_C5:
		case SEC_C6:
		case SEC_D5:
			//San Mona -- not until Kingpin is dead.
			return CheckFact(FACT_KINGPIN_DEAD, 0);

		case SEC_G1:
			if( PlayerSectorDefended( SEC_G2 ) && (PlayerSectorDefended( SEC_H1 ) || PlayerSectorDefended( SEC_H2 )) )
			{
				return FALSE;
			}
			break;
		case SEC_H2:
			if( PlayerSectorDefended( SEC_H2 ) && (PlayerSectorDefended( SEC_G1 ) || PlayerSectorDefended( SEC_G2 )) )
			{
				return FALSE;
			}
			break;
	}
	return TRUE;
}


enum SAIMOVECODE
{
	DIRECT,
	EVASIVE,
	STAGE
};
static void MoveSAIGroupToSector(GROUP** pGroup, UINT8 ubSectorID, UINT32 uiMoveCode, UINT8 ubIntention);


static BOOLEAN HandlePlayerGroupNoticedByPatrolGroup(const GROUP* const pPlayerGroup, GROUP* pEnemyGroup)
{
	UINT16 usDefencePoints;
	UINT16 usOffensePoints;

	UINT8 const ubSectorID = SECTOR(pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY);
	usOffensePoints = pEnemyGroup->pEnemyGroup->ubNumAdmins * 2 +
										pEnemyGroup->pEnemyGroup->ubNumTroops * 4 +
										pEnemyGroup->pEnemyGroup->ubNumElites * 6;
	if( PlayerForceTooStrong( ubSectorID, usOffensePoints, &usDefencePoints ) )
	{
		RequestAttackOnSector( ubSectorID, usDefencePoints );
		return FALSE;
	}
	//For now, automatically attack.
	if( pPlayerGroup->ubNextX )
	{
		MoveSAIGroupToSector( &pEnemyGroup, (UINT8)SECTOR( pPlayerGroup->ubNextX, pPlayerGroup->ubNextY ), DIRECT, PURSUIT );

		#ifdef JA2BETAVERSION
			LogStrategicEvent( "Enemy group at %c%d detected player group at %c%d and is moving to intercept them at %c%d.",
				pEnemyGroup->ubSectorY + 'A' - 1, pEnemyGroup->ubSectorX,
				pPlayerGroup->ubSectorY + 'A' - 1, pPlayerGroup->ubSectorX,
				pPlayerGroup->ubNextY + 'A' - 1, pPlayerGroup->ubNextX );
		#endif
	}
	else
	{
		MoveSAIGroupToSector( &pEnemyGroup, (UINT8)SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY ), DIRECT, PURSUIT );

		#ifdef JA2BETAVERSION
			LogStrategicEvent( "Enemy group at %c%d detected player group at %c%d and is moving to intercept them at %c%d.",
				pEnemyGroup->ubSectorY + 'A' - 1, pEnemyGroup->ubSectorX,
				pPlayerGroup->ubSectorY + 'A' - 1, pPlayerGroup->ubSectorX,
				pPlayerGroup->ubSectorY + 'A' - 1, pPlayerGroup->ubSectorX );
		#endif
	}
	return TRUE;
}


static void ConvertGroupTroopsToComposition(GROUP* pGroup, INT32 iCompositionID);
static void RemoveSoldiersFromGarrisonBasedOnComposition(INT32 iGarrisonID, UINT8 ubSize);


static void HandlePlayerGroupNoticedByGarrison(const GROUP* const pPlayerGroup, const UINT8 ubSectorID)
{
	SECTORINFO *pSector;
	GROUP *pGroup;
	INT32 iReinforcementsApproved;
	UINT16 usOffensePoints, usDefencePoints;
	UINT8 ubEnemies;
	pSector = &SectorInfo[ ubSectorID ];
	//First check to see if the player is at his final destination.
	if( !GroupAtFinalDestination( pPlayerGroup ) )
	{
		return;
	}
	usOffensePoints = pSector->ubNumAdmins * 2 +
										pSector->ubNumTroops * 4 +
										pSector->ubNumElites * 6;
	if( PlayerForceTooStrong( ubSectorID, usOffensePoints, &usDefencePoints ) )
	{
		RequestAttackOnSector( ubSectorID, usDefencePoints );
		return;
	}

	if( pSector->ubGarrisonID != NO_GARRISON )
	{
		//Decide whether or not they will attack them with some of the troops.
		ubEnemies = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
		iReinforcementsApproved = (ubEnemies - gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bDesiredPopulation / 2);
		if( iReinforcementsApproved*2 > pPlayerGroup->ubGroupSize*3 && iReinforcementsApproved > gubMinEnemyGroupSize )
		{ //Then enemy's available outnumber the player by at least 3:2, so attack them.
			pGroup = CreateNewEnemyGroupDepartingFromSector( ubSectorID, 0, (UINT8)iReinforcementsApproved, 0 );

			ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition );

			MoveSAIGroupToSector( &pGroup, (UINT8)SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY ), DIRECT, REINFORCEMENTS );

			RemoveSoldiersFromGarrisonBasedOnComposition( pSector->ubGarrisonID, pGroup->ubGroupSize );

			if( pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins > MAX_STRATEGIC_TEAM_SIZE )
			{
				#ifdef JA2BETAVERSION
					LogStrategicEvent( "ERROR:  Sector %c%d now has %d enemies (max %d).",
						gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID / 16 + 'A' , gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID % 16,
						pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins, MAX_STRATEGIC_TEAM_SIZE );
				#endif
			}

			#ifdef JA2BETAVERSION
				LogStrategicEvent( "Enemy garrison at %c%d detected stopped player group at %c%d and is sending %d troops to attack.",
					gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID / 16 + 'A' , gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID % 16,
					pPlayerGroup->ubSectorY + 'A' - 1, pPlayerGroup->ubSectorX,
					pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumTroops );
			#endif
		}
	}
}


static BOOLEAN HandleMilitiaNoticedByPatrolGroup(UINT8 ubSectorID, GROUP* pEnemyGroup)
{
	//For now, automatically attack.
	UINT16 usOffensePoints, usDefencePoints;
	UINT8 ubSectorX = (UINT8)(ubSectorID % 16) + 1;
	UINT8 ubSectorY = (UINT8)(ubSectorID / 16) + 1;
	usOffensePoints = pEnemyGroup->pEnemyGroup->ubNumAdmins * 2 +
										pEnemyGroup->pEnemyGroup->ubNumTroops * 4 +
										pEnemyGroup->pEnemyGroup->ubNumElites * 6;
	if( PlayerForceTooStrong( ubSectorID, usOffensePoints, &usDefencePoints ) )
	{
		RequestAttackOnSector( ubSectorID, usDefencePoints );
		return FALSE;
	}

	MoveSAIGroupToSector( &pEnemyGroup, (UINT8)SECTOR( ubSectorX, ubSectorY ), DIRECT, REINFORCEMENTS );

	#ifdef JA2BETAVERSION
		LogStrategicEvent( "Enemy group at %c%d detected militia at %c%d and is moving to attack them.",
			pEnemyGroup->ubSectorY + 'A' - 1, pEnemyGroup->ubSectorX,
			ubSectorY + 'A' - 1, ubSectorX );
	#endif
	return FALSE;
}


static BOOLEAN AttemptToNoticeEmptySectorSucceeds(void)
{
	if( gubNumAwareBattles || gfAutoAIAware )
	{ //The queen is in high-alert and is searching for players.  All adjacent checks will automatically succeed.
		return TRUE;
	}
	if( DayTime() )
	{ //Day time chances are normal
		if( Chance( giArmyAlertness ) )
		{
			giArmyAlertness -= giArmyAlertnessDecay;
			//Minimum alertness should always be at least 0.
			giArmyAlertness = MAX( 0, giArmyAlertness );
			return TRUE;
		}
		giArmyAlertness++;
		return FALSE;
	}
	//Night time chances are one third of normal.
	if( Chance( giArmyAlertness/3 ) )
	{
		giArmyAlertness -= giArmyAlertnessDecay;
		//Minimum alertness should always be at least 0.
		giArmyAlertness = MAX( 0, giArmyAlertness );
		return TRUE;
	}
	if( Chance( 33 ) )
	{
		giArmyAlertness++;
	}
	return FALSE;
}


//Calling the function assumes that a player group is found to be adjacent to an enemy group.
//This uses the alertness rating to emulate the chance that the group will notice.  If it does
//notice, then the alertness drops accordingly to simulate a period of time where the enemy would
//not notice as much.  If it fails, the alertness gradually increases until it succeeds.
static BOOLEAN AttemptToNoticeAdjacentGroupSucceeds(void)
{
	if( gubNumAwareBattles || gfAutoAIAware )
	{ //The queen is in high-alert and is searching for players.  All adjacent checks will automatically succeed.
		return TRUE;
	}
	if( DayTime() )
	{ //Day time chances are normal
		if( Chance( giArmyAlertness ) )
		{
			giArmyAlertness -= giArmyAlertnessDecay;
			//Minimum alertness should always be at least 0.
			giArmyAlertness = MAX( 0, giArmyAlertness );
			return TRUE;
		}
		giArmyAlertness++;
		return FALSE;
	}
	//Night time chances are one third of normal.
	if( Chance( giArmyAlertness/3 ) )
	{
		giArmyAlertness -= giArmyAlertnessDecay;
		//Minimum alertness should always be at least 0.
		giArmyAlertness = MAX( 0, giArmyAlertness );
		return TRUE;
	}
	if( Chance( 33 ) )
	{
		giArmyAlertness++;
	}
	return FALSE;
}


static BOOLEAN HandleEmptySectorNoticedByPatrolGroup(GROUP* pGroup, UINT8 ubEmptySectorID)
{
	UINT8 ubGarrisonID;
	UINT8 ubSectorX = (UINT8)(ubEmptySectorID % 16) + 1;
	UINT8 ubSectorY = (UINT8)(ubEmptySectorID / 16) + 1;

	ubGarrisonID = SectorInfo[ ubEmptySectorID ].ubGarrisonID;
	if( ubGarrisonID != NO_GARRISON )
	{
		if( gGarrisonGroup[ ubGarrisonID ].ubPendingGroupID )
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	//Clear the patrol group's previous orders.
	RemoveGroupFromStrategicAILists(*pGroup);

	gGarrisonGroup[ ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
	MoveSAIGroupToSector( &pGroup, (UINT8)SECTOR( ubSectorX, ubSectorY ), DIRECT, REINFORCEMENTS );

	#ifdef JA2BETAVERSION
		LogStrategicEvent( "Enemy group at %c%d detected undefended sector at %c%d and is moving to retake it.",
			pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
			ubSectorY + 'A' - 1, ubSectorX );
	#endif

	return TRUE;
}


static void HandleEmptySectorNoticedByGarrison(UINT8 ubGarrisonSectorID, UINT8 ubEmptySectorID)
{
	SECTORINFO *pSector;
	GROUP *pGroup;
	UINT8 ubAvailableTroops;
	UINT8 ubSrcGarrisonID = 255, ubDstGarrisonID = 255;

	//Make sure that the destination sector doesn't already have a pending group.
	pSector = &SectorInfo[ ubEmptySectorID ];

	ubSrcGarrisonID = SectorInfo[ ubGarrisonSectorID ].ubGarrisonID;
	ubDstGarrisonID = SectorInfo[ ubEmptySectorID ].ubGarrisonID;

	if( ubSrcGarrisonID == NO_GARRISON || ubDstGarrisonID == NO_GARRISON )
	{ //Bad logic
		return;
	}

	if( gGarrisonGroup[ ubDstGarrisonID ].ubPendingGroupID )
	{ //A group is already on-route, so don't send anybody from here.
		return;
	}

	//An opportunity has arisen, where the enemy has noticed an important sector that is undefended.
	pSector = &SectorInfo[ ubGarrisonSectorID ];
	ubAvailableTroops = pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins;

	if( ubAvailableTroops >= gubMinEnemyGroupSize * 2 )
	{ //split group into two groups, and move one of the groups to the next sector.
		pGroup = CreateNewEnemyGroupDepartingFromSector( ubGarrisonSectorID, 0, (UINT8)(ubAvailableTroops / 2), 0 );
		ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ ubDstGarrisonID ].ubComposition );
		RemoveSoldiersFromGarrisonBasedOnComposition( ubSrcGarrisonID, pGroup->ubGroupSize );
		gGarrisonGroup[ ubDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
		MoveSAIGroupToSector( &pGroup, ubEmptySectorID, DIRECT, REINFORCEMENTS );
	}
}


static BOOLEAN ReinforcementsApproved(INT32 iGarrisonID, UINT16* pusDefencePoints)
{
	SECTORINFO *pSector;
	UINT16 usOffensePoints;
	UINT8 ubSectorX, ubSectorY;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	ubSectorX = (UINT8)SECTORX( gGarrisonGroup[ iGarrisonID ].ubSectorID );
	ubSectorY = (UINT8)SECTORY( gGarrisonGroup[ iGarrisonID ].ubSectorID );

	*pusDefencePoints = pSector->ubNumberOfCivsAtLevel[ GREEN_MILITIA ]		* 1 +
										pSector->ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] * 2 +
										pSector->ubNumberOfCivsAtLevel[ ELITE_MILITIA ]		* 3 +
										PlayerMercsInSector( ubSectorX, ubSectorY, 0 )		* 4;
	usOffensePoints = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bAdminPercentage * 2 +
										gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bTroopPercentage * 3 +
										gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bElitePercentage * 4 +
										gubGarrisonReinforcementsDenied[ iGarrisonID ];
	usOffensePoints = usOffensePoints * gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation / 100;

	if( usOffensePoints > *pusDefencePoints )
	{
		return TRUE;
	}
	//Before returning false, determine if reinforcements have been denied repeatedly.  If so, then
	//we might send an augmented force to take it back.
	if( gubGarrisonReinforcementsDenied[ iGarrisonID ] + usOffensePoints > *pusDefencePoints )
	{
		#ifdef JA2BETAVERSION
			LogStrategicEvent( "Sector %c%d will now recieve an %d extra troops due to multiple denials for reinforcements in the past for strong player presence.",
				ubSectorY + 'A' - 1, ubSectorX, gubGarrisonReinforcementsDenied[ iGarrisonID ] / 3 );
		#endif
		return TRUE;
	}
	//Reinforcements will have to wait.  For now, increase the reinforcements denied.  The amount increase is 20 percent
	//of the garrison's priority.
	gubGarrisonReinforcementsDenied[ iGarrisonID ] += (UINT8)(gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bPriority / 2);

	return FALSE;
}


static void EliminateSurplusTroopsForGarrison(GROUP* pGroup, SECTORINFO* pSector);
static void RecalculateGarrisonWeight(INT32 iGarrisonID);
static void RecalculatePatrolWeight(PATROL_GROUP&);


//if the group has arrived in a sector, and doesn't have any particular orders, then
//send him back where they came from.
//RETURNS TRUE if the group is deleted or told to move somewhere else.
//This is important as the calling function will need
//to abort processing of the group for obvious reasons.
static BOOLEAN EvaluateGroupSituation(GROUP* pGroup)
{
	SECTORINFO *pSector;
	GROUP *pPatrolGroup;
	INT32 i;

	ValidateWeights( 2 );

	if( !gfQueenAIAwake )
	{
		return FALSE;
	}
	Assert( !pGroup->fPlayer );
	if( pGroup->pEnemyGroup->ubIntention == PURSUIT )
	{ //Lost the player group that he was going to attack.  Return to original position.
		ReassignAIGroup( &pGroup );
		return TRUE;
	}
	else if( pGroup->pEnemyGroup->ubIntention == REINFORCEMENTS )
	{ //The group has arrived at the location where he is supposed to reinforce.
		//Step 1 -- Check for matching garrison location
		for( i = 0; i < giGarrisonArraySize; i++ )
		{
			if( gGarrisonGroup[ i ].ubSectorID == SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) &&
					gGarrisonGroup[ i ].ubPendingGroupID == pGroup->ubGroupID )
			{
				pSector = &SectorInfo[ SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) ];

				if( gGarrisonGroup[ i ].ubSectorID != SEC_P3 )
				{
					EliminateSurplusTroopsForGarrison( pGroup, pSector );
					pSector->ubNumAdmins = (UINT8)(pSector->ubNumAdmins + pGroup->pEnemyGroup->ubNumAdmins);
					pSector->ubNumTroops = (UINT8)(pSector->ubNumTroops + pGroup->pEnemyGroup->ubNumTroops);
					pSector->ubNumElites = (UINT8)(pSector->ubNumElites + pGroup->pEnemyGroup->ubNumElites);

					#ifdef JA2BETAVERSION
						LogStrategicEvent( "%d reinforcements have arrived to garrison sector %c%d",
							pGroup->pEnemyGroup->ubNumAdmins + pGroup->pEnemyGroup->ubNumTroops +
							pGroup->pEnemyGroup->ubNumElites, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
					#endif
					if( IsThisSectorASAMSector( pGroup->ubSectorX, pGroup->ubSectorY, 0 ) )
					{
						StrategicMap[ pGroup->ubSectorX + pGroup->ubSectorY * MAP_WORLD_X ].bSAMCondition = 100;
						UpdateSAMDoneRepair( pGroup->ubSectorX, pGroup->ubSectorY, 0 );
					}
				}
				else
				{ //The group was sent back to the queen's palace (probably because they couldn't be reassigned
					//anywhere else, but it is possible that the queen's sector is requesting the reinforcements.  In
					//any case, if the queen's sector is less than full strength, fill it up first, then
					//simply add the rest to the global pool.
					if( pSector->ubNumElites < MAX_STRATEGIC_TEAM_SIZE )
					{
						if( pSector->ubNumElites + pGroup->ubGroupSize >= MAX_STRATEGIC_TEAM_SIZE )
						{ //Fill up the queen's guards, then apply the rest to the reinforcement pool
							giReinforcementPool += MAX_STRATEGIC_TEAM_SIZE - pSector->ubNumElites;
							pSector->ubNumElites = MAX_STRATEGIC_TEAM_SIZE;
							#ifdef JA2BETAVERSION
								LogStrategicEvent( "%d reinforcements have arrived to garrison queen's sector.  The excess troops will be relocated to the reinforcement pool.",
									pGroup->ubGroupSize );
							#endif
						}
						else
						{ //Add all the troops to the queen's guard.
							pSector->ubNumElites += pGroup->ubGroupSize;
							#ifdef JA2BETAVERSION
								LogStrategicEvent( "%d reinforcements have arrived to garrison queen's sector.",
									pGroup->ubGroupSize, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
							#endif
						}
					}
					else
					{ //Add all the troops to the reinforcement pool as the queen's guard is at full strength.
						giReinforcementPool += pGroup->ubGroupSize;
						#ifdef JA2BETAVERSION
							LogStrategicEvent( "%d reinforcements have arrived at queen's sector and have been added to the reinforcement pool.",
								pGroup->ubGroupSize, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
						#endif
					}
				}

				SetThisSectorAsEnemyControlled(pGroup->ubSectorX, pGroup->ubSectorY, 0);
				RemoveGroup(*pGroup);
				RecalculateGarrisonWeight( i );

				return TRUE;
			}
		}
		//Step 2 -- Check for Patrol groups matching waypoint index.
		for( i = 0; i < giPatrolArraySize; i++ )
		{
			if( gPatrolGroup[ i ].ubSectorID[ 1 ] == SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) &&
					gPatrolGroup[ i ].ubPendingGroupID == pGroup->ubGroupID )
			{
				gPatrolGroup[ i ].ubPendingGroupID = 0;
				if( gPatrolGroup[ i ].ubGroupID && gPatrolGroup[ i ].ubGroupID != pGroup->ubGroupID )
				{ //cheat, and warp our reinforcements to them!
					pPatrolGroup = GetGroup( gPatrolGroup[ i ].ubGroupID );
					pPatrolGroup->pEnemyGroup->ubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
					pPatrolGroup->pEnemyGroup->ubNumElites += pGroup->pEnemyGroup->ubNumElites;
					pPatrolGroup->pEnemyGroup->ubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
					pPatrolGroup->ubGroupSize += (UINT8)(pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins );
					#ifdef JA2BETAVERSION
						LogStrategicEvent( "%d reinforcements have joined patrol group at sector %c%d (new size: %d)",
							pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
							pPatrolGroup->ubSectorY + 'A' - 1, pPatrolGroup->ubSectorX, pPatrolGroup->ubGroupSize );
					#endif
					if( pPatrolGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE )
					{
						UINT8 ubCut;
						#ifdef JA2BETAVERSION
						wchar_t str[512];
						swprintf(str, lengthof(str), L"Patrol group #%d in %c%d received too many reinforcements from group #%d that was created in %c%d.  Size truncated from %d to %d."
													 L"Please send Strategic Decisions.txt and PRIOR save.",
													 pPatrolGroup->ubGroupID, pPatrolGroup->ubSectorY + 'A' - 1, pPatrolGroup->ubSectorX,
													 pGroup->ubGroupID, SECTORY( pGroup->ubCreatedSectorID ) + 'A' - 1, SECTORX( pGroup->ubCreatedSectorID ),
													 pPatrolGroup->ubGroupSize, MAX_STRATEGIC_TEAM_SIZE );
						SAIReportError( str );
						#endif
						//truncate the group size.
						ubCut = pPatrolGroup->ubGroupSize - MAX_STRATEGIC_TEAM_SIZE;
						while( ubCut-- )
						{
							if( pGroup->pEnemyGroup->ubNumAdmins )
							{
								pGroup->pEnemyGroup->ubNumAdmins--;
								pPatrolGroup->pEnemyGroup->ubNumAdmins--;
							}
							else if( pGroup->pEnemyGroup->ubNumTroops )
							{
								pGroup->pEnemyGroup->ubNumTroops--;
								pPatrolGroup->pEnemyGroup->ubNumTroops--;
							}
							else if( pGroup->pEnemyGroup->ubNumElites )
							{
								pGroup->pEnemyGroup->ubNumElites--;
								pPatrolGroup->pEnemyGroup->ubNumElites--;
							}
						}
						pPatrolGroup->ubGroupSize = MAX_STRATEGIC_TEAM_SIZE;
						Assert( pPatrolGroup->pEnemyGroup->ubNumAdmins +
										pPatrolGroup->pEnemyGroup->ubNumTroops +
										pPatrolGroup->pEnemyGroup->ubNumElites == MAX_STRATEGIC_TEAM_SIZE );
					}
					RemoveGroup(*pGroup);
					RecalculatePatrolWeight(gPatrolGroup[i]);
					ValidateLargeGroup( pPatrolGroup );
				}
				else
				{ //the reinforcements have become the new patrol group (even if same group)
					gPatrolGroup[ i ].ubGroupID = pGroup->ubGroupID;
					pGroup->pEnemyGroup->ubIntention = PATROL;
					pGroup->ubMoveType = ENDTOEND_FORWARDS;
					RemoveGroupWaypoints(*pGroup);
					AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 0 ] );
					AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 1 ] );
					if( gPatrolGroup[ i ].ubSectorID[ 2 ] )
					{ //Add optional waypoints if included.
						AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 2 ] );
						if( gPatrolGroup[ i ].ubSectorID[ 3 ] )
							AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 3 ] );
					}

					#ifdef JA2BETAVERSION
						LogStrategicEvent( "%d soldiers have arrived to patrol area near sector %c%d",
							pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
							pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
					#endif
					RecalculatePatrolWeight(gPatrolGroup[i]);
				}
				return TRUE;
			}
		}
	}
	else
	{	//This is a floating group at his final destination...
		if( pGroup->pEnemyGroup->ubIntention != STAGING && pGroup->pEnemyGroup->ubIntention != REINFORCEMENTS )
		{
			ReassignAIGroup( &pGroup );
			return TRUE;
		}
	}
	ValidateWeights( 3 );
	return FALSE;
}


static void SendGroupToPool(GROUP** pGroup);


//returns TRUE if the group was deleted.
BOOLEAN StrategicAILookForAdjacentGroups( GROUP *pGroup )
{
	SECTORINFO *pSector;
	UINT8 ubNumEnemies;
	UINT8 ubSectorID;
	if( !gfQueenAIAwake )
	{ //The queen isn't aware the player's presence yet, so she is oblivious to any situations.

		if( !pGroup->fPlayer )
		{
			//Exception case!
			//In the beginning of the game, a group is sent to A9 after the first battle.  If you leave A9, when they arrive,
			//they will stay there indefinately because the AI isn't awake.  What we do, is if this is a group in A9, then
			//send them home.
			if( GroupAtFinalDestination( pGroup ) )
			{
				//Wake up the queen now, if she hasn't woken up already.
				WakeUpQueen();
				if( pGroup->ubSectorX == 9 && pGroup->ubSectorY == 1 ||
						pGroup->ubSectorX == 3 && pGroup->ubSectorY == 16 )
				{
					SendGroupToPool( &pGroup );
					if( !pGroup )
					{ //Group was transferred to the pool
						return TRUE;
					}
				}
			}
		}

		if( !gfQueenAIAwake )
		{
			return FALSE;
		}
	}
	if( !pGroup->fPlayer )
	{	//The enemy group has arrived at a new sector and now controls it.
		//Look in each of the four directions, and the alertness rating will
		//determine the chance to detect any players that may exist in that sector.
		GROUP* pEnemyGroup = pGroup;
		if( GroupAtFinalDestination( pEnemyGroup ) )
		{
			return EvaluateGroupSituation( pEnemyGroup );
		}
		ubSectorID = (UINT8)SECTOR( pEnemyGroup->ubSectorX, pEnemyGroup->ubSectorY );
		if( pEnemyGroup && pEnemyGroup->ubSectorY > 1 && EnemyPermittedToAttackSector( &pEnemyGroup, (UINT8)(ubSectorID - 16) ) )
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSectorX, pEnemyGroup->ubSectorY - 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if( CountAllMilitiaInSector( pEnemyGroup->ubSectorX, (UINT8)(pEnemyGroup->ubSectorY-1) ) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup( (UINT8)SECTOR( pEnemyGroup->ubSectorX, pEnemyGroup->ubSectorY-1 ), pEnemyGroup );
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID-16) );
			}
		}
		if( pEnemyGroup && pEnemyGroup->ubSectorX > 1 && EnemyPermittedToAttackSector( &pEnemyGroup, (UINT8)(ubSectorID - 1) ) )
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSectorX - 1, pEnemyGroup->ubSectorY);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if( CountAllMilitiaInSector( (UINT8)(pEnemyGroup->ubSectorX-1), pEnemyGroup->ubSectorY ) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup( (UINT8)SECTOR( pEnemyGroup->ubSectorX-1, pEnemyGroup->ubSectorY ), pEnemyGroup );
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID-1) );
			}
		}
		if( pEnemyGroup && pEnemyGroup->ubSectorY < 16 && EnemyPermittedToAttackSector( &pEnemyGroup, (UINT8)(ubSectorID + 16) ) )
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSectorX, pEnemyGroup->ubSectorY + 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if( CountAllMilitiaInSector( pEnemyGroup->ubSectorX, (UINT8)(pEnemyGroup->ubSectorY+1) ) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup( (UINT8)SECTOR( pEnemyGroup->ubSectorX, pEnemyGroup->ubSectorY+1 ), pEnemyGroup );
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID+16) );
			}
		}
		if( pEnemyGroup && pEnemyGroup->ubSectorX < 16 && EnemyPermittedToAttackSector( &pEnemyGroup, (UINT8)(ubSectorID + 1) ) )
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSectorX + 1, pEnemyGroup->ubSectorY);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if( CountAllMilitiaInSector( (UINT8)(pEnemyGroup->ubSectorX+1), pEnemyGroup->ubSectorY ) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup( (UINT8)SECTOR( pEnemyGroup->ubSectorX+1, pEnemyGroup->ubSectorY ), pEnemyGroup );
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID+1) );
			}
		}
		if( !pEnemyGroup )
		{ //group deleted.
			return TRUE;
		}
	}
	else
	{ //The player group has arrived at a new sector and now controls it.
		//Look in each of the four directions, and the enemy alertness rating will
		//determine if the enemy notices that the player is here.
		//Additionally, there are also stationary enemy groups that may also notice the
		//player's new presence.
		//NOTE:  Always returns false because it is the player group that we are handling.  We
		//       don't mess with the player group here!
		const GROUP* const pPlayerGroup = pGroup;
		if( pPlayerGroup->ubSectorZ )
			return FALSE;
		if( !EnemyPermittedToAttackSector( NULL, (UINT8)SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY ) ) )
			return FALSE;
		if( pPlayerGroup->ubSectorY > 1 )
		{
			GROUP* const pEnemyGroup = FindEnemyMovementGroupInSector(pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY - 1);
			if( pEnemyGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
				return FALSE;
			}
			pSector = &SectorInfo[ SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY-1 ) ];
			ubNumEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			if( ubNumEnemies && pSector->ubGarrisonID != NO_GARRISON && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, (UINT8)SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY-1 ) );
				return FALSE;
			}
		}
		if( pPlayerGroup->ubSectorX < 16 )
		{
			GROUP* const pEnemyGroup = FindEnemyMovementGroupInSector(pPlayerGroup->ubSectorX + 1, pPlayerGroup->ubSectorY);
			if( pEnemyGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
				return FALSE;
			}
			pSector = &SectorInfo[ SECTOR( pPlayerGroup->ubSectorX-1, pPlayerGroup->ubSectorY ) ];
			ubNumEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			if( ubNumEnemies && pSector->ubGarrisonID != NO_GARRISON && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, (UINT8)SECTOR( pPlayerGroup->ubSectorX-1, pPlayerGroup->ubSectorY ) );
				return FALSE;
			}
		}
		if( pPlayerGroup->ubSectorY < 16 )
		{
			GROUP* const pEnemyGroup = FindEnemyMovementGroupInSector(pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY + 1);
			if( pEnemyGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
				return FALSE;
			}
			pSector = &SectorInfo[ SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY+1 ) ];
			ubNumEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			if( ubNumEnemies && pSector->ubGarrisonID != NO_GARRISON && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, (UINT8)SECTOR( pPlayerGroup->ubSectorX, pPlayerGroup->ubSectorY+1 ) );
				return FALSE;
			}
		}
		if( pPlayerGroup->ubSectorX > 1 )
		{
			GROUP* const pEnemyGroup = FindEnemyMovementGroupInSector(pPlayerGroup->ubSectorX - 1, pPlayerGroup->ubSectorY);
			if( pEnemyGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
				return FALSE;
			}
			pSector = &SectorInfo[ SECTOR( pPlayerGroup->ubSectorX+1, pPlayerGroup->ubSectorY ) ];
			ubNumEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			if( ubNumEnemies && pSector->ubGarrisonID != NO_GARRISON && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, (UINT8)SECTOR( pPlayerGroup->ubSectorX+1, pPlayerGroup->ubSectorY ) );
				return FALSE;
			}
		}
	}
	return FALSE;
}

//This is called periodically for each enemy occupied sector containing garrisons.
void CheckEnemyControlledSector( UINT8 ubSectorID )
{
	SECTORINFO *pSector;
	UINT8 ubSectorX, ubSectorY;
	if( !gfQueenAIAwake )
	{
		return;
	}
	//First, determine if the sector is still owned by the enemy.
	pSector = &SectorInfo[ ubSectorID ];
	if( pSector->ubGarrisonID != NO_GARRISON )
	{
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{ //Look for a staging group.
			GROUP *pGroup;
			pGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			if( pGroup )
			{ //We have a staging group
				if( GroupAtFinalDestination( pGroup ) )
				{
					if( pGroup->pEnemyGroup->ubPendingReinforcements )
					{
						if( pGroup->pEnemyGroup->ubPendingReinforcements > 4 )
						{
							UINT8 ubNum = (UINT8)(3 + Random( 3 ));
							pGroup->pEnemyGroup->ubNumTroops += ubNum;
							pGroup->ubGroupSize += ubNum;
							pGroup->pEnemyGroup->ubPendingReinforcements -= ubNum;
							RecalculateGroupWeight(*pGroup);
							ValidateLargeGroup( pGroup );
						}
						else
						{
							pGroup->pEnemyGroup->ubNumTroops += pGroup->pEnemyGroup->ubPendingReinforcements;
							pGroup->ubGroupSize += pGroup->pEnemyGroup->ubPendingReinforcements;
							pGroup->pEnemyGroup->ubPendingReinforcements = 0;
							ValidateLargeGroup( pGroup );
						}
					}
					else if( SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) != gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID )
					{
						MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID, DIRECT, pGroup->pEnemyGroup->ubIntention );
					}
				}
				//else the group is on route to stage hopefully...
			}
		}
	}
	if( pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites )
	{

		//The sector is still controlled, so look around to see if there are any players nearby.
		ubSectorX = (UINT8)SECTORX( ubSectorID );
		ubSectorY = (UINT8)SECTORY( ubSectorID );
		if( ubSectorY > 1 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID - 16) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(bSectorX, ubSectorY - 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID-16) );
				return;
			}
		}
		if( ubSectorX < 16 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID + 1) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(ubSectorX + 1, ubSectorY);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID+1) );
				return;
			}
		}
		if( ubSectorY < 16 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID + 16) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(ubSectorX, ubSectorY + 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID+16) );
				return;
			}
		}
		if( ubSectorX > 1 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID - 1) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(ubSectorX - 1, ubSectorY);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID-1) );
				return;
			}
		}
	}
}


void RemoveGroupFromStrategicAILists(GROUP const& g)
{
	UINT8 const group_id = g.ubGroupID;
	for (INT32 i = 0; i < giPatrolArraySize; ++i)
	{
		PATROL_GROUP& pg = gPatrolGroup[i];
		if (pg.ubGroupID == group_id)
		{ // Patrol group was destroyed
			pg.ubGroupID = 0;
			RecalculatePatrolWeight(pg);
			return;
		}
		if (pg.ubPendingGroupID == group_id)
		{ // Group never arrived to reinforce
			pg.ubPendingGroupID = 0;
			return;
		}
	}
	for (INT32 i = 0; i < giGarrisonArraySize; ++i)
	{
		GARRISON_GROUP& gg = gGarrisonGroup[i];
		if (gg.ubPendingGroupID == group_id)
		{ // Group never arrived to reinforce
			gg.ubPendingGroupID = 0;
			return;
		}
	}
}


/* Recalculates a group's weight based on any changes.
 * @@@Alex, this is possibly missing in some areas. It is hard to ensure it is
 * everywhere with all the changes I've made. I'm sure you could probably find
 * some missing calls. */
static void RecalculatePatrolWeight(PATROL_GROUP& p)
{
	ValidateWeights(4);

	// First, remove the previous weight from the applicable field
	INT32 const prev_weight = p.bWeight;
	if (prev_weight > 0) giRequestPoints -= prev_weight;

	INT32 need_population;
	if (p.ubGroupID != 0)
	{
		need_population = p.bSize - GetGroup(p.ubGroupID)->ubGroupSize;
		if (need_population < 0)
		{
			p.bWeight = 0;
			ValidateWeights(27);
			return;
		}
	}
	else
	{
		need_population = p.bSize;
	}
	INT32 weight = need_population * 3 * p.bPriority / 96;
	weight = MIN(weight, 2);
	p.bWeight        = weight;
	giRequestPoints += weight;

	ValidateWeights(5);
}


//Recalculates a group's weight based on any changes.
//@@@Alex, this is possibly missing in some areas.  It is hard to ensure it is
//everywhere with all the changes I've made.  I'm sure you could probably find some missing calls.
static void RecalculateGarrisonWeight(INT32 iGarrisonID)
{
	SECTORINFO *pSector;
	INT32 iWeight, iPrevWeight;
	INT32 iDesiredPop, iCurrentPop, iPriority;

	ValidateWeights( 6 );

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iDesiredPop = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;
	iCurrentPop = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iPriority = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bPriority;

	//First, remove the previous weight from the applicable field.
	iPrevWeight = gGarrisonGroup[ iGarrisonID ].bWeight;
	if( iPrevWeight > 0 )
		giRequestPoints -= iPrevWeight;
	else if( iPrevWeight < 0 )
		giReinforcementPoints += iPrevWeight;

	//Calculate weight (range is -20 to +20 before multiplier).
	//The multiplier of 3 brings it to a range of -96 to +96 which is
	//close enough to a plus/minus 100%.  The resultant percentage is then
	//converted based on the priority.
	iWeight = (iDesiredPop - iCurrentPop) * 3;
	if( iWeight > 0 )
	{ //modify it by it's priority.
		//generates a value between 2 and 100
		iWeight = iWeight * iPriority / 96;
		iWeight = MAX( iWeight, 2 );
		giRequestPoints += iWeight;
	}
	else if( iWeight < 0 )
	{ //modify it by it's reverse priority
		//generates a value between -2 and -100
		iWeight = iWeight * (100-iPriority) / 96;
		iWeight = MIN( iWeight, -2 );
		giReinforcementPoints -= (INT8)iWeight;
	}

	gGarrisonGroup[ iGarrisonID ].bWeight = (INT8)iWeight;

	ValidateWeights( 7 );
}

void RecalculateSectorWeight( UINT8 ubSectorID )
{
	INT32 i;
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		if( gGarrisonGroup[ i ].ubSectorID == ubSectorID )
		{
			RecalculateGarrisonWeight( i );
			return;
		}
	}
}


static void TagSAIGroupWithGracePeriod(GROUP const&);


void RecalculateGroupWeight(GROUP const& g)
{
	for (INT32 i = 0; i != giPatrolArraySize; ++i)
	{
		PATROL_GROUP& p = gPatrolGroup[i];
		if (p.ubGroupID != g.ubGroupID) continue;
		if (g.ubGroupSize == 0)
		{
			TagSAIGroupWithGracePeriod(g);
			p.ubGroupID = 0;
		}
		RecalculatePatrolWeight(p);
		return;
	}
}


static BOOLEAN GarrisonCanProvideMinimumReinforcements(INT32 iGarrisonID);


static INT32 ChooseSuitableGarrisonToProvideReinforcements(INT32 iDstGarrisonID, INT32 iReinforcementsRequested)
{
	INT32 iSrcGarrisonID, iBestGarrisonID = NO_GARRISON;
	INT32 iReinforcementsAvailable;
	INT32 i, iRandom, iWeight;
	INT8 bBestWeight;
	UINT8 ubSectorID;

	//Check to see if we could send reinforcements from Alma.  Only Drassen/Cambria get preferred
	//service from Alma, due to it's proximity and Alma's purpose as a forward military base.
	ubSectorID = gGarrisonGroup[ iDstGarrisonID ].ubSectorID;
	switch( ubSectorID )
	{
		case SEC_B13: case SEC_C13: case SEC_D13: case SEC_D15:								//Drassen + nearby SAM site
		case SEC_F8:  case SEC_F9:  case SEC_G8:  case SEC_G9:  case SEC_H8:	//Cambria
			//reinforcements will be primarily sent from Alma whenever possible.

			//find which the first sector that contains Alma soldiers.
			for( i = 0; i < giGarrisonArraySize; i++ )
			{
				if( gGarrisonGroup[ i ].ubComposition == ALMA_DEFENCE )
					break;
			}
			iSrcGarrisonID = i;
			//which of these 4 Alma garrisons have the most reinforcements available?  It is
			//possible that none of these garrisons can provide any reinforcements.
			bBestWeight = 0;
			for( i = iSrcGarrisonID; i < iSrcGarrisonID + 4; i++ )
			{
				RecalculateGarrisonWeight( i );
				if( bBestWeight > gGarrisonGroup[ i ].bWeight && GarrisonCanProvideMinimumReinforcements( i ) )
				{
					bBestWeight = gGarrisonGroup[ i ].bWeight;
					iBestGarrisonID = i;
				}
			}
			//If we can provide reinforcements from Alma, then make sure that it can provide at least 67% of
			//the requested reinforcements.
			if( bBestWeight < 0 )
			{
				iReinforcementsAvailable = ReinforcementsAvailable( iBestGarrisonID );
				if( iReinforcementsAvailable * 100 >= iReinforcementsRequested * 67 )
				{ //This is the approved group to provide the reinforcements.
					return iBestGarrisonID;
				}
			}
			break;
	}

	//The Alma case either wasn't applicable or failed to have the right reinforcements.  Do a general weighted search.
	iRandom = Random( giReinforcementPoints );
	for( iSrcGarrisonID = 0; iSrcGarrisonID < giGarrisonArraySize; iSrcGarrisonID++ )
	{ //go through the garrisons
		RecalculateGarrisonWeight( iSrcGarrisonID );
		iWeight = -gGarrisonGroup[ iSrcGarrisonID ].bWeight;
		if( iWeight > 0 )
		{ //if group is able to provide reinforcements.
			if( iRandom < iWeight && GarrisonCanProvideMinimumReinforcements( iSrcGarrisonID ) )
			{
				iReinforcementsAvailable = ReinforcementsAvailable( iSrcGarrisonID );
				if( iReinforcementsAvailable * 100 >= iReinforcementsRequested * 67 )
				{ //This is the approved group to provide the reinforcements.
					return iSrcGarrisonID;
				}
			}
			iRandom -= iWeight;
		}
	}

	//So far we have failed on all accounts.  Now, simply process all the garrisons, and return the first garrison that can
	//provide the reinforcements.
	for( iSrcGarrisonID = 0; iSrcGarrisonID < giGarrisonArraySize; iSrcGarrisonID++ )
	{ //go through the garrisons
		RecalculateGarrisonWeight( iSrcGarrisonID );
		iWeight = -gGarrisonGroup[ iSrcGarrisonID ].bWeight;
		if( iWeight > 0 && GarrisonCanProvideMinimumReinforcements( iSrcGarrisonID ) )
		{ //if group is able to provide reinforcements.
			iReinforcementsAvailable = ReinforcementsAvailable( iSrcGarrisonID );
			if( iReinforcementsAvailable * 100 >= iReinforcementsRequested * 67 )
			{ //This is the approved group to provide the reinforcements.
				return iSrcGarrisonID;
			}
		}
	}

	//Well, if we get this far, the queen must be low on troops.  Send whatever we can.
	iRandom = Random( giReinforcementPoints );
	for( iSrcGarrisonID = 0; iSrcGarrisonID < giGarrisonArraySize; iSrcGarrisonID++ )
	{ //go through the garrisons
		RecalculateGarrisonWeight( iSrcGarrisonID );
		iWeight = -gGarrisonGroup[ iSrcGarrisonID ].bWeight;
		if( iWeight > 0 && GarrisonCanProvideMinimumReinforcements( iSrcGarrisonID ) )
		{ //if group is able to provide reinforcements.
			if( iRandom < iWeight )
			{
				iReinforcementsAvailable = ReinforcementsAvailable( iSrcGarrisonID );
				return iSrcGarrisonID;
			}
			iRandom -= iWeight;
		}
	}

	//Failed completely.
	return -1;
}


static void SendReinforcementsForGarrison(INT32 iDstGarrisonID, UINT16 usDefencePoints, GROUP** pOptionalGroup)
{
	INT32 iChance, iRandom, iSrcGarrisonID;
	INT32 iMaxReinforcementsAllowed, iReinforcementsAvailable, iReinforcementsRequested, iReinforcementsApproved;
	GROUP *pGroup;
	UINT8 ubSrcSectorX, ubSrcSectorY, ubDstSectorX, ubDstSectorY;
	UINT8 ubNumExtraReinforcements;
	UINT8 ubGroupSize;
	BOOLEAN fLimitMaxTroopsAllowable = FALSE;

	ValidateWeights( 8 );

	//Determine how many units the garrison needs.
	iReinforcementsRequested = GarrisonReinforcementsRequested( iDstGarrisonID, &ubNumExtraReinforcements );

	//The maximum number of reinforcements can't be offsetted past a certain point based on the
	//priority of the garrison.
	iMaxReinforcementsAllowed = //from 1 to 3 times the desired size of the normal force.
		gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bDesiredPopulation +
		gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bDesiredPopulation *
		gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bPriority / 50;

	if( iReinforcementsRequested + ubNumExtraReinforcements > iMaxReinforcementsAllowed )
	{ //adjust the extra reinforcements so that it doesn't exceed the maximum allowed.
		fLimitMaxTroopsAllowable = TRUE;
		ubNumExtraReinforcements = (UINT8)(iMaxReinforcementsAllowed - iReinforcementsRequested);
	}

	iReinforcementsRequested += ubNumExtraReinforcements;

	if( iReinforcementsRequested <= 0 )
	{
		ValidateWeights( 9 );
		return;
	}

	ubDstSectorX = (UINT8)SECTORX( gGarrisonGroup[ iDstGarrisonID ].ubSectorID );
	ubDstSectorY = (UINT8)SECTORY( gGarrisonGroup[ iDstGarrisonID ].ubSectorID );

	if( pOptionalGroup && *pOptionalGroup )
	{ //This group will provide the reinforcements
		pGroup = *pOptionalGroup;

		#ifdef JA2BETAVERSION
			LogStrategicEvent( "%d troops have been reassigned from %c%d to garrison sector %c%d",
				pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
				pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
				ubDstSectorY + 'A' - 1, ubDstSectorX );
		#endif

		gGarrisonGroup[ iDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
		ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ iDstGarrisonID ].ubComposition );
		MoveSAIGroupToSector( pOptionalGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, REINFORCEMENTS );

		ValidateWeights( 10 );

		return;
	}
	iRandom = Random( giReinforcementPoints + giReinforcementPool );
	if( iRandom < giReinforcementPool )
	{ //use the pool and send the requested amount from SECTOR P3 (queen's palace)
		QUEEN_POOL:

		//KM : Sep 9, 1999
		//If the player owns sector P3, any troops that spawned there were causing serious problems, seeing battle checks
		//were not performed!
		if( !StrategicMap[ CALCULATE_STRATEGIC_INDEX( 3, 16 ) ].fEnemyControlled )
		{ //Queen can no longer send reinforcements from the palace if she doesn't control it!
			return;
		}


		if( !giReinforcementPool )
		{
			ValidateWeights( 11 );
			return;
		}
		iReinforcementsApproved = MIN( iReinforcementsRequested, giReinforcementPool );

		if( iReinforcementsApproved * 3 < usDefencePoints )
		{ //The enemy force that would be sent would likely be decimated by the player forces.
			gubGarrisonReinforcementsDenied[ iDstGarrisonID ] += (UINT8)(gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bPriority / 2);
			ValidateWeights( 12 );
			return;
		}
		else
		{
			//The force is strong enough to be able to take the sector.
			gubGarrisonReinforcementsDenied[ iDstGarrisonID ] = 0;
		}

		//The chance she will send them is related with the strength difference between the
		//player's force and the queen's.
		if( ubNumExtraReinforcements && fLimitMaxTroopsAllowable && iReinforcementsApproved == iMaxReinforcementsAllowed )
		{
			iChance = (iReinforcementsApproved + ubNumExtraReinforcements) * 100 / usDefencePoints;
			if( !Chance( iChance ) )
			{
				ValidateWeights( 13 );
				return;
			}
		}


		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, (UINT8)iReinforcementsApproved, 0 );
		ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ iDstGarrisonID ].ubComposition );
		pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
		giReinforcementPool -= iReinforcementsApproved;
		pGroup->ubMoveType = ONE_WAY;
		gGarrisonGroup[ iDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;

		ubGroupSize = (UINT8)(pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins);

		if( ubNumExtraReinforcements )
		{
			#ifdef JA2BETAVERSION
				LogStrategicEvent( "%d troops have been sent from palace to stage assault near sector %c%d",
					ubGroupSize, ubDstSectorY + 'A' - 1, ubDstSectorX );
			#endif

			MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, STAGING );
		}
		else
		{
			#ifdef JA2BETAVERSION
				LogStrategicEvent( "%d troops have been sent from palace to garrison sector %c%d",
					ubGroupSize, ubDstSectorY + 'A' - 1, ubDstSectorX );
			#endif

			MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, REINFORCEMENTS );
		}
		ValidateWeights( 14 );
		return;
	}
	else
	{
		iSrcGarrisonID = ChooseSuitableGarrisonToProvideReinforcements( iDstGarrisonID, iReinforcementsRequested );
		if( iSrcGarrisonID == -1 )
		{
			ValidateWeights( 15 );
			goto QUEEN_POOL;
		}

		ubSrcSectorX = (gGarrisonGroup[ iSrcGarrisonID ].ubSectorID % 16) + 1;
		ubSrcSectorY = (gGarrisonGroup[ iSrcGarrisonID ].ubSectorID / 16) + 1;
		if( ubSrcSectorX != gWorldSectorX || ubSrcSectorY != gWorldSectorY || gbWorldSectorZ > 0 )
		{ //The reinforcements aren't coming from the currently loaded sector!
			iReinforcementsAvailable = ReinforcementsAvailable( iSrcGarrisonID );
			if( iReinforcementsAvailable <= 0)
			{
				SAIReportError( L"Attempting to send reinforcements from a garrison that doesn't have any! -- KM:0 (with prior saved game and strategic decisions.txt)" );
				return;
			}
			//Send the lowest of the two:  number requested or number available

			iReinforcementsApproved = MIN( iReinforcementsRequested, iReinforcementsAvailable );
			if( iReinforcementsApproved > iMaxReinforcementsAllowed - ubNumExtraReinforcements )
			{ //The force isn't strong enough, but the queen isn't willing to apply extra resources
				iReinforcementsApproved = iMaxReinforcementsAllowed - ubNumExtraReinforcements;
			}
			else if( (iReinforcementsApproved + ubNumExtraReinforcements) * 3 < usDefencePoints )
			{ //The enemy force that would be sent would likely be decimated by the player forces.
				gubGarrisonReinforcementsDenied[ iDstGarrisonID ] += (UINT8)(gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bPriority / 2);
				ValidateWeights( 17 );
				return;
			}
			else
			{
				//The force is strong enough to be able to take the sector.
				gubGarrisonReinforcementsDenied[ iDstGarrisonID ] = 0;
			}

			//The chance she will send them is related with the strength difference between the
			//player's force and the queen's.
			if( iReinforcementsApproved + ubNumExtraReinforcements == iMaxReinforcementsAllowed && usDefencePoints )
			{
				iChance = (iReinforcementsApproved + ubNumExtraReinforcements) * 100 / usDefencePoints;
				if( !Chance( iChance ) )
				{
					ValidateWeights( 18 );
					return;
				}
			}

			pGroup = CreateNewEnemyGroupDepartingFromSector( gGarrisonGroup[ iSrcGarrisonID ].ubSectorID, 0, (UINT8)iReinforcementsApproved, 0 );
			ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ iDstGarrisonID ].ubComposition );
			RemoveSoldiersFromGarrisonBasedOnComposition( iSrcGarrisonID, pGroup->ubGroupSize );
			pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
			pGroup->ubMoveType = ONE_WAY;
			gGarrisonGroup[ iDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
			ubGroupSize = (UINT8)( pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins );

			if( ubNumExtraReinforcements )
			{
				pGroup->pEnemyGroup->ubPendingReinforcements = ubNumExtraReinforcements;

				#ifdef JA2BETAVERSION
					LogStrategicEvent( "%d troops have been sent from sector %c%d to stage assault near sector %c%d",
						ubGroupSize, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX, ubDstSectorY + 'A' - 1, ubDstSectorX );
				#endif

				MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, STAGING );
			}
			else
			{
				#ifdef JA2BETAVERSION
					LogStrategicEvent( "%d troops have been sent from sector %c%d to garrison sector %c%d",
						ubGroupSize, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX, ubDstSectorY + 'A' - 1, ubDstSectorX );
				#endif

				MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, REINFORCEMENTS );
			}

			#ifdef JA2BETAVERSION
				LogStrategicEvent( "%d troops have been sent from garrison sector %c%d to patrol area near sector %c%d",
					ubGroupSize, ubSrcSectorY + 'A' - 1, ubSrcSectorX, ubDstSectorY + 'A' - 1, ubDstSectorX );
			#endif

			ValidateWeights( 19 );
			return;
		}
	}
	ValidateWeights( 20 );
}


static void SendReinforcementsForPatrol(INT32 iPatrolID, GROUP** pOptionalGroup)
{
	GROUP *pGroup;
	INT32 iRandom, iSrcGarrisonID, iWeight;
	INT32 iReinforcementsAvailable, iReinforcementsRequested, iReinforcementsApproved;
	UINT8 ubSrcSectorX, ubSrcSectorY;

	ValidateWeights( 21 );

	PATROL_GROUP* const pg = &gPatrolGroup[iPatrolID];

	//Determine how many units the patrol group needs.
	iReinforcementsRequested = PatrolReinforcementsRequested(pg);

	if( iReinforcementsRequested <= 0)
		return;

	UINT8 const ubDstSectorX = (pg->ubSectorID[1] % 16) + 1;
	UINT8 const ubDstSectorY = (pg->ubSectorID[1] / 16) + 1;

	if( pOptionalGroup && *pOptionalGroup )
	{ //This group will provide the reinforcements
		pGroup = *pOptionalGroup;

		pg->ubPendingGroupID = pGroup->ubGroupID;

		#ifdef JA2BETAVERSION
			LogStrategicEvent( "%d troops have been reassigned from %c%d to reinforce patrol group covering sector %c%d",
				pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
				pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
				ubDstSectorY + 'A' - 1, ubDstSectorX );
		#endif

		MoveSAIGroupToSector(pOptionalGroup, pg->ubSectorID[1], EVASIVE, REINFORCEMENTS);

		ValidateWeights( 22 );
		return;
	}
	iRandom = Random( giReinforcementPoints + giReinforcementPool );
	if( iRandom < giReinforcementPool )
	{ //use the pool and send the requested amount from SECTOR P3 (queen's palace)
		iReinforcementsApproved = MIN( iReinforcementsRequested, giReinforcementPool );
		if( !iReinforcementsApproved )
		{
			iReinforcementsApproved = iReinforcementsApproved;
		}
		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, (UINT8)iReinforcementsApproved, 0 );
		pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
		giReinforcementPool -= iReinforcementsApproved;

		pg->ubPendingGroupID = pGroup->ubGroupID;

		#ifdef JA2BETAVERSION
			LogStrategicEvent( "%d troops have been sent from palace to patrol area near sector %c%d",
				pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
				ubDstSectorY + 'A' - 1, ubDstSectorX );
		#endif

		MoveSAIGroupToSector(&pGroup, pg->ubSectorID[1], EVASIVE, REINFORCEMENTS);

		ValidateWeights( 23 );
		return;
	}
	else
	{
		iRandom -= giReinforcementPool;
		for( iSrcGarrisonID = 0; iSrcGarrisonID < giGarrisonArraySize; iSrcGarrisonID++ )
		{ //go through the garrisons
			RecalculateGarrisonWeight( iSrcGarrisonID );
			iWeight = -gGarrisonGroup[ iSrcGarrisonID ].bWeight;
			if( iWeight > 0 )
			{ //if group is able to provide reinforcements.
				if( iRandom < iWeight )
				{ //This is the group that gets the reinforcements!
					ubSrcSectorX = (UINT8)SECTORX(gGarrisonGroup[ iSrcGarrisonID ].ubSectorID );
					ubSrcSectorY = (UINT8)SECTORY(gGarrisonGroup[ iSrcGarrisonID ].ubSectorID );
					if( ubSrcSectorX != gWorldSectorX || ubSrcSectorY != gWorldSectorY || gbWorldSectorZ > 0 )
					{ //The reinforcements aren't coming from the currently loaded sector!
						iReinforcementsAvailable = ReinforcementsAvailable( iSrcGarrisonID );
						//Send the lowest of the two:  number requested or number available
						iReinforcementsApproved = MIN( iReinforcementsRequested, iReinforcementsAvailable );
						pGroup = CreateNewEnemyGroupDepartingFromSector( gGarrisonGroup[ iSrcGarrisonID ].ubSectorID, 0, (UINT8)iReinforcementsApproved, 0 );
						pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
						pg->ubPendingGroupID = pGroup->ubGroupID;

						RemoveSoldiersFromGarrisonBasedOnComposition( iSrcGarrisonID, pGroup->ubGroupSize );

						#ifdef JA2BETAVERSION
							LogStrategicEvent( "%d troops have been sent from garrison sector %c%d to patrol area near sector %c%d",
								pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
								ubSrcSectorY + 'A' - 1, ubSrcSectorX,
								ubDstSectorY + 'A' - 1, ubDstSectorX );
						#endif

						MoveSAIGroupToSector(&pGroup, pg->ubSectorID[1], EVASIVE, REINFORCEMENTS);

						ValidateWeights( 24 );

						return;
					}
				}
				iRandom -= iWeight;
			}
		}
	}
	ValidateWeights( 25 );
}


static void EvolveQueenPriorityPhase(BOOLEAN fForceChange);
static BOOLEAN GarrisonRequestingMinimumReinforcements(INT32 iGarrisonID);
static BOOLEAN PatrolRequestingMinimumReinforcements(INT32 iPatrolID);
static void UpgradeAdminsToTroops(void);


//Periodically does a general poll and check on each of the groups and garrisons, determines
//reinforcements, new patrol groups, planned assaults, etc.
void EvaluateQueenSituation()
{
	INT32 i, iRandom;
	INT32 iWeight;
	UINT32 uiOffset;
	UINT16 usDefencePoints;
	INT32 iSumOfAllWeights = 0;

	ValidateWeights( 26 );

	// figure out how long it shall be before we call this again

	// The more work to do there is (request points the queen's army is asking for), the more often she will make decisions
	// This can increase the decision intervals by up to 500 extra minutes (> 8 hrs)
	uiOffset = MAX( 100 - giRequestPoints, 0);
	uiOffset = uiOffset + Random( uiOffset * 4 );
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			uiOffset += EASY_TIME_EVALUATE_IN_MINUTES + Random( EASY_TIME_EVALUATE_VARIANCE );
			break;
		case DIF_LEVEL_MEDIUM:
			uiOffset += NORMAL_TIME_EVALUATE_IN_MINUTES + Random( NORMAL_TIME_EVALUATE_VARIANCE );
			break;
		case DIF_LEVEL_HARD:
			uiOffset += HARD_TIME_EVALUATE_IN_MINUTES + Random( HARD_TIME_EVALUATE_VARIANCE );
			break;
	}

	if( !giReinforcementPool )
	{ //Queen has run out of reinforcements.  Simulate recruiting and training new troops
		uiOffset *= 10;
		giReinforcementPool += 30;
		AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, GetWorldTotalMin() + uiOffset, 0 );
		return;
	}

	//Re-post the event
	AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, GetWorldTotalMin() + uiOffset, 0 );

	// if the queen hasn't been alerted to player's presence yet
	if( !gfQueenAIAwake )
	{ //no decisions can be made yet.
		return;
	}

	// Adjust queen's disposition based on player's progress
	EvolveQueenPriorityPhase( FALSE );

	// Gradually promote any remaining admins into troops
	UpgradeAdminsToTroops();

	if( ( giRequestPoints <= 0 ) || ( ( giReinforcementPoints <= 0 ) && ( giReinforcementPool <= 0 ) ) )
	{ //we either have no reinforcements or request for reinforcements.
		return;
	}

	//now randomly choose who gets the reinforcements.
	// giRequestPoints is the combined sum of all the individual weights of all garrisons and patrols requesting reinforcements
	iRandom = Random( giRequestPoints );

	//go through garrisons first
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		RecalculateGarrisonWeight( i );
		iWeight = gGarrisonGroup[ i ].bWeight;
		if( iWeight > 0 )
		{	//if group is requesting reinforcements.

			iSumOfAllWeights += iWeight;	// debug only!

			if( iRandom < iWeight && !gGarrisonGroup[ i ].ubPendingGroupID &&
					EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) &&
					GarrisonRequestingMinimumReinforcements( i ) )
			{ //This is the group that gets the reinforcements!
				if( ReinforcementsApproved( i, &usDefencePoints ) )
				{
					SendReinforcementsForGarrison( i, usDefencePoints, NULL );
				}
				else
				{
					#ifdef JA2BETAVERSION
						LogStrategicEvent( "Reinforcements were denied to go to %c%d because player forces too strong.",
							SECTORY( gGarrisonGroup[ i ].ubSectorID ) + 'A' - 1, SECTORX( gGarrisonGroup[ i ].ubSectorID ) );
					#endif
				}
				return;
			}
			iRandom -= iWeight;
		}
	}

	//go through the patrol groups
	for( i = 0; i < giPatrolArraySize; i++ )
	{
		RecalculatePatrolWeight(gPatrolGroup[i]);
		iWeight = gPatrolGroup[ i ].bWeight;
		if( iWeight > 0 )
		{
			iSumOfAllWeights += iWeight;	// debug only!

			if( iRandom < iWeight && !gPatrolGroup[ i ].ubPendingGroupID && PatrolRequestingMinimumReinforcements( i ) )
			{ //This is the group that gets the reinforcements!
				SendReinforcementsForPatrol( i, NULL );
				return;
			}
			iRandom -= iWeight;
		}
	}

	ValidateWeights( 27 );
}


void SaveStrategicAI(HWFILE const hFile)
{
	GARRISON_GROUP gTempGarrisonGroup;
	PATROL_GROUP gTempPatrolGroup;
	ARMY_COMPOSITION gTempArmyComp;
	INT32 i;

	memset( &gTempPatrolGroup, 0, sizeof( PATROL_GROUP ) );
	memset( &gTempArmyComp, 0, sizeof( ARMY_COMPOSITION ) );

	FileWrite(hFile, gbPadding2,                          3);
	FileWrite(hFile, &gfExtraElites,                      1);
	FileWrite(hFile, &giGarrisonArraySize,                4);
	FileWrite(hFile, &giPatrolArraySize,                  4);
	FileWrite(hFile, &giReinforcementPool,                4);
	FileWrite(hFile, &giForcePercentage,                  4);
	FileWrite(hFile, &giArmyAlertness,                    4);
	FileWrite(hFile, &giArmyAlertnessDecay,               4);
	FileWrite(hFile, &gfQueenAIAwake,                     1);
	FileWrite(hFile, &giReinforcementPoints,              4);
	FileWrite(hFile, &giRequestPoints,                    4);
	FileWrite(hFile, &gubNumAwareBattles,                 1);
	FileWrite(hFile, &gubSAIVersion,                      1);
	FileWrite(hFile, &gubQueenPriorityPhase,              1);
	FileWrite(hFile, &gfFirstBattleMeanwhileScenePending, 1);
	FileWrite(hFile, &gfMassFortificationOrdered,         1);
	FileWrite(hFile, &gubMinEnemyGroupSize,               1);
	FileWrite(hFile, &gubHoursGracePeriod,                1);
	FileWrite(hFile, &gusPlayerBattleVictories,           2);
	FileWrite(hFile, &gfUseAlternateQueenPosition,        1);
	FileWrite(hFile, gbPadding,           SAI_PADDING_BYTES);
	//Save the army composition (which does get modified)
	FileWrite(hFile, gArmyComp, NUM_ARMY_COMPOSITIONS * sizeof(ARMY_COMPOSITION));
	i = SAVED_ARMY_COMPOSITIONS - NUM_ARMY_COMPOSITIONS;
	while( i-- )
	{
		FileWrite(hFile, &gTempArmyComp, sizeof(ARMY_COMPOSITION));
	}
	//Save the patrol group definitions
	if (giPatrolArraySize != 0) FileWrite(hFile, gPatrolGroup, giPatrolArraySize * sizeof(PATROL_GROUP));
	i = SAVED_PATROL_GROUPS - giPatrolArraySize;
	while( i-- )
	{
		FileWrite(hFile, &gTempPatrolGroup, sizeof(PATROL_GROUP));
	}
	//Save the garrison information!
	memset( &gTempGarrisonGroup, 0, sizeof( GARRISON_GROUP ) );
	if (giGarrisonArraySize != 0) FileWrite(hFile, gGarrisonGroup, giGarrisonArraySize * sizeof(GARRISON_GROUP));
	i = SAVED_GARRISON_GROUPS - giGarrisonArraySize;
	while( i-- )
	{
		FileWrite(hFile, &gTempGarrisonGroup, sizeof(GARRISON_GROUP));
	}

	FileWrite(hFile, gubPatrolReinforcementsDenied, giPatrolArraySize);

	FileWrite(hFile, gubGarrisonReinforcementsDenied, giGarrisonArraySize);

	#ifdef JA2BETAVERSION
		ValidatePlayersAreInOneGroupOnly();
	#endif
}


static void ReinitializeUnvisitedGarrisons(void);


void LoadStrategicAI(HWFILE const hFile)
{
	GARRISON_GROUP gTempGarrisonGroup;
	PATROL_GROUP gTempPatrolGroup;
	ARMY_COMPOSITION gTempArmyComp;
	INT32 i;
	UINT8 ubSAIVersion;

	FileRead(hFile, gbPadding2,                          3);
	FileRead(hFile, &gfExtraElites,                      1);
	FileRead(hFile, &giGarrisonArraySize,                4);
	FileRead(hFile, &giPatrolArraySize,                  4);
	FileRead(hFile, &giReinforcementPool,                4);
	FileRead(hFile, &giForcePercentage,                  4);
	FileRead(hFile, &giArmyAlertness,                    4);
	FileRead(hFile, &giArmyAlertnessDecay,               4);
	FileRead(hFile, &gfQueenAIAwake,                     1);
	FileRead(hFile, &giReinforcementPoints,              4);
	FileRead(hFile, &giRequestPoints,                    4);
	FileRead(hFile, &gubNumAwareBattles,                 1);
	FileRead(hFile, &ubSAIVersion,                       1);
	FileRead(hFile, &gubQueenPriorityPhase,              1);
	FileRead(hFile, &gfFirstBattleMeanwhileScenePending, 1);
	FileRead(hFile, &gfMassFortificationOrdered,         1);
	FileRead(hFile, &gubMinEnemyGroupSize,               1);
	FileRead(hFile, &gubHoursGracePeriod,                1);
	FileRead(hFile, &gusPlayerBattleVictories,           2);
	FileRead(hFile, &gfUseAlternateQueenPosition,        1);
	FileRead(hFile, gbPadding,           SAI_PADDING_BYTES);
	//Restore the army composition
	FileRead(hFile, gArmyComp,	NUM_ARMY_COMPOSITIONS * sizeof(ARMY_COMPOSITION));
	i = SAVED_ARMY_COMPOSITIONS - NUM_ARMY_COMPOSITIONS;
	while( i-- )
	{
		FileRead(hFile, &gTempArmyComp, sizeof(ARMY_COMPOSITION));
	}

	//Restore the patrol group definitions
	if( gPatrolGroup )
	{
		MemFree( gPatrolGroup );
	}
	gPatrolGroup = MALLOCN(PATROL_GROUP, giPatrolArraySize);
	FileRead(hFile, gPatrolGroup, giPatrolArraySize * sizeof(PATROL_GROUP));
	i = SAVED_PATROL_GROUPS - giPatrolArraySize;
	while( i-- )
	{
		FileRead(hFile, &gTempPatrolGroup, sizeof(PATROL_GROUP));
	}

	gubSAIVersion = SAI_VERSION;
	//Load the garrison information!
	if( gGarrisonGroup )
	{
		MemFree( gGarrisonGroup );
	}
	gGarrisonGroup = MALLOCN(GARRISON_GROUP, giGarrisonArraySize);
	FileRead(hFile, gGarrisonGroup, giGarrisonArraySize * sizeof(GARRISON_GROUP));
	i = SAVED_GARRISON_GROUPS - giGarrisonArraySize;
	while( i-- )
	{
		FileRead(hFile, &gTempGarrisonGroup, sizeof(GARRISON_GROUP));
	}

	//Load the list of reinforcement patrol points.
	if( gubPatrolReinforcementsDenied )
	{
		MemFree( gubPatrolReinforcementsDenied );
		gubPatrolReinforcementsDenied = NULL;
	}
	gubPatrolReinforcementsDenied = MALLOCN(UINT8, giPatrolArraySize);
	FileRead(hFile, gubPatrolReinforcementsDenied, giPatrolArraySize);

	//Load the list of reinforcement garrison points.
	if( gubGarrisonReinforcementsDenied )
	{
		MemFree( gubGarrisonReinforcementsDenied );
		gubGarrisonReinforcementsDenied = NULL;
	}
	gubGarrisonReinforcementsDenied = MALLOCN(UINT8, giGarrisonArraySize);
	FileRead(hFile, gubGarrisonReinforcementsDenied, giGarrisonArraySize);

	#ifdef JA2BETAVERSION
		InitStrategicMovementCosts();
	#endif

	if( ubSAIVersion < 6 )
	{ //Reinitialize the costs since they have changed.

		//Recreate the compositions
		memcpy( gArmyComp, gOrigArmyComp, NUM_ARMY_COMPOSITIONS * sizeof( ARMY_COMPOSITION ) );
		EvolveQueenPriorityPhase( TRUE );

		//Recreate the patrol desired sizes
		for( i = 0; i < giPatrolArraySize; i++ )
		{
			gPatrolGroup[ i ].bSize = gOrigPatrolGroup[ i ].bSize;
		}
	}
	if( ubSAIVersion < 7 )
	{
		BuildUndergroundSectorInfoList();
	}
	if( ubSAIVersion < 8 )
	{
		ReinitializeUnvisitedGarrisons();
	}
	if( ubSAIVersion < 10 )
	{
		for( i = 0; i < giPatrolArraySize; i++ )
		{
			if( gPatrolGroup[ i ].bSize >= 16 )
			{
				gPatrolGroup[ i ].bSize = 10;
			}
		}
		FOR_ALL_ENEMY_GROUPS(pGroup)
		{
			if (pGroup->ubGroupSize >= 16)
			{ //accident in patrol groups being too large
				UINT8	ubGetRidOfXTroops = pGroup->ubGroupSize - 10;
				if( gbWorldSectorZ || pGroup->ubSectorX != gWorldSectorX || pGroup->ubSectorY != gWorldSectorY )
				{ //don't modify groups in the currently loaded sector.
					if( pGroup->pEnemyGroup->ubNumTroops >= ubGetRidOfXTroops )
					{
						pGroup->pEnemyGroup->ubNumTroops -= ubGetRidOfXTroops;
						pGroup->ubGroupSize -= ubGetRidOfXTroops;
					}
				}
			}
		}
	}
	if( ubSAIVersion < 13 )
	{
		for( i = 0; i < 255; i++ )
		{
			SectorInfo[ i ].bBloodCatPlacements = 0;
			SectorInfo[ i ].bBloodCats = -1;
		}
		InitBloodCatSectors();
		//This info is used to clean up the two new codes inserted into the
		//middle of the enumeration for battle codes.
		if( gubEnemyEncounterCode > CREATURE_ATTACK_CODE )
		{
			gubEnemyEncounterCode += 2;
		}
		if( gubExplicitEnemyEncounterCode > CREATURE_ATTACK_CODE )
		{
			gubExplicitEnemyEncounterCode += 2;
		}

	}
	if( ubSAIVersion < 14 )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( 4, 11, 1 );
		if( pSector->ubNumTroops + pSector->ubNumElites > 20 )
		{
			pSector->ubNumTroops -= 2;
		}
		pSector = FindUnderGroundSector( 3, 15, 1 );
		if( pSector->ubNumTroops + pSector->ubNumElites > 20 )
		{
			pSector->ubNumTroops -= 2;
		}
	}
	if( ubSAIVersion < 16 )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( 3, 15, 1 );
		if( pSector )
		{
			pSector->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
		}
		pSector = FindUnderGroundSector( 3, 16, 1 );
		if( pSector )
		{
			pSector->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
		}
	}
	if( ubSAIVersion < 17 )
	{ //Patch all groups that have this flag set
		gubNumGroupsArrivedSimultaneously = 0;
		{
			FOR_ALL_GROUPS(pGroup)
			{
				if( pGroup->uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY )
				{
					pGroup->uiFlags &= ~GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
				}
			}
		}
	}
	if( ubSAIVersion < 18 )
	{ //adjust down the number of bloodcats based on difficulty in the two special bloodcat levels
		switch( gGameOptions.ubDifficultyLevel )
		{
			case DIF_LEVEL_EASY: //50%
				SectorInfo[ SEC_I16	].bBloodCatPlacements = 14;
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 13;
				SectorInfo[ SEC_I16	].bBloodCats = 14;
				SectorInfo[ SEC_N5	].bBloodCats = 13;
				break;
			case DIF_LEVEL_MEDIUM: //75%
				SectorInfo[ SEC_I16	].bBloodCatPlacements = 19;
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 18;
				SectorInfo[ SEC_I16	].bBloodCats = 19;
				SectorInfo[ SEC_N5	].bBloodCats = 18;
			case DIF_LEVEL_HARD: //100%
				SectorInfo[ SEC_I16	].bBloodCatPlacements = 26;
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 25;
				SectorInfo[ SEC_I16	].bBloodCats = 26;
				SectorInfo[ SEC_N5	].bBloodCats = 25;
				break;
		}
	}
	if( ubSAIVersion < 19 )
	{
		//Clear the garrison in C5
		gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_C5 ].ubGarrisonID ].ubComposition ].bPriority = 0;
		gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_C5 ].ubGarrisonID ].ubComposition ].bDesiredPopulation = 0;
	}
	if( ubSAIVersion < 20 )
	{
		gArmyComp[ QUEEN_DEFENCE ].bDesiredPopulation = 32;
		SectorInfo[ SEC_P3 ].ubNumElites = 32;
	}
	if( ubSAIVersion < 21 )
	{
		FOR_ALL_GROUPS(pGroup) pGroup->uiFlags = 0;
	}
	if( ubSAIVersion < 22 )
	{ //adjust down the number of bloodcats based on difficulty in the two special bloodcat levels
		switch( gGameOptions.ubDifficultyLevel )
		{
			case DIF_LEVEL_EASY: //50%
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
				SectorInfo[ SEC_N5	].bBloodCats = 10;
				break;
			case DIF_LEVEL_MEDIUM: //75%
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
				SectorInfo[ SEC_N5	].bBloodCats = 10;
			case DIF_LEVEL_HARD: //100%
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
				SectorInfo[ SEC_N5	].bBloodCats = 10;
				break;
		}
	}
	if( ubSAIVersion < 23 )
	{
		if( gWorldSectorX != 3 || gWorldSectorY != 16 || !gbWorldSectorZ )
		{
			SectorInfo[ SEC_P3 ].ubNumElites = 32;
		}
	}
	if( ubSAIVersion < 24 )
	{
		//If the queen has escaped to the basement, do not use the profile insertion info
		//when we finally go down there, otherwise she will end up in the wrong spot, possibly inside
		//the walls.
		if( !gubFact[ FACT_QUEEN_DEAD ] && gMercProfiles[ QUEEN ].bSectorZ == 1 )
		if( gbWorldSectorZ != 1 || gWorldSectorX != 16 || gWorldSectorY != 3 )
		{ //We aren't in the basement sector
			gMercProfiles[ QUEEN ].fUseProfileInsertionInfo = FALSE;
		}
		else
		{ //We are in the basement sector, relocate queen to proper position.
			FOR_ALL_IN_TEAM(i, CIV_TEAM)
			{
				SOLDIERTYPE& s = *i;
				if (s.ubProfile != QUEEN) continue;
				// Found queen, relocate her to 16866
				BumpAnyExistingMerc(16866);
				TeleportSoldier(s, 16866, true);
				break;
			}
		}
	}
	if( ubSAIVersion < 25 )
	{
		if( gubFact[ FACT_SKYRIDER_CLOSE_TO_CHOPPER ] )
		{
			gMercProfiles[ SKYRIDER ].fUseProfileInsertionInfo = FALSE;
		}
	}

	//KM : July 21, 1999 patch fix
	if( ubSAIVersion < 26 )
	{
		INT32 i;
		for( i = 0; i < 255; i++ )
		{
			if( SectorInfo[ i ].ubNumberOfCivsAtLevel[ GREEN_MILITIA ] +
					SectorInfo[ i ].ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] +
					SectorInfo[ i ].ubNumberOfCivsAtLevel[ ELITE_MILITIA ] > 20 )
			{
				SectorInfo[ i ].ubNumberOfCivsAtLevel[ GREEN_MILITIA ] = 0;
				SectorInfo[ i ].ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] = 20;
				SectorInfo[ i ].ubNumberOfCivsAtLevel[ ELITE_MILITIA ] = 0;
			}
		}
	}

	//KM : August 4, 1999 patch fix
	//     This addresses the problem of not having any soldiers in sector N7 when playing the game under easy difficulty.
	//		 If captured and interrogated, the player would find no soldiers defending the sector.  This changes the composition
	//     so that it will always be there, and adds the soldiers accordingly if the sector isn't loaded when the update is made.
	if( ubSAIVersion < 27 )
	{
		if( gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY )
		{
			if( gWorldSectorX != 7 || gWorldSectorY != 14 || gbWorldSectorZ )
			{
				INT32 cnt, iRandom;
				INT32 iEliteChance, iTroopChance, iAdminChance;
				INT32 iStartPop;
				SECTORINFO *pSector = NULL;

				//Change the garrison composition to LEVEL1_DEFENCE from LEVEL2_DEFENCE
				pSector = &SectorInfo[ SEC_N7 ];
				gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition = LEVEL1_DEFENCE;

				iStartPop = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bStartPopulation;
				iEliteChance = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bElitePercentage;
				iTroopChance = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bTroopPercentage + iEliteChance;
				iAdminChance = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bAdminPercentage;

				if( iStartPop )
				{
					// if population is less than maximum
					if( iStartPop != MAX_STRATEGIC_TEAM_SIZE )
					{
						// then vary it a bit (+/- 25%)
						iStartPop = iStartPop * ( 100 + ( Random ( 51 ) - 25 ) ) / 100;
					}

					iStartPop = MAX( gubMinEnemyGroupSize, MIN( MAX_STRATEGIC_TEAM_SIZE, iStartPop ) );
					cnt = iStartPop;

					if( iAdminChance )
					{
						pSector->ubNumAdmins = iAdminChance * iStartPop / 100;
					}
					else while( cnt-- )
					{ //for each person, randomly determine the types of each soldier.
						{
							iRandom = Random( 100 );
							if( iRandom < iEliteChance )
							{
								pSector->ubNumElites++;
							}
							else if( iRandom < iTroopChance )
							{
								pSector->ubNumTroops++;
							}
						}
					}
				}
			}
		}
	}

	if( ubSAIVersion < 28 )
	{
		if( !StrategicMap[ CALCULATE_STRATEGIC_INDEX( 3, 16 ) ].fEnemyControlled )
		{ //Eliminate all enemy groups in this sector, because the player owns the sector, and it is not
			//possible for them to spawn there!
			FOR_ALL_GROUPS_SAFE(i)
			{
				GROUP& g = *i;
				if (g.fPlayer)         continue;
				if (g.ubSectorX !=  3) continue;
				if (g.ubSectorY != 16) continue;
				if (g.ubPrevX   !=  0) continue;
				if (g.ubPrevY   !=  0) continue;
				RemoveGroupFromStrategicAILists(g);
				RemoveGroup(g);
			}
		}
	}
	if( ubSAIVersion < 29 )
	{
		InitStrategicMovementCosts();
	}

	//KM : Aug 11, 1999 -- Patch fix:  Blindly update the airspace control.  There is a bug somewhere
	//		 that is failing to keep this information up to date, and I failed to find it.  At least this
	//		 will patch saves.
	UpdateAirspaceControl( );

	EvolveQueenPriorityPhase( TRUE );

	//Count and correct the floating groups
	FOR_ALL_GROUPS_SAFE(pGroup)
	{
		if( !pGroup->fPlayer )
		{
			if( !pGroup->fBetweenSectors )
			{
				if( pGroup->ubSectorX != gWorldSectorX ||
						pGroup->ubSectorY != gWorldSectorY ||
						gbWorldSectorZ )
				{
					RepollSAIGroup( pGroup );
					ValidateGroup( pGroup );
				}
			}
		}
	}

	#ifdef JA2BETAVERSION
		LogStrategicMsg( "" );
		LogStrategicMsg( "-------------------------------------------------" );
		LogStrategicMsg( "Restoring saved game at Day %02d, %02d:%02d ", GetWorldDay(), GetWorldHour(), GetWorldMinutesInDay()%60 );
		LogStrategicMsg( "-------------------------------------------------" );
		LogStrategicMsg( "" );
	#endif

	//Update the version number to the most current.
	gubSAIVersion = SAI_VERSION;

	ValidateWeights( 28 );
	ValidatePendingGroups();
	#ifdef JA2BETAVERSION
		ValidatePlayersAreInOneGroupOnly();
	#endif
}


//As the player's progress changes in the game, the queen will adjust her priorities accordingly.
//Basically, increasing priorities and numbers for sectors she owns, and lowering them.
//@@@Alex, this is tweakable.  My philosophies could be incorrect.  It might be better if instead of lowering
//priorities and numbers for towns the queen has lost, to instead lower the priority but increase the numbers so
//she would send larger attack forces.  This is questionable.
static void EvolveQueenPriorityPhase(BOOLEAN fForceChange)
{
	INT32 i, index, num, iFactor;
	INT32 iChange, iNew, iNumSoldiers, iNumPromotions;
	SECTORINFO *pSector;
	UINT8 ubOwned[ NUM_ARMY_COMPOSITIONS ];
	UINT8 ubTotal[ NUM_ARMY_COMPOSITIONS ];
	UINT8 ubNewPhase;
	ubNewPhase = CurrentPlayerProgressPercentage() / 10;

	if( !fForceChange && ubNewPhase == gubQueenPriorityPhase )
	{
		return;
	}

	if( gubQueenPriorityPhase > ubNewPhase )
	{
		#ifdef JA2BETAVERSION
			LogStrategicEvent( "The queen's defence priority has decreased from %d0%% to %d0%%.", gubQueenPriorityPhase, ubNewPhase );
		#endif
	}
	else if( gubQueenPriorityPhase < ubNewPhase )
	{
		#ifdef JA2BETAVERSION
			LogStrategicEvent( "The queen's defence priority has increased from %d0%% to %d0%%.", gubQueenPriorityPhase, ubNewPhase );
		#endif
	}
  else
  {
		#ifdef JA2BETAVERSION
			LogStrategicEvent( "The queen's defence priority is the same (%d0%%), but has been forced to update.", gubQueenPriorityPhase );
		#endif
  }

	gubQueenPriorityPhase = ubNewPhase;

	//The phase value refers to the deviation percentage she will apply to original garrison values.
	//All sector values are evaluated to see how many of those sectors are enemy controlled.  If they
	//are controlled by her, the desired number will be increased as well as the priority.  On the other
	//hand, if she doesn't own those sectors, the values will be decreased instead.  All values are based off of
	//the originals.
	memset( ubOwned, 0, NUM_ARMY_COMPOSITIONS );
	memset( ubTotal, 0, NUM_ARMY_COMPOSITIONS );

	//Record the values required to calculate the percentage of each composition type that the queen controls.
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		index = gGarrisonGroup[ i ].ubComposition;
		if( StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( gGarrisonGroup[ i ].ubSectorID ) ].fEnemyControlled )
		{
			ubOwned[ index ]++;
		}
		ubTotal[ index ]++;
	}

	//Go through the *majority* of compositions and modify the priority/desired values.
	for( i = 0; i < NUM_ARMY_COMPOSITIONS; i++ )
	{
		switch( i )
		{
			case QUEEN_DEFENCE:
			case MEDUNA_DEFENCE:
			case MEDUNA_SAMSITE:
			case LEVEL1_DEFENCE:
			case LEVEL2_DEFENCE:
			case LEVEL3_DEFENCE:
			case OMERTA_WELCOME_WAGON:
			case ROADBLOCK:
			//case SANMONA_SMALL:
				//don't consider these compositions
				continue;
		}
		//If the queen owns them ALL, then she gets the maximum defensive bonus.  If she owns NONE,
		//then she gets a maximum defensive penalty.  Everything else lies in the middle.  The legal
		//range is +-50.
		if( ubTotal[ i ] )
		{
			iFactor = (ubOwned[ i ] * 100 / ubTotal[ i ]) - 50;
		}
		else
		{
			iFactor = -50;
		}
		iFactor = iFactor * gubQueenPriorityPhase / 10;

		//modify priority by + or - 25% of original
		if( gArmyComp[ i ].bPriority )
		{
			num = gOrigArmyComp[ i ].bPriority + iFactor / 2;
			num = MIN( MAX( 0, num ), 100 );
			gArmyComp[ i ].bPriority = (INT8)num;
		}

		//modify desired population by + or - 50% of original population
		num = gOrigArmyComp[ i ].bDesiredPopulation * (100 + iFactor) / 100;
		num = MIN( MAX( 6, num ), MAX_STRATEGIC_TEAM_SIZE );
		gArmyComp[ i ].bDesiredPopulation = (INT8)num;

		//if gfExtraElites is set, then augment the composition sizes
		if( gfExtraElites && iFactor >= 15 && gArmyComp[ i ].bElitePercentage )
		{
			iChange = gGameOptions.ubDifficultyLevel * 5;

			//increase elite % (max 100)
			iNew = gArmyComp[ i ].bElitePercentage + iChange;
			iNew = (INT8)MIN( 100, iNew );
			gArmyComp[ i ].bElitePercentage = (INT8)iNew;

			//decrease troop % (min 0)
			iNew = gArmyComp[ i ].bTroopPercentage - iChange;
			iNew = (INT8)MAX( 0, iNew );
			gArmyComp[ i ].bTroopPercentage = (INT8)iNew;
		}
	}
	if( gfExtraElites )
	{
		//Turn off the flag so that this doesn't happen everytime this function is called!
		gfExtraElites = FALSE;

		for( i = 0; i < giGarrisonArraySize; i++ )
		{
			//if we are dealing with extra elites, then augment elite compositions (but only if they exist in the sector).
			//If the queen still owns the town by more than 65% (iFactor >= 15), then upgrade troops to elites in those sectors.
			index = gGarrisonGroup[ i ].ubComposition;
			switch( index )
			{
				case QUEEN_DEFENCE:
				case MEDUNA_DEFENCE:
				case MEDUNA_SAMSITE:
				case LEVEL1_DEFENCE:
				case LEVEL2_DEFENCE:
				case LEVEL3_DEFENCE:
				case OMERTA_WELCOME_WAGON:
				case ROADBLOCK:
				//case SANMONA_SMALL:
					//don't consider these compositions
					continue;
			}
			pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
			if( ubTotal[ index ] )
			{
				iFactor = (ubOwned[ index ] * 100 / ubTotal[ index ]) - 50;
			}
			else
			{
				iFactor = -50;
			}
			if( iFactor >= 15 )
			{ //Make the actual elites in sector match the new garrison percentage
				if( !gfWorldLoaded || gbWorldSectorZ ||
						gWorldSectorX != SECTORX( gGarrisonGroup[ i ].ubSectorID ) ||
						gWorldSectorY != SECTORY( gGarrisonGroup[ i ].ubSectorID ) )
				{ //Also make sure the sector isn't currently loaded!
					iNumSoldiers = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
					iNumPromotions = gArmyComp[ index ].bElitePercentage * iNumSoldiers / 100 - pSector->ubNumElites;

					if( iNumPromotions > 0 )
					{
						while( iNumPromotions-- )
						{
							if( pSector->ubNumAdmins )
							{
								pSector->ubNumAdmins--;
							}
							else if( pSector->ubNumTroops )
							{
								pSector->ubNumTroops--;
							}
							else
							{
								Assert( 0 );
							}
							pSector->ubNumElites++;
						}
						Assert( iNumSoldiers == pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites );
					}
				}
			}
		}
	}
	//Recalculate all of the weights.
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		RecalculateGarrisonWeight( i );
	}
}


static void RequestHighPriorityGarrisonReinforcements(INT32 iGarrisonID, UINT8 ubSoldiersRequested);


void ExecuteStrategicAIAction( UINT16 usActionCode, INT16 sSectorX, INT16 sSectorY )
{
	GROUP *pGroup, *pPendingGroup = NULL;
	SECTORINFO *pSector;
	UINT8 ubSectorID;
	UINT8 ubNumSoldiers;
	switch( usActionCode )
	{
		case STRATEGIC_AI_ACTION_WAKE_QUEEN:
			WakeUpQueen();
			break;

		case STRATEGIC_AI_ACTION_QUEEN_DEAD:
			gfQueenAIAwake = FALSE;
			break;

		case STRATEGIC_AI_ACTION_KINGPIN_DEAD:
			//Immediate send a small garrison to C5 (to discourage access to Tony the dealer)
			/*
			for( i = 0; i < giGarrisonArraySize; i++ )
			{
				if( gGarrisonGroup[ i ].ubComposition == SANMONA_SMALL )
				{
					//Setup the composition so from now on the queen will consider this an important sector
					//to hold.
					gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bPriority = 65;
					gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bTroopPercentage = 100;
					gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bDesiredPopulation = 5;
					RequestHighPriorityGarrisonReinforcements( i, (UINT8)(2 + Random( 4 )) ); //send 2-5 soldiers now.
					break;
				}
			}
			*/
			break;
		case NPC_ACTION_SEND_SOLDIERS_TO_DRASSEN:
			//Send 6, 9, or 12 troops (based on difficulty) one of the Drassen sectors.  If nobody is there when they arrive,
			//those troops will get reassigned.

			if( Chance( 50 ) )
			{
				ubSectorID = SEC_D13;
			}
			else if( Chance( 60 ) )
			{
				ubSectorID = SEC_B13;
			}
			else
			{
				ubSectorID = SEC_C13;
			}
			ubNumSoldiers = (UINT8)(3 + gGameOptions.ubDifficultyLevel * 3);
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubNumSoldiers, 0 );

			if( !gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID )
			{
				pGroup->pEnemyGroup->ubIntention = STAGE;
				gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
			}
			else
			{ //this should never happen (but if it did, then this is the best way to deal with it).
				pGroup->pEnemyGroup->ubIntention = PURSUIT;
			}
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );

			MoveSAIGroupToSector( &pGroup, ubSectorID, EVASIVE, pGroup->pEnemyGroup->ubIntention );

			break;
		case NPC_ACTION_SEND_SOLDIERS_TO_BATTLE_LOCATION:

			//Send 4, 8, or 12 troops (based on difficulty) to the location of the first battle.  If nobody is there when they arrive,
			//those troops will get reassigned.
			ubSectorID = (UINT8)STRATEGIC_INDEX_TO_SECTOR_INFO( sWorldSectorLocationOfFirstBattle );
			pSector = &SectorInfo[ ubSectorID ];
			ubNumSoldiers = (UINT8)(gGameOptions.ubDifficultyLevel * 4);
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubNumSoldiers, 0 );
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );

			//Determine if the battle location actually has a garrison assignment.  If so, and the following
			//checks succeed, the enemies will be sent to attack and reinforce that sector.  Otherwise, the
			//enemies will simply check it out, then leave.
			if( pSector->ubGarrisonID != NO_GARRISON )
			{ //sector has a garrison
				if( !NumEnemiesInSector( (INT16)SECTORX( ubSectorID ), (INT16)SECTORY( ubSectorID ) ) )
				{ //no enemies are here
					if( gArmyComp[ !gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bPriority )
					{ //the garrison is important
						if( !gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
						{ //the garrison doesn't have reinforcements already on route.
							gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
							MoveSAIGroupToSector( &pGroup, ubSectorID, STAGE, REINFORCEMENTS );
							break;
						}
					}
				}
			}
			else
			{
				MoveSAIGroupToSector( &pGroup, ubSectorID, EVASIVE, PURSUIT );
			}

			break;
		case NPC_ACTION_SEND_SOLDIERS_TO_OMERTA:
			ubNumSoldiers = (UINT8)(gGameOptions.ubDifficultyLevel * 6); //6, 12, or 18 based on difficulty.
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubNumSoldiers, (UINT8)(ubNumSoldiers/7) ); //add 1 elite to normal, and 2 for hard
			ubNumSoldiers = (UINT8)(ubNumSoldiers + ubNumSoldiers / 7);
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );
			if( PlayerMercsInSector( 9, 1, 1 ) && !PlayerMercsInSector( 10, 1, 1 ) && !PlayerMercsInSector( 10, 1, 2 ) )
			{ //send to A9 (if mercs in A9, but not in A10 or A10 basement)
				ubSectorID = SEC_A9;
			}
			else
			{ //send to A10
				ubSectorID = SEC_A10;
			}

			MoveSAIGroupToSector( &pGroup, ubSectorID, EVASIVE, PURSUIT );

			ValidateGroup( pGroup );
			break;
		case NPC_ACTION_SEND_TROOPS_TO_SAM:
			ubSectorID = (UINT8)SECTOR( sSectorX, sSectorY );
			ubNumSoldiers = (UINT8)( 3 + gGameOptions.ubDifficultyLevel + HighestPlayerProgressPercentage() / 15 );
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, 0, ubNumSoldiers );
			MoveSAIGroupToSector( &pGroup, ubSectorID, STAGE, REINFORCEMENTS );

			if( gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID )
			{	//Clear the pending group's assignment.
				pPendingGroup = GetGroup( gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID );
				Assert( pPendingGroup );
				RemoveGroupFromStrategicAILists(*pPendingGroup);
			}
			//Assign the elite squad to attack the SAM site
			pGroup->pEnemyGroup->ubIntention = REINFORCEMENTS;
			gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;

			if( pPendingGroup )
			{ //Reassign the pending group
				ReassignAIGroup( &pPendingGroup );
			}

			break;
		case NPC_ACTION_ADD_MORE_ELITES:
			gfExtraElites = TRUE;
			EvolveQueenPriorityPhase( TRUE );
			break;
		case NPC_ACTION_GIVE_KNOWLEDGE_OF_ALL_MERCS:
			//temporarily make the queen's forces more aware (high alert)
			switch( gGameOptions.ubDifficultyLevel )
			{
				case DIF_LEVEL_EASY:
					gubNumAwareBattles = EASY_NUM_AWARE_BATTLES;
					break;
				case DIF_LEVEL_MEDIUM:
					gubNumAwareBattles = NORMAL_NUM_AWARE_BATTLES;
					break;
				case DIF_LEVEL_HARD:
					gubNumAwareBattles = HARD_NUM_AWARE_BATTLES;
					break;
			}
			break;
		default:
			ScreenMsg( FONT_RED, MSG_DEBUG, L"QueenAI failed to handle action code %d.", usActionCode );
			break;
	}
}

#ifdef JA2BETAVERSION

static void LogStrategicMsg(const char* str, ...)
{
	va_list argptr;

	FILE *fp;

	fp = fopen( "Strategic Decisions.txt", "a" );
	if( !fp )
		return;

	va_start(argptr, str );
	char string[512];
	vsprintf( string, str, argptr);
	va_end(argptr);

	fprintf( fp, "%s\n", string );

	if( gfDisplayStrategicAILogs )
	{
		ScreenMsg(FONT_LTKHAKI, MSG_DIALOG, L"%hs", string);
	}
	if( guiCurrentScreen == AIVIEWER_SCREEN )
	{
		DebugMsg(TOPIC_JA2SAI, DBG_LEVEL_1, string);
	}

	fclose( fp );
}


static void LogStrategicEvent(const char* str, ...)
{
	va_list argptr;

	FILE *fp;

	fp = fopen( "Strategic Decisions.txt", "a" );
	if( !fp )
		return;

	va_start(argptr, str );
	char string[512];
	vsprintf( string, str, argptr);
	va_end(argptr);


	fprintf( fp, "\n%ls:\n", WORLDTIMESTR );
	fprintf( fp, "%s\n", string );

	if( gfDisplayStrategicAILogs )
	{
		ScreenMsg(FONT_LTKHAKI, MSG_DIALOG, L"%hs", string);
	}
	if( guiCurrentScreen == AIVIEWER_SCREEN )
	{
		DebugMsg(TOPIC_JA2SAI, DBG_LEVEL_1, string);
	}

	fclose( fp );
}


static void ClearStrategicLog(void)
{
	FILE *fp;
	fp = fopen( "Strategic Decisions.txt", "w" );
	if( !fp )
		return;

	fprintf( fp, "STRATEGIC LOG\n" );

	fclose( fp );
}
#endif


static UINT8 RedirectEnemyGroupsMovingThroughSector(UINT8 ubSectorX, UINT8 ubSectorY);


void StrategicHandleQueenLosingControlOfSector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	SECTORINFO *pSector;
	UINT8 ubSectorID;
	if( sSectorZ )
	{ //The queen doesn't care about anything happening under the ground.
		return;
	}

	if( StrategicMap[ sSectorX + sSectorY * MAP_WORLD_X ].fEnemyControlled )
	{ //If the sector doesn't belong to the player, then we shouldn't be calling this function!
		SAIReportError( L"StrategicHandleQueenLosingControlOfSector() was called for a sector that is internally considered to be enemy controlled." );
		return;
	}

	ubSectorID = SECTOR( sSectorX, sSectorY );
	pSector = &SectorInfo[ ubSectorID ];

	//Keep track of victories and wake up the queen after x number of battles.
	gusPlayerBattleVictories++;
	if( gusPlayerBattleVictories == 5 - gGameOptions.ubDifficultyLevel )
	{ //4 victories for easy, 3 for normal, 2 for hard
		WakeUpQueen();
	}

	if( pSector->ubGarrisonID == NO_GARRISON )
	{ //Queen doesn't care if the sector lost wasn't a garrison sector.
		return;
	}
	else
	{ //check to see if there are any pending reinforcements.  If so, then cancel their orders and have them
		//reassigned, so the player doesn't get pestered.  This is a feature that *dumbs* down the AI, and is done
		//for the sake of gameplay.  We don't want the game to be tedious.
		if( !pSector->uiTimeLastPlayerLiberated )
		{
			pSector->uiTimeLastPlayerLiberated = GetWorldTotalSeconds();
		}
		else
		{ //convert hours to seconds and subtract up to half of it randomly "seconds - (hours*3600 / 2)"
			pSector->uiTimeLastPlayerLiberated = GetWorldTotalSeconds() - Random( gubHoursGracePeriod * 1800 );
		}
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			GROUP *pGroup;
			pGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			if( pGroup )
			{
				ReassignAIGroup( &pGroup );
			}
			gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID = 0;
		}
	}

	//If there are any enemy groups that will be moving through this sector due, they will have to repath which
	//will cause them to avoid the sector.  Returns the number of redirected groups.
	RedirectEnemyGroupsMovingThroughSector( (UINT8)sSectorX, (UINT8)sSectorY );
}


static UINT8 SectorDistance(UINT8 ubSectorID1, UINT8 ubSectorID2)
{
	UINT8 ubSectorX1, ubSectorX2, ubSectorY1, ubSectorY2;
	UINT8 ubDist;
	ubSectorX1 = (UINT8)SECTORX( ubSectorID1 );
	ubSectorX2 = (UINT8)SECTORX( ubSectorID2 );
	ubSectorY1 = (UINT8)SECTORY( ubSectorID1 );
	ubSectorY2 = (UINT8)SECTORY( ubSectorID2 );

	ubDist = (UINT8)( abs( ubSectorX1 - ubSectorX2 ) + abs( ubSectorY1 - ubSectorY2 ) );

	return ubDist;
}


static void RequestHighPriorityGarrisonReinforcements(INT32 iGarrisonID, UINT8 ubSoldiersRequested)
{
	INT32 i, iBestIndex;
	GROUP *pGroup;
	UINT8 ubBestDist, ubDist;
	UINT8 ubDstSectorX, ubDstSectorY;
	//AssertMsg( giPatrolArraySize == PATROL_GROUPS && giGarrisonArraySize == GARRISON_GROUPS, "Strategic AI -- Patrol and/or garrison group definition mismatch." );
	ubBestDist = 255;
	iBestIndex = -1;
	for( i = 0; i < giPatrolArraySize; i++ )
	{
		if( gPatrolGroup[ i ].ubGroupID )
		{
			pGroup = GetGroup( gPatrolGroup[ i ].ubGroupID );
			if( pGroup && pGroup->ubGroupSize >= ubSoldiersRequested )
			{
				ubDist = SectorDistance( (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ), gGarrisonGroup[ iGarrisonID ].ubSectorID );
				if( ubDist < ubBestDist )
				{
					ubBestDist = ubDist;
					iBestIndex = i;
				}
			}
		}
	}
	ubDstSectorX = (UINT8)SECTORX( gGarrisonGroup[ iGarrisonID ].ubSectorID );
	ubDstSectorY = (UINT8)SECTORY( gGarrisonGroup[ iGarrisonID ].ubSectorID );
	if( iBestIndex != -1 )
	{ //Send the group to the garrison
		pGroup = GetGroup( gPatrolGroup[ iBestIndex ].ubGroupID );
		if( pGroup->ubGroupSize > ubSoldiersRequested && pGroup->ubGroupSize - ubSoldiersRequested >= gubMinEnemyGroupSize )
		{ //Split the group, and send to location
			GROUP *pNewGroup;
			pNewGroup = CreateNewEnemyGroupDepartingFromSector( (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ), 0, 0, 0 );
			//Transfer the troops from group to new group
			if( pGroup->pEnemyGroup->ubNumTroops >= ubSoldiersRequested )
			{ //All of them are troops, so do it in one shot.
				pGroup->pEnemyGroup->ubNumTroops -= ubSoldiersRequested;
				pGroup->ubGroupSize -= ubSoldiersRequested;
				pNewGroup->pEnemyGroup->ubNumTroops = ubSoldiersRequested;
				pNewGroup->ubGroupSize += ubSoldiersRequested;
				ValidateLargeGroup( pGroup );
				ValidateLargeGroup( pNewGroup );
			}
			else while( ubSoldiersRequested )
			{ //There aren't enough troops, so transfer other types when we run out of troops, prioritizing admins, then elites.
				if( pGroup->pEnemyGroup->ubNumTroops )
				{
					pGroup->pEnemyGroup->ubNumTroops--;
					pGroup->ubGroupSize--;
					pNewGroup->pEnemyGroup->ubNumTroops++;
					pNewGroup->ubGroupSize++;
					ubSoldiersRequested--;
					ValidateLargeGroup( pGroup );
					ValidateLargeGroup( pNewGroup );
				}
				else if( pGroup->pEnemyGroup->ubNumAdmins )
				{
					pGroup->pEnemyGroup->ubNumAdmins--;
					pGroup->ubGroupSize--;
					pNewGroup->pEnemyGroup->ubNumAdmins++;
					pNewGroup->ubGroupSize++;
					ubSoldiersRequested--;
					ValidateLargeGroup( pGroup );
					ValidateLargeGroup( pNewGroup );
				}
				else if( pGroup->pEnemyGroup->ubNumElites )
				{
					pGroup->pEnemyGroup->ubNumElites--;
					pGroup->ubGroupSize--;
					pNewGroup->pEnemyGroup->ubNumElites++;
					pNewGroup->ubGroupSize++;
					ubSoldiersRequested--;
					ValidateLargeGroup( pGroup );
					ValidateLargeGroup( pNewGroup );
				}
				else
				{
					AssertMsg( 0, "Strategic AI group transfer error.  KM : 0" );
					return;
				}
			}
			pNewGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
			gGarrisonGroup[ iGarrisonID ].ubPendingGroupID = pNewGroup->ubGroupID;
			RecalculatePatrolWeight(gPatrolGroup[iBestIndex]);

			MoveSAIGroupToSector( &pNewGroup, gGarrisonGroup[ iGarrisonID ].ubSectorID, EVASIVE, REINFORCEMENTS );
		}
		else
		{ //Send the whole group and kill it's patrol assignment.
			gPatrolGroup[ iBestIndex ].ubGroupID = 0;
			gGarrisonGroup[ iGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
			pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
			RecalculatePatrolWeight(gPatrolGroup[iBestIndex]);
			//The ONLY case where the group is told to move somewhere else when they could be BETWEEN sectors.  The movegroup functions
			//don't work if this is the case.  Teleporting them to their previous sector is the best and easiest way to deal with this.
			SetEnemyGroupSector( pGroup, (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) );

			MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iGarrisonID ].ubSectorID, EVASIVE, REINFORCEMENTS );
			ValidateGroup( pGroup );
		}
	}
	else
	{ //There are no groups that have enough troops.  Send a new force from the palace instead.
		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubSoldiersRequested, 0 );
		pGroup->ubMoveType = ONE_WAY;
		pGroup->pEnemyGroup->ubIntention = REINFORCEMENTS;
		gGarrisonGroup[ iGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
		pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
		giReinforcementPool -= (INT32)ubSoldiersRequested;

		MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iGarrisonID ].ubSectorID, EVASIVE, REINFORCEMENTS );
		ValidateGroup( pGroup );
	}
}


static void MassFortifyTowns(void);


void WakeUpQueen()
{
	gfQueenAIAwake = TRUE;
	if( !gfMassFortificationOrdered )
	{
		gfMassFortificationOrdered = TRUE;
		MassFortifyTowns();
	}
}


//Simply orders all garrisons to take troops from the patrol groups and send the closest troops from them.  Any garrison,
//whom there request isn't fulfilled (due to lack of troops), will recieve their reinforcements from the queen (P3).
static void MassFortifyTowns(void)
{
	INT32 i;
	SECTORINFO *pSector;
	GROUP *pGroup;
	UINT8 ubNumTroops, ubDesiredTroops;
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
		ubNumTroops = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
		ubDesiredTroops = (UINT8)gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bDesiredPopulation;
		if( ubNumTroops < ubDesiredTroops  )
		{
			if( !gGarrisonGroup[ i ].ubPendingGroupID &&
					gGarrisonGroup[ i ].ubComposition != ROADBLOCK &&
					EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) )
			{
				RequestHighPriorityGarrisonReinforcements( i, (UINT8)(ubDesiredTroops - ubNumTroops) );
			}
		}
	}
	//Convert the garrison sitting in Omerta (if alive), and reassign them
	pSector = &SectorInfo[ SEC_A9 ];
	if( pSector->ubNumTroops )
	{
		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_A9, 0, pSector->ubNumTroops, 0 );
		pSector->ubNumTroops = 0;
		pGroup->pEnemyGroup->ubIntention = PATROL;
		pGroup->ubMoveType = ONE_WAY;
		ReassignAIGroup( &pGroup );
		ValidateGroup( pGroup );
		RecalculateSectorWeight( SEC_A9 );
	}
}


static void RenderAIViewerGarrisonInfo(INT32 x, INT32 y, SECTORINFO* pSector)
{
	if( pSector->ubGarrisonID != NO_GARRISON )
	{
		INT32 iDesired, iSurplus;
		iDesired = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bDesiredPopulation;
		iSurplus = pSector->ubNumTroops + pSector->ubNumAdmins + pSector->ubNumElites - iDesired;
		SetFontForeground( FONT_WHITE );
		if( iSurplus >= 0 )
		{
			mprintf( x, y, L"%d desired, %d surplus troops", iDesired, iSurplus );
		}
		else
		{
			mprintf( x, y, L"%d desired, %d reinforcements requested", iDesired, -iSurplus );
		}
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			GROUP *pGroup;
			pGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			mprintf( x, y+10, L"%d reinforcements on route from group %d in %c%d", pGroup->ubGroupSize, pGroup->ubGroupID,
				pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
		}
		else
		{
			MPrint(x, y + 10, L"No pending reinforcements for this sector.");
		}
	}
	else
	{
		SetFontForeground( FONT_GRAY2 );
		MPrint(x, y, L"No garrison information for this sector.");
	}
}

void StrategicHandleMineThatRanOut( UINT8 ubSectorID )
{
	switch( ubSectorID )
	{
		case SEC_B2:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_A2 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_B2 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
		case SEC_D13:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_B13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_C13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_D13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
		case SEC_H8:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_F8 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_F9 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_G8 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_G9 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_H8 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
		case SEC_I14:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_H13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_H14 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_I13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_I14 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
	}
}


static BOOLEAN GarrisonCanProvideMinimumReinforcements(INT32 iGarrisonID)
{
	INT32 iAvailable;
	INT32 iDesired;
	SECTORINFO *pSector;
	UINT8 ubSectorX, ubSectorY;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];

	iAvailable = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iDesired = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;

	if( iAvailable - iDesired >= gubMinEnemyGroupSize )
	{
		//Do a more expensive check first to determine if there is a player presence here (combat in progress)
		//If so, do not provide reinforcements from here.
		ubSectorX = (UINT8)SECTORX( gGarrisonGroup[ iGarrisonID ].ubSectorID );
		ubSectorY = (UINT8)SECTORY( gGarrisonGroup[ iGarrisonID ].ubSectorID );
		if( PlayerMercsInSector( ubSectorX, ubSectorY, 0 ) || CountAllMilitiaInSector( ubSectorX, ubSectorY ) )
		{
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN GarrisonRequestingMinimumReinforcements(INT32 iGarrisonID)
{
	INT32 iAvailable;
	INT32 iDesired;
	SECTORINFO *pSector;

	if( gGarrisonGroup[ iGarrisonID ].ubPendingGroupID )
	{
		return FALSE;
	}

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iAvailable = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iDesired = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;

	if( iDesired - iAvailable >= gubMinEnemyGroupSize )
	{
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN PermittedToFillPatrolGroup(INT32 iPatrolID);


static BOOLEAN PatrolRequestingMinimumReinforcements(INT32 iPatrolID)
{
	GROUP *pGroup;

	if( gPatrolGroup[ iPatrolID ].ubPendingGroupID )
	{
		return FALSE;
	}
	if( !PermittedToFillPatrolGroup( iPatrolID ) )
	{ //if the group was defeated, it won't be considered for reinforcements again for several days
		return FALSE;
	}
	pGroup = GetGroup( gPatrolGroup[ iPatrolID ].ubGroupID );
	if( pGroup )
	{
		if( gPatrolGroup[ iPatrolID ].bSize - pGroup->ubGroupSize >= gubMinEnemyGroupSize )
		{
			return TRUE;
		}
	}
	return FALSE;
}


static void EliminateSurplusTroopsForGarrison(GROUP* pGroup, SECTORINFO* pSector)
{
	INT32 iTotal;
	iTotal = pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins +
					 pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins;
	if( iTotal <= MAX_STRATEGIC_TEAM_SIZE )
	{
		return;
	}
	iTotal -= MAX_STRATEGIC_TEAM_SIZE;
	while( iTotal )
	{
		if( pGroup->pEnemyGroup->ubNumAdmins )
		{
			if( pGroup->pEnemyGroup->ubNumAdmins < iTotal )
			{
				iTotal -= pGroup->pEnemyGroup->ubNumAdmins;
				pGroup->pEnemyGroup->ubNumAdmins = 0;
			}
			else
			{
				pGroup->pEnemyGroup->ubNumAdmins -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pSector->ubNumAdmins )
		{
			if( pSector->ubNumAdmins < iTotal )
			{
				iTotal -= pSector->ubNumAdmins;
				pSector->ubNumAdmins = 0;
			}
			else
			{
				pSector->ubNumAdmins -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pGroup->pEnemyGroup->ubNumTroops )
		{
			if( pGroup->pEnemyGroup->ubNumTroops < iTotal )
			{
				iTotal -= pGroup->pEnemyGroup->ubNumTroops;
				pGroup->pEnemyGroup->ubNumTroops = 0;
			}
			else
			{
				pGroup->pEnemyGroup->ubNumTroops -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pSector->ubNumTroops )
		{
			if( pSector->ubNumTroops < iTotal )
			{
				iTotal -= pSector->ubNumTroops;
				pSector->ubNumTroops = 0;
			}
			else
			{
				pSector->ubNumTroops -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pGroup->pEnemyGroup->ubNumElites )
		{
			if( pGroup->pEnemyGroup->ubNumElites < iTotal )
			{
				iTotal -= pGroup->pEnemyGroup->ubNumElites;
				pGroup->pEnemyGroup->ubNumElites = 0;
			}
			else
			{
				pGroup->pEnemyGroup->ubNumElites -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pSector->ubNumElites )
		{
			if( pSector->ubNumElites < iTotal )
			{
				iTotal -= pSector->ubNumElites;
				pSector->ubNumElites = 0;
			}
			else
			{
				pSector->ubNumElites -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
	}
}


// once Queen is awake, she'll gradually begin replacing admins with regular troops.  This is mainly to keep player from
// fighting many more admins once they are no longer any challenge for him.  Eventually all admins will vanish off map.
static void UpgradeAdminsToTroops(void)
{
	INT32 i;
	SECTORINFO *pSector;
	INT8 bPriority;
	UINT8 ubAdminsToCheck;
	INT16 sPatrolIndex;


	// on normal, AI evaluates approximately every 10 hrs.  There are about 130 administrators seeded on the map.
	// Some of these will be killed by the player.

	// check all garrisons for administrators
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		// skip sector if it's currently loaded, we'll never upgrade guys in those
		if ( SECTOR( gWorldSectorX, gWorldSectorY ) == gGarrisonGroup[ i ].ubSectorID )
		{
			continue;
		}

		pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];

		// if there are any admins currently in this garrison
		if ( pSector->ubNumAdmins > 0 )
		{
			bPriority = gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bPriority;

			// highest priority sectors are upgraded first. Each 1% of progress lower the
			// priority threshold required to start triggering upgrades by 10%.
			if ( ( 100 - ( 10 * HighestPlayerProgressPercentage() ) ) < bPriority )
			{
				ubAdminsToCheck = pSector->ubNumAdmins;

				while ( ubAdminsToCheck > 0)
				{
					// chance to upgrade at each check is random, and also dependant on the garrison's priority
					if ( Chance ( bPriority ) )
					{
						pSector->ubNumAdmins--;
						pSector->ubNumTroops++;
					}

					ubAdminsToCheck--;
				}
			}
		}
	}


	// check all moving enemy groups for administrators
	FOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (pGroup->ubGroupSize && !pGroup->fVehicle)
		{
			Assert ( pGroup->pEnemyGroup );

			// skip sector if it's currently loaded, we'll never upgrade guys in those
			if ( ( pGroup->ubSectorX == gWorldSectorX ) && ( pGroup->ubSectorY == gWorldSectorY ) )
			{
				continue;
			}

			// if there are any admins currently in this group
			if ( pGroup->pEnemyGroup->ubNumAdmins > 0 )
			{
				// if it's a patrol group
				if ( pGroup->pEnemyGroup->ubIntention == PATROL )
				{
					sPatrolIndex = FindPatrolGroupIndexForGroupID( pGroup->ubGroupID );
					Assert( sPatrolIndex != -1 );

					// use that patrol's priority
					bPriority = gPatrolGroup[ sPatrolIndex ].bPriority;
				}
				else	// not a patrol group
				{
					// use a default priority
					bPriority = 50;
				}

				// highest priority groups are upgraded first. Each 1% of progress lower the
				// priority threshold required to start triggering upgrades by 10%.
				if ( ( 100 - ( 10 * HighestPlayerProgressPercentage() ) ) < bPriority )
				{
					ubAdminsToCheck = pGroup->pEnemyGroup->ubNumAdmins;

					while ( ubAdminsToCheck > 0)
					{
						// chance to upgrade at each check is random, and also dependant on the group's priority
						if ( Chance ( bPriority ) )
						{
							pGroup->pEnemyGroup->ubNumAdmins--;
							pGroup->pEnemyGroup->ubNumTroops++;
						}

						ubAdminsToCheck--;
					}
				}
			}
		}
	}
}


INT16 FindPatrolGroupIndexForGroupID( UINT8 ubGroupID )
{
	INT16 sPatrolIndex;

	for( sPatrolIndex = 0; sPatrolIndex < giPatrolArraySize; sPatrolIndex++ )
	{
		if ( gPatrolGroup[ sPatrolIndex ].ubGroupID == ubGroupID )
		{
			// found it
			return( sPatrolIndex );
		}
	}

	// not there!
	return( -1 );
}


INT16 FindPatrolGroupIndexForGroupIDPending( UINT8 ubGroupID )
{
	INT16 sPatrolIndex;

	for( sPatrolIndex = 0; sPatrolIndex < giPatrolArraySize; sPatrolIndex++ )
	{
		if ( gPatrolGroup[ sPatrolIndex ].ubPendingGroupID == ubGroupID )
		{
			// found it
			return( sPatrolIndex );
		}
	}

	// not there!
	return( -1 );
}


INT16 FindGarrisonIndexForGroupIDPending( UINT8 ubGroupID )
{
	INT16 sGarrisonIndex;

	for( sGarrisonIndex = 0; sGarrisonIndex < giGarrisonArraySize; sGarrisonIndex++ )
	{
		if ( gGarrisonGroup[ sGarrisonIndex ].ubPendingGroupID == ubGroupID )
		{
			// found it
			return( sGarrisonIndex );
		}
	}

	// not there!
	return( -1 );
}


static void TransferGroupToPool(GROUP** pGroup)
{
	giReinforcementPool += (*pGroup)->ubGroupSize;
	RemoveGroup(**pGroup);
	*pGroup = NULL;
}


//NOTE:  Make sure you call SetEnemyGroupSector() first if the group is between sectors!!  See example in ReassignAIGroup()...
static void SendGroupToPool(GROUP** pGroup)
{
	if( (*pGroup)->ubSectorX == 3 && (*pGroup)->ubSectorY == 16 )
	{
		TransferGroupToPool( pGroup );
	}
	else
	{
		(*pGroup)->ubSectorIDOfLastReassignment = (UINT8)SECTOR( (*pGroup)->ubSectorX, (*pGroup)->ubSectorY );
		MoveSAIGroupToSector( pGroup, SEC_P3, EVASIVE, REINFORCEMENTS );
	}
}


static void ReassignAIGroup(GROUP** pGroup)
{
	INT32 i, iRandom;
	INT32 iWeight;
	UINT16 usDefencePoints;
	INT32 iReloopLastIndex = -1;
	UINT8 ubSectorID;

	ubSectorID = (UINT8)SECTOR( (*pGroup)->ubSectorX, (*pGroup)->ubSectorY );

	(*pGroup)->ubSectorIDOfLastReassignment = ubSectorID;

	RemoveGroupFromStrategicAILists(**pGroup);

	//First thing to do, is teleport the group to be AT the sector he is currently moving from.  Otherwise, the
	//strategic pathing can break if the group is between sectors upon reassignment.
	SetEnemyGroupSector( *pGroup, ubSectorID );

	if( giRequestPoints <= 0  )
	{ //we have no request for reinforcements, so send the group to Meduna for reassignment in the pool.
		SendGroupToPool( pGroup );
		return;
	}

	//now randomly choose who gets the reinforcements.
	// giRequestPoints is the combined sum of all the individual weights of all garrisons and patrols requesting reinforcements
	iRandom = Random( giRequestPoints );

	//go through garrisons first and begin considering where the random value dictates.  If that garrison doesn't require
	//reinforcements, it'll continue on considering all subsequent garrisons till the end of the array.  If it fails at that
	//point, it'll restart the loop at zero, and consider all garrisons to the index that was first considered by the random value.
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		RecalculateGarrisonWeight( i );
		iWeight = gGarrisonGroup[ i ].bWeight;
		if( iWeight > 0 )
		{	//if group is requesting reinforcements.
			if( iRandom < iWeight )
			{
				if( !gGarrisonGroup[ i ].ubPendingGroupID &&
						EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) &&
						GarrisonRequestingMinimumReinforcements( i ) )
				{ //This is the group that gets the reinforcements!
					if( ReinforcementsApproved( i, &usDefencePoints ) )
					{
						SendReinforcementsForGarrison( i, usDefencePoints, pGroup );
						return;
					}
				}
				if( iReloopLastIndex == -1 )
				{ //go to the next garrison and clear the iRandom value so it attempts to use all subsequent groups.
					iReloopLastIndex = i - 1;
					iRandom = 0;
				}
			}
			//Decrease the iRandom value until it hits 0.  When that happens, all garrisons will get considered until
			//we either have a match or process all of the garrisons.
			iRandom -= iWeight;
		}
	}
	if( iReloopLastIndex >= 0 )
	{ //Process the loop again to the point where the original random slot started considering, and consider
		//all of the garrisons.  If this fails, all patrol groups will be considered next.
		for( i = 0; i <= iReloopLastIndex; i++ )
		{
			RecalculateGarrisonWeight( i );
			iWeight = gGarrisonGroup[ i ].bWeight;
			if( iWeight > 0 )
			{	//if group is requesting reinforcements.
				if( !gGarrisonGroup[ i ].ubPendingGroupID &&
						EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) &&
						GarrisonRequestingMinimumReinforcements( i ) )
				{ //This is the group that gets the reinforcements!
					if( ReinforcementsApproved( i, &usDefencePoints ) )
					{
						SendReinforcementsForGarrison( i, usDefencePoints, pGroup );
						return;
					}
				}
			}
		}
	}
	if( iReloopLastIndex == -1 )
	{
		//go through the patrol groups
		for( i = 0; i < giPatrolArraySize; i++ )
		{
			RecalculatePatrolWeight(gPatrolGroup[i]);
			iWeight = gPatrolGroup[ i ].bWeight;
			if( iWeight > 0 )
			{
				if( iRandom < iWeight )
				{
					if( !gPatrolGroup[ i ].ubPendingGroupID && PatrolRequestingMinimumReinforcements( i ) )
					{ //This is the group that gets the reinforcements!
						SendReinforcementsForPatrol( i, pGroup );
						return;
					}
				}
				if( iReloopLastIndex == -1 )
				{
					iReloopLastIndex = i - 1;
					iRandom = 0;
				}
				iRandom -= iWeight;
			}
		}
	}
	else
	{
		iReloopLastIndex = giPatrolArraySize - 1;
	}

	for( i = 0; i <= iReloopLastIndex; i++ )
	{
		RecalculatePatrolWeight(gPatrolGroup[i]);
		iWeight = gPatrolGroup[ i ].bWeight;
		if( iWeight > 0 )
		{
			if( !gPatrolGroup[ i ].ubPendingGroupID && PatrolRequestingMinimumReinforcements( i ) )
			{ //This is the group that gets the reinforcements!
				SendReinforcementsForPatrol( i, pGroup );
				return;
			}
		}
	}
	TransferGroupToPool( pGroup );
}


/* When an enemy AI group is eliminated by the player, apply a grace period in
 * which the group isn't allowed to be filled for several days. */
static void TagSAIGroupWithGracePeriod(GROUP const& g)
{
	INT32 const patrol_id = FindPatrolGroupIndexForGroupID(g.ubGroupID);
	if (patrol_id == -1) return;

	UINT32 grace_period;
	switch (gGameOptions.ubDifficultyLevel)
	{
		case DIF_LEVEL_EASY:   grace_period = EASY_PATROL_GRACE_PERIOD_IN_DAYS;   break;
		case DIF_LEVEL_MEDIUM: grace_period = NORMAL_PATROL_GRACE_PERIOD_IN_DAYS; break;
		case DIF_LEVEL_HARD:   grace_period = HARD_PATROL_GRACE_PERIOD_IN_DAYS;   break;
		default:               return;
	}
	gPatrolGroup[patrol_id].bFillPermittedAfterDayMod100 = (GetWorldDay() + grace_period) % 100;
}


static BOOLEAN PermittedToFillPatrolGroup(INT32 iPatrolID)
{
	INT32 iDay;
	INT32 iDayAllowed;
	iDay = GetWorldDay();
	iDayAllowed = gPatrolGroup[ iPatrolID ].bFillPermittedAfterDayMod100 + (iDay / 100) * 100;
	return iDay >= iDayAllowed;
}

void RepollSAIGroup( GROUP *pGroup )
{
	INT32 i;
	Assert( !pGroup->fPlayer );
	if( GroupAtFinalDestination( pGroup ) )
	{
		EvaluateGroupSituation( pGroup );
		return;
	}
	for( i = 0; i < giPatrolArraySize; i++ )
	{
		if( gPatrolGroup[ i ].ubGroupID == pGroup->ubGroupID )
		{
			RecalculatePatrolWeight(gPatrolGroup[i]); //in case there are any dead enemies
			CalculateNextMoveIntention( pGroup );
			return;
		}
	}
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		//KM : August 6, 1999 Patch fix
		//     Ack, wasn't checking for the matching group to garrison
		if( gGarrisonGroup[ i ].ubPendingGroupID == pGroup->ubGroupID )
		//end
		{
			RecalculateGarrisonWeight( i ); //in case there are any dead enemies
			CalculateNextMoveIntention( pGroup );
			return;
		}
	}
}


static void CalcNumTroopsBasedOnComposition(UINT8* pubNumTroops, UINT8* pubNumElites, UINT8 ubTotal, INT32 iCompositionID)
{
	*pubNumTroops = gArmyComp[ iCompositionID ].bTroopPercentage * ubTotal / 100;
	*pubNumElites = gArmyComp[ iCompositionID ].bElitePercentage * ubTotal / 100;

	//Due to low roundoff, it is highly possible that we will be short one soldier.
	while( *pubNumTroops + *pubNumElites < ubTotal )
	{
		if( Chance( gArmyComp[ iCompositionID ].bTroopPercentage ) )
		{
			(*pubNumTroops)++;
		}
		else
		{
			(*pubNumElites)++;
		}
	}
	Assert( *pubNumTroops + *pubNumElites == ubTotal );
}


static void ConvertGroupTroopsToComposition(GROUP* pGroup, INT32 iCompositionID)
{
	Assert( pGroup );
	Assert( !pGroup->fPlayer );
	CalcNumTroopsBasedOnComposition( &pGroup->pEnemyGroup->ubNumTroops, &pGroup->pEnemyGroup->ubNumElites, pGroup->ubGroupSize, iCompositionID );
	pGroup->pEnemyGroup->ubNumAdmins = 0;
	pGroup->ubGroupSize = pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites;
	ValidateLargeGroup( pGroup );
}


static void RemoveSoldiersFromGarrisonBasedOnComposition(INT32 iGarrisonID, UINT8 ubSize)
{
	SECTORINFO *pSector;
	INT32 iCompositionID;
	UINT8 ubNumTroops, ubNumElites;

	iCompositionID = gGarrisonGroup[ iGarrisonID ].ubComposition;

	CalcNumTroopsBasedOnComposition( &ubNumTroops, &ubNumElites, ubSize, iCompositionID );
	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	//if there are administrators in this sector, remove them first.

	while( ubSize && pSector->ubNumAdmins )
	{
		pSector->ubNumAdmins--;
		ubSize--;
		if( ubNumTroops )
		{
			ubNumTroops--;
		}
		else
		{
			ubNumElites--;
		}
	}
	//No administrators are left.

	//Eliminate the troops
	while( ubNumTroops )
	{
		if( pSector->ubNumTroops )
		{
			pSector->ubNumTroops--;
		}
		else if( pSector->ubNumElites )
		{
			pSector->ubNumElites--;
		}
		else
		{
			Assert( 0 );
		}
		ubNumTroops--;
	}

	//Eliminate the elites
	while( ubNumElites )
	{
		if( pSector->ubNumElites )
		{
			pSector->ubNumElites--;
		}
		else if( pSector->ubNumTroops )
		{
			pSector->ubNumTroops--;
		}
		else
		{
			Assert( 0 );
		}
		ubNumElites--;
	}

	RecalculateGarrisonWeight( iGarrisonID );
}


static void MoveSAIGroupToSector(GROUP** pGroup, UINT8 ubSectorID, UINT32 uiMoveCode, UINT8 ubIntention)
{
	UINT8 ubDstSectorX, ubDstSectorY;

	ubDstSectorX = (UINT8)SECTORX( ubSectorID );
	ubDstSectorY = (UINT8)SECTORY( ubSectorID );

	if( (*pGroup)->fBetweenSectors )
	{
		SetEnemyGroupSector( *pGroup, (UINT8)SECTOR( (*pGroup)->ubSectorX, (*pGroup)->ubSectorY ) );
	}

	(*pGroup)->pEnemyGroup->ubIntention = ubIntention;
	(*pGroup)->ubMoveType = ONE_WAY;

	if( ubIntention == PURSUIT )
	{	//Make sure that the group isn't moving into a garrison sector.  These sectors should be using ASSAULT intentions!
		if( SectorInfo[ ubSectorID ].ubGarrisonID != NO_GARRISON )
		{
			//Good place for a breakpoint.
			pGroup = pGroup;
		}
	}

	if( (*pGroup)->ubSectorX == ubDstSectorX && (*pGroup)->ubSectorY == ubDstSectorY )
	{ //The destination sector is the current location.  Instead of causing code logic problems,
		//simply process them as if they just arrived.
		if( EvaluateGroupSituation( *pGroup ) )
		{ //The group was deleted.
			*pGroup = NULL;
			return;
		}
	}

	switch( uiMoveCode )
	{
		case STAGE:
			MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd(**pGroup, (*pGroup)->ubSectorX, (*pGroup)->ubSectorY, ubDstSectorX, ubDstSectorY);
			break;
		case EVASIVE:
			MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors(**pGroup, (*pGroup)->ubSectorX, (*pGroup)->ubSectorY, ubDstSectorX, ubDstSectorY);
			break;
		case DIRECT:
		default:
			MoveGroupFromSectorToSector(**pGroup, (*pGroup)->ubSectorX, (*pGroup)->ubSectorY, ubDstSectorX, ubDstSectorY);
			break;
	}
	//Make sure that the group is moving.  If this fails, then the pathing may have failed for some reason.
	ValidateGroup( *pGroup );
}


//If there are any enemy groups that will be moving through this sector due, they will have to repath which
//will cause them to avoid the sector.  Returns the number of redirected groups.
static UINT8 RedirectEnemyGroupsMovingThroughSector(UINT8 ubSectorX, UINT8 ubSectorY)
{
	UINT8 ubNumGroupsRedirected = 0;
	WAYPOINT *pWaypoint;
	UINT8 ubDestSectorID;
	FOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (pGroup->ubMoveType == ONE_WAY)
		{ //check the waypoint list
			if( GroupWillMoveThroughSector( pGroup, ubSectorX, ubSectorY ) )
			{
				//extract the group's destination.
				pWaypoint = GetFinalWaypoint( pGroup );
				Assert( pWaypoint );
				ubDestSectorID = (UINT8)SECTOR( pWaypoint->x, pWaypoint->y );
				SetEnemyGroupSector( pGroup, (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) );
				MoveSAIGroupToSector( &pGroup, ubDestSectorID, EVASIVE, pGroup->pEnemyGroup->ubIntention );
				ubNumGroupsRedirected++;
			}
		}
	}
	if( ubNumGroupsRedirected )
	{
		ScreenMsg( FONT_LTBLUE, MSG_BETAVERSION, L"Test message for new feature:  %d enemy groups were redirected away from moving through sector %c%d.  Please don't report unless this number is greater than 5.",
			ubNumGroupsRedirected, ubSectorY + 'A' - 1, ubSectorX );
	}
	return ubNumGroupsRedirected;
}


//when the SAI compositions change, it is necessary to call this function upon version load,
//to reflect the changes of the compositions to the sector that haven't been visited yet.
static void ReinitializeUnvisitedGarrisons(void)
{
	SECTORINFO *pSector;
	ARMY_COMPOSITION *pArmyComp;
	GROUP *pGroup;
	INT32 i, cnt, iEliteChance, iAdminChance;

	//Recreate the compositions
	memcpy( gArmyComp, gOrigArmyComp, NUM_ARMY_COMPOSITIONS * sizeof( ARMY_COMPOSITION ) );
	EvolveQueenPriorityPhase( TRUE );

	//Go through each unvisited sector and recreate the garrison forces based on
	//the desired population.
	for( i = 0; i < giGarrisonArraySize; i++ )
	{
		if( gGarrisonGroup[ i ].ubComposition >= LEVEL1_DEFENCE && gGarrisonGroup[ i ].ubComposition <= LEVEL3_DEFENCE )
		{ //These 3 compositions make up the perimeter around Meduna.  The existance of these are based on the
			//difficulty level, and we don't want to reset these anyways, due to the fact that many of the reinforcements
			//come from these sectors, and it could potentially add upwards of 150 extra troops which would seriously
			//unbalance the difficulty.
			continue;
		}
		pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
		pArmyComp = &gArmyComp[ gGarrisonGroup[ i ].ubComposition ];
		if( !(pSector->uiFlags & SF_ALREADY_VISITED) )
		{
			pSector->ubNumAdmins = 0;
			pSector->ubNumTroops = 0;
			pSector->ubNumElites = 0;
			if( gfQueenAIAwake )
			{
				cnt = pArmyComp->bDesiredPopulation;
			}
			else
			{
				cnt = pArmyComp->bStartPopulation;
			}

			if( gGarrisonGroup[ i ].ubPendingGroupID )
			{ //if the garrison has reinforcements on route, then subtract the number of
				//reinforcements from the value we reset the size of the garrison.  This is to
				//prevent overfilling the group.
				pGroup = GetGroup( gGarrisonGroup[ i ].ubPendingGroupID );
				if( pGroup )
				{
					cnt -= pGroup->ubGroupSize;
					cnt = MAX( cnt, 0 );
				}
			}

			iEliteChance = pArmyComp->bElitePercentage;
			iAdminChance = pArmyComp->bAdminPercentage;
			if( iAdminChance && !gfQueenAIAwake && cnt )
			{
				pSector->ubNumAdmins = iAdminChance * cnt / 100;
			}
			else while( cnt-- )
			{ //for each person, randomly determine the types of each soldier.
				if( Chance( iEliteChance ) )
				{
					pSector->ubNumElites++;
				}
				else
				{
					pSector->ubNumTroops++;
				}
			}
		}
	}
}
