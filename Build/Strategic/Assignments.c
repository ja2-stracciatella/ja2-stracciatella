#include "Assignments.h"
#include "Auto_Resolve.h"
#include "Font.h"
#include "Local.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "Soldier_Control.h"
#include "Item_Types.h"
#include "Strategic.h"
#include "Items.h"
#include "Overhead.h"
#include "Game_Clock.h"
#include "Message.h"
#include "Font_Control.h"
#include "Map_Screen_Interface.h"
#include "Soldier_Profile_Type.h"
#include "Soldier_Profile.h"
#include "Campaign.h"
#include "Queen_Command.h"
#include "StrategicMap.h"
#include "Strategic_Movement_Costs.h"
#include "Text.h"
#include "Dialogue_Control.h"
#include "NPC.h"
#include "Strategic_Town_Loyalty.h"
#include "Animation_Control.h"
#include "MapScreen.h"
#include "Squads.h"
#include "Map_Screen_Helicopter.h"
#include "PopUpBox.h"
#include "VObject.h"
#include "Vehicles.h"
#include "Strategic_Merc_Handler.h"
#include "Merc_Contract.h"
#include "Map_Screen_Interface_Map.h"
#include "Strategic_Movement.h"
#include "Laptop.h"
#include "Finances.h"
#include "LaptopSave.h"
#include "RenderWorld.h"
#include "Interface_Control.h"
#include "Interface.h"
#include "Soldier_Find.h"
#include "AI.h"
#include "Random.h"
#include "Line.h"
#include "Soldier_Add.h"
#include "GameSettings.h"
#include "Isometric_Utils.h"
#include "Soldier_Macros.h"
#include "Explosion_Control.h"
#include "SkillCheck.h"
#include "Quests.h"
#include "Town_Militia.h"
#include "Map_Screen_Interface_Border.h"
#include "Strategic_Pathing.h"
#include "Game_Event_Hook.h"
#include "Strategic_Event_Handler.h"
#include "Map_Information.h"
#include "Strategic_Status.h"
#include "History.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Interface_Dialogue.h"
#include "JAScreens.h"
#include "Debug.h"
#include "Button_System.h"
#include "ScreenIDs.h"
#include "VSurface.h"
#include "WCheck.h"


// various reason an assignment can be aborted before completion
enum{
	NO_MORE_MED_KITS = 40,
	INSUF_DOCTOR_SKILL,
	NO_MORE_TOOL_KITS,
	INSUF_REPAIR_SKILL,

	NUM_ASSIGN_ABORT_REASONS
};

enum{
	REPAIR_MENU_VEHICLE1 = 0,
	REPAIR_MENU_VEHICLE2,
	REPAIR_MENU_VEHICLE3,
//	REPAIR_MENU_SAM_SITE,
	REPAIR_MENU_ROBOT,
	REPAIR_MENU_ITEMS,
	REPAIR_MENU_CANCEL,
};


enum {
	REPAIR_HANDS_AND_ARMOR = 0,
	REPAIR_HEADGEAR,
	REPAIR_POCKETS,
	NUM_REPAIR_PASS_TYPES,
};

#define FINAL_REPAIR_PASS			REPAIR_POCKETS


typedef struct REPAIR_PASS_SLOTS_TYPE
{
	UINT8		ubChoices;						// how many valid choices there are in this pass
	INT8		bSlot[ 12 ];					// list of slots to be repaired in this pass
} REPAIR_PASS_SLOTS_TYPE;


REPAIR_PASS_SLOTS_TYPE gRepairPassSlotList[ NUM_REPAIR_PASS_TYPES ] =
{					// pass					# choices												slots repaired in this pass
	{ /* hands and armor */  5, { HANDPOS, SECONDHANDPOS, VESTPOS, HELMETPOS, LEGPOS, -1, -1, -1, -1, -1, -1, -1 } },
	{ /* headgear */         2, { HEAD1POS, HEAD2POS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ /* pockets */         12, { BIGPOCK1POS, BIGPOCK2POS, BIGPOCK3POS, BIGPOCK4POS, SMALLPOCK1POS, SMALLPOCK2POS, SMALLPOCK3POS, SMALLPOCK4POS, SMALLPOCK5POS, SMALLPOCK6POS, SMALLPOCK7POS, SMALLPOCK8POS } }
};


// PopUp Box Handles
PopUpBox* ghAssignmentBox;
PopUpBox* ghEpcBox;
PopUpBox* ghSquadBox;
PopUpBox* ghVehicleBox;
PopUpBox* ghRepairBox;
PopUpBox* ghTrainingBox;
PopUpBox* ghAttributeBox;
PopUpBox* ghRemoveMercAssignBox;
PopUpBox* ghContractBox;
PopUpBox* ghMoveBox;

// the x,y position of assignment pop up in tactical
static INT16 gsAssignmentBoxesX;
static INT16 gsAssignmentBoxesY;


// assignment menu mouse regions
static MOUSE_REGION gAssignmentMenuRegion[MAX_ASSIGN_STRING_COUNT];
static MOUSE_REGION gTrainingMenuRegion[MAX_TRAIN_STRING_COUNT];
static MOUSE_REGION gAttributeMenuRegion[MAX_ATTRIBUTE_STRING_COUNT];
static MOUSE_REGION gSquadMenuRegion[MAX_SQUAD_MENU_STRING_COUNT];
static MOUSE_REGION gContractMenuRegion[MAX_CONTRACT_MENU_STRING_COUNT];
static MOUSE_REGION gRemoveMercAssignRegion[MAX_REMOVE_MERC_COUNT];
static MOUSE_REGION gRepairMenuRegion[20];

// mouse region for vehicle menu
static MOUSE_REGION gVehicleMenuRegion[20];

static MOUSE_REGION gAssignmentScreenMaskRegion;

BOOLEAN fShownAssignmentMenu = FALSE;
BOOLEAN fShowVehicleMenu = FALSE;
BOOLEAN fShowRepairMenu = FALSE;
BOOLEAN fShownContractMenu = FALSE;

BOOLEAN fFirstClickInAssignmentScreenMask = FALSE;

// render pre battle interface?
extern BOOLEAN gfRenderPBInterface;
extern SOLDIERTYPE *pMilitiaTrainerSoldier;

// we are in fact training?..then who temmates, or self?
INT8 gbTrainingMode = -1;

extern INT32 giMapBorderButtons[];

BOOLEAN gfAddDisplayBoxToWaitingQueue = FALSE;

// redraw character list
extern BOOLEAN fDrawCharacterList;

SOLDIERTYPE *gpDismissSoldier = NULL;

BOOLEAN gfReEvaluateEveryonesNothingToDo = FALSE;


// the amount time must be on assignment before it can have any effect
#define MINUTES_FOR_ASSIGNMENT_TO_COUNT	45

// number we divide the total pts accumlated per day by for each assignment period
#define ASSIGNMENT_UNITS_PER_DAY 24

// base skill to deal with an emergency
#define BASE_MEDICAL_SKILL_TO_DEAL_WITH_EMERGENCY 20

// multiplier for skill needed for each point below OKLIFE
#define MULTIPLIER_FOR_DIFFERENCE_IN_LIFE_VALUE_FOR_EMERGENCY 4

// number of pts needed for each point below OKLIFE
#define POINT_COST_PER_HEALTH_BELOW_OKLIFE 2

// how many points of healing each hospital patients gains per hour in the hospital
#define HOSPITAL_HEALING_RATE		5				// a top merc doctor can heal about 4 pts/hour maximum, but that's spread among patients!

// increase to reduce repair pts, or vice versa
#define REPAIR_RATE_DIVISOR 2500
// increase to reduce doctoring pts, or vice versa
#define DOCTORING_RATE_DIVISOR 2400				// at 2400, the theoretical maximum is 150 full healing pts/day

// cost to unjam a weapon in repair pts
#define REPAIR_COST_PER_JAM 2

// divisor for rate of self-training
#define SELF_TRAINING_DIVISOR				1000
// the divisor for rate of training bonus due to instructors influence
#define INSTRUCTED_TRAINING_DIVISOR 3000

// this controls how fast town militia gets trained
#define TOWN_TRAINING_RATE	4

#define MAX_MILITIA_TRAINERS_PER_SECTOR 2

// militia training bonus for EACH level of teaching skill (percentage points)
#define TEACH_BONUS_TO_TRAIN 30
// militia training bonus for RPC (percentage points)
#define RPC_BONUS_TO_TRAIN   10

// the bonus to training in marksmanship in the Alma gun range sector
#define GUN_RANGE_TRAINING_BONUS	25

// breath bonus divider
#define BREATH_BONUS_DIVIDER 10

// the min rating that is need to teach a fellow teammate
#define MIN_RATING_TO_TEACH 25

// activity levels for natural healing ( the higher the number, the slower the natural recover rate
#define LOW_ACTIVITY_LEVEL      1
#define MEDIUM_ACTIVITY_LEVEL   4
#define HIGH_ACTIVITY_LEVEL			12

/* Assignment distance limits removed.  Sep/11/98.  ARM
#define MAX_DISTANCE_FOR_DOCTORING	5
#define MAX_DISTANCE_FOR_REPAIR			5
#define MAX_DISTANCE_FOR_TRAINING		5
*/

/*
// controls how easily SAM sites are repaired
// NOTE: A repairman must generate a least this many points / hour to be ABLE to repair a SAM site at all!
#define SAM_SITE_REPAIR_DIVISOR		10

// minimum condition a SAM site must be in to be fixable
#define MIN_CONDITION_TO_FIX_SAM 20
*/


// a list of which sectors have characters
BOOLEAN fSectorsWithSoldiers[ MAP_WORLD_X * MAP_WORLD_Y ][ 4 ];

// glow area for contract region?
BOOLEAN fGlowContractRegion = FALSE;


/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
BOOLEAN IsTheSAMSiteInSectorRepairable( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ );
BOOLEAN SoldierInSameSectorAsSAM( SOLDIERTYPE *pSoldier );
BOOLEAN CanSoldierRepairSAM( SOLDIERTYPE *pSoldier, INT8 bRepairPoints );
BOOLEAN IsSoldierCloseEnoughToSAMControlPanel( SOLDIERTYPE *pSoldier );
*/

#ifdef JA2BETAVERSION
void VerifyTownTrainingIsPaidFor( void );
#endif




void InitSectorsWithSoldiersList( void )
{
	// init list of sectors
	memset( &fSectorsWithSoldiers, 0, sizeof( fSectorsWithSoldiers ) );
}


void BuildSectorsWithSoldiersList( void )
{
	// fills array with pressence of player controlled characters
	CFOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		fSectorsWithSoldiers[s->sSectorX + s->sSectorY * MAP_WORLD_X][s->bSectorZ] = TRUE;
	}
}


void ChangeSoldiersAssignment( SOLDIERTYPE *pSoldier, INT8 bAssignment )
{
	// This is the most basic assignment-setting function.  It must be called before setting any subsidiary
	// values like fFixingRobot.  It will clear all subsidiary values so we don't leave the merc in a messed
	// up state!

	pSoldier->bAssignment = bAssignment;
/// don't kill iVehicleId, though, 'cause militia training tries to put guys back in their vehicles when it's done(!)

	pSoldier->fFixingSAMSite = FALSE;
	pSoldier->fFixingRobot = FALSE;
	pSoldier->bVehicleUnderRepairID = -1;

	if ( ( bAssignment == DOCTOR ) || ( bAssignment == PATIENT ) || ( bAssignment == ASSIGNMENT_HOSPITAL ) )
	{
		AddStrategicEvent( EVENT_BANDAGE_BLEEDING_MERCS, GetWorldTotalMin() + 1, 0 );
	}

	// update character info, and the team panel
	fCharacterInfoPanelDirty = TRUE;
	fTeamPanelDirty = TRUE;

	// merc may have come on/off duty, make sure map icons are updated
	fMapPanelDirty = TRUE;
}


static BOOLEAN IsSoldierInHelicopterInHostileSector(const SOLDIERTYPE* const s)
{
	return
		s->bAssignment       == VEHICLE       &&
		iHelicopterVehicleId != -1            &&
		iHelicopterVehicleId == s->iVehicleId &&
		NumEnemiesInSector(s->sSectorX, s->sSectorY) > 0;
}


typedef enum AssignmentConditions
{
	AC_CONSCIOUS                     = 1U << 0,
	AC_NOT_COMBAT                    = 1U << 1,
	AC_NOT_EPC                       = 1U << 2,
	AC_NOT_IN_HELI_IN_HOSTILE_SECTOR = 1U << 3,
	AC_NOT_MECHANICAL                = 1U << 4,
	AC_NOT_MOVING                    = 1U << 5,
	AC_NOT_UNDERGROUND               = 1U << 6
} AssignmentConditions;


static BOOLEAN AreAssignmentConditionsMet(const SOLDIERTYPE* const s, AssignmentConditions c)
{
	if (SectorIsImpassable(SECTOR(s->sSectorX, s->sSectorY)))                            return FALSE;
	if (c & AC_CONSCIOUS && s->bLife < OKLIFE)                                           return FALSE;
	if (c & AC_NOT_COMBAT && s->bInSector && gTacticalStatus.fEnemyInSector)             return FALSE;
	if (c & AC_NOT_EPC && s->ubWhatKindOfMercAmI == MERC_TYPE__EPC)                      return FALSE;
	if (c & AC_NOT_IN_HELI_IN_HOSTILE_SECTOR && IsSoldierInHelicopterInHostileSector(s)) return FALSE;
	if (c & AC_NOT_MECHANICAL && (s->uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(s)))  return FALSE;
	if (c & AC_NOT_MOVING && (IsCharacterInTransit(s) || s->fBetweenSectors))            return FALSE;
	if (c & AC_NOT_UNDERGROUND && s->bSectorZ != 0)                                      return FALSE;
	return TRUE;
}


static BOOLEAN CanCharacterDoctorButDoesntHaveMedKit(const SOLDIERTYPE* const s)
{
#ifdef JA2DEMO
	// this assignment is no go in the demo
	return FALSE;
#else
	return
		s->bMedical > 0 &&
		AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_COMBAT | AC_NOT_EPC | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MOVING);
#endif
}


// is character capable of 'playing' doctor?
// check that character is alive, conscious, has medical skill and equipment
static BOOLEAN CanCharacterDoctor(SOLDIERTYPE* pSoldier)
{
	INT8 bPocket = 0;

	if( CanCharacterDoctorButDoesntHaveMedKit( pSoldier ) == FALSE )
	{
		return( FALSE );
	}

	// find med kit
	for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
	{
		if (IsMedicalKitItem(&pSoldier->inv[bPocket]))
		{
			return TRUE;
		}
	}

	return FALSE;
}


static BOOLEAN CanCharacterRepairRobot(SOLDIERTYPE* pSoldier);
static BOOLEAN CanCharacterRepairVehicle(SOLDIERTYPE* pSoldier, INT32 iVehicleId);
static BOOLEAN DoesCharacterHaveAnyItemsToRepair(SOLDIERTYPE* pSoldier, INT8 bHighestPass);


static BOOLEAN IsAnythingAroundForSoldierToRepair(SOLDIERTYPE* pSoldier)
{
	// items?
	if ( DoesCharacterHaveAnyItemsToRepair( pSoldier, FINAL_REPAIR_PASS ) )
	{
		return( TRUE );
	}

	// robot?
	if ( CanCharacterRepairRobot( pSoldier ) )
	{
		return( TRUE );
	}

	// vehicles?
	if ( pSoldier->bSectorZ == 0 )
	{
		CFOR_ALL_VEHICLES(v)
		{
			// the helicopter, is NEVER repairable...
			if (VEHICLE2ID(v) != iHelicopterVehicleId &&
					IsThisVehicleAccessibleToSoldier(pSoldier, v) &&
					CanCharacterRepairVehicle(pSoldier, VEHICLE2ID(v)) == TRUE)
			{
				// there is a repairable vehicle here
				return TRUE;
			}
		}
	}

	return( FALSE );
}


static BOOLEAN HasCharacterFinishedRepairing(SOLDIERTYPE* pSoldier)
{
	BOOLEAN fCanStillRepair;

	// NOTE: This must detect situations where the vehicle/robot has left the sector, in which case we want the
	// guy to say "assignment done", so we return that he can no longer repair

	// check if we are repairing a vehicle
	if ( pSoldier->bVehicleUnderRepairID != -1 )
	{
		fCanStillRepair = CanCharacterRepairVehicle( pSoldier, pSoldier->bVehicleUnderRepairID );
	}
	// check if we are repairing a robot
	else if( pSoldier -> fFixingRobot )
	{
		fCanStillRepair = CanCharacterRepairRobot( pSoldier );
	}
	else	// repairing items
	{
		fCanStillRepair = DoesCharacterHaveAnyItemsToRepair( pSoldier, FINAL_REPAIR_PASS );
	}

	// if it's no longer damaged, we're finished!
	return( !fCanStillRepair );
}


static BOOLEAN CanCharacterRepairAnotherSoldiersStuff(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pOtherSoldier);
static INT8 FindRepairableItemOnOtherSoldier(const SOLDIERTYPE* pSoldier, UINT8 ubPassType);
static BOOLEAN IsItemRepairable(UINT16 usItem, INT8 bStatus);


static BOOLEAN DoesCharacterHaveAnyItemsToRepair(SOLDIERTYPE* pSoldier, INT8 bHighestPass)
{
	INT8	bPocket;
	UINT8	ubItemsInPocket, ubObjectInPocketCounter;
	OBJECTTYPE * pObj;
	UINT8 ubPassType;


	// check for jams
	for (bPocket = HELMETPOS; bPocket <= SMALLPOCK8POS; bPocket++)
	{
		ubItemsInPocket = pSoldier -> inv[ bPocket ].ubNumberOfObjects;
		// unjam any jammed weapons
		// run through pocket and repair
		for( ubObjectInPocketCounter = 0; ubObjectInPocketCounter < ubItemsInPocket; ubObjectInPocketCounter++ )
		{
			// jammed gun?
			if ( ( Item[ pSoldier -> inv[ bPocket ].usItem ].usItemClass == IC_GUN ) && ( pSoldier -> inv[ bPocket ].bGunAmmoStatus < 0 ) )
			{
				return( TRUE );
			}
		}
	}

	// now check for items to repair
	for( bPocket = HELMETPOS; bPocket <= SMALLPOCK8POS; bPocket++ )
	{
		pObj = &(pSoldier->inv[ bPocket ]);

		ubItemsInPocket = pObj->ubNumberOfObjects;

		// run through pocket
		for( ubObjectInPocketCounter = 0; ubObjectInPocketCounter < ubItemsInPocket; ubObjectInPocketCounter++ )
		{
			// if it's repairable and NEEDS repairing
			if ( IsItemRepairable( pObj->usItem, pObj->bStatus[ubObjectInPocketCounter] ) )
			{
				return( TRUE );
			}
		}

		// have to check for attachments...
		for (INT8 bLoop = 0; bLoop < MAX_ATTACHMENTS; ++bLoop)
		{
			if ( pObj->usAttachItem[ bLoop ] != NOTHING )
			{
				// if it's repairable and NEEDS repairing
				if ( IsItemRepairable( pObj->usAttachItem[ bLoop ], pObj->bAttachStatus[ bLoop ] ) )
				{
					return( TRUE );
				}
			}
		}
	}


	// if we wanna check for the items belonging to others in the sector
	if ( bHighestPass != - 1 )
	{
		// now look for items to repair on other mercs
		CFOR_ALL_IN_TEAM(pOtherSoldier, gbPlayerNum)
		{
			if ( CanCharacterRepairAnotherSoldiersStuff( pSoldier, pOtherSoldier ) )
			{
				// okay, seems like a candidate!  Check if he has anything that needs unjamming or repairs

				for ( bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++ )
				{
					// the object a weapon? and jammed?
					if ( ( Item[ pOtherSoldier->inv[ bPocket ].usItem ].usItemClass == IC_GUN ) && ( pOtherSoldier->inv[ bPocket ].bGunAmmoStatus < 0 ) )
					{
						return( TRUE );
					}
				}

				// repair everyone's hands and armor slots first, then headgear, and pockets last
				for ( ubPassType = REPAIR_HANDS_AND_ARMOR; ubPassType <= ( UINT8 ) bHighestPass; ubPassType++ )
				{
					bPocket = FindRepairableItemOnOtherSoldier( pOtherSoldier, ubPassType );
					if ( bPocket != NO_SLOT )
					{
						return( TRUE );
					}
				}
			}
		}
	}

	return( FALSE );
}


static BOOLEAN BasicCanCharacterRepair(const SOLDIERTYPE* const s)
{
	return
		s->bMechanical > 0 &&
		AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_COMBAT | AC_NOT_EPC | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MOVING);
}


static BOOLEAN CanCharacterRepairButDoesntHaveARepairkit(SOLDIERTYPE* pSoldier)
{
	if ( BasicCanCharacterRepair( pSoldier ) == FALSE )
	{
		return( FALSE );
	}

	// make sure he actually doesn't have a toolkit
	if ( FindObj( pSoldier, TOOLKIT ) != NO_SLOT )
	{
		return( FALSE );
	}

	return( TRUE );
}


// can character be assigned as repairman?
// check that character is alive, oklife, has repair skill, and equipment, etc.
static BOOLEAN CanCharacterRepair(SOLDIERTYPE* pSoldier)
{
	// this assignment is no go in the demo
	#ifdef JA2DEMO
		return FALSE;
	#endif

	if ( BasicCanCharacterRepair( pSoldier ) == FALSE )
	{
		return( FALSE );
	}

	// make sure he has a toolkit
	if ( FindObj( pSoldier, TOOLKIT ) == NO_SLOT )
	{
		return( FALSE );
	}

	// anything around to fix?
	if ( !IsAnythingAroundForSoldierToRepair( pSoldier ) )
	{
		return( FALSE );
	}

	// NOTE: This will not detect situations where character lacks the SKILL to repair the stuff that needs repairing...
	// So, in that situation, his assignment will NOT flash, but a message to that effect will be reported every hour.

	// all criteria fit, can repair
	return ( TRUE );
}


// can character be set to patient?
static BOOLEAN CanCharacterPatient(const SOLDIERTYPE* const s)
{
#ifdef JA2DEMO
	// this assignment is no go in the demo
	return FALSE;
#else
	return
		s->bLife > 0 &&
		s->bLife != s->bLifeMax &&
		s->bAssignment != ASSIGNMENT_POW &&
		AreAssignmentConditionsMet(s, AC_NOT_COMBAT | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MECHANICAL | AC_NOT_MOVING);
#endif
}


static BOOLEAN CanSectorContainMilita(const INT16 x, const INT16 y, const INT16 z)
{
	return
		(z == 0 && StrategicMap[CALCULATE_STRATEGIC_INDEX(x, y)].bNameId != BLANK_SECTOR) || // is there a town?
		IsThisSectorASAMSector(x, y, z);
}


// can this character EVER train militia?
static BOOLEAN BasicCanCharacterTrainMilitia(const SOLDIERTYPE* const s)
{
	// is the character capable of training a town?
	// they must be alive/conscious and in the sector with the town
#ifdef JA2DEMO
	// this assignment is no go in the demo
	return FALSE;
#else
	return
		s->bLeadership > 0 &&
		CanSectorContainMilita(s->sSectorX, s->sSectorY, s->bSectorZ) &&
		NumEnemiesInAnySector(s->sSectorX, s->sSectorY, s->bSectorZ) == 0 &&
		AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_COMBAT | AC_NOT_EPC | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MECHANICAL | AC_NOT_MOVING);
#endif
}


static INT8 CountMilitiaTrainersInSoldiersSector(const SOLDIERTYPE* s);


BOOLEAN CanCharacterTrainMilitia(const SOLDIERTYPE* const pSoldier)
{
	if( BasicCanCharacterTrainMilitia( pSoldier ) &&
			MilitiaTrainingAllowedInSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) &&
			DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia( pSoldier ) &&
			!IsAreaFullOfMilitia(pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ) &&
			( CountMilitiaTrainersInSoldiersSector( pSoldier ) < MAX_MILITIA_TRAINERS_PER_SECTOR ) )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static BOOLEAN DoesTownHaveRatingToTrainMilitia(INT8 bTownId)
{
	// min loyalty rating?
	if( ( gTownLoyalty[ bTownId ].ubRating < MIN_RATING_TO_TRAIN_TOWN ) )
	{
		// nope
		return( FALSE );
	}

	return( TRUE );
}


BOOLEAN DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(const SOLDIERTYPE* const pSoldier)
{
	INT8 bTownId = 0;
	BOOLEAN fSamSitePresent = FALSE;


	// underground training is not allowed (code doesn't support and it's a reasonable enough limitation)
	if( pSoldier -> bSectorZ != 0 )
	{
		return( FALSE );
	}

	bTownId = GetTownIdForSector( pSoldier->sSectorX, pSoldier->sSectorY );

	// is there a town really here
	if( bTownId == BLANK_SECTOR )
	{
		fSamSitePresent = IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ );

		// if there is a sam site here
		if( fSamSitePresent )
		{
			return( TRUE );
		}

		return( FALSE );
	}

	// does this town have sufficient loyalty to train militia
	if( DoesTownHaveRatingToTrainMilitia( bTownId ) == FALSE )
	{
		return( FALSE );
	}

	return( TRUE );
}


// only 2 trainers are allowed per sector, so this function counts the # in a guy's sector
static INT8 CountMilitiaTrainersInSoldiersSector(const SOLDIERTYPE* const pSoldier)
{
	INT8 bCount = 0;
	CFOR_ALL_IN_TEAM(s, gbPlayerNum)
	{
		if (s != pSoldier &&
				s->bLife >= OKLIFE &&
				s->sSectorX == pSoldier->sSectorX &&
				s->sSectorY == pSoldier->sSectorY &&
				s->bSectorZ == pSoldier->bSectorZ &&
				s->bAssignment == TRAIN_TOWN)
		{
			++bCount;
		}
	}
	return bCount;
}


static INT8 GetTrainingStatValue(const SOLDIERTYPE* const s, const INT8 stat)
{
	switch (stat)
	{
		case STRENGTH:         return s->bStrength;     break;
		case DEXTERITY:        return s->bDexterity;    break;
		case AGILITY:          return s->bAgility;      break;
		case HEALTH:           return s->bLifeMax;      break;
		case MARKSMANSHIP:     return s->bMarksmanship; break;
		case MEDICAL:          return s->bMedical;      break;
		case MECHANICAL:       return s->bMechanical;   break;
		case LEADERSHIP:       return s->bLeadership;   break;
		case EXPLOSIVE_ASSIGN: return s->bExplosive;    break;
		// NOTE: Wisdom can't be trained!

		default:
#ifdef JA2BETAVERSION
			ScreenMsg(FONT_ORANGE, MSG_BETAVERSION, L"ERROR - Unknown training stat %d", stat);
#endif
			return 0;
	}
}


// can character train stat?..as train self or as trainer?
static BOOLEAN CanCharacterTrainStat(const SOLDIERTYPE* const s, INT8 bStat, const BOOLEAN fTrainSelf, const BOOLEAN fTrainTeammate)
{
	// is the character capable of training this stat? either self or as trainer
#ifdef JA2DEMO
	// this assignment is no go in the demo
	return FALSE;
#else
	// underground training is not allowed (code doesn't support and it's a reasonable enough limitation)
	if (!AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_COMBAT | AC_NOT_EPC | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MOVING | AC_NOT_UNDERGROUND)) return FALSE;

	const INT8 stat_val = GetTrainingStatValue(s, bStat);
	return
		stat_val != 0 &&
		(!fTrainTeammate || stat_val >= MIN_RATING_TO_TEACH) &&
		(!fTrainSelf     || stat_val <  TRAINING_RATING_CAP);
#endif
}


// put character on duty?
static BOOLEAN CanCharacterOnDuty(const SOLDIERTYPE* const s)
{
	// can character commit themselves to on duty?
	return AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MOVING);
}


// is character capable of practising at all?
static BOOLEAN CanCharacterPractise(const SOLDIERTYPE* const s)
{
	// can character practise right now?
#ifdef JA2DEMO
	// this assignment is no go in the demo
	return FALSE;
#else
	return AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_COMBAT | AC_NOT_EPC | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MOVING | AC_NOT_UNDERGROUND);
#endif
}


// can this character train others?
static BOOLEAN CanCharacterTrainTeammates(SOLDIERTYPE* pSoldier)
{
	// can character train at all
	if( CanCharacterPractise( pSoldier ) == FALSE )
	{
		// nope
		return( FALSE );
	}

	// if alone in sector, can't enter the attributes submenu at all
	if ( PlayerMercsInSector( ( UINT8 ) pSoldier->sSectorX, ( UINT8 ) pSoldier->sSectorY, pSoldier->bSectorZ ) == 0 )
	{
		return( FALSE );
	}

	// ARM: we allow this even if there are no students assigned yet.  Flashing is warning enough.
	return( TRUE );
}


static BOOLEAN CanCharacterBeTrainedByOther(SOLDIERTYPE* pSoldier)
{
	// can character train at all
	if( CanCharacterPractise( pSoldier ) == FALSE )
	{
		return( FALSE );
	}

	// if alone in sector, can't enter the attributes submenu at all
	if ( PlayerMercsInSector( ( UINT8 ) pSoldier->sSectorX, ( UINT8 ) pSoldier->sSectorY, pSoldier->bSectorZ ) == 0 )
	{
		return( FALSE );
	}

	// ARM: we now allow this even if there are no trainers assigned yet.  Flashing is warning enough.
	return( TRUE );
}



// can character sleep right now?
static BOOLEAN CanCharacterSleep(SOLDIERTYPE* pSoldier, BOOLEAN fExplainWhyNot)
{
	CHAR16 sString[ 128 ];


	// this assignment is no go in the demo
	#ifdef JA2DEMO
		return(FALSE );
	#endif

	// dead or dying?
	if( pSoldier -> bLife < OKLIFE )
	{
		return( FALSE );
	}

	// vehicle or robot?
	if( ( pSoldier -> uiStatusFlags & SOLDIER_VEHICLE ) || AM_A_ROBOT( pSoldier ) )
	{
		return( FALSE );
	}

	// in transit?
	if( IsCharacterInTransit( pSoldier ) == TRUE )
	{
		return( FALSE );
	}

	// POW?
	if( pSoldier -> bAssignment == ASSIGNMENT_POW )
	{
		return( FALSE );
	}


	// traveling?
	if ( pSoldier->fBetweenSectors )
	{
		// if walking
		if ( pSoldier->bAssignment != VEHICLE )
		{
			// can't sleep while walking or driving a vehicle
			if( fExplainWhyNot )
			{
				// on the move, can't sleep
				swprintf( sString, lengthof(sString), zMarksMapScreenText[ 5 ], pSoldier->name );
				DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
			}

			return( FALSE );
		}
		else	// in a vehicle
		{
			// if this guy has to drive ('cause nobody else can)
			if ( SoldierMustDriveVehicle( pSoldier, pSoldier->iVehicleId, FALSE ) )
			{
				// can't sleep while walking or driving a vehicle
				if( fExplainWhyNot )
				{
					// is driving, can't sleep
					swprintf( sString, lengthof(sString), zMarksMapScreenText[ 7 ], pSoldier->name );
					DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
				}

				return( FALSE );
			}
		}
	}
	else	// in a sector
	{
		// if not above it all...
		if ( !SoldierAboardAirborneHeli( pSoldier ) )
		{
			// if he's in the loaded sector, and it's hostile or in combat
			if( pSoldier->bInSector && ( ( gTacticalStatus.uiFlags & INCOMBAT ) || gTacticalStatus.fEnemyInSector ) )
			{
				if( fExplainWhyNot )
				{
					DoScreenIndependantMessageBox( Message[ STR_SECTOR_NOT_CLEARED ], MSG_BOX_FLAG_OK, NULL );
				}

				return( FALSE );
			}

			// on surface, and enemies are in the sector
			if( ( pSoldier->bSectorZ == 0 ) && ( NumEnemiesInAnySector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) > 0 ) )
			{
				if( fExplainWhyNot )
				{
					DoScreenIndependantMessageBox( Message[ STR_SECTOR_NOT_CLEARED ], MSG_BOX_FLAG_OK, NULL );
				}

				return( FALSE );
			}
		}
	}


	// not tired?
	if( pSoldier->bBreathMax >= BREATHMAX_FULLY_RESTED )
	{
		if( fExplainWhyNot )
		{
			swprintf( sString, lengthof(sString), zMarksMapScreenText[ 4 ], pSoldier->name );
			DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
		}

		return( FALSE );
	}


	// can sleep
	return( TRUE );
}


static BOOLEAN CanCharacterBeAwakened(SOLDIERTYPE* pSoldier, BOOLEAN fExplainWhyNot)
{
	CHAR16 sString[ 128 ];

	// if dead tired
	if( ( pSoldier -> bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM ) && !pSoldier->fMercCollapsedFlag )
	{
		// should be collapsed, then!
		pSoldier->fMercCollapsedFlag = TRUE;
	}

	// merc collapsed due to being dead tired, you can't wake him up until he recovers substantially
	if ( pSoldier->fMercCollapsedFlag == TRUE )
	{
		if ( fExplainWhyNot )
		{
			swprintf( sString, lengthof(sString), zMarksMapScreenText[ 6 ], pSoldier->name );
			DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
		}

		return( FALSE );
	}

	// can be awakened
	return( TRUE );
}


// put character in vehicle?
static BOOLEAN CanCharacterVehicle(const SOLDIERTYPE* const s)
{
	// can character enter/leave vehicle?
#ifdef JA2DEMO
	// this assignment is no go in the demo
	return FALSE;
#else
	return
		fInMapMode && // strictly for visual reasons - we don't want them just vanishing if in tactical
		AnyAccessibleVehiclesInSoldiersSector(s) &&
		(!gTacticalStatus.fEnemyInSector || s->sSectorX != gWorldSectorX || s->sSectorY != gWorldSectorY || s->bSectorZ != gbWorldSectorZ) && // if we're in BATTLE in the current sector, disallow
		AreAssignmentConditionsMet(s, AC_CONSCIOUS | AC_NOT_COMBAT | AC_NOT_IN_HELI_IN_HOSTILE_SECTOR | AC_NOT_MOVING | AC_NOT_UNDERGROUND);
#endif
}


