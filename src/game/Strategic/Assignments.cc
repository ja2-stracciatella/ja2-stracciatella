#include "Assignments.h"
#include "Animation_Control.h"
#include "Auto_Resolve.h"
#include "Button_System.h"
#include "Campaign.h"
#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Facts.h"
#include "Finances.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Handle_UI.h"
#include "History.h"
#include "Input.h"
#include "Interface.h"
#include "Interface_Dialogue.h"
#include "Isometric_Utils.h"
#include "Item_Types.h"
#include "ItemModel.h"
#include "Items.h"
#include "JA2Types.h"
#include "JAScreens.h"
#include "Logger.h"
#include "Map_Information.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "MapScreen.h"
#include "Merc_Contract.h"
#include "MercProfile.h"
#include "Message.h"
#include "MessageBoxScreen.h"
#include "MouseSystem.h"
#include "NPC.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "PopUpBox.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Random.h"
#include "RenderWorld.h"
#include "SAM_Sites.h"
#include "ScreenIDs.h"
#include "SGPStrings.h"
#include "SkillCheck.h"
#include "Soldier_Add.h"
#include "Soldier_Control.h"
#include "Soldier_Find.h"
#include "Soldier_Macros.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"
#include "Squads.h"
#include "Strategic_Event_Handler.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"
#include "Strategic_Status.h"
#include "Strategic_Town_Loyalty.h"
#include "StrategicMap.h"
#include "Text.h"
#include "Town_Militia.h"
#include "UILayout.h"
#include "Vehicles.h"
#include "VObject.h"
#include "VSurface.h"
#include <algorithm>
#include <iterator>
#include <string_theory/format>
#include <string_theory/string>
struct PopUpBox;


// various reason an assignment can be aborted before completion
enum AssignmentAbortReason
{
	NO_MORE_MED_KITS   = STR_LATE_40,
	INSUF_DOCTOR_SKILL = STR_LATE_41,
	NO_MORE_TOOL_KITS  = STR_LATE_42,
	INSUF_REPAIR_SKILL = STR_LATE_43,

	NUM_ASSIGN_ABORT_REASONS
};

