#ifndef __OVERHEAD_H
#define __OVERHEAD_H

#include "Debug.h"
#include "Soldier_Control.h"


#define MAX_REALTIME_SPEED_VAL 10


// Enums for waiting for mercs to finish codes
enum
{
	NO_WAIT_EVENT = 0,
	WAIT_FOR_MERCS_TO_WALKOFF_SCREEN,
	WAIT_FOR_MERCS_TO_WALKON_SCREEN,
	WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO
};


// TACTICAL ENGINE STATUS FLAGS
typedef struct
{
	UINT8    bFirstID;
	UINT8    bLastID;
	COLORVAL RadarColor;
	INT8     bSide;
	INT8     bMenInSector;
	UINT8    ubLastMercToRadio;
	INT8     bTeamActive;
	INT8     bAwareOfOpposition;
	INT8     bHuman;
} TacticalTeamType;


#define PANIC_BOMBS_HERE    0x01
#define PANIC_TRIGGERS_HERE 0x02

#define NUM_PANIC_TRIGGERS 3

#define ENEMY_OFFERED_SURRENDER 0x01

typedef struct
{
	UINT32           uiFlags;
	TacticalTeamType Team[MAXTEAMS];
	UINT8            ubCurrentTeam;
	INT16            sSlideTarget;
	INT16            sSlideReason;
	UINT32           uiTimeSinceMercAIStart;
	INT8             fPanicFlags;
	INT16            sPanicTriggerGridno_UNUSED; // XXX HACK000B
	INT16            sHandGrid_UNUSED; // XXX HACK000B
	UINT8            ubSpottersCalledForBy;
	UINT8            ubTheChosenOne;
	UINT32           uiTimeOfLastInput;
	UINT32           uiTimeSinceDemoOn;
	UINT32           uiCountdownToRestart_UNUSED; // XXX HACK000B
	BOOLEAN          fGoingToEnterDemo_UNUSED; // XXX HACK000B
	BOOLEAN          fNOTDOLASTDEMO_UNUSED; // XXX HACK000B
	BOOLEAN          fMultiplayer_UNUSED; // XXX HACK000B
	BOOLEAN          fCivGroupHostile[ NUM_CIV_GROUPS ];
	UINT8            ubLastBattleSectorX;
	UINT8            ubLastBattleSectorY;
	BOOLEAN          fLastBattleWon;
	INT8             bOriginalSizeOfEnemyForce;
	INT8             bPanicTriggerIsAlarm_UNUSED; // XXX HACK000B
	BOOLEAN          fVirginSector;
	BOOLEAN          fEnemyInSector;
	BOOLEAN          fInterruptOccurred;
	INT8             bRealtimeSpeed;
	UINT8            ubEnemyIntention_UNUSED; // XXX HACK000B
	UINT8            ubEnemyIntendedRetreatDirection_UNUSED; // XXX HACK000B
	UINT8            ubEnemySightingOnTheirTurnEnemyID;
	UINT8            ubEnemySightingOnTheirTurnPlayerID_UNUSED; // XXX HACK000B
	BOOLEAN          fEnemySightingOnTheirTurn;
	BOOLEAN          fAutoBandageMode;
	UINT8            ubAttackBusyCount;
	INT8             bNumEnemiesFoughtInBattle_UNUSED; // XXX HACK000B
	UINT8            ubEngagedInConvFromActionMercID;
	UINT16           usTactialTurnLimitCounter;
	BOOLEAN          fInTopMessage;
	UINT8            ubTopMessageType;
	wchar_t          zTopMessageString[20];
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
	BOOLEAN          fSaidCreatureFlavourQuote_UNUSED; // XXX HACK000B
	BOOLEAN          fHaveSeenCreature_UNUSED; // XXX HACK000B
	BOOLEAN          fKilledEnemyOnAttack;
	UINT8            ubEnemyKilledOnAttack;
	INT8             bEnemyKilledOnAttackLevel;
	UINT16           ubEnemyKilledOnAttackLocation;
	BOOLEAN          fItemsSeenOnAttack;
	BOOLEAN          ubItemsSeenOnAttackSoldier;
	BOOLEAN          fBeenInCombatOnce_UNUSED; // XXX HACK000B
	BOOLEAN          fSaidCreatureSmellQuote_UNUSED; // XXX HACK000B
	UINT16           usItemsSeenOnAttackGridNo;
	BOOLEAN          fLockItemLocators;
	UINT8            ubLastQuoteSaid;
	UINT8            ubLastQuoteProfileNUm;
	BOOLEAN          fCantGetThrough;
	INT16            sCantGetThroughGridNo;
	INT16            sCantGetThroughSoldierGridNo;
	UINT8            ubCantGetThroughID;
	BOOLEAN          fDidGameJustStart;
	BOOLEAN          fStatChangeCheatOn_UNUSED; // XXX HACK000B
	UINT8            ubLastRequesterTargetID;
	BOOLEAN          fGoodToAllowCrows;
	UINT8            ubNumCrowsPossible;
	UINT32           uiTimeCounterForGiveItemSrc_UNUSED; // XXX HACK000B
	BOOLEAN          fUnLockUIAfterHiddenInterrupt;
	INT8             bNumFoughtInBattle[ MAXTEAMS ];
	UINT32           uiDecayBloodLastUpdate;
	UINT32           uiTimeSinceLastInTactical;
	BOOLEAN          fHasAGameBeenStarted;
	INT8             bConsNumTurnsWeHaventSeenButEnemyDoes;
	BOOLEAN          fSomeoneHit;
	UINT8            ubPaddingSmall_UNUSED; // XXX HACK000B
	UINT32           uiTimeSinceLastOpplistDecay;
	INT8             bMercArrivingQuoteBeingUsed;
	UINT8            ubEnemyKilledOnAttackKiller;
	BOOLEAN          fCountingDownForGuideDescription;
	INT8             bGuideDescriptionCountDown;
	UINT8            ubGuideDescriptionToUse;
	INT8             bGuideDescriptionSectorX;
	INT8             bGuideDescriptionSectorY;
	INT8             fEnemyFlags;
	BOOLEAN          fAutoBandagePending;
	BOOLEAN          fHasEnteredCombatModeSinceEntering;
	BOOLEAN          fDontAddNewCrows;
	UINT8            ubMorePadding_UNUSED; // XXX HACK000B
	UINT16           sCreatureTenseQuoteDelay;
	UINT32           uiCreatureTenseQuoteLastUpdate;
} TacticalStatusType;
CASSERT(sizeof(TacticalStatusType) == 360)