// can character be added to squad
static INT8 CanCharacterSquad(SOLDIERTYPE* pSoldier, INT8 bSquadValue)
{
	// can character join this squad?
	INT16 sX, sY, sZ;


	Assert( bSquadValue < ON_DUTY );

	if ( pSoldier->bAssignment == bSquadValue )
	{
		return( CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT );
	}

	// is the character alive and well?
	if( pSoldier -> bLife < OKLIFE )
	{
		// dead or unconscious...
		return ( CHARACTER_CANT_JOIN_SQUAD );
	}

	// in transit?
	if( IsCharacterInTransit( pSoldier ) == TRUE )
	{
		return ( CHARACTER_CANT_JOIN_SQUAD );
	}

	if ( pSoldier->bAssignment == ASSIGNMENT_POW )
	{
		// not allowed to be put on a squad
		return( CHARACTER_CANT_JOIN_SQUAD );
	}

	// see if the squad us at the same x,y,z
	SectorSquadIsIn( bSquadValue, &sX, &sY, &sZ );

	// check sector x y and z, if not same, cannot join squad
	if( ( sX != pSoldier -> sSectorX ) || ( sY != pSoldier -> sSectorY ) || ( sZ != pSoldier -> bSectorZ ) )
	{
		// is there anyone on this squad?
		if( NumberOfPeopleInSquad( bSquadValue ) > 0 )
		{
			return ( CHARACTER_CANT_JOIN_SQUAD_TOO_FAR );
		}
	}

	if( IsThisSquadOnTheMove( bSquadValue ) == TRUE )
	{
		// can't join while squad is moving
		return( CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING );
	}

	if ( DoesVehicleExistInSquad( bSquadValue ) )
	{
		// sorry can't change to this squad that way!
		return( CHARACTER_CANT_JOIN_SQUAD_VEHICLE );
	}

	if ( NumberOfPeopleInSquad( bSquadValue ) >= NUMBER_OF_SOLDIERS_PER_SQUAD )
	{
		return( CHARACTER_CANT_JOIN_SQUAD_FULL );
	}

	return ( CHARACTER_CAN_JOIN_SQUAD );
}


BOOLEAN IsCharacterInTransit(const SOLDIERTYPE* const pSoldier)
{
	// valid character?
	if( pSoldier == NULL )
	{
		return ( FALSE );
	}


	// check if character is currently in transit
	if( pSoldier -> bAssignment == IN_TRANSIT )
	{
		// yep
		return ( TRUE );
	}

	// no
	return ( FALSE );
}


static void CheckForAndHandleHospitalPatients(void);
static void HandleDoctorsInSector(INT16 sX, INT16 sY, INT8 bZ);
static void HandleNaturalHealing(void);
static void HandleRepairmenInSector(INT16 sX, INT16 sY, INT8 bZ);
static void HandleRestFatigueAndSleepStatus(void);
static void HandleTrainingInSector(INT16 sMapX, INT16 sMapY, INT8 bZ);
static void ReportTrainersTraineesWithoutPartners(void);
static void UpdatePatientsWhoAreDoneHealing(void);


void UpdateAssignments()
{
	INT8 sX,sY, bZ;

	// this assignment is no go in the demo
	#ifdef JA2DEMO
		return;
	#endif

	// init sectors with soldiers list
	InitSectorsWithSoldiersList( );

	// build list
	BuildSectorsWithSoldiersList(  );

	// handle natural healing
	HandleNaturalHealing( );

	// handle any patients in the hospital
	CheckForAndHandleHospitalPatients( );

	// see if any grunt or trainer just lost student/teacher
	ReportTrainersTraineesWithoutPartners( );

	// clear out the update list
	ClearSectorListForCompletedTrainingOfMilitia( );


	// rest resting mercs, fatigue active mercs,
	// check for mercs tired enough go to sleep, and wake up well-rested mercs
	HandleRestFatigueAndSleepStatus( );


#ifdef JA2BETAVERSION
	// put this BEFORE training gets handled to avoid detecting an error everytime a sector completes training
	VerifyTownTrainingIsPaidFor();
#endif

	// run through sectors and handle each type in sector
	for(sX = 0 ; sX < MAP_WORLD_X; sX++ )
	{
		for( sY =0; sY < MAP_WORLD_X; sY++ )
		{
			for( bZ = 0; bZ < 4; bZ++)
			{
				// is there anyone in this sector?
				if( fSectorsWithSoldiers[ sX + sY * MAP_WORLD_X ][ bZ ]  == TRUE )
				{
					// handle any doctors
					HandleDoctorsInSector( sX, sY, bZ );

					// handle any repairmen
					HandleRepairmenInSector( sX, sY, bZ );

					// handle any training
					HandleTrainingInSector( sX, sY, bZ );
				}
			}
		}
	}

	// check to see if anyone is done healing?
	UpdatePatientsWhoAreDoneHealing( );


	// check if we have anyone who just finished their assignment
	if( gfAddDisplayBoxToWaitingQueue )
	{
		AddDisplayBoxToWaitingQueue( );
		gfAddDisplayBoxToWaitingQueue = FALSE;
	}


	HandleContinueOfTownTraining( );

	// check if anyone is on an assignment where they have nothing to do
	ReEvaluateEveryonesNothingToDo();

	// update mapscreen
	fCharacterInfoPanelDirty = TRUE;
	fTeamPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;
}



#ifdef JA2BETAVERSION
void VerifyTownTrainingIsPaidFor( void )
{
	INT32 iCounter = 0;

	for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++)
	{
		const SOLDIERTYPE* const pSoldier = gCharactersList[iCounter].merc;
		if (pSoldier == NULL) continue;

		if( pSoldier->bActive && ( pSoldier->bAssignment == TRAIN_TOWN ) )
		{
			// make sure that sector is paid up!
			if( SectorInfo[ SECTOR( pSoldier -> sSectorX, pSoldier -> sSectorY ) ].fMilitiaTrainingPaid == FALSE )
			{
				// NOPE!  We've got a bug somewhere
				StopTimeCompression();

				// report the error
				DoScreenIndependantMessageBox( L"ERROR: Unpaid militia training. Describe *how* you're re-assigning mercs, how many/where/when! Send *prior* save!", MSG_BOX_FLAG_OK, NULL );

				// avoid repeating this
				break;
			}
		}
	}
}
#endif


static BOOLEAN CanSoldierBeHealedByDoctor(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pDoctor, BOOLEAN fIgnoreAssignment, BOOLEAN fThisHour, BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck);


static UINT8 GetNumberThatCanBeDoctored(SOLDIERTYPE* pDoctor, BOOLEAN fThisHour, BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck)
{
	// go through list of characters, find all who are patients/doctors healable by this doctor
	UINT8 ubNumberOfPeople = 0;
	CFOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (CanSoldierBeHealedByDoctor(s, pDoctor, FALSE, fThisHour, fSkipKitCheck, fSkipSkillCheck) == TRUE)
		{
			// increment number of doctorable patients/doctors
			++ubNumberOfPeople;
		}
	}
	return ubNumberOfPeople;
}


SOLDIERTYPE *AnyDoctorWhoCanHealThisPatient( SOLDIERTYPE *pPatient, BOOLEAN fThisHour )
{
	// go through list of characters, find all who are patients/doctors healable by this doctor
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		// doctor?
		if (s->bAssignment == DOCTOR &&
				CanSoldierBeHealedByDoctor(pPatient, s, FALSE, fThisHour, FALSE, FALSE) == TRUE)
		{
			// found one
			return s;
		}
	}

	// there aren't any doctors, or the ones there can't do the job
	return NULL;
}


static UINT16 TotalMedicalKitPoints(SOLDIERTYPE* pSoldier);
static BOOLEAN MakeSureMedKitIsInHand(SOLDIERTYPE* pSoldier);


UINT16 CalculateHealingPointsForDoctor(SOLDIERTYPE *pDoctor, UINT16 *pusMaxPts, BOOLEAN fMakeSureKitIsInHand )
{
	UINT16 usHealPts = 0;
	UINT16 usKitPts = 0;

	// make sure he has a medkit in his hand
	if( fMakeSureKitIsInHand )
	{
		if ( !MakeSureMedKitIsInHand( pDoctor ) )
		{
			return(0);
		}
	}

	// calculate effective doctoring rate (adjusted for drugs, alcohol, etc.)
	usHealPts = ( EffectiveMedical( pDoctor ) * (( EffectiveDexterity( pDoctor ) + EffectiveWisdom( pDoctor ) ) / 2) * (100 + ( 5 * EffectiveExpLevel( pDoctor) ) ) ) / DOCTORING_RATE_DIVISOR;

	// calculate normal doctoring rate - what it would be if his stats were "normal" (ignoring drugs, fatigue, equipment condition)
	// and equipment was not a hindrance
	*pusMaxPts = ( pDoctor -> bMedical * (( pDoctor -> bDexterity + pDoctor -> bWisdom ) / 2 ) * (100 + ( 5 * pDoctor->bExpLevel) ) ) / DOCTORING_RATE_DIVISOR;

	// adjust for fatigue
	ReducePointsForFatigue( pDoctor, &usHealPts );

	// count how much medical supplies we have
	usKitPts = 100 * TotalMedicalKitPoints( pDoctor );

	// if we don't have enough medical KIT points, reduce what we can heal
	if (usKitPts < usHealPts)
	{
		usHealPts = usKitPts;
	}

	// return healing pts value
	return( usHealPts );
}


static UINT16 ToolKitPoints(SOLDIERTYPE* pSoldier);
static void MakeSureToolKitIsInHand(SOLDIERTYPE* pSoldier);


UINT8 CalculateRepairPointsForRepairman(SOLDIERTYPE *pSoldier, UINT16 *pusMaxPts, BOOLEAN fMakeSureKitIsInHand )
{
	UINT16 usRepairPts;
	UINT16 usKitPts;
	UINT8  ubKitEffectiveness;

	// make sure toolkit in hand?
	if( fMakeSureKitIsInHand )
	{
		MakeSureToolKitIsInHand( pSoldier );
	}

	// can't repair at all without a toolkit
	if (pSoldier -> inv[HANDPOS].usItem != TOOLKIT)
	{
		*pusMaxPts = 0;
		return(0);
	}

	// calculate effective repair rate (adjusted for drugs, alcohol, etc.)
	usRepairPts = (EffectiveMechanical( pSoldier ) * EffectiveDexterity( pSoldier ) * (100 + ( 5 * EffectiveExpLevel( pSoldier) ) ) ) / ( REPAIR_RATE_DIVISOR * ASSIGNMENT_UNITS_PER_DAY );

	// calculate normal repair rate - what it would be if his stats were "normal" (ignoring drugs, fatigue, equipment condition)
	// and equipment was not a hindrance
	*pusMaxPts = ( pSoldier -> bMechanical * pSoldier -> bDexterity * (100 + ( 5 * pSoldier->bExpLevel) ) ) / ( REPAIR_RATE_DIVISOR * ASSIGNMENT_UNITS_PER_DAY );


	// adjust for fatigue
	ReducePointsForFatigue( pSoldier, &usRepairPts );


	// figure out what shape his "equipment" is in ("coming" in JA3: Viagra - improves the "shape" your "equipment" is in)
	usKitPts = ToolKitPoints( pSoldier );

	// if kit(s) in extremely bad shape
	if ( usKitPts < 25 )
	{
		ubKitEffectiveness = 50;
	}
	// if kit(s) in pretty bad shape
	else if ( usKitPts < 50 )
	{
		ubKitEffectiveness = 75;
	}
	else
	{
		ubKitEffectiveness = 100;
	}

	// adjust for equipment
	usRepairPts = (usRepairPts * ubKitEffectiveness) / 100;


	// return current repair pts
	return(( UINT8 )usRepairPts);
}


// how many points worth of tool kits does the character have?
static UINT16 ToolKitPoints(SOLDIERTYPE* pSoldier)
{
	UINT16 usKitpts=0;
	UINT8 ubPocket;

	// add up kit points
	for (ubPocket=HANDPOS; ubPocket <= SMALLPOCK8POS; ubPocket++)
	{
		if( pSoldier -> inv[ ubPocket ].usItem == TOOLKIT )
		{
			usKitpts += TotalPoints( &( pSoldier -> inv[ ubPocket ] ) );
		}
	}

	return( usKitpts );
}


// how many points worth of doctoring does the character have in his medical kits?
static UINT16 TotalMedicalKitPoints(SOLDIERTYPE* pSoldier)
{
	UINT8 ubPocket;
	UINT16 usKitpts=0;

	// add up kit points of all medkits
	for (ubPocket = HANDPOS; ubPocket <= SMALLPOCK8POS; ubPocket++)
	{
		if (IsMedicalKitItem(&pSoldier->inv[ubPocket]))
		{
			usKitpts += TotalPoints( &( pSoldier -> inv[ ubPocket ] ) );
		}
	}

	return( usKitpts );
}


static BOOLEAN EnoughTimeOnAssignment(const SOLDIERTYPE* pSoldier);
static void HealCharacters(SOLDIERTYPE* pDoctor, INT16 sX, INT16 sY, INT8 bZ);


// handle doctor in this sector
static void HandleDoctorsInSector(INT16 sX, INT16 sY, INT8 bZ)
{
	// will handle doctor/patient relationship in sector

	// go through list of characters, find all doctors in sector
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->sSectorX == sX &&
				s->sSectorY == sY &&
				s->bSectorZ == bZ &&
				s->bAssignment == DOCTOR &&
				!s->fMercAsleep)
		{
			MakeSureMedKitIsInHand(s);
			// character is in sector, check if can doctor, if so...heal people
			if (CanCharacterDoctor(s) && EnoughTimeOnAssignment(s))
			{
				HealCharacters(s, sX, sY, bZ);
			}
		}
	}

	// total healing pts for this sector, now heal people
}


// update characters who might done healing but are still patients
static void UpdatePatientsWhoAreDoneHealing(void)
{
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		// patient who doesn't need healing
		if (s->bAssignment == PATIENT && s->bLife == s->bLifeMax)
		{
			AssignmentDone(s, TRUE, TRUE);
		}
	}
}


static void AssignmentAborted(SOLDIERTYPE* pSoldier, UINT8 ubReason);
static UINT16 HealPatient(SOLDIERTYPE* pPatient, SOLDIERTYPE* pDoctor, UINT16 usHundredthsHealed);


// heal characters in this sector with this doctor
static void HealCharacters(SOLDIERTYPE* pDoctor, INT16 sX, INT16 sY, INT8 bZ)
{
	// heal all patients in this sector
	UINT16 usAvailableHealingPts = 0;
	UINT16 usRemainingHealingPts = 0;
	UINT16 usUsedHealingPts = 0;
	UINT16 usEvenHealingAmount = 0;
	UINT16 usMax =0;
	UINT8 ubTotalNumberOfPatients = 0;
	UINT16 usOldLeftOvers = 0;

	// now find number of healable mercs in sector that are wounded
	ubTotalNumberOfPatients = GetNumberThatCanBeDoctored( pDoctor, HEALABLE_THIS_HOUR, FALSE, FALSE );

	// if there is anybody who can be healed right now
	if( ubTotalNumberOfPatients > 0 )
	{
		// get available healing pts
		usAvailableHealingPts = CalculateHealingPointsForDoctor( pDoctor, &usMax, TRUE );
		usRemainingHealingPts = usAvailableHealingPts;

		// find how many healing points can be evenly distributed to each wounded, healable merc
		usEvenHealingAmount = usRemainingHealingPts / ubTotalNumberOfPatients;


		// heal each of the healable mercs by this equal amount
		FOR_ALL_IN_TEAM(s, OUR_TEAM)
		{
			if (CanSoldierBeHealedByDoctor(s, pDoctor, FALSE, HEALABLE_THIS_HOUR, FALSE, FALSE) == TRUE)
			{
				// can heal and is patient, heal them
				usRemainingHealingPts -= HealPatient(s, pDoctor, usEvenHealingAmount);
			}
		}


		// if we have any remaining pts
		if ( usRemainingHealingPts > 0)
		{
			// split those up based on need - lowest life patients get them first
			SOLDIERTYPE* pWorstHurtSoldier;
			do
			{
				// find the worst hurt patient
				pWorstHurtSoldier = NULL;
				FOR_ALL_IN_TEAM(s, OUR_TEAM)
				{
					if (CanSoldierBeHealedByDoctor(s, pDoctor, FALSE, HEALABLE_THIS_HOUR, FALSE, FALSE) == TRUE)
					{
						if (pWorstHurtSoldier == NULL)
						{
							pWorstHurtSoldier = s;
						}
						else
						{
							// check to see if this guy is hurt worse than anyone previous?
							if (s->bLife < pWorstHurtSoldier->bLife)
							{
								// he is now the worse hurt guy
								pWorstHurtSoldier = s;
							}
						}
					}
				}

				if( pWorstHurtSoldier != NULL )
				{
					// heal the worst hurt guy
					usOldLeftOvers = usRemainingHealingPts;
					usRemainingHealingPts -= HealPatient( pWorstHurtSoldier, pDoctor, usRemainingHealingPts );

					// couldn't expend any pts, leave
					if( usRemainingHealingPts == usOldLeftOvers )
					{
						usRemainingHealingPts = 0;
					}
				}
			} while( ( usRemainingHealingPts > 0 ) && ( pWorstHurtSoldier != NULL ) );
		}


		usUsedHealingPts = usAvailableHealingPts - usRemainingHealingPts;

		// increment skills based on healing pts used
		StatChange(pDoctor, MEDICALAMT,	(UINT16) (usUsedHealingPts / 100), FALSE);
		StatChange(pDoctor, DEXTAMT,		(UINT16) (usUsedHealingPts / 200), FALSE);
		StatChange(pDoctor, WISDOMAMT,	(UINT16) (usUsedHealingPts / 200), FALSE);
	}


	// if there's nobody else here who can EVER be helped by this doctor (regardless of whether they got healing this hour)
	if( GetNumberThatCanBeDoctored( pDoctor, HEALABLE_EVER, FALSE, FALSE ) == 0 )
	{
		// then this doctor has done all that he can do, but let's find out why and tell player the reason

		// try again, but skip the med kit check!
		if( GetNumberThatCanBeDoctored( pDoctor, HEALABLE_EVER, TRUE, FALSE ) > 0 )
		{
			// he could doctor somebody, but can't because he doesn't have a med kit!
			AssignmentAborted( pDoctor, NO_MORE_MED_KITS );
		}
		// try again, but skip the skill check!
		else if( GetNumberThatCanBeDoctored( pDoctor, HEALABLE_EVER, FALSE, TRUE ) > 0 )
		{
			// he could doctor somebody, but can't because he doesn't have enough skill!
			AssignmentAborted( pDoctor, INSUF_DOCTOR_SKILL );
		}
		else
		{
			// all patients should now be healed - truly DONE!
			AssignmentDone( pDoctor, TRUE, TRUE );
		}
	}
}


static UINT8 GetMinHealingSkillNeeded(const SOLDIERTYPE* pPatient);


// can this soldier be healed by this doctor?
static BOOLEAN CanSoldierBeHealedByDoctor(const SOLDIERTYPE* const pSoldier, const SOLDIERTYPE* const pDoctor, const BOOLEAN fIgnoreAssignment, const BOOLEAN fThisHour, const BOOLEAN fSkipKitCheck, const BOOLEAN fSkipSkillCheck)
{
	// must be an active guy
	if (pSoldier -> bActive == FALSE)
	{
		return(FALSE);
	}

	// must be a patient or a doctor
	if( ( pSoldier -> bAssignment != PATIENT ) && ( pSoldier -> bAssignment != DOCTOR ) && ( fIgnoreAssignment == FALSE ) )
	{
		return(FALSE);
	}

	// if dead or unhurt
	if ( (pSoldier -> bLife == 0) || (pSoldier -> bLife == pSoldier -> bLifeMax ) )
	{
		return(FALSE);
	}

	// if we care about how long it's been, and he hasn't been on a healable assignment long enough
	if( fThisHour && ( EnoughTimeOnAssignment( pSoldier ) == FALSE ) && ( fIgnoreAssignment == FALSE ) )
	{
		return( FALSE );
	}

	// must be in the same sector
	if( ( pSoldier -> sSectorX != pDoctor -> sSectorX ) || ( pSoldier -> sSectorY != pDoctor -> sSectorY ) || ( pSoldier -> bSectorZ != pDoctor -> bSectorZ ) )
	{
		return(FALSE);
	}

	// can't be between sectors (possible to get here if ignoring assignment)
	if ( pSoldier->fBetweenSectors )
	{
		return(FALSE);
	}

	// if doctor's skill is unsufficient to save this guy
	if ( !fSkipSkillCheck && ( pDoctor -> bMedical < GetMinHealingSkillNeeded( pSoldier ) ) )
	{
		return(FALSE);
	}

	if ( !fSkipKitCheck && ( FindObj( pDoctor, MEDICKIT ) == NO_SLOT ) )
	{
		// no medical kit to use!
		return( FALSE );
	}

	return( TRUE );
}


// returns minimum medical skill necessary to treat this patient
static UINT8 GetMinHealingSkillNeeded(const SOLDIERTYPE* const pPatient)
{
	// get the minimum skill to handle a character under OKLIFE

	if( pPatient -> bLife < OKLIFE )
	{
		// less than ok life, return skill needed
		return( BASE_MEDICAL_SKILL_TO_DEAL_WITH_EMERGENCY + ( MULTIPLIER_FOR_DIFFERENCE_IN_LIFE_VALUE_FOR_EMERGENCY * ( OKLIFE - pPatient -> bLife ) ) );
	}
	else
	{
		// only need some skill
		return ( 1 );
	}
}


// heal patient, given doctor and total healing pts available to doctor at this time
static UINT16 HealPatient(SOLDIERTYPE* pPatient, SOLDIERTYPE* pDoctor, UINT16 usHundredthsHealed)
{
	// heal patient and return the number of healing pts used
	UINT16 usHealingPtsLeft;
	UINT16 usTotalFullPtsUsed = 0;
	UINT16 usTotalHundredthsUsed = 0;
	INT8 bPointsToUse = 0;
	INT8 bPointsUsed = 0;
	INT8 bPointsHealed = 0;
	INT8 bPocket = 0;
//	INT8 bOldPatientLife = pPatient -> bLife;


	pPatient->sFractLife += usHundredthsHealed;
	usTotalHundredthsUsed = usHundredthsHealed;		// we'll subtract any unused amount later if we become fully healed...

	// convert fractions into full points
	usHealingPtsLeft = pPatient->sFractLife / 100;
	pPatient->sFractLife %= 100;

	// if we haven't accumulated any full points yet
	if (usHealingPtsLeft == 0)
	{
		return( usTotalHundredthsUsed );
	}

	// if below ok life, heal these first at double point cost
	if( pPatient -> bLife < OKLIFE )
	{
		// get points needed to heal him to OKLIFE
		bPointsToUse = POINT_COST_PER_HEALTH_BELOW_OKLIFE * ( OKLIFE - pPatient -> bLife );

		// if he needs more than we have, reduce to that
		if( bPointsToUse > usHealingPtsLeft )
		{
			bPointsToUse = usHealingPtsLeft;
		}

		// go through doctor's pockets and heal, starting at with his in-hand item
		for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
		{
			if (IsMedicalKitItem(&pDoctor->inv[bPocket]))
			{
				// ok, we have med kit in this pocket, use it
				bPointsUsed = UseKitPoints(&pDoctor->inv[bPocket], bPointsToUse, pDoctor);
				bPointsHealed = bPointsUsed;

				bPointsToUse -= bPointsHealed;
				usHealingPtsLeft -= bPointsHealed;
				usTotalFullPtsUsed += bPointsHealed;

				// heal person the amount / POINT_COST_PER_HEALTH_BELOW_OKLIFE
				pPatient -> bLife += (bPointsHealed / POINT_COST_PER_HEALTH_BELOW_OKLIFE);

				// if we're done all we're supposed to, or the guy's at OKLIFE, bail
				if ( ( bPointsToUse <= 0 ) || ( pPatient -> bLife >= OKLIFE ) )
				{
					break;
				}
			}
		}
	}

	// critical conditions handled, now apply normal healing

	if (pPatient -> bLife < pPatient -> bLifeMax)
	{
		bPointsToUse = ( pPatient -> bLifeMax - pPatient -> bLife );

		// if guy is hurt more than points we have...heal only what we have
		if( bPointsToUse > usHealingPtsLeft )
		{
			bPointsToUse = ( INT8 )usHealingPtsLeft;
		}

		// go through doctor's pockets and heal, starting at with his in-hand item
		// the healing pts are based on what type of medkit is in his hand, so we HAVE to start there first!
		for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
		{
			if (IsMedicalKitItem(&pDoctor->inv[bPocket]))
			{
				// ok, we have med kit in this pocket, use it  (use only half if it's worth double)
				bPointsUsed = UseKitPoints(&pDoctor->inv[bPocket], bPointsToUse, pDoctor);
				bPointsHealed = bPointsUsed;

				bPointsToUse -= bPointsHealed;
				usHealingPtsLeft -= bPointsHealed;
				usTotalFullPtsUsed += bPointsHealed;

				pPatient -> bLife += bPointsHealed;

				// if we're done all we're supposed to, or the guy's fully healed, bail
				if ( ( bPointsToUse <= 0 ) || ( pPatient -> bLife == pPatient -> bLifeMax ) )
				{
					break;
				}
			}
		}
	}


	// if this patient is fully healed
	if( pPatient->bLife == pPatient->bLifeMax )
	{
		// don't count unused full healing points as being used
		usTotalHundredthsUsed -= (100 * usHealingPtsLeft);

		// wipe out fractions of extra life, and DON'T count them as used
		usTotalHundredthsUsed -= pPatient->sFractLife;
		pPatient->sFractLife = 0;

/* ARM Removed.  This is duplicating the check in UpdatePatientsWhoAreDoneHealing(), guy would show up twice!
		// if it isn't the doctor himself)
		if( ( pPatient != pDoctor )
		{
			AssignmentDone( pPatient, TRUE, TRUE );
		}
*/
	}

	return ( usTotalHundredthsUsed );
}


static void HealHospitalPatient(SOLDIERTYPE* pPatient, UINT16 usHealingPtsLeft);


static void CheckForAndHandleHospitalPatients(void)
{
	if( fSectorsWithSoldiers[ HOSPITAL_SECTOR_X + HOSPITAL_SECTOR_Y * MAP_WORLD_X ][ 0 ] == FALSE )
	{
		// nobody in the hospital sector... leave
		return;
	}

	// go through list of characters, find all who are on this assignment
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == ASSIGNMENT_HOSPITAL &&
				s->sSectorX == HOSPITAL_SECTOR_X &&
				s->sSectorY == HOSPITAL_SECTOR_Y &&
				s->bSectorZ == 0)
		{
			// heal this character
			HealHospitalPatient(s, HOSPITAL_HEALING_RATE);
		}
	}
}


static void HealHospitalPatient(SOLDIERTYPE* pPatient, UINT16 usHealingPtsLeft)
{
	INT8 bPointsToUse;

	if (usHealingPtsLeft <= 0)
	{
		return;
	}

/*  Stopping hospital patients' bleeding must be handled immediately, not during a regular hourly check
	// stop all bleeding of patient..for 1 pt.
	if (pPatient -> bBleeding > 0)
	{
		usHealingPtsLeft--;
		pPatient -> bBleeding = 0;
	}
*/

	// if below ok life, heal these first at double cost
	if( pPatient -> bLife < OKLIFE )
	{
		// get points needed to heal him to OKLIFE
		bPointsToUse = POINT_COST_PER_HEALTH_BELOW_OKLIFE * ( OKLIFE - pPatient -> bLife );

		// if he needs more than we have, reduce to that
		if( bPointsToUse > usHealingPtsLeft )
		{
			bPointsToUse = usHealingPtsLeft;
		}

		usHealingPtsLeft -= bPointsToUse;

		// heal person the amount / POINT_COST_PER_HEALTH_BELOW_OKLIFE
		pPatient -> bLife += ( bPointsToUse / POINT_COST_PER_HEALTH_BELOW_OKLIFE );
	}

	// critical condition handled, now solve normal healing

	if ( pPatient -> bLife < pPatient -> bLifeMax )
	{
		bPointsToUse = ( pPatient -> bLifeMax - pPatient -> bLife );

		// if guy is hurt more than points we have...heal only what we have
		if( bPointsToUse > usHealingPtsLeft )
		{
			bPointsToUse = ( INT8 )usHealingPtsLeft;
		}

		usHealingPtsLeft -= bPointsToUse;

		// heal person the amount
		pPatient -> bLife += bPointsToUse;
	}

	// if this patient is fully healed
	if ( pPatient -> bLife == pPatient -> bLifeMax )
	{
		AssignmentDone( pPatient, TRUE, TRUE );
	}
}


static void HandleRepairBySoldier(SOLDIERTYPE* pSoldier);


// handle any repair man in sector
static void HandleRepairmenInSector(INT16 sX, INT16 sY, INT8 bZ)
{
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->sSectorX == sX &&
				s->sSectorY == sY &&
				s->bSectorZ == bZ &&
				s->bAssignment == REPAIR &&
				!s->fMercAsleep)
		{
			MakeSureToolKitIsInHand(s);
			// character is in sector, check if can repair
			if (CanCharacterRepair(s) && EnoughTimeOnAssignment(s))
			{
				HandleRepairBySoldier(s);
			}
		}
	}
}


/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
INT8 HandleRepairOfSAMSite( SOLDIERTYPE *pSoldier, INT8 bPointsAvailable, BOOLEAN * pfNothingLeftToRepair )
{
	INT8 bPtsUsed = 0;
	INT16 sStrategicSector = 0;

	if( IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) == FALSE )
	{
		return( bPtsUsed );
	}
	else if( ( pSoldier -> sSectorX == gWorldSectorX ) && ( pSoldier -> bSectorZ == gbWorldSectorZ )&&( pSoldier -> sSectorY == gWorldSectorY ) )
	{
		if( CanSoldierRepairSAM( pSoldier, bPointsAvailable ) == FALSE )
		{
			return( bPtsUsed );
		}
	}

	// repair the SAM

	sStrategicSector = CALCULATE_STRATEGIC_INDEX( pSoldier->sSectorX, pSoldier->sSectorY );

	// do we have more than enough?
	if( 100 - StrategicMap[ sStrategicSector ].bSAMCondition >= bPointsAvailable / SAM_SITE_REPAIR_DIVISOR )
	{
		// no, use up all we have
		StrategicMap[ sStrategicSector ].bSAMCondition += bPointsAvailable / SAM_SITE_REPAIR_DIVISOR;
		bPtsUsed = bPointsAvailable - ( bPointsAvailable % SAM_SITE_REPAIR_DIVISOR );

		// SAM site may have been put back into working order...
		UpdateAirspaceControl( );
	}
	else
	{
		// yep
		bPtsUsed = SAM_SITE_REPAIR_DIVISOR * ( 100 - StrategicMap[ sStrategicSector ].bSAMCondition );
		StrategicMap[ sStrategicSector ].bSAMCondition = 100;

//ARM: NOTE THAT IF THIS CODE IS EVER RE-ACTIVATED, THE SAM GRAPHICS SHOULD CHANGE NOT WHEN THE SAM SITE RETURNS TO
// FULL STRENGTH (condition 100), but as soon as it reaches MIN_CONDITION_TO_FIX_SAM!!!

		// Bring Hit points back up to full, adjust graphic to full graphic.....
		UpdateSAMDoneRepair( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ );
	}

	if ( StrategicMap[ sStrategicSector ].bSAMCondition == 100 )
	{
		*pfNothingLeftToRepair = TRUE;
	}
	else
	{
		*pfNothingLeftToRepair = FALSE;
	}
	return( bPtsUsed );
}
*/


// does another merc have a repairable item on them?
static INT8 FindRepairableItemOnOtherSoldier(const SOLDIERTYPE* const pSoldier, const UINT8 ubPassType)
{
	INT8 bLoop, bLoop2;
	REPAIR_PASS_SLOTS_TYPE *pPassList;
	INT8 bSlotToCheck;

	Assert( ubPassType < NUM_REPAIR_PASS_TYPES );

	pPassList = &( gRepairPassSlotList[ ubPassType ] );

	for ( bLoop = 0; bLoop < pPassList->ubChoices; bLoop++ )
	{
		bSlotToCheck = pPassList->bSlot[ bLoop ];
		Assert( bSlotToCheck != -1 );

		const OBJECTTYPE* const pObj = &pSoldier->inv[bSlotToCheck];
		for ( bLoop2 = 0; bLoop2 < pSoldier->inv[ bSlotToCheck ].ubNumberOfObjects; bLoop2++ )
		{
			if ( IsItemRepairable( pObj->usItem, pObj->bStatus[bLoop2] ) )
			{
				return( bSlotToCheck );
			}
		}

		// have to check for attachments...
		for ( bLoop2 = 0; bLoop2 < MAX_ATTACHMENTS; bLoop2++ )
		{
			if ( pObj->usAttachItem[ bLoop2 ] != NOTHING )
			{
				if ( IsItemRepairable( pObj->usAttachItem[ bLoop2 ], pObj->bAttachStatus[ bLoop2 ] ) )
				{
					return( bSlotToCheck );
				}
			}
		}
	}

	return( NO_SLOT );
}


static void DoActualRepair(SOLDIERTYPE* pSoldier, UINT16 usItem, INT8* pbStatus, UINT8* pubRepairPtsLeft)
{
	INT16		sRepairCostAdj;
	UINT16	usDamagePts, usPtsFixed;

	// get item's repair ease, for each + point is 10% easier, each - point is 10% harder to repair
	sRepairCostAdj = 100 - ( 10 * Item[ usItem ].bRepairEase );

	// make sure it ain't somehow gone too low!
	if (sRepairCostAdj < 10)
	{
		sRepairCostAdj = 10;
	}

	// repairs on electronic items take twice as long if the guy doesn't have the skill
	if ( ( Item[ usItem ].fFlags & ITEM_ELECTRONIC ) && ( !( HAS_SKILL_TRAIT( pSoldier, ELECTRONICS ) ) ) )
	{
		sRepairCostAdj *= 2;
	}

	// how many points of damage is the item down by?
	usDamagePts = 100 - *pbStatus;

	// adjust that by the repair cost adjustment percentage
	usDamagePts = (usDamagePts * sRepairCostAdj) / 100;

	// do we have enough pts to fully repair the item?
	if ( *pubRepairPtsLeft >= usDamagePts )
	{
		// fix it to 100%
		*pbStatus = 100;
		*pubRepairPtsLeft -= usDamagePts;
	}
	else	// not enough, partial fix only, if any at all
	{
		// fix what we can - add pts left adjusted by the repair cost
		usPtsFixed = ( *pubRepairPtsLeft * 100 ) / sRepairCostAdj;

		// if we have enough to actually fix anything
		// NOTE: a really crappy repairman with only 1 pt/hr CAN'T repair electronics or difficult items!
		if (usPtsFixed > 0)
		{
			*pbStatus += usPtsFixed;

			// make sure we don't somehow end up over 100
			if ( *pbStatus > 100 )
			{
				*pbStatus = 100;
			}
		}

		*pubRepairPtsLeft = 0;
	}
}