enum{
	REPAIR_MENU_VEHICLE1 = 0,
	REPAIR_MENU_VEHICLE2,
	REPAIR_MENU_VEHICLE3,
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


struct REPAIR_PASS_SLOTS_TYPE
{
	UINT8		ubChoices;						// how many valid choices there are in this pass
	INT8		bSlot[ 12 ];					// list of slots to be repaired in this pass
};


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
static PopUpBox* ghVehicleBox;
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

BOOLEAN fShownAssignmentMenu    = FALSE;
static BOOLEAN fShowVehicleMenu = FALSE;
BOOLEAN fShowRepairMenu         = FALSE;
BOOLEAN fShownContractMenu      = FALSE;

BOOLEAN fFirstClickInAssignmentScreenMask = FALSE;

// we are in fact training?..then who temmates, or self?
static INT8 gbTrainingMode = -1;

static BOOLEAN gfAddDisplayBoxToWaitingQueue = FALSE;

static SOLDIERTYPE* gpDismissSoldier = NULL;

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

// a list of which sectors have characters
static BOOLEAN fSectorsWithSoldiers[MAP_WORLD_X * MAP_WORLD_Y][4];


void InitSectorsWithSoldiersList( void )
{
	// init list of sectors
	for (auto& i : fSectorsWithSoldiers)
	{
		std::fill(std::begin(i), std::end(i), 0);
	}
}


void BuildSectorsWithSoldiersList( void )
{
	// fills array with pressence of player controlled characters
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
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

	if (bAssignment == ASSIGNMENT_DEAD)
	{
		// life checks should agree with the assignment
		pSoldier->bLife = 0;
	}
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


static BOOLEAN IsSoldierInHelicopterInHostileSector(SOLDIERTYPE const& s)
{
	return InHelicopter(s) && NumEnemiesInSector(s.sSectorX, s.sSectorY) > 0;
}


/* Which conditions are allowed to perform an assignment? */
enum AssignmentConditions
{
	AC_NONE                      = 0,
	AC_IMPASSABLE                = 1U << 0,
	AC_UNCONSCIOUS               = 1U << 1,
	AC_COMBAT                    = 1U << 2,
	AC_EPC                       = 1U << 3,
	AC_IN_HELI_IN_HOSTILE_SECTOR = 1U << 4,
	AC_MECHANICAL                = 1U << 5,
	AC_MOVING                    = 1U << 6,
	AC_UNDERGROUND               = 1U << 7
};
ENUM_BITSET(AssignmentConditions)


static bool AreAssignmentConditionsMet(SOLDIERTYPE const& s, AssignmentConditions const c)
{
	return
		(c & AC_IMPASSABLE  || SectorIsPassable(SECTOR(s.sSectorX, s.sSectorY)))       &&
		(c & AC_UNCONSCIOUS || s.bLife >= OKLIFE)                                      &&
		(c & AC_COMBAT      || !s.bInSector || !gTacticalStatus.fEnemyInSector)        &&
		(c & AC_EPC         || s.ubWhatKindOfMercAmI != MERC_TYPE__EPC)                &&
		(c & AC_IN_HELI_IN_HOSTILE_SECTOR || !IsSoldierInHelicopterInHostileSector(s)) &&
		(c & AC_MECHANICAL  || !IsMechanical(s))                                       &&
		(c & AC_MOVING      || !s.fBetweenSectors)                                     &&
		(c & AC_UNDERGROUND || s.bSectorZ == 0)                                        &&
		!IsCharacterInTransit(s)                                                       &&
		s.bAssignment != ASSIGNMENT_POW;
}


static BOOLEAN BasicCanCharacterDoctor(const SOLDIERTYPE* const s)
{
	return
		s->bMedical > 0 &&
		AreAssignmentConditionsMet(*s, AC_UNDERGROUND);
}


// is character capable of 'playing' doctor?
// check that character is alive, conscious, has medical skill and equipment
static BOOLEAN CanCharacterDoctor(SOLDIERTYPE const* const pSoldier)
{
	INT8 bPocket = 0;

	if (!BasicCanCharacterDoctor(pSoldier)) return FALSE;

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


static BOOLEAN CanCharacterRepairRobot(SOLDIERTYPE const*);
static bool CanCharacterRepairVehicle(SOLDIERTYPE const&, VEHICLETYPE const&);
static BOOLEAN DoesCharacterHaveAnyItemsToRepair(SOLDIERTYPE const*, INT8 bHighestPass);


static bool IsAnythingAroundForSoldierToRepair(SOLDIERTYPE const& s)
{
	// Items?
	if (DoesCharacterHaveAnyItemsToRepair(&s, FINAL_REPAIR_PASS)) return true;

	// Robot?
	if (CanCharacterRepairRobot(&s)) return true;

	// Vehicles?
	if (s.bSectorZ == 0)
	{
		CFOR_EACH_VEHICLE(v)
		{
			// The helicopter, is NEVER repairable
			if (IsHelicopter(v))                         continue;
			if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;
			if (!CanCharacterRepairVehicle(s, v))        continue;
			// There is a repairable vehicle here
			return true;
		}
	}

	return false;
}


static BOOLEAN HasCharacterFinishedRepairing(SOLDIERTYPE* pSoldier)
{
	BOOLEAN fCanStillRepair;

	// NOTE: This must detect situations where the vehicle/robot has left the sector, in which case we want the
	// guy to say "assignment done", so we return that he can no longer repair

	// check if we are repairing a vehicle
	if ( pSoldier->bVehicleUnderRepairID != -1 )
	{
		fCanStillRepair = CanCharacterRepairVehicle(*pSoldier, GetVehicle(pSoldier->bVehicleUnderRepairID));
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
static bool IsItemRepairable(UINT16 item_id, INT8 status);


static BOOLEAN DoesCharacterHaveAnyItemsToRepair(SOLDIERTYPE const* const pSoldier, INT8 const bHighestPass)
{
	UINT8	ubItemsInPocket, ubObjectInPocketCounter;
	UINT8 ubPassType;

	// check for jams
	CFOR_EACH_SOLDIER_INV_SLOT(i, *pSoldier)
	{
		ubItemsInPocket = i->ubNumberOfObjects;
		// unjam any jammed weapons
		// run through pocket and repair
		for( ubObjectInPocketCounter = 0; ubObjectInPocketCounter < ubItemsInPocket; ubObjectInPocketCounter++ )
		{
			// jammed gun?
			if (GCM->getItem(i->usItem)->getItemClass() == IC_GUN && i->bGunAmmoStatus < 0)
			{
				return( TRUE );
			}
		}
	}

	// now check for items to repair
	CFOR_EACH_SOLDIER_INV_SLOT(i, *pSoldier)
	{
		OBJECTTYPE const& o = *i;

		ubItemsInPocket = o.ubNumberOfObjects;

		// run through pocket
		for( ubObjectInPocketCounter = 0; ubObjectInPocketCounter < ubItemsInPocket; ubObjectInPocketCounter++ )
		{
			// if it's repairable and NEEDS repairing
			if (IsItemRepairable(o.usItem, o.bStatus[ubObjectInPocketCounter]))
			{
				return( TRUE );
			}
		}

		// have to check for attachments...
		for (INT8 bLoop = 0; bLoop < MAX_ATTACHMENTS; ++bLoop)
		{
			if (o.usAttachItem[bLoop] != NOTHING)
			{
				// if it's repairable and NEEDS repairing
				if (IsItemRepairable(o.usAttachItem[bLoop], o.bAttachStatus[bLoop]))
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
		CFOR_EACH_IN_TEAM(pOtherSoldier, OUR_TEAM)
		{
			if ( CanCharacterRepairAnotherSoldiersStuff( pSoldier, pOtherSoldier ) )
			{
				// okay, seems like a candidate!  Check if he has anything that needs unjamming or repairs

				for (INT8 bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; ++bPocket)
				{
					// the object a weapon? and jammed?
					if ( ( GCM->getItem(pOtherSoldier->inv[ bPocket ].usItem)->getItemClass() == IC_GUN ) && ( pOtherSoldier->inv[ bPocket ].bGunAmmoStatus < 0 ) )
					{
						return( TRUE );
					}
				}

				// repair everyone's hands and armor slots first, then headgear, and pockets last
				for ( ubPassType = REPAIR_HANDS_AND_ARMOR; ubPassType <= ( UINT8 ) bHighestPass; ubPassType++ )
				{
					INT8 const bPocket = FindRepairableItemOnOtherSoldier(pOtherSoldier, ubPassType);
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
		AreAssignmentConditionsMet(*s, AC_UNDERGROUND);
}


static BOOLEAN CanCharacterRepairButDoesntHaveARepairkit(const SOLDIERTYPE* const s)
{
	return
		BasicCanCharacterRepair(s) &&
		FindObj(s, TOOLKIT) == NO_SLOT; // make sure he actually doesn't have a toolkit
}


// can character be assigned as repairman?
// check that character is alive, oklife, has repair skill, and equipment, etc.
static BOOLEAN CanCharacterRepair(SOLDIERTYPE const* const pSoldier)
{
	if (!BasicCanCharacterRepair(pSoldier)) return FALSE;

	// make sure he has a toolkit
	if ( FindObj( pSoldier, TOOLKIT ) == NO_SLOT )
	{
		return( FALSE );
	}

	// anything around to fix?
	if (!IsAnythingAroundForSoldierToRepair(*pSoldier))
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
	return
		s->bLife > 0 &&
		s->bLife != s->bLifeMax &&
		AreAssignmentConditionsMet(*s, AC_UNCONSCIOUS | AC_EPC | AC_UNDERGROUND);
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
	return
		s->bLeadership > 0 &&
		CanSectorContainMilita(s->sSectorX, s->sSectorY, s->bSectorZ) &&
		NumEnemiesInAnySector(s->sSectorX, s->sSectorY, s->bSectorZ) == 0 &&
		AreAssignmentConditionsMet(*s, AC_NONE);
}


static INT8 CountMilitiaTrainersInSoldiersSector(const SOLDIERTYPE* s);


BOOLEAN CanCharacterTrainMilitia(const SOLDIERTYPE* const s)
{
	return
		BasicCanCharacterTrainMilitia(s)                                      &&
		MilitiaTrainingAllowedInSector(s->sSectorX, s->sSectorY, s->bSectorZ) &&
		DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(s)              &&
		!IsAreaFullOfMilitia(s->sSectorX, s->sSectorY, s->bSectorZ)           &&
		CountMilitiaTrainersInSoldiersSector(s) < MAX_MILITIA_TRAINERS_PER_SECTOR;
}


BOOLEAN DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(const SOLDIERTYPE* const s)
{
	// underground training is not allowed (code doesn't support and it's a reasonable enough limitation)
	if (s->bSectorZ != 0) return FALSE;

	INT8 const bTownId = GetTownIdForSector(SECTOR(s->sSectorX, s->sSectorY));
	if (bTownId != BLANK_SECTOR)
	{
		// Does this town have sufficient loyalty to train militia?
		return gTownLoyalty[bTownId].ubRating >= MIN_RATING_TO_TRAIN_TOWN;
	}
	else
	{
		return IsThisSectorASAMSector(s->sSectorX, s->sSectorY, s->bSectorZ);
	}
}


// only 2 trainers are allowed per sector, so this function counts the # in a guy's sector
static INT8 CountMilitiaTrainersInSoldiersSector(const SOLDIERTYPE* const pSoldier)
{
	INT8 bCount = 0;
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
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
		case STRENGTH:         return s->bStrength;
		case DEXTERITY:        return s->bDexterity;
		case AGILITY:          return s->bAgility;
		case HEALTH:           return s->bLifeMax;
		case MARKSMANSHIP:     return s->bMarksmanship;
		case MEDICAL:          return s->bMedical;
		case MECHANICAL:       return s->bMechanical;
		case LEADERSHIP:       return s->bLeadership;
		case EXPLOSIVE_ASSIGN: return s->bExplosive;
		// NOTE: Wisdom can't be trained!

		default:
			SLOGE("Unknown training stat %d", stat);
			return 0;
	}
}


// can character train stat?..as train self or as trainer?
static BOOLEAN CanCharacterTrainStat(const SOLDIERTYPE* const s, INT8 bStat, const BOOLEAN fTrainSelf, const BOOLEAN fTrainTeammate)
{
	// is the character capable of training this stat? either self or as trainer
	// underground training is not allowed (code doesn't support and it's a reasonable enough limitation)
	if (!AreAssignmentConditionsMet(*s, AC_NONE)) return FALSE;

	const INT8 stat_val = GetTrainingStatValue(s, bStat);
	return
		stat_val != 0 &&
		(!fTrainTeammate || stat_val >= MIN_RATING_TO_TEACH) &&
		(!fTrainSelf     || stat_val <  TRAINING_RATING_CAP);
}


// put character on duty?
static BOOLEAN CanCharacterOnDuty(const SOLDIERTYPE* const s)
{
	// can character commit themselves to on duty?
	return AreAssignmentConditionsMet(*s, AC_COMBAT | AC_EPC | AC_MECHANICAL | AC_UNDERGROUND);
}


// is character capable of practising at all?
static BOOLEAN CanCharacterPractise(const SOLDIERTYPE* const s)
{
	// can character practise right now?
	return AreAssignmentConditionsMet(*s, AC_NONE);
}


// can this character train others?
static BOOLEAN CanCharacterTrainTeammates(SOLDIERTYPE const* const pSoldier)
{
	// can character train at all
	if (!CanCharacterPractise(pSoldier)) return FALSE;

	// if alone in sector, can't enter the attributes submenu at all
	if ( PlayerMercsInSector( ( UINT8 ) pSoldier->sSectorX, ( UINT8 ) pSoldier->sSectorY, pSoldier->bSectorZ ) == 0 )
	{
		return( FALSE );
	}

	// ARM: we allow this even if there are no students assigned yet.  Flashing is warning enough.
	return( TRUE );
}


static BOOLEAN CanCharacterBeTrainedByOther(SOLDIERTYPE const* const pSoldier)
{
	// can character train at all
	if (!CanCharacterPractise(pSoldier)) return FALSE;

	// if alone in sector, can't enter the attributes submenu at all
	if ( PlayerMercsInSector( ( UINT8 ) pSoldier->sSectorX, ( UINT8 ) pSoldier->sSectorY, pSoldier->bSectorZ ) == 0 )
	{
		return( FALSE );
	}

	// ARM: we now allow this even if there are no trainers assigned yet.  Flashing is warning enough.
	return( TRUE );
}


// Can character sleep right now?
static bool CanCharacterSleep(SOLDIERTYPE const& s, bool const explain_why_not)
{
	if (!AreAssignmentConditionsMet(s, AC_IMPASSABLE | AC_COMBAT | AC_EPC | AC_IN_HELI_IN_HOSTILE_SECTOR | AC_MOVING | AC_UNDERGROUND)) return false;

	ST::string why;
	if (s.fBetweenSectors) // Traveling?
	{
		if (s.bAssignment != VEHICLE)
		{ // Can't sleep while walking
			why = zMarksMapScreenText[5];
			goto cannot_sleep;
		}
		else // in a vehicle
		{
			// If this guy has to drive (because nobody else can)
			if (SoldierMustDriveVehicle(s, false))
			{ // Can't sleep while driving a vehicle
				why = zMarksMapScreenText[7];
				goto cannot_sleep;
			}
		}
	}
	else // In a sector
	{
		// If not above it all
		if (!SoldierAboardAirborneHeli(s))
		{
			// If he's in the loaded sector, and it's hostile or in combat
			if (s.bInSector && (gTacticalStatus.uiFlags & INCOMBAT || gTacticalStatus.fEnemyInSector))
			{
				why = g_langRes->Message[STR_SECTOR_NOT_CLEARED];
				goto cannot_sleep;
			}

			// on surface, and enemies are in the sector
			if (s.bSectorZ == 0 && NumEnemiesInAnySector(s.sSectorX, s.sSectorY, s.bSectorZ) > 0)
			{
				why = g_langRes->Message[STR_SECTOR_NOT_CLEARED];
				goto cannot_sleep;
			}
		}
	}

	if (s.bBreathMax >= BREATHMAX_FULLY_RESTED) // Not tired?
	{
		why = zMarksMapScreenText[4];
		goto cannot_sleep;
	}

	return true;

cannot_sleep:
	if (explain_why_not)
	{
		ST::string buf = st_format_printf(why, s.name);
		DoScreenIndependantMessageBox(buf, MSG_BOX_FLAG_OK, 0);
	}
	return false;
}


static BOOLEAN CanCharacterBeAwakened(SOLDIERTYPE* pSoldier, BOOLEAN fExplainWhyNot)
{
	ST::string sString;

	// if dead tired
	if( ( pSoldier -> bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM ) && !pSoldier->fMercCollapsedFlag )
	{
		// should be collapsed, then!
		pSoldier->fMercCollapsedFlag = TRUE;
	}

	// merc collapsed due to being dead tired, you can't wake him up until he recovers substantially
	if (pSoldier->fMercCollapsedFlag)
	{
		if ( fExplainWhyNot )
		{
			sString = st_format_printf(zMarksMapScreenText[ 6 ], pSoldier->name);
			DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
		}

		return( FALSE );
	}

	// can be awakened
	return( TRUE );
}


// put character in vehicle?
static bool CanCharacterVehicle(SOLDIERTYPE const& s)
{ // Can character enter/leave vehicle?
	return
		fInMapMode && // Strictly for visual reasons - we don't want them just vanishing if in tactical
		AnyAccessibleVehiclesInSoldiersSector(s) &&
		( // If we're in battle in the current sector, disallow
			!gTacticalStatus.fEnemyInSector ||
			s.sSectorX != gWorldSectorX     ||
			s.sSectorY != gWorldSectorY     ||
			s.bSectorZ != gbWorldSectorZ
		) &&
		AreAssignmentConditionsMet(s, AC_EPC | AC_MECHANICAL);
}


enum JoinSquadResult
{
	CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT = -6,
	CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING  = -5,
	CHARACTER_CANT_JOIN_SQUAD_MOVING        = -4,
	CHARACTER_CANT_JOIN_SQUAD_VEHICLE       = -3,
	CHARACTER_CANT_JOIN_SQUAD_TOO_FAR       = -2,
	CHARACTER_CANT_JOIN_SQUAD_FULL          = -1,
	CHARACTER_CANT_JOIN_SQUAD               =  0,
	CHARACTER_CAN_JOIN_SQUAD                =  1
};


// can character be added to squad
static JoinSquadResult CanCharacterSquad(SOLDIERTYPE const& s, INT8 const squad_no)
{
	Assert(squad_no < ON_DUTY);

	INT16 x;
	INT16 y;
	INT8  z;
	if (s.bAssignment == squad_no)
	{
		return CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT;
	}
	else if (s.bLife < OKLIFE)
	{
		return CHARACTER_CANT_JOIN_SQUAD;
	}
	else if (IsCharacterInTransit(s))
	{
		return CHARACTER_CANT_JOIN_SQUAD;
	}
	else if (s.bAssignment == ASSIGNMENT_POW)
	{
		return CHARACTER_CANT_JOIN_SQUAD;
	}
	else if (SectorSquadIsIn(squad_no, &x, &y, &z) &&
			(x != s.sSectorX || y != s.sSectorY || z != s.bSectorZ))
	{
		return CHARACTER_CANT_JOIN_SQUAD_TOO_FAR;
	}
	else if (IsThisSquadOnTheMove(squad_no))
	{
		return CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING;
	}
	else if (DoesVehicleExistInSquad(squad_no))
	{
		return CHARACTER_CANT_JOIN_SQUAD_VEHICLE;
	}
	else if (NumberOfPeopleInSquad(squad_no) >= NUMBER_OF_SOLDIERS_PER_SQUAD)
	{
		return CHARACTER_CANT_JOIN_SQUAD_FULL;
	}
	else
	{
		return CHARACTER_CAN_JOIN_SQUAD;
	}
}


bool IsCharacterInTransit(SOLDIERTYPE const& s)
{
	return s.bAssignment == IN_TRANSIT;
}


static void CheckForAndHandleHospitalPatients(void);
static void HandleDoctorsInSector(INT16 x, INT16 y, INT8 z);
static void HandleNaturalHealing(void);
static void HandleRepairmenInSector(INT16 sX, INT16 sY, INT8 bZ);
static void HandleRestFatigueAndSleepStatus();
static void HandleTrainingInSector(INT16 sMapX, INT16 sMapY, INT8 bZ);
static void ReportTrainersTraineesWithoutPartners(void);
static void UpdatePatientsWhoAreDoneHealing();


void UpdateAssignments()
{
	INT8 sX,sY, bZ;

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

	// run through sectors and handle each type in sector
	for(sX = 0 ; sX < MAP_WORLD_X; sX++ )
	{
		for( sY =0; sY < MAP_WORLD_X; sY++ )
		{
			for( bZ = 0; bZ < 4; bZ++)
			{
				// is there anyone in this sector?
				if (fSectorsWithSoldiers[sX + sY * MAP_WORLD_X][bZ])
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


static BOOLEAN CanSoldierBeHealedByDoctor(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pDoctor, BOOLEAN fThisHour, BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck);


static UINT8 GetNumberThatCanBeDoctored(SOLDIERTYPE* pDoctor, BOOLEAN fThisHour, BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck)
{
	// go through list of characters, find all who are patients/doctors healable by this doctor
	UINT8 ubNumberOfPeople = 0;
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (CanSoldierBeHealedByDoctor(s, pDoctor, fThisHour, fSkipKitCheck, fSkipSkillCheck))
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
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// doctor?
		if (s->bAssignment == DOCTOR &&
				CanSoldierBeHealedByDoctor(pPatient, s, fThisHour, FALSE, FALSE))
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


// Have we spent enough time on assignment for it to count?
static bool EnoughTimeOnAssignment(SOLDIERTYPE const& s)
{
	return GetWorldTotalMin() - s.uiLastAssignmentChangeMin >= MINUTES_FOR_ASSIGNMENT_TO_COUNT;
}


static void HealCharacters(SOLDIERTYPE* pDoctor);


// handle doctor in this sector
static void HandleDoctorsInSector(INT16 const x, INT16 const y, INT8 const z)
{
	// will handle doctor/patient relationship in sector

	// go through list of characters, find all doctors in sector
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE& s = *i;
		if (s.sSectorX != x)         continue;
		if (s.sSectorY != y)         continue;
		if (s.bSectorZ != z)         continue;
		if (s.bAssignment != DOCTOR) continue;
		if (s.fMercAsleep)           continue;
		MakeSureMedKitIsInHand(&s);
		// Character is in sector, check if can doctor, if so, heal people
		if (!CanCharacterDoctor(&s))    continue;
		if (!EnoughTimeOnAssignment(s)) continue;
		HealCharacters(&s);
	}
}


// Update characters who might done healing but are still patients
static void UpdatePatientsWhoAreDoneHealing()
{
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE& s = *i;
		if (s.bAssignment != PATIENT) continue;
		if (s.bLife != s.bLifeMax)    continue;
		// Patient who doesn't need healing
		AssignmentDone(&s, TRUE, TRUE);
	}
}


static void AssignmentAborted(SOLDIERTYPE const&, AssignmentAbortReason);
static UINT16 HealPatient(SOLDIERTYPE* pPatient, SOLDIERTYPE* pDoctor, UINT16 usHundredthsHealed);


// heal characters in this sector with this doctor
static void HealCharacters(SOLDIERTYPE* const pDoctor)
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
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			if (CanSoldierBeHealedByDoctor(s, pDoctor, HEALABLE_THIS_HOUR, FALSE, FALSE))
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
				FOR_EACH_IN_TEAM(s, OUR_TEAM)
				{
					if (CanSoldierBeHealedByDoctor(s, pDoctor, HEALABLE_THIS_HOUR, FALSE, FALSE))
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
		StatChange(*pDoctor, MEDICALAMT, usUsedHealingPts / 100, FROM_SUCCESS);
		StatChange(*pDoctor, DEXTAMT,    usUsedHealingPts / 200, FROM_SUCCESS);
		StatChange(*pDoctor, WISDOMAMT,  usUsedHealingPts / 200, FROM_SUCCESS);
	}


	// if there's nobody else here who can EVER be helped by this doctor (regardless of whether they got healing this hour)
	if( GetNumberThatCanBeDoctored( pDoctor, HEALABLE_EVER, FALSE, FALSE ) == 0 )
	{
		// then this doctor has done all that he can do, but let's find out why and tell player the reason

		// try again, but skip the med kit check!
		if( GetNumberThatCanBeDoctored( pDoctor, HEALABLE_EVER, TRUE, FALSE ) > 0 )
		{
			// he could doctor somebody, but can't because he doesn't have a med kit!
			AssignmentAborted(*pDoctor, NO_MORE_MED_KITS);
		}
		// try again, but skip the skill check!
		else if( GetNumberThatCanBeDoctored( pDoctor, HEALABLE_EVER, FALSE, TRUE ) > 0 )
		{
			// he could doctor somebody, but can't because he doesn't have enough skill!
			AssignmentAborted(*pDoctor, INSUF_DOCTOR_SKILL);
		}
		else
		{
			// all patients should now be healed - truly DONE!
			AssignmentDone( pDoctor, TRUE, TRUE );
		}
	}
}


// returns minimum medical skill necessary to treat this patient
static UINT8 GetMinHealingSkillNeeded(SOLDIERTYPE const* const patient)
{
	if (patient->bLife < OKLIFE)
	{
		return
			BASE_MEDICAL_SKILL_TO_DEAL_WITH_EMERGENCY +
			MULTIPLIER_FOR_DIFFERENCE_IN_LIFE_VALUE_FOR_EMERGENCY * (OKLIFE - patient->bLife);
	}
	else
	{
		return 1;
	}
}


// can this soldier be healed by this doctor?
static BOOLEAN CanSoldierBeHealedByDoctor(SOLDIERTYPE const* const patient, SOLDIERTYPE const* const doctor, BOOLEAN const fThisHour, BOOLEAN const fSkipKitCheck, BOOLEAN const fSkipSkillCheck)
{
	if (patient->bAssignment != PATIENT && patient->bAssignment != DOCTOR)        return FALSE;
	if (patient->bLife == 0)                                                      return FALSE;
	if (patient->bLife == patient->bLifeMax)                                      return FALSE;
	if (fThisHour && !EnoughTimeOnAssignment(*patient))                           return FALSE;
	if (patient->sSectorX != doctor->sSectorX)                                    return FALSE;
	if (patient->sSectorY != doctor->sSectorY)                                    return FALSE;
	if (patient->bSectorZ != doctor->bSectorZ)                                    return FALSE;
	if (patient->fBetweenSectors)                                                 return FALSE;
	if (!fSkipSkillCheck && doctor->bMedical < GetMinHealingSkillNeeded(patient)) return FALSE;
	if (!fSkipKitCheck && FindObj(doctor, MEDICKIT) == NO_SLOT)                   return FALSE;
	return TRUE;
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
			OBJECTTYPE o = pDoctor->inv[bPocket];
			if (IsMedicalKitItem(&o))
			{
				// ok, we have med kit in this pocket, use it
				bPointsUsed   = UseKitPoints(o, bPointsToUse, *pDoctor);
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
			OBJECTTYPE& o = pDoctor->inv[bPocket];
			if (IsMedicalKitItem(&o))
			{
				// ok, we have med kit in this pocket, use it  (use only half if it's worth double)
				bPointsUsed   = UseKitPoints(o, bPointsToUse, *pDoctor);
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
	if (!fSectorsWithSoldiers[HOSPITAL_SECTOR_X + HOSPITAL_SECTOR_Y * MAP_WORLD_X][0])
	{
		// nobody in the hospital sector... leave
		return;
	}

	// go through list of characters, find all who are on this assignment
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
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


static void HandleRepairBySoldier(SOLDIERTYPE&);


// handle any repair man in sector
static void HandleRepairmenInSector(INT16 const x, INT16 const y, INT8 const z)
{
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE& s = *i;
		if (s.sSectorX    != x)      continue;
		if (s.sSectorY    != y)      continue;
		if (s.bSectorZ    != z)      continue;
		if (s.bAssignment != REPAIR) continue;
		if (s.fMercAsleep)           continue;

		MakeSureToolKitIsInHand(&s);

		// character is in sector, check if can repair
		if (!CanCharacterRepair(&s))    continue;
		if (!EnoughTimeOnAssignment(s)) continue;

		HandleRepairBySoldier(s);
	}
}

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

		OBJECTTYPE const& o = pSoldier->inv[bSlotToCheck];
		for ( bLoop2 = 0; bLoop2 < pSoldier->inv[ bSlotToCheck ].ubNumberOfObjects; bLoop2++ )
		{
			if (IsItemRepairable(o.usItem, o.bStatus[bLoop2]))
			{
				return( bSlotToCheck );
			}
		}

		// have to check for attachments...
		for ( bLoop2 = 0; bLoop2 < MAX_ATTACHMENTS; bLoop2++ )
		{
			if (o.usAttachItem[bLoop2] != NOTHING)
			{
				if (IsItemRepairable(o.usAttachItem[bLoop2], o.bAttachStatus[bLoop2]))
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
	sRepairCostAdj = 100 - ( 10 * GCM->getItem(usItem)->getRepairEase() );

	// make sure it ain't somehow gone too low!
	if (sRepairCostAdj < 10)
	{
		sRepairCostAdj = 10;
	}

	// repairs on electronic items take twice as long if the guy doesn't have the skill
	if ( ( GCM->getItem(usItem)->getFlags() & ITEM_ELECTRONIC ) && ( !( HAS_SKILL_TRAIT( pSoldier, ELECTRONICS ) ) ) )
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


static bool DoRepair(SOLDIERTYPE* const repairer, SOLDIERTYPE const* const owner, UINT16 const item, INT8& status, UINT8* const repair_pts_left)
{
	// if it's repairable and NEEDS repairing
	if (!IsItemRepairable(item, status)) return false;
	DoActualRepair(repairer, item, &status, repair_pts_left);
	if (status == 100)
	{ // report it as fixed
		if (repairer == owner)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_REPAIRED], repairer->name, ItemNames[item]));
		}
		else
		{ // NOTE: may need to be changed for localized versions
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_35], repairer->name, owner->name, ItemNames[item]));
		}
	}
	return true;
}


static bool RepairObject(SOLDIERTYPE* const repairer, SOLDIERTYPE const* const owner, OBJECTTYPE* const pObj, UINT8* const repair_pts_left)
{
	bool something_was_repaired = false;

	UINT8 const n_items = pObj->ubNumberOfObjects;
	for (UINT8 i = 0; i != n_items; ++i)
	{
		UINT16 const item   = pObj->usItem;
		INT8&        status = pObj->bStatus[i];
		if (!DoRepair(repairer, owner, item, status, repair_pts_left)) continue;
		something_was_repaired = true;
		if (*repair_pts_left == 0) break; // we're out of points!
	}

	// now check for attachments
	for (UINT8 i = 0; i != MAX_ATTACHMENTS; ++i)
	{
		UINT16 const item = pObj->usAttachItem[i];
		if (item == NOTHING) continue;
		INT8& status = pObj->bAttachStatus[i];
		if (!DoRepair(repairer, owner, item, status, repair_pts_left)) continue;
		something_was_repaired = true;
		if (*repair_pts_left == 0) break; // we're out of points!
	}

	return something_was_repaired;
}


static UINT8 HandleRepairOfRobotBySoldier(UINT8 ubRepairPts, BOOLEAN* pfNothingLeftToRepair);
static void RepairItemsOnOthers(SOLDIERTYPE* pSoldier, UINT8* pubRepairPtsLeft);
static BOOLEAN UnjamGunsOnSoldier(SOLDIERTYPE* pOwnerSoldier, SOLDIERTYPE* pRepairSoldier, UINT8* pubRepairPtsLeft);


// Repair stuff
static void HandleRepairBySoldier(SOLDIERTYPE& s)
{
	BOOLEAN nothing_left_to_repair;

	// grab max number of repair pts open to this soldier
	UINT16 usMax;
	UINT8  initial_repair_pts = CalculateRepairPointsForRepairman(&s, &usMax, TRUE);
	if (initial_repair_pts == 0)
	{ // No points
		AssignmentDone(&s, TRUE, TRUE);
		return;
	}
	UINT8 repair_pts_left = initial_repair_pts;

	// check if we are repairing a vehicle
	if (s.bVehicleUnderRepairID != -1)
	{
		VEHICLETYPE const& v = GetVehicle(s.bVehicleUnderRepairID);
		if (CanCharacterRepairVehicle(s, v))
		{ // Attempt to fix vehicle
			repair_pts_left -= RepairVehicle(v, repair_pts_left, &nothing_left_to_repair);
		}
	}
	// check if we are repairing a robot
	else if (s.fFixingRobot)
	{
		if (CanCharacterRepairRobot(&s))
		{
			// repairing the robot is very slow & difficult
			repair_pts_left    /= 2;
			initial_repair_pts /= 2;

			if (!(HAS_SKILL_TRAIT(&s, ELECTRONICS)))
			{
				repair_pts_left    /= 2;
				initial_repair_pts /= 2;
			}

			// Robot
			repair_pts_left -= HandleRepairOfRobotBySoldier(repair_pts_left, &nothing_left_to_repair);
		}
	}
	else
	{
		BOOLEAN fAnyOfSoldiersOwnItemsWereFixed = UnjamGunsOnSoldier(&s, &s, &repair_pts_left);

		// Repair items on self
		for (INT8 i = 0; i != 2; ++i)
		{
			INT8 start;
			INT8 end;
			if (i == 0)
			{
				start = SECONDHANDPOS;
				end   = SMALLPOCK8POS;
			}
			else
			{
				start = HELMETPOS;
				end   = HEAD2POS;
			}

			// now repair objects running from left hand to small pocket
			for (INT8 pocket = start; pocket <= end; ++pocket)
			{
				if (RepairObject(&s, &s, &s.inv[pocket], &repair_pts_left))
				{
					fAnyOfSoldiersOwnItemsWereFixed = TRUE;

					// quit looking if we're already out
					if (repair_pts_left == 0) break;
				}
			}
		}

		// if he fixed something of his, and now has no more of his own items to fix
		if (fAnyOfSoldiersOwnItemsWereFixed && !DoesCharacterHaveAnyItemsToRepair(&s, -1))
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(sRepairsDoneString[0], s.name));

			// let player react
			StopTimeCompression();
		}


		// repair items on others
		RepairItemsOnOthers(&s, &repair_pts_left);
	}

	// what are the total amount of pts used by character?
	UINT8 const repair_pts_used = initial_repair_pts - repair_pts_left;
	if (repair_pts_used > 0)
	{
		// improve stats
		StatChange(s, MECHANAMT, repair_pts_used / 2, FROM_SUCCESS);
		StatChange(s, DEXTAMT,   repair_pts_used / 2, FROM_SUCCESS);

		// check if kit damaged/depleted
		if (Random(100) < repair_pts_used * 5) // CJC: added a x5 as this wasn't going down anywhere fast enough
		{
			// kit item damaged/depleted, burn up points of toolkit..which is in right hand
			UseKitPoints(s.inv[HANDPOS], 1, s);
		}
	}


	// if he really done
	if (HasCharacterFinishedRepairing(&s))
	{
		// yup, that's all folks
		AssignmentDone(&s, TRUE, TRUE);
	}
	else // still has stuff to repair
	{
		// if nothing got repaired, there's a problem
		if (repair_pts_used == 0)
		{
			// see if not having a toolkit is the problem
			if (FindObj(&s, TOOLKIT) == NO_SLOT)
			{
				// he could (maybe) repair something, but can't because he doesn't have a tool kit!
				AssignmentAborted(s, NO_MORE_TOOL_KITS);
			}
			else
			{
				// he can't repair anything because he doesn't have enough skill!
				AssignmentAborted(s, INSUF_REPAIR_SKILL);
			}
		}
	}
}


// can item be repaired?
static bool IsItemRepairable(UINT16 const item_id, INT8 const status)
{
	return status < 100 && GCM->getItem(item_id)->getFlags() & ITEM_REPAIRABLE;
}


static UINT8 CalcSoldierNeedForSleep(SOLDIERTYPE const&);


// rest the character
static void RestCharacter(SOLDIERTYPE* pSoldier)
{
	// handle the sleep of this character, update bBreathMax based on sleep they have
	INT8 bMaxBreathRegain = 0;

	bMaxBreathRegain = 50 / CalcSoldierNeedForSleep(*pSoldier);

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


void FatigueCharacter(SOLDIERTYPE& s)
{
	if (IsMechanical(s))                 return;
	if (IsCharacterInTransit(s))         return;
	if (s.bAssignment == ASSIGNMENT_POW) return;

	INT8 const divisor         = 24 - CalcSoldierNeedForSleep(s);
	INT8       max_breath_loss = 50 / divisor;
	if (max_breath_loss < 2) max_breath_loss = 2;

	/* KM: Added encumbrance calculation to soldiers moving on foot. Anything
	 * above 100% will increase rate of fatigue. 200% encumbrance will cause
	 * soldiers to tire twice as quickly. */
	if (s.fBetweenSectors && s.bAssignment != VEHICLE)
	{ // Soldier is on foot and traveling. Factor encumbrance into fatigue rate.
		INT32 const percent_encumbrance = CalculateCarriedWeight(&s);
		if (percent_encumbrance > 100)
		{
			INT32 const breath_loss = max_breath_loss * percent_encumbrance / 100;
			max_breath_loss = MIN(breath_loss, 127);
		}
	}

	INT8 breath_max = s.bBreathMax;

	if (breath_max < BREATHMAX_PRETTY_TIRED)
	{ // Real tired, fatigue rate is 50% higher
		max_breath_loss = max_breath_loss * 3 / 2;
	}

	breath_max -= max_breath_loss;

	if (breath_max > 100)
	{
		breath_max = 100;
	}
	else if (breath_max < BREATHMAX_ABSOLUTE_MINIMUM)
	{
		breath_max = BREATHMAX_ABSOLUTE_MINIMUM;
	}

	s.bBreathMax = breath_max;
	// Current breath can't exceed maximum
	if (s.bBreath > breath_max) s.bBreath = breath_max;
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
	std::fill(std::begin(pStatTrainerList), std::end(pStatTrainerList), nullptr);

	// build list of teammate trainers in this sector.

	// Only the trainer with the HIGHEST training ability in each stat is effective.  This is mainly to avoid having to
	// sort them from highest to lowest if some form of trainer degradation formula was to be used for multiple trainers.

	// for each trainable stat
	for (ubStat = 0; ubStat < NUM_TRAINABLE_STATS; ubStat++)
	{
		sBestTrainingPts = -1;

		// search team for active instructors in this sector
		CFOR_EACH_IN_TEAM(pTrainer, OUR_TEAM)
		{
			if (pTrainer->sSectorX == sMapX && pTrainer->sSectorY == sMapY && pTrainer->bSectorZ == bZ)
			{
				// if he's training teammates in this stat
				if (pTrainer->bAssignment == TRAIN_TEAMMATE &&
						pTrainer->bTrainStat  == ubStat         &&
						EnoughTimeOnAssignment(*pTrainer)       &&
						!pTrainer->fMercAsleep)
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
	FOR_EACH_IN_TEAM(pStudent, OUR_TEAM)
	{
		// see if this merc is active and in the same sector
		if (pStudent->sSectorX == sMapX && pStudent->sSectorY == sMapY && pStudent->bSectorZ == bZ)
		{
			// if he's training himself (alone, or by others), then he's a student
			if ( ( pStudent -> bAssignment == TRAIN_SELF ) || ( pStudent -> bAssignment == TRAIN_BY_OTHER ) )
			{
				if (EnoughTimeOnAssignment(*pStudent) && !pStudent->fMercAsleep)
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
									EnoughTimeOnAssignment(*pTrainer))
*/
							// NB this EnoughTimeOnAssignment() call is redundent since it is called up above
							//if (EnoughTimeOnAssignment(*pTrainer))
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
		std::fill(std::begin(TownTrainer), std::end(TownTrainer), TOWN_TRAINER_TYPE{});
		ubTownTrainers = 0;

		// build list of all the town trainers in this sector and their training pts
		FOR_EACH_IN_TEAM(pTrainer, OUR_TEAM)
		{
			if (pTrainer->sSectorX == sMapX && pTrainer->sSectorY == sMapY && pTrainer->bSectorZ == bZ)
			{
				if (pTrainer->bAssignment == TRAIN_TOWN &&
						EnoughTimeOnAssignment(*pTrainer)   &&
						!pTrainer->fMercAsleep)
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
	const TOWN_TRAINER_TYPE* const t1 = (const TOWN_TRAINER_TYPE*)pArg1;
	const TOWN_TRAINER_TYPE* const t2 = (const TOWN_TRAINER_TYPE*)pArg2;
	return (t1->sTrainingPts < t2->sTrainingPts) - (t1->sTrainingPts > t2->sTrainingPts);
}


INT16 GetBonusTrainingPtsDueToInstructor(const SOLDIERTYPE* pInstructor, const SOLDIERTYPE* pStudent, INT8 bTrainStat, BOOLEAN fAtGunRange, UINT16* pusMaxPts)
{
	// return the bonus training pts of this instructor with this student,...if student null, simply assignment student skill of 0 and student wisdom of 100
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
			SLOGE("GetBonusTrainingPtsDueToInstructor: Unknown bTrainStat %d", bTrainStat);
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
	UINT16 sTrainingPts = (bTrainerEffSkill - bTraineeSkill) * (bTraineeEffWisdom + (EffectiveWisdom(pInstructor) + EffectiveLeadership(pInstructor)) / 2) / INSTRUCTED_TRAINING_DIVISOR;

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
	UINT16 sTrainingPts = __max(EffectiveWisdom(s) * (TRAINING_RATING_CAP - bSkill) / SELF_TRAINING_DIVISOR, 1);

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
	INT8	bTrainingBonus = 0;

	INT16 sBestTrainingPts, sTrainingPtsDueToInstructor;
	UINT16	usMaxTrainerPts;

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
	UINT16 sTrainingPts = __max(EffectiveWisdom(s) * (TRAINING_RATING_CAP - bSkill) / SELF_TRAINING_DIVISOR, 1);

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
	UINT16 usBestMaxTrainerPts = 0; // XXX HACK000E

	// search team for active instructors in this sector
	CFOR_EACH_IN_TEAM(pTrainer, OUR_TEAM)
	{
		if (pTrainer->sSectorX == s->sSectorX && pTrainer->sSectorY == s->sSectorY && pTrainer->bSectorZ == s->bSectorZ)
		{
			// if he's training teammates in this stat
			// NB skip the EnoughTime requirement to display what the value should be even if haven't been training long yet...
			if (pTrainer->bAssignment == TRAIN_TEAMMATE &&
					pTrainer->bTrainStat  == bTrainStat     &&
					!pTrainer->fMercAsleep)
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
static void TrainSoldierWithPts(SOLDIERTYPE* const s, const INT16 train_pts)
{
	if (train_pts <= 0) return;

	// which stat to modify?
	StatKind stat;
	switch (s->bTrainStat)
	{
		case STRENGTH:         stat = STRAMT;     break;
		case DEXTERITY:        stat = DEXTAMT;    break;
		case AGILITY:          stat = AGILAMT;    break;
		case HEALTH:           stat = HEALTHAMT;  break;
		case LEADERSHIP:       stat = LDRAMT;     break;
		case MARKSMANSHIP:     stat = MARKAMT;    break;
		case EXPLOSIVE_ASSIGN: stat = EXPLODEAMT; break;
		case MEDICAL:          stat = MEDICALAMT; break;
		case MECHANICAL:       stat = MECHANAMT;  break;
		// NOTE: Wisdom can't be trained!
		default:
			// BETA message
			SLOGE("TrainSoldierWithPts: Unknown bTrainStat %d", s->bTrainStat);
			return;
	}

	// give this merc a few chances to increase a stat (TRUE means it's training, reverse evolution doesn't apply)
	StatChange(*s, stat, train_pts, FROM_TRAINING);
}


// train militia in this sector with this soldier
static BOOLEAN TrainTownInSector(SOLDIERTYPE* pTrainer, INT16 sMapX, INT16 sMapY, INT16 sTrainingPts)
{
	Assert(CanSectorContainMilita(pTrainer->sSectorX, pTrainer->sSectorY, pTrainer->bSectorZ));

	SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( sMapX, sMapY ) ] );

	// trainer gains leadership - training argument is FROM_SUCCESS, because the trainer is not the one training!
	StatChange(*pTrainer, LDRAMT, 1 + sTrainingPts / 200, FROM_SUCCESS);

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
	INT8 bTrainingBonus = 0;
//	UINT8 ubTownId = 0;

	// calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs, fatigue)
	*pusMaxPts = ( pTrainer -> bWisdom + pTrainer -> bLeadership + ( 10 * pTrainer -> bExpLevel ) ) * TOWN_TRAINING_RATE;

	// calculate effective training points (this is hundredths of pts / hour)
	// typical: 300/hr, maximum: 600/hr
	UINT16 sTotalTrainingPts = (EffectiveWisdom(pTrainer) + EffectiveLeadership(pTrainer) + 10 * EffectiveExpLevel(pTrainer)) * TOWN_TRAINING_RATE;

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
	if (MercProfile(pTrainer->ubProfile).isNPCorRPC())
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


void MakeSoldiersTacticalAnimationReflectAssignment(SOLDIERTYPE* const s)
{
	if (!s->bInSector || !gfWorldLoaded || s->bLife < OKLIFE) return;
	// soldier is in tactical, world loaded, he's OKLIFE

	// Set animation based on his assignment
	switch (s->bAssignment)
	{
		case DOCTOR:  SoldierInSectorDoctor( s, s->usStrategicInsertionData); break;
		case PATIENT: SoldierInSectorPatient(s, s->usStrategicInsertionData); break;
		case REPAIR:  SoldierInSectorRepair( s, s->usStrategicInsertionData); break;

		default:
			if (s->usAnimState != WKAEUP_FROM_SLEEP)
			{
				ChangeSoldierState(s, STANDING, 1, TRUE);
			}
			break;
	}
}


static void AssignmentAborted(SOLDIERTYPE const& s, AssignmentAbortReason const reason)
{
	Assert(reason < NUM_ASSIGN_ABORT_REASONS);
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[reason], s.name));
	StopTimeCompression();
	fCharacterInfoPanelDirty = TRUE;
	fTeamPanelDirty          = TRUE;
	fMapScreenBottomDirty    = TRUE;
}


void AssignmentDone( SOLDIERTYPE *pSoldier, BOOLEAN fSayQuote, BOOLEAN fMeToo )
{
	if ( ( pSoldier -> bInSector ) && ( gfWorldLoaded ) )
	{
		if ( pSoldier -> bAssignment == DOCTOR )
		{
			const UINT16 state = (guiCurrentScreen == GAME_SCREEN ? END_DOCTOR : STANDING);
			ChangeSoldierState(pSoldier, state, 1, TRUE);
		}
		else if ( pSoldier -> bAssignment == REPAIR )
		{
			const UINT16 state = (guiCurrentScreen == GAME_SCREEN ? END_REPAIRMAN : STANDING);
			ChangeSoldierState(pSoldier, state, 1, TRUE);
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
		if (!fMeToo && pSoldier->bAssignment == TRAIN_TOWN)
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
		AddSoldierToWaitingListQueue(*pSoldier);

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
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE& s = *i;
		// mechanical members don't regenerate!
		if (IsMechanical(s)) continue;
		HandleHealingByNaturalCauses(&s);
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
	if (pSoldier->fMercAsleep            ||
			pSoldier->bAssignment == PATIENT ||
			pSoldier->bAssignment == ASSIGNMENT_HOSPITAL)
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


static void CreateDestroyMouseRegionsForRemoveMenu(void);


// Check if we can only remove character from team
static BOOLEAN HandleRemoveMenu(const INT8 remove_char)
{
	static BOOLEAN fShowRemoveMenu = FALSE;

	if (!fShowRemoveMenu)
	{
		if (remove_char == -1) return FALSE;
		const SOLDIERTYPE* const s = gCharactersList[remove_char].merc;
		if (s->bLife != 0 && s->bAssignment != ASSIGNMENT_POW) return FALSE;

		bSelectedAssignChar = remove_char;
	}

	// dead guy handle menu stuff
	fShowRemoveMenu = fShowAssignmentMenu | fShowContractMenu;
	CreateDestroyMouseRegionsForRemoveMenu();
	return TRUE;
}


static void AssignmentMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void AssignmentMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void CheckAndUpdateTacticalAssignmentPopUpPositions(void);
static SOLDIERTYPE* GetSelectedAssignSoldier(BOOLEAN fNullOK);
static void PositionCursorForTacticalAssignmentBox(void);


// Create/destroy mouse regions for the map screen assignment main menu
static void CreateDestroyMouseRegionsForAssignmentMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	if (HandleRemoveMenu(bSelectedAssignChar)) return;

	if (fShowAssignmentMenu && !fCreated)
	{
		gfIgnoreScrolling = FALSE;

		if (guiCurrentScreen == MAP_SCREEN)
		{
			SetBoxXY(ghAssignmentBox, AssignmentPosition.iX, AssignmentPosition.iY);
		}

		SOLDIERTYPE     const& s    = *GetSelectedAssignSoldier(FALSE);
		PopUpBox const* const  box  = s.ubWhatKindOfMercAmI == MERC_TYPE__EPC ? ghEpcBox : ghAssignmentBox;
		SGPBox          const& area = GetBoxArea(box);
		UINT16          const  x    = area.x;
		UINT16                 y    = area.y + GetTopMarginSize(ghAssignmentBox);
		UINT16          const  w    = area.w;
		UINT16          const  dy   = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));

		// Add mouse region for each line of text and set user data
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghAssignmentBox); ++i)
		{
			MOUSE_REGION* const r = &gAssignmentMenuRegion[i];
			MSYS_DefineRegion(r, x, y, x + w, y + dy, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, AssignmentMenuMvtCallBack, AssignmentMenuBtnCallback);
			MSYS_SetRegionUserData(r, 0, i);
			y += dy;
		}

		UnHighLightBox(ghAssignmentBox); // unhighlight all strings in box
		CheckAndUpdateTacticalAssignmentPopUpPositions();
		PositionCursorForTacticalAssignmentBox();

		fCreated = TRUE;
	}
	else if (!fShowAssignmentMenu && fCreated)
	{
		// destroy
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghAssignmentBox); ++i)
		{
			MSYS_RemoveRegion(&gAssignmentMenuRegion[i]);
		}

		fShownAssignmentMenu = FALSE;

		SetRenderFlags(RENDER_FLAG_FULL);

		fCreated = FALSE;
	}
}


static void HandleShadingOfLinesForVehicleMenu(void);
static void VehicleMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void VehicleMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void VehicleMenuCancelBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionForVehicleMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	PopUpBox* const box = ghVehicleBox;

	if (fShowVehicleMenu)
	{
		SGPBox const& area = GetBoxArea(ghAssignmentBox);
		VehiclePosition.iX = area.x + area.w;
		SetBoxXY(box, VehiclePosition.iX, VehiclePosition.iY);
	}

	if (!fCreated && fShowVehicleMenu)
	{
		SGPBox      const& area = GetBoxArea(box);
		UINT16      const  x    = area.x;
		UINT16                    y    = area.y + GetTopMarginSize(ghAssignmentBox); // XXX wrong box?
		UINT16      const  w    = area.w;
		UINT16      const  h    = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));
		MOUSE_REGION*      r    = gVehicleMenuRegion;
		SOLDIERTYPE const& s    = *GetSelectedAssignSoldier(FALSE);
		FOR_EACH_VEHICLE(v)
		{
			if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;

			// add mouse region for each accessible vehicle
			MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, VehicleMenuMvtCallback, VehicleMenuBtnCallback);
			r->SetUserPtr(&v);
			y += h;
			++r;
		}

		// cancel line
		MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, VehicleMenuMvtCallback, VehicleMenuCancelBtnCallback);

		PauseGame();
		UnHighLightBox(box);
		HandleShadingOfLinesForVehicleMenu();

		fCreated = TRUE;
	}
	else if (fCreated && (!fShowVehicleMenu || !fShowAssignmentMenu))
	{
		// remove these regions
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			MSYS_RemoveRegion(&gVehicleMenuRegion[i]);
		}

		fShowVehicleMenu = FALSE;

		SetRenderFlags(RENDER_FLAG_FULL);
		HideBox(box);

		if (fShowAssignmentMenu) UnHighLightBox(ghAssignmentBox);

		fCreated = FALSE;
	}
}


static void HandleShadingOfLinesForVehicleMenu()
{
	if (!fShowVehicleMenu) return;

	PopUpBox* const box = ghVehicleBox;
	if (box == NO_POPUP_BOX) return;

	SOLDIERTYPE const& s    = *GetSelectedAssignSoldier(FALSE);
	UINT32             line = 0;
	CFOR_EACH_VEHICLE(v)
	{
		// inaccessible vehicles aren't listed at all!
		if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;

		PopUpShade const shade = IsEnoughSpaceInVehicle(v) ?
			POPUP_SHADE_NONE : POPUP_SHADE_SECONDARY;
		ShadeStringInBox(box, line++, shade);
	}
}


static void VehicleMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SOLDIERTYPE& s = *GetSelectedAssignSoldier(FALSE);
		VEHICLETYPE& v = *pRegion->GetUserPtr<VEHICLETYPE>();

		// inaccessible vehicles shouldn't be listed in the menu!
		Assert(IsThisVehicleAccessibleToSoldier(s, v));

		if (IsEnoughSpaceInVehicle(v))
		{
			PutSoldierInVehicle(s, v);
		}
		else
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(gzLateLocalizedString[STR_LATE_18], zVehicleName[v.ubVehicleType]));
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
		const INT32 line = (INT32)(pRegion - gVehicleMenuRegion);
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


static void DisplayRepairMenu(SOLDIERTYPE const& s)
{
	RemoveBox(ghRepairBox);
	ghRepairBox = NO_POPUP_BOX;

	/* PLEASE NOTE: make sure any changes you do here are reflected in all 3
	 * routines which must remain in synch:
	 * CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and
	 * HandleShadingOfLinesForRepairMenu(). */
	PopUpBox* const box = CreateRepairBox();

	if (s.bSectorZ == 0)
	{ // Run through list of vehicles in sector and add them to pop up box
		CFOR_EACH_VEHICLE(v)
		{
			// Don't even list the helicopter, because it's never repairable
			if (IsHelicopter(v))                         continue;
			if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;
			AddMonoString(box, pVehicleStrings[v.ubVehicleType]);
		}
	}

	if (IsRobotInThisSector(s.sSectorX, s.sSectorY, s.bSectorZ))
	{ // Robot
		AddMonoString(box, pRepairStrings[3]);
	}

	AddMonoString(box, pRepairStrings[0]); // Items
	AddMonoString(box, pRepairStrings[2]); // Cancel

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
	CheckAndUpdateTacticalAssignmentPopUpPositions();
}


static void HandleShadingOfLinesForRepairMenu()
{
	if (!fShowRepairMenu) return;

	PopUpBox* const box = ghRepairBox;
	if (box == NO_POPUP_BOX) return;

	/* PLEASE NOTE: make sure any changes you do here are reflected in all 3
	 * routines, which must remain in synch:
	 * CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu() and
	 * HandleShadingOfLinesForRepairMenu(). */
	SOLDIERTYPE const& s    = *GetSelectedAssignSoldier(FALSE);
	INT32              line = 0;

	if (s.bSectorZ == 0)
	{
		CFOR_EACH_VEHICLE(v)
		{
			// don't even list the helicopter, because it's NEVER repairable...
			if (IsHelicopter(v))                         continue;
			if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;
			ShadeStringInBox(box, line++, !CanCharacterRepairVehicle(s, v));
		}
	}

	if (IsRobotInThisSector(s.sSectorX, s.sSectorY, s.bSectorZ))
	{
		// handle shading of repair robot option
		ShadeStringInBox(box, line++, !CanCharacterRepairRobot(&s));
	}

	ShadeStringInBox(box, line++, !DoesCharacterHaveAnyItemsToRepair(&s, FINAL_REPAIR_PASS));
}


static void RepairMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RepairMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void MakeRepairRegion(const INT32 idx, const UINT16 x, const UINT16 y, const UINT16 w, const UINT16 h, const UINT32 data)
{
	MOUSE_REGION* const r = &gRepairMenuRegion[idx];
	MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);
	MSYS_SetRegionUserData(r, 0, idx);
	MSYS_SetRegionUserData(r, 1, data);
}


static void CreateDestroyMouseRegionForRepairMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	if (fShowRepairMenu && !fCreated)
	{
		CheckAndUpdateTacticalAssignmentPopUpPositions();

		SOLDIERTYPE const& s = *GetSelectedAssignSoldier(FALSE);

		PopUpBox* const  box  = ghRepairBox;
		SGPBox    const& area = GetBoxArea(box);
		UINT16    const  x    = area.x;
		UINT16           y    = area.y + GetTopMarginSize(ghAssignmentBox); // XXX wrong box?
		UINT16    const  w    = area.w;
		UINT16    const  h    = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));
		INT32            idx  = 0;

		// PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must remain in synch:
		// CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and HandleShadingOfLinesForRepairMenu().

		if (s.bSectorZ == 0)
		{
			// vehicles
			CFOR_EACH_VEHICLE(v)
			{
				// don't even list the helicopter, because it's NEVER repairable...
				if (IsHelicopter(v)) continue;

				// other vehicles *in the sector* are listed, but later shaded dark if they're not repairable
				if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;

				// add mouse region for each line of text..and set user data
				MakeRepairRegion(idx++, x, y, w, h, VEHICLE2ID(v));
				y += h;
			}
		}

		// robot
		if (IsRobotInThisSector(s.sSectorX, s.sSectorY, s.bSectorZ))
		{
			MakeRepairRegion(idx++, x, y, w, h, REPAIR_MENU_ROBOT);
			y += h;
		}

		// items
		MakeRepairRegion(idx++, x, y, w, h, REPAIR_MENU_ITEMS);
		y += h;

		// cancel
		MakeRepairRegion(idx, x, y, w, h, REPAIR_MENU_CANCEL);

		PauseGame();

		// unhighlight all strings in box
		UnHighLightBox(box);

		fCreated = TRUE;
	}
	else if ((!fShowRepairMenu || !fShowAssignmentMenu) && fCreated)
	{
		fCreated = FALSE;

		// remove these regions
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghRepairBox); ++i)
		{
			MSYS_RemoveRegion(&gRepairMenuRegion[i]);
		}

		fShowRepairMenu = FALSE;

		SetRenderFlags(RENDER_FLAG_FULL);

		HideBox(ghRepairBox);

		// Remove highlight on the parent menu
		if (fShowAssignmentMenu) UnHighLightBox(ghAssignmentBox);
	}
}


