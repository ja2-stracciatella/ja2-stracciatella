#ifndef __OVERHEAD_H
#define __OVERHEAD_H

#include "Debug.h"
#include "Soldier_Control.h"


#define MAX_REALTIME_SPEED_VAL		10


// Enums for waiting for mercs to finish codes
enum
{
	NO_WAIT_EVENT = 0,
	WAIT_FOR_MERCS_TO_WALKOFF_SCREEN,
	WAIT_FOR_MERCS_TO_WALKON_SCREEN,
	WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO
};


// TACTICAL ENGINE STATUS FLAGS
struct TacticalTeamType
{
	UINT8        bFirstID;
	UINT8        bLastID;
	COLORVAL     RadarColor;
	INT8         bSide;
	INT8         bMenInSector;
	INT8         bAwareOfOpposition;
	INT8         bHuman;
	SOLDIERTYPE* last_merc_to_radio;
};


#define PANIC_BOMBS_HERE		0x01
#define PANIC_TRIGGERS_HERE		0x02

#define NUM_PANIC_TRIGGERS		3

#define ENEMY_OFFERED_SURRENDER	0x01

struct TacticalStatusType
{
	UINT32           uiFlags;
	TacticalTeamType Team[MAXTEAMS];
	UINT8            ubCurrentTeam;
	INT16            sSlideTarget;
	UINT32           uiTimeSinceMercAIStart;
	INT8             fPanicFlags;
	UINT8            ubSpottersCalledForBy;
	SOLDIERTYPE*     the_chosen_one;
	UINT32           uiTimeOfLastInput;
	UINT32           uiTimeSinceDemoOn;
	BOOLEAN          fCivGroupHostile[ NUM_CIV_GROUPS ];
	UINT8            ubLastBattleSectorX;
	UINT8            ubLastBattleSectorY;
	BOOLEAN          fLastBattleWon;
	BOOLEAN          fVirginSector;
	BOOLEAN          fEnemyInSector;
	BOOLEAN          fInterruptOccurred;
	INT8             bRealtimeSpeed;
	SOLDIERTYPE*     enemy_sighting_on_their_turn_enemy;
	BOOLEAN          fEnemySightingOnTheirTurn;
	BOOLEAN          fAutoBandageMode;
	UINT8            ubAttackBusyCount;
	UINT8            ubEngagedInConvFromActionMercID;
	UINT16           usTactialTurnLimitCounter;
	BOOLEAN          fInTopMessage;
	UINT8            ubTopMessageType;
	UINT16           usTactialTurnLimitMax;
	UINT32           uiTactialTurnLimitClock;
	BOOLEAN          fTactialTurnLimitStartedBeep;
	INT8             bBoxingState;
	INT8             bConsNumTurnsNotSeen;
	UINT8            ubArmyGuysKilled;

	INT16            sPanicTriggerGridNo[NUM_PANIC_TRIGGERS];
	INT8             bPanicTriggerIsAlarm[NUM_PANIC_TRIGGERS];
	UINT8            ubPanicTolerance[NUM_PANIC_TRIGGERS];
	BOOLEAN          fAtLeastOneGuyOnMultiSelect;
	BOOLEAN          fKilledEnemyOnAttack;
	SOLDIERTYPE*     enemy_killed_on_attack;
	INT8             bEnemyKilledOnAttackLevel;
	UINT16           ubEnemyKilledOnAttackLocation;
	BOOLEAN          fItemsSeenOnAttack;
	SOLDIERTYPE*     items_seen_on_attack_soldier;
	UINT16           usItemsSeenOnAttackGridNo;
	BOOLEAN          fLockItemLocators;
	UINT8            ubLastQuoteSaid;
	UINT8            ubLastQuoteProfileNUm;
	BOOLEAN          fCantGetThrough;
	INT16            sCantGetThroughGridNo;
	INT16            sCantGetThroughSoldierGridNo;
	SOLDIERTYPE*     cant_get_through;
	BOOLEAN          fDidGameJustStart;
	UINT8            ubLastRequesterTargetID;
	UINT8            ubNumCrowsPossible;
	BOOLEAN          fUnLockUIAfterHiddenInterrupt;
	INT8             bNumFoughtInBattle[ MAXTEAMS ];
	UINT32           uiDecayBloodLastUpdate;
	UINT32           uiTimeSinceLastInTactical;
	BOOLEAN          fHasAGameBeenStarted;
	INT8             bConsNumTurnsWeHaventSeenButEnemyDoes;
	BOOLEAN          fSomeoneHit;
	UINT32           uiTimeSinceLastOpplistDecay;
	SOLDIERTYPE*     enemy_killed_on_attack_killer;
	INT8             bMercArrivingQuoteBeingUsed;
	BOOLEAN          fCountingDownForGuideDescription;
	INT8             bGuideDescriptionCountDown;
	UINT8            ubGuideDescriptionToUse;
	INT8             bGuideDescriptionSectorX;
	INT8             bGuideDescriptionSectorY;
	INT8             fEnemyFlags;
	BOOLEAN          fAutoBandagePending;
	BOOLEAN          fHasEnteredCombatModeSinceEntering;
	BOOLEAN          fDontAddNewCrows;
	UINT16           sCreatureTenseQuoteDelay;
	UINT32           uiCreatureTenseQuoteLastUpdate;
};