static BOOLEAN RepairObject(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pOwner, OBJECTTYPE* pObj, UINT8* pubRepairPtsLeft)
{
	UINT8	ubLoop, ubItemsInPocket;
	BOOLEAN fSomethingWasRepaired = FALSE;


	ubItemsInPocket = pObj->ubNumberOfObjects;

	for ( ubLoop = 0; ubLoop < ubItemsInPocket; ubLoop++ )
	{
		// if it's repairable and NEEDS repairing
		if ( IsItemRepairable( pObj->usItem, pObj->bStatus[ubLoop] ) )
		{
			// repairable, try to repair it

			DoActualRepair( pSoldier, pObj->usItem, &(pObj->bStatus[ ubLoop ]), pubRepairPtsLeft );

			fSomethingWasRepaired = TRUE;

			if ( pObj->bStatus[ ubLoop ] == 100 )
			{
				// report it as fixed
				if ( pSoldier == pOwner )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[ STR_REPAIRED ], pSoldier->name, ItemNames[ pObj->usItem ] );
				}
				else
				{
					// NOTE: may need to be changed for localized versions
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 35 ], pSoldier->name, pOwner->name, ItemNames[ pObj->usItem ] );
				}
			}

			if ( *pubRepairPtsLeft == 0 )
			{
				// we're out of points!
				break;
			}
		}
	}

	// now check for attachments
	for ( ubLoop = 0; ubLoop < MAX_ATTACHMENTS; ubLoop++ )
	{
		if ( pObj->usAttachItem[ ubLoop ] != NOTHING )
		{
			if ( IsItemRepairable( pObj->usAttachItem[ ubLoop ], pObj->bAttachStatus[ ubLoop ] ) )
			{
				// repairable, try to repair it

				DoActualRepair( pSoldier, pObj->usAttachItem[ ubLoop ], &(pObj->bAttachStatus[ ubLoop ]), pubRepairPtsLeft );

				fSomethingWasRepaired = TRUE;

				if ( pObj->bAttachStatus[ ubLoop ] == 100 )
				{
					// report it as fixed
					if ( pSoldier == pOwner )
					{
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[ STR_REPAIRED ], pSoldier->name, ItemNames[ pObj->usAttachItem[ ubLoop ] ] );
					}
					else
					{
						// NOTE: may need to be changed for localized versions
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 35 ], pSoldier->name, pOwner->name, ItemNames[ pObj->usAttachItem[ ubLoop ] ] );
					}
				}

				if ( *pubRepairPtsLeft == 0 )
				{
					// we're out of points!
					break;
				}
			}
		}
	}

	return( fSomethingWasRepaired );
}


static UINT8 HandleRepairOfRobotBySoldier(SOLDIERTYPE* pSoldier, UINT8 ubRepairPts, BOOLEAN* pfNothingLeftToRepair);
static void RepairItemsOnOthers(SOLDIERTYPE* pSoldier, UINT8* pubRepairPtsLeft);
static BOOLEAN UnjamGunsOnSoldier(SOLDIERTYPE* pOwnerSoldier, SOLDIERTYPE* pRepairSoldier, UINT8* pubRepairPtsLeft);


// repair stuff
static void HandleRepairBySoldier(SOLDIERTYPE* pSoldier)
{
	UINT16 usMax=0;
	UINT8 ubRepairPtsLeft =0;
	UINT8 ubInitialRepairPts = 0;
	UINT8 ubRepairPtsUsed = 0;
	INT8 bPocket =0;
	BOOLEAN fNothingLeftToRepair = FALSE;
	INT8	bLoop, bLoopStart, bLoopEnd;
	BOOLEAN fAnyOfSoldiersOwnItemsWereFixed = FALSE;
	OBJECTTYPE * pObj;


	// grab max number of repair pts open to this soldier
	ubRepairPtsLeft = CalculateRepairPointsForRepairman(pSoldier, &usMax, TRUE);

	// no points
	if (ubRepairPtsLeft == 0)
	{
		AssignmentDone( pSoldier, TRUE, TRUE );
		return;
	}

	// remember what we've started off with
	ubInitialRepairPts = ubRepairPtsLeft;

	// check if we are repairing a vehicle
	if ( pSoldier->bVehicleUnderRepairID != -1 )
	{
		if ( CanCharacterRepairVehicle( pSoldier, pSoldier->bVehicleUnderRepairID ) )
		{
			// attempt to fix vehicle
			ubRepairPtsLeft -= RepairVehicle( pSoldier->bVehicleUnderRepairID, ubRepairPtsLeft, &fNothingLeftToRepair );
		}
	}
	// check if we are repairing a robot
	else if( pSoldier->fFixingRobot )
	{
		if ( CanCharacterRepairRobot( pSoldier ) )
		{
			// repairing the robot is very slow & difficult
			ubRepairPtsLeft /= 2;
			ubInitialRepairPts /= 2;

			if( !( HAS_SKILL_TRAIT( pSoldier, ELECTRONICS ) ) )
			{
				ubRepairPtsLeft /= 2;
				ubInitialRepairPts /= 2;
			}

			// robot
			ubRepairPtsLeft -= HandleRepairOfRobotBySoldier( pSoldier, ubRepairPtsLeft, &fNothingLeftToRepair );
		}
	}
	else
	{
		fAnyOfSoldiersOwnItemsWereFixed = UnjamGunsOnSoldier( pSoldier, pSoldier, &ubRepairPtsLeft );

		// repair items on self
		for( bLoop = 0; bLoop < 2; bLoop++ )
		{
			if ( bLoop == 0 )
			{
				bLoopStart = SECONDHANDPOS;
				bLoopEnd = SMALLPOCK8POS;
			}
			else
			{
				bLoopStart = HELMETPOS;
				bLoopEnd = HEAD2POS;
			}

			// now repair objects running from left hand to small pocket
			for( bPocket = bLoopStart; bPocket <= bLoopEnd; bPocket++ )
			{
				pObj = &(pSoldier->inv[ bPocket ]);

				if ( RepairObject( pSoldier, pSoldier, pObj, &ubRepairPtsLeft ) )
				{
					fAnyOfSoldiersOwnItemsWereFixed = TRUE;

					// quit looking if we're already out
					if ( ubRepairPtsLeft == 0 )
						break;
				}
			}
		}

		// if he fixed something of his, and now has no more of his own items to fix
		if ( fAnyOfSoldiersOwnItemsWereFixed && !DoesCharacterHaveAnyItemsToRepair( pSoldier, -1 ) )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sRepairsDoneString[ 0 ], pSoldier->name );

			// let player react
			StopTimeCompression();
		}


		// repair items on others
		RepairItemsOnOthers( pSoldier, &ubRepairPtsLeft );
	}

	// what are the total amount of pts used by character?
	ubRepairPtsUsed = ubInitialRepairPts - ubRepairPtsLeft;
	if( ubRepairPtsUsed > 0 )
	{
		// improve stats
		StatChange( pSoldier, MECHANAMT, ( UINT16 ) (ubRepairPtsUsed / 2), FALSE );
		StatChange( pSoldier, DEXTAMT,	 ( UINT16 ) (ubRepairPtsUsed / 2), FALSE );

		// check if kit damaged/depleted
		if( ( Random( 100 ) ) < (UINT32) (ubRepairPtsUsed * 5) ) // CJC: added a x5 as this wasn't going down anywhere fast enough
		{
			// kit item damaged/depleted, burn up points of toolkit..which is in right hand
			UseKitPoints( &( pSoldier -> inv[ HANDPOS ] ), 1, pSoldier );
		}
	}


	// if he really done
	if ( HasCharacterFinishedRepairing( pSoldier ) )
	{
		// yup, that's all folks
		AssignmentDone( pSoldier, TRUE, TRUE );
	}
	else	// still has stuff to repair
	{
		// if nothing got repaired, there's a problem
		if ( ubRepairPtsUsed == 0 )
		{
			// see if not having a toolkit is the problem
			if ( FindObj( pSoldier, TOOLKIT ) == NO_SLOT )
			{
				// he could (maybe) repair something, but can't because he doesn't have a tool kit!
				AssignmentAborted( pSoldier, NO_MORE_TOOL_KITS );
			}
			else
			{
				// he can't repair anything because he doesn't have enough skill!
				AssignmentAborted( pSoldier, INSUF_REPAIR_SKILL );
			}
		}
	}
}


// can item be repaired?
static BOOLEAN IsItemRepairable(UINT16 usItem, INT8 bStatus)
{
	// check to see if item can/needs to be repaired
	if ( ( bStatus < 100) && ( Item[ usItem ].fFlags & ITEM_REPAIRABLE ) )
	{
		// yep
		return ( TRUE );
	}

	// nope
	return ( FALSE );
}


static UINT8 CalcSoldierNeedForSleep(SOLDIERTYPE* pSoldier);


// rest the character
static void RestCharacter(SOLDIERTYPE* pSoldier)
{
	// handle the sleep of this character, update bBreathMax based on sleep they have
	INT8 bMaxBreathRegain = 0;

	bMaxBreathRegain = 50 / CalcSoldierNeedForSleep( pSoldier );

	// if breath max is below the "really tired" threshold
	if( pSoldier -> bBreathMax < BREATHMAX_PRETTY_TIRED )
	{
		// real tired, rest rate is 50% higher (this is to prevent absurdly long sleep times for totally exhausted mercs)
		bMaxBreathRegain = ( bMaxBreathRegain * 3 / 2 );
	}

	pSoldier -> bBreathMax += bMaxBreathRegain;


	if( pSoldier -> bBreathMax > 100 )
	{
		pSoldier -> bBreathMax = 100;
	}
	else if( pSoldier -> bBreathMax < BREATHMAX_ABSOLUTE_MINIMUM )
	{
		pSoldier -> bBreathMax = BREATHMAX_ABSOLUTE_MINIMUM;
	}

	pSoldier -> bBreath = pSoldier -> bBreathMax;


	if ( pSoldier-> bBreathMax >= BREATHMAX_CANCEL_TIRED )
	{
		pSoldier->fComplainedThatTired = FALSE;
	}
}


static void FatigueCharacter(SOLDIERTYPE* pSoldier)
{
	// fatigue character
	INT32 iPercentEncumbrance;
	INT32 iBreathLoss;
	INT8 bMaxBreathLoss = 0, bDivisor = 1;


	// vehicle or robot?
	if( ( pSoldier -> uiStatusFlags & SOLDIER_VEHICLE ) || AM_A_ROBOT( pSoldier ) )
	{
		return;
	}

	// check if in transit, do not wear out
	if( IsCharacterInTransit( pSoldier ) == TRUE )
	{
		return;
	}

	// POW?
	if( pSoldier -> bAssignment == ASSIGNMENT_POW )
	{
		return;
	}


	bDivisor = 24 - CalcSoldierNeedForSleep( pSoldier );
	bMaxBreathLoss = 50 / bDivisor;

	if( bMaxBreathLoss < 2 )
	{
		bMaxBreathLoss = 2;
	}

	//KM: Added encumbrance calculation to soldiers moving on foot.  Anything above 100% will increase
	//    rate of fatigue.  200% encumbrance will cause soldiers to tire twice as quickly.
	if( pSoldier->fBetweenSectors && pSoldier->bAssignment != VEHICLE )
	{ //Soldier is on foot and travelling.  Factor encumbrance into fatigue rate.
		iPercentEncumbrance = CalculateCarriedWeight( pSoldier );
		if( iPercentEncumbrance > 100 )
		{
			iBreathLoss = (bMaxBreathLoss * iPercentEncumbrance / 100);
			bMaxBreathLoss = (INT8)min( 127, iBreathLoss );
		}
	}

	// if breath max is below the "really tired" threshold
	if( pSoldier -> bBreathMax < BREATHMAX_PRETTY_TIRED )
	{
		// real tired, fatigue rate is 50% higher
		bMaxBreathLoss = ( bMaxBreathLoss * 3 / 2 );
	}


	pSoldier -> bBreathMax -= bMaxBreathLoss;

	if( pSoldier -> bBreathMax > 100 )
	{
		pSoldier -> bBreathMax = 100;
	}
	else if( pSoldier -> bBreathMax < BREATHMAX_ABSOLUTE_MINIMUM )
	{
		pSoldier -> bBreathMax = BREATHMAX_ABSOLUTE_MINIMUM;
	}

	// current breath can't exceed maximum
	if( pSoldier -> bBreath > pSoldier -> bBreathMax )
	{
		pSoldier -> bBreath = pSoldier -> bBreathMax;
	}
}


static int TownTrainerQsortCompare(const void* pArg1, const void* pArg2);
static void TrainSoldierWithPts(SOLDIERTYPE* pSoldier, INT16 sTrainPts);
static BOOLEAN TrainTownInSector(SOLDIERTYPE* pTrainer, INT16 sMapX, INT16 sMapY, INT16 sTrainingPts);


// ONCE PER HOUR, will handle ALL kinds of training (self, teaching, and town) in this sector
static void HandleTrainingInSector(const INT16 sMapX, const INT16 sMapY, const INT8 bZ)
{
	UINT8 ubStat;
	BOOLEAN fAtGunRange = FALSE;
	INT16 sTotalTrainingPts = 0;
	INT16 sTrainingPtsDueToInstructor = 0;
	INT16 sBestTrainingPts;
	INT16 sTownTrainingPts;
	TOWN_TRAINER_TYPE TownTrainer[ MAX_CHARACTER_COUNT ];
	UINT8 ubTownTrainers;
	UINT16 usMaxPts;
	BOOLEAN fTrainingCompleted = FALSE;

	// Training in underground sectors is disallowed by the interface code, so there should never be any
	if (bZ != 0)
	{
		return;
	}

	// if sector not under our control, has enemies in it, or is currently in combat mode
	if (!SectorOursAndPeaceful( sMapX, sMapY, bZ ))
	{
		// then training is canceled for this hour.
		// This is partly logical, but largely to prevent newly trained militia from appearing in mid-battle
		return;
	}

	// are we training in the sector with gun range in Alma?
	if ( (sMapX == GUN_RANGE_X) && (sMapY == GUN_RANGE_Y) && (bZ == GUN_RANGE_Z) )
	{
		fAtGunRange = TRUE;
	}

	// init trainer list
	const SOLDIERTYPE* pStatTrainerList[NUM_TRAINABLE_STATS]; // can't have more "best" trainers than trainable stats
	memset( pStatTrainerList, 0, sizeof( pStatTrainerList ) );

	// build list of teammate trainers in this sector.

	// Only the trainer with the HIGHEST training ability in each stat is effective.  This is mainly to avoid having to
	// sort them from highest to lowest if some form of trainer degradation formula was to be used for multiple trainers.

	// for each trainable stat
	for (ubStat = 0; ubStat < NUM_TRAINABLE_STATS; ubStat++)
	{
		sBestTrainingPts = -1;

		// search team for active instructors in this sector
		CFOR_ALL_IN_TEAM(pTrainer, OUR_TEAM)
		{
			if (pTrainer->sSectorX == sMapX && pTrainer->sSectorY == sMapY && pTrainer->bSectorZ == bZ)
			{
				// if he's training teammates in this stat
				if( ( pTrainer -> bAssignment == TRAIN_TEAMMATE ) && ( pTrainer -> bTrainStat == ubStat) && ( EnoughTimeOnAssignment( pTrainer ) ) && ( pTrainer->fMercAsleep == FALSE ) )
				{
					sTrainingPtsDueToInstructor = GetBonusTrainingPtsDueToInstructor( pTrainer, NULL, ubStat, fAtGunRange, &usMaxPts );

					// if he's the best trainer so far for this stat
					if (sTrainingPtsDueToInstructor > sBestTrainingPts)
					{
						// then remember him as that, and the points he scored
						pStatTrainerList[ ubStat ] = pTrainer;
						sBestTrainingPts = sTrainingPtsDueToInstructor;
					}
				}
			}
		}
	}


	// now search team for active self-trainers in this sector
	FOR_ALL_IN_TEAM(pStudent, OUR_TEAM)
	{
		// see if this merc is active and in the same sector
		if (pStudent->sSectorX == sMapX && pStudent->sSectorY == sMapY && pStudent->bSectorZ == bZ)
		{
			// if he's training himself (alone, or by others), then he's a student
			if ( ( pStudent -> bAssignment == TRAIN_SELF ) || ( pStudent -> bAssignment == TRAIN_BY_OTHER ) )
			{
				if ( EnoughTimeOnAssignment( pStudent ) && ( pStudent->fMercAsleep == FALSE ) )
				{
					// figure out how much the grunt can learn in one training period
					sTotalTrainingPts = GetSoldierTrainingPts( pStudent, pStudent -> bTrainStat, fAtGunRange, &usMaxPts );

					// if he's getting help
					if ( pStudent -> bAssignment == TRAIN_BY_OTHER )
					{
						// grab the pointer to the (potential) trainer for this stat
						const SOLDIERTYPE* const pTrainer = pStatTrainerList[pStudent->bTrainStat];

						// if this stat HAS a trainer in sector at all
						if (pTrainer != NULL)
						{
/* Assignment distance limits removed.  Sep/11/98.  ARM
							// if this sector either ISN'T currently loaded, or it is but the trainer is close enough to the student
							if ( ( sMapX != gWorldSectorX ) || ( sMapY != gWorldSectorY ) || ( pStudent -> bSectorZ != gbWorldSectorZ ) ||
									PythSpacesAway(pStudent->sGridNo, pTrainer->sGridNo) < MAX_DISTANCE_FOR_TRAINING &&
									EnoughTimeOnAssignment(pTrainer))
*/
							// NB this EnoughTimeOnAssignment() call is redundent since it is called up above
							//if ( EnoughTimeOnAssignment( pTrainer ) )
							{
								// valid trainer is available, this gives the student a large training bonus!
								sTrainingPtsDueToInstructor = GetBonusTrainingPtsDueToInstructor( pTrainer, pStudent, pStudent -> bTrainStat, fAtGunRange, &usMaxPts );

								// add the bonus to what merc can learn on his own
								sTotalTrainingPts += sTrainingPtsDueToInstructor;
							}
						}
					}

					// now finally train the grunt
					TrainSoldierWithPts( pStudent, sTotalTrainingPts );
				}
			}
		}
	}

	// check if we're doing a sector where militia can be trained
	if (CanSectorContainMilita(sMapX, sMapY, bZ))
	{
		// init town trainer list
		memset( TownTrainer, 0, sizeof( TownTrainer ) );
		ubTownTrainers = 0;

		// build list of all the town trainers in this sector and their training pts
		FOR_ALL_IN_TEAM(pTrainer, OUR_TEAM)
		{
			if (pTrainer->sSectorX == sMapX && pTrainer->sSectorY == sMapY && pTrainer->bSectorZ == bZ)
			{
				if( ( pTrainer -> bAssignment == TRAIN_TOWN ) && ( EnoughTimeOnAssignment( pTrainer ) )  && ( pTrainer->fMercAsleep == FALSE ) )
				{
					sTownTrainingPts = GetTownTrainPtsForCharacter( pTrainer, &usMaxPts );

					// if he's actually worth anything
					if( sTownTrainingPts > 0 )
					{
						// remember this guy as a town trainer
						TownTrainer[ubTownTrainers].sTrainingPts = sTownTrainingPts;
						TownTrainer[ubTownTrainers].pSoldier = pTrainer;
						ubTownTrainers++;
					}
				}
			}
		}


		// if we have more than one
		if (ubTownTrainers > 1)
		{
			// sort the town trainer list from best trainer to worst
			qsort( TownTrainer, ubTownTrainers, sizeof(TOWN_TRAINER_TYPE), TownTrainerQsortCompare);
		}

		// for each trainer, in sorted order from the best to the worst
		for (UINT32 uiCnt = 0; uiCnt < ubTownTrainers; uiCnt++)
		{
			// top trainer has full effect (divide by 1), then divide by 2, 4, 8, etc.
			//sTownTrainingPts = TownTrainer[ uiCnt ].sTrainingPts / (UINT16) pow(2, uiCnt);
			// CJC: took this out and replaced with limit of 2 guys per sector
			sTownTrainingPts = TownTrainer[ uiCnt ].sTrainingPts;

			if (sTownTrainingPts > 0)
			{
				fTrainingCompleted = TrainTownInSector( TownTrainer[ uiCnt ].pSoldier, sMapX, sMapY, sTownTrainingPts );

				if ( fTrainingCompleted )
				{
					// there's no carryover into next session for extra training (cause player might cancel), so break out of loop
					break;
				}
			}
		}
	}
}


static int TownTrainerQsortCompare(const void* pArg1, const void* pArg2)
{
	const TOWN_TRAINER_TYPE* const t1 = pArg1;
	const TOWN_TRAINER_TYPE* const t2 = pArg2;
	return (t1->sTrainingPts < t2->sTrainingPts) - (t1->sTrainingPts > t2->sTrainingPts);
}


INT16 GetBonusTrainingPtsDueToInstructor(const SOLDIERTYPE* pInstructor, const SOLDIERTYPE* pStudent, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16* pusMaxPts)
{
	// return the bonus training pts of this instructor with this student,...if student null, simply assignment student skill of 0 and student wisdom of 100
	INT16 sTrainingPts = 0;
	INT8 bTraineeEffWisdom = 0;
	INT8 bTraineeNatWisdom = 0;
	INT8 bTraineeSkill = 0;
	INT8 bTrainerEffSkill = 0;
	INT8 bTrainerNatSkill = 0;
	INT8 bTrainingBonus = 0;
	INT8 bOpinionFactor;

	// assume training impossible for max pts
	*pusMaxPts = 0;

	if( pInstructor == NULL )
	{
		// no instructor, leave
		return ( 0 );
	}


	switch( bTrainStat )
	{
		case( STRENGTH ):
			bTrainerEffSkill = EffectiveStrength ( pInstructor );
			bTrainerNatSkill = pInstructor->bStrength;
		break;
		case( DEXTERITY ):
			bTrainerEffSkill = EffectiveDexterity ( pInstructor );
			bTrainerNatSkill = pInstructor->bDexterity;
		break;
		case( AGILITY ):
			bTrainerEffSkill = EffectiveAgility( pInstructor );
			bTrainerNatSkill = pInstructor->bAgility;
		break;
		case( HEALTH ):
			bTrainerEffSkill = pInstructor -> bLifeMax;
			bTrainerNatSkill = pInstructor->bLifeMax;
		break;
		case( LEADERSHIP ):
			bTrainerEffSkill = EffectiveLeadership( pInstructor );
			bTrainerNatSkill = pInstructor->bLeadership;
		break;
		case( MARKSMANSHIP ):
			bTrainerEffSkill = EffectiveMarksmanship( pInstructor );
			bTrainerNatSkill = pInstructor->bMarksmanship;
		break;
		case( EXPLOSIVE_ASSIGN ):
			bTrainerEffSkill = EffectiveExplosive( pInstructor );
			bTrainerNatSkill = pInstructor->bExplosive;
		break;
		case( MEDICAL ):
			bTrainerEffSkill = EffectiveMedical( pInstructor );
			bTrainerNatSkill = pInstructor->bMedical;
		break;
		case( MECHANICAL ):
			bTrainerEffSkill = EffectiveMechanical( pInstructor );
			bTrainerNatSkill = pInstructor->bMechanical;
		break;
		// NOTE: Wisdom can't be trained!
		default:
			// BETA message
			#ifdef JA2BETAVERSION
				ScreenMsg( FONT_ORANGE, MSG_BETAVERSION, L"GetBonusTrainingPtsDueToInstructor: ERROR - Unknown bTrainStat %d", bTrainStat);
			#endif
			return(0);
	}


	// if there's no student
	if( pStudent == NULL )
	{
		// assume these default values
		bTraineeEffWisdom = 100;
		bTraineeNatWisdom = 100;
		bTraineeSkill     =   0;
		bOpinionFactor    =   0;
	}
	else
	{
		// set student's variables
		bTraineeEffWisdom = EffectiveWisdom ( pStudent );
		bTraineeNatWisdom = pStudent->bWisdom;

		// for trainee's stat skill, must use the natural value, not the effective one, to avoid drunks training beyond cap
		bTraineeSkill = GetTrainingStatValue(pStudent, bTrainStat);

		// if trainee skill 0 or at/beyond the training cap, can't train
		if ( ( bTraineeSkill == 0 ) || ( bTraineeSkill >= TRAINING_RATING_CAP ) )
		{
			return 0;
		}

		// factor in their mutual relationship
		bOpinionFactor  = gMercProfiles[    pStudent->ubProfile ].bMercOpinion[ pInstructor->ubProfile ];
		bOpinionFactor += gMercProfiles[ pInstructor->ubProfile ].bMercOpinion[    pStudent->ubProfile ] / 2;
	}


	// check to see if student better than/equal to instructor's effective skill, if so, return 0
	// don't use natural skill - if the guy's too doped up to tell what he know, student learns nothing until sobriety returns!
	if( bTraineeSkill >= bTrainerEffSkill )
	{
		return ( 0 );
	}

	// calculate effective training pts
	sTrainingPts = ( bTrainerEffSkill - bTraineeSkill ) * ( bTraineeEffWisdom + ( EffectiveWisdom( pInstructor ) + EffectiveLeadership( pInstructor ) ) / 2 ) / INSTRUCTED_TRAINING_DIVISOR;

	// calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs, fatigue)
	*pusMaxPts   = ( bTrainerNatSkill - bTraineeSkill ) * ( bTraineeNatWisdom + ( pInstructor->bWisdom + pInstructor->bLeadership ) / 2 ) / INSTRUCTED_TRAINING_DIVISOR;

	// put in a minimum (that can be reduced due to instructor being tired?)
	if (*pusMaxPts == 0)
	{
		// we know trainer is better than trainee, make sure they are at least 10 pts better
		if ( bTrainerEffSkill > bTraineeSkill + 10 )
		{
			sTrainingPts = 1;
			*pusMaxPts = 1;
		}
	}

	// check for teaching skill bonuses
	if( gMercProfiles[ pInstructor -> ubProfile ].bSkillTrait == TEACHING )
	{
		bTrainingBonus += TEACH_BONUS_TO_TRAIN;
	}
	if( gMercProfiles[ pInstructor -> ubProfile ].bSkillTrait2 == TEACHING )
	{
		bTrainingBonus += TEACH_BONUS_TO_TRAIN;
	}

	// teaching bonus is counted as normal, but gun range bonus is not
	*pusMaxPts += ( ( ( bTrainingBonus + bOpinionFactor ) * *pusMaxPts ) / 100 );

	// get special bonus if we're training marksmanship and we're in the gun range sector in Alma
	if ( ( bTrainStat == MARKSMANSHIP ) && fAtGunRange )
	{
		bTrainingBonus += GUN_RANGE_TRAINING_BONUS;
	}

	// adjust for any training bonuses and for the relationship
	sTrainingPts += ( ( ( bTrainingBonus + bOpinionFactor ) * sTrainingPts ) / 100 );

	// adjust for instructor fatigue
	ReducePointsForFatigue( pInstructor, &sTrainingPts );

	return( sTrainingPts );
}


INT16 GetSoldierTrainingPts(const SOLDIERTYPE* s, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16* pusMaxPts)
{
	INT16 sTrainingPts = 0;
	INT8	bTrainingBonus = 0;

	// assume training impossible for max pts
	*pusMaxPts = 0;

	// use NATURAL not EFFECTIVE values here
	const INT8 bSkill = GetTrainingStatValue(s, bTrainStat);

	// if skill 0 or at/beyond the training cap, can't train
	if ( ( bSkill == 0 ) || ( bSkill >= TRAINING_RATING_CAP ) )
	{
		return 0;
	}


	// calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs, fatigue)
	*pusMaxPts = __max(s->bWisdom * (TRAINING_RATING_CAP - bSkill) / SELF_TRAINING_DIVISOR, 1);

	// calculate effective training pts
	sTrainingPts = __max(EffectiveWisdom(s) * (TRAINING_RATING_CAP - bSkill) / SELF_TRAINING_DIVISOR, 1);

	// get special bonus if we're training marksmanship and we're in the gun range sector in Alma
	if ( ( bTrainStat == MARKSMANSHIP ) && fAtGunRange )
	{
		bTrainingBonus += GUN_RANGE_TRAINING_BONUS;
	}

	// adjust for any training bonuses
	sTrainingPts += ( ( bTrainingBonus * sTrainingPts ) / 100 );

	// adjust for fatigue
	ReducePointsForFatigue(s, &sTrainingPts);

	return( sTrainingPts );
}


INT16 GetSoldierStudentPts(const SOLDIERTYPE* s, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16* pusMaxPts)
{
	INT16 sTrainingPts = 0;
	INT8	bTrainingBonus = 0;

	INT16 sBestTrainingPts, sTrainingPtsDueToInstructor;
	UINT16	usMaxTrainerPts, usBestMaxTrainerPts;

	// assume training impossible for max pts
	*pusMaxPts = 0;

	// use NATURAL not EFFECTIVE values here
	const INT8 bSkill = GetTrainingStatValue(s, bTrainStat);

	// if skill 0 or at/beyond the training cap, can't train
	if ( ( bSkill == 0 ) || ( bSkill >= TRAINING_RATING_CAP ) )
	{
		return 0;
	}


	// calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs, fatigue)
	*pusMaxPts = __max(s->bWisdom * (TRAINING_RATING_CAP - bSkill) / SELF_TRAINING_DIVISOR, 1);

	// calculate effective training pts
	sTrainingPts = __max(EffectiveWisdom(s) * (TRAINING_RATING_CAP - bSkill) / SELF_TRAINING_DIVISOR, 1);

	// get special bonus if we're training marksmanship and we're in the gun range sector in Alma
	if ( ( bTrainStat == MARKSMANSHIP ) && fAtGunRange )
	{
		bTrainingBonus += GUN_RANGE_TRAINING_BONUS;
	}

	// adjust for any training bonuses
	sTrainingPts += ( ( bTrainingBonus * sTrainingPts ) / 100 );

	// adjust for fatigue
	ReducePointsForFatigue(s, &sTrainingPts);


	// now add in stuff for trainer

	// for each trainable stat
	sBestTrainingPts = -1;

	// search team for active instructors in this sector
	CFOR_ALL_IN_TEAM(pTrainer, OUR_TEAM)
	{
		if (pTrainer->sSectorX == s->sSectorX && pTrainer->sSectorY == s->sSectorY && pTrainer->bSectorZ == s->bSectorZ)
		{
			// if he's training teammates in this stat
			// NB skip the EnoughTime requirement to display what the value should be even if haven't been training long yet...
			if ( ( pTrainer->bAssignment == TRAIN_TEAMMATE ) && ( pTrainer->bTrainStat == bTrainStat) && ( pTrainer->fMercAsleep == FALSE ) )
			{
				sTrainingPtsDueToInstructor = GetBonusTrainingPtsDueToInstructor(pTrainer, s, bTrainStat, fAtGunRange, &usMaxTrainerPts);

				// if he's the best trainer so far for this stat
				if (sTrainingPtsDueToInstructor > sBestTrainingPts)
				{
					// then remember him as that, and the points he scored
					sBestTrainingPts = sTrainingPtsDueToInstructor;
					usBestMaxTrainerPts = usMaxTrainerPts;
				}
			}
		}
	}

	if ( sBestTrainingPts != -1 )
	{
		// add the bonus to what merc can learn on his own
		sTrainingPts += sBestTrainingPts;
		*pusMaxPts += usBestMaxTrainerPts;
	}

	return( sTrainingPts );
}


// this function will actually pass on the pts to the mercs stat
static void TrainSoldierWithPts(SOLDIERTYPE* pSoldier, INT16 sTrainPts)
{
	UINT8 ubChangeStat = 0;

	if( sTrainPts <= 0 )
	{
		return;
	}

	// which stat to modify?
	switch( pSoldier -> bTrainStat )
	{
		case( STRENGTH ):
			ubChangeStat = STRAMT;
			break;
		case( DEXTERITY ):
			ubChangeStat = DEXTAMT;
			break;
		case( AGILITY ):
			ubChangeStat = AGILAMT;
			break;
		case( HEALTH ):
			ubChangeStat = HEALTHAMT;
			break;
		case( LEADERSHIP ):
			ubChangeStat = LDRAMT;
			break;
		case( MARKSMANSHIP ):
			ubChangeStat = MARKAMT;
			break;
		case( EXPLOSIVE_ASSIGN ):
			ubChangeStat = EXPLODEAMT;
			break;
		case( MEDICAL ):
			ubChangeStat = MEDICALAMT;
			break;
		case( MECHANICAL ):
			ubChangeStat = MECHANAMT;
			break;
		// NOTE: Wisdom can't be trained!
		default:
			// BETA message
#ifdef JA2BETAVERSION
			ScreenMsg(FONT_ORANGE, MSG_BETAVERSION, L"TrainSoldierWithPts: ERROR - Unknown bTrainStat %d", pSoldier->bTrainStat);
#endif
			return;
	}

	// give this merc a few chances to increase a stat (TRUE means it's training, reverse evolution doesn't apply)
	StatChange( pSoldier, ubChangeStat, sTrainPts, FROM_TRAINING );
}


// train militia in this sector with this soldier
static BOOLEAN TrainTownInSector(SOLDIERTYPE* pTrainer, INT16 sMapX, INT16 sMapY, INT16 sTrainingPts)
{
	Assert(CanSectorContainMilita(pTrainer->sSectorX, pTrainer->sSectorY, pTrainer->bSectorZ));

	SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( sMapX, sMapY ) ] );

	// trainer gains leadership - training argument is FALSE, because the trainer is not the one training!
	StatChange( pTrainer, LDRAMT,		 (UINT16) ( 1 + ( sTrainingPts / 200 ) ), FALSE );
//	StatChange( pTrainer, WISDOMAMT, (UINT16) ( 1 + ( sTrainingPts / 400 ) ), FALSE );


	// increase town's training completed percentage
	pSectorInfo -> ubMilitiaTrainingPercentDone += (sTrainingPts / 100);
	pSectorInfo -> ubMilitiaTrainingHundredths  += (sTrainingPts % 100);

	if (pSectorInfo -> ubMilitiaTrainingHundredths >= 100)
	{
		pSectorInfo -> ubMilitiaTrainingPercentDone++;
		pSectorInfo -> ubMilitiaTrainingHundredths -= 100;
	}

	// NOTE: Leave this at 100, change TOWN_TRAINING_RATE if necessary.  This value gets reported to player as a %age!
	if( pSectorInfo -> ubMilitiaTrainingPercentDone >= 100 )
	{
		// zero out training completion - there's no carryover to the next training session
		pSectorInfo -> ubMilitiaTrainingPercentDone = 0;
		pSectorInfo -> ubMilitiaTrainingHundredths  = 0;

		// make the player pay again next time he wants to train here
		pSectorInfo -> fMilitiaTrainingPaid = FALSE;

		TownMilitiaTrainingCompleted( pTrainer, sMapX, sMapY );

		// training done
		return( TRUE );
	}
	else
	{
		// not done
		return ( FALSE );
	}
}