static void PreChangeAssignment(SOLDIERTYPE& s)
{
	if (s.bAssignment == VEHICLE) TakeSoldierOutOfVehicle(&s);
	RemoveCharacterFromSquads(&s);
}


static bool AssignMercToAMovementGroup(SOLDIERTYPE&);


static void RepairMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iRepairWhat;


	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	// ignore clicks on disabled lines
	if (GetBoxShadeFlag(ghRepairBox, iValue)) return;

	// WHAT is being repaired is stored in the second user data argument
	iRepairWhat = MSYS_GetRegionUserData( pRegion, 1 );


	pSoldier = GetSelectedAssignSoldier( FALSE );


	if ( pSoldier && pSoldier->bActive && ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP ) )
	{
		if( ( iRepairWhat >= REPAIR_MENU_VEHICLE1 ) && ( iRepairWhat <= REPAIR_MENU_VEHICLE3 ) )
		{
			// repair VEHICLE
			PreChangeAssignment(*pSoldier);

			if( ( pSoldier->bAssignment != REPAIR )|| ( pSoldier -> fFixingRobot ) )
			{
				SetTimeOfAssignmentChangeForMerc( pSoldier );
			}

			MakeSureToolKitIsInHand( pSoldier );

			ChangeSoldiersAssignment( pSoldier, REPAIR );

			pSoldier -> bVehicleUnderRepairID = ( INT8 ) iRepairWhat;

			MakeSureToolKitIsInHand( pSoldier );

			// assign to a movement group
			AssignMercToAMovementGroup(*pSoldier);

			// set assignment for group
			SetAssignmentForList( ( INT8 ) REPAIR, 0 );
			fShowAssignmentMenu = FALSE;

		}
		else if( iRepairWhat == REPAIR_MENU_ROBOT )
		{
			// repair ROBOT
			PreChangeAssignment(*pSoldier);
			MakeSureToolKitIsInHand( pSoldier );

			if (pSoldier->bAssignment != REPAIR || !pSoldier->fFixingRobot)
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
			AssignMercToAMovementGroup(*pSoldier);
		}
		else if( iRepairWhat == REPAIR_MENU_ITEMS )
		{
			// items
			SetSoldierAssignmentRepair(*pSoldier, FALSE, -1);

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
			if (!GetBoxShadeFlag(ghRepairBox, iValue))
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


// updates which menus are selectable based on character status
void HandleShadingOfLinesForAssignmentMenus()
{
	if (!fShowAssignmentMenu || ghAssignmentBox == NO_POPUP_BOX) return;

	SOLDIERTYPE const* const s = GetSelectedAssignSoldier(FALSE);
	if (s && s->bActive)
	{
		if (s->ubWhatKindOfMercAmI == MERC_TYPE__EPC)
		{
			PopUpBox* const box = ghEpcBox;
			ShadeStringInBox(box, EPC_MENU_PATIENT, !CanCharacterPatient(s));
			ShadeStringInBox(box, EPC_MENU_ON_DUTY, !CanCharacterOnDuty(s));
			ShadeStringInBox(box, EPC_MENU_VEHICLE, !CanCharacterVehicle(*s));
		}
		else
		{
			PopUpBox* const box = ghAssignmentBox;

			{ // doctor
				PopUpShade const shade =
					!BasicCanCharacterDoctor(s) ? POPUP_SHADE           :
					!CanCharacterDoctor(s)      ? POPUP_SHADE_SECONDARY :
									POPUP_SHADE_NONE;
				ShadeStringInBox(box, ASSIGN_MENU_DOCTOR, shade);
			}

			{ // repair
				PopUpShade const shade =
					!BasicCanCharacterRepair(s) ? POPUP_SHADE           :
					!CanCharacterRepair(s)      ? POPUP_SHADE_SECONDARY :
									POPUP_SHADE_NONE;
				ShadeStringInBox(box, ASSIGN_MENU_REPAIR, shade);
			}

			ShadeStringInBox(box, ASSIGN_MENU_PATIENT, !CanCharacterPatient(s));
			ShadeStringInBox(box, ASSIGN_MENU_ON_DUTY, !CanCharacterOnDuty(s));
			ShadeStringInBox(box, ASSIGN_MENU_TRAIN,   !CanCharacterPractise(s));
			ShadeStringInBox(box, ASSIGN_MENU_VEHICLE, !CanCharacterVehicle(*s));
		}
	}

	HandleShadingOfLinesForSquadMenu();
	HandleShadingOfLinesForVehicleMenu();
	HandleShadingOfLinesForRepairMenu();
	HandleShadingOfLinesForTrainingMenu();
	HandleShadingOfLinesForAttributeMenus();
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


static void ShowAssignmentBox(void)
{
	if (fInMapMode)
	{
		const SOLDIERTYPE* const s = GetSelectedInfoChar();
		if (s->bLife == 0 || s->bAssignment == ASSIGNMENT_POW)
		{
			ShowBox(ghRemoveMercAssignBox);
			return;
		}
	}

	SOLDIERTYPE const& s = *GetSelectedAssignSoldier(FALSE);
	if (s.ubWhatKindOfMercAmI == MERC_TYPE__EPC)
	{
		ShowBox(ghEpcBox);
	}
	else
	{
		ShowBox(ghAssignmentBox);
	}
}


static void CreateDestroyMouseRegionsForTrainingMenu(void);
static void CreateDestroyMouseRegionsForAttributeMenu(void);
static void CreateDestroyMouseRegionsForSquadMenu();
static BOOLEAN HandleShowingOfMovementBox(void);


void DetermineWhichAssignmentMenusCanBeShown(void)
{
	BOOLEAN fCharacterNoLongerValid = FALSE;

	if (fInMapMode)
	{
		if (fShowMapScreenMovementList)
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
	if (!fShowAssignmentMenu || fCharacterNoLongerValid)
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
		CreateDestroyMouseRegionsForSquadMenu();
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
	CreateDestroyMouseRegionsForSquadMenu();
	CreateDestroyMouseRegionForRepairMenu(  );

	ShowAssignmentBox();

	// TRAINING menu
	if (fShowTrainingMenu)
	{
		HandleShadingOfLinesForTrainingMenu( );
		ShowBox( ghTrainingBox );
	}
	else
	{
		HideBoxIfShownMap(ghTrainingBox);
	}

	// REPAIR menu
	if (fShowRepairMenu)
	{
		HandleShadingOfLinesForRepairMenu( );
		ShowBox( ghRepairBox );
	}
	else
	{
		HideBoxIfShownMap(ghRepairBox);
	}

	// ATTRIBUTE menu
	if (fShowAttributeMenu)
	{
		HandleShadingOfLinesForAttributeMenus( );
		ShowBox( ghAttributeBox );
	}
	else
	{
		HideBoxIfShownMap(ghAttributeBox);
	}

	// VEHICLE menu
	if (fShowVehicleMenu)
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
	if (!fCreated && (fShowAssignmentMenu || fShowContractMenu || fShowTownInfo))
	{
		MSYS_DefineRegion(&gAssignmentScreenMaskRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, AssignmentScreenMaskBtnCallback);

		// created
		fCreated = TRUE;

		if (!fInMapMode)
		{
			gAssignmentScreenMaskRegion.ChangeCursor(0);
		}

	}
	else if (fCreated && !fShowAssignmentMenu && !fShowContractMenu && !fShowTownInfo)
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
		if (fFirstClickInAssignmentScreenMask)
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
	CreateDestroyMouseRegionsForSquadMenu();
	CreateDestroyMouseRegionForRepairMenu(  );
}


static void ContractMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void ContractMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


// Create/destroy mouse regions for the map screen Contract main menu
void CreateDestroyMouseRegionsForContractMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	if (HandleRemoveMenu(bSelectedContractChar)) return;

	PopUpBox* const box = ghContractBox;
	if (fShowContractMenu && !fCreated)
	{
		if (bSelectedContractChar == -1) return;

		SetBoxXY(box, ContractPosition.iX, ContractPosition.iY);

		SGPBox const& area = GetBoxArea(box);
		UINT16 const  x    = area.x;
		UINT16        y    = area.y + GetTopMarginSize(box);
		UINT16 const  w    = area.w;
		UINT16 const  dy   = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));

		// Add mouse region for each line of text and set user data
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			MOUSE_REGION* const r = &gContractMenuRegion[i];
			MSYS_DefineRegion(r, x, y, x + w, y + dy, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, ContractMenuMvtCallback, ContractMenuBtnCallback);
			MSYS_SetRegionUserData(r, 0, i);
			y += dy;
		}

		UnHighLightBox(box); // unhighlight all strings in box
		PauseGame();

		fCreated = TRUE;
	}
	else if (!fShowContractMenu && fCreated)
	{
		// destroy
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			MSYS_RemoveRegion(&gContractMenuRegion[i]);
		}

		fShownContractMenu       = FALSE;
		fMapPanelDirty           = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;

		fCreated = FALSE;
	}
}