static inline bool IsOnOurTeam(SOLDIERTYPE const& s)
{
	return s.bTeam == OUR_TEAM;
}

extern SOLDIERTYPE* g_selected_man;

extern const char* const gzActionStr[];

// Soldier List used for all soldier overhead interaction
extern SOLDIERTYPE Menptr[TOTAL_SOLDIERS];

static inline SOLDIERTYPE& GetMan(UINT const idx)
{
	Assert(idx < lengthof(Menptr));
	return Menptr[idx];
}

typedef UINT8 SoldierID;

static inline SoldierID Soldier2ID(const SOLDIERTYPE* const s)
{
	return s != NULL ? s->ubID : NOBODY;
}

static inline SOLDIERTYPE* ID2Soldier(const SoldierID id)
{
	return id != NOBODY ? &GetMan(id) : 0;
}

// For temporary use
#define SOLDIER2ID(s)			(Soldier2ID((s)))
#define ID2SOLDIER(i)			(ID2Soldier((i)))

static inline SOLDIERTYPE* GetSelectedMan(void)
{
	SOLDIERTYPE* const sel = g_selected_man;
	Assert(sel == NULL || sel->bActive);
	return sel;
}

static inline void SetSelectedMan(SOLDIERTYPE* const s)
{
	Assert(s == NULL || s->bActive);
	g_selected_man = s;
}

// MERC SLOTS - A LIST OF ALL ACTIVE MERCS
extern SOLDIERTYPE* MercSlots[TOTAL_SOLDIERS];
extern UINT32       guiNumMercSlots;

#define FOR_EACH_MERC(iter)                                           \
	for (SOLDIERTYPE** iter = MercSlots,                   \
		** const end__##iter = MercSlots + guiNumMercSlots; \
		iter != end__##iter;                                           \
		++iter)                                                        \
		if (Assert(!*iter || (*iter)->bActive), !*iter) continue; else


extern TacticalStatusType gTacticalStatus;

static inline BOOLEAN IsTeamActive(const UINT team)
{
	return gTacticalStatus.Team[team].bMenInSector > 0;
}


#define BASE_FOR_EACH_IN_TEAM(type, iter, team)                                      \
	for (type* iter = &Menptr[gTacticalStatus.Team[(team)].bFirstID],    \
		* const end__##iter = &Menptr[gTacticalStatus.Team[(team)].bLastID + 1]; \
		iter != end__##iter;                                                         \
		++iter)                                                                      \
		if (!iter->bActive) continue; else
#define FOR_EACH_IN_TEAM( iter, team) BASE_FOR_EACH_IN_TEAM(      SOLDIERTYPE, iter, (team))
#define CFOR_EACH_IN_TEAM(iter, team) BASE_FOR_EACH_IN_TEAM(const SOLDIERTYPE, iter, (team))

#define BASE_FOR_EACH_SOLDIER(type, iter)                  \
	for (type* iter = Menptr; iter != Menptr + MAX_NUM_SOLDIERS; ++iter) \
		if (!iter->bActive) continue; else