INT16 GetTownTrainPtsForCharacter(const SOLDIERTYPE* pTrainer, UINT16* pusMaxPts)
{
	INT16 sTotalTrainingPts = 0;
	INT8 bTrainingBonus = 0;
//	UINT8 ubTownId = 0;

	// calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs, fatigue)
	*pusMaxPts = ( pTrainer -> bWisdom + pTrainer -> bLeadership + ( 10 * pTrainer -> bExpLevel ) ) * TOWN_TRAINING_RATE;

	// calculate effective training points (this is hundredths of pts / hour)
	// typical: 300/hr, maximum: 600/hr
	sTotalTrainingPts = ( EffectiveWisdom( pTrainer ) + EffectiveLeadership ( pTrainer ) + ( 10 * EffectiveExpLevel ( pTrainer ) ) ) * TOWN_TRAINING_RATE;

	// check for teaching bonuses
	if( gMercProfiles[ pTrainer -> ubProfile ].bSkillTrait == TEACHING )
	{
		bTrainingBonus += TEACH_BONUS_TO_TRAIN;
	}
	if( gMercProfiles[ pTrainer -> ubProfile ].bSkillTrait2 == TEACHING )
	{
		bTrainingBonus += TEACH_BONUS_TO_TRAIN;
	}

	// RPCs get a small training bonus for being more familiar with the locals and their customs/needs than outsiders
	if( pTrainer->ubProfile >= FIRST_RPC )
	{
		bTrainingBonus += RPC_BONUS_TO_TRAIN;
	}

	// adjust for teaching bonus (a percentage)
	sTotalTrainingPts += ( ( bTrainingBonus * sTotalTrainingPts ) / 100 );
	// teach bonus is considered "normal" - it's always there
	*pusMaxPts				+= ( ( bTrainingBonus * *pusMaxPts        ) / 100 );


	// adjust for fatigue of trainer
	ReducePointsForFatigue( pTrainer, &sTotalTrainingPts );


/* ARM: Decided this didn't make much sense - the guys I'm training damn well BETTER be loyal - and screw the rest!
	// get town index
	ubTownId = StrategicMap[ pTrainer -> sSectorX + pTrainer -> sSectorY * MAP_WORLD_X ].bNameId;
	Assert(ubTownId != BLANK_SECTOR);

	// adjust for town loyalty
	sTotalTrainingPts = (sTotalTrainingPts * gTownLoyalty[ ubTownId ].ubRating) / 100;
*/

	return( sTotalTrainingPts );
}


void MakeSoldiersTacticalAnimationReflectAssignment( SOLDIERTYPE *pSoldier )
{
	// soldier is in tactical, world loaded, he's OKLIFE
	if( ( pSoldier -> bInSector ) && gfWorldLoaded && ( pSoldier -> bLife >= OKLIFE ) )
	{
		// Set animation based on his assignment
		if ( pSoldier -> bAssignment == DOCTOR )
		{
			SoldierInSectorDoctor( pSoldier, pSoldier -> usStrategicInsertionData );
		}
		else if ( pSoldier -> bAssignment == PATIENT )
		{
			SoldierInSectorPatient( pSoldier, pSoldier -> usStrategicInsertionData );
		}
		else if ( pSoldier -> bAssignment == REPAIR )
		{
			SoldierInSectorRepair( pSoldier, pSoldier -> usStrategicInsertionData );
		}
		else
		{
			if (pSoldier->usAnimState != WKAEUP_FROM_SLEEP && !(pSoldier->bOldAssignment < ON_DUTY))
			{
				// default: standing
				ChangeSoldierState( pSoldier, STANDING, 1, TRUE );
			}
		}
	}
}


static void AssignmentAborted(SOLDIERTYPE* pSoldier, UINT8 ubReason)
{
	Assert( ubReason < NUM_ASSIGN_ABORT_REASONS );

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ ubReason ], pSoldier->name );

	StopTimeCompression();

	// update mapscreen
	fCharacterInfoPanelDirty = TRUE;
	fTeamPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;
}



void AssignmentDone( SOLDIERTYPE *pSoldier, BOOLEAN fSayQuote, BOOLEAN fMeToo )
{
	if ( ( pSoldier -> bInSector ) && ( gfWorldLoaded ) )
	{
		if ( pSoldier -> bAssignment == DOCTOR )
		{
			if ( guiCurrentScreen == GAME_SCREEN )
			{
				ChangeSoldierState( pSoldier, END_DOCTOR, 1, TRUE );
			}
			else
			{
				ChangeSoldierState( pSoldier, STANDING, 1, TRUE );
			}

		}
		else if ( pSoldier -> bAssignment == REPAIR )
		{
			if ( guiCurrentScreen == GAME_SCREEN )
			{
				ChangeSoldierState( pSoldier, END_REPAIRMAN, 1, TRUE );
			}
			else
			{
				ChangeSoldierState( pSoldier, STANDING, 1, TRUE );
			}
		}
		else if ( pSoldier -> bAssignment == PATIENT )
		{
			if ( guiCurrentScreen == GAME_SCREEN )
			{
				ChangeSoldierStance( pSoldier, ANIM_CROUCH );
			}
			else
			{
				ChangeSoldierState( pSoldier, STANDING, 1, TRUE );
			}
		}
	}

	if ( pSoldier->bAssignment == ASSIGNMENT_HOSPITAL )
	{
		// hack - reset AbsoluteFinalDestination in case it was left non-nowhere
		pSoldier->sAbsoluteFinalDestination = NOWHERE;
	}

	if ( fSayQuote )
	{
		if ( ( fMeToo == FALSE ) && (pSoldier->bAssignment == TRAIN_TOWN  ) )
		{
			TacticalCharacterDialogue( pSoldier, QUOTE_ASSIGNMENT_COMPLETE );

			if( pSoldier -> bAssignment == TRAIN_TOWN )
			{
				AddSectorForSoldierToListOfSectorsThatCompletedMilitiaTraining( pSoldier );
			}
		}
	}

	// don't bother telling us again about guys we already know about
	if ( !( pSoldier->usQuoteSaidExtFlags & SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT ) )
	{
		pSoldier->usQuoteSaidExtFlags |= SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;

		if ( fSayQuote )
		{
			if ( pSoldier->bAssignment == DOCTOR || pSoldier->bAssignment == REPAIR ||
					pSoldier->bAssignment == PATIENT || pSoldier->bAssignment == ASSIGNMENT_HOSPITAL)
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_ASSIGNMENT_COMPLETE );
			}
		}


		AddReasonToWaitingListQueue( ASSIGNMENT_FINISHED_FOR_UPDATE );
		AddSoldierToWaitingListQueue( pSoldier );

		// trigger a single call AddDisplayBoxToWaitingQueue for assignments done
		gfAddDisplayBoxToWaitingQueue = TRUE;
	}

	// update mapscreen
	fCharacterInfoPanelDirty = TRUE;
	fTeamPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;
}


static void HandleHealingByNaturalCauses(SOLDIERTYPE* pSoldier);


// handle natural healing for all mercs on players team
static void HandleNaturalHealing(void)
{
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		// mechanical members don't regenerate!
		if (!(s->uiStatusFlags & SOLDIER_VEHICLE) && !(AM_A_ROBOT(s)))
		{
			HandleHealingByNaturalCauses(s);
		}
	}
}


static void UpDateSoldierLife(SOLDIERTYPE* pSoldier);


// handle healing of this soldier by natural causes.
static void HandleHealingByNaturalCauses(SOLDIERTYPE* pSoldier)
{
	UINT32 uiPercentHealth = 0;
	INT8 bActivityLevelDivisor = 0;


	// check if soldier valid
	if( pSoldier == NULL )
	{
		return;
	}

	// dead
	if( pSoldier -> bLife == 0 )
	{
		return;
	}

	// lost any pts?
	if( pSoldier -> bLife == pSoldier -> bLifeMax )
	{
		return;
	}

	// any bleeding pts - can' recover if still bleeding!
	if( pSoldier -> bBleeding != 0 )
	{
		return;
	}


	// not bleeding and injured...

	if( pSoldier -> bAssignment == ASSIGNMENT_POW )
	{
		// use high activity level to simulate stress, torture, poor conditions for healing
		bActivityLevelDivisor = HIGH_ACTIVITY_LEVEL;
	}
	if( ( pSoldier -> fMercAsleep == TRUE ) || ( pSoldier -> bAssignment == PATIENT ) || ( pSoldier -> bAssignment == ASSIGNMENT_HOSPITAL ) )
	{
		bActivityLevelDivisor = LOW_ACTIVITY_LEVEL;
	}
	else if ( pSoldier->bAssignment < ON_DUTY )
	{
		// if time is being compressed, and the soldier is not moving strategically
		if ( IsTimeBeingCompressed() && !PlayerIDGroupInMotion( pSoldier->ubGroupID ) )
		{
			// basically resting
			bActivityLevelDivisor = LOW_ACTIVITY_LEVEL;
		}
		else
		{
			// either they're on the move, or they're being tactically active
			bActivityLevelDivisor = HIGH_ACTIVITY_LEVEL;
		}
	}
	else	// this includes being in a vehicle - that's neither very strenous, nor very restful
	{
		bActivityLevelDivisor = MEDIUM_ACTIVITY_LEVEL;
	}


	// what percentage of health is he down to
	uiPercentHealth = ( pSoldier->bLife * 100 ) / pSoldier->bLifeMax;

	// gain that many hundredths of life points back, divided by the activity level modifier
	pSoldier->sFractLife += ( INT16 ) ( uiPercentHealth / bActivityLevelDivisor );

	// now update the real life values
	UpDateSoldierLife( pSoldier );
}


static void UpDateSoldierLife(SOLDIERTYPE* pSoldier)
{
	// update soldier life, make sure we don't go out of bounds
	pSoldier -> bLife += pSoldier -> sFractLife / 100;

	// keep remaining fract of life
	pSoldier -> sFractLife %= 100;

	// check if we have gone too far
	if( pSoldier -> bLife >= pSoldier -> bLifeMax )
	{
		// reduce
		pSoldier -> bLife = pSoldier -> bLifeMax;
		pSoldier -> sFractLife = 0;
	}
}


void CheckIfSoldierUnassigned( SOLDIERTYPE *pSoldier )
{
	if( pSoldier -> bAssignment == NO_ASSIGNMENT )
	{
		// unassigned
		AddCharacterToAnySquad( pSoldier );

		if( ( gfWorldLoaded ) && ( pSoldier->bInSector ) )
		{
			ChangeSoldierState( pSoldier, STANDING, 1, TRUE );
		}
	}
}


static void AssignmentMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void AssignmentMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void CheckAndUpdateTacticalAssignmentPopUpPositions(void);
static void CreateDestroyMouseRegionsForRemoveMenu(void);
static SOLDIERTYPE* GetSelectedAssignSoldier(BOOLEAN fNullOK);
static void PositionCursorForTacticalAssignmentBox(void);


static void CreateDestroyMouseRegionsForAssignmentMenu(void)
{
	static BOOLEAN fCreated = FALSE;
	UINT32 iCounter = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SOLDIERTYPE *pSoldier = NULL;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;
	static BOOLEAN fShowRemoveMenu = FALSE;


	// will create/destroy mouse regions for the map screen assignment main menu
	// check if we can only remove character from team..not assign
	if( ( bSelectedAssignChar != -1 )|| ( fShowRemoveMenu == TRUE ) )
	{
		if( fShowRemoveMenu == TRUE )
		{
			// dead guy handle menu stuff
			fShowRemoveMenu = fShowAssignmentMenu | fShowContractMenu;

			CreateDestroyMouseRegionsForRemoveMenu( );

			return;
		}
		const SOLDIERTYPE* const s = gCharactersList[bSelectedAssignChar].merc;
		if (s->bLife == 0 || s->bAssignment == ASSIGNMENT_POW)
		{
			// dead guy handle menu stuff
			fShowRemoveMenu = fShowAssignmentMenu | fShowContractMenu;

			CreateDestroyMouseRegionsForRemoveMenu( );

			return;
		}
	}


	if( ( fShowAssignmentMenu == TRUE ) && ( fCreated == FALSE ) )
	{

		gfIgnoreScrolling = FALSE;

		if( ( fShowAssignmentMenu ) && ( guiCurrentScreen == MAP_SCREEN ) )
		{
			SetBoxPosition(ghAssignmentBox, AssignmentPosition);
		}

		pSoldier = GetSelectedAssignSoldier( FALSE );

		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			// grab height of font
			iFontHeight = GetLineSpace( ghEpcBox ) + GetFontHeight( GetBoxFont( ghEpcBox ) );

			// get x.y position of box
			GetBoxPosition( ghEpcBox, &pPosition);

			// grab box x and y position
			iBoxXPosition = pPosition.iX;
			iBoxYPosition = pPosition.iY;

			// get dimensions..mostly for width
			GetBoxSize( ghEpcBox, &pDimensions );

			// get width
			iBoxWidth = pDimensions.iRight;
		}
		else
		{
			// grab height of font
			iFontHeight = GetLineSpace( ghAssignmentBox ) + GetFontHeight( GetBoxFont( ghAssignmentBox ) );

			// get x.y position of box
			GetBoxPosition( ghAssignmentBox, &pPosition);

			// grab box x and y position
			iBoxXPosition = pPosition.iX;
			iBoxYPosition = pPosition.iY;

			// get dimensions..mostly for width
			GetBoxSize( ghAssignmentBox, &pDimensions );

			// get width
			iBoxWidth = pDimensions.iRight;
		}


		// define regions
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghAssignmentBox ); iCounter++ )
		{
			// add mouse region for each line of text..and set user data
			MSYS_DefineRegion(&gAssignmentMenuRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, AssignmentMenuMvtCallBack, AssignmentMenuBtnCallback);

			MSYS_SetRegionUserData( &gAssignmentMenuRegion[ iCounter ], 0, iCounter );
		}

		// created
		fCreated = TRUE;

		// unhighlight all strings in box
		UnHighLightBox( ghAssignmentBox );
		CheckAndUpdateTacticalAssignmentPopUpPositions( );

		PositionCursorForTacticalAssignmentBox( );
	}
	else if( ( fShowAssignmentMenu == FALSE ) && ( fCreated == TRUE ) )
	{
		// destroy
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghAssignmentBox ); iCounter++ )
		{
			MSYS_RemoveRegion( &gAssignmentMenuRegion[ iCounter ] );
		}

		fShownAssignmentMenu = FALSE;

		// not created
		fCreated = FALSE;
		SetRenderFlags( RENDER_FLAG_FULL );

	}
}


static void HandleShadingOfLinesForVehicleMenu(void);
static void VehicleMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void VehicleMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void VehicleMenuCancelBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionForVehicleMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	UINT32 uiMenuLine = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition, pPoint;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;
	SOLDIERTYPE *pSoldier = NULL;


	if( fShowVehicleMenu )
	{
		GetBoxPosition( ghAssignmentBox, &pPoint);

		// get dimensions..mostly for width
		GetBoxSize( ghAssignmentBox, &pDimensions );

		// vehicle position
		VehiclePosition.iX = pPoint.iX + pDimensions.iRight;

		SetBoxPosition( ghVehicleBox , VehiclePosition );
	}


	if( ( fShowVehicleMenu == TRUE ) && ( fCreated == FALSE ) )
	{
		// grab height of font
		iFontHeight = GetLineSpace( ghVehicleBox ) + GetFontHeight( GetBoxFont( ghVehicleBox ) );

		// get x.y position of box
		GetBoxPosition( ghVehicleBox, &pPosition);

		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghVehicleBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		pSoldier = GetSelectedAssignSoldier( FALSE );

		// run through list of vehicles in sector
		FOR_ALL_VEHICLES(v)
		{
			if (IsThisVehicleAccessibleToSoldier(pSoldier, v))
			{
				// add mouse region for each accessible vehicle
				MOUSE_REGION* const r = &gVehicleMenuRegion[uiMenuLine];
				const UINT16        x = iBoxXPosition;
				const UINT16        y = iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * uiMenuLine;
				const UINT16        w = iBoxWidth;
				const UINT16        h = iFontHeight;
				MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, VehicleMenuMvtCallback, VehicleMenuBtnCallback);
				MSYS_SetRegionUserPtr(r, v);

				uiMenuLine++;
			}
		}

		// cancel line
		MOUSE_REGION* const r = &gVehicleMenuRegion[uiMenuLine];
		const UINT16        x = iBoxXPosition;
		const UINT16        y = iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * uiMenuLine;
		const UINT16        w = iBoxWidth;
		const UINT16        h = iFontHeight;
		MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, VehicleMenuMvtCallback, VehicleMenuCancelBtnCallback);

		// created
		fCreated = TRUE;

		// pause game
		PauseGame( );

		// unhighlight all strings in box
		UnHighLightBox( ghVehicleBox );

		fCreated = TRUE;

		HandleShadingOfLinesForVehicleMenu( );
	}
	else if( ( ( fShowVehicleMenu == FALSE ) || ( fShowAssignmentMenu == FALSE ) ) && ( fCreated == TRUE ) )
	{
		fCreated = FALSE;

		// remove these regions
		for( uiMenuLine = 0; uiMenuLine < GetNumberOfLinesOfTextInBox( ghVehicleBox ); uiMenuLine++ )
		{
			MSYS_RemoveRegion( &gVehicleMenuRegion[ uiMenuLine ] );
		}

		fShowVehicleMenu = FALSE;

		SetRenderFlags( RENDER_FLAG_FULL );

		HideBox( ghVehicleBox );

		if ( fShowAssignmentMenu )
		{
			// remove highlight on the parent menu
			UnHighLightBox( ghAssignmentBox );
		}
	}
}


static void HandleShadingOfLinesForVehicleMenu(void)
{
	SOLDIERTYPE *pSoldier = NULL;
	UINT32 uiMenuLine = 0;

	if (!fShowVehicleMenu || ghVehicleBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( FALSE );

	// run through list of vehicles
	CFOR_ALL_VEHICLES(v)
	{
		// inaccessible vehicles aren't listed at all!
		if (IsThisVehicleAccessibleToSoldier(pSoldier, v))
		{
			if (IsEnoughSpaceInVehicle(v))
			{
				// legal vehicle, leave it green
				UnShadeStringInBox(ghVehicleBox, uiMenuLine);
				UnSecondaryShadeStringInBox(ghVehicleBox, uiMenuLine);
			}
			else
			{
				// unjoinable vehicle - yellow
				SecondaryShadeStringInBox(ghVehicleBox, uiMenuLine);
			}

			uiMenuLine++;
		}
	}
}


static void VehicleMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SOLDIERTYPE* const s = GetSelectedAssignSoldier(FALSE);
		VEHICLETYPE* const v = MSYS_GetRegionUserPtr(pRegion);

		// inaccessible vehicles shouldn't be listed in the menu!
		Assert(IsThisVehicleAccessibleToSoldier(s, v));

		if (IsEnoughSpaceInVehicle(v))
		{
			PutSoldierInVehicle(s, v);
		}
		else
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[18], zVehicleName[v->ubVehicleType]);
		}

		fShowAssignmentMenu = FALSE;

		// update mapscreen
		fTeamPanelDirty          = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fMapScreenBottomDirty    = TRUE;
		giAssignHighLine         = -1;

		SetAssignmentForList(VEHICLE, VEHICLE2ID(v));
	}
}


static void VehicleMenuCancelBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UnHighLightBox(ghAssignmentBox);
		fShowVehicleMenu         = FALSE;
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
}


static void VehicleMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		const size_t line = pRegion - gVehicleMenuRegion;
		HighLightBoxLine(ghVehicleBox, line);
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghVehicleBox );

		HandleShadingOfLinesForVehicleMenu( );
	}
}


static void SetBoxTextAttrs(PopUpBox* const box)
{
	SetBoxFont(box, MAP_SCREEN_FONT);
	SetBoxHighLight(box, FONT_WHITE);
	SetBoxForeground(box, FONT_LTGREEN);
	SetBoxBackground(box, FONT_BLACK);
	SetBoxShade(box, FONT_GRAY7);
	SetBoxSecondaryShade(box, FONT_YELLOW);
}


static PopUpBox* CreateRepairBox(void);
static BOOLEAN IsRobotInThisSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);


static BOOLEAN DisplayRepairMenu(SOLDIERTYPE* pSoldier)
{
	// run through list of vehicles in sector and add them to pop up box
	// first, clear pop up box
	RemoveBox(ghRepairBox);
	ghRepairBox = NO_POPUP_BOX;

	PopUpBox* const box = CreateRepairBox();

	// PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must remain in synch:
	// CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and HandleShadingOfLinesForRepairMenu().

	if( pSoldier->bSectorZ == 0 )
	{
		// run through list of vehicles and see if any in sector
		CFOR_ALL_VEHICLES(v)
		{
			// don't even list the helicopter, because it's NEVER repairable...
			if (VEHICLE2ID(v) != iHelicopterVehicleId)
			{
				if (IsThisVehicleAccessibleToSoldier(pSoldier, v))
				{
					AddMonoString(box, pVehicleStrings[v->ubVehicleType]);
				}
			}
		}
	}


/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
	// is there a SAM SITE Here?
	if( ( IsThisSectorASAMSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) == TRUE ) && ( IsTheSAMSiteInSectorRepairable( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) ) )
	{
		// SAM site
		AddMonoString(box, pRepairStrings[1]);
	}
*/


	// is the ROBOT here?
	if( IsRobotInThisSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) )
	{
		// robot
		AddMonoString(box, pRepairStrings[3]);
	}


	// items
	AddMonoString(box, pRepairStrings[0]);


	// cancel
	AddMonoString(box, pRepairStrings[2]);

	SetBoxTextAttrs(ghRepairBox);

	// resize box to text
	ResizeBoxToText( ghRepairBox );

	CheckAndUpdateTacticalAssignmentPopUpPositions( );

	return TRUE;
}


static void HandleShadingOfLinesForRepairMenu(void)
{
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iCount = 0;

	if (!fShowRepairMenu || ghRepairBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( FALSE );


	// PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must remain in synch:
	// CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and HandleShadingOfLinesForRepairMenu().

	if ( pSoldier->bSectorZ == 0 )
	{
		CFOR_ALL_VEHICLES(v)
		{
			// don't even list the helicopter, because it's NEVER repairable...
			if (VEHICLE2ID(v) != iHelicopterVehicleId &&
					IsThisVehicleAccessibleToSoldier(pSoldier, v))
			{
				if (CanCharacterRepairVehicle(pSoldier, VEHICLE2ID(v)) == TRUE)
				{
					// unshade vehicle line
					UnShadeStringInBox(ghRepairBox, iCount);
				}
				else
				{
					// shade vehicle line
					ShadeStringInBox(ghRepairBox, iCount);
				}

				iCount++;
			}
		}
	}


/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
	if( ( IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) == TRUE ) && ( IsTheSAMSiteInSectorRepairable( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) ) )
	{
		// handle enable disable of repair sam option
		if( CanSoldierRepairSAM( pSoldier, SAM_SITE_REPAIR_DIVISOR ) )
		{
			// unshade SAM line
			UnShadeStringInBox( ghRepairBox, iCount );
		}
		else
		{
			// shade SAM line
			ShadeStringInBox( ghRepairBox, iCount );
		}

		iCount++;
	}
*/


	if( IsRobotInThisSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) )
	{
		// handle shading of repair robot option
		if( CanCharacterRepairRobot( pSoldier ) )
		{
			// unshade robot line
			UnShadeStringInBox( ghRepairBox, iCount );
		}
		else
		{
			// shade robot line
			ShadeStringInBox( ghRepairBox, iCount );
		}

		iCount++;
	}


	if ( DoesCharacterHaveAnyItemsToRepair( pSoldier, FINAL_REPAIR_PASS ) )
	{
		// unshade items line
		UnShadeStringInBox( ghRepairBox, iCount );
	}
	else
	{
		// shade items line
		ShadeStringInBox( ghRepairBox, iCount );
	}

	iCount++;
}


static void RepairMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RepairMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionForRepairMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	UINT32 uiCounter = 0;
	INT32 iCount = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;
	SOLDIERTYPE *pSoldier = NULL;

	if( ( fShowRepairMenu == TRUE ) && ( fCreated == FALSE ) )
	{
		CheckAndUpdateTacticalAssignmentPopUpPositions( );

		if( ( fShowRepairMenu ) && ( guiCurrentScreen == MAP_SCREEN ) )
		{
			//SetBoxPosition(ghRepairBox, RepairPosition);
		}

		// grab height of font
		iFontHeight = GetLineSpace( ghRepairBox ) + GetFontHeight( GetBoxFont( ghRepairBox ) );

		// get x.y position of box
		GetBoxPosition( ghRepairBox, &pPosition);

		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghRepairBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		pSoldier = GetSelectedAssignSoldier( FALSE );

		// PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must remain in synch:
		// CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and HandleShadingOfLinesForRepairMenu().

		if ( pSoldier->bSectorZ == 0 )
		{
			// vehicles
			CFOR_ALL_VEHICLES(v)
			{
				// don't even list the helicopter, because it's NEVER repairable...
				if (VEHICLE2ID(v) != iHelicopterVehicleId)
				{
					// other vehicles *in the sector* are listed, but later shaded dark if they're not repairable
					if (IsThisVehicleAccessibleToSoldier(pSoldier, v))
					{
						// add mouse region for each line of text..and set user data
						MOUSE_REGION* const r = &gRepairMenuRegion[iCount];
						const UINT16        x = iBoxXPosition;
						const UINT16        y = iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * iCount;
						const UINT16        w = iBoxWidth;
						const UINT16        h = iFontHeight;
						MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

						MSYS_SetRegionUserData(r, 0, iCount);
						// 2nd user data is the vehicle index, which can easily be different from the region index!
						MSYS_SetRegionUserData(r, 1, VEHICLE2ID(v));
						iCount++;
					}
				}
			}
		}


/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
		// SAM site
		if( ( IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) == TRUE ) && ( IsTheSAMSiteInSectorRepairable( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) ) )
		{
			MSYS_DefineRegion(&gRepairMenuRegion[iCount], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * iCount, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * (iCount + 1), MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

			MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 0, REPAIR_MENU_SAM_SITE );
			iCount++;
		}
*/


		// robot
		if( IsRobotInThisSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) )
		{
			MSYS_DefineRegion(&gRepairMenuRegion[iCount], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * iCount, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * (iCount + 1), MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

			MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 0, iCount );
			MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 1, REPAIR_MENU_ROBOT );
			iCount++;
		}


		// items
		MSYS_DefineRegion(&gRepairMenuRegion[iCount], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * iCount, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * (iCount + 1), MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

		MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 0, iCount );
		MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 1, REPAIR_MENU_ITEMS );
		iCount++;


		// cancel
		MSYS_DefineRegion(&gRepairMenuRegion[iCount], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * iCount, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + iFontHeight * (iCount + 1), MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

		MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 0, iCount );
		MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 1, REPAIR_MENU_CANCEL );


		PauseGame( );

		// unhighlight all strings in box
		UnHighLightBox( ghRepairBox );

		fCreated = TRUE;
	}
	else if( ( ( fShowRepairMenu == FALSE ) || ( fShowAssignmentMenu == FALSE ) ) && ( fCreated == TRUE ) )
	{
		fCreated = FALSE;

		// remove these regions
		for( uiCounter = 0; uiCounter < GetNumberOfLinesOfTextInBox( ghRepairBox ); uiCounter++ )
		{
			MSYS_RemoveRegion( &gRepairMenuRegion[ uiCounter ] );
		}

		fShowRepairMenu = FALSE;

		SetRenderFlags( RENDER_FLAG_FULL );

		HideBox( ghRepairBox );

		if ( fShowAssignmentMenu )
		{
			// remove highlight on the parent menu
			UnHighLightBox( ghAssignmentBox );
		}
	}
}


static BOOLEAN AssignMercToAMovementGroup(SOLDIERTYPE* pSoldier);


static void RepairMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iRepairWhat;


	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	// ignore clicks on disabled lines
	if( GetBoxShadeFlag( ghRepairBox, iValue ) == TRUE )
	{
		return;
	}

	// WHAT is being repaired is stored in the second user data argument
	iRepairWhat = MSYS_GetRegionUserData( pRegion, 1 );


	pSoldier = GetSelectedAssignSoldier( FALSE );


	if ( pSoldier && pSoldier->bActive && ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP ) )
	{
		if( ( iRepairWhat >= REPAIR_MENU_VEHICLE1 ) && ( iRepairWhat <= REPAIR_MENU_VEHICLE3 ) )
		{
			// repair VEHICLE

			pSoldier->bOldAssignment = pSoldier->bAssignment;

			if( ( pSoldier->bAssignment != REPAIR )|| ( pSoldier -> fFixingRobot ) || ( pSoldier -> fFixingSAMSite ) )
			{
				SetTimeOfAssignmentChangeForMerc( pSoldier );
			}

			if( pSoldier->bOldAssignment == VEHICLE )
			{
				TakeSoldierOutOfVehicle( pSoldier );
			}

			// remove from squad
			RemoveCharacterFromSquads( pSoldier );
			MakeSureToolKitIsInHand( pSoldier );

			ChangeSoldiersAssignment( pSoldier, REPAIR );

			pSoldier -> bVehicleUnderRepairID = ( INT8 ) iRepairWhat;

			MakeSureToolKitIsInHand( pSoldier );

			// assign to a movement group
			AssignMercToAMovementGroup( pSoldier );

			// set assignment for group
			SetAssignmentForList( ( INT8 ) REPAIR, 0 );
			fShowAssignmentMenu = FALSE;

		}
/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
		else if( iRepairWhat == REPAIR_MENU_SAM_SITE )
		{
			// repair SAM site

			// remove from squad
			RemoveCharacterFromSquads( pSoldier );
			MakeSureToolKitIsInHand( pSoldier );

			if( ( pSoldier->bAssignment != REPAIR )|| ( pSoldier -> fFixingSAMSite == FALSE ) )
			{
				SetTimeOfAssignmentChangeForMerc( pSoldier );
			}

			ChangeSoldiersAssignment( pSoldier, REPAIR );
			pSoldier -> fFixingSAMSite = TRUE;

			// the second argument is irrelevant here, function looks at pSoldier itself to know what's being repaired
			SetAssignmentForList( ( INT8 ) REPAIR, 0 );
			fShowAssignmentMenu = FALSE;

			MakeSureToolKitIsInHand( pSoldier );

			// assign to a movement group
			AssignMercToAMovementGroup( pSoldier );
		}
*/
		else if( iRepairWhat == REPAIR_MENU_ROBOT )
		{
			// repair ROBOT
			pSoldier->bOldAssignment = pSoldier->bAssignment;

			// remove from squad
			if( pSoldier->bOldAssignment == VEHICLE )
			{
				TakeSoldierOutOfVehicle( pSoldier );
			}

			RemoveCharacterFromSquads( pSoldier );
			MakeSureToolKitIsInHand( pSoldier );

			if( ( pSoldier->bAssignment != REPAIR )|| ( pSoldier -> fFixingRobot == FALSE ) )
			{
				SetTimeOfAssignmentChangeForMerc( pSoldier );
			}

			ChangeSoldiersAssignment( pSoldier, REPAIR );
			pSoldier->fFixingRobot = TRUE;

			// the second argument is irrelevant here, function looks at pSoldier itself to know what's being repaired
			SetAssignmentForList( ( INT8 ) REPAIR, 0 );
			fShowAssignmentMenu = FALSE;

			MakeSureToolKitIsInHand( pSoldier );

			// assign to a movement group
			AssignMercToAMovementGroup( pSoldier );
		}
		else if( iRepairWhat == REPAIR_MENU_ITEMS )
		{
			// items
			SetSoldierAssignment( pSoldier, REPAIR, FALSE, FALSE, -1 );

			// the second argument is irrelevant here, function looks at pSoldier itself to know what's being repaired
			SetAssignmentForList( ( INT8 ) REPAIR, 0 );
			fShowAssignmentMenu = FALSE;
		}
		else
		{
			// CANCEL
			fShowRepairMenu = FALSE;
		}

		// update mapscreen
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;

		giAssignHighLine = -1;
	}
}


static void RepairMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if( iValue < REPAIR_MENU_CANCEL )
		{
			if( GetBoxShadeFlag( ghRepairBox, iValue ) == FALSE )
			{
				// highlight choice
				HighLightBoxLine( ghRepairBox, iValue );
			}
		}
		else
		{
			// highlight cancel line
			HighLightBoxLine( ghRepairBox, GetNumberOfLinesOfTextInBox( ghRepairBox ) - 1 );
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghRepairBox );
	}
}