static void TrainingMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TrainingMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForTrainingMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	PopUpBox* const box = ghTrainingBox;
	if (!fCreated && fShowTrainingMenu)
	{
		if (guiCurrentScreen == MAP_SCREEN)
		{
			SetBoxXY(box, TrainPosition.iX, TrainPosition.iY);
		}

		HandleShadingOfLinesForTrainingMenu();
		CheckAndUpdateTacticalAssignmentPopUpPositions();

		SGPBox const& area = GetBoxArea(box);
		UINT16 const  x    = area.x;
		UINT16        y    = area.y + GetTopMarginSize(box);
		UINT16 const  w    = area.w;
		UINT16 const  h    = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));

		// define regions
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			// add mouse region for each line of text..and set user data
			MOUSE_REGION* const r = &gTrainingMenuRegion[i];
			MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 3, MSYS_NO_CURSOR, TrainingMenuMvtCallBack, TrainingMenuBtnCallback);
			MSYS_SetRegionUserData(r, 0, i);
			y += h;
		}

		UnHighLightBox(box);

		fCreated = TRUE;
	}
	else if (fCreated && (!fShowAssignmentMenu || !fShowTrainingMenu))
	{
		fShowTrainingMenu = FALSE;

		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			MSYS_RemoveRegion(&gTrainingMenuRegion[i]);
		}

		fMapPanelDirty           = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;
		HideBox(box);
		SetRenderFlags(RENDER_FLAG_FULL);

		if (fShowAssignmentMenu) UnHighLightBox(ghAssignmentBox);

		fCreated = FALSE;
	}
}


