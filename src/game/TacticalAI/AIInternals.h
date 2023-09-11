#ifndef TACTICALAI_AIINTERNALS_H
#define TACTICALAI_AIINTERNALS_H

#include "JA2Types.h"
#include "Overhead_Types.h"


extern BOOLEAN gfTurnBasedAI;

// THIS IS AN ITEM #  - AND FOR NOW JUST COMPLETELY FAKE...

#define MAX_TOSS_SEARCH_DIST    1       // must throw within this of opponent
#define NPC_TOSS_SAFETY_MARGIN  4       // all friends must be this far away

#define ACTING_ON_SCHEDULE( p ) ( (p)->fAIFlags & AI_CHECK_SCHEDULE )

// the AI should try to have this many APs before climbing a roof, if possible
#define AI_AP_CLIMBROOF 15

#define TEMPORARILY     0
#define FOREVER         1

#define IGNORE_PATH             0
#define ENSURE_PATH             1
#define ENSURE_PATH_COST        2

//Kris:  November 10, 1997
//Please don't change this value from 10.  It will invalidate all of the maps and soldiers.
#define MAXPATROLGRIDS  10

#define NOWATER         0
#define WATEROK         1

constexpr bool DONTADDTURNCOST = false;
constexpr bool ADDTURNCOST     = true;

enum
{
	URGENCY_LOW = 0,
	URGENCY_MED,
	URGENCY_HIGH,
	NUM_URGENCY_STATES
};

#define NOWATER         0
#define WATEROK         1

#define IGNORE_PATH             0
#define ENSURE_PATH             1
#define ENSURE_PATH_COST        2

#define MAX_ROAMING_RANGE       WORLD_COLS

#define PTR_CIV_OR_MILITIA (IsOnCivTeam(pSoldier) || pSoldier->bTeam == MILITIA_TEAM)

#define REALTIME_AI_DELAY (10000 + Random( 1000 ))
#define REALTIME_CIV_AI_DELAY ( 1000 * (gTacticalStatus.Team[ MILITIA_TEAM ].bMenInSector + gTacticalStatus.Team[ CIV_TEAM ].bMenInSector) + 5000 + 2000 * Random( 3 ) )
#define REALTIME_CREATURE_AI_DELAY ( 10000 + 1000 * Random( 3 ) )

#define NOSHOOT_WAITABIT        -1
#define NOSHOOT_WATER           -2
#define NOSHOOT_MYSELF          -3
#define NOSHOOT_HURT            -4
#define NOSHOOT_NOAMMO          -5
#define NOSHOOT_NOLOAD          -6
#define NOSHOOT_NOWEAPON        -7

#define PERCENT_TO_IGNORE_THREAT        50      // any less, use threat value
#define ACTION_TIMEOUT_CYCLES	50	// # failed cycles through AI
#define MAX_THREAT_RANGE  	400	// 30 tiles worth
#define MIN_PERCENT_BETTER	5	// 5% improvement in cover is good


#define TOSSES_PER_10TURNS      18      // max # of grenades tossable in 10 turns
#define SHELLS_PER_10TURNS      13      // max # of shells   firable  in 10 turns

#define SEE_THRU_COVER_THRESHOLD        5      // min chance to get through

struct THREATTYPE
{
	SOLDIERTYPE *pOpponent;
	INT16       sGridNo;
	INT32       iValue;
	INT32       iAPs;
	INT32       iCertainty;
	INT32       iOrigRange;
};

// define for bAimTime for bursting
#define BURSTING 5

struct ATTACKTYPE
{
	SOLDIERTYPE* opponent;     // which soldier is the victim?
	UINT8 ubPossible;          // is this attack form possible?  T/F
	UINT8 ubAimTime;           // how many extra APs to spend on aiming
	UINT8 ubChanceToReallyHit; // chance to hit * chance to get through cover
	INT32 iAttackValue;        // relative worthiness of this type of attack
	INT16 sTarget;             // target gridno of this attack
	INT8  bTargetLevel;        // target level of this attack
	UINT8 ubAPCost;            // how many APs the attack will use up
	INT8  bWeaponIn;           // the inv slot of the weapon in question
};

extern THREATTYPE Threat[MAXMERCS];
extern int ThreatPercent[10];
extern UINT8 SkipCoverCheck;

enum ItemSearchReason
{
	SEARCH_GENERAL_ITEMS,
	SEARCH_AMMO,
	SEARCH_WEAPONS
};

// go as far as possible flags
#define FLAG_CAUTIOUS		0x01
#define FLAG_STOPSHORT	0x02

#define STOPSHORTDIST 5

INT16 AdvanceToFiringRange( SOLDIERTYPE * pSoldier, INT16 sClosestOpponent );

void CalcBestShot(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestShot);
void CalcBestStab(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab, BOOLEAN fBladeAttack);
void CalcTentacleAttack(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestStab );

INT16 CalcSpreadBurst( SOLDIERTYPE * pSoldier, INT16 sFirstTarget, INT8 bTargetLevel );
INT32 CalcManThreatValue(SOLDIERTYPE *pSoldier, INT16 sMyGrid, UINT8 ubReduceForCover, SOLDIERTYPE * pMe );
INT8 CanNPCAttack(SOLDIERTYPE *pSoldier);
void CheckIfTossPossible(SOLDIERTYPE *pSoldier, ATTACKTYPE *pBestThrow);
BOOLEAN ClimbingNecessary( SOLDIERTYPE * pSoldier, INT16 sDestGridNo, INT8 bDestLevel );
INT8 ClosestPanicTrigger( SOLDIERTYPE * pSoldier );
INT16 ClosestReachableDisturbance(SOLDIERTYPE *pSoldier, UINT8 ubUnconsciousOK, BOOLEAN * pfChangeLevel );
INT16 ClosestReachableFriendInTrouble(SOLDIERTYPE *pSoldier, BOOLEAN * pfClimbingNecessary);
INT16 ClosestSeenOpponent(SOLDIERTYPE *pSoldier, INT16 * psGridNo, INT8 * pbLevel);
void CreatureCall( SOLDIERTYPE * pCaller );
INT8 CreatureDecideAction( SOLDIERTYPE * pCreature );
void CreatureDecideAlertStatus( SOLDIERTYPE *pCreature );
INT8 CrowDecideAction( SOLDIERTYPE * pSoldier );
void DecideAlertStatus( SOLDIERTYPE *pSoldier );
INT8 DecideAutoBandage( SOLDIERTYPE * pSoldier );
UINT16 DetermineMovementMode( SOLDIERTYPE * pSoldier, INT8 bAction );

INT16 EstimatePathCostToLocation( SOLDIERTYPE * pSoldier, INT16 sDestGridNo, INT8 bDestLevel, BOOLEAN fAddCostAfterClimbingUp, BOOLEAN * pfClimbingNecessary, INT16 * psClimbGridNo );

bool FindBetterSpotForItem(SOLDIERTYPE&, INT8 slot);
INT16 FindClosestBoxingRingSpot( SOLDIERTYPE * pSoldier, BOOLEAN fInRing );
INT16 GetInterveningClimbingLocation( SOLDIERTYPE * pSoldier, INT16 sDestGridNo, INT8 bDestLevel, BOOLEAN * pfClimbingNecessary );
UINT8 GetTraversalQuoteActionID( INT8 bDirection );
INT16 GoAsFarAsPossibleTowards(SOLDIERTYPE *pSoldier, INT16 sDesGrid, INT8 bAction);

INT8 HeadForTheStairCase( SOLDIERTYPE * pSoldier );

bool InGas(SOLDIERTYPE const*, GridNo);
bool InGasOrSmoke(SOLDIERTYPE const*, GridNo);
bool InWaterGasOrSmoke(SOLDIERTYPE const*, GridNo);

void InitAttackType(ATTACKTYPE *pAttack);

INT16 InternalGoAsFarAsPossibleTowards(SOLDIERTYPE *pSoldier, INT16 sDesGrid, INT8 bReserveAPs, INT8 bAction, INT8 fFlags );

int LegalNPCDestination(SOLDIERTYPE *pSoldier, INT16 sGridno, UINT8 ubPathMode, UINT8 ubWaterOK, UINT8 fFlags);
void LoadWeaponIfNeeded(SOLDIERTYPE *pSoldier);
INT16 MostImportantNoiseHeard( SOLDIERTYPE *pSoldier, INT32 *piRetValue, BOOLEAN * pfClimbingNecessary, BOOLEAN * pfReachable );
void NPCDoesAct(SOLDIERTYPE *pSoldier);
INT8 OKToAttack(const SOLDIERTYPE *ptr, int target);
BOOLEAN NeedToRadioAboutPanicTrigger( void );
bool PointPatrolAI(SOLDIERTYPE *pSoldier);
void PossiblyMakeThisEnemyChosenOne( SOLDIERTYPE * pSoldier );
bool RandomPointPatrolAI(SOLDIERTYPE *pSoldier);
INT32 RangeChangeDesire( const SOLDIERTYPE * pSoldier );
UINT16 RealtimeDelay( SOLDIERTYPE * pSoldier );
void RearrangePocket(SOLDIERTYPE *pSoldier, INT8 bPocket1, INT8 bPocket2, UINT8 bPermanent);
void RTHandleAI( SOLDIERTYPE * pSoldier );
INT8  SearchForItems(SOLDIERTYPE&, ItemSearchReason, UINT16 usItem);
UINT8 ShootingStanceChange( SOLDIERTYPE * pSoldier, ATTACKTYPE * pAttack, INT8 bDesiredDirection );
UINT8 StanceChange( SOLDIERTYPE * pSoldier, UINT8 ubAttackAPCost );
INT16 TrackScent( SOLDIERTYPE * pSoldier );
void RefreshAI(SOLDIERTYPE *pSoldier);
BOOLEAN InLightAtNight( INT16 sGridNo, INT8 bLevel );
INT16 FindNearbyDarkerSpot( SOLDIERTYPE *pSoldier );

BOOLEAN ArmySeesOpponents( void );

#endif