static void MakeSureToolKitIsInHand(SOLDIERTYPE* pSoldier)
{
	INT8 bPocket = 0;

	// if there isn't a toolkit in his hand
	if( pSoldier -> inv[ HANDPOS].usItem != TOOLKIT )
	{
		// run through rest of inventory looking for toolkits, swap the first one into hand if found
		for (bPocket = SECONDHANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
		{
			if( pSoldier -> inv[ bPocket ].usItem == TOOLKIT )
			{
				SwapObjs( &pSoldier -> inv[ HANDPOS ], &pSoldier -> inv[ bPocket ] );
				break;
			}
		}
	}
}


static BOOLEAN MakeSureMedKitIsInHand(SOLDIERTYPE* pSoldier)
{
	INT8 bPocket = 0;

	fTeamPanelDirty = TRUE;

	// if there is a MEDICAL BAG in his hand, we're set
	if ( pSoldier -> inv[ HANDPOS ].usItem == MEDICKIT )
	{
		return(TRUE);
	}

	// run through rest of inventory looking for MEDICAL BAGS, swap the first one into hand if found
	for (bPocket = SECONDHANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
	{
		if ( pSoldier -> inv[ bPocket ].usItem == MEDICKIT )
		{
			fCharacterInfoPanelDirty = TRUE;
			SwapObjs( &pSoldier -> inv[ HANDPOS ], &pSoldier -> inv[ bPocket ] );
			return(TRUE);
		}
	}

	// no medkit items in possession!
	return(FALSE);
}


static void HandleShadingOfLinesForAttributeMenus(void);
static void HandleShadingOfLinesForSquadMenu(void);
static void HandleShadingOfLinesForTrainingMenu(void);


void HandleShadingOfLinesForAssignmentMenus( void )
{
	SOLDIERTYPE *pSoldier = NULL;

	// updates which menus are selectable based on character status

	if (!fShowAssignmentMenu || ghAssignmentBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( FALSE );

	if ( pSoldier && pSoldier->bActive )
	{
		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			// patient
			if( CanCharacterPatient( pSoldier ) )
			{
				// unshade patient line
				UnShadeStringInBox( ghEpcBox, EPC_MENU_PATIENT );
			}
			else
			{
				// shade patient line
				ShadeStringInBox( ghEpcBox, EPC_MENU_PATIENT );
			}


			if( CanCharacterOnDuty( pSoldier ) )
			{
				// unshade on duty line
				UnShadeStringInBox( ghEpcBox, EPC_MENU_ON_DUTY );
			}
			else
			{
				// shade on duty line
				ShadeStringInBox( ghEpcBox, EPC_MENU_ON_DUTY );
			}

			if( CanCharacterVehicle( pSoldier ) )
			{
				// unshade vehicle line
				UnShadeStringInBox( ghEpcBox, EPC_MENU_VEHICLE );
			}
			else
			{
				// shade vehicle line
				ShadeStringInBox( ghEpcBox, EPC_MENU_VEHICLE );
			}
		}
		else
		{
			// doctor
			if( CanCharacterDoctor( pSoldier ) )
			{
				// unshade doctor line
				UnShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_DOCTOR );
				UnSecondaryShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_DOCTOR );
			}
			else
			{
				// only missing a med kit
				if( CanCharacterDoctorButDoesntHaveMedKit( pSoldier ) )
				{
					SecondaryShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_DOCTOR );
				}
				else
				{
					// shade doctor line
					ShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_DOCTOR );
				}

			}

			// repair
			if( CanCharacterRepair( pSoldier ) )
			{
				// unshade repair line
				UnShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_REPAIR );
				UnSecondaryShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_REPAIR );
			}
			else
			{
				// only missing a tool kit
				if( CanCharacterRepairButDoesntHaveARepairkit( pSoldier ) )
				{
					SecondaryShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_REPAIR );
				}
				else
				{
					// shade repair line
					ShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_REPAIR );
				}
			}


			// patient
			if( CanCharacterPatient( pSoldier ) )
			{
				// unshade patient line
				UnShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_PATIENT );
			}
			else
			{
				// shade patient line
				ShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_PATIENT );
			}


			if( CanCharacterOnDuty( pSoldier ) )
			{
				// unshade on duty line
				UnShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_ON_DUTY );
			}
			else
			{
				// shade on duty line
				ShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_ON_DUTY );
			}


			if( CanCharacterPractise( pSoldier ) )
			{
				// unshade train line
				UnShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_TRAIN );
			}
			else
			{
				// shade train line
				ShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_TRAIN );
			}


			if( CanCharacterVehicle( pSoldier ) )
			{
				// unshade vehicle line
				UnShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_VEHICLE );
			}
			else
			{
				// shade vehicle line
				ShadeStringInBox( ghAssignmentBox, ASSIGN_MENU_VEHICLE );
			}
		}
	}

	// squad submenu
	HandleShadingOfLinesForSquadMenu( );

	// vehicle submenu
	HandleShadingOfLinesForVehicleMenu( );

	// repair submenu
	HandleShadingOfLinesForRepairMenu( );

	// training submenu
	HandleShadingOfLinesForTrainingMenu( );

	// training attributes submenu
	HandleShadingOfLinesForAttributeMenus( );
}


static void HideBoxIfShown(PopUpBox* const box)
{
	if (!IsBoxShown(box)) return;

	HideBox(box);
	fTeamPanelDirty     = TRUE;
	gfRenderPBInterface = TRUE;
}


static void HideBoxIfShownMap(PopUpBox* const box)
{
	if (!IsBoxShown(box)) return;

	HideBox(box);
	fTeamPanelDirty     = TRUE;
	fMapPanelDirty      = TRUE;
	gfRenderPBInterface = TRUE;
}


static void CreateDestroyMouseRegionsForTrainingMenu(void);
static void CreateDestroyMouseRegionsForAttributeMenu(void);
static void CreateDestroyMouseRegionsForSquadMenu(BOOLEAN fPositionBox);
static BOOLEAN HandleShowingOfMovementBox(void);


void DetermineWhichAssignmentMenusCanBeShown(void)
{
	BOOLEAN fCharacterNoLongerValid = FALSE;
	SOLDIERTYPE *pSoldier = NULL;

	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		if( fShowMapScreenMovementList == TRUE )
		{
			if( bSelectedDestChar == -1 )
			{
				fCharacterNoLongerValid = TRUE;
				HandleShowingOfMovementBox( );
			}
			else
			{
				fShowMapScreenMovementList = FALSE;
				fCharacterNoLongerValid = TRUE;
			}
		}
		else if( bSelectedAssignChar == -1 )
		{
			fCharacterNoLongerValid = TRUE;
		}

		// update the assignment positions
		UpdateMapScreenAssignmentPositions( );
	}

	// determine which assign menu needs to be shown
	if( ( ( fShowAssignmentMenu == FALSE ) )||( fCharacterNoLongerValid == TRUE ) )
	{
		// reset show assignment menus
		fShowAssignmentMenu = FALSE;
		fShowVehicleMenu = FALSE;
		fShowRepairMenu = FALSE;

		// destroy mask, if needed
		CreateDestroyScreenMaskForAssignmentAndContractMenus( );


		// destroy menu if needed
		CreateDestroyMouseRegionForVehicleMenu( );
		CreateDestroyMouseRegionsForAssignmentMenu( );
		CreateDestroyMouseRegionsForTrainingMenu( );
		CreateDestroyMouseRegionsForAttributeMenu( );
		CreateDestroyMouseRegionsForSquadMenu( TRUE );
		CreateDestroyMouseRegionForRepairMenu( );

		// hide all boxes being shown
		HideBoxIfShown(ghEpcBox);
		HideBoxIfShown(ghAssignmentBox);
		HideBoxIfShown(ghTrainingBox);
		HideBoxIfShown(ghRepairBox);
		HideBoxIfShown(ghAttributeBox);
		HideBoxIfShown(ghVehicleBox);

		// do we really want ot hide this box?
		if (!fShowContractMenu) HideBoxIfShown(ghRemoveMercAssignBox);
		//HideBox( ghSquadBox );


		//SetRenderFlags(RENDER_FLAG_FULL);

		// no menus, leave
		return;
	}

	// update the assignment positions
	UpdateMapScreenAssignmentPositions( );

	// create mask, if needed
	CreateDestroyScreenMaskForAssignmentAndContractMenus( );


	// created assignment menu if needed
	CreateDestroyMouseRegionsForAssignmentMenu( );
	CreateDestroyMouseRegionsForTrainingMenu( );
	CreateDestroyMouseRegionsForAttributeMenu( );
	CreateDestroyMouseRegionsForSquadMenu( TRUE );
	CreateDestroyMouseRegionForRepairMenu(  );

	const SOLDIERTYPE* const s = GetSelectedInfoChar();
	if ((s->bLife == 0 || s->bAssignment == ASSIGNMENT_POW) &&
			guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN)
	{
		// show basic assignment menu
		ShowBox( ghRemoveMercAssignBox );
	}
	else
	{
		pSoldier = GetSelectedAssignSoldier( FALSE );

		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			ShowBox( ghEpcBox );
		}
		else
		{
			// show basic assignment menu

			ShowBox( ghAssignmentBox );
		}
	}

	// TRAINING menu
	if( fShowTrainingMenu == TRUE )
	{
		HandleShadingOfLinesForTrainingMenu( );
		ShowBox( ghTrainingBox );
	}
	else
	{
		HideBoxIfShownMap(ghTrainingBox);
	}

	// REPAIR menu
	if( fShowRepairMenu == TRUE )
	{
		HandleShadingOfLinesForRepairMenu( );
		ShowBox( ghRepairBox );
	}
	else
	{
		HideBoxIfShownMap(ghRepairBox);
	}

	// ATTRIBUTE menu
	if( fShowAttributeMenu == TRUE )
	{
		HandleShadingOfLinesForAttributeMenus( );
		ShowBox( ghAttributeBox );
	}
	else
	{
		HideBoxIfShownMap(ghAttributeBox);
	}

	// VEHICLE menu
	if( fShowVehicleMenu == TRUE )
	{
		ShowBox( ghVehicleBox );
	}
	else
	{
		HideBoxIfShownMap(ghVehicleBox);
	}

	CreateDestroyMouseRegionForVehicleMenu( );
}


static void AssignmentScreenMaskBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


void CreateDestroyScreenMaskForAssignmentAndContractMenus( void )
{

	static BOOLEAN fCreated = FALSE;
	// will create a screen mask to catch mouse input to disable assignment menus

	// not created, create
	if( ( fCreated == FALSE ) && ( ( fShowAssignmentMenu == TRUE )||( fShowContractMenu == TRUE ) || ( fShowTownInfo == TRUE) ) )
	{
		MSYS_DefineRegion(&gAssignmentScreenMaskRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, AssignmentScreenMaskBtnCallback);

		// created
		fCreated = TRUE;

		if ( !(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
		{
			MSYS_ChangeRegionCursor(  &gAssignmentScreenMaskRegion, 0 );
		}

	}
	else if( ( fCreated == TRUE ) && ( fShowAssignmentMenu == FALSE )&&( fShowContractMenu == FALSE ) && ( fShowTownInfo == FALSE )  )
	{
		// created, get rid of it
		MSYS_RemoveRegion(  &gAssignmentScreenMaskRegion );

		// not created
		fCreated = FALSE;
	}
}


static void AssignmentScreenMaskBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment screen mask region

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP ) || ( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP ) )
	{
		if( fFirstClickInAssignmentScreenMask == TRUE )
		{
			fFirstClickInAssignmentScreenMask = FALSE;
			return;
		}

		// button event, stop showing menus
		fShowAssignmentMenu = FALSE;

		fShowVehicleMenu = FALSE;

		fShowContractMenu = FALSE;

		// stop showing town mine info
		fShowTownInfo = FALSE;

		// reset contract character and contract highlight line
		giContractHighLine =-1;
		bSelectedContractChar = -1;
		fGlowContractRegion = FALSE;

		// update mapscreen
		fTeamPanelDirty          = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fMapScreenBottomDirty    = TRUE;
		gfRenderPBInterface      = TRUE;
		SetRenderFlags(RENDER_FLAG_FULL);
	}
}

void ClearScreenMaskForMapScreenExit( void )
{

	// reset show assignment menu
	fShowAssignmentMenu = FALSE;

	// update the assignment positions
	UpdateMapScreenAssignmentPositions( );

	// stop showing town mine info too
	fShowTownInfo = FALSE;

	// destroy mask, if needed
	CreateDestroyScreenMaskForAssignmentAndContractMenus( );

	// destroy assignment menu if needed
	CreateDestroyMouseRegionsForAssignmentMenu( );
	CreateDestroyMouseRegionsForTrainingMenu( );
	CreateDestroyMouseRegionsForAttributeMenu( );
	CreateDestroyMouseRegionsForSquadMenu( TRUE );
	CreateDestroyMouseRegionForRepairMenu(  );
}


static void ContractMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void ContractMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RestorePopUpBoxes(void);


void CreateDestroyMouseRegionsForContractMenu( void )
{
	static BOOLEAN fCreated = FALSE;
	UINT32 iCounter = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;
	static BOOLEAN fShowRemoveMenu = FALSE;

	// will create/destroy mouse regions for the map screen Contract main menu
	// will create/destroy mouse regions for the map screen assignment main menu
	// check if we can only remove character from team..not assign
	if( ( bSelectedContractChar != -1 )|| ( fShowRemoveMenu == TRUE ) )
	{
		if( fShowRemoveMenu == TRUE )
		{
			// dead guy handle menu stuff
			fShowRemoveMenu =  fShowContractMenu;

			// ATE: Added this setting of global variable 'cause
			// it will cause an assert failure in GetSelectedAssignSoldier()
			bSelectedAssignChar = bSelectedContractChar;

			CreateDestroyMouseRegionsForRemoveMenu( );

			return;
		}
		if (gCharactersList[bSelectedContractChar].merc->bLife == 0)
		{
			// dead guy handle menu stuff
			fShowRemoveMenu =  fShowContractMenu;

			// ATE: Added this setting of global variable 'cause
			// it will cause an assert failure in GetSelectedAssignSoldier()
			bSelectedAssignChar = bSelectedContractChar;

			CreateDestroyMouseRegionsForRemoveMenu( );

			return;
		}
	}


	if( ( fShowContractMenu == TRUE ) && ( fCreated == FALSE ) )
	{
		if( bSelectedContractChar == -1 )
		{
			return;
		}

		if( fShowContractMenu )
		{
			SetBoxPosition(ghContractBox, ContractPosition);
		}
		// grab height of font
		iFontHeight = GetLineSpace( ghContractBox ) + GetFontHeight( GetBoxFont( ghContractBox ) );

		// get x.y position of box
		GetBoxPosition( ghContractBox, &pPosition);

		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghContractBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		// define regions
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghContractBox ); iCounter++ )
		{
			// add mouse region for each line of text..and set user data


			MSYS_DefineRegion(&gContractMenuRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghContractBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghContractBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, ContractMenuMvtCallback, ContractMenuBtnCallback);

			// set user defines
			MSYS_SetRegionUserData( &gContractMenuRegion[ iCounter ], 0, iCounter );
		}

		// created
		fCreated = TRUE;

		// pause game
		PauseGame( );

		// unhighlight all strings in box
		UnHighLightBox( ghContractBox );

	}
	else if( ( fShowContractMenu == FALSE ) && ( fCreated == TRUE ) )
	{
		// destroy
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghContractBox ); iCounter++ )
		{
			MSYS_RemoveRegion( &gContractMenuRegion[ iCounter ] );
		}

		fShownContractMenu = FALSE;

		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty= TRUE;
		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;

		RestorePopUpBoxes( );

		// not created
		fCreated = FALSE;
	}
}


static void TrainingMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TrainingMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForTrainingMenu(void)
{
	static BOOLEAN fCreated = FALSE;
	UINT32 iCounter = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;

	// will create/destroy mouse regions for the map screen assignment main menu



	if( ( fShowTrainingMenu == TRUE ) && ( fCreated == FALSE ) )
	{

		if( ( fShowTrainingMenu ) && ( guiCurrentScreen == MAP_SCREEN ) )
		{
			SetBoxPosition(ghTrainingBox, TrainPosition);
		}

		HandleShadingOfLinesForTrainingMenu( );
		CheckAndUpdateTacticalAssignmentPopUpPositions( );

		// grab height of font
		iFontHeight = GetLineSpace( ghTrainingBox ) + GetFontHeight( GetBoxFont( ghTrainingBox ) );

		// get x.y position of box
		GetBoxPosition( ghTrainingBox, &pPosition);

		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghTrainingBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		// define regions
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghTrainingBox ); iCounter++ )
		{
			// add mouse region for each line of text..and set user data


			MSYS_DefineRegion(&gTrainingMenuRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghTrainingBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghTrainingBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 3, MSYS_NO_CURSOR, TrainingMenuMvtCallBack, TrainingMenuBtnCallback);

			// set user defines
			MSYS_SetRegionUserData( &gTrainingMenuRegion[ iCounter ], 0, iCounter );
		}

		// created
		fCreated = TRUE;

		// unhighlight all strings in box
		UnHighLightBox( ghTrainingBox );

	}
	else if( ( ( fShowAssignmentMenu == FALSE ) || ( fShowTrainingMenu == FALSE ) ) && ( fCreated == TRUE ) )
	{
		// destroy
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghTrainingBox ); iCounter++ )
		{
			MSYS_RemoveRegion( &gTrainingMenuRegion[ iCounter ] );
		}

		// stop showing training menu
		if( fShowAssignmentMenu == FALSE )
		{
			fShowTrainingMenu = FALSE;
		}

		RestorePopUpBoxes( );

		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty= TRUE;
		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
		HideBox( ghTrainingBox );
		SetRenderFlags( RENDER_FLAG_FULL );

		// not created
		fCreated = FALSE;

		if ( fShowAssignmentMenu )
		{
			// remove highlight on the parent menu
			UnHighLightBox( ghAssignmentBox );
		}
	}
}


static void AttributeMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void AttributesMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForAttributeMenu(void)
{
	static BOOLEAN fCreated = FALSE;
	UINT32 iCounter = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;

	// will create/destroy mouse regions for the map screen attribute  menu



	if( ( fShowAttributeMenu == TRUE ) && ( fCreated == FALSE ) )
	{

		if( ( fShowAssignmentMenu ) && ( guiCurrentScreen == MAP_SCREEN ) )
		{
			SetBoxPosition(ghAssignmentBox, AssignmentPosition);
		}

		HandleShadingOfLinesForAttributeMenus( );
		CheckAndUpdateTacticalAssignmentPopUpPositions( );

		// grab height of font
		iFontHeight = GetLineSpace( ghAttributeBox ) + GetFontHeight( GetBoxFont( ghAttributeBox ) );

		// get x.y position of box
		GetBoxPosition( ghAttributeBox, &pPosition);

		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghAttributeBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		// define regions
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghAttributeBox ); iCounter++ )
		{
			// add mouse region for each line of text..and set user data


			MSYS_DefineRegion(&gAttributeMenuRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAttributeBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAttributeBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, AttributeMenuMvtCallBack, AttributesMenuBtnCallback);

			// set user defines
			MSYS_SetRegionUserData( &gAttributeMenuRegion[ iCounter ], 0, iCounter );
		}

		// created
		fCreated = TRUE;

		// unhighlight all strings in box
		UnHighLightBox( ghAttributeBox );

	}
	else if( ( ( fShowAssignmentMenu == FALSE ) || ( fShowTrainingMenu == FALSE ) ||( fShowAttributeMenu == FALSE) ) && ( fCreated == TRUE ) )
	{
		// destroy
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghAttributeBox ); iCounter++ )
		{
			MSYS_RemoveRegion( &gAttributeMenuRegion[ iCounter ] );
		}

		// stop showing training menu
		if( ( fShowAssignmentMenu == FALSE )||(  fShowTrainingMenu == FALSE ) || ( fShowAttributeMenu == FALSE))
		{
			fShowAttributeMenu = FALSE;
			gfRenderPBInterface = TRUE;
		}


		RestorePopUpBoxes( );

		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty= TRUE;
		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
		HideBox( ghAttributeBox );
		SetRenderFlags( RENDER_FLAG_FULL );

		// not created
		fCreated = FALSE;

		if ( fShowTrainingMenu )
		{
			// remove highlight on the parent menu
			UnHighLightBox( ghTrainingBox );
		}
	}
}


static void RemoveMercMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void RemoveMercMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForRemoveMenu(void)
{
	static BOOLEAN fCreated = FALSE;
	UINT32 iCounter = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;

	// will create/destroy mouse regions for the map screen attribute  menu
	if( ( ( fShowAssignmentMenu == TRUE ) || ( fShowContractMenu == TRUE ) ) && ( fCreated == FALSE ) )
	{

		if( fShowContractMenu )
		{
			SetBoxPosition(ghContractBox, ContractPosition);
		}
		else
		{
			SetBoxPosition( ghAssignmentBox , AssignmentPosition );
		}

		if( fShowContractMenu )
		{
			// set box position to contract box position
			SetBoxPosition( ghRemoveMercAssignBox , ContractPosition );
		}
		else
		{
			// set box position to contract box position
			SetBoxPosition( ghRemoveMercAssignBox , AssignmentPosition );
		}

		CheckAndUpdateTacticalAssignmentPopUpPositions( );

		// grab height of font
		iFontHeight = GetLineSpace( ghRemoveMercAssignBox ) + GetFontHeight( GetBoxFont( ghRemoveMercAssignBox ) );

		// get x.y position of box
		GetBoxPosition( ghRemoveMercAssignBox, &pPosition);

		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghRemoveMercAssignBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		// define regions
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghRemoveMercAssignBox ); iCounter++ )
		{
			// add mouse region for each line of text..and set user data


			MSYS_DefineRegion(&gRemoveMercAssignRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghAttributeBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghAttributeBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, RemoveMercMenuMvtCallBack, RemoveMercMenuBtnCallback);

			// set user defines
			MSYS_SetRegionUserData( &gRemoveMercAssignRegion[ iCounter ], 0, iCounter );
		}

		// created
		fCreated = TRUE;

		// unhighlight all strings in box
		UnHighLightBox( ghRemoveMercAssignBox );

	}
	else if( ( fShowAssignmentMenu == FALSE ) && ( fCreated == TRUE ) && ( fShowContractMenu == FALSE )  )
	{
		// destroy
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghRemoveMercAssignBox ); iCounter++ )
		{
			MSYS_RemoveRegion( &gRemoveMercAssignRegion[ iCounter ] );
		}

		fShownContractMenu = FALSE;

		// stop showing  menu
		if( fShowRemoveMenu == FALSE )
		{
			fShowAttributeMenu = FALSE;
			fMapPanelDirty = TRUE;
			gfRenderPBInterface = TRUE;
		}


		RestorePopUpBoxes( );

		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty= TRUE;
		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;

		// turn off the GLOBAL fShowRemoveMenu flag!!!
		fShowRemoveMenu = FALSE;
		// and the assignment menu itself!!!
		fShowAssignmentMenu = FALSE;

		// not created
		fCreated = FALSE;
	}


}


static void SquadMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SquadMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void CreateSquadBox(void);


static void CreateDestroyMouseRegionsForSquadMenu(BOOLEAN fPositionBox)
{
	static BOOLEAN fCreated = FALSE;
	UINT32 iCounter = 0;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect pDimensions;


	// will create/destroy mouse regions for the map screen attribute  menu

	if( ( fShowSquadMenu == TRUE ) && ( fCreated == FALSE ) )
	{
		// create squad box
		CreateSquadBox( );
		GetBoxSize( ghAssignmentBox, &pDimensions );

		CheckAndUpdateTacticalAssignmentPopUpPositions( );

		// grab height of font
		iFontHeight = GetLineSpace( ghSquadBox ) + GetFontHeight( GetBoxFont( ghSquadBox ) );

		// get x.y position of box
		GetBoxPosition( ghSquadBox, &pPosition);


		// grab box x and y position
		iBoxXPosition = pPosition.iX;
		iBoxYPosition = pPosition.iY;

		// get dimensions..mostly for width
		GetBoxSize( ghSquadBox, &pDimensions );

		// get width
		iBoxWidth = pDimensions.iRight;

		// define regions
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghSquadBox ) - 1; iCounter++ )
		{
			// add mouse region for each line of text..and set user data
			MSYS_DefineRegion(&gSquadMenuRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghSquadBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghSquadBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, SquadMenuMvtCallBack, SquadMenuBtnCallback);

			MSYS_SetRegionUserData( &gSquadMenuRegion[ iCounter ], 0, iCounter );
		}

		// now create cancel region
		MSYS_DefineRegion(&gSquadMenuRegion[iCounter], iBoxXPosition, iBoxYPosition + GetTopMarginSize(ghSquadBox) + iFontHeight * iCounter, iBoxXPosition + iBoxWidth, iBoxYPosition + GetTopMarginSize(ghSquadBox) + iFontHeight * (iCounter + 1), MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, SquadMenuMvtCallBack, SquadMenuBtnCallback);

		MSYS_SetRegionUserData( &gSquadMenuRegion[ iCounter ], 0, SQUAD_MENU_CANCEL );


		// created
		fCreated = TRUE;

		// show the box
		ShowBox( ghSquadBox );

		// unhighlight all strings in box
		UnHighLightBox( ghSquadBox );

		// update based on current squad
		HandleShadingOfLinesForSquadMenu( );
	}
	else if( ( ( fShowAssignmentMenu == FALSE ) || ( fShowSquadMenu == FALSE ) ) && ( fCreated == TRUE ) )
	{
		// destroy
		for( iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox( ghSquadBox ); iCounter++ )
		{
			MSYS_RemoveRegion( &gSquadMenuRegion[ iCounter ] );
		}

		fShowSquadMenu = FALSE;

		// remove squad box
		RemoveBox(ghSquadBox);
		ghSquadBox = NO_POPUP_BOX;

		RestorePopUpBoxes( );

		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty= TRUE;
		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
		SetRenderFlags( RENDER_FLAG_FULL );

		// not created
		fCreated = FALSE;
		fMapPanelDirty = TRUE;

		if ( fShowAssignmentMenu )
		{
			// remove highlight on the parent menu
			UnHighLightBox( ghAssignmentBox );
		}
	}
}


static BOOLEAN HandleAssignmentExpansionAndHighLightForAssignMenu(SOLDIERTYPE* pSoldier);


static void AssignmentMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE *pSoldier;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );


	pSoldier = GetSelectedAssignSoldier( FALSE );

	if( HandleAssignmentExpansionAndHighLightForAssignMenu( pSoldier ) == TRUE )
	{
		return;
	}

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// is the line shaded?..if so, don't highlight
		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			if( GetBoxShadeFlag( ghEpcBox, iValue ) == FALSE )
			{
				HighLightBoxLine( ghEpcBox, iValue );
			}
		}
		else
		{
			if( GetBoxShadeFlag( ghAssignmentBox, iValue ) == FALSE )
			{
				HighLightBoxLine( ghAssignmentBox, iValue );
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			// unhighlight all strings in box
			UnHighLightBox( ghEpcBox );
		}
		else
		{
			// unhighlight all strings in box
			UnHighLightBox( ghAssignmentBox );
		}
	}
}


static void RemoveMercMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string

		// get the string line handle
		// is the line shaded?..if so, don't highlight
		if( GetBoxShadeFlag( ghRemoveMercAssignBox, iValue ) == FALSE )
		{
			HighLightBoxLine( ghRemoveMercAssignBox, iValue );
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghRemoveMercAssignBox );
	}
}


static void ContractMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for Contract region
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string

		if( iValue != CONTRACT_MENU_CURRENT_FUNDS )
		{
			if( GetBoxShadeFlag( ghContractBox, iValue ) == FALSE )
			{
				// get the string line handle
				HighLightBoxLine( ghContractBox, iValue );
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghContractBox );
	}
}


static void SquadMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string

		if( iValue != SQUAD_MENU_CANCEL )
		{
			if( GetBoxShadeFlag( ghSquadBox, iValue ) == FALSE )
			{
				// get the string line handle
				HighLightBoxLine( ghSquadBox, iValue );
			}
		}
		else
		{
			// highlight cancel line
			HighLightBoxLine(ghSquadBox, GetNumberOfLinesOfTextInBox(ghSquadBox) - 1);
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghSquadBox );

		// update based on current squad
		HandleShadingOfLinesForSquadMenu( );
	}
}


static void RemoveMercMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for contract region
	INT32 iValue = -1;
	SOLDIERTYPE * pSoldier = NULL;


	pSoldier = GetSelectedAssignSoldier( FALSE );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		switch( iValue )
		{
			case REMOVE_MERC_CANCEL:
				// stop showing menus
				fShowAssignmentMenu = FALSE;
				fShowContractMenu = FALSE;

				// reset characters
				bSelectedAssignChar = -1;
				bSelectedContractChar = -1;
				giAssignHighLine = -1;

				// dirty regions
				fCharacterInfoPanelDirty = TRUE;
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;


				// stop contratc glow if we are
				fGlowContractRegion = FALSE;
				giContractHighLine = -1;

				break;
			case( REMOVE_MERC ):
				StrategicRemoveMerc( pSoldier );

				// dirty region
				fCharacterInfoPanelDirty = TRUE;
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;

				// stop contratc glow if we are
				fGlowContractRegion = FALSE;
				giContractHighLine = -1;

				// reset selected characters
				bSelectedAssignChar = -1;
				bSelectedContractChar = -1;
				giAssignHighLine = -1;

				// stop showing menus
				fShowAssignmentMenu = FALSE;
				fShowContractMenu = FALSE;

				//Def: 10/13/99:  When a merc is either dead or a POW, the Remove Merc popup comes up instead of the
				// Assign menu popup.  When the the user removes the merc, we need to make sure the assignment menu
				//doesnt come up ( because the both assign menu and remove menu flags are needed for the remove pop up to appear
				//dont ask why?!! )
				fShownContractMenu = FALSE;
				fShownAssignmentMenu = FALSE;
				fShowRemoveMenu = FALSE;

				break;
		}
	}
}


static void BeginRemoveMercFromContract(SOLDIERTYPE* pSoldier)
{
	// This function will setup the quote, then start dialogue beginning the actual leave sequence
	if( ( pSoldier->bLife > 0 ) && ( pSoldier->bAssignment != ASSIGNMENT_POW ) )
	{
		if( ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC ) || ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC ) )
		{
			HandleImportantMercQuote( pSoldier,  QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO );

			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE,0 ,MAP_SCREEN ,0 ,0 ,0 );
			TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 1,0 );

		}
		else
		// quote is different if he's fired in less than 48 hours
		if( ( GetWorldTotalMin() - pSoldier->uiTimeOfLastContractUpdate ) < 60 * 48 )
		{
			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE,1 ,MAP_SCREEN ,0 ,0 ,0 );
			if( ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ) )
			{
				// Only do this if they want to renew.....
				if ( WillMercRenew( pSoldier, FALSE ) )
				{
					HandleImportantMercQuote( pSoldier, QUOTE_DEPART_COMMET_CONTRACT_NOT_RENEWED_OR_TERMINATED_UNDER_48 );
				}
			}

			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE,0 ,MAP_SCREEN ,0 ,0 ,0 );
			TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 1,0 );

		}
		else
		{
			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE,1 ,MAP_SCREEN ,0 ,0 ,0 );
			if( ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ) )
			{
				// Only do this if they want to renew.....
				if ( WillMercRenew( pSoldier, FALSE ) )
				{
					HandleImportantMercQuote( pSoldier,  QUOTE_DEPARTING_COMMENT_CONTRACT_NOT_RENEWED_OR_48_OR_MORE );
				}
			}
			else if( ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC ) || ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC ) )
			{
				HandleImportantMercQuote( pSoldier,  QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO );
			}

			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE,0 ,MAP_SCREEN ,0 ,0 ,0 );
			TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 1,0 );
		}

		if( ( GetWorldTotalMin() - pSoldier->uiTimeOfLastContractUpdate ) < 60 * 3 )
		{
			// this will cause him give us lame excuses for a while until he gets over it
			// 3-6 days (but the first 1-2 days of that are spent "returning" home)
			gMercProfiles[ pSoldier->ubProfile ].ubDaysOfMoraleHangover = (UINT8) (3 + Random(4));

			// if it's an AIM merc, word of this gets back to AIM...  Bad rep.
			if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC )
			{
				ModifyPlayerReputation(REPUTATION_EARLY_FIRING);
			}
		}

	}
}


static void MercDismissConfirmCallBack(UINT8 bExitValue)
{
	if (bExitValue == MSG_BOX_RETURN_YES)
	{
		// Setup history code
		gpDismissSoldier->ubLeaveHistoryCode = HISTORY_MERC_FIRED;
		BeginRemoveMercFromContract(gpDismissSoldier);
	}
}


static void ContractMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for contract region
	INT32 iValue = -1;
	BOOLEAN fOkToClose = FALSE;

	// can't renew contracts from tactical!
	Assert(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN);

	SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	Assert( pSoldier && pSoldier->bActive );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		fOkToClose = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( GetBoxShadeFlag( ghContractBox, iValue ) == TRUE )
		{
			// not valid
			return;
		}


		if( iValue == CONTRACT_MENU_CANCEL )
		{
			// reset contract character and contract highlight line
			giContractHighLine =-1;
			bSelectedContractChar = -1;
			fGlowContractRegion = FALSE;

			fShowContractMenu = FALSE;
			// dirty region
			fTeamPanelDirty = TRUE;
			fMapScreenBottomDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			gfRenderPBInterface = TRUE;

			if ( gfInContractMenuFromRenewSequence )
			{
				BeginRemoveMercFromContract( pSoldier );
			}
			return;
		}

		// else handle based on contract

		switch( iValue )
		{
			case CONTRACT_MENU_DAY:
				MercContractHandling( pSoldier, CONTRACT_EXTEND_1_DAY );
				fOkToClose = TRUE;
			break;
			case( CONTRACT_MENU_WEEK ):
				MercContractHandling( pSoldier, CONTRACT_EXTEND_1_WEEK );
				fOkToClose = TRUE;
			break;
			case( CONTRACT_MENU_TWO_WEEKS ):
				MercContractHandling( pSoldier, CONTRACT_EXTEND_2_WEEK );
				fOkToClose = TRUE;
			break;

			case( CONTRACT_MENU_TERMINATE ):
				gpDismissSoldier = pSoldier;

				// If in the renewal sequence.. do right away...
				// else put up requester.
				if (gfInContractMenuFromRenewSequence)
				{
					MercDismissConfirmCallBack(MSG_BOX_RETURN_YES);
				}
				else
				{
					DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[48], MAP_SCREEN, MSG_BOX_FLAG_YESNO, MercDismissConfirmCallBack);
				}

				fOkToClose = TRUE;
				break;
		}
	}


	if( fOkToClose == TRUE )
	{
		// reset contract character and contract highlight line
		giContractHighLine =-1;
		bSelectedContractChar = -1;
		fGlowContractRegion = FALSE;
		fShowContractMenu = FALSE;

		// dirty region
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		gfRenderPBInterface      = TRUE;
	}
}


static BOOLEAN HandleAssignmentExpansionAndHighLightForTrainingMenu(void);


static void TrainingMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if( HandleAssignmentExpansionAndHighLightForTrainingMenu( ) == TRUE )
	{
		return;
	}

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string

		// do not highlight current balance
		if( GetBoxShadeFlag( ghTrainingBox, iValue ) == FALSE )
		{
			// get the string line handle
			HighLightBoxLine(ghTrainingBox, iValue);
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghTrainingBox );
	}
}


static void AttributeMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for assignment region
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string
		if( GetBoxShadeFlag( ghAttributeBox, iValue ) == FALSE )
		{
			// get the string line handle
			HighLightBoxLine( ghAttributeBox, iValue );
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghAttributeBox );
	}
}