static void AttributeMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void AttributesMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForAttributeMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	// will create/destroy mouse regions for the map screen attribute  menu

	PopUpBox* const box = ghAttributeBox;
	if (!fCreated && fShowAttributeMenu)
	{
		if (fShowAssignmentMenu && guiCurrentScreen == MAP_SCREEN)
		{
			SetBoxXY(ghAssignmentBox, AssignmentPosition.iX, AssignmentPosition.iY);
		}

		HandleShadingOfLinesForAttributeMenus();
		CheckAndUpdateTacticalAssignmentPopUpPositions();

		// grab height of font
		SGPBox const& area = GetBoxArea(box);
		UINT16 const  x    = area.x;
		UINT16        y    = area.y + GetTopMarginSize(box);
		UINT16 const  w    = area.w;
		UINT16 const  h    = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));

		// define regions
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			// add mouse region for each line of text..and set user data
			MOUSE_REGION* const r = &gAttributeMenuRegion[i];
			MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, AttributeMenuMvtCallBack, AttributesMenuBtnCallback);
			MSYS_SetRegionUserData(r, 0, i);
			y += h;
		}

		UnHighLightBox(box);

		fCreated = TRUE;
	}
	else if (fCreated && (!fShowAssignmentMenu || !fShowTrainingMenu || !fShowAttributeMenu))
	{
		fShowAttributeMenu = FALSE;

		// destroy
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(box); ++i)
		{
			MSYS_RemoveRegion(&gAttributeMenuRegion[i]);
		}

		gfRenderPBInterface      = TRUE;
		fMapPanelDirty           = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;
		HideBox(box);
		SetRenderFlags(RENDER_FLAG_FULL);

		if (fShowTrainingMenu) UnHighLightBox(ghTrainingBox);

		fCreated = FALSE;
	}
}


static void RemoveMercMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void RemoveMercMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForRemoveMenu(void)
{
	static BOOLEAN fCreated = FALSE;

	// will create/destroy mouse regions for the map screen attribute  menu
	if ((fShowAssignmentMenu || fShowContractMenu) && !fCreated)
	{
		const SGPPoint* pos;
		if (fShowContractMenu)
		{
			pos = &ContractPosition;
			SetBoxXY(ghContractBox, pos->iX, pos->iY);
		}
		else
		{
			pos = &AssignmentPosition,
			SetBoxXY(ghAssignmentBox, pos->iX, pos->iY);
		}
		SetBoxXY(ghRemoveMercAssignBox, pos->iX, pos->iY);

		CheckAndUpdateTacticalAssignmentPopUpPositions();

		SGPBox const& area = GetBoxArea(ghRemoveMercAssignBox);
		INT32  const  x    = area.x;
		INT32         y    = area.y + GetTopMarginSize(ghAttributeBox);
		INT32  const  w    = area.w;
		INT32  const  h    = GetLineSpace(ghRemoveMercAssignBox) + GetFontHeight(GetBoxFont(ghRemoveMercAssignBox));

		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghRemoveMercAssignBox); ++i)
		{
			// add mouse region for each line of text..and set user data
			MOUSE_REGION* const r = &gRemoveMercAssignRegion[i];
			MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, RemoveMercMenuMvtCallBack, RemoveMercMenuBtnCallback);
			MSYS_SetRegionUserData(r, 0, i);
			y += h;
		}

		UnHighLightBox(ghRemoveMercAssignBox);

		fCreated = TRUE;
	}
	else if (fCreated && !fShowAssignmentMenu && !fShowContractMenu)
	{
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghRemoveMercAssignBox); ++i)
		{
			MSYS_RemoveRegion(&gRemoveMercAssignRegion[i]);
		}

		fShownContractMenu = FALSE;

		gfRenderPBInterface      = TRUE;
		fMapPanelDirty           = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;

		fShowAssignmentMenu = FALSE;

		fCreated = FALSE;
	}
}


static void SquadMenuMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SquadMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void CreateSquadBox(void);


static void CreateDestroyMouseRegionsForSquadMenu()
{
	static BOOLEAN fCreated = FALSE;

	if (fShowSquadMenu && !fCreated)
	{
		CreateSquadBox();
		CheckAndUpdateTacticalAssignmentPopUpPositions();

		SGPBox const& area = GetBoxArea(ghSquadBox);
		INT32  const  x    = area.x;
		INT32         y    = area.y + GetTopMarginSize(ghSquadBox);
		INT32  const  w    = area.w;
		INT32  const  h    = GetLineSpace(ghSquadBox) + GetFontHeight(GetBoxFont(ghSquadBox));

		// add mouse region for each line of text except cancel
		UINT32 lines = GetNumberOfLinesOfTextInBox(ghSquadBox);
		if (lines > 0) lines--;
		UINT32 i;
		for (i = 0; i < lines; ++i)
		{
			MOUSE_REGION* const r = &gSquadMenuRegion[i];
			MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, SquadMenuMvtCallBack, SquadMenuBtnCallback);
			MSYS_SetRegionUserData(r, 0, i);
			y += h;
		}

		// now create cancel region
		MSYS_DefineRegion(&gSquadMenuRegion[i], x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, SquadMenuMvtCallBack, SquadMenuBtnCallback);
		MSYS_SetRegionUserData(&gSquadMenuRegion[i], 0, SQUAD_MENU_CANCEL);

		ShowBox(ghSquadBox);
		UnHighLightBox(ghSquadBox);
		HandleShadingOfLinesForSquadMenu();

		fCreated = TRUE;
	}
	else if ((!fShowAssignmentMenu || !fShowSquadMenu) && fCreated)
	{
		for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghSquadBox); ++i)
		{
			MSYS_RemoveRegion(&gSquadMenuRegion[i]);
		}

		fShowSquadMenu = FALSE;

		RemoveBox(ghSquadBox);
		ghSquadBox = NO_POPUP_BOX;

		fMapPanelDirty           = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		fTeamPanelDirty          = TRUE;
		fMapScreenBottomDirty    = TRUE;
		SetRenderFlags(RENDER_FLAG_FULL);

		// remove highlight on the parent menu
		if (fShowAssignmentMenu) UnHighLightBox(ghAssignmentBox);

		fCreated = FALSE;
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

	if (HandleAssignmentExpansionAndHighLightForAssignMenu(pSoldier))
	{
		return;
	}

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// is the line shaded?..if so, don't highlight
		if( pSoldier -> ubWhatKindOfMercAmI == MERC_TYPE__EPC )
		{
			if (!GetBoxShadeFlag(ghEpcBox, iValue))
			{
				HighLightBoxLine( ghEpcBox, iValue );
			}
		}
		else
		{
			if (!GetBoxShadeFlag(ghAssignmentBox, iValue))
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
		if (!GetBoxShadeFlag(ghRemoveMercAssignBox, iValue))
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
			if (!GetBoxShadeFlag(ghContractBox, iValue))
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
			if (!GetBoxShadeFlag(ghSquadBox, iValue))
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
				giContractHighLine = -1;

				break;
			case( REMOVE_MERC ):
				bSelectedInfoChar = -1;
				StrategicRemoveMerc(*pSoldier);

				// dirty region
				fCharacterInfoPanelDirty = TRUE;
				fTeamPanelDirty = TRUE;
				fMapScreenBottomDirty = TRUE;
				gfRenderPBInterface = TRUE;

				// stop contratc glow if we are
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
				break;
		}
	}
}


// Setup the quote, then start dialogue beginning the actual leave sequence
static void BeginRemoveMercFromContract(SOLDIERTYPE* const s)
{
	if (s->bLife <= 0 || s->bAssignment == ASSIGNMENT_POW) return;

	switch (s->ubWhatKindOfMercAmI)
	{
		case MERC_TYPE__MERC:
		case MERC_TYPE__NPC:
			HandleImportantMercQuoteLocked(s, QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO);
			break;

		case MERC_TYPE__AIM_MERC:
			if (WillMercRenew(s, FALSE)) // Only do this if they want to renew
			{
				// Quote is different if he's fired in less than 48 hours
				UINT16 const quote = GetWorldTotalMin() - s->uiTimeOfLastContractUpdate < 60 * 48 ?
					QUOTE_DEPART_COMMET_CONTRACT_NOT_RENEWED_OR_TERMINATED_UNDER_48 :
					QUOTE_DEPARTING_COMMENT_CONTRACT_NOT_RENEWED_OR_48_OR_MORE;
				HandleImportantMercQuoteLocked(s, quote);
			}
			break;
	}
	MakeCharacterDialogueEventContractEnding(*s, true);

	if (GetWorldTotalMin() - s->uiTimeOfLastContractUpdate < 60 * 3)
	{
		/* This will cause him give us lame excuses for a while until he gets over
		 * it.  3-6 days (but the first 1-2 days of that are spent "returning" home)
		 */
		gMercProfiles[s->ubProfile].ubDaysOfMoraleHangover = 3 + Random(4);

		// if it's an AIM merc, word of this gets back to AIM...  Bad rep.
		if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
		{
			ModifyPlayerReputation(REPUTATION_EARLY_FIRING);
		}
	}
}


static void MercDismissConfirmCallBack(MessageBoxReturnValue const bExitValue)
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
	Assert(fInMapMode);

	SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	Assert(pSoldier);

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		fOkToClose = TRUE;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// not valid?
		if (GetBoxShadeFlag(ghContractBox, iValue)) return;

		if( iValue == CONTRACT_MENU_CANCEL )
		{
			// reset contract character and contract highlight line
			giContractHighLine =-1;
			bSelectedContractChar = -1;

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
					DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_48], MAP_SCREEN, MSG_BOX_FLAG_YESNO, MercDismissConfirmCallBack);
				}

				fOkToClose = TRUE;
				break;
		}
	}

	if (fOkToClose)
	{
		// reset contract character and contract highlight line
		giContractHighLine =-1;
		bSelectedContractChar = -1;
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

	if (HandleAssignmentExpansionAndHighLightForTrainingMenu()) return;

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string

		// do not highlight current balance
		if (!GetBoxShadeFlag(ghTrainingBox, iValue))
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
		if (!GetBoxShadeFlag(ghAttributeBox, iValue))
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


static void SquadMenuBtnCallback(MOUSE_REGION* const pRegion, INT32 const reason)
{
	// btn callback handler for assignment region
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		INT32 const value = MSYS_GetRegionUserData(pRegion, 0);

		if (value == SQUAD_MENU_CANCEL)
		{ // Stop displaying, leave
			UnHighLightBox(ghAssignmentBox);
			fShowSquadMenu           = FALSE;
			fTeamPanelDirty          = TRUE;
			fMapScreenBottomDirty    = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			gfRenderPBInterface      = TRUE;
			return;
		}

		/* Can the character join this squad?  If already in it, accept that as a
			* legal choice and exit menu */
		SOLDIERTYPE& s = *GetSelectedAssignSoldier(FALSE);
		ST::string buf;
		switch (CanCharacterSquad(s, value))
		{
			case CHARACTER_CAN_JOIN_SQUAD: // able to add, do it
			{
				bool const exiting_helicopter = InHelicopter(s);
				PreChangeAssignment(s);
				AddCharacterToSquad(&s, value);
				if (exiting_helicopter) SetSoldierExitHelicopterInsertionData(&s); // XXX TODO001D
				MakeSoldiersTacticalAnimationReflectAssignment(&s);
			}
				// fallthrough
			case CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT:
				// Stop displaying, leave
				fShowAssignmentMenu      = FALSE;
				giAssignHighLine         = -1;
				fTeamPanelDirty          = TRUE;
				fMapScreenBottomDirty    = TRUE;
				fCharacterInfoPanelDirty = TRUE;
				gfRenderPBInterface      = TRUE;
				break;

			case CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING:
				buf = st_format_printf(pMapErrorString[36], s.name, pLongAssignmentStrings[value]);
				break;
			case CHARACTER_CANT_JOIN_SQUAD_VEHICLE:
				buf = st_format_printf(pMapErrorString[37], s.name);
				break;
			case CHARACTER_CANT_JOIN_SQUAD_TOO_FAR:
				buf = st_format_printf(pMapErrorString[20], s.name, pLongAssignmentStrings[value]);
				break;
			case CHARACTER_CANT_JOIN_SQUAD_FULL:
				buf = st_format_printf(pMapErrorString[19], s.name, pLongAssignmentStrings[value]);
				break;
			default: // generic "you can't join this squad" msg
				buf = st_format_printf(pMapErrorString[38], s.name, pLongAssignmentStrings[value]);
				break;
		}
		if (!buf.empty())
		{
			DoScreenIndependantMessageBox(buf, MSG_BOX_FLAG_OK, NULL);
		}

		SetAssignmentForList(value, 0);
	}
}


