#ifndef _ASSIGNMENTS_H
#define _ASSIGNMENTS_H

// header for assignment manipulation/updating for characters

#include "Types.h"
#include "Soldier Control.h"
#include "mousesystem.h"
#include "Strategic Movement.h"


// this distinguishes whether we're only looking for patients healable THIS HOUR (those that have been on their assignment
// long enough), or those that will be healable EVER (regardless of whether they're getting healed during this hour)
#define HEALABLE_EVER				0
#define HEALABLE_THIS_HOUR	1


// merc collapses from fatigue if max breath drops to this.  Can't go any lower!
#define BREATHMAX_ABSOLUTE_MINIMUM	10
#define BREATHMAX_GOTTA_STOP_MOVING	30
#define BREATHMAX_PRETTY_TIRED			50
#define BREATHMAX_CANCEL_COLLAPSE		60
#define BREATHMAX_CANCEL_TIRED			75
#define BREATHMAX_FULLY_RESTED			95


#define	VEHICLE_REPAIR_POINTS_DIVISOR		10


// Assignments Defines
enum{
 SQUAD_1 =0,
 SQUAD_2,
 SQUAD_3,
 SQUAD_4,
 SQUAD_5,
 SQUAD_6,
 SQUAD_7,
 SQUAD_8,
 SQUAD_9,
 SQUAD_10,
 SQUAD_11,
 SQUAD_12,
 SQUAD_13,
 SQUAD_14,
 SQUAD_15,
 SQUAD_16,
 SQUAD_17,
 SQUAD_18,
 SQUAD_19,
 SQUAD_20,
 ON_DUTY,
 DOCTOR,
 PATIENT,
 VEHICLE,
 IN_TRANSIT,
 REPAIR,
 TRAIN_SELF,
 TRAIN_TOWN,
 TRAIN_TEAMMATE,
 TRAIN_BY_OTHER,
 ASSIGNMENT_DEAD,
 ASSIGNMENT_UNCONCIOUS,			// unused
 ASSIGNMENT_POW,
 ASSIGNMENT_HOSPITAL,
 ASSIGNMENT_EMPTY,
};

#define NO_ASSIGNMENT		127 //used when no pSoldier->ubDesiredSquad

// Train stats defines (must match ATTRIB_MENU_ defines, and pAttributeMenuStrings )
enum{
	STRENGTH = 0,
  DEXTERITY,
	AGILITY,
  HEALTH,
	MARKSMANSHIP,
	MEDICAL,
	MECHANICAL,
	LEADERSHIP,
	EXPLOSIVE_ASSIGN,
	NUM_TRAINABLE_STATS
	// NOTE: Wisdom isn't trainable!
};


typedef struct TOWN_TRAINER_TYPE
{
	SOLDIERTYPE *pSoldier;
	INT16	sTrainingPts;

} TOWN_TRAINER_TYPE;


// can character do this assignment?
//BOOLEAN CanSoldierAssignment( SOLDIERTYPE *pSoldier, INT8 bAssignment );

// can this character be assigned as a doctor?
BOOLEAN CanCharacterDoctor( SOLDIERTYPE *pCharacter );

// can this character be assigned as a repairman?
BOOLEAN CanCharacterRepair( SOLDIERTYPE *pCharacter );

// can character be patient?
BOOLEAN CanCharacterPatient( SOLDIERTYPE *pCharacter );

// can character train militia?
BOOLEAN CanCharacterTrainMilitia( SOLDIERTYPE *pCharacter );

// can character train stat?..as train self or as trainer?
BOOLEAN CanCharacterTrainStat( SOLDIERTYPE *pSoldier, INT8 bStat, BOOLEAN fTrainSelf, BOOLEAN fTrainTeammate );

// is character capable of practising at all?
BOOLEAN CanCharacterPractise( SOLDIERTYPE *pCharacter );

// can this character train others?
BOOLEAN CanCharacterTrainTeammates( SOLDIERTYPE *pSoldier );

// put character on duty?
BOOLEAN CanCharacterOnDuty( SOLDIERTYPE *pCharacter );

// put character to sleep?
BOOLEAN CanCharacterSleep( SOLDIERTYPE *pCharacter, BOOLEAN fExplainWhyNot );

BOOLEAN CanCharacterBeAwakened( SOLDIERTYPE *pSoldier, BOOLEAN fExplainWhyNot );

// put character in vehicle?
BOOLEAN CanCharacterVehicle( SOLDIERTYPE *pCharacter );