static void SquadMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE * pSoldier = NULL;
	CHAR16 sString[ 128 ];
	INT8	bCanJoinSquad;

	pSoldier = GetSelectedAssignSoldier( FALSE );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( iValue == SQUAD_MENU_CANCEL )
		{
			// stop displaying, leave
			fShowSquadMenu = FALSE;

			// unhighlight the assignment box
			UnHighLightBox( ghAssignmentBox );

			// dirty region
			fTeamPanelDirty = TRUE;
			fMapScreenBottomDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			gfRenderPBInterface = TRUE;

			return;
		}

		bCanJoinSquad =  CanCharacterSquad( pSoldier, ( INT8 )iValue );
		// can the character join this squad?  (If already in it, accept that as a legal choice and exit menu)
		if (bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD ||
				bCanJoinSquad == CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT)
		{
			if ( bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD )
			{
				// able to add, do it
				pSoldier->bOldAssignment = pSoldier->bAssignment;

				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				AddCharacterToSquad( pSoldier, ( INT8 )iValue );

				if (pSoldier->bOldAssignment == VEHICLE)
				{
					SetSoldierExitVehicleInsertionData(pSoldier, pSoldier->iVehicleId);
				}

				MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );
			}

			// stop displaying, leave
			fShowAssignmentMenu = FALSE;
			giAssignHighLine = -1;

			// dirty region
			fTeamPanelDirty = TRUE;
			fMapScreenBottomDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			gfRenderPBInterface = TRUE;
		}
		else
		{
			BOOLEAN fDisplayError = TRUE;

			switch( bCanJoinSquad )
			{
				case CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING:
					swprintf( sString, lengthof(sString), pMapErrorString[ 36 ], pSoldier->name, pLongAssignmentStrings[ iValue ] );
					break;
				case CHARACTER_CANT_JOIN_SQUAD_VEHICLE:
					swprintf( sString, lengthof(sString), pMapErrorString[ 37 ], pSoldier->name );
					break;
				case CHARACTER_CANT_JOIN_SQUAD_TOO_FAR:
					swprintf( sString, lengthof(sString), pMapErrorString[ 20 ], pSoldier->name, pLongAssignmentStrings[ iValue ] );
					break;
				case CHARACTER_CANT_JOIN_SQUAD_FULL:
					swprintf( sString, lengthof(sString), pMapErrorString[ 19 ], pSoldier->name, pLongAssignmentStrings[ iValue ] );
					break;
				default:
					// generic "you can't join this squad" msg
					swprintf( sString, lengthof(sString), pMapErrorString[ 38 ], pSoldier->name, pLongAssignmentStrings[ iValue ] );
					break;
			}

			if ( fDisplayError )
			{
				DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL);
			}

		}

		// set this assignment for the list too
		SetAssignmentForList( ( INT8 ) iValue, 0 );
	}
}


static void TrainingMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE * pSoldier = NULL;
	INT8 bTownId;
	CHAR16 sString[ 128 ];
	CHAR16 sStringA[ 128 ];


	pSoldier = GetSelectedAssignSoldier( FALSE );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN ) || ( iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN ) )
	{
		if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) && !fShowMapInventoryPool )
		{
			UnMarkButtonDirty( giMapBorderButtons[ MAP_BORDER_TOWN_BTN ] );
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( fShowAttributeMenu )
		{
			// cancel attribute submenu
			fShowAttributeMenu = FALSE;
			// rerender tactical stuff
			gfRenderPBInterface = TRUE;

			return;
		}

		switch( iValue )
		{
			case( TRAIN_MENU_SELF):

				// practise in stat
				gbTrainingMode = TRAIN_SELF;

				// show menu
				fShowAttributeMenu = TRUE;
				DetermineBoxPositions( );

			break;
			case( TRAIN_MENU_TOWN):
				if( BasicCanCharacterTrainMilitia(pSoldier) )
				{
					bTownId = GetTownIdForSector( pSoldier->sSectorX, pSoldier->sSectorY );

					// if it's a town sector (the following 2 errors can't happen at non-town SAM sites)
					if( bTownId != BLANK_SECTOR )
					{
						// can we keep militia in this town?
						if( MilitiaTrainingAllowedInSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) == FALSE )
						{
							swprintf( sString, lengthof(sString), pMapErrorString[ 31 ], pTownNames[ bTownId ] );
							DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
							break;
						}

						// is the current loyalty high enough to train some?
						if( DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia( pSoldier ) == FALSE )
						{
							DoScreenIndependantMessageBox(zMarksMapScreenText[19], MSG_BOX_FLAG_OK, NULL);
							break;
						}
					}

					if (IsAreaFullOfMilitia(pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ))
					{
						if( bTownId == BLANK_SECTOR )
						{
							// SAM site
							GetShortSectorString(  pSoldier->sSectorX, pSoldier->sSectorY, sStringA, lengthof(sStringA));
							swprintf(sString, lengthof(sString), zMarksMapScreenText[20], sStringA);
						}
						else
						{
							// town
							swprintf(sString, lengthof(sString), zMarksMapScreenText[20], pTownNames[bTownId]);
						}

						DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
						break;
					}

					if ( CountMilitiaTrainersInSoldiersSector( pSoldier ) >= MAX_MILITIA_TRAINERS_PER_SECTOR )
					{
						swprintf( sString, lengthof(sString), gzLateLocalizedString[ 47 ], MAX_MILITIA_TRAINERS_PER_SECTOR );
						DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
						break;
					}


					// PASSED ALL THE TESTS - ALLOW SOLDIER TO TRAIN MILITIA HERE

					pSoldier -> bOldAssignment = pSoldier -> bAssignment;

					if( ( pSoldier->bAssignment != TRAIN_TOWN ) )
					{
						SetTimeOfAssignmentChangeForMerc( pSoldier );
					}

					MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

					// stop showing menu
					fShowAssignmentMenu = FALSE;
					giAssignHighLine = -1;

					// remove from squad

					if( pSoldier->bOldAssignment == VEHICLE )
					{
						TakeSoldierOutOfVehicle( pSoldier );
					}
					RemoveCharacterFromSquads(  pSoldier );

					ChangeSoldiersAssignment( pSoldier, TRAIN_TOWN );

					// assign to a movement group
					AssignMercToAMovementGroup( pSoldier );
					if( SectorInfo[ SECTOR( pSoldier -> sSectorX, pSoldier -> sSectorY ) ].fMilitiaTrainingPaid == FALSE )
					{
						// show a message to confirm player wants to charge cost
						HandleInterfaceMessageForCostOfTrainingMilitia( pSoldier );
					}
					else
					{
						SetAssignmentForList( TRAIN_TOWN, 0 );
					}

					gfRenderPBInterface = TRUE;

				}
			break;
			case( TRAIN_MENU_TEAMMATES):

				if( CanCharacterTrainTeammates( pSoldier ) == TRUE )
				{
					// train teammates
					gbTrainingMode = TRAIN_TEAMMATE;

					// show menu
					fShowAttributeMenu = TRUE;
					DetermineBoxPositions( );
				}
			break;
			case( TRAIN_MENU_TRAIN_BY_OTHER ):

				if( CanCharacterBeTrainedByOther( pSoldier ) == TRUE )
				{
					// train teammates
					gbTrainingMode = TRAIN_BY_OTHER;

					// show menu
					fShowAttributeMenu = TRUE;
					DetermineBoxPositions( );
				}
			break;
			case( TRAIN_MENU_CANCEL ):
				// stop showing menu
				fShowTrainingMenu = FALSE;

				// unhighlight the assignment box
				UnHighLightBox(ghAssignmentBox);

				// reset list
				ResetSelectedListForMapScreen();
				gfRenderPBInterface = TRUE;
			break;
		}

		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
	}
	else if( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		if( fShowAttributeMenu )
		{
			// cancel attribute submenu
			fShowAttributeMenu = FALSE;
			// rerender tactical stuff
			gfRenderPBInterface = TRUE;

			return;
		}
	}
}


static void AttributesMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE * pSoldier = NULL;


	pSoldier = GetSelectedAssignSoldier( FALSE );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );


	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( iValue == ATTRIB_MENU_CANCEL )
		{
			// cancel, leave

			// stop showing menu
			fShowAttributeMenu = FALSE;

			// unhighlight the training box
			UnHighLightBox( ghTrainingBox );
		}
		else if( CanCharacterTrainStat( pSoldier, ( INT8 )( iValue ), ( BOOLEAN )( ( gbTrainingMode == TRAIN_SELF ) || ( gbTrainingMode == TRAIN_BY_OTHER ) ), ( BOOLEAN )( gbTrainingMode == TRAIN_TEAMMATE ) ) )
		{
			pSoldier->bOldAssignment = pSoldier->bAssignment;

			if( ( pSoldier->bAssignment != gbTrainingMode ) )
			{
				SetTimeOfAssignmentChangeForMerc( pSoldier );
			}

			// set stat to train
			pSoldier -> bTrainStat = ( INT8 )iValue;

			MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

			// stop showing ALL menus
			fShowAssignmentMenu = FALSE;
			giAssignHighLine = -1;

			// remove from squad/vehicle
			if( pSoldier->bOldAssignment == VEHICLE )
			{
				TakeSoldierOutOfVehicle( pSoldier );
			}
			RemoveCharacterFromSquads( pSoldier );

			// train stat
			ChangeSoldiersAssignment( pSoldier, gbTrainingMode );

			// assign to a movement group
			AssignMercToAMovementGroup( pSoldier );

			// set assignment for group
			SetAssignmentForList( gbTrainingMode, ( INT8 )iValue );
		}

		// rerender tactical stuff
		gfRenderPBInterface = TRUE;

		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
	}
}


static BOOLEAN DisplayVehicleMenu(SOLDIERTYPE* pSoldier);


static void AssignmentMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	CHAR16 sString[ 128 ];

	SOLDIERTYPE * pSoldier = NULL;


	pSoldier = GetSelectedAssignSoldier( FALSE );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );


	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{

		if( ( fShowAttributeMenu )||( fShowTrainingMenu ) || ( fShowRepairMenu ) || ( fShowVehicleMenu ) ||( fShowSquadMenu ) )
		{
			return;
		}

		UnHighLightBox( ghAssignmentBox );

		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			switch( iValue )
			{
				case( EPC_MENU_ON_DUTY ):
					if( CanCharacterOnDuty( pSoldier ) )
						{
							// put character on a team
							fShowSquadMenu = TRUE;
							fShowTrainingMenu = FALSE;
							fShowVehicleMenu = FALSE;
							fTeamPanelDirty = TRUE;
							fMapScreenBottomDirty = TRUE;

						}
				break;
				case( EPC_MENU_PATIENT ):
						// can character doctor?
					if( CanCharacterPatient( pSoldier ) )
					{
						pSoldier->bOldAssignment = pSoldier->bAssignment;

						if( ( pSoldier->bAssignment != PATIENT ) )
						{
							SetTimeOfAssignmentChangeForMerc( pSoldier );
						}

						// stop showing menu
						fShowAssignmentMenu = FALSE;
						giAssignHighLine = -1;

						MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

						// set dirty flag
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;

						// remove from squad

						if ( pSoldier->bOldAssignment == VEHICLE )
						{
							TakeSoldierOutOfVehicle( pSoldier );
						}
						RemoveCharacterFromSquads(  pSoldier );
						ChangeSoldiersAssignment( pSoldier, PATIENT );
						AssignMercToAMovementGroup( pSoldier );

						// set assignment for group
						SetAssignmentForList( ( INT8 ) PATIENT, 0 );
					}
				break;

				case( EPC_MENU_VEHICLE ):
					if ( CanCharacterVehicle( pSoldier ) )
					{
						if( DisplayVehicleMenu( pSoldier ) )
						{
							fShowVehicleMenu = TRUE;
							ShowBox( ghVehicleBox );
						}
						else
						{
							fShowVehicleMenu = FALSE;
						}
					}
				break;

				case( EPC_MENU_REMOVE ):
					fShowAssignmentMenu = FALSE;
					UnEscortEPC(pSoldier);
				break;

				case( EPC_MENU_CANCEL ):
					fShowAssignmentMenu = FALSE;
					giAssignHighLine = -1;

					// set dirty flag
					fTeamPanelDirty = TRUE;
					fMapScreenBottomDirty = TRUE;

					// reset list of characters
					ResetSelectedListForMapScreen( );
				break;
			}
		}
		else
		{
			switch( iValue )
			{
				case( ASSIGN_MENU_ON_DUTY ):
						if( CanCharacterOnDuty( pSoldier ) )
						{
							// put character on a team
							fShowSquadMenu = TRUE;
							fShowTrainingMenu = FALSE;
							fShowVehicleMenu = FALSE;
							fTeamPanelDirty = TRUE;
							fMapScreenBottomDirty = TRUE;
							fShowRepairMenu = FALSE;
						}
				break;
				case( ASSIGN_MENU_DOCTOR ):

					// can character doctor?
					if( CanCharacterDoctor( pSoldier ) )
					{
						// stop showing menu
						fShowAssignmentMenu = FALSE;
						giAssignHighLine = -1;


						pSoldier -> bOldAssignment = pSoldier -> bAssignment;

						if( ( pSoldier->bAssignment != DOCTOR ) )
						{
							SetTimeOfAssignmentChangeForMerc( pSoldier );
						}

						// remove from squad

						if( pSoldier->bOldAssignment == VEHICLE )
						{
							TakeSoldierOutOfVehicle( pSoldier );
						}
						RemoveCharacterFromSquads(  pSoldier );

						ChangeSoldiersAssignment( pSoldier, DOCTOR );

						MakeSureMedKitIsInHand( pSoldier );
						AssignMercToAMovementGroup( pSoldier );

						MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

						// set dirty flag
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;


						// set assignment for group
						SetAssignmentForList( ( INT8 ) DOCTOR, 0 );
					}
					else if( CanCharacterDoctorButDoesntHaveMedKit( pSoldier ) )
					{
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;
						swprintf(sString, lengthof(sString), zMarksMapScreenText[18], pSoldier->name);

						DoScreenIndependantMessageBox( sString , MSG_BOX_FLAG_OK, NULL );
					}

				break;
				case( ASSIGN_MENU_PATIENT ):

					// can character patient?
					if( CanCharacterPatient( pSoldier ) )
					{
						pSoldier -> bOldAssignment = pSoldier -> bAssignment;

						if( ( pSoldier->bAssignment != PATIENT ) )
						{
							SetTimeOfAssignmentChangeForMerc( pSoldier );
						}

						MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

						// stop showing menu
						fShowAssignmentMenu = FALSE;
						giAssignHighLine = -1;

						// set dirty flag
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;

						// remove from squad

						if( pSoldier->bOldAssignment == VEHICLE )
						{
							TakeSoldierOutOfVehicle( pSoldier );
						}
						RemoveCharacterFromSquads(  pSoldier );
						ChangeSoldiersAssignment( pSoldier, PATIENT );

						AssignMercToAMovementGroup( pSoldier );

						// set assignment for group
						SetAssignmentForList( ( INT8 ) PATIENT, 0 );

					}
				break;

				case( ASSIGN_MENU_VEHICLE ):
					if ( CanCharacterVehicle( pSoldier ) )
					{
						if( DisplayVehicleMenu( pSoldier ) )
						{
							fShowVehicleMenu = TRUE;
							ShowBox( ghVehicleBox );
						}
						else
						{
							fShowVehicleMenu = FALSE;
						}
					}
				break;
				case( ASSIGN_MENU_REPAIR ):
					if( CanCharacterRepair( pSoldier ) )
					{

						fShowSquadMenu = FALSE;
						fShowTrainingMenu = FALSE;
						fShowVehicleMenu = FALSE;
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;

						pSoldier -> bOldAssignment = pSoldier -> bAssignment;


						if( pSoldier -> bSectorZ ==0 )
						{
							fShowRepairMenu = FALSE;

							if( DisplayRepairMenu( pSoldier ) )
							{
								fShowRepairMenu = TRUE;
							}

						}

					}
					else if( CanCharacterRepairButDoesntHaveARepairkit( pSoldier ) )
					{
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;
						swprintf(sString, lengthof(sString), zMarksMapScreenText[17], pSoldier->name);

						DoScreenIndependantMessageBox( sString , MSG_BOX_FLAG_OK, NULL );
					}
				break;
				case( ASSIGN_MENU_TRAIN ):
					if( CanCharacterPractise( pSoldier ) )
					{
						fShowTrainingMenu = TRUE;
						DetermineBoxPositions( );
						fShowSquadMenu = FALSE;
						fShowVehicleMenu = FALSE;
						fShowRepairMenu = FALSE;

						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;
					}
				break;
				case( ASSIGN_MENU_CANCEL ):
					fShowAssignmentMenu = FALSE;
					giAssignHighLine = -1;

					// set dirty flag
					fTeamPanelDirty = TRUE;
					fMapScreenBottomDirty = TRUE;

					// reset list of characters
					ResetSelectedListForMapScreen( );
				break;
			}
		}
		gfRenderPBInterface = TRUE;

	}
	else if( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		if( ( fShowAttributeMenu )||( fShowTrainingMenu ) || ( fShowRepairMenu ) || ( fShowVehicleMenu ) ||( fShowSquadMenu ) )
		{
			fShowAttributeMenu = FALSE;
			fShowTrainingMenu = FALSE;
			fShowRepairMenu = FALSE;
			fShowVehicleMenu = FALSE;
			fShowSquadMenu = FALSE;

			// rerender tactical stuff
			gfRenderPBInterface = TRUE;

			// set dirty flag
			fTeamPanelDirty       = TRUE;
			fMapScreenBottomDirty = TRUE;
		}
	}
}


static void RestorePopUpBoxes(void)
{
	ContractPosition.iX   = OrigContractPosition.iX;
	AttributePosition.iX  = OrigAttributePosition.iX;
	SquadPosition.iX      = OrigSquadPosition.iX;
	AssignmentPosition.iX = OrigAssignmentPosition.iX;
	TrainPosition.iX      = OrigTrainPosition.iX;
	VehiclePosition.iX    = OrigVehiclePosition.iX;
}


static PopUpBox* MakeBox(const SGPPoint pos, const UINT32 flags)
{
	PopUpBox* const box = CreatePopUpBox(pos, flags | POPUP_BOX_FLAG_RESIZE);
	SetBoxBuffer(box, FRAME_BUFFER);
	SetBorderType(box, guiPOPUPBORDERS);
	SetBackGroundSurface(box, guiPOPUPTEX);
	SetMargins(box, 6, 6, 4, 4);
	SetLineSpace(box, 2);
	return box;
}


static UINT32 GetLastSquadListedInSquadMenu(void);


static void CreateSquadBox(void)
{
	// will create a pop up box for squad selection
	SGPPoint pPoint;
	SGPRect pDimensions;
	CHAR16 sString[ 64 ];
	UINT32 uiMaxSquad;

	PopUpBox* const box = MakeBox(SquadPosition, 0);
	ghSquadBox = box;

	uiMaxSquad = GetLastSquadListedInSquadMenu();

	// add strings for box
	for (UINT32 uiCounter=0; uiCounter <= uiMaxSquad; ++uiCounter)
	{
		// get info about current squad and put in  string
		swprintf( sString, lengthof(sString), L"%ls ( %d/%d )", pSquadMenuStrings[uiCounter], NumberOfPeopleInSquad( ( INT8 )uiCounter ), NUMBER_OF_SOLDIERS_PER_SQUAD );
		AddMonoString(box, sString);
	}

	// add cancel line
	AddMonoString(box, pSquadMenuStrings[NUMBER_OF_SQUADS]);

	SetBoxTextAttrs(ghSquadBox);

	// resize box to text
	ResizeBoxToText(ghSquadBox);

	DetermineBoxPositions();

	GetBoxPosition(ghSquadBox, &pPoint);
	GetBoxSize(ghSquadBox, &pDimensions);

	if (giBoxY + pDimensions.iBottom >= SCREEN_HEIGHT)
	{
		pPoint.iY = SquadPosition.iY = SCREEN_HEIGHT - 1 - pDimensions.iBottom;
	}

	SetBoxPosition( ghSquadBox, pPoint );
}


