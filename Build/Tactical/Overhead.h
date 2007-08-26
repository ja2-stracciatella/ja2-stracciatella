#ifndef __OVERHEAD_H
#define __OVERHEAD_H

#include "Soldier_Control.h"
#include "Overhead_Types.h"


#define  MAX_REALTIME_SPEED_VAL						10


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
	UINT8			bFirstID;
	UINT8			bLastID;
	COLORVAL  RadarColor;
	INT8			bSide;
	INT8			bMenInSector;
	UINT8			ubLastMercToRadio;
	INT8			bTeamActive;
	INT8			bAwareOfOpposition;
	INT8			bHuman;
} TacticalTeamType;


#define PANIC_BOMBS_HERE			0x01
#define PANIC_TRIGGERS_HERE		0x02

#define NUM_PANIC_TRIGGERS 3

#define ENEMY_OFFERED_SURRENDER 0x01

typedef struct
{
	UINT32							uiFlags;
	TacticalTeamType		Team[ MAXTEAMS ];
	UINT8								ubCurrentTeam;
	INT16								sSlideTarget;
	INT16								sSlideReason;
	UINT32							uiTimeSinceMercAIStart;
	INT8								fPanicFlags;
	INT16								sPanicTriggerGridno_UNUSED; // XXX HACK000B
	INT16								sHandGrid_UNUSED; // XXX HACK000B
	UINT8								ubSpottersCalledForBy;
	UINT8								ubTheChosenOne;
	UINT32							uiTimeOfLastInput;
	UINT32							uiTimeSinceDemoOn;
	UINT32							uiCountdownToRestart_UNUSED; // XXX HACK000B
	BOOLEAN							fGoingToEnterDemo_UNUSED; // XXX HACK000B
	BOOLEAN							fNOTDOLASTDEMO_UNUSED; // XXX HACK000B
	BOOLEAN							fMultiplayer_UNUSED; // XXX HACK000B
	BOOLEAN							fCivGroupHostile[ NUM_CIV_GROUPS ];
	UINT8								ubLastBattleSectorX;
	UINT8								ubLastBattleSectorY;
	BOOLEAN							fLastBattleWon;
	INT8								bOriginalSizeOfEnemyForce;
	INT8								bPanicTriggerIsAlarm_UNUSED; // XXX HACK000B
	BOOLEAN							fVirginSector;
	BOOLEAN							fEnemyInSector;
	BOOLEAN							fInterruptOccurred;
	INT8								bRealtimeSpeed;
	UINT8								ubEnemyIntention_UNUSED; // XXX HACK000B
	UINT8								ubEnemyIntendedRetreatDirection_UNUSED; // XXX HACK000B
	UINT8								ubEnemySightingOnTheirTurnEnemyID;
	UINT8								ubEnemySightingOnTheirTurnPlayerID_UNUSED; // XXX HACK000B
	BOOLEAN							fEnemySightingOnTheirTurn;
	BOOLEAN							fAutoBandageMode;
	UINT8								ubAttackBusyCount;
	INT8								bNumEnemiesFoughtInBattle_UNUSED; // XXX HACK000B
	UINT8								ubEngagedInConvFromActionMercID;
	UINT16							usTactialTurnLimitCounter;
	BOOLEAN							fInTopMessage;
	UINT8								ubTopMessageType;
	wchar_t							zTopMessageString[20];
	UINT16							usTactialTurnLimitMax;
	UINT32							uiTactialTurnLimitClock;
	BOOLEAN							fTactialTurnLimitStartedBeep;
	INT8								bBoxingState;
	INT8								bConsNumTurnsNotSeen;
	UINT8								ubArmyGuysKilled;

	INT16								sPanicTriggerGridNo[ NUM_PANIC_TRIGGERS ];
	INT8								bPanicTriggerIsAlarm[ NUM_PANIC_TRIGGERS ];
	UINT8								ubPanicTolerance[ NUM_PANIC_TRIGGERS ];
	BOOLEAN							fAtLeastOneGuyOnMultiSelect;
	BOOLEAN							fSaidCreatureFlavourQuote_UNUSED; // XXX HACK000B
	BOOLEAN							fHaveSeenCreature_UNUSED; // XXX HACK000B
	BOOLEAN							fKilledEnemyOnAttack;
	UINT8								ubEnemyKilledOnAttack;
	INT8								bEnemyKilledOnAttackLevel;
	UINT16							ubEnemyKilledOnAttackLocation;
	BOOLEAN							fItemsSeenOnAttack;
	BOOLEAN							ubItemsSeenOnAttackSoldier;
	BOOLEAN							fBeenInCombatOnce_UNUSED; // XXX HACK000B
	BOOLEAN							fSaidCreatureSmellQuote_UNUSED; // XXX HACK000B
	UINT16							usItemsSeenOnAttackGridNo;
	BOOLEAN							fLockItemLocators;
	UINT8								ubLastQuoteSaid;
	UINT8								ubLastQuoteProfileNUm;
	BOOLEAN							fCantGetThrough;
	INT16								sCantGetThroughGridNo;
	INT16								sCantGetThroughSoldierGridNo;
	UINT8								ubCantGetThroughID;
	BOOLEAN							fDidGameJustStart;
	BOOLEAN							fStatChangeCheatOn_UNUSED; // XXX HACK000B
	UINT8								ubLastRequesterTargetID;
	BOOLEAN							fGoodToAllowCrows;
	UINT8								ubNumCrowsPossible;
	UINT32							uiTimeCounterForGiveItemSrc_UNUSED; // XXX HACK000B
	BOOLEAN							fUnLockUIAfterHiddenInterrupt;
	INT8								bNumFoughtInBattle[ MAXTEAMS ];
	UINT32							uiDecayBloodLastUpdate;
	UINT32							uiTimeSinceLastInTactical;
	BOOLEAN							fHasAGameBeenStarted;
	INT8								bConsNumTurnsWeHaventSeenButEnemyDoes;
	BOOLEAN								fSomeoneHit;
	UINT8								ubPaddingSmall_UNUSED; // XXX HACK000B
	UINT32							uiTimeSinceLastOpplistDecay;
	INT8								bMercArrivingQuoteBeingUsed;
	UINT8								ubEnemyKilledOnAttackKiller;
	BOOLEAN							fCountingDownForGuideDescription;
	INT8								bGuideDescriptionCountDown;
	UINT8								ubGuideDescriptionToUse;
	INT8								bGuideDescriptionSectorX;
	INT8								bGuideDescriptionSectorY;
	INT8								fEnemyFlags;
  BOOLEAN             fAutoBandagePending;
  BOOLEAN             fHasEnteredCombatModeSinceEntering;
  BOOLEAN             fDontAddNewCrows;
  UINT8               ubMorePadding_UNUSED; // XXX HACK000B
  UINT16              sCreatureTenseQuoteDelay;
  UINT32              uiCreatureTenseQuoteLastUpdate;
} TacticalStatusType;
CASSERT(sizeof(TacticalStatusType) == 360)