static void TrainingMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	SOLDIERTYPE * pSoldier = NULL;
	ST::string sString;
	ST::string sStringA;


	pSoldier = GetSelectedAssignSoldier( FALSE );

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN ) || ( iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN ) )
	{
		if (fInMapMode && !fShowMapInventoryPool)
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
					INT8 const bTownId = GetTownIdForSector(SECTOR(pSoldier->sSectorX, pSoldier->sSectorY));

					// if it's a town sector (the following 2 errors can't happen at non-town SAM sites)
					if( bTownId != BLANK_SECTOR )
					{
						// can we keep militia in this town?
						if (!MilitiaTrainingAllowedInSector(pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ))
						{
							sString = st_format_printf(pMapErrorString[ 31 ], GCM->getTownName(bTownId));
							DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
							break;
						}

						// is the current loyalty high enough to train some?
						if (!DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(pSoldier))
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
							sStringA = GetShortSectorString(pSoldier->sSectorX, pSoldier->sSectorY);
							sString = st_format_printf(zMarksMapScreenText[20], sStringA);
						}
						else
						{
							// town
							sString = st_format_printf(zMarksMapScreenText[20], GCM->getTownName(bTownId));
						}

						DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
						break;
					}

					if ( CountMilitiaTrainersInSoldiersSector( pSoldier ) >= MAX_MILITIA_TRAINERS_PER_SECTOR )
					{
						sString = st_format_printf(gzLateLocalizedString[STR_LATE_47], MAX_MILITIA_TRAINERS_PER_SECTOR);
						DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
						break;
					}


					// PASSED ALL THE TESTS - ALLOW SOLDIER TO TRAIN MILITIA HERE
					PreChangeAssignment(*pSoldier);

					if( ( pSoldier->bAssignment != TRAIN_TOWN ) )
					{
						SetTimeOfAssignmentChangeForMerc( pSoldier );
					}

					MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

					// stop showing menu
					fShowAssignmentMenu = FALSE;
					giAssignHighLine = -1;

					ChangeSoldiersAssignment( pSoldier, TRAIN_TOWN );

					// assign to a movement group
					AssignMercToAMovementGroup(*pSoldier);
					if (!SectorInfo[SECTOR(pSoldier->sSectorX, pSoldier->sSectorY)].fMilitiaTrainingPaid)
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
				if (CanCharacterTrainTeammates(pSoldier))
				{
					// train teammates
					gbTrainingMode = TRAIN_TEAMMATE;

					// show menu
					fShowAttributeMenu = TRUE;
					DetermineBoxPositions( );
				}
			break;

			case( TRAIN_MENU_TRAIN_BY_OTHER ):
				if (CanCharacterBeTrainedByOther(pSoldier))
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
			PreChangeAssignment(*pSoldier);

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

			// train stat
			ChangeSoldiersAssignment( pSoldier, gbTrainingMode );

			// assign to a movement group
			AssignMercToAMovementGroup(*pSoldier);

			// set assignment for group
			SetAssignmentForList( gbTrainingMode, ( INT8 )iValue );
		}

		// rerender tactical stuff
		gfRenderPBInterface = TRUE;

		fTeamPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
	}
}


static bool DisplayVehicleMenu(SOLDIERTYPE const&);