static void CreateEPCBox(void)
{
	// will create a pop up box for squad selection
	SGPPoint pPoint;
	SGPRect pDimensions;

	PopUpBox* const box = MakeBox(AssignmentPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghEpcBox = box;

	for (INT32 iCount = 0; iCount < MAX_EPC_MENU_STRING_COUNT; ++iCount)
	{
		AddMonoString(box, pEpcMenuStrings[iCount]);
	}

	SetBoxTextAttrs(ghEpcBox);

	// resize box to text
	ResizeBoxToText(ghEpcBox);

	GetBoxPosition(ghEpcBox, &pPoint);

	GetBoxSize(ghEpcBox, &pDimensions);

	if (giBoxY + pDimensions.iBottom >= SCREEN_HEIGHT)
	{
		pPoint.iY = AssignmentPosition.iY = SCREEN_HEIGHT - 1 - pDimensions.iBottom;
	}

	SetBoxPosition(ghEpcBox, pPoint);
}


static void HandleShadingOfLinesForSquadMenu(void)
{
	// find current squad and set that line the squad box a lighter green
	UINT32 uiCounter;
	SOLDIERTYPE *pSoldier = NULL;
	UINT32 uiMaxSquad;
	INT8 bResult;

	if (!fShowSquadMenu || ghSquadBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( FALSE );

	uiMaxSquad = GetLastSquadListedInSquadMenu();

	for( uiCounter = 0; uiCounter <= uiMaxSquad; uiCounter++ )
	{
		if ( pSoldier != NULL )
		{
			bResult = CanCharacterSquad( pSoldier, (INT8) uiCounter );
		}

		// if no soldier, or a reason which doesn't have a good explanatory message
		if ( ( pSoldier == NULL ) || ( bResult == CHARACTER_CANT_JOIN_SQUAD ) )
		{
			// darken /disable line
			ShadeStringInBox( ghSquadBox, uiCounter );
		}
		else
		{
			if ( bResult == CHARACTER_CAN_JOIN_SQUAD )
			{
				// legal squad, leave it green
				UnShadeStringInBox( ghSquadBox, uiCounter );
				UnSecondaryShadeStringInBox( ghSquadBox, uiCounter );
			}
			else
			{
				// unjoinable squad - yellow
				SecondaryShadeStringInBox( ghSquadBox, uiCounter );
			}
		}
	}
}


static PopUpBox* CreateVehicleBox(void);


static BOOLEAN DisplayVehicleMenu(SOLDIERTYPE* pSoldier)
{
	BOOLEAN fVehiclePresent=FALSE;

	// first, clear pop up box
	RemoveBox(ghVehicleBox);
	ghVehicleBox = NO_POPUP_BOX;

	PopUpBox* const box = CreateVehicleBox();

	// run through list of vehicles in sector and add them to pop up box
	CFOR_ALL_VEHICLES(v)
	{
		if (IsThisVehicleAccessibleToSoldier(pSoldier, v))
		{
			AddMonoString(box, pVehicleStrings[v->ubVehicleType]);
			fVehiclePresent = TRUE;
		}
	}

	// cancel string (borrow the one in the squad menu)
	AddMonoString(box, pSquadMenuStrings[SQUAD_MENU_CANCEL]);

	SetBoxTextAttrs(ghVehicleBox);
	ResizeBoxToText(box);

	return fVehiclePresent;
}


static PopUpBox* CreateVehicleBox(void)
{
	ghVehicleBox = MakeBox(VehiclePosition, POPUP_BOX_FLAG_CENTER_TEXT);
	return ghVehicleBox;
}


static PopUpBox* CreateRepairBox(void)
{
	ghRepairBox = MakeBox(RepairPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	return ghRepairBox;
}


void CreateContractBox(const SOLDIERTYPE* const pCharacter)
{
	wchar_t sString[ 50 ];
	wchar_t sDollarString[ 50 ];

	ContractPosition.iX = OrigContractPosition.iX;

	if( giBoxY != 0 )
	{
		ContractPosition.iX = giBoxY;
	}

	PopUpBox* const box = MakeBox(ContractPosition, 0);
	ghContractBox = box;

	// not null character?
	if( pCharacter != NULL )
	{
		for (UINT32 uiCounter = 0; uiCounter < MAX_CONTRACT_MENU_STRING_COUNT; ++uiCounter)
		{
			switch (uiCounter)
			{
				case CONTRACT_MENU_CURRENT_FUNDS:
/*
					// add current balance after title string
					SPrintMoney(sDollarString, LaptopSaveInfo.iCurrentBalance);
					swprintf(sString, L"%ls %ls", pContractStrings[uiCounter], sDollarString);
					AddMonoString(box, sString);
*/
					AddMonoString(box, pContractStrings[uiCounter]);
					break;

				case CONTRACT_MENU_DAY:
					SPrintMoney(sDollarString, pCharacter->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ? gMercProfiles[pCharacter->ubProfile].sSalary : 0);
					swprintf(sString, lengthof(sString), L"%ls ( %ls )", pContractStrings[uiCounter], sDollarString);
					AddMonoString(box, sString);
					break;

				case CONTRACT_MENU_WEEK:
					SPrintMoney(sDollarString, pCharacter->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ? gMercProfiles[pCharacter->ubProfile].uiWeeklySalary : 0);
					swprintf(sString, lengthof(sString), L"%ls ( %ls )", pContractStrings[uiCounter], sDollarString);
					AddMonoString(box, sString);
					break;

				case CONTRACT_MENU_TWO_WEEKS:
					SPrintMoney(sDollarString, pCharacter->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ? gMercProfiles[pCharacter->ubProfile].uiBiWeeklySalary : 0);
					swprintf(sString, lengthof(sString), L"%ls ( %ls )", pContractStrings[uiCounter], sDollarString);
					AddMonoString(box, sString);
					break;

				default:
					AddMonoString(box, pContractStrings[uiCounter]);
					break;
			}
		}
	}

	SetBoxTextAttrs(ghContractBox);

	if (pCharacter != NULL)
	{
		// now set the color for the current balance value
		SetBoxLineForeground(ghContractBox, 0, FONT_YELLOW);
	}

	// resize box to text
	ResizeBoxToText(ghContractBox);
}


static void CreateAttributeBox(void)
{
	// will create attribute pop up menu for mapscreen assignments
	AttributePosition.iX = OrigAttributePosition.iX;

	if (giBoxY != 0)
	{
		AttributePosition.iY = giBoxY;
	}

	// update screen assignment positions
	UpdateMapScreenAssignmentPositions();

	PopUpBox* const box = MakeBox(AttributePosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghAttributeBox = box;

	// add strings for box
	for (UINT32 uiCounter = 0; uiCounter < MAX_ATTRIBUTE_STRING_COUNT; ++uiCounter)
	{
		AddMonoString(box, pAttributeMenuStrings[uiCounter]);
	}

	SetBoxTextAttrs(ghAttributeBox);

	// resize box to text
	ResizeBoxToText(ghAttributeBox);
}


static void CreateTrainingBox(void)
{
	// will create attribute pop up menu for mapscreen assignments
	TrainPosition.iX = OrigTrainPosition.iX;

	if (giBoxY != 0)
	{
		TrainPosition.iY = giBoxY + ASSIGN_MENU_TRAIN * GetFontHeight(MAP_SCREEN_FONT);
	}

	PopUpBox* const box = MakeBox(TrainPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghTrainingBox = box;

	// add strings for box
	for (UINT32 uiCounter = 0; uiCounter < MAX_TRAIN_STRING_COUNT; ++uiCounter)
	{
		AddMonoString(box, pTrainingMenuStrings[uiCounter]);
	}

	SetBoxTextAttrs(ghTrainingBox);

	// resize box to text
	ResizeBoxToText(ghTrainingBox);

	DetermineBoxPositions();
}


static void CreateAssignmentsBox(void)
{
	// will create attribute pop up menu for mapscreen assignments
	AssignmentPosition.iX = OrigAssignmentPosition.iX;

	if( giBoxY != 0 )
	{
		AssignmentPosition.iY = giBoxY;
	}

	SOLDIERTYPE* const pSoldier = GetSelectedAssignSoldier(TRUE);
	// pSoldier NULL is legal here!  Gets called during every mapscreen initialization even when nobody is assign char

	PopUpBox* const box = MakeBox(AssignmentPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghAssignmentBox = box;

	// add strings for box
	for (UINT32 uiCounter = 0; uiCounter < MAX_ASSIGN_STRING_COUNT; ++uiCounter)
	{
		wchar_t sString[128];
		// if we have a soldier, and this is the squad line
		if( ( uiCounter == ASSIGN_MENU_ON_DUTY ) && ( pSoldier != NULL ) && ( pSoldier->bAssignment < ON_DUTY ) )
		{
			// show his squad # in brackets
			swprintf( sString, lengthof(sString), L"%ls(%d)", pAssignMenuStrings[uiCounter], pSoldier->bAssignment + 1 );
		}
		else
		{
			wcslcpy(sString, pAssignMenuStrings[uiCounter], lengthof(sString));
		}

		AddMonoString(box, sString);
	}

	SetBoxTextAttrs(ghAssignmentBox);

	// resize box to text
	ResizeBoxToText( ghAssignmentBox );

	DetermineBoxPositions( );
}


void CreateMercRemoveAssignBox( void )
{
	// will create remove mercbox to be placed in assignment area
	PopUpBox* const box = MakeBox(AssignmentPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghRemoveMercAssignBox = box;

	// add strings for box
	for (UINT32 uiCounter = 0; uiCounter < MAX_REMOVE_MERC_COUNT; ++uiCounter)
	{
		AddMonoString(box, pRemoveMercStrings[uiCounter]);
	}

	SetBoxTextAttrs(ghRemoveMercAssignBox);

	// resize box to text
	ResizeBoxToText(ghRemoveMercAssignBox);
}


BOOLEAN CreateDestroyAssignmentPopUpBoxes( void )
{
	static BOOLEAN fCreated= FALSE;

	if( ( fShowAssignmentMenu == TRUE ) && ( fCreated == FALSE ) )
	{
		guiPOPUPBORDERS = AddVideoObjectFromFile("INTERFACE/popup.sti");
		CHECKF(guiPOPUPBORDERS != NO_VOBJECT);
		guiPOPUPTEX = AddVideoSurfaceFromFile("INTERFACE/popupbackground.pcx");
		CHECKF(guiPOPUPTEX != NO_VSURFACE);

		// these boxes are always created while in mapscreen...
		CreateEPCBox( );
		CreateAssignmentsBox( );
		CreateTrainingBox( );
		CreateAttributeBox();
		CreateVehicleBox();
		CreateRepairBox( );

		UpdateMapScreenAssignmentPositions( );

		fCreated = TRUE;
	}
	else if( ( fShowAssignmentMenu == FALSE ) && ( fCreated == TRUE ) )
	{
		DeleteVideoObject(guiPOPUPBORDERS);
		DeleteVideoSurface(guiPOPUPTEX);

		RemoveBox(ghAttributeBox);
		ghAttributeBox = NO_POPUP_BOX;

		RemoveBox(ghVehicleBox);
		ghVehicleBox = NO_POPUP_BOX;

		RemoveBox(ghAssignmentBox);
		ghAssignmentBox = NO_POPUP_BOX;

		RemoveBox(ghEpcBox);
		ghEpcBox = NO_POPUP_BOX;

		RemoveBox(ghRepairBox);
		ghRepairBox = NO_POPUP_BOX;

		RemoveBox(ghTrainingBox);
		ghTrainingBox = NO_POPUP_BOX;

		fCreated = FALSE;
		gfIgnoreScrolling = FALSE;
		RebuildCurrentSquad( );
	}

	return( TRUE );
}



void DetermineBoxPositions( void )
{
	// depending on how many boxes there are, reposition as needed
	SGPPoint pPoint;
	SGPPoint pNewPoint;
	SGPRect pDimensions;
	SOLDIERTYPE *pSoldier = NULL;

	if (!fShowAssignmentMenu || ghAssignmentBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( TRUE );
	// pSoldier NULL is legal here!  Gets called during every mapscreen initialization even when nobody is assign char
	if ( pSoldier == NULL )
	{
		return;
	}

	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		GetBoxPosition( ghAssignmentBox, &pPoint );
		gsAssignmentBoxesX = ( INT16 )pPoint.iX;
		gsAssignmentBoxesY = ( INT16 )pPoint.iY;
	}

	pPoint.iX = gsAssignmentBoxesX;
	pPoint.iY = gsAssignmentBoxesY;

	if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
	{
		SetBoxPosition( ghEpcBox, pPoint );
		GetBoxSize( ghEpcBox, &pDimensions );
	}
	else
	{
		SetBoxPosition( ghAssignmentBox, pPoint );
		GetBoxSize( ghAssignmentBox, &pDimensions );
	}


	// hang it right beside the assignment/EPC box menu
	pNewPoint.iX = pPoint.iX + pDimensions.iRight;
	pNewPoint.iY = pPoint.iY;

	if (fShowSquadMenu && ghSquadBox != NO_POPUP_BOX)
	{
		SetBoxPosition( ghSquadBox, pNewPoint );
	}

	if (fShowRepairMenu && ghRepairBox != NO_POPUP_BOX)
	{
		CreateDestroyMouseRegionForRepairMenu( );
		pNewPoint.iY += ( ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_REPAIR );

		SetBoxPosition( ghRepairBox, pNewPoint );
	}

	if (fShowTrainingMenu && ghTrainingBox != NO_POPUP_BOX)
	{
		pNewPoint.iY += ( ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_TRAIN );
		SetBoxPosition( ghTrainingBox, pNewPoint );
		TrainPosition.iX = pNewPoint.iX;
		TrainPosition.iY = pNewPoint.iY;
		OrigTrainPosition.iY = pNewPoint.iY;
		OrigTrainPosition.iX = pNewPoint.iX;

		GetBoxSize( ghTrainingBox, &pDimensions );
		GetBoxPosition( ghTrainingBox, &pPoint );

		if (fShowAttributeMenu && ghAttributeBox != NO_POPUP_BOX)
		{
			// hang it right beside the training box menu
			pNewPoint.iX = pPoint.iX + pDimensions.iRight;
			pNewPoint.iY = pPoint.iY;
			SetBoxPosition( ghAttributeBox, pNewPoint );
		}
	}
}



void SetTacticalPopUpAssignmentBoxXY( void )
{
	INT16 sX, sY;
	SOLDIERTYPE *pSoldier;


	//get the soldier
	pSoldier = GetSelectedAssignSoldier( FALSE );

	// grab soldier's x,y screen position
	GetSoldierScreenPos( pSoldier, &sX, &sY );

	if( sX < 0 )
	{
		sX = 0;
	}

	gsAssignmentBoxesX = sX + 30;

	if( sY < 0 )
	{
		sY = 0;
	}

	gsAssignmentBoxesY = sY;


	// ATE: Check if we are past tactical viewport....
	// Use estimate width's/heights
	if (gsAssignmentBoxesX > SCREEN_WIDTH - 100)
	{
		gsAssignmentBoxesX = SCREEN_WIDTH - 100;
	}

	if ( ( gsAssignmentBoxesY + 130 ) > 320 )
	{
		gsAssignmentBoxesY = 190;
	}
}


static void RepositionMouseRegions(void)
{
	INT16 sDeltaX, sDeltaY;
	INT32 iCounter = 0;

	if( fShowAssignmentMenu == TRUE )
	{
		sDeltaX = gsAssignmentBoxesX - gAssignmentMenuRegion[ 0 ].RegionTopLeftX;
		sDeltaY = ( INT16 ) ( gsAssignmentBoxesY - gAssignmentMenuRegion[ 0 ].RegionTopLeftY + GetTopMarginSize( ghAssignmentBox ) );

		// find the delta from the old to the new, and alter values accordingly
		for( iCounter = 0; iCounter < ( INT32 )GetNumberOfLinesOfTextInBox( ghAssignmentBox ); iCounter++ )
		{
			gAssignmentMenuRegion[ iCounter ].RegionTopLeftX += sDeltaX;
			gAssignmentMenuRegion[ iCounter ].RegionTopLeftY += sDeltaY;

			gAssignmentMenuRegion[ iCounter ].RegionBottomRightX += sDeltaX;
			gAssignmentMenuRegion[ iCounter ].RegionBottomRightY += sDeltaY;
		}

		gfPausedTacticalRenderFlags = RENDER_FLAG_FULL;
	}
}


static void CheckAndUpdateTacticalAssignmentPopUpPositions(void)
{
	SGPRect pDimensions, pDimensions2, pDimensions3;
	SGPPoint pPoint;
	INT16 sLongest;
	SOLDIERTYPE *pSoldier = NULL;


	if( fShowAssignmentMenu == FALSE )
	{
		return;
	}

	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}


	//get the soldier
	pSoldier = GetSelectedAssignSoldier( FALSE );

	if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
	{
		GetBoxSize( ghEpcBox, &pDimensions2 );
	}
	else
	{
		GetBoxSize( ghAssignmentBox, &pDimensions2 );
	}

	if( fShowRepairMenu == TRUE )
	{
		GetBoxSize( ghRepairBox, &pDimensions );

		if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight >= SCREEN_WIDTH)
		{
			gsAssignmentBoxesX = SCREEN_WIDTH - 1 - (pDimensions2.iRight + pDimensions.iRight);
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		if( pDimensions2.iBottom >  pDimensions.iBottom )
		{
			sLongest = pDimensions2.iBottom + (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR;
		}
		else
		{
			sLongest = pDimensions.iBottom  + (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR;
		}

		if( gsAssignmentBoxesY + sLongest >= 360 )
		{
			gsAssignmentBoxesY = ( INT16 )( 359 - ( sLongest ) );
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
		pPoint.iY = gsAssignmentBoxesY + (  ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_REPAIR );

		SetBoxPosition( ghRepairBox, pPoint );
	}
	else if( fShowSquadMenu == TRUE )
	{
		GetBoxSize( ghSquadBox, &pDimensions );


		if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight >= SCREEN_WIDTH)
		{
			gsAssignmentBoxesX = SCREEN_WIDTH - 1 - (pDimensions2.iRight + pDimensions.iRight);
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		if( pDimensions2.iBottom >  pDimensions.iBottom )
		{
			sLongest = ( INT16 )pDimensions2.iBottom;
		}
		else
		{
			sLongest  = ( INT16 )pDimensions.iBottom;
		}

		if( gsAssignmentBoxesY + sLongest >= 360 )
		{
			gsAssignmentBoxesY = ( INT16 )( 359 - ( sLongest ) );
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
		pPoint.iY = gsAssignmentBoxesY;

		SetBoxPosition( ghSquadBox, pPoint );
	}
	else if( fShowAttributeMenu == TRUE )
	{
		GetBoxSize( ghTrainingBox, &pDimensions );
		GetBoxSize( ghAttributeBox, &pDimensions3 );

		if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight + pDimensions3.iRight >= SCREEN_WIDTH)
		{
			gsAssignmentBoxesX = SCREEN_WIDTH - 1 - (pDimensions2.iRight + pDimensions.iRight + pDimensions3.iRight);
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		if( gsAssignmentBoxesY + pDimensions3.iBottom + ( GetFontHeight( MAP_SCREEN_FONT ) * ASSIGN_MENU_TRAIN ) >= 360 )
		{
			gsAssignmentBoxesY = ( INT16 )( 359 - ( pDimensions3.iBottom ) );
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight;
		pPoint.iY = gsAssignmentBoxesY;

		pPoint.iY += (  ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_TRAIN );
		SetBoxPosition( ghAttributeBox, pPoint );

		pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
		pPoint.iY = gsAssignmentBoxesY;

		pPoint.iY += (  ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_TRAIN );

		SetBoxPosition( ghTrainingBox, pPoint );


	}
	else if( fShowTrainingMenu == TRUE )
	{
		GetBoxSize( ghTrainingBox, &pDimensions );

		if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight >= SCREEN_WIDTH)
		{
			gsAssignmentBoxesX = SCREEN_WIDTH - 1 - (pDimensions2.iRight + pDimensions.iRight);
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		if (gsAssignmentBoxesY + pDimensions2.iBottom + (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN >= 360)
		{
			gsAssignmentBoxesY = 359 - pDimensions2.iBottom - (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN;
			SetRenderFlags( RENDER_FLAG_FULL );
		}



		pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
		pPoint.iY = gsAssignmentBoxesY;
		pPoint.iY += ( ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_TRAIN );

		SetBoxPosition( ghTrainingBox, pPoint );
	}
	else
	{
		// just the assignment box
		if (gsAssignmentBoxesX + pDimensions2.iRight >= SCREEN_WIDTH)
		{
			gsAssignmentBoxesX = SCREEN_WIDTH - 1 - pDimensions2.iRight;
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		if( gsAssignmentBoxesY + pDimensions2.iBottom  >= 360 )
		{
			gsAssignmentBoxesY = ( INT16 )( 359 - ( pDimensions2.iBottom  ) );
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		pPoint.iX = gsAssignmentBoxesX;
		pPoint.iY = gsAssignmentBoxesY;

		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC)
		{
			SetBoxPosition( ghEpcBox, pPoint );
		}
		else
		{
			SetBoxPosition( ghAssignmentBox, pPoint );
		}
	}

	RepositionMouseRegions( );
}


static void PositionCursorForTacticalAssignmentBox(void)
{
	// position cursor over y of on duty in tactical assignments
	SGPPoint pPosition;
	SGPRect pDimensions;
	INT32 iFontHeight;

	// get x.y position of box
	GetBoxPosition( ghAssignmentBox, &pPosition);

	// get dimensions..mostly for width
	GetBoxSize( ghAssignmentBox, &pDimensions );

	iFontHeight = GetLineSpace( ghAssignmentBox ) + GetFontHeight( GetBoxFont( ghAssignmentBox ) );

	if( gGameSettings.fOptions[ TOPTION_DONT_MOVE_MOUSE ] == FALSE )
	{
		SimulateMouseMovement( pPosition.iX + pDimensions.iRight - 6, pPosition.iY + ( iFontHeight / 2 ) + 2 );
	}
}


static BOOLEAN CharacterIsTakingItEasy(SOLDIERTYPE* pSoldier);


static void HandleRestFatigueAndSleepStatus(void)
{
	BOOLEAN fReasonAdded = FALSE;
	BOOLEAN fBoxSetUp = FALSE;
	BOOLEAN fMeToo = FALSE;

	// run through all player characters and handle their rest, fatigue, and going to sleep
	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(pSoldier))
		{
			continue;
		}

		if (pSoldier->bAssignment == ASSIGNMENT_POW || pSoldier->bAssignment == IN_TRANSIT)
		{
			continue;
		}

		// if character CAN sleep, he doesn't actually have to be put asleep to get some rest,
		// many other assignments and conditions allow for automatic recovering from fatigue.
		if (CharacterIsTakingItEasy(pSoldier))
		{
			// let them rest some
			RestCharacter(pSoldier);
		}
		else
		{
			// wear them down
			FatigueCharacter(pSoldier);
		}

		// CHECK FOR MERCS GOING TO SLEEP

		// if awake
		if (!pSoldier->fMercAsleep)
		{
			// if dead tired
			if (pSoldier->bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM)
			{
				// if between sectors, don't put tired mercs to sleep...  will be handled when they arrive at the next sector
				if (pSoldier->fBetweenSectors)
				{
					continue;
				}

				// he goes to sleep, provided it's at all possible (it still won't happen in a hostile sector, etc.)
				if (SetMercAsleep(pSoldier, FALSE))
				{
					if (pSoldier->bAssignment < ON_DUTY || pSoldier->bAssignment == VEHICLE)
					{
						// on a squad/vehicle, complain, then drop
						TacticalCharacterDialogue(pSoldier, QUOTE_NEED_SLEEP);
						TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_NEED_SLEEP, DIALOGUE_SPECIAL_EVENT_SLEEP, 1, 0);
						fMeToo = TRUE;
					}

					// guy collapses
					pSoldier->fMercCollapsedFlag = TRUE;
				}
			}
			// if pretty tired, and not forced to stay awake
			else if (pSoldier->bBreathMax < BREATHMAX_PRETTY_TIRED && pSoldier->fForcedToStayAwake == FALSE)
			{
				// if not on squad/ in vehicle
				if (pSoldier->bAssignment >= ON_DUTY && pSoldier->bAssignment != VEHICLE)
				{
					// try to go to sleep on your own
					if (SetMercAsleep(pSoldier, FALSE))
					{
						if (gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION])
						{
							// if the first one
							if (!fReasonAdded)
							{
								// tell player about it
								AddReasonToWaitingListQueue(ASLEEP_GOING_AUTO_FOR_UPDATE);
								TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0, DIALOGUE_SPECIAL_EVENT_SHOW_UPDATE_MENU, 0, 0);

								fReasonAdded = TRUE;
							}

							AddSoldierToWaitingListQueue(pSoldier);
							fBoxSetUp = TRUE;
						}

						// seems unnecessary now?  ARM
						pSoldier->bOldAssignment = pSoldier->bAssignment;
					}
				}
				else	// tired, in a squad / vehicle
				{
					// if he hasn't complained yet
					if (!pSoldier->fComplainedThatTired)
					{
						// say quote
						if (!fMeToo)
						{
							TacticalCharacterDialogue(pSoldier, QUOTE_NEED_SLEEP);
							fMeToo = TRUE;
						}
						else
						{
							TacticalCharacterDialogue(pSoldier, QUOTE_ME_TOO);
						}

						pSoldier->fComplainedThatTired = TRUE;
					}
				}
			}
		}
	}

	if( fBoxSetUp )
	{
		UnPauseGameDuringNextQuote( );
		AddDisplayBoxToWaitingQueue( );
		fBoxSetUp = FALSE;
	}

	fReasonAdded = FALSE;

	// now handle waking (needs seperate list queue, that's why it has its own loop)
	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(pSoldier))
		{
			continue;
		}

		if (pSoldier->bAssignment == ASSIGNMENT_POW || pSoldier->bAssignment == IN_TRANSIT)
		{
			continue;
		}

		// guys between sectors CAN wake up while between sectors (sleeping in vehicle)...

		// CHECK FOR MERCS WAKING UP

		if (pSoldier->bBreathMax >= BREATHMAX_CANCEL_COLLAPSE)
		{
			// reset the collapsed flag well before reaching the wakeup state
			pSoldier->fMercCollapsedFlag = FALSE;
		}

		// if asleep
		if (pSoldier->fMercAsleep)
		{
			// but has had enough rest?
			if (pSoldier->bBreathMax >= BREATHMAX_FULLY_RESTED)
			{
				// try to wake merc up
				if (SetMercAwake(pSoldier, FALSE, FALSE))
				{
					// if not on squad/ in vehicle, tell player about it
					if (pSoldier->bAssignment >= ON_DUTY && pSoldier->bAssignment != VEHICLE)
					{
						if (gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION])
						{
							if (!fReasonAdded)
							{
								AddReasonToWaitingListQueue(ASSIGNMENT_RETURNING_FOR_UPDATE);
								fReasonAdded = TRUE;
							}

							AddSoldierToWaitingListQueue(pSoldier);
							fBoxSetUp = TRUE;
						}
					}
				}
			}
		}
	}

	if( fBoxSetUp )
	{
		UnPauseGameDuringNextQuote( );
		AddDisplayBoxToWaitingQueue( );
		fBoxSetUp = FALSE;
	}
}


// can the character repair this vehicle?
static BOOLEAN CanCharacterRepairVehicle(SOLDIERTYPE* pSoldier, INT32 iVehicleId)
{
	const VEHICLETYPE* const v = GetVehicle(iVehicleId);
	if (v == NULL) return FALSE;

	// is vehicle destroyed?
	if (v->fDestroyed) return FALSE;

	// is it damaged at all?
	if ( !DoesVehicleNeedAnyRepairs( iVehicleId ) )
	{
		return(FALSE);
	}

	// same sector, neither is between sectors, and OK To Use (player owns it) ?
	if (!IsThisVehicleAccessibleToSoldier(pSoldier, v)) return FALSE;

/* Assignment distance limits removed.  Sep/11/98.  ARM
	// if currently loaded sector, are we close enough?
	if( ( pSoldier->sSectorX == gWorldSectorX ) && ( pSoldier->sSectorY == gWorldSectorY ) && ( pSoldier->bSectorZ == gbWorldSectorZ ) )
	{
		if (PythSpacesAway(pSoldier->sGridNo, v->sGridNo) > MAX_DISTANCE_FOR_REPAIR)
		{
			return FALSE;
		}
	}
*/

	return( TRUE );
}


static SOLDIERTYPE* GetRobotSoldier(void);


static BOOLEAN IsRobotInThisSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ)
{
	SOLDIERTYPE *pSoldier;

	pSoldier = GetRobotSoldier( );

	if ( pSoldier != NULL )
	{
		if( ( pSoldier -> sSectorX == sSectorX ) && ( pSoldier -> sSectorY == sSectorY ) && ( pSoldier -> bSectorZ == bSectorZ ) && ( pSoldier -> fBetweenSectors == FALSE ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


static SOLDIERTYPE* GetRobotSoldier(void)
{
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (AM_A_ROBOT(s)) return s;
	}
	return NULL;
}


// can soldier repair robot
static BOOLEAN CanCharacterRepairRobot(SOLDIERTYPE* pSoldier)
{
	SOLDIERTYPE *pRobot = NULL;

	// do we in fact have the robot on the team?
	pRobot = GetRobotSoldier( );
	if( pRobot == NULL )
	{
		return( FALSE );
	}

	// if robot isn't damaged at all
	if( pRobot -> bLife == pRobot -> bLifeMax )
	{
		return( FALSE );
	}

	// is the robot in the same sector
	if( IsRobotInThisSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) == FALSE )
	{
		return( FALSE );
	}

/* Assignment distance limits removed.  Sep/11/98.  ARM
	// if that sector is currently loaded, check distance to robot
	if( ( pSoldier -> sSectorX == gWorldSectorX ) && ( pSoldier -> sSectorY == gWorldSectorY ) && ( pSoldier -> bSectorZ == gbWorldSectorZ ) )
	{
		if( PythSpacesAway( pSoldier -> sGridNo, pRobot -> sGridNo ) > MAX_DISTANCE_FOR_REPAIR )
		{
			return FALSE;
		}
	}
*/

	return( TRUE );
}


static UINT8 RepairRobot(SOLDIERTYPE* pRobot, UINT8 ubRepairPts, BOOLEAN* pfNothingLeftToRepair);


static UINT8 HandleRepairOfRobotBySoldier(SOLDIERTYPE* pSoldier, UINT8 ubRepairPts, BOOLEAN* pfNothingLeftToRepair)
{
	SOLDIERTYPE *pRobot = NULL;

	pRobot = GetRobotSoldier( );

	// do the actual repairs
	return( RepairRobot( pRobot, ubRepairPts, pfNothingLeftToRepair ) );
}


static UINT8 RepairRobot(SOLDIERTYPE* pRobot, UINT8 ubRepairPts, BOOLEAN* pfNothingLeftToRepair)
{
	UINT8 ubPointsUsed = 0;


	// is it "dead" ?
	if( pRobot -> bLife == 0 )
	{
		*pfNothingLeftToRepair = TRUE;
		return( ubPointsUsed );
	}

	// is it "unhurt" ?
	if( pRobot -> bLife == pRobot -> bLifeMax )
	{
		*pfNothingLeftToRepair = TRUE;
		return( ubPointsUsed );
	}

	// if we have enough or more than we need
	if( pRobot -> bLife + ubRepairPts >= pRobot -> bLifeMax )
	{
		ubPointsUsed = ( pRobot -> bLifeMax - pRobot -> bLife );
		pRobot -> bLife = pRobot -> bLifeMax;
	}
	else
	{
		// not enough, do what we can
		ubPointsUsed = ubRepairPts;
		pRobot -> bLife += ubRepairPts;
	}

	if ( pRobot->bLife == pRobot->bLifeMax )
	{
		*pfNothingLeftToRepair = TRUE;
	}
	else
	{
		*pfNothingLeftToRepair = FALSE;
	}

	return( ubPointsUsed );
}


void SetSoldierAssignment( SOLDIERTYPE *pSoldier, INT8 bAssignment, INT32 iParam1, INT32 iParam2, INT32 iParam3 )
{
	switch( bAssignment )
	{
		case( ASSIGNMENT_HOSPITAL ):
			if( CanCharacterPatient( pSoldier ) )
			{

				pSoldier -> bOldAssignment = pSoldier -> bAssignment;
				pSoldier->bBleeding = 0;

				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;

				// remove from squad

				RemoveCharacterFromSquads(  pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != ASSIGNMENT_HOSPITAL ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}


				RebuildCurrentSquad( );

				ChangeSoldiersAssignment( pSoldier, ASSIGNMENT_HOSPITAL );

				AssignMercToAMovementGroup( pSoldier );
			}
			break;
		case( PATIENT ):
			if( CanCharacterPatient( pSoldier ) )
			{
				// set as doctor
				pSoldier -> bOldAssignment = pSoldier -> bAssignment;


				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;

				// remove from squad
				RemoveCharacterFromSquads(  pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != PATIENT ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, PATIENT );

				AssignMercToAMovementGroup( pSoldier );
			}

		break;
		case( DOCTOR ):
			if( CanCharacterDoctor( pSoldier ) )
			{

				pSoldier -> bOldAssignment = pSoldier -> bAssignment;


					// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;

				// remove from squad
				RemoveCharacterFromSquads(  pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != DOCTOR ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, DOCTOR );

				MakeSureMedKitIsInHand( pSoldier );
				AssignMercToAMovementGroup( pSoldier );
			}
		break;
		case( TRAIN_TOWN ):
			if( CanCharacterTrainMilitia( pSoldier ) )
			{
				// train militia
				pSoldier -> bOldAssignment = pSoldier -> bAssignment;

				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;

				// remove from squad
				RemoveCharacterFromSquads(  pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != TRAIN_TOWN ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, TRAIN_TOWN );

				if( pMilitiaTrainerSoldier == NULL )
				{
					if( SectorInfo[ SECTOR( pSoldier -> sSectorX, pSoldier -> sSectorY ) ].fMilitiaTrainingPaid == FALSE )
					{
						// show a message to confirm player wants to charge cost
						HandleInterfaceMessageForCostOfTrainingMilitia( pSoldier );
					}
				}

				AssignMercToAMovementGroup( pSoldier );
				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;
			}
		break;
		case( TRAIN_SELF ):
			if( CanCharacterTrainStat( pSoldier, ( INT8 )iParam1, TRUE, FALSE ) )
			{
				// train stat
				pSoldier -> bOldAssignment = pSoldier -> bAssignment;

				// remove from squad
				RemoveCharacterFromSquads( pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != TRAIN_SELF ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, TRAIN_SELF );

				AssignMercToAMovementGroup( pSoldier );

				// set stat to train
				pSoldier -> bTrainStat = ( INT8 )iParam1;

				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;
			}
			break;
			case( TRAIN_TEAMMATE ):
			if( CanCharacterTrainStat( pSoldier, ( INT8 )iParam1, FALSE, TRUE ) )
			{

				pSoldier -> bOldAssignment = pSoldier -> bAssignment;
				// remove from squad
				RemoveCharacterFromSquads( pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != TRAIN_TEAMMATE ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, TRAIN_TEAMMATE );
				AssignMercToAMovementGroup( pSoldier );

				// set stat to train
				pSoldier -> bTrainStat = ( INT8 )iParam1;

				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;
			}
		break;
		case( TRAIN_BY_OTHER ):
			if( CanCharacterTrainStat( pSoldier, ( INT8 )iParam1, TRUE, FALSE ) )
			{
				// train stat
				pSoldier -> bOldAssignment = pSoldier -> bAssignment;

				// remove from squad
				RemoveCharacterFromSquads( pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != TRAIN_BY_OTHER ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, TRAIN_BY_OTHER );

				AssignMercToAMovementGroup( pSoldier );

				// set stat to train
				pSoldier -> bTrainStat = ( INT8 )iParam1;

				// set dirty flag
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;
			}
			break;
		case( REPAIR ):
			if( CanCharacterRepair( pSoldier ) )
			{
				pSoldier -> bOldAssignment = pSoldier -> bAssignment;

				// remove from squad
				RemoveCharacterFromSquads( pSoldier );

				// remove from any vehicle
				if( pSoldier->bOldAssignment == VEHICLE )
				{
					TakeSoldierOutOfVehicle( pSoldier );
				}

				if( ( pSoldier->bAssignment != REPAIR ) ||( pSoldier ->fFixingSAMSite != ( UINT8 )iParam1 ) || ( pSoldier ->fFixingRobot != ( UINT8 )iParam2 ) || ( pSoldier ->bVehicleUnderRepairID != ( UINT8 )iParam3 ) )
				{
					SetTimeOfAssignmentChangeForMerc( pSoldier );
				}

				ChangeSoldiersAssignment( pSoldier, REPAIR );
				MakeSureToolKitIsInHand( pSoldier );
				AssignMercToAMovementGroup( pSoldier );
				pSoldier -> fFixingSAMSite = ( UINT8 )iParam1;
				pSoldier -> fFixingRobot = ( UINT8 )iParam2;
				pSoldier -> bVehicleUnderRepairID = ( INT8 )iParam3;
			}
			break;

		case( VEHICLE ):
			if (CanCharacterVehicle(pSoldier))
			{
				VEHICLETYPE* const v = GetVehicle(iParam1);
				if (v != NULL && IsThisVehicleAccessibleToSoldier(pSoldier, v))
				{
					if (IsEnoughSpaceInVehicle(v))
					{
						pSoldier -> bOldAssignment = pSoldier -> bAssignment;

						// set dirty flag
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;
						gfRenderPBInterface = TRUE;


						if( pSoldier->bOldAssignment == VEHICLE )
						{
							TakeSoldierOutOfVehicle( pSoldier );
						}

						// remove from squad
						RemoveCharacterFromSquads( pSoldier );

						if (!PutSoldierInVehicle(pSoldier, v))
						{
							AddCharacterToAnySquad( pSoldier );
						}
						else
						{
							if( ( pSoldier->bAssignment != VEHICLE ) || ( pSoldier ->iVehicleId != ( UINT8 )iParam1 ) )
							{
								SetTimeOfAssignmentChangeForMerc( pSoldier );
							}

							pSoldier -> iVehicleId = iParam1;
							ChangeSoldiersAssignment( pSoldier, VEHICLE );
							AssignMercToAMovementGroup( pSoldier );
						}
					}
					else
					{
						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[18], zVehicleName[v->ubVehicleType]);
					}
				}
			}
			break;
	}
}



/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
BOOLEAN CanSoldierRepairSAM( SOLDIERTYPE *pSoldier, INT8 bRepairPoints)
{
	INT16 sGridNoA = 0, sGridNoB = 0;

	// is the soldier in the sector as the SAM
	if( SoldierInSameSectorAsSAM( pSoldier ) == FALSE )
	{
		return( FALSE );
	}

	// is the soldier close enough to the control panel?
	if( IsSoldierCloseEnoughToSAMControlPanel( pSoldier ) == FALSE )
	{
		return( FALSE );
	}

	//can it be fixed?
	if( IsTheSAMSiteInSectorRepairable( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ ) == FALSE )
	{
		return( FALSE );
	}

	// is he good enough?  (Because of the division of repair pts in SAM repair formula, a guy with any less that this
	// can't make any headway
	if (bRepairPoints < SAM_SITE_REPAIR_DIVISOR )
	{
		return( FALSE );
	}

	return( TRUE );
}

BOOLEAN IsTheSAMSiteInSectorRepairable( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	INT32 iCounter = 0;
	INT8 bSAMCondition;


	// is the guy above ground, if not, it can't be fixed, now can it?
	if( sSectorZ != 0 )
	{
		return( FALSE );
	}

	for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
	{
		if( pSamList[ iCounter ] == SECTOR( sSectorX, sSectorY ) )
		{
			bSAMCondition = StrategicMap[ CALCULATE_STRATEGIC_INDEX( sSectorX, sSectorY ) ].bSAMCondition;

			if( ( bSAMCondition < 100 ) && ( bSAMCondition >= MIN_CONDITION_TO_FIX_SAM ) )
			{
				return( TRUE );
			}
			else
			{
				// it's not broken at all, or it's beyond repair
				return( FALSE );
			}
		}
	}

	// none found
	return( FALSE );
}

BOOLEAN SoldierInSameSectorAsSAM( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	// is the soldier on the surface?
	if( pSoldier -> bSectorZ != 0 )
	{
		return( FALSE );
	}

	// now check each sam site in the list
	for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
	{
		if( pSamList[ iCounter] == SECTOR( pSoldier -> sSectorX, pSoldier -> sSectorY ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

BOOLEAN IsSoldierCloseEnoughToSAMControlPanel( SOLDIERTYPE *pSoldier )
{

	INT32 iCounter = 0;

		// now check each sam site in the list
	for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
	{
		if( pSamList[ iCounter ] == SECTOR( pSoldier -> sSectorX, pSoldier -> sSectorY ) )
		{
// Assignment distance limits removed.  Sep/11/98.  ARM
//			if( ( PythSpacesAway( pSamGridNoAList[ iCounter ], pSoldier -> sGridNo ) < MAX_DISTANCE_FOR_REPAIR )||( PythSpacesAway( pSamGridNoBList[ iCounter ], pSoldier -> sGridNo ) < MAX_DISTANCE_FOR_REPAIR ) )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}
*/


static BOOLEAN HandleAssignmentExpansionAndHighLightForAssignMenu(SOLDIERTYPE* pSoldier)
{
	if( fShowSquadMenu )
	{
		// squad menu up?..if so, highlight squad line the previous menu
		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			HighLightBoxLine(ghEpcBox, EPC_MENU_ON_DUTY);
		}
		else
		{
			HighLightBoxLine( ghAssignmentBox, ASSIGN_MENU_ON_DUTY );
		}

		return( TRUE );
	}
	else if( fShowTrainingMenu )
	{
		// highlight train line the previous menu
		HighLightBoxLine( ghAssignmentBox, ASSIGN_MENU_TRAIN );
		return( TRUE );
	}
	else if( fShowRepairMenu )
	{
		// highlight repair line the previous menu
		HighLightBoxLine( ghAssignmentBox, ASSIGN_MENU_REPAIR );
		return( TRUE );
	}
	else if( fShowVehicleMenu )
	{
		// highlight vehicle line the previous menu
		HighLightBoxLine( ghAssignmentBox, ASSIGN_MENU_VEHICLE );
		return( TRUE );
	}

	return( FALSE );
}


static BOOLEAN HandleAssignmentExpansionAndHighLightForTrainingMenu(void)
{
	if( fShowAttributeMenu )
	{
		switch ( gbTrainingMode )
		{
			case TRAIN_SELF:
				HighLightBoxLine( ghTrainingBox, TRAIN_MENU_SELF );
				return( TRUE );
			case TRAIN_TEAMMATE:
				HighLightBoxLine( ghTrainingBox, TRAIN_MENU_TEAMMATES );
				return( TRUE );
			case TRAIN_BY_OTHER:
				HighLightBoxLine( ghTrainingBox, TRAIN_MENU_TRAIN_BY_OTHER );
				return( TRUE );

			default:
				Assert( FALSE );
				break;
		}
	}

	return( FALSE );
}


static BOOLEAN HandleShowingOfMovementBox(void)
{

	// if the list is being shown, then show it
	if( fShowMapScreenMovementList == TRUE )
	{
		MarkAllBoxesAsAltered( );
		ShowBox( ghMoveBox );
		return( TRUE );
	}
	else
	{
		if( IsBoxShown( ghMoveBox ) )
		{
			HideBox( ghMoveBox );
			fMapPanelDirty = TRUE;
			gfRenderPBInterface = TRUE;
			fTeamPanelDirty = TRUE;
			fMapScreenBottomDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
		}
	}

	return( FALSE );
}


static void HandleShadingOfLinesForTrainingMenu(void)
{
	SOLDIERTYPE *pSoldier = NULL;

	// check if valid
	if (!fShowTrainingMenu || ghTrainingBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( FALSE );

	// can character practise?
	if( CanCharacterPractise( pSoldier ) == FALSE )
	{
		ShadeStringInBox( ghTrainingBox, TRAIN_MENU_SELF );
	}
	else
	{
		UnShadeStringInBox( ghTrainingBox, TRAIN_MENU_SELF );
	}


	// can character EVER train militia?
	if( BasicCanCharacterTrainMilitia( pSoldier ) )
	{
		// can he train here, now?
		if( CanCharacterTrainMilitia( pSoldier ) )
		{
			// unshade train militia line
			UnShadeStringInBox( ghTrainingBox, TRAIN_MENU_TOWN );
			UnSecondaryShadeStringInBox( ghTrainingBox, TRAIN_MENU_TOWN );
		}
		else
		{
			SecondaryShadeStringInBox( ghTrainingBox, TRAIN_MENU_TOWN );
		}
	}
	else
	{
		// shade train militia line
		ShadeStringInBox( ghTrainingBox, TRAIN_MENU_TOWN );
	}


	// can character train teammates?
	if( CanCharacterTrainTeammates( pSoldier ) == FALSE )
	{
		ShadeStringInBox( ghTrainingBox, TRAIN_MENU_TEAMMATES );
	}
	else
	{
		UnShadeStringInBox( ghTrainingBox, TRAIN_MENU_TEAMMATES );
	}


	// can character be trained by others?
	if( CanCharacterBeTrainedByOther( pSoldier ) == FALSE )
	{
		ShadeStringInBox( ghTrainingBox, TRAIN_MENU_TRAIN_BY_OTHER );
	}
	else
	{
		UnShadeStringInBox( ghTrainingBox, TRAIN_MENU_TRAIN_BY_OTHER );
	}
}


static void HandleShadingOfLinesForAttributeMenus(void)
{
	// will do the same as updateassignments...but with training pop up box strings
	SOLDIERTYPE *pSoldier;
	INT8 bAttrib =0;
	BOOLEAN fStatTrainable;

	if (!fShowTrainingMenu  || ghTrainingBox  == NO_POPUP_BOX) return;
	if (!fShowAttributeMenu || ghAttributeBox == NO_POPUP_BOX) return;

	pSoldier = GetSelectedAssignSoldier( FALSE );

	for( bAttrib = 0; bAttrib < ATTRIB_MENU_CANCEL; bAttrib++ )
	{
		switch ( gbTrainingMode )
		{
			case TRAIN_SELF:
				fStatTrainable = CanCharacterTrainStat( pSoldier, bAttrib, TRUE, FALSE );
				break;
			case TRAIN_TEAMMATE:
				// DO allow trainers to be assigned without any partners (students)
				fStatTrainable = CanCharacterTrainStat( pSoldier, bAttrib, FALSE, TRUE );
				break;
			case TRAIN_BY_OTHER:
				// DO allow students to be assigned without any partners (trainers)
				fStatTrainable = CanCharacterTrainStat( pSoldier, bAttrib, TRUE, FALSE );
				break;
			default:
				Assert( FALSE );
				fStatTrainable = FALSE;
				break;
		}

		if ( fStatTrainable )
		{
			// also unshade stat
			UnShadeStringInBox( ghAttributeBox, bAttrib );
		}
		else
		{
			// shade stat
			ShadeStringInBox( ghAttributeBox, bAttrib );
		}
	}
}


static BOOLEAN ValidTrainingPartnerInSameSectorOnAssignmentFound(SOLDIERTYPE* pTargetSoldier, INT8 bTargetAssignment, INT8 bTargetStat);


static void ReportTrainersTraineesWithoutPartners(void)
{
	// check for each instructor
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == TRAIN_TEAMMATE &&
				s->bLife > 0 &&
				!ValidTrainingPartnerInSameSectorOnAssignmentFound(s, TRAIN_BY_OTHER, s->bTrainStat))
		{
			AssignmentDone(s, TRUE, TRUE);
		}
	}

	// check each trainee
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == TRAIN_BY_OTHER &&
				s->bLife > 0 &&
				!ValidTrainingPartnerInSameSectorOnAssignmentFound(s, TRAIN_TEAMMATE, s->bTrainStat))
		{
			AssignmentDone(s, TRUE, TRUE);
		}
	}
}


BOOLEAN SetMercAsleep( SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning )
{
	if( CanCharacterSleep( pSoldier, fGiveWarning ) )
	{
		// put him to sleep
		PutMercInAsleepState( pSoldier );

		// successful
		return( TRUE );
	}
	else
	{
		// can't sleep for some other reason
		return( FALSE );
	}
}



BOOLEAN PutMercInAsleepState( SOLDIERTYPE *pSoldier )
{
	if( pSoldier -> fMercAsleep == FALSE )
	{
		if( ( gfWorldLoaded ) && ( pSoldier->bInSector ) )
		{
			if( guiCurrentScreen == GAME_SCREEN )
			{
				ChangeSoldierState( pSoldier, GOTO_SLEEP, 1, TRUE );
			}
			else
			{
				ChangeSoldierState( pSoldier, SLEEPING, 1, TRUE );
			}
		}

		// set merc asleep
		pSoldier -> fMercAsleep = TRUE;

		// refresh panels
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
	}

	return( TRUE );
}


BOOLEAN SetMercAwake( SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning, BOOLEAN fForceHim )
{
	// forcing him skips all normal checks!
	if ( !fForceHim )
	{
		if ( !CanCharacterBeAwakened( pSoldier, fGiveWarning ) )
		{
			return( FALSE );
		}
	}

	PutMercInAwakeState( pSoldier );
	return( TRUE );
}


BOOLEAN PutMercInAwakeState( SOLDIERTYPE *pSoldier )
{
	if ( pSoldier->fMercAsleep )
	{
		if ( ( gfWorldLoaded ) && ( pSoldier->bInSector ) )
		{
			if ( guiCurrentScreen == GAME_SCREEN )
			{
				ChangeSoldierState( pSoldier, WKAEUP_FROM_SLEEP, 1, TRUE );
			}
			else
			{
				ChangeSoldierState( pSoldier, STANDING, 1, TRUE );
			}
		}

		// set merc awake
		pSoldier->fMercAsleep = FALSE;

		// refresh panels
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;

		// determine if merc is being forced to stay awake
		if ( pSoldier->bBreathMax < BREATHMAX_PRETTY_TIRED )
		{
			pSoldier->fForcedToStayAwake = TRUE;
		}
		else
		{
			pSoldier->fForcedToStayAwake = FALSE;
		}
	}

	return( TRUE );
}

BOOLEAN IsThereASoldierInThisSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	if( fSectorsWithSoldiers[  sSectorX + sSectorY * MAP_WORLD_X ][ bSectorZ ] == TRUE )
	{
		return( TRUE );
	}

	return( FALSE );
}


// set the time this soldier's assignment changed
void SetTimeOfAssignmentChangeForMerc( SOLDIERTYPE *pSoldier )
{
	// if someone is being taken off of HOSPITAL then track how much
	// of payment wasn't used up
	if ( pSoldier->bAssignment == ASSIGNMENT_HOSPITAL )
	{
		giHospitalRefund += CalcPatientMedicalCost( pSoldier );
		pSoldier->bHospitalPriceModifier = 0;
	}

	// set time of last assignment change
	pSoldier->uiLastAssignmentChangeMin = GetWorldTotalMin( );

	// assigning new PATIENTs gives a DOCTOR something to do, etc., so set flag to recheck them all.
	// CAN'T DO IT RIGHT AWAY IN HERE 'CAUSE WE TYPICALLY GET CALLED *BEFORE* bAssignment GETS SET TO NEW VALUE!!
	gfReEvaluateEveryonesNothingToDo = TRUE;
}


// have we spent enough time on assignment for it to count?
static BOOLEAN EnoughTimeOnAssignment(const SOLDIERTYPE* const pSoldier)
{
	if( GetWorldTotalMin() - pSoldier->uiLastAssignmentChangeMin >= MINUTES_FOR_ASSIGNMENT_TO_COUNT )
	{
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN AnyMercInGroupCantContinueMoving( GROUP *pGroup )
{
	PLAYERGROUP *pPlayer;
	SOLDIERTYPE *pSoldier;
	BOOLEAN fMeToo = FALSE;
	BOOLEAN fGroupMustStop = FALSE;


	Assert( pGroup );
	Assert( pGroup->fPlayer );

	pPlayer = pGroup->pPlayerList;

	while( pPlayer )
	{
		// if group has player list...  and a valid first soldier
		if( pPlayer && pPlayer->pSoldier )
		{
			pSoldier = pPlayer->pSoldier;

			if ( PlayerSoldierTooTiredToTravel( pSoldier ) )
			{
				// NOTE: we only complain about it if it's gonna force the group to stop moving!
				fGroupMustStop = TRUE;

				// say quote
				if( fMeToo == FALSE )
				{
					HandleImportantMercQuote( pSoldier, QUOTE_NEED_SLEEP );
					fMeToo = TRUE;
				}
				else
				{
					HandleImportantMercQuote( pSoldier, QUOTE_ME_TOO );
				}

				// put him to bed
				PutMercInAsleepState( pSoldier );

				// player can't wake him up right away
				pSoldier->fMercCollapsedFlag = TRUE;
			}
		}

		pPlayer = pPlayer->next;
	}

	return( fGroupMustStop );
}


BOOLEAN PlayerSoldierTooTiredToTravel(SOLDIERTYPE* pSoldier)
{
	Assert( pSoldier );

	// if this guy ever needs sleep at all
	if ( CanChangeSleepStatusForSoldier( pSoldier ) )
	{
		// if walking, or only remaining possible driver for a vehicle group
		if ( ( pSoldier->bAssignment != VEHICLE ) || SoldierMustDriveVehicle( pSoldier, pSoldier->iVehicleId, TRUE ) )
		{
			// if awake, but so tired they can't move/drive anymore
			if ( ( !pSoldier->fMercAsleep ) && ( pSoldier->bBreathMax < BREATHMAX_GOTTA_STOP_MOVING ) )
			{
				return( TRUE );
			}

			// asleep, and can't be awakened?
			if ( ( pSoldier->fMercAsleep ) && !CanCharacterBeAwakened( pSoldier, FALSE ) )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


static BOOLEAN AssignMercToAMovementGroup(SOLDIERTYPE* pSoldier)
{
	// if merc doesn't have a group or is in a vehicle or on a squad assign to group
	INT8 bGroupId = 0;

	// on a squad?
	if( pSoldier->bAssignment < ON_DUTY )
	{
		return( FALSE );
	}

	// in a vehicle?
	if( pSoldier->bAssignment == VEHICLE )
	{
		return( FALSE );
	}

	// in transit
	if( pSoldier->bAssignment == IN_TRANSIT )
	{
		return( FALSE );
	}

	// in a movement group?
	if( pSoldier->ubGroupID != 0 )
	{
		return( FALSE );
	}

	// create group
	bGroupId = CreateNewPlayerGroupDepartingFromSector( ( UINT8 )( pSoldier->sSectorX ), ( UINT8 )( pSoldier->sSectorY ) );

	if( bGroupId )
	{
		// add merc
		AddPlayerToGroup( bGroupId, pSoldier );

		// success
		return( TRUE );
	}

	return( TRUE );
}


// notify player of assignment attempt failure
static void NotifyPlayerOfAssignmentAttemptFailure(INT8 bAssignment)
{
	// notify player
	if ( guiCurrentScreen != MSG_BOX_SCREEN )
	{
		DoScreenIndependantMessageBox( pMapErrorString[ 18 ], MSG_BOX_FLAG_OK, NULL);
	}
	else
	{
		// use screen msg instead!
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMapErrorString[ 18 ] );
	}

	if ( bAssignment == TRAIN_TOWN )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMapErrorString[ 48 ] );
	}
}


BOOLEAN HandleSelectedMercsBeingPutAsleep( BOOLEAN fWakeUp, BOOLEAN fDisplayWarning )
{
	BOOLEAN fSuccess = TRUE;
	INT32 iCounter = 0;
	UINT8 ubNumberOfSelectedSoldiers = 0;

	const SOLDIERTYPE* const sel_char = GetSelectedInfoChar();
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		// if the current character in the list is valid...then grab soldier pointer for the character
		SOLDIERTYPE* const pSoldier = gCharactersList[iCounter].merc;
		if (pSoldier == NULL) continue;
		if (!pSoldier->bActive) continue;
		if (pSoldier == sel_char) continue;
		if (!IsEntryInSelectedListSet(iCounter)) continue;

		// don't try to put vehicles, robots, to sleep if they're also selected
		if (!CanChangeSleepStatusForSoldier(pSoldier)) continue;

		// up the total number of soldiers
		ubNumberOfSelectedSoldiers++;

		if (fWakeUp)
		{
			// try to wake merc up
			if (!SetMercAwake(pSoldier, FALSE, FALSE)) fSuccess = FALSE;
		}
		else
		{
			// set this soldier asleep
			if (!SetMercAsleep(pSoldier, FALSE)) fSuccess = FALSE;
		}
	}

	// if there was anyone processed, check for success and inform player of failure
	if( ubNumberOfSelectedSoldiers )
	{
		if( fSuccess == FALSE )
		{
			const wchar_t* WarningMsg;
			if( fWakeUp )
			{
				// inform player not everyone could be woke up
				WarningMsg = pMapErrorString[27];
			}
			else
			{
				// inform player not everyone could be put to sleep
				WarningMsg = pMapErrorString[26];
			}

			if( fDisplayWarning )
			{
				DoScreenIndependantMessageBox(WarningMsg, MSG_BOX_FLAG_OK, NULL);
			}
		}
	}

	return( fSuccess );
}


BOOLEAN IsAnyOneOnPlayersTeamOnThisAssignment( INT8 bAssignment )
{
	CFOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == bAssignment) return TRUE;
	}
	return FALSE;
}


void RebuildAssignmentsBox( void )
{
	// destroy and recreate assignments box
	if (ghAssignmentBox != NO_POPUP_BOX)
	{
		RemoveBox( ghAssignmentBox );
		ghAssignmentBox = NO_POPUP_BOX;
	}

	CreateAssignmentsBox( );
}



void BandageBleedingDyingPatientsBeingTreated( )
{
	SOLDIERTYPE *pDoctor = NULL;
	INT32 iKitSlot;
	OBJECTTYPE *pKit = NULL;
	UINT16 usKitPts;
	UINT32 uiKitPtsUsed;
	BOOLEAN fSomeoneStillBleedingDying = FALSE;

	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// and he is bleeding or dying
		if( ( pSoldier->bBleeding ) || ( pSoldier->bLife < OKLIFE ) )
		{
			// if soldier is receiving care
			if( ( pSoldier->bAssignment == PATIENT ) || ( pSoldier->bAssignment == ASSIGNMENT_HOSPITAL ) || ( pSoldier->bAssignment == DOCTOR ) )
			{
				// if in the hospital
				if ( pSoldier->bAssignment == ASSIGNMENT_HOSPITAL )
				{
					// this is instantaneous, and doesn't use up any bandages!

					// stop bleeding automatically
					pSoldier->bBleeding = 0;

					if ( pSoldier->bLife < OKLIFE )
					{
						pSoldier->bLife = OKLIFE;
					}
				}
				else	// assigned to DOCTOR/PATIENT
				{
					// see if there's a doctor around who can help him
					pDoctor = AnyDoctorWhoCanHealThisPatient( pSoldier, HEALABLE_EVER );
					if ( pDoctor != NULL )
					{
						iKitSlot = FindObjClass( pDoctor, IC_MEDKIT );
						if( iKitSlot != NO_SLOT )
						{
							pKit = &( pDoctor->inv[ iKitSlot ] );

							usKitPts = TotalPoints( pKit );
							if( usKitPts )
							{
								uiKitPtsUsed = VirtualSoldierDressWound( pDoctor, pSoldier, pKit, usKitPts, usKitPts );
								UseKitPoints( pKit, (UINT16)uiKitPtsUsed, pDoctor );

								// if he is STILL bleeding or dying
								if( ( pSoldier->bBleeding ) || ( pSoldier->bLife < OKLIFE ) )
								{
									fSomeoneStillBleedingDying = TRUE;
								}
							}
						}
					}
				}
			}
		}
	}


	// this event may be posted many times because of multiple assignment changes.  Handle it only once per minute!
	DeleteAllStrategicEventsOfType( EVENT_BANDAGE_BLEEDING_MERCS );

	if ( fSomeoneStillBleedingDying )
	{
		AddStrategicEvent( EVENT_BANDAGE_BLEEDING_MERCS, GetWorldTotalMin() + 1, 0 );
	}
}



void ReEvaluateEveryonesNothingToDo()
{
	BOOLEAN fNothingToDo;

	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		switch (pSoldier->bAssignment)
		{
			case DOCTOR:
				fNothingToDo = !CanCharacterDoctor(pSoldier) || GetNumberThatCanBeDoctored(pSoldier, HEALABLE_EVER, FALSE, FALSE) == 0;
				break;

			case PATIENT:
				fNothingToDo = !CanCharacterPatient(pSoldier) || AnyDoctorWhoCanHealThisPatient(pSoldier, HEALABLE_EVER) == NULL;
				break;

			case ASSIGNMENT_HOSPITAL:
				fNothingToDo = !CanCharacterPatient(pSoldier);
				break;

			case REPAIR:
				fNothingToDo = !CanCharacterRepair(pSoldier) || HasCharacterFinishedRepairing(pSoldier);
				break;

			case TRAIN_TOWN:
				fNothingToDo = !CanCharacterTrainMilitia(pSoldier);
				break;

			case TRAIN_SELF:
				fNothingToDo = !CanCharacterTrainStat(pSoldier, pSoldier->bTrainStat, TRUE, FALSE);
				break;

			case TRAIN_TEAMMATE:
				fNothingToDo = !CanCharacterTrainStat(pSoldier, pSoldier->bTrainStat, FALSE, TRUE) ||
				               !ValidTrainingPartnerInSameSectorOnAssignmentFound(pSoldier, TRAIN_BY_OTHER, pSoldier->bTrainStat);
				break;

			case TRAIN_BY_OTHER:
				fNothingToDo = !CanCharacterTrainStat(pSoldier, pSoldier->bTrainStat, TRUE, FALSE) ||
				               !ValidTrainingPartnerInSameSectorOnAssignmentFound(pSoldier, TRAIN_TEAMMATE, pSoldier->bTrainStat);
				break;

			case VEHICLE:
			default:	// squads
				fNothingToDo = FALSE;
				break;
		}

		// if his flag is wrong
		if (fNothingToDo != pSoldier->fDoneAssignmentAndNothingToDoFlag)
		{
			// update it!
			pSoldier->fDoneAssignmentAndNothingToDoFlag = fNothingToDo;

			// update mapscreen's character list display
			fDrawCharacterList = TRUE;
		}

		// if he now has something to do, reset the quote flag
		if (!fNothingToDo)
		{
			pSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;
		}
	}

	// re-evaluation completed
	gfReEvaluateEveryonesNothingToDo = FALSE;


	// redraw the map, in case we're showing teams, and someone just came on duty or off duty, their icon needs updating
	fMapPanelDirty = TRUE;
}



void SetAssignmentForList( INT8 bAssignment, INT8 bParam )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSelectedSoldier = NULL;
	BOOLEAN fItWorked;
	BOOLEAN fRemoveFromSquad = TRUE;
	BOOLEAN fNotifiedOfFailure = FALSE;
	INT8 bCanJoinSquad;

	// if not in mapscreen, there is no functionality available to change multiple assignments simultaneously!
	if ( !( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}

	// pSelectedSoldier is currently used only for REPAIR, and this block of code is copied from RepairMenuBtnCallback()
	if( bSelectedAssignChar != -1 )
	{
		pSelectedSoldier = gCharactersList[bSelectedAssignChar].merc;
	}

	Assert( pSelectedSoldier && pSelectedSoldier->bActive );


	// sets assignment for the list
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const MapScreenCharacterSt* const c        = &gCharactersList[iCounter];
		SOLDIERTYPE* const                pSoldier = c->merc;
		if (pSoldier != NULL &&
				c->selected &&
				( iCounter != bSelectedAssignChar ) &&
				!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE))
		{
			// assume it's NOT gonna work
			fItWorked = FALSE;

			switch( bAssignment )
			{
				case( DOCTOR ):
					// can character doctor?
					if( CanCharacterDoctor( pSoldier ) )
					{
						// set as doctor
						pSoldier->bOldAssignment = pSoldier->bAssignment;
						SetSoldierAssignment( pSoldier, DOCTOR, 0,0,0 );
						fItWorked = TRUE;
					}
					break;
				case( PATIENT ):
					// can character patient?
					if( CanCharacterPatient( pSoldier ) )
					{
						// set as patient
						pSoldier->bOldAssignment = pSoldier->bAssignment;
						SetSoldierAssignment( pSoldier, PATIENT, 0,0,0 );
						fItWorked = TRUE;
					}
					break;
				case( VEHICLE ):
					if (CanCharacterVehicle(pSoldier))
					{
						VEHICLETYPE* const v = GetVehicle(pSoldier->bVehicleID);
						if (v != NULL && IsThisVehicleAccessibleToSoldier(pSoldier, v))
						{
							// if the vehicle is FULL, then this will return FALSE!
							fItWorked = PutSoldierInVehicle(pSoldier, v);
							// failure produces its own error popup
							fNotifiedOfFailure = TRUE;
						}
					}
					break;

				case( REPAIR ):
					if( CanCharacterRepair( pSoldier ) )
					{
						BOOLEAN fCanFixSpecificTarget = TRUE;

						// make sure he can repair the SPECIFIC thing being repaired too (must be in its sector, for example)

/*
						if ( pSelectedSoldier->fFixingSAMSite )
						{
							fCanFixSpecificTarget = CanSoldierRepairSAM( pSoldier, SAM_SITE_REPAIR_DIVISOR );
						}
						else
*/
						if ( pSelectedSoldier->bVehicleUnderRepairID != -1 )
						{
							fCanFixSpecificTarget = CanCharacterRepairVehicle( pSoldier, pSelectedSoldier->bVehicleUnderRepairID );
						}
						else if( pSoldier->fFixingRobot )
						{
							fCanFixSpecificTarget = CanCharacterRepairRobot( pSoldier );
						}

						if ( fCanFixSpecificTarget )
						{
							// set as repair
							pSoldier->bOldAssignment = pSoldier->bAssignment;
							SetSoldierAssignment( pSoldier, REPAIR, pSelectedSoldier->fFixingSAMSite, pSelectedSoldier->fFixingRobot, pSelectedSoldier->bVehicleUnderRepairID );
							fItWorked = TRUE;
						}
					}
					break;
				case( TRAIN_SELF ):
					if( CanCharacterTrainStat( pSoldier, bParam , TRUE, FALSE ) )
					{
						pSoldier->bOldAssignment = pSoldier->bAssignment;
						SetSoldierAssignment( pSoldier, TRAIN_SELF, bParam, 0,0 );
						fItWorked = TRUE;
					}
					break;
				case( TRAIN_TOWN ):
					if( CanCharacterTrainMilitia( pSoldier ) )
					{
						pSoldier->bOldAssignment = pSoldier->bAssignment;
						SetSoldierAssignment( pSoldier, TRAIN_TOWN, 0, 0, 0 );
						fItWorked = TRUE;
					}
					break;
				case( TRAIN_TEAMMATE ):
					if( CanCharacterTrainStat( pSoldier, bParam, FALSE, TRUE ) )
					{
						pSoldier->bOldAssignment = pSoldier->bAssignment;
						SetSoldierAssignment( pSoldier, TRAIN_TEAMMATE, bParam, 0,0 );
						fItWorked = TRUE;
					}
					break;
				case TRAIN_BY_OTHER:
					if( CanCharacterTrainStat( pSoldier, bParam, TRUE, FALSE ) )
					{
						pSoldier->bOldAssignment = pSoldier->bAssignment;
						SetSoldierAssignment( pSoldier, TRAIN_BY_OTHER, bParam, 0,0 );
						fItWorked = TRUE;
					}
					break;

				case( SQUAD_1 ):
				case( SQUAD_2 ):
				case( SQUAD_3 ):
				case( SQUAD_4 ):
				case( SQUAD_5 ):
				case( SQUAD_6 ):
				case( SQUAD_7 ):
				case( SQUAD_8 ):
				case( SQUAD_9 ):
				case( SQUAD_10 ):
				case( SQUAD_11 ):
				case( SQUAD_12 ):
				case( SQUAD_13 ):
				case( SQUAD_14 ):
				case( SQUAD_15 ):
				case( SQUAD_16 ):
				case( SQUAD_17 ):
				case( SQUAD_18 ):
				case( SQUAD_19 ):
				case( SQUAD_20 ):
					bCanJoinSquad = CanCharacterSquad( pSoldier, ( INT8 )bAssignment );

					// if already in it, don't repor that as an error...
					if (bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD ||
							bCanJoinSquad == CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT)
					{
						if ( bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD )
						{
							pSoldier->bOldAssignment = pSoldier->bAssignment;

							// is the squad between sectors
							if( Squad[ bAssignment ][ 0 ] )
							{
								if( Squad[ bAssignment ][ 0 ]->fBetweenSectors )
								{
									// between sectors, remove from old mvt group
									if ( pSoldier->bOldAssignment >= ON_DUTY )
									{
										// remove from group
										// the guy wasn't in a sqaud, but moving through a sector?
										if ( pSoldier->ubGroupID != 0 )
										{
											// now remove from mvt group
											RemovePlayerFromGroup( pSoldier->ubGroupID, pSoldier );
										}
									}
								}
							}

							if( pSoldier->bOldAssignment == VEHICLE )
							{
								TakeSoldierOutOfVehicle( pSoldier );
							}
							// remove from current squad, if any
							RemoveCharacterFromSquads( pSoldier );

							// able to add, do it
							AddCharacterToSquad( pSoldier, bAssignment );
						}

						fItWorked = TRUE;
						fRemoveFromSquad = FALSE;	// already done, would screw it up!
					}
					break;

				default:
					// remove from current vehicle/squad, if any
					if( pSoldier->bAssignment == VEHICLE )
					{
						TakeSoldierOutOfVehicle( pSoldier );
					}
					RemoveCharacterFromSquads( pSoldier );

					AddCharacterToAnySquad( pSoldier );

					fItWorked = TRUE;
					fRemoveFromSquad = FALSE;	// already done, would screw it up!
					break;
			}

			if ( fItWorked )
			{
				if ( fRemoveFromSquad )
				{
					// remove him from his old squad if he was on one
					RemoveCharacterFromSquads( pSoldier );
				}

				MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );
			}
			else
			{
				// didn't work - report it once
				if( !fNotifiedOfFailure )
				{
					fNotifiedOfFailure = TRUE;
					NotifyPlayerOfAssignmentAttemptFailure( bAssignment );
				}
			}
		}
	}
	// reset list
//	ResetSelectedListForMapScreen( );


	// check if we should start/stop flashing any mercs' assignment strings after these changes
	gfReEvaluateEveryonesNothingToDo = TRUE;
}


static BOOLEAN ValidTrainingPartnerInSameSectorOnAssignmentFound(SOLDIERTYPE* pTargetSoldier, INT8 bTargetAssignment, INT8 bTargetStat)
{
	INT16 sTrainingPts = 0;
	BOOLEAN fAtGunRange = FALSE;
	UINT16 usMaxPts;

	// this function only makes sense for training teammates or by others, not for self training which doesn't require partners
	Assert( ( bTargetAssignment == TRAIN_TEAMMATE ) || ( bTargetAssignment == TRAIN_BY_OTHER ) );

	CFOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// if the guy is not the target, has the assignment we want, is training the same stat, and is in our sector, alive
		// and is training the stat we want
		if (pSoldier != pTargetSoldier &&
				pSoldier->bAssignment == bTargetAssignment &&
				// CJC: this seems incorrect in light of the check for bTargetStat and in any case would
				// cause a problem if the trainer was assigned and we weren't!
				//pSoldier->bTrainStat == pTargetSoldier->bTrainStat &&
				pSoldier->sSectorX == pTargetSoldier->sSectorX &&
				pSoldier->sSectorY == pTargetSoldier->sSectorY &&
				pSoldier->bSectorZ == pTargetSoldier->bSectorZ &&
				pSoldier->bTrainStat == bTargetStat &&
				pSoldier->bLife > 0)
		{
			// so far so good, now let's see if the trainer can really teach the student anything new

			// are we training in the sector with gun range in Alma?
			if (pSoldier->sSectorX == GUN_RANGE_X && pSoldier->sSectorY == GUN_RANGE_Y && pSoldier->bSectorZ == GUN_RANGE_Z)
			{
				fAtGunRange = TRUE;
			}

			if (pSoldier->bAssignment == TRAIN_TEAMMATE)
			{
				// pSoldier is the instructor, target is the student
				sTrainingPts = GetBonusTrainingPtsDueToInstructor(pSoldier, pTargetSoldier, bTargetStat, fAtGunRange, &usMaxPts);
			}
			else
			{
				// target is the instructor, pSoldier is the student
				sTrainingPts = GetBonusTrainingPtsDueToInstructor(pTargetSoldier, pSoldier, bTargetStat, fAtGunRange, &usMaxPts);
			}

			if (sTrainingPts > 0)
			{
				// yes, then he makes a valid training partner for us!
				return TRUE;
			}
		}
	}

	// no one found
	return( FALSE );
}


static void InternalUnescortEPC(SOLDIERTYPE* const s)
{
	SetupProfileInsertionDataForSoldier(s);

	const ProfileID profile = s->ubProfile;
	UINT16 quote_num;
	UINT16 fact_to_set_to_true;
	if (GetInfoForAbandoningEPC(profile, &quote_num, &fact_to_set_to_true))
	{
		gMercProfiles[profile].ubMiscFlags |= PROFILE_MISC_FLAG_FORCENPCQUOTE;
		TacticalCharacterDialogue(s, quote_num);
		SetFactTrue(fact_to_set_to_true);
	}
	SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_REMOVE_EPC, profile, 0, 0, 0, 0);
}