#define FOR_EACH_SOLDIER( iter) BASE_FOR_EACH_SOLDIER(      SOLDIERTYPE, iter)
#define CFOR_EACH_SOLDIER(iter) BASE_FOR_EACH_SOLDIER(const SOLDIERTYPE, iter)

#define BASE_FOR_EACH_NON_PLAYER_SOLDIER(type, iter)                                    \
	for (type* iter = &Menptr[gTacticalStatus.Team[ENEMY_TEAM].bFirstID],    \
		* const end__##iter = &Menptr[gTacticalStatus.Team[CIV_TEAM  ].bLastID + 1]; \
		iter != end__##iter;                                                             \
		++iter)                                                                          \
		if (!iter->bActive) continue; else
#define FOR_EACH_NON_PLAYER_SOLDIER( iter) BASE_FOR_EACH_NON_PLAYER_SOLDIER(      SOLDIERTYPE, iter)
#define CFOR_EACH_NON_PLAYER_SOLDIER(iter) BASE_FOR_EACH_NON_PLAYER_SOLDIER(const SOLDIERTYPE, iter)


void InitTacticalEngine(void);
void ShutdownTacticalEngine(void);


void InitOverhead();
void ShutdownOverhead(void);

INT16 NewOKDestination(const SOLDIERTYPE* pCurrSoldier, INT16 sGridNo, BOOLEAN fPeopleToo, INT8 bLevel);

//Simple check to see if a (one-tiled) soldier can occupy a given location on the ground or roof.
extern BOOLEAN IsLocationSittable( INT32 iMapIndex, BOOLEAN fOnRoof );
extern BOOLEAN IsLocationSittableExcludingPeople( INT32 iMapIndex, BOOLEAN fOnRoof );
extern BOOLEAN FlatRoofAboveGridNo( INT32 iMapIndex );


void ExecuteOverhead(void);

void EndTurn(UINT8 ubNextTeam);
void StartPlayerTeamTurn( BOOLEAN fDoBattleSnd, BOOLEAN fEnteringCombatMode );

enum SelSoldierFlags
{
	SELSOLDIER_NONE           = 0,
	SELSOLDIER_ACKNOWLEDGE    = 1U << 0,
	SELSOLDIER_FORCE_RESELECT = 1U << 1,
	SELSOLDIER_FROM_UI        = 1U << 2
};
ENUM_BITSET(SelSoldierFlags)

void SelectSoldier(SOLDIERTYPE* s, SelSoldierFlags flags);


void InternalLocateGridNo(UINT16 sGridNo, BOOLEAN fForce);
void LocateGridNo( UINT16 sGridNo );
void LocateSoldier(SOLDIERTYPE* s, BOOLEAN fSetLocator);

void BeginTeamTurn( UINT8 ubTeam );
void SlideTo(SOLDIERTYPE* tgt, BOOLEAN fSetLocator);
void SlideToLocation(INT16 sDestGridNo);

void RebuildAllSoldierShadeTables(void);
void HandlePlayerTeamMemberDeath( SOLDIERTYPE *pSoldier );

SOLDIERTYPE* FindNextActiveAndAliveMerc(const SOLDIERTYPE* curr, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs);
SOLDIERTYPE* FindPrevActiveAndAliveMerc(const SOLDIERTYPE* curr, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs);

void HandleNPCTeamMemberDeath( SOLDIERTYPE *pSoldier );

BOOLEAN UIOKMoveDestination(const SOLDIERTYPE* pSoldier, UINT16 usMapPos);

INT16 FindAdjacentGridEx(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8* pubDirection, INT16* psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor);
INT16 FindNextToAdjacentGridEx( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 *pubDirection, INT16 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor );


void SelectNextAvailSoldier(const SOLDIERTYPE* s);
BOOLEAN TeamMemberNear(INT8 bTeam, INT16 sGridNo, INT32 iRange);


// FUNCTIONS FOR MANIPULATING MERC SLOTS - A LIST OF ALL ACTIVE MERCS
void    AddMercSlot(SOLDIERTYPE* pSoldier);
BOOLEAN RemoveMercSlot( SOLDIERTYPE *pSoldier  );