extern UINT8 gbPlayerNum;

extern UINT16 gusSelectedSoldier;

extern const char* const gzActionStr[];

// Soldier List used for all soldier overhead interaction
extern SOLDIERTYPE  Menptr[TOTAL_SOLDIERS];
extern SOLDIERTYPE* MercPtrs[TOTAL_SOLDIERS];

static inline SOLDIERTYPE* GetMan(UINT idx)
{
	Assert(idx < lengthof(Menptr));
	return &Menptr[idx];
}

typedef UINT8 SoldierID;

static inline SoldierID Soldier2ID(const SOLDIERTYPE* const s)
{
	return s != NULL ? s->ubID : NOBODY;
}

static inline SOLDIERTYPE* ID2Soldier(const SoldierID id)
{
	return id != NOBODY ? GetMan(id) : NULL;
}

// For temporary use
#define SOLDIER2ID(s) (Soldier2ID((s)))
#define ID2SOLDIER(i) (ID2Soldier((i)))

static inline SOLDIERTYPE* GetSelectedMan(void)
{
	return GetMan(gusSelectedSoldier);
}

// MERC SLOTS - A LIST OF ALL ACTIVE MERCS
extern SOLDIERTYPE* MercSlots[TOTAL_SOLDIERS];
extern UINT32       guiNumMercSlots;


extern TacticalStatusType gTacticalStatus;