void UnEscortEPC(SOLDIERTYPE* const s)
{
	if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN)
	{
		InternalUnescortEPC(s);

		SOLDIERTYPE* other;
		switch (s->ubProfile)
		{
			case JOHN: other = FindSoldierByProfileID(MARY, TRUE); break;
			case MARY: other = FindSoldierByProfileID(JOHN, TRUE); break;
			default:   other = NULL;                               break;
		}
		if (other != NULL) InternalUnescortEPC(other);

		// stop showing menu
		giAssignHighLine = -1;

		// set dirty flag
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
	else
	{
		// how do we handle this if it's the right sector?
		TriggerNPCWithGivenApproach(s->ubProfile, APPROACH_EPC_IN_WRONG_SECTOR, TRUE);
	}
}


static BOOLEAN CharacterIsTakingItEasy(SOLDIERTYPE* pSoldier)
{
	// actually asleep?
	if ( pSoldier -> fMercAsleep == TRUE )
	{
		return( TRUE );
	}

	// if able to sleep
	if ( CanCharacterSleep( pSoldier, FALSE ) )
	{
		// on duty, but able to catch naps (either not traveling, or not the driver of the vehicle)
		// The actual checks for this are in the "can he sleep" check above
		if ( ( pSoldier -> bAssignment < ON_DUTY ) || ( pSoldier -> bAssignment == VEHICLE ) )
		{
			return( TRUE );
		}

		// and healing up?
		if ( ( pSoldier -> bAssignment == PATIENT ) || ( pSoldier -> bAssignment == ASSIGNMENT_HOSPITAL ) )
		{
			return( TRUE );
		}

		// on a real assignment, but done with it?
		if ( pSoldier -> fDoneAssignmentAndNothingToDoFlag )
		{
			return( TRUE );
		}
	}


	// on assignment, or walking/driving & unable to sleep
	return( FALSE );
}


static UINT8 CalcSoldierNeedForSleep(SOLDIERTYPE* pSoldier)
{
	UINT8 ubNeedForSleep;
	UINT8 ubPercentHealth;


	// base comes from profile
	ubNeedForSleep = gMercProfiles[ pSoldier -> ubProfile ].ubNeedForSleep;


	ubPercentHealth = pSoldier->bLife / pSoldier->bLifeMax;

	if ( ubPercentHealth < 75 )
	{
		ubNeedForSleep++;

		if ( ubPercentHealth < 50 )
		{
			ubNeedForSleep++;

			if ( ubPercentHealth < 25 )
			{
				ubNeedForSleep += 2;
			}
		}
	}

	// reduce for each Night Ops or Martial Arts trait
	ubNeedForSleep -= NUM_SKILL_TRAITS( pSoldier, NIGHTOPS );
	ubNeedForSleep -= NUM_SKILL_TRAITS( pSoldier, MARTIALARTS );

	if ( ubNeedForSleep < 4 )
	{
		ubNeedForSleep = 4;
	}

	if ( ubNeedForSleep > 12 )
	{
		ubNeedForSleep = 12;
	}

	return( ubNeedForSleep );
}


static UINT32 GetLastSquadListedInSquadMenu(void)
{
	UINT32 uiMaxSquad;

	uiMaxSquad = GetLastSquadActive( ) + 1;

	if ( uiMaxSquad >= NUMBER_OF_SQUADS )
	{
		uiMaxSquad = NUMBER_OF_SQUADS - 1;
	}

	return( uiMaxSquad );
}


static BOOLEAN CanCharacterRepairAnotherSoldiersStuff(const SOLDIERTYPE* const pSoldier, const SOLDIERTYPE* const pOtherSoldier)
{
	if ( pOtherSoldier == pSoldier )
	{
		return( FALSE );
	}
	if ( pOtherSoldier->bLife == 0 )
	{
		return( FALSE );
	}
	if (pOtherSoldier->sSectorX != pSoldier->sSectorX ||
			pOtherSoldier->sSectorY != pSoldier->sSectorY ||
			pOtherSoldier->bSectorZ != pSoldier->bSectorZ )
	{
		return( FALSE );
	}

	if ( pOtherSoldier->fBetweenSectors )
	{
		return( FALSE );
	}

	if (pOtherSoldier->bAssignment == IN_TRANSIT ||
			pOtherSoldier->bAssignment == ASSIGNMENT_POW ||
			pOtherSoldier->bAssignment == ASSIGNMENT_DEAD ||
			pSoldier->uiStatusFlags & SOLDIER_VEHICLE ||
			AM_A_ROBOT(pSoldier) ||
			pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC)
	{
		return( FALSE );
	}

	return( TRUE );
}


static SOLDIERTYPE* GetSelectedAssignSoldier(BOOLEAN fNullOK)
{
	SOLDIERTYPE *pSoldier = NULL;

	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		// mapscreen version
		if (bSelectedAssignChar >= 0 && bSelectedAssignChar < MAX_CHARACTER_COUNT)
		{
			pSoldier = gCharactersList[bSelectedAssignChar].merc;
		}
	}
	else
	{
		// tactical version
		pSoldier = gUIFullTarget;
	}

	if ( !fNullOK )
	{
		Assert( pSoldier );
	}

	if ( pSoldier != NULL )
	{
		// better be an active person, not a vehicle
		Assert( pSoldier->bActive );
		Assert( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) );
	}

	return( pSoldier );
}


void ResumeOldAssignment(SOLDIERTYPE* pSoldier)
{
	BOOLEAN fOldAssignmentInvalid = FALSE;


	// ARM: I don't think the whole "old assignment" idea is a very good one, and I doubt the code that maintains that
	// variable is very foolproof, plus what meaning does the old assignemnt have later, anyway?
	// so I'd rather just settle for putting him into any squad:
	fOldAssignmentInvalid = TRUE;

/*
	if ( pSoldier->bOldAssignment == pSoldier->bAssigment )
	{
		// no good: we rely on this to make sure guys training militia STOP training militia!
		fOldAssignmentInvalid = TRUE;
	}
	else if( pSoldier->bOldAssignment == VEHICLE )
	{
		SetSoldierAssignment( pSoldier, ( INT8 )( pSoldier->bOldAssignment ), ( pSoldier->iVehicleId ), 0, 0 );

		// it might not work - check
		if ( pSoldier->bAssignment != VEHICLE )
		{
			fOldAssignmentInvalid = TRUE;
		}
	}
	else if( pSoldier->bOldAssignment < ON_DUTY )
	{
		if( AddCharacterToSquad( pSoldier, pSoldier->bOldAssignment ) == FALSE )
		{
			fOldAssignmentInvalid = TRUE;
		}
	}
	else
	{
		fOldAssignmentInvalid = TRUE;
	}
*/

	if ( fOldAssignmentInvalid )
	{
		AddCharacterToAnySquad( pSoldier );
	}


	// make sure the player has time to OK this before proceeding
	StopTimeCompression();

	// assignment has changed, redraw left side as well as the map (to update on/off duty icons)
	fTeamPanelDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;
	fMapPanelDirty = TRUE;
}


static void RepairItemsOnOthers(SOLDIERTYPE* pSoldier, UINT8* pubRepairPtsLeft)
{
	UINT8 ubPassType;
	INT8 bPocket;
	SOLDIERTYPE * pBestOtherSoldier;
	INT8 bPriority, bBestPriority = -1;
	BOOLEAN fSomethingWasRepairedThisPass;


	// repair everyone's hands and armor slots first, then headgear, and pockets last
	for ( ubPassType = REPAIR_HANDS_AND_ARMOR; ubPassType <= FINAL_REPAIR_PASS; ubPassType++ )
	{
		fSomethingWasRepairedThisPass = FALSE;


		// look for jammed guns on other soldiers in sector and unjam them
		FOR_ALL_IN_TEAM(pOtherSoldier, gbPlayerNum)
		{
			// check character is valid, alive, same sector, not between, has inventory, etc.
			if ( CanCharacterRepairAnotherSoldiersStuff( pSoldier, pOtherSoldier ) )
			{
				if ( UnjamGunsOnSoldier( pOtherSoldier, pSoldier, pubRepairPtsLeft ) )
				{
					fSomethingWasRepairedThisPass = TRUE;
				}
			}
		}


		while ( *pubRepairPtsLeft > 0 )
		{
			bBestPriority = -1;
			pBestOtherSoldier = NULL;

			// now look for items to repair on other mercs
			FOR_ALL_IN_TEAM(pOtherSoldier, gbPlayerNum)
			{
				// check character is valid, alive, same sector, not between, has inventory, etc.
				if ( CanCharacterRepairAnotherSoldiersStuff( pSoldier, pOtherSoldier ) )
				{
					// okay, seems like a candidate!
					if ( FindRepairableItemOnOtherSoldier( pOtherSoldier, ubPassType ) != NO_SLOT )
					{
						bPriority = pOtherSoldier->bExpLevel;
						if ( bPriority > bBestPriority )
						{
							bBestPriority = bPriority;
							pBestOtherSoldier = pOtherSoldier;
						}
					}
				}
			}

			// did we find anyone to repair on this pass?
			if ( pBestOtherSoldier != NULL )
			{
				// yes, repair all items (for this pass type!) on this soldier that need repair
				do
				{
					bPocket = FindRepairableItemOnOtherSoldier( pBestOtherSoldier, ubPassType );
					if ( bPocket != NO_SLOT )
					{
						if ( RepairObject( pSoldier, pBestOtherSoldier, &(pBestOtherSoldier->inv[ bPocket ]), pubRepairPtsLeft ) )
						{
							fSomethingWasRepairedThisPass = TRUE;
						}
					}
				}
				while ( bPocket != NO_SLOT && *pubRepairPtsLeft > 0 );
			}
			else
			{
				break;
			}
		}

		if ( fSomethingWasRepairedThisPass && !DoesCharacterHaveAnyItemsToRepair( pSoldier, ubPassType ) )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sRepairsDoneString[ 1 + ubPassType ], pSoldier->name );

			// let player react
			StopTimeCompression();
		}
	}
}


static BOOLEAN UnjamGunsOnSoldier(SOLDIERTYPE* pOwnerSoldier, SOLDIERTYPE* pRepairSoldier, UINT8* pubRepairPtsLeft)
{
	BOOLEAN fAnyGunsWereUnjammed = FALSE;
	INT8	bPocket;


	// try to unjam everything before beginning any actual repairs.. successful unjamming costs 2 points per weapon
	for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
	{
		// the object a weapon? and jammed?
		if ( ( Item[ pOwnerSoldier->inv[ bPocket ].usItem ].usItemClass == IC_GUN ) && ( pOwnerSoldier->inv[ bPocket ].bGunAmmoStatus < 0 ) )
		{
			if ( *pubRepairPtsLeft >= REPAIR_COST_PER_JAM )
			{
				*pubRepairPtsLeft -= REPAIR_COST_PER_JAM;

				pOwnerSoldier->inv [ bPocket ].bGunAmmoStatus *= -1;

				// MECHANICAL/DEXTERITY GAIN: Unjammed a gun
				StatChange( pRepairSoldier, MECHANAMT, 5, FALSE );
				StatChange( pRepairSoldier, DEXTAMT, 5, FALSE );

				// report it as unjammed
				if ( pRepairSoldier == pOwnerSoldier )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 53 ], pRepairSoldier->name, ItemNames[ pOwnerSoldier->inv[ bPocket ].usItem ] );
				}
				else
				{
					// NOTE: may need to be changed for localized versions
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ 54 ], pRepairSoldier->name, pOwnerSoldier->name, ItemNames[ pOwnerSoldier->inv[ bPocket ].usItem ] );
				}

				fAnyGunsWereUnjammed = TRUE;
			}
			else
			{
				// out of points, we're done for now
				break;
			}
		}
	}

	return ( fAnyGunsWereUnjammed );
}