INT32   AddAwaySlot(SOLDIERTYPE* pSoldier);
BOOLEAN RemoveAwaySlot(SOLDIERTYPE* pSoldier);
INT32   MoveSoldierFromMercToAwaySlot(SOLDIERTYPE* pSoldier);
void    MoveSoldierFromAwayToMercSlot(SOLDIERTYPE* pSoldier);

void EnterCombatMode( UINT8 ubStartingTeam );
void ExitCombatMode(void);

void HandleTeamServices( UINT8 ubTeamNum );
void HandlePlayerServices(SOLDIERTYPE&);

void SetEnemyPresence(void);

void CycleThroughKnownEnemies(void);

BOOLEAN CheckForEndOfCombatMode( BOOLEAN fIncrementTurnsNotSeen );

SOLDIERTYPE* FreeUpAttacker(SOLDIERTYPE* attacker);

BOOLEAN PlayerTeamFull(void);

void SetActionToDoOnceMercsGetToLocation(UINT8 ubActionCode, INT8 bNumMercsWaiting);

void ResetAllMercSpeeds(void);

BOOLEAN HandleGotoNewGridNo( SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving, BOOLEAN fInitialMove, UINT16 usAnimState );

SOLDIERTYPE* ReduceAttackBusyCount(SOLDIERTYPE* attacker, BOOLEAN fCalledByAttacker);

void CommonEnterCombatModeCode(void);

void CheckForPotentialAddToBattleIncrement( SOLDIERTYPE *pSoldier );

void CencelAllActionsForTimeCompression( void );

BOOLEAN CheckForEndOfBattle( BOOLEAN fAnEnemyRetreated );

void AddManToTeam( INT8 bTeam );

void RemoveManFromTeam( INT8 bTeam );

void RemoveSoldierFromTacticalSector(SOLDIERTYPE&);

void MakeCivHostile( SOLDIERTYPE *pSoldier, INT8 bNewSide );

#define REASON_NORMAL_ATTACK		1
#define REASON_EXPLOSION		2

BOOLEAN ProcessImplicationsOfPCAttack(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pTarget, INT8 bReason);

INT16 FindAdjacentPunchTarget(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pTargetSoldier, INT16* psAdjustedTargetGridNo);

SOLDIERTYPE * CivilianGroupMemberChangesSides( SOLDIERTYPE * pAttacked );
void CivilianGroupChangesSides( UINT8 ubCivilianGroup );

void CycleVisibleEnemies( SOLDIERTYPE *pSrcSoldier );
UINT8 CivilianGroupMembersChangeSidesWithinProximity( SOLDIERTYPE * pAttacked );

void PauseAITemporarily( void );
void PauseAIUntilManuallyUnpaused( void );
void UnPauseAI( void );

void DoPOWPathChecks( void );

BOOLEAN HostileCiviliansPresent( void );
BOOLEAN HostileBloodcatsPresent( void );
UINT8 NumPCsInSector( void );
UINT8 NumEnemyInSector();

void SetSoldierNonNeutral( SOLDIERTYPE * pSoldier );
void SetSoldierNeutral( SOLDIERTYPE * pSoldier );

void         CaptureTimerCallback(void);
SOLDIERTYPE* FindNextActiveSquad(SOLDIERTYPE* pSoldier);

extern BOOLEAN gfSurrendered;
extern BOOLEAN gfKillingGuysForLosingBattle;
extern UINT8   gubWaitingForAllMercsToExitCode;

UINT8        NumCapableEnemyInSector();
SOLDIERTYPE* FreeUpAttackerGivenTarget(SOLDIERTYPE* target);
SOLDIERTYPE* ReduceAttackBusyGivenTarget(SOLDIERTYPE* target);

UINT NumberOfMercsOnPlayerTeam(void);

void InitializeTacticalStatusAtBattleStart(void);

void MakeCharacterDialogueEventSignalItemLocatorStart(SOLDIERTYPE&, GridNo location);

void HandleThePlayerBeNotifiedOfSomeoneElseInSector(void);

bool WildernessSectorWithAllProfiledNPCsNotSpokenWith(INT16 const x, INT16 const y, INT8 const z);

#endif