static void AssignmentMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment region
	INT32 iValue = -1;
	ST::string sString;

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
						PreChangeAssignment(*pSoldier);

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

						ChangeSoldiersAssignment( pSoldier, PATIENT );
						AssignMercToAMovementGroup(*pSoldier);

						// set assignment for group
						SetAssignmentForList( ( INT8 ) PATIENT, 0 );
					}
				break;

				case( EPC_MENU_VEHICLE ):
					if (CanCharacterVehicle(*pSoldier))
					{
						if (DisplayVehicleMenu(*pSoldier))
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

						PreChangeAssignment(*pSoldier);

						if( ( pSoldier->bAssignment != DOCTOR ) )
						{
							SetTimeOfAssignmentChangeForMerc( pSoldier );
						}

						ChangeSoldiersAssignment( pSoldier, DOCTOR );

						MakeSureMedKitIsInHand( pSoldier );
						AssignMercToAMovementGroup(*pSoldier);

						MakeSoldiersTacticalAnimationReflectAssignment( pSoldier );

						// set dirty flag
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;


						// set assignment for group
						SetAssignmentForList( ( INT8 ) DOCTOR, 0 );
					}
					else if (BasicCanCharacterDoctor(pSoldier))
					{
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;
						sString = st_format_printf(zMarksMapScreenText[18], pSoldier->name);

						DoScreenIndependantMessageBox( sString , MSG_BOX_FLAG_OK, NULL );
					}

				break;
				case( ASSIGN_MENU_PATIENT ):

					// can character patient?
					if( CanCharacterPatient( pSoldier ) )
					{
						PreChangeAssignment(*pSoldier);

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

						ChangeSoldiersAssignment( pSoldier, PATIENT );

						AssignMercToAMovementGroup(*pSoldier);

						// set assignment for group
						SetAssignmentForList( ( INT8 ) PATIENT, 0 );

					}
				break;

				case( ASSIGN_MENU_VEHICLE ):
					if (CanCharacterVehicle(*pSoldier))
					{
						if (DisplayVehicleMenu(*pSoldier))
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
						fShowRepairMenu = TRUE;
						DisplayRepairMenu(*pSoldier);
					}
					else if( CanCharacterRepairButDoesntHaveARepairkit( pSoldier ) )
					{
						fTeamPanelDirty = TRUE;
						fMapScreenBottomDirty = TRUE;
						sString = st_format_printf(zMarksMapScreenText[17], pSoldier->name);

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


static PopUpBox* MakeBox(const SGPPoint pos, const UINT32 flags)
{
	return CreatePopUpBox(pos, flags | POPUP_BOX_FLAG_RESIZE, FRAME_BUFFER, guiPOPUPBORDERS, guiPOPUPTEX, 6, 6, 4, 4, 2);
}


static UINT32 GetLastSquadListedInSquadMenu(void);


// Create a pop up box for squad selection
static void CreateSquadBox(void)
{
	PopUpBox* const box = MakeBox(SquadPosition, 0);
	ghSquadBox = box;

	// add strings for box
	UINT32 const uiMaxSquad = GetLastSquadListedInSquadMenu();
	for (UINT32 i = 0; i <= uiMaxSquad; ++i)
	{
		// get info about current squad and put in  string
		ST::string buf = ST::format("{} ( {}/{} )", pSquadMenuStrings[i], NumberOfPeopleInSquad(i), NUMBER_OF_SOLDIERS_PER_SQUAD);
		AddMonoString(box, buf);
	}

	// add cancel line
	AddMonoString(box, pSquadMenuStrings[NUMBER_OF_SQUADS]);

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
	DetermineBoxPositions();

	SGPBox const& area  = GetBoxArea(box);
	INT16  const  max_y = SCREEN_HEIGHT - area.h;
	if (giBoxY > max_y)
	{
		SquadPosition.iY = max_y;
		SetBoxY(box, SquadPosition.iY);
	}
}


static void CreateEPCBox(void)
{
	// will create a pop up box for squad selection
	PopUpBox* const box = MakeBox(AssignmentPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghEpcBox = box;

	for (INT32 i = 0; i < MAX_EPC_MENU_STRING_COUNT; ++i)
	{
		AddMonoString(box, pEpcMenuStrings[i]);
	}

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);

	SGPBox const& area  = GetBoxArea(box);
	INT16  const  max_y = SCREEN_HEIGHT - area.h;
	if (giBoxY > max_y)
	{
		AssignmentPosition.iY = max_y;
		SetBoxY(box, AssignmentPosition.iY);
	}
}


static void HandleShadingOfLinesForSquadMenu(void)
{
	if (!fShowSquadMenu) return;

	PopUpBox* const box = ghSquadBox;
	if (box == NO_POPUP_BOX) return;

	SOLDIERTYPE const& s         = *GetSelectedAssignSoldier(FALSE);
	UINT32      const  max_squad = GetLastSquadListedInSquadMenu();
	for (UINT32 i = 0; i <= max_squad; ++i)
	{
		JoinSquadResult const bResult = CanCharacterSquad(s, (INT8)i);
		PopUpShade const shade =
			// Shade, if the reason doesn't have a good explanatory message
			bResult == CHARACTER_CANT_JOIN_SQUAD ? POPUP_SHADE      :
			bResult == CHARACTER_CAN_JOIN_SQUAD  ? POPUP_SHADE_NONE :
					POPUP_SHADE_SECONDARY;
		ShadeStringInBox(box, i, shade);
	}
}


static PopUpBox* CreateVehicleBox(void);


static bool DisplayVehicleMenu(SOLDIERTYPE const& s)
{
	// First, clear pop up box
	RemoveBox(ghVehicleBox);
	ghVehicleBox = NO_POPUP_BOX;

	PopUpBox* const box = CreateVehicleBox();

	// Run through list of vehicles in sector and add them to pop up box
	bool vehicle_present = false;
	CFOR_EACH_VEHICLE(v)
	{
		if (!IsThisVehicleAccessibleToSoldier(s, v)) continue;
		AddMonoString(box, pVehicleStrings[v.ubVehicleType]);
		vehicle_present = true;
	}

	// Cancel string (borrow the one in the squad menu)
	AddMonoString(box, pSquadMenuStrings[SQUAD_MENU_CANCEL]);

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
	return vehicle_present;
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


void CreateContractBox(const SOLDIERTYPE* const s)
{
	if (giBoxY != 0) ContractPosition.iY = giBoxY;

	PopUpBox* const box = MakeBox(ContractPosition, 0);
	ghContractBox = box;

	if (s)
	{
		MERCPROFILESTRUCT const* const p =
			s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ?
				&GetProfile(s->ubProfile) : 0;
		for (UINT32 i = 0; i < MAX_CONTRACT_MENU_STRING_COUNT; ++i)
		{
			INT32 salary;
			switch (i)
			{
				case CONTRACT_MENU_DAY:       salary = p ? p->sSalary          : 0; break;
				case CONTRACT_MENU_WEEK:      salary = p ? p->uiWeeklySalary   : 0; break;
				case CONTRACT_MENU_TWO_WEEKS: salary = p ? p->uiBiWeeklySalary : 0; break;

				default:
					AddMonoString(box, pContractStrings[i]);
					continue;
			}

			ST::string sDollarString = SPrintMoney(salary);
			ST::string sString = ST::format("{} ( {} )", pContractStrings[i], sDollarString);
			AddMonoString(box, sString);
		}
	}

	SetBoxTextAttrs(box);
	if (s) SetBoxLineForeground(box, 0, FONT_YELLOW);
	ResizeBoxToText(box);
}


// create attribute pop up menu for mapscreen assignments
static void CreateAttributeBox()
{
	if (giBoxY != 0) AttributePosition.iY = giBoxY;

	UpdateMapScreenAssignmentPositions();

	PopUpBox* const box = MakeBox(AttributePosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghAttributeBox = box;

	for (UINT32 i = 0; i < MAX_ATTRIBUTE_STRING_COUNT; ++i)
	{
		AddMonoString(box, pAttributeMenuStrings[i]);
	}

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
}


static void CreateTrainingBox()
{
	if (giBoxY != 0)
	{
		TrainPosition.iY = giBoxY + ASSIGN_MENU_TRAIN * GetFontHeight(MAP_SCREEN_FONT);
	}

	PopUpBox* const box = MakeBox(TrainPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghTrainingBox = box;

	for (UINT32 i = 0; i < MAX_TRAIN_STRING_COUNT; ++i)
	{
		AddMonoString(box, pTrainingMenuStrings[i]);
	}

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
	DetermineBoxPositions();
}


static void CreateAssignmentsBox()
{
	if (giBoxY != 0) AssignmentPosition.iY = giBoxY;

	PopUpBox* const box = MakeBox(AssignmentPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghAssignmentBox = box;

	// No soldier is legal here!  Gets called during every mapscreen initialization even when nobody is assign char
	SOLDIERTYPE* const s = GetSelectedAssignSoldier(TRUE);

	for (UINT32 i = 0; i < MAX_ASSIGN_STRING_COUNT; ++i)
	{
		ST::string str = pAssignMenuStrings[i];
		// if we have a soldier, and this is the squad line
		ST::string buf;
		if (i == ASSIGN_MENU_ON_DUTY && s != NULL && s->bAssignment < ON_DUTY)
		{
			// show his squad # in brackets
			buf = ST::format("{}({})", str, s->bAssignment + 1);
			str = buf;
		}
		AddMonoString(box, str);
	}

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
	DetermineBoxPositions();
}


// create remove mercbox to be placed in assignment area
void CreateMercRemoveAssignBox()
{
	PopUpBox* const box = MakeBox(AssignmentPosition, POPUP_BOX_FLAG_CENTER_TEXT);
	ghRemoveMercAssignBox = box;

	for (UINT32 i = 0; i < MAX_REMOVE_MERC_COUNT; ++i)
	{
		AddMonoString(box, pRemoveMercStrings[i]);
	}

	SetBoxTextAttrs(box);
	ResizeBoxToText(box);
}


void CreateDestroyAssignmentPopUpBoxes()
{
	static BOOLEAN fCreated = FALSE;

	if (!fCreated && fShowAssignmentMenu)
	{
		guiPOPUPBORDERS = AddVideoObjectFromFile(INTERFACEDIR "/popup.sti");
		guiPOPUPTEX     = AddVideoSurfaceFromFile(INTERFACEDIR "/popupbackground.pcx");

		// these boxes are always created while in mapscreen...
		CreateEPCBox();
		CreateAssignmentsBox();
		CreateTrainingBox();
		CreateAttributeBox();
		CreateVehicleBox();
		CreateRepairBox();

		UpdateMapScreenAssignmentPositions();

		fCreated = TRUE;
	}
	else if (fCreated && !fShowAssignmentMenu)
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

		RebuildCurrentSquad();

		gfIgnoreScrolling = FALSE;
		fCreated          = FALSE;
	}
}


void DetermineBoxPositions()
{
	// depending on how many boxes there are, reposition as needed
	if (!fShowAssignmentMenu || ghAssignmentBox == NO_POPUP_BOX) return;

	SOLDIERTYPE const* const s = GetSelectedAssignSoldier(TRUE);
	// no soldier is legal here!  Gets called during every mapscreen initialization even when nobody is assign char
	if (s == NULL) return;

	if (fInMapMode)
	{
		SGPBox const& area = GetBoxArea(ghAssignmentBox);
		gsAssignmentBoxesX = area.x;
		gsAssignmentBoxesY = area.y;
	}

	INT16 x = gsAssignmentBoxesX;
	INT16 y = gsAssignmentBoxesY;

	PopUpBox* const box = (s->ubWhatKindOfMercAmI == MERC_TYPE__EPC ? ghEpcBox : ghAssignmentBox);
	SetBoxXY(box, x, y);

	// hang it right beside the assignment/EPC box menu
	x += GetBoxArea(box).w;

	if (fShowSquadMenu && ghSquadBox != NO_POPUP_BOX)
	{
		SetBoxXY(ghSquadBox, x, y);
	}

	if (fShowRepairMenu && ghRepairBox != NO_POPUP_BOX)
	{
		CreateDestroyMouseRegionForRepairMenu();
		y += (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR;
		SetBoxXY(ghRepairBox, x, y);
	}

	if (fShowTrainingMenu && ghTrainingBox != NO_POPUP_BOX)
	{
		y += (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN;
		SetBoxXY(ghTrainingBox, x, y);
		TrainPosition.iX = x;
		TrainPosition.iY = y;

		if (fShowAttributeMenu && ghAttributeBox != NO_POPUP_BOX)
		{
			// hang it right beside the training box menu
			SGPBox const& area = GetBoxArea(ghTrainingBox);
			SetBoxXY(ghAttributeBox, area.x + area.w, area.y);
		}
	}
}


void SetTacticalPopUpAssignmentBoxXY()
{
	SOLDIERTYPE const& s = *GetSelectedAssignSoldier(FALSE);
	INT16 sX;
	INT16 sY;
	GetSoldierScreenPos(&s, &sX, &sY);

	if (sX < 0) sX = 0;
	if (sY < 0) sY = 0;
	sX += 30;

	// ATE: Check if we are past tactical viewport....
	// Use estimate widths/heights
	if (sX > g_ui.m_screenWidth  - 50)	sX -= 90;
	if (sY > g_ui.m_screenHeight)		sY = g_ui.m_screenHeight;

	gsAssignmentBoxesX = sX;
	gsAssignmentBoxesY = sY;
}


static void RepositionMouseRegions(void)
{
	INT16 const sDeltaX = gsAssignmentBoxesX - gAssignmentMenuRegion[0].RegionTopLeftX;
	INT16 const sDeltaY = gsAssignmentBoxesY - gAssignmentMenuRegion[0].RegionTopLeftY + GetTopMarginSize(ghAssignmentBox);

	// find the delta from the old to the new, and alter values accordingly
	for (UINT32 i = 0; i < GetNumberOfLinesOfTextInBox(ghAssignmentBox); ++i)
	{
		MOUSE_REGION* const r = &gAssignmentMenuRegion[i];
		r->RegionTopLeftX     += sDeltaX;
		r->RegionTopLeftY     += sDeltaY;
		r->RegionBottomRightX += sDeltaX;
		r->RegionBottomRightY += sDeltaY;
	}

	gfPausedTacticalRenderFlags = TRUE;
}


static void AdjustBoxPos(SGPBox const& assignment_area, PopUpBox* const other_box, INT32 const offset_line)
{
	SGPBox const& other_area = GetBoxArea(other_box);

	INT16 const max_x = SCREEN_WIDTH - assignment_area.w - other_area.w;
	if (gsAssignmentBoxesX > max_x)
	{
		gsAssignmentBoxesX = max_x;
		SetRenderFlags(RENDER_FLAG_FULL);
	}

	INT16 const dy    = (GetFontHeight(MAP_SCREEN_FONT) + 2) * offset_line;
	INT16 const ah    = assignment_area.h;
	INT16 const oh    = other_area.h + dy;
	INT16 const max_y = gsVIEWPORT_WINDOW_END_Y - (ah > oh ? ah : oh);
	if (gsAssignmentBoxesY > max_y)
	{
		gsAssignmentBoxesY = max_y;
		SetRenderFlags(RENDER_FLAG_FULL);
	}

	INT16 const x = gsAssignmentBoxesX + assignment_area.w;
	INT16 const y = gsAssignmentBoxesY + dy;
	SetBoxXY(other_box, x, y);
}


static void CheckAndUpdateTacticalAssignmentPopUpPositions(void)
{
	if (!fShowAssignmentMenu) return;
	if (fInMapMode)           return;

	SOLDIERTYPE const& s               = *GetSelectedAssignSoldier(FALSE);
	PopUpBox*   const  assignment_box  = s.ubWhatKindOfMercAmI == MERC_TYPE__EPC ? ghEpcBox : ghAssignmentBox;
	SGPBox      const& assignment_area = GetBoxArea(assignment_box);

	if (fShowRepairMenu)
	{
		AdjustBoxPos(assignment_area, ghRepairBox, ASSIGN_MENU_REPAIR);
	}
	else if (fShowSquadMenu)
	{
		AdjustBoxPos(assignment_area, ghSquadBox, ASSIGN_MENU_ON_DUTY);
	}
	else if (fShowAttributeMenu)
	{
		SGPBox const& train_area = GetBoxArea(ghTrainingBox);
		SGPBox const& attr_area  = GetBoxArea(ghAttributeBox);

		INT16 const max_x = SCREEN_WIDTH - assignment_area.w - train_area.w - attr_area.w;
		if (gsAssignmentBoxesX > max_x)
		{
			gsAssignmentBoxesX = max_x;
			SetRenderFlags(RENDER_FLAG_FULL);
		}

		INT16 const dy    = (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN;
		INT16 const max_y = gsVIEWPORT_WINDOW_END_Y - attr_area.h - dy;
		if (gsAssignmentBoxesY > max_y)
		{
			gsAssignmentBoxesY = max_y;
			SetRenderFlags(RENDER_FLAG_FULL);
		}

		INT16 const x = gsAssignmentBoxesX + assignment_area.w;
		INT16 const y = gsAssignmentBoxesY + dy;
		SetBoxXY(ghAttributeBox, x + train_area.w, y);
		SetBoxXY(ghTrainingBox,  x,                 y);
	}
	else if (fShowTrainingMenu)
	{
		AdjustBoxPos(assignment_area, ghTrainingBox, ASSIGN_MENU_TRAIN);
	}
	else
	{
		// just the assignment box
		INT16 const max_x = SCREEN_WIDTH - assignment_area.w;
		if (gsAssignmentBoxesX > max_x)
		{
			gsAssignmentBoxesX = max_x;
			SetRenderFlags(RENDER_FLAG_FULL);
		}

		INT16 const max_y = gsVIEWPORT_WINDOW_END_Y - assignment_area.h;
		if (gsAssignmentBoxesY > max_y)
		{
			gsAssignmentBoxesY = max_y;
			SetRenderFlags(RENDER_FLAG_FULL);
		}

		SetBoxXY(assignment_box, gsAssignmentBoxesX, gsAssignmentBoxesY);
	}

	RepositionMouseRegions();
}


static void PositionCursorForTacticalAssignmentBox(void)
{
	// position cursor over y of on duty in tactical assignments
	if (gGameSettings.fOptions[TOPTION_DONT_MOVE_MOUSE]) return;

	PopUpBox const* const  box  = ghAssignmentBox;
	INT32           const  h    = GetLineSpace(box) + GetFontHeight(GetBoxFont(box));
	SGPBox          const& area = GetBoxArea(box);
	SimulateMouseMovement(area.x + area.w - 6, area.y + h / 2 + 2);
}


static bool CharacterIsTakingItEasy(SOLDIERTYPE const&);


static void HandleRestFatigueAndSleepStatus()
{
	{ /* Run through all player characters and handle their rest, fatigue, and
		* going to sleep */
		bool   reason_added = false;
		bool   box_set_up   = false;
		UINT16 sleep_quote  = QUOTE_NEED_SLEEP;
		FOR_EACH_IN_TEAM(i, OUR_TEAM)
		{
			SOLDIERTYPE& s = *i;
			if (IsMechanical(s))                 continue;
			if (s.bAssignment == ASSIGNMENT_POW) continue;
			if (s.bAssignment == IN_TRANSIT)     continue;

			/* If character can sleep, he doesn't actually have to be put asleep to
			 * get some rest, many other assignments and conditions allow for
			 * automatic recovering from fatigue. */
			if (CharacterIsTakingItEasy(s))
			{ // Let them rest some
				RestCharacter(&s);
			}
			else
			{ // Wear them down
				FatigueCharacter(s);
			}

			// Check for mercs going to sleep
			if (s.fMercAsleep) continue;

			// If dead tired
			if (s.bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM)
			{
				/* If between sectors, don't put tired mercs to sleep, will be handled
					* when they arrive at the next sector */
				if (s.fBetweenSectors) continue;

				/* He goes to sleep, provided it's at all possible (it still won't happen
					* in a hostile sector, etc.) */
				if (!SetMercAsleep(s, false)) continue;

				if (s.bAssignment < ON_DUTY || s.bAssignment == VEHICLE)
				{ // On a squad/vehicle, complain, then drop
					TacticalCharacterDialogue(&s, QUOTE_NEED_SLEEP);
					MakeCharacterDialogueEventSleep(s, true);
					sleep_quote = QUOTE_ME_TOO;
				}

				// Guy collapses
				s.fMercCollapsedFlag = TRUE;
			}
			else if (s.bBreathMax < BREATHMAX_PRETTY_TIRED && !s.fForcedToStayAwake)
			{ // Pretty tired, and not forced to stay awake
				if (s.bAssignment >= ON_DUTY && s.bAssignment != VEHICLE)
				{ // Not on squad/in vehicle
					// Try to go to sleep on your own
					if (!SetMercAsleep(s, false)) continue;

					if (!gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION]) continue;

					// If the first one
					if (!reason_added)
					{ // Tell player about it
						AddReasonToWaitingListQueue(ASLEEP_GOING_AUTO_FOR_UPDATE);
						reason_added = true;
					}

					AddSoldierToWaitingListQueue(s);
					box_set_up = true;
				}
				else
				{ // Tired, in a squad / vehicle
					if (s.fComplainedThatTired) continue;
					// He hasn't complained yet

					TacticalCharacterDialogue(&s, sleep_quote);
					sleep_quote            = QUOTE_ME_TOO;
					s.fComplainedThatTired = TRUE;
				}
			}
		}

		if (box_set_up) AddDisplayBoxToWaitingQueue();
	}

	{ /* Now handle waking. Needs seperate list queue, that's why it has its own
		* loop */
		bool box_set_up   = false;
		bool reason_added = false;
		FOR_EACH_IN_TEAM(i, OUR_TEAM)
		{
			SOLDIERTYPE& s = *i;
			if (IsMechanical(s))                 continue;
			if (s.bAssignment == ASSIGNMENT_POW) continue;
			if (s.bAssignment == IN_TRANSIT)     continue;
			// Guys between sectors CAN wake up while between sectors (sleeping in vehicle)

			if (s.bBreathMax >= BREATHMAX_CANCEL_COLLAPSE)
			{ // Reset the collapsed flag well before reaching the wakeup state
				s.fMercCollapsedFlag = FALSE;
			}

			if (!s.fMercAsleep) continue;

			// Had enough rest?
			if (s.bBreathMax < BREATHMAX_FULLY_RESTED) continue;

			// Try to wake merc up
			if (!SetMercAwake(&s, FALSE, FALSE)) continue;

			// If not on squad/in vehicle, tell player about it
			if (s.bAssignment < ON_DUTY || s.bAssignment == VEHICLE) continue;

			if (!gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION]) continue;

			if (!reason_added)
			{
				AddReasonToWaitingListQueue(ASSIGNMENT_RETURNING_FOR_UPDATE);
				reason_added = true;
			}

			AddSoldierToWaitingListQueue(s);
			box_set_up = true;
		}

		if (box_set_up)
		{
			AddDisplayBoxToWaitingQueue();
		}
	}
}


// Can the character repair this vehicle?
static bool CanCharacterRepairVehicle(SOLDIERTYPE const& s, VEHICLETYPE const& v)
{
	// is vehicle destroyed?
	if (v.fDestroyed) return false;

	// is it damaged at all?
	if (!DoesVehicleNeedAnyRepairs(v)) return false;

	// same sector, neither is between sectors, and OK To Use (player owns it) ?
	if (!IsThisVehicleAccessibleToSoldier(s, v)) return false;

#if 0 // Assignment distance limits removed.  Sep/11/98.  ARM
	// If currently loaded sector, are we close enough?
	if (s.sSectorX == gWorldSectorX  &&
			s.sSectorY == gWorldSectorY  &&
			s.bSectorZ == gbWorldSectorZ &&
			PythSpacesAway(s.sGridNo, v.sGridNo) > MAX_DISTANCE_FOR_REPAIR)
	{
		return false;
	}
#endif

	return true;
}


static SOLDIERTYPE* GetRobotSoldier(void);


static BOOLEAN IsRobotInThisSector(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	SOLDIERTYPE const* const s = GetRobotSoldier();
	return
		s                       &&
		s->sSectorX == sSectorX &&
		s->sSectorY == sSectorY &&
		s->bSectorZ == bSectorZ &&
		!s->fBetweenSectors;
}


static SOLDIERTYPE* GetRobotSoldier(void)
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (AM_A_ROBOT(s)) return s;
	}
	return NULL;
}


// can soldier repair robot
static BOOLEAN CanCharacterRepairRobot(SOLDIERTYPE const* const pSoldier)
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
	if (!IsRobotInThisSector(pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ))
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


static UINT8 HandleRepairOfRobotBySoldier(UINT8 const ubRepairPts, BOOLEAN* const pfNothingLeftToRepair)
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


static void PreSetAssignment(SOLDIERTYPE& s, INT8 const assignment)
{
	PreChangeAssignment(s);
	fTeamPanelDirty       = TRUE;
	fMapScreenBottomDirty = TRUE;
	gfRenderPBInterface   = TRUE;
}


static void PostSetAssignment(SOLDIERTYPE& s, INT8 const assignment)
{
	ChangeSoldiersAssignment(&s, assignment);
	AssignMercToAMovementGroup(s);
}


void SetSoldierAssignmentHospital(SOLDIERTYPE& s)
{
	if (!CanCharacterPatient(&s)) return;
	PreSetAssignment(s, ASSIGNMENT_HOSPITAL);
	s.bBleeding = 0;
	if (s.bAssignment != ASSIGNMENT_HOSPITAL) SetTimeOfAssignmentChangeForMerc(&s);
	RebuildCurrentSquad();
	PostSetAssignment(s, ASSIGNMENT_HOSPITAL);
}


static void SetSoldierAssignmentPatient(SOLDIERTYPE& s)
{
	if (!CanCharacterPatient(&s)) return;
	PreSetAssignment(s, PATIENT);
	if (s.bAssignment != PATIENT) SetTimeOfAssignmentChangeForMerc(&s);
	PostSetAssignment(s, PATIENT);
}


static void SetSoldierAssignmentDoctor(SOLDIERTYPE& s)
{
	if (!CanCharacterDoctor(&s)) return;
	PreSetAssignment(s, DOCTOR);
	if (s.bAssignment != DOCTOR) SetTimeOfAssignmentChangeForMerc(&s);
	MakeSureMedKitIsInHand(&s);
	PostSetAssignment(s, DOCTOR);
}


static void SetSoldierAssignmentTrainTown(SOLDIERTYPE& s)
{
	if (!CanCharacterTrainMilitia(&s)) return;
	PreSetAssignment(s, TRAIN_TOWN);
	if (s.bAssignment != TRAIN_TOWN) SetTimeOfAssignmentChangeForMerc(&s);
	if (!pMilitiaTrainerSoldier &&
			!SectorInfo[SECTOR(s.sSectorX, s.sSectorY)].fMilitiaTrainingPaid)
	{
		// show a message to confirm player wants to charge cost
		HandleInterfaceMessageForCostOfTrainingMilitia(&s);
	}
	PostSetAssignment(s, TRAIN_TOWN);
}


static void SetSoldierAssignmentTrainSelf(SOLDIERTYPE& s, INT8 const stat)
{
	if (!CanCharacterTrainStat(&s, stat, TRUE, FALSE)) return;
	PreSetAssignment(s, TRAIN_SELF);
	if (s.bAssignment != TRAIN_SELF) SetTimeOfAssignmentChangeForMerc(&s);
	// set stat to train
	s.bTrainStat = stat;
	PostSetAssignment(s, TRAIN_SELF);
}


static void SetSoldierAssignmentTrainTeammate(SOLDIERTYPE& s, INT8 const stat)
{
	if (!CanCharacterTrainStat(&s, stat, FALSE, TRUE)) return;
	PreSetAssignment(s, TRAIN_TEAMMATE);
	if (s.bAssignment != TRAIN_TEAMMATE) SetTimeOfAssignmentChangeForMerc(&s);
	// set stat to train
	s.bTrainStat = stat;
	PostSetAssignment(s, TRAIN_TEAMMATE);
}


static void SetSoldierAssignmentTrainByOther(SOLDIERTYPE& s, INT8 const stat)
{
	if (!CanCharacterTrainStat(&s, stat, TRUE, FALSE)) return;
	PreSetAssignment(s, TRAIN_BY_OTHER);
	if (s.bAssignment != TRAIN_BY_OTHER) SetTimeOfAssignmentChangeForMerc(&s);
	// set stat to train
	s.bTrainStat = stat;
	PostSetAssignment(s, TRAIN_BY_OTHER);
}


void SetSoldierAssignmentRepair(SOLDIERTYPE& s, BOOLEAN const robot, INT8 const vehicle_id)
{
	if (!CanCharacterRepair(&s)) return;
	PreSetAssignment(s, REPAIR);
	if (s.bAssignment != REPAIR || s.fFixingRobot != robot || s.bVehicleUnderRepairID != vehicle_id)
	{
		SetTimeOfAssignmentChangeForMerc(&s);
	}
	MakeSureToolKitIsInHand(&s);
	s.fFixingRobot          = robot;
	s.bVehicleUnderRepairID = vehicle_id;
	PostSetAssignment(s, REPAIR);
}

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
	if (fShowMapScreenMovementList)
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
	if (!fShowTrainingMenu) return;

	PopUpBox* const box = ghTrainingBox;
	if (box == NO_POPUP_BOX) return;

	SOLDIERTYPE const& s = *GetSelectedAssignSoldier(FALSE);

	ShadeStringInBox(box, TRAIN_MENU_SELF,           !CanCharacterPractise(&s));
	PopUpShade const shade =
		!BasicCanCharacterTrainMilitia(&s) ? POPUP_SHADE           :
		!CanCharacterTrainMilitia(&s)      ? POPUP_SHADE_SECONDARY :
		POPUP_SHADE_NONE;
	ShadeStringInBox(box, TRAIN_MENU_TOWN,           shade);
	ShadeStringInBox(box, TRAIN_MENU_TEAMMATES,      !CanCharacterTrainTeammates(&s));
	ShadeStringInBox(box, TRAIN_MENU_TRAIN_BY_OTHER, !CanCharacterBeTrainedByOther(&s));
}


static void HandleShadingOfLinesForAttributeMenus(void)
{
	// will do the same as updateassignments...but with training pop up box strings
	if (!fShowAttributeMenu) return;

	PopUpBox* const box = ghAttributeBox;
	if (box == NO_POPUP_BOX) return;

	SOLDIERTYPE const& s = *GetSelectedAssignSoldier(FALSE);
	for (INT8 stat = 0; stat < ATTRIB_MENU_CANCEL; ++stat)
	{
		BOOLEAN stat_trainable;
		switch (gbTrainingMode)
		{
			case TRAIN_SELF:
				stat_trainable = CanCharacterTrainStat(&s, stat, TRUE, FALSE);
				break;

			case TRAIN_TEAMMATE:
				// DO allow trainers to be assigned without any partners (students)
				stat_trainable = CanCharacterTrainStat(&s, stat, FALSE, TRUE);
				break;

			case TRAIN_BY_OTHER:
				// DO allow students to be assigned without any partners (trainers)
				stat_trainable = CanCharacterTrainStat(&s, stat, TRUE, FALSE);
				break;

			default:
				Assert(FALSE);
				stat_trainable = FALSE;
				break;
		}
		ShadeStringInBox(box, stat, !stat_trainable);
	}
}


static BOOLEAN ValidTrainingPartnerInSameSectorOnAssignmentFound(SOLDIERTYPE* pTargetSoldier, INT8 bTargetAssignment, INT8 bTargetStat);


static void ReportTrainersTraineesWithoutPartners(void)
{
	// check for each instructor
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == TRAIN_TEAMMATE &&
				s->bLife > 0 &&
				!ValidTrainingPartnerInSameSectorOnAssignmentFound(s, TRAIN_BY_OTHER, s->bTrainStat))
		{
			AssignmentDone(s, TRUE, TRUE);
		}
	}

	// check each trainee
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == TRAIN_BY_OTHER &&
				s->bLife > 0 &&
				!ValidTrainingPartnerInSameSectorOnAssignmentFound(s, TRAIN_TEAMMATE, s->bTrainStat))
		{
			AssignmentDone(s, TRUE, TRUE);
		}
	}
}