#define CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT -6
#define CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING -5
#define CHARACTER_CANT_JOIN_SQUAD_MOVING -4
#define CHARACTER_CANT_JOIN_SQUAD_VEHICLE -3
#define CHARACTER_CANT_JOIN_SQUAD_TOO_FAR -2
#define CHARACTER_CANT_JOIN_SQUAD_FULL -1
#define CHARACTER_CANT_JOIN_SQUAD 0
#define CHARACTER_CAN_JOIN_SQUAD 1

// can character be added to squad
INT8 CanCharacterSquad( SOLDIERTYPE *pCharacter, INT8 bSquadValue );

// if merc could train militia here, do they have sufficient loyalty?
BOOLEAN DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia( SOLDIERTYPE *pSoldier );
BOOLEAN DoesTownHaveRatingToTrainMilitia( INT8 bTownId );

// is the character in transit?
BOOLEAN IsCharacterInTransit( SOLDIERTYPE *pCharacter );

// handler for assignments -- called once per hour via event
void UpdateAssignments();

// how many people in this secotr have this assignment?
UINT8 FindNumberInSectorWithAssignment( INT16 sX, INT16 sY, INT8 bAssignment );

void MakeSoldiersTacticalAnimationReflectAssignment( SOLDIERTYPE *pSoldier );

// build list of sectors with mercs
void BuildSectorsWithSoldiersList( void );

// init sectors with soldiers list
void InitSectorsWithSoldiersList( void );

// is there a soldier in this sector?..only use after BuildSectorsWithSoldiersList is called
BOOLEAN IsThereASoldierInThisSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

void CheckIfSoldierUnassigned( SOLDIERTYPE *pSoldier );

// figure out the assignment menu pop up box positions
void DetermineBoxPositions( void );

// set x,y position in tactical
void SetTacticalPopUpAssignmentBoxXY( void );

// get number of pts that are being used this strategic turn
INT16 GetTownTrainPtsForCharacter( SOLDIERTYPE *pTrainer, UINT16 *pusMaxPts );


// find number of healing pts
UINT16 CalculateHealingPointsForDoctor(SOLDIERTYPE *pSoldier, UINT16 *pusMaxPts, BOOLEAN fMakeSureKitIsInHand );

// find number of repair pts repairman has available
UINT8 CalculateRepairPointsForRepairman(SOLDIERTYPE *pSoldier, UINT16 *pusMaxPts, BOOLEAN fMakeSureKitIsInHand );


// get bonus tarining pts due to an instructor for this student
INT16 GetBonusTrainingPtsDueToInstructor( SOLDIERTYPE *pInstructor, SOLDIERTYPE *pStudent, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16 *pusMaxPts );

// get training pts for this soldier
INT16 GetSoldierTrainingPts( SOLDIERTYPE *pSoldier, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16 *pusMaxPts );

// pts for being a student for this soldier
INT16 GetSoldierStudentPts( SOLDIERTYPE *pSoldier, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16 *pusMaxPts );

// reset these soldiers
void ResetAssignmentsForAllSoldiersInSectorWhoAreTrainingTown( SOLDIERTYPE *pSoldier );

// Handle assignment done
void AssignmentDone( SOLDIERTYPE *pSoldier, BOOLEAN fSayQuote, BOOLEAN fMeToo );

extern INT32 ghAssignmentBox;
extern INT32 ghEpcBox;
extern INT32 ghSquadBox;
extern INT32 ghVehicleBox;
extern INT32 ghRepairBox;
extern INT32 ghTrainingBox;
extern INT32 ghAttributeBox;
extern INT32 ghRemoveMercAssignBox;
extern INT32 ghContractBox;
extern INT32 ghMoveBox;
//extern INT32 ghUpdateBox;


extern MOUSE_REGION    gAssignmentScreenMaskRegion;

extern MOUSE_REGION    gAssignmentMenuRegion[  ];
extern MOUSE_REGION    gTrainingMenuRegion[  ];
extern MOUSE_REGION    gAttributeMenuRegion[  ];
extern MOUSE_REGION    gSquadMenuRegion[  ];
extern MOUSE_REGION    gContractMenuRegion[  ];
extern MOUSE_REGION    gRemoveMercAssignRegion[  ];
extern MOUSE_REGION		 gVehicleMenuRegion[];