#define BASE_FOR_ALL_IN_TEAM(type, iter, team)                                      \
	for (type*       iter        = &Menptr[gTacticalStatus.Team[(team)].bFirstID],    \
	         * const end__##iter = &Menptr[gTacticalStatus.Team[(team)].bLastID + 1]; \
	     iter != end__##iter;                                                         \
	     ++iter)                                                                      \
		if (!iter->bActive) continue; else
#define FOR_ALL_IN_TEAM( iter, team) BASE_FOR_ALL_IN_TEAM(      SOLDIERTYPE, iter, (team))
#define CFOR_ALL_IN_TEAM(iter, team) BASE_FOR_ALL_IN_TEAM(const SOLDIERTYPE, iter, (team))

#define BASE_FOR_ALL_SOLDIERS(type, iter)                  \
	for (type* iter = Menptr; iter != endof(Menptr); ++iter) \
		if (!iter->bActive) continue; else
#define FOR_ALL_SOLDIERS( iter) BASE_FOR_ALL_SOLDIERS(      SOLDIERTYPE, iter)
#define CFOR_ALL_SOLDIERS(iter) BASE_FOR_ALL_SOLDIERS(const SOLDIERTYPE, iter)

#define BASE_FOR_ALL_NON_PLANNING_SOLDIERS(type, iter)                 \
	for (type* iter = Menptr; iter != Menptr + MAX_NUM_SOLDIERS; ++iter) \
		if (!iter->bActive) continue; else
#define FOR_ALL_NON_PLANNING_SOLDIERS( iter) BASE_FOR_ALL_SOLDIERS(      SOLDIERTYPE, iter)
#define CFOR_ALL_NON_PLANNING_SOLDIERS(iter) BASE_FOR_ALL_SOLDIERS(const SOLDIERTYPE, iter)


BOOLEAN InitTacticalEngine(void);
void ShutdownTacticalEngine(void);


BOOLEAN InitOverhead(void);
BOOLEAN ShutdownOverhead(void);
SOLDIERTYPE* GetSoldier(UINT16 usSoldierIndex);

INT16 NewOKDestination(const SOLDIERTYPE* pCurrSoldier, INT16 sGridNo, BOOLEAN fPeopleToo, INT8 bLevel);

//Simple check to see if a (one-tiled) soldier can occupy a given location on the ground or roof.
extern BOOLEAN IsLocationSittable( INT32 iMapIndex, BOOLEAN fOnRoof );
extern BOOLEAN IsLocationSittableExcludingPeople( INT32 iMapIndex, BOOLEAN fOnRoof );
extern BOOLEAN FlatRoofAboveGridNo( INT32 iMapIndex );


BOOLEAN ExecuteOverhead(void);

void EndTurn(UINT8 ubNextTeam);
void StartPlayerTeamTurn( BOOLEAN fDoBattleSnd, BOOLEAN fEnteringCombatMode );

typedef enum SelSoldierFlags
{
	SELSOLDIER_ACKNOWLEDGE    = 1U << 0,
	SELSOLDIER_FORCE_RESELECT = 1U << 1,
	SELSOLDIER_FROM_UI        = 1U << 2
} SelSoldierFlags;

void SelectSoldier(SOLDIERTYPE* s, SelSoldierFlags flags);


void LocateGridNo( UINT16 sGridNo );
void LocateSoldier(SOLDIERTYPE* s, BOOLEAN fSetLocator);

void BeginTeamTurn( UINT8 ubTeam );
void SlideTo(INT16 sGridno, SOLDIERTYPE* tgt, UINT16 usReasonID, BOOLEAN fSetLocator);
void SlideToLocation( UINT16 usReasonID, INT16 sDestGridNo );

void RebuildAllSoldierShadeTables(void);
void HandlePlayerTeamMemberDeath( SOLDIERTYPE *pSoldier );
BOOLEAN SoldierOnVisibleWorldTile( SOLDIERTYPE *pSoldier );

SOLDIERTYPE* FindNextActiveAndAliveMerc(SOLDIERTYPE* s, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs);
SOLDIERTYPE* FindPrevActiveAndAliveMerc(SOLDIERTYPE* s, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs);