UINT8		gbPlayerNum;

UINT16  gusSelectedSoldier;

extern CHAR8		gzActionStr[][ 30 ];

// Soldier List used for all soldier overhead interaction
SOLDIERTYPE	 Menptr[ TOTAL_SOLDIERS ];
SOLDIERTYPE	 *MercPtrs[ TOTAL_SOLDIERS ];

// MERC SLOTS - A LIST OF ALL ACTIVE MERCS
SOLDIERTYPE	 *MercSlots[ TOTAL_SOLDIERS ];
UINT32			 guiNumMercSlots;


TacticalStatusType	gTacticalStatus;



BOOLEAN InitTacticalEngine( );
void ShutdownTacticalEngine( );


BOOLEAN InitOverhead( );
BOOLEAN ShutdownOverhead( );
SOLDIERTYPE* GetSoldier(UINT16 usSoldierIndex);

INT16 NewOKDestination(const SOLDIERTYPE* pCurrSoldier, INT16 sGridNo, BOOLEAN fPeopleToo, INT8 bLevel);

//Simple check to see if a (one-tiled) soldier can occupy a given location on the ground or roof.
extern BOOLEAN IsLocationSittable( INT32 iMapIndex, BOOLEAN fOnRoof );
extern BOOLEAN IsLocationSittableExcludingPeople( INT32 iMapIndex, BOOLEAN fOnRoof );
extern BOOLEAN FlatRoofAboveGridNo( INT32 iMapIndex );