extern BOOLEAN fShownContractMenu;
extern BOOLEAN fShownAssignmentMenu;
extern BOOLEAN fShowRepairMenu;

extern BOOLEAN fFirstClickInAssignmentScreenMask;

extern void RestorePopUpBoxes( void );

extern BOOLEAN fGlowContractRegion;

extern BOOLEAN gfReEvaluateEveryonesNothingToDo;


// pop up menu mouse regions
void CreateDestroyMouseRegionsForAssignmentMenu( void );
void AssignmentMenuMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void AssignmentMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );

void CreateDestroyMouseRegionsForTrainingMenu( void );
void TrainingMenuMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void TrainingMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );

void CreateDestroyMouseRegionsForAttributeMenu( void );
void AttributeMenuMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void AttributesMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );

void CreateDestroyMouseRegionsForSquadMenu( BOOLEAN fPositionBox );
void SquadMenuMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void SquadMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );

// vehicle menu
void CreateDestroyMouseRegionForVehicleMenu( void );
void VehicleMenuMvtCallback(MOUSE_REGION * pRegion, INT32 iReason );
void VehicleMenuBtnCallback(MOUSE_REGION * pRegion, INT32 iReason );

// repair menu
void CreateDestroyMouseRegionForRepairMenu( void );
void RepairMenuMvtCallback(MOUSE_REGION * pRegion, INT32 iReason );
void RepairMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );

// contract menu
void CreateDestroyMouseRegionsForContractMenu( void );
void ContractMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );
void ContractMenuMvtCallback( MOUSE_REGION * pRegion, INT32 iReason );
void RebuildContractBoxForMerc( SOLDIERTYPE *pCharacter );

// remove merc from team menu callback
void RemoveMercMenuBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );
void RemoveMercMenuMvtCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void CreateDestroyMouseRegionsForRemoveMenu( void );

// misc assignment GUI functions
void HandleShadingOfLinesForAssignmentMenus( void );
BOOLEAN IsCharacterAliveAndConscious( SOLDIERTYPE *pCharacter );
void CreateDestroyScreenMaskForAssignmentAndContractMenus( void );


BOOLEAN CreateDestroyAssignmentPopUpBoxes( void );
void SetSoldierAssignment( SOLDIERTYPE *pSoldier, INT8 bAssignment, INT32 iParam1, INT32 iParam2, INT32 iParam3 );

// set merc asleep and awake under the new sleep system implemented June 29, 1998
// if give warning is false, the function can be used as an internal function
BOOLEAN SetMercAwake( SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning, BOOLEAN fForceHim );
BOOLEAN SetMercAsleep( SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning );
BOOLEAN PutMercInAsleepState( SOLDIERTYPE *pSoldier );
BOOLEAN PutMercInAwakeState( SOLDIERTYPE *pSoldier );

BOOLEAN AssignMercToAMovementGroup( SOLDIERTYPE *pSoldier );

// set what time this merc undertook this assignment
void SetTimeOfAssignmentChangeForMerc( SOLDIERTYPE *pSoldier );

// enough time on assignment for it to count?
BOOLEAN EnoughTimeOnAssignment( SOLDIERTYPE *pSoldier );

// check if any merc in group is too tired to keep moving
BOOLEAN AnyMercInGroupCantContinueMoving( GROUP *pGroup );

// handle selected group of mercs being put to sleep
BOOLEAN HandleSelectedMercsBeingPutAsleep( BOOLEAN fWakeUp, BOOLEAN fDisplayWarning );

// is any one on the team on this assignment?
BOOLEAN IsAnyOneOnPlayersTeamOnThisAssignment( INT8 bAssignment );

// rebuild assignments box
void RebuildAssignmentsBox( void );

void BandageBleedingDyingPatientsBeingTreated( );

void ReEvaluateEveryonesNothingToDo();

// set assignment for list of characters
void SetAssignmentForList( INT8 bAssignment, INT8 bParam );

// is this area maxed out on militia?
BOOLEAN IsMilitiaTrainableFromSoldiersSectorMaxed( SOLDIERTYPE *pSoldier );

// function where we actually set someone's assignment so we can trap certain situations
void ChangeSoldiersAssignment( SOLDIERTYPE *pSoldier, INT8 bAssignment );

void UnEscortEPC( SOLDIERTYPE *pSoldier );

SOLDIERTYPE *AnyDoctorWhoCanHealThisPatient( SOLDIERTYPE *pPatient, BOOLEAN fThisHour );


#endif