void HandleNPCTeamMemberDeath( SOLDIERTYPE *pSoldier );

BOOLEAN UIOKMoveDestination(const SOLDIERTYPE* pSoldier, UINT16 usMapPos);

INT16 FindAdjacentGridEx(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8* pubDirection, INT16* psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor);
INT16 FindNextToAdjacentGridEx( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 *pubDirection, INT16 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor );


void SelectNextAvailSoldier(const SOLDIERTYPE* s);
BOOLEAN TeamMemberNear(INT8 bTeam, INT16 sGridNo, INT32 iRange);


// FUNCTIONS FOR MANIPULATING MERC SLOTS - A LIST OF ALL ACTIVE MERCS
INT32 AddMercSlot(SOLDIERTYPE* pSoldier);
BOOLEAN RemoveMercSlot( SOLDIERTYPE *pSoldier  );

INT32   AddAwaySlot(SOLDIERTYPE* pSoldier);
BOOLEAN RemoveAwaySlot(SOLDIERTYPE* pSoldier);
INT32   MoveSoldierFromMercToAwaySlot(SOLDIERTYPE* pSoldier);
INT32   MoveSoldierFromAwayToMercSlot(SOLDIERTYPE* pSoldier);

void EnterCombatMode( UINT8 ubStartingTeam );
void ExitCombatMode( );

void HandleTeamServices( UINT8 ubTeamNum );
void HandlePlayerServices( SOLDIERTYPE *pTeamSoldier );

void SetEnemyPresence( );

void CycleThroughKnownEnemies( );

BOOLEAN CheckForEndOfCombatMode( BOOLEAN fIncrementTurnsNotSeen );

SOLDIERTYPE* FreeUpAttacker(SOLDIERTYPE* attacker);

BOOLEAN PlayerTeamFull( );

void SetActionToDoOnceMercsGetToLocation(UINT8 ubActionCode, INT8 bNumMercsWaiting);

void ResetAllMercSpeeds(void);

BOOLEAN HandleGotoNewGridNo( SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving, BOOLEAN fInitialMove, UINT16 usAnimState );

SOLDIERTYPE* ReduceAttackBusyCount(SOLDIERTYPE* attacker, BOOLEAN fCalledByAttacker);

void CommonEnterCombatModeCode( );

void CheckForPotentialAddToBattleIncrement( SOLDIERTYPE *pSoldier );

void CencelAllActionsForTimeCompression( void );

BOOLEAN CheckForEndOfBattle( BOOLEAN fAnEnemyRetreated );

void AddManToTeam( INT8 bTeam );

void RemoveManFromTeam( INT8 bTeam );

void RemoveSoldierFromTacticalSector( SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected );

void MakeCivHostile( SOLDIERTYPE *pSoldier, INT8 bNewSide );

#define REASON_NORMAL_ATTACK 1
#define REASON_EXPLOSION 2

BOOLEAN ProcessImplicationsOfPCAttack(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pTarget, INT8 bReason);

INT16 FindAdjacentPunchTarget(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pTargetSoldier, INT16* psAdjustedTargetGridNo, UINT8* pubDirection);

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

void SetSoldierNonNeutral( SOLDIERTYPE * pSoldier );
void SetSoldierNeutral( SOLDIERTYPE * pSoldier );

void         CaptureTimerCallback(void);
SOLDIERTYPE* FindNextActiveSquad(SOLDIERTYPE* pSoldier);

extern BOOLEAN gfSurrendered;
extern BOOLEAN gfKillingGuysForLosingBattle;
extern UINT8   gubWaitingForAllMercsToExitCode;

UINT8        NumCapableEnemyInSector(void);
SOLDIERTYPE* FreeUpAttackerGivenTarget(SOLDIERTYPE* target);
SOLDIERTYPE* ReduceAttackBusyGivenTarget(SOLDIERTYPE* target);

#ifdef JA2DEMO
void HandleEndDemoInCreatureLevel(void);
#endif

#endif