BOOLEAN ExecuteOverhead( );

void EndTurn(UINT8 ubNextTeam);
void StartPlayerTeamTurn( BOOLEAN fDoBattleSnd, BOOLEAN fEnteringCombatMode );

void SelectSoldier( UINT16 usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect  );


void LocateGridNo( UINT16 sGridNo );
void LocateSoldier( UINT16 usID, BOOLEAN fSetLocator);

void BeginTeamTurn( UINT8 ubTeam );
void SlideTo(INT16 sGridno, UINT16 usSoldierID , UINT16 usReasonID, BOOLEAN fSetLocator) ;
void SlideToLocation( UINT16 usReasonID, INT16 sDestGridNo );

void RebuildAllSoldierShadeTables( );
void HandlePlayerTeamMemberDeath( SOLDIERTYPE *pSoldier );
BOOLEAN SoldierOnVisibleWorldTile( SOLDIERTYPE *pSoldier );

UINT8 FindNextActiveAndAliveMerc(const SOLDIERTYPE* pSoldier, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs);
UINT8 FindPrevActiveAndAliveMerc( SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife, BOOLEAN fOnlyRegularMercs );

void HandleNPCTeamMemberDeath( SOLDIERTYPE *pSoldier );

BOOLEAN UIOKMoveDestination(const SOLDIERTYPE* pSoldier, UINT16 usMapPos);

INT16 FindAdjacentGridEx(SOLDIERTYPE* pSoldier, INT16 sGridNo, UINT8* pubDirection, INT16* psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor);
INT16 FindNextToAdjacentGridEx( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 *pubDirection, INT16 *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor );


void SelectNextAvailSoldier( SOLDIERTYPE *pSoldier );
BOOLEAN TeamMemberNear(INT8 bTeam, INT16 sGridNo, INT32 iRange);
BOOLEAN IsValidTargetMerc( UINT8 ubSoldierID );


// FUNCTIONS FOR MANIPULATING MERC SLOTS - A LIST OF ALL ACTIVE MERCS
INT32	AddMercSlot( SOLDIERTYPE *pSoldier );
BOOLEAN RemoveMercSlot( SOLDIERTYPE *pSoldier  );

INT32		AddAwaySlot( SOLDIERTYPE *pSoldier );
BOOLEAN RemoveAwaySlot( SOLDIERTYPE *pSoldier );
INT32		MoveSoldierFromMercToAwaySlot( SOLDIERTYPE * pSoldier );
INT32		MoveSoldierFromAwayToMercSlot( SOLDIERTYPE * pSoldier );

void EnterCombatMode( UINT8 ubStartingTeam );
void ExitCombatMode( );

void HandleTeamServices( UINT8 ubTeamNum );
void HandlePlayerServices( SOLDIERTYPE *pTeamSoldier );

void SetEnemyPresence( );

void CycleThroughKnownEnemies( );

BOOLEAN CheckForEndOfCombatMode( BOOLEAN fIncrementTurnsNotSeen );

SOLDIERTYPE * FreeUpAttacker( UINT8 ubID );

BOOLEAN PlayerTeamFull( );

void SetActionToDoOnceMercsGetToLocation(UINT8 ubActionCode, INT8 bNumMercsWaiting);

void ResetAllMercSpeeds( );

BOOLEAN HandleGotoNewGridNo( SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving, BOOLEAN fInitialMove, UINT16 usAnimState );

SOLDIERTYPE * ReduceAttackBusyCount( UINT8 ubID, BOOLEAN fCalledByAttacker );

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

BOOLEAN ProcessImplicationsOfPCAttack( SOLDIERTYPE * pSoldier, SOLDIERTYPE ** ppTarget, INT8 bReason );

INT16 FindAdjacentPunchTarget( SOLDIERTYPE * pSoldier, SOLDIERTYPE * pTargetSoldier, INT16 * psAdjustedTargetGridNo, UINT8 * pubDirection );

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



#endif