bool SetMercAsleep(SOLDIERTYPE& s, bool const give_warning)
{
	if (!CanCharacterSleep(s, give_warning)) return false;
	PutMercInAsleepState(s);
	return true;
}


void PutMercInAsleepState(SOLDIERTYPE& s)
{
	if (s.fMercAsleep) return;

	if (gfWorldLoaded && s.bInSector)
	{
		UINT16 const state = guiCurrentScreen == GAME_SCREEN ? GOTO_SLEEP : SLEEPING;
		ChangeSoldierState(&s, state, 1, TRUE);
	}

	// Set merc asleep
	s.fMercAsleep            = TRUE;
	// Refresh panels
	fCharacterInfoPanelDirty = TRUE;
	fTeamPanelDirty          = TRUE;
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
			const UINT16 state = (guiCurrentScreen == GAME_SCREEN ? WKAEUP_FROM_SLEEP : STANDING);
			ChangeSoldierState(pSoldier, state, 1, TRUE);
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
	return fSectorsWithSoldiers[sSectorX + sSectorY * MAP_WORLD_X][bSectorZ];
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


BOOLEAN AnyMercInGroupCantContinueMoving(GROUP const& g)
{
	BOOLEAN group_must_stop = FALSE;
	UINT16  quote           = QUOTE_NEED_SLEEP;
	CFOR_EACH_PLAYER_IN_GROUP(i, &g)
	{
		if (!i->pSoldier) continue;
		SOLDIERTYPE& s = *i->pSoldier;

		if (!PlayerSoldierTooTiredToTravel(s)) continue;

		/* NOTE: we only complain about it if it's gonna force the group to stop
			* moving! */
		group_must_stop = TRUE;

		HandleImportantMercQuote(&s, quote);
		quote = QUOTE_ME_TOO;

		PutMercInAsleepState(s);

		// player can't wake him up right away
		s.fMercCollapsedFlag = TRUE;
	}

	return group_must_stop;
}


bool PlayerSoldierTooTiredToTravel(SOLDIERTYPE& s)
{
	// If this guy ever needs sleep at all
	if (!CanChangeSleepStatusForSoldier(&s)) return false;

	if (s.bAssignment == VEHICLE && !SoldierMustDriveVehicle(s, true)) return false;

	if (s.fMercAsleep)
	{ // Asleep, and can't be awakened?
		if (!CanCharacterBeAwakened(&s, FALSE)) return true;
	}
	else
	{ // If awake, but so tired they can't move/drive anymore
		if (s.bBreathMax < BREATHMAX_GOTTA_STOP_MOVING) return true;
	}

	return false;
}


static bool AssignMercToAMovementGroup(SOLDIERTYPE& s)
{
	// If merc doesn't have a group or is in a vehicle or on a squad assign to group
	if (s.bAssignment < ON_DUTY)     return false; // On a squad
	if (s.bAssignment == VEHICLE)    return false; // In a vehicle
	if (s.bAssignment == IN_TRANSIT) return false; // In transit
	if (s.ubGroupID != 0)            return false; // In a movement group

	GROUP& g = *CreateNewPlayerGroupDepartingFromSector(s.sSectorX, s.sSectorY);
	AddPlayerToGroup(g, s);
	return true;
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


BOOLEAN HandleSelectedMercsBeingPutAsleep(BOOLEAN const wake_up, BOOLEAN const display_warning)
{
	BOOLEAN                  success = TRUE;
	const SOLDIERTYPE* const sel     = GetSelectedInfoChar();
	CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
	{
		// if the current character in the list is valid...then grab soldier pointer for the character
		SOLDIERTYPE& s = *c->merc;
		if (&s == sel) continue;

		// don't try to put vehicles, robots, to sleep if they're also selected
		if (!CanChangeSleepStatusForSoldier(&s)) continue;

		if (wake_up)
		{
			if (!SetMercAwake(&s, FALSE, FALSE)) success = FALSE;
		}
		else
		{
			if (!SetMercAsleep(s, false)) success = FALSE;
		}
	}

	// if there was anyone processed, check for success and inform player of failure
	if (!success && display_warning)
	{
		// inform player not everyone could be woken up/put to sleep
		ST::string warning = wake_up ?
			pMapErrorString[27] : pMapErrorString[26];
		DoScreenIndependantMessageBox(warning, MSG_BOX_FLAG_OK, NULL);
	}

	return success;
}


BOOLEAN IsAnyOneOnPlayersTeamOnThisAssignment( INT8 bAssignment )
{
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
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
	UINT16 usKitPts;
	UINT32 uiKitPtsUsed;
	BOOLEAN fSomeoneStillBleedingDying = FALSE;

	FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
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
							OBJECTTYPE& kit = pDoctor->inv[iKitSlot];

							usKitPts = TotalPoints(&kit);
							if( usKitPts )
							{
								uiKitPtsUsed = VirtualSoldierDressWound(pDoctor, pSoldier, &kit, usKitPts, usKitPts);
								UseKitPoints(kit, (UINT16)uiKitPtsUsed, *pDoctor);

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

	FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
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


void SetAssignmentForList(INT8 const bAssignment, INT8 const bParam)
{
	// if not in mapscreen, there is no functionality available to change multiple assignments simultaneously!
	if (!fInMapMode) return;

	SOLDIERTYPE* const sel = bSelectedAssignChar != -1 ?
		gCharactersList[bSelectedAssignChar].merc : NULL;
	Assert(sel && sel->bActive);

	// sets assignment for the list
	BOOLEAN fNotifiedOfFailure = FALSE;
	CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE& s = *c->merc;
		if (&s == sel || s.uiStatusFlags & SOLDIER_VEHICLE) continue;

		BOOLEAN fItWorked = FALSE; // assume it's NOT gonna work
		switch (bAssignment)
		{
			case DOCTOR:
				if (CanCharacterDoctor(&s))
				{
					SetSoldierAssignmentDoctor(s);
					fItWorked = TRUE;
				}
				break;

			case PATIENT:
				if (CanCharacterPatient(&s))
				{
					SetSoldierAssignmentPatient(s);
					fItWorked = TRUE;
				}
				break;

			case VEHICLE:
				if (CanCharacterVehicle(s))
				{
					VEHICLETYPE& v = GetVehicle(bParam);
					if (IsThisVehicleAccessibleToSoldier(s, v))
					{
						// if the vehicle is FULL, then this will return FALSE!
						fItWorked = PutSoldierInVehicle(s, v);
						// failure produces its own error popup
						fNotifiedOfFailure = TRUE;
					}
				}
				break;

			case REPAIR:
				if (CanCharacterRepair(&s))
				{
					// make sure he can repair the SPECIFIC thing being repaired too (must be in its sector, for example)
					BOOLEAN const fCanFixSpecificTarget =
						sel->bVehicleUnderRepairID != -1 ? CanCharacterRepairVehicle(s, GetVehicle(sel->bVehicleUnderRepairID)) :
						s.fFixingRobot                   ? CanCharacterRepairRobot(&s)                                          : // XXX s in condition seems wrong, should probably be sel
						TRUE;
					if (fCanFixSpecificTarget)
					{
						SetSoldierAssignmentRepair(s, sel->fFixingRobot, sel->bVehicleUnderRepairID);
						fItWorked = TRUE;
					}
				}
				break;

			case TRAIN_SELF:
				if (CanCharacterTrainStat(&s, bParam, TRUE, FALSE))
				{
					SetSoldierAssignmentTrainSelf(s, bParam);
					fItWorked = TRUE;
				}
				break;

			case TRAIN_TOWN:
				if (CanCharacterTrainMilitia(&s))
				{
					SetSoldierAssignmentTrainTown(s);
					fItWorked = TRUE;
				}
				break;

			case TRAIN_TEAMMATE:
				if (CanCharacterTrainStat(&s, bParam, FALSE, TRUE))
				{
					SetSoldierAssignmentTrainTeammate(s, bParam);
					fItWorked = TRUE;
				}
				break;

			case TRAIN_BY_OTHER:
				if (CanCharacterTrainStat(&s, bParam, TRUE, FALSE))
				{
					SetSoldierAssignmentTrainByOther(s, bParam);
					fItWorked = TRUE;
				}
				break;

			case SQUAD_1:
			case SQUAD_2:
			case SQUAD_3:
			case SQUAD_4:
			case SQUAD_5:
			case SQUAD_6:
			case SQUAD_7:
			case SQUAD_8:
			case SQUAD_9:
			case SQUAD_10:
			case SQUAD_11:
			case SQUAD_12:
			case SQUAD_13:
			case SQUAD_14:
			case SQUAD_15:
			case SQUAD_16:
			case SQUAD_17:
			case SQUAD_18:
			case SQUAD_19:
			case SQUAD_20:
				switch (CanCharacterSquad(s, (INT8)bAssignment))
				{
					case CHARACTER_CAN_JOIN_SQUAD:
					{
						PreChangeAssignment(s);

						// if the squad is, between sectors, remove from old mvt group
						const SOLDIERTYPE* const t = Squad[bAssignment][0];
						if (t                        &&
								t->fBetweenSectors       &&
								s.bAssignment >= ON_DUTY &&
								s.ubGroupID != 0)
						{
							RemovePlayerFromGroup(s);
						}

						// able to add, do it
						AddCharacterToSquad(&s, bAssignment);
						fItWorked = TRUE;
						break;
					}

					// if already in it, don't report that as an error
					case CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT:
						fItWorked = TRUE;
						break;

					default: break;
				}
				break;

			default:
				// remove from current vehicle/squad, if any
				if (s.bAssignment == VEHICLE) TakeSoldierOutOfVehicle(&s);
				AddCharacterToAnySquad(&s);
				fItWorked = TRUE;
				break;
		}

		if (fItWorked)
		{
			// remove him from his old squad if he was on one
			MakeSoldiersTacticalAnimationReflectAssignment(&s);
		}
		else
		{
			// didn't work - report it once
			if (!fNotifiedOfFailure)
			{
				fNotifiedOfFailure = TRUE;
				NotifyPlayerOfAssignmentAttemptFailure(bAssignment);
			}
		}
	}

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

	CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
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
	Fact fact_to_set_to_true;
	if (GetInfoForAbandoningEPC(profile, &quote_num, &fact_to_set_to_true))
	{
		gMercProfiles[profile].ubMiscFlags |= PROFILE_MISC_FLAG_FORCENPCQUOTE;
		TacticalCharacterDialogue(s, quote_num);
		SetFactTrue(fact_to_set_to_true);
	}

	class DialogueEventRemoveEPC : public DialogueEvent
	{
		public:
			DialogueEventRemoveEPC(ProfileID const epc) : epc_(epc) {}

			bool Execute()
			{
				GetProfile(epc_).ubMiscFlags &= ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
				UnRecruitEPC(epc_);
				ReBuildCharactersList();
				return false;
			}

		private:
			ProfileID const epc_;
	};

	DialogueEvent::Add(new DialogueEventRemoveEPC(profile));
}


void UnEscortEPC(SOLDIERTYPE* const s)
{
	if (fInMapMode)
	{
		InternalUnescortEPC(s);

		SOLDIERTYPE* other;
		switch (s->ubProfile)
		{
			case JOHN: other = FindSoldierByProfileIDOnPlayerTeam(MARY); break;
			case MARY: other = FindSoldierByProfileIDOnPlayerTeam(JOHN); break;
			default:   other = NULL;                                     break;
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
		TriggerNPCWithGivenApproach(s->ubProfile, APPROACH_EPC_IN_WRONG_SECTOR);
	}
}


static bool CharacterIsTakingItEasy(SOLDIERTYPE const& s)
{
	if (s.fMercAsleep) return true;

	if (!CanCharacterSleep(s, false)) return false;

	/* On duty, but able to catch naps (either not traveling, or not the driver of
	 * the vehicle). The actual checks for this are in the "can he sleep" check
	 * above */
	if (s.bAssignment < ON_DUTY || s.bAssignment == VEHICLE) return true;

	// Healing up?
	if (s.bAssignment == PATIENT)             return true;
	if (s.bAssignment == ASSIGNMENT_HOSPITAL) return true;

	// On a real assignment, but done with it?
	if (s.fDoneAssignmentAndNothingToDoFlag) return true;

	// On assignment
	return false;
}


static UINT8 CalcSoldierNeedForSleep(SOLDIERTYPE const& s)
{
	// Base comes from profile
	UINT8 need_for_sleep = GetProfile(s.ubProfile).ubNeedForSleep;

	/* < 1/4 health -> 4 more
	 * < 2/4 health -> 2 more
	 * < 3/4 health -> 1 more */
	UINT8 const part_health = 4 * s.bLife / s.bLifeMax;
	need_for_sleep += 4U >> part_health;

	// Reduce for each Night Ops or Martial Arts trait
	need_for_sleep -= NUM_SKILL_TRAITS(&s, NIGHTOPS);
	need_for_sleep -= NUM_SKILL_TRAITS(&s, MARTIALARTS);

	if (need_for_sleep <  4) return  4;
	if (need_for_sleep > 12) return 12;
	return need_for_sleep;
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
			IsMechanical(*pSoldier) ||
			pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC)
	{
		return( FALSE );
	}

	return( TRUE );
}


static SOLDIERTYPE* GetSelectedAssignSoldier(BOOLEAN fNullOK)
{
	SOLDIERTYPE *pSoldier = NULL;

	if (fInMapMode)
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


void ResumeOldAssignment(SOLDIERTYPE* const s)
{
	/* ARM: I don't think the whole "old assignment" idea is a very good one, and
	 * I doubt the code that maintains that variable is very foolproof, plus what
	 * meaning does the old assignemnt have later, anyway?  So I'd rather just
	 * settle for putting him into any squad: */
	AddCharacterToAnySquad(s);

	// make sure the player has time to OK this before proceeding
	StopTimeCompression();

	/* Assignment has changed, redraw left side as well as the map (to update
		* on/off duty icons) */
	fTeamPanelDirty          = TRUE;
	fCharacterInfoPanelDirty = TRUE;
	fMapPanelDirty           = TRUE;
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
		FOR_EACH_IN_TEAM(pOtherSoldier, OUR_TEAM)
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
			FOR_EACH_IN_TEAM(pOtherSoldier, OUR_TEAM)
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
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(sRepairsDoneString[ 1 + ubPassType ], pSoldier->name) );

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
		if ( ( GCM->getItem(pOwnerSoldier->inv[ bPocket ].usItem)->getItemClass() == IC_GUN ) && ( pOwnerSoldier->inv[ bPocket ].bGunAmmoStatus < 0 ) )
		{
			if ( *pubRepairPtsLeft >= REPAIR_COST_PER_JAM )
			{
				*pubRepairPtsLeft -= REPAIR_COST_PER_JAM;

				pOwnerSoldier->inv [ bPocket ].bGunAmmoStatus *= -1;

				// MECHANICAL/DEXTERITY GAIN: Unjammed a gun
				StatChange(*pRepairSoldier, MECHANAMT, 5, FROM_SUCCESS);
				StatChange(*pRepairSoldier, DEXTAMT,   5, FROM_SUCCESS);

				// report it as unjammed
				if ( pRepairSoldier == pOwnerSoldier )
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_53], pRepairSoldier->name, ItemNames[pOwnerSoldier->inv[bPocket].usItem]));
				}
				else
				{
					// NOTE: may need to be changed for localized versions
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(gzLateLocalizedString[STR_LATE_54], pRepairSoldier->name, pOwnerSoldier->name, ItemNames[pOwnerSoldier->inv[bPocket].usItem]));
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
