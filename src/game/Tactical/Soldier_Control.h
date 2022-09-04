#ifndef __SOLDER_CONTROL_H
#define __SOLDER_CONTROL_H


//Kris:  November 10, 1997
//Please don't change this value from 10.  It will invalidate all of the maps and soldiers.
#define MAXPATROLGRIDS  10  // *** THIS IS A DUPLICATION - MUST BE MOVED !

#include "Animation_Cache.h"
#include "JA2Types.h"
#include "Keys.h"
#include "Overhead_Types.h"
#include "Item_Types.h"

#include <string_theory/string>


// ANDREW: these are defines for OKDestanation usage - please move to approprite file
#define IGNOREPEOPLE					0
#define PEOPLETOO					1
#define ALLPEOPLE					2
#define FALLINGTEST					3

#define LOCKED_NO_NEWGRIDNO				2

constexpr ProfileID NO_PROFILE = 200;

#define BATTLE_SND_LOWER_VOLUME			1

#define TAKE_DAMAGE_GUNFIRE				1
#define TAKE_DAMAGE_BLADE				2
#define TAKE_DAMAGE_HANDTOHAND				3
#define TAKE_DAMAGE_FALLROOF				4
#define TAKE_DAMAGE_BLOODLOSS				5
#define TAKE_DAMAGE_EXPLOSION				6
#define TAKE_DAMAGE_ELECTRICITY			7
#define TAKE_DAMAGE_GAS				8
#define TAKE_DAMAGE_TENTACLES				9
#define TAKE_DAMAGE_STRUCTURE_EXPLOSION		10
#define TAKE_DAMAGE_OBJECT				11


#define SOLDIER_MULTI_SELECTED				0x00000004
#define SOLDIER_PC					0x00000008
#define SOLDIER_ATTACK_NOTICED				0x00000010
#define SOLDIER_PCUNDERAICONTROL			0x00000020
#define SOLDIER_UNDERAICONTROL				0x00000040
#define SOLDIER_DEAD					0x00000080
#define SOLDIER_LOOKFOR_ITEMS				0x00000200
#define SOLDIER_ENEMY					0x00000400
#define SOLDIER_ENGAGEDINACTION			0x00000800
#define SOLDIER_ROBOT					0x00001000
#define SOLDIER_MONSTER				0x00002000
#define SOLDIER_ANIMAL					0x00004000
#define SOLDIER_VEHICLE				0x00008000
#define SOLDIER_MULTITILE				0x00020000
#define SOLDIER_TURNINGFROMHIT				0x00080000
#define SOLDIER_BOXER					0x00100000
#define SOLDIER_LOCKPENDINGACTIONCOUNTER		0x00200000
#define SOLDIER_COWERING				0x00400000
#define SOLDIER_MUTE					0x00800000
#define SOLDIER_GASSED					0x01000000
#define SOLDIER_OFF_MAP				0x02000000
#define SOLDIER_PAUSEANIMOVE				0x04000000
#define SOLDIER_DRIVER					0x08000000
#define SOLDIER_PASSENGER				0x10000000
#define SOLDIER_NPC_DOING_PUNCH			0x20000000
#define SOLDIER_NPC_SHOOTING				0x40000000
#define SOLDIER_LOOK_NEXT_TURNSOLDIER			0x80000000


/*
#define SOLDIER_TRAIT_LOCKPICKING			0x0001
#define SOLDIER_TRAIT_HANDTOHAND			0x0002
#define SOLDIER_TRAIT_ELECTRONICS			0x0004
#define SOLDIER_TRAIT_NIGHTOPS				0x0008
#define SOLDIER_TRAIT_THROWING				0x0010
#define SOLDIER_TRAIT_TEACHING				0x0020
#define SOLDIER_TRAIT_HEAVY_WEAPS			0x0040
#define SOLDIER_TRAIT_AUTO_WEAPS			0x0080
#define SOLDIER_TRAIT_STEALTHY				0x0100
#define SOLDIER_TRAIT_AMBIDEXT				0x0200
#define SOLDIER_TRAIT_THIEF				0x0400
#define SOLDIER_TRAIT_MARTIALARTS			0x0800
#define SOLDIER_TRAIT_KNIFING				0x1000
*/
#define HAS_SKILL_TRAIT(s, t)				((s)->ubSkillTrait1 == (t) || (s)->ubSkillTrait2 == (t))
#define NUM_SKILL_TRAITS(s, t)				(((s)->ubSkillTrait1 == (t)) + ((s)->ubSkillTrait2 == (t)))

#define SOLDIER_QUOTE_SAID_IN_SHIT			0x0001
#define SOLDIER_QUOTE_SAID_LOW_BREATH			0x0002
#define SOLDIER_QUOTE_SAID_BEING_PUMMELED		0x0004
#define SOLDIER_QUOTE_SAID_NEED_SLEEP			0x0008
#define SOLDIER_QUOTE_SAID_LOW_MORAL			0x0010
#define SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES		0x0020
#define SOLDIER_QUOTE_SAID_ANNOYING_MERC		0x0040
#define SOLDIER_QUOTE_SAID_LIKESGUN			0x0080
#define SOLDIER_QUOTE_SAID_DROWNING			0x0100
#define SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK	0x0400
#define SOLDIER_QUOTE_SAID_SMELLED_CREATURE		0x0800
#define SOLDIER_QUOTE_SAID_ANTICIPATING_DANGER		0x1000
#define SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES	0x2000
#define SOLDIER_QUOTE_SAID_PERSONALITY			0x4000
#define SOLDIER_QUOTE_SAID_FOUND_SOMETHING_NICE	0x8000

#define SOLDIER_QUOTE_SAID_EXT_HEARD_SOMETHING	0x0001
#define SOLDIER_QUOTE_SAID_EXT_SEEN_CREATURE_ATTACK	0x0002
#define SOLDIER_QUOTE_SAID_EXT_USED_BATTLESOUND_HIT	0x0004
#define SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL		0x0008
#define SOLDIER_QUOTE_SAID_EXT_MIKE			0x0010
#define SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT		0x0020
#define SOLDIER_QUOTE_SAID_BUDDY_1_WITNESSED		0x0040
#define SOLDIER_QUOTE_SAID_BUDDY_2_WITNESSED		0x0080
#define SOLDIER_QUOTE_SAID_BUDDY_3_WITNESSED		0x0100


#define SOLDIER_CONTRACT_RENEW_QUOTE_NOT_USED		0
#define SOLDIER_CONTRACT_RENEW_QUOTE_89_USED		1
#define SOLDIER_CONTRACT_RENEW_QUOTE_115_USED		2


#define SOLDIER_MISC_HEARD_GUNSHOT			0x01
// make sure soldiers (esp tanks) are not hurt multiple times by explosions
#define SOLDIER_MISC_HURT_BY_EXPLOSION			0x02
// should be revealed due to xrays
#define SOLDIER_MISC_XRAYED				0x04

#define NOBLOOD					40
#define MIN_BLEEDING_THRESHOLD				12 // you're OK while <4 Yellow life bars

#define BANDAGED( s )					(s->bLifeMax - s->bLife - s->bBleeding)

// amount of time a stats is to be displayed differently, due to change
#define CHANGE_STAT_RECENTLY_DURATION			60000


#define NO_PENDING_ACTION				255
#define NO_PENDING_ANIMATION				32001
#define NO_PENDING_DIRECTION				253
#define NO_PENDING_STANCE				254
#define NO_DESIRED_HEIGHT				255


//ENUMERATIONS FOR ACTIONS
enum
{
	MERC_OPENDOOR,
	MERC_OPENSTRUCT,
	MERC_PICKUPITEM,
	MERC_PUNCH,
	MERC_KNIFEATTACK,
	MERC_GIVEAID,
	MERC_GIVEITEM,
	MERC_WAITFOROTHERSTOTRIGGER,
	MERC_CUTFFENCE,
	MERC_DROPBOMB,
	MERC_STEAL,
	MERC_TALK,
	MERC_ENTER_VEHICLE,
	MERC_REPAIR,
	MERC_RELOADROBOT,
	MERC_TAKEBLOOD,
	MERC_ATTACH_CAN,
	MERC_FUEL_VEHICLE,
};

// ENUMERATIONS FOR THROW ACTIONS
enum
{
	NO_THROW_ACTION,
	THROW_ARM_ITEM,
	THROW_TARGET_MERC_CATCH,
};

// An enumeration for playing battle sounds
enum BattleSound
{
	BATTLE_SOUND_OK1,
	BATTLE_SOUND_OK2,
	BATTLE_SOUND_COOL1,
	BATTLE_SOUND_CURSE1,
	BATTLE_SOUND_HIT1,
	BATTLE_SOUND_HIT2,
	BATTLE_SOUND_LAUGH1,
	BATTLE_SOUND_ATTN1,
	BATTLE_SOUND_DIE1,
	BATTLE_SOUND_HUMM,
	BATTLE_SOUND_NOTHING,
	BATTLE_SOUND_GOTIT,
	BATTLE_SOUND_LOWMARALE_OK1,
	BATTLE_SOUND_LOWMARALE_OK2,
	BATTLE_SOUND_LOWMARALE_ATTN1,
	BATTLE_SOUND_LOCKED,
	BATTLE_SOUND_ENEMY,
	NUM_MERC_BATTLE_SOUNDS
};


//different kinds of merc
enum
{
	MERC_TYPE__PLAYER_CHARACTER,
	MERC_TYPE__AIM_MERC,
	MERC_TYPE__MERC,
	MERC_TYPE__NPC,
	MERC_TYPE__EPC,
	MERC_TYPE__NPC_WITH_UNEXTENDABLE_CONTRACT,
	MERC_TYPE__VEHICLE,
};

// I don't care if this isn't intuitive!  The hand positions go right
// before the big pockets so we can loop through them that way. --CJC
#define NO_SLOT -1

// vehicle/human path structure
struct PathSt
{
	UINT32 uiSectorId;
	PathSt* pNext;
	PathSt* pPrev;
};


enum InvSlotPos
{
	HELMETPOS = 0,
	VESTPOS,
	LEGPOS,
	HEAD1POS,
	HEAD2POS,
	HANDPOS,
	SECONDHANDPOS,
	BIGPOCK1POS,
	BIGPOCK2POS,
	BIGPOCK3POS,
	BIGPOCK4POS,
	SMALLPOCK1POS,
	SMALLPOCK2POS,
	SMALLPOCK3POS,
	SMALLPOCK4POS,
	SMALLPOCK5POS,
	SMALLPOCK6POS,
	SMALLPOCK7POS,
	SMALLPOCK8POS, // = 18, so 19 pockets needed

	NUM_INV_SLOTS,
};

//used for color codes, but also shows the enemy type for debugging purposes
enum SoldierClass
{
	SOLDIER_CLASS_NONE,
	SOLDIER_CLASS_ADMINISTRATOR,
	SOLDIER_CLASS_ELITE,
	SOLDIER_CLASS_ARMY,
	SOLDIER_CLASS_GREEN_MILITIA,
	SOLDIER_CLASS_REG_MILITIA,
	SOLDIER_CLASS_ELITE_MILITIA,
	SOLDIER_CLASS_CREATURE,
	SOLDIER_CLASS_MINER,
};

#define SOLDIER_CLASS_ENEMY( bSoldierClass )		( ( bSoldierClass >= SOLDIER_CLASS_ADMINISTRATOR ) && ( bSoldierClass <= SOLDIER_CLASS_ARMY ) )
#define SOLDIER_CLASS_MILITIA( bSoldierClass )		( ( bSoldierClass >= SOLDIER_CLASS_GREEN_MILITIA ) && ( bSoldierClass <= SOLDIER_CLASS_ELITE_MILITIA ) )

// This macro should be used whenever we want to see if someone is neutral
// IF WE ARE CONSIDERING ATTACKING THEM.  Creatures & bloodcats will attack neutrals
// but they can't attack empty vehicles!!
#define CONSIDERED_NEUTRAL(me, them)			((them)->bNeutral && ((me)->bTeam != CREATURE_TEAM || (them)->uiStatusFlags & SOLDIER_VEHICLE))

struct KEY_ON_RING
{
	UINT8 ubKeyID{INVALID_KEY_NUMBER};
	UINT8 ubNumber{0};

	bool isValid() const { return ubKeyID != INVALID_KEY_NUMBER && ubNumber != 0; }
};


struct THROW_PARAMS
{
	float dX;
	float dY;
	float dZ;
	float dForceX;
	float dForceY;
	float dForceZ;
	float dLifeSpan;
	UINT8 ubActionCode;
	SOLDIERTYPE *target;
};

#define DELAYED_MOVEMENT_FLAG_PATH_THROUGH_PEOPLE	0x01

// reasons for being unable to continue movement
enum
{
	REASON_STOPPED_NO_APS,
	REASON_STOPPED_SIGHT,
};


enum
{
	HIT_BY_TEARGAS = 0x01,
	HIT_BY_MUSTARDGAS = 0x02,
	HIT_BY_CREATUREGAS = 0x04,
};


enum WeaponModes : INT8
{
	WM_NORMAL = 0,
	WM_BURST,
	WM_ATTACHED,
	NUM_WEAPON_MODES
};

using SoldierID = UINT8;

#define SOLDIERTYPE_NAME_LENGTH 10


struct SOLDIERTYPE
{
	// ID
	SoldierID ubID;

	// DESCRIPTION / STATS, ETC
	UINT8 ubBodyType;
	INT8 bActionPoints;
	INT8 bInitialActionPoints;

	UINT32 uiStatusFlags;

	OBJECTTYPE inv[ NUM_INV_SLOTS ];
	OBJECTTYPE *pTempObject;
	KEY_ON_RING *pKeyRing;

	INT8 bOldLife; // life at end of last turn, recorded for monster AI
	// attributes
	UINT8 bInSector;
	INT8 bFlashPortraitFrame;
	INT16 sFractLife; // fraction of life pts (in hundreths)
	INT8 bBleeding; // blood loss control variable
	INT8 bBreath; // current breath value
	INT8 bBreathMax; // max breath, affected by fatigue/sleep
	INT8 bStealthMode;

	INT16 sBreathRed; // current breath value
	BOOLEAN fDelayedMovement;

	UINT8 ubWaitActionToDo;
	INT8 ubInsertionDirection;
	// skills
	SOLDIERTYPE* opponent;
	INT8 bLastRenderVisibleValue;
	UINT8 ubAttackingHand;
	// traits
	INT16 sWeightCarriedAtTurnStart;
	ST::string name;

	INT8 bVisible; // to render or not to render...


	INT8 bActive;

	INT8 bTeam; // Team identifier

	//NEW MOVEMENT INFORMATION for Strategic Movement
	UINT8 ubGroupID; //the movement group the merc is currently part of.
	BOOLEAN fBetweenSectors; //set when the group isn't actually in a sector.
	//sSectorX and sSectorY will reflect the sector the
	//merc was at last.

	UINT8 ubMovementNoiseHeard;// 8 flags by direction

	// WORLD POSITION STUFF
	FLOAT dXPos;
	FLOAT dYPos;
	INT16 sInitialGridNo;
	INT16 sGridNo;
	UINT8 bDirection;
	INT16 sHeightAdjustment;
	INT16 sDesiredHeight;
	INT16 sTempNewGridNo; // New grid no for advanced animations
	INT8 bOverTerrainType;

	INT8 bCollapsed; // collapsed due to being out of APs
	INT8 bBreathCollapsed; // collapsed due to being out of APs

	UINT8 ubDesiredHeight;
	UINT16 usPendingAnimation;
	UINT8 ubPendingStanceChange;
	UINT16 usAnimState;
	BOOLEAN fNoAPToFinishMove;
	BOOLEAN fPausedMove;
	BOOLEAN fUIdeadMerc; // UI Flags for removing a newly dead merc
	BOOLEAN fUICloseMerc; // UI Flags for closing panels

	TIMECOUNTER UpdateCounter;
	TIMECOUNTER DamageCounter;
	TIMECOUNTER AICounter;
	TIMECOUNTER FadeCounter;

	UINT8 ubSkillTrait1;
	UINT8 ubSkillTrait2;

	INT8 bDexterity; // dexterity (hand coord) value
	INT8 bWisdom;
	SOLDIERTYPE* attacker;
	SOLDIERTYPE* previous_attacker;
	SOLDIERTYPE* next_to_previous_attacker;
	BOOLEAN fTurnInProgress;

	BOOLEAN fIntendedTarget; // intentionally shot?
	BOOLEAN fPauseAllAnimation;

	INT8 bExpLevel; // general experience level
	INT16 sInsertionGridNo;

	BOOLEAN fContinueMoveAfterStanceChange;

	AnimationSurfaceCacheType AnimCache;

	INT8 bLife; // current life (hit points or health)
	UINT8 bSide;
	INT8 bNewOppCnt;

	UINT16 usAniCode;
	UINT16 usAniFrame;
	INT16 sAniDelay;

	// MOVEMENT TO NEXT TILE HANDLING STUFF
	INT8 bAgility; // agility (speed) value
	INT16 sDelayedMovementCauseGridNo;
	INT16 sReservedMovementGridNo;

	INT8 bStrength;

	// Weapon Stuff
	INT16 sTargetGridNo;
	INT8 bTargetLevel;
	INT8 bTargetCubeLevel;
	INT16 sLastTarget;
	INT8 bTilesMoved;
	INT8 bLeadership;
	FLOAT dNextBleed;
	BOOLEAN fWarnedAboutBleeding;
	BOOLEAN fDyingComment;

	UINT8 ubTilesMovedPerRTBreathUpdate;
	UINT16 usLastMovementAnimPerRTBreathUpdate;

	BOOLEAN fTurningToShoot;
	BOOLEAN fTurningUntilDone;
	BOOLEAN fGettingHit;
	BOOLEAN fInNonintAnim;
	BOOLEAN fFlashLocator;
	INT16 sLocatorFrame;
	BOOLEAN fShowLocator;
	BOOLEAN fFlashPortrait;
	INT8 bMechanical;
	INT8 bLifeMax; // maximum life for this merc

	FACETYPE* face;


	// PALETTE MANAGEMENT STUFF
	ST::string HeadPal;
	ST::string PantsPal;
	ST::string VestPal;
	ST::string SkinPal;

	UINT16 *pShades[ NUM_SOLDIER_SHADES ]; // Shading tables
	UINT16 *pGlowShades[20];
	INT8 bMedical;
	BOOLEAN fBeginFade;
	UINT8 ubFadeLevel;
	UINT8 ubServiceCount;
	SOLDIERTYPE* service_partner;
	INT8 bMarksmanship;
	INT8 bExplosive;
	THROW_PARAMS *pThrowParams;
	BOOLEAN fTurningFromPronePosition;
	INT8 bReverse;
	LEVELNODE* pLevelNode;

	// WALKING STUFF
	INT8 bDesiredDirection;
	INT16 sDestXPos;
	INT16 sDestYPos;
	INT16 sDestination;
	INT16 sFinalDestination;
	INT8 bLevel;

	// PATH STUFF
	UINT8 ubPathingData[ MAX_PATH_LIST_SIZE ];
	UINT8 ubPathDataSize;
	UINT8 ubPathIndex;
	INT16 sBlackList;
	INT8 bAimTime;
	INT8 bShownAimTime;
	INT8 bPathStored; // good for AI to reduct redundancy
	INT8 bHasKeys; // allows AI controlled dudes to open locked doors

	UINT8 ubStrategicInsertionCode;
	UINT16 usStrategicInsertionData;

	LIGHT_SPRITE* light;
	LIGHT_SPRITE* muzzle_flash;
	INT8 bMuzFlashCount;

	INT16 sX;
	INT16 sY;

	UINT16 usOldAniState;
	INT16 sOldAniCode;

	INT8 bBulletsLeft;
	UINT8 ubSuppressionPoints;

	// STUFF FOR RANDOM ANIMATIONS
	UINT32 uiTimeOfLastRandomAction;

	// AI STUFF
	INT8 bOppList[MAX_NUM_SOLDIERS]; // AI knowledge database
	INT8 bLastAction;
	INT8 bAction;
	UINT16 usActionData;
	INT8 bNextAction;
	UINT16 usNextActionData;
	INT8 bActionInProgress;
	INT8 bAlertStatus;
	INT8 bOppCnt;
	INT8 bNeutral;
	INT8 bNewSituation;
	INT8 bNextTargetLevel;
	INT8 bOrders;
	INT8 bAttitude;
	INT8 bUnderFire;
	INT8 bShock;
	INT8 bBypassToGreen;
	INT8 bDominantDir; // AI main direction to face...
	INT8 bPatrolCnt; // number of patrol gridnos
	INT8 bNextPatrolPnt; // index to next patrol gridno
	INT16 usPatrolGrid[MAXPATROLGRIDS];// AI list for ptr->orders==PATROL
	INT16 sNoiseGridno;
	UINT8 ubNoiseVolume;
	INT8 bLastAttackHit;
	SOLDIERTYPE* xrayed_by;
	FLOAT dHeightAdjustment;
	INT8 bMorale;
	INT8 bTeamMoraleMod;
	INT8 bTacticalMoraleMod;
	INT8 bStrategicMoraleMod;
	INT8 bAIMorale;
	UINT8 ubPendingAction;
	UINT8 ubPendingActionAnimCount;
	UINT32 uiPendingActionData1;
	INT16 sPendingActionData2;
	INT8 bPendingActionData3;
	INT8 ubDoorHandleCode;
	UINT32 uiPendingActionData4;
	INT8 bInterruptDuelPts;
	INT8 bPassedLastInterrupt;
	INT8 bIntStartAPs;
	INT8 bMoved;
	INT8 bHunting;
	UINT8 ubCaller;
	INT16 sCallerGridNo;
	UINT8 bCallPriority;
	INT8 bCallActedUpon;
	INT8 bFrenzied;
	INT8 bNormalSmell;
	INT8 bMonsterSmell;
	INT8 bMobility;
	INT8 fAIFlags;

	BOOLEAN fDontChargeReadyAPs;
	UINT16 usAnimSurface;
	UINT16 sZLevel;
	BOOLEAN fPrevInWater;
	BOOLEAN fGoBackToAimAfterHit;

	INT16 sWalkToAttackGridNo;
	INT16 sWalkToAttackWalkToCost;

	BOOLEAN fForceShade;
	UINT16 *pForcedShade;

	INT8 bDisplayDamageCount;
	INT8 fDisplayDamage;
	INT16 sDamage;
	INT16 sDamageX;
	INT16 sDamageY;
	INT8 bDoBurst;
	INT16 usUIMovementMode;
	BOOLEAN fUIMovementFast;

	TIMECOUNTER BlinkSelCounter;
	TIMECOUNTER PortraitFlashCounter;
	BOOLEAN fDeadSoundPlayed;
	ProfileID ubProfile;
	UINT8 ubQuoteRecord;
	UINT8 ubQuoteActionID;
	UINT8 ubBattleSoundID;

	BOOLEAN fClosePanel;
	BOOLEAN fClosePanelToDie;
	UINT8 ubClosePanelFrame;
	BOOLEAN fDeadPanel;
	UINT8 ubDeadPanelFrame;

	INT16 sPanelFaceX;
	INT16 sPanelFaceY;

	// QUOTE STUFF
	INT8 bNumHitsThisTurn;
	UINT16 usQuoteSaidFlags;
	INT8 fCloseCall;
	INT8 bLastSkillCheck;
	INT8 ubSkillCheckAttempts;

	INT8 bStartFallDir;
	INT8 fTryingToFall;

	UINT8 ubPendingDirection;
	UINT32 uiAnimSubFlags;

	UINT8 bAimShotLocation;
	UINT8 ubHitLocation;

	UINT16* effect_shade; // Shading table for effects

	INT16 sSpreadLocations[ 6 ];
	BOOLEAN fDoSpread;
	INT16 sStartGridNo;
	INT16 sEndGridNo;
	INT16 sForcastGridno;
	INT16 sZLevelOverride;
	INT8 bMovedPriorToInterrupt;
	INT32 iEndofContractTime; // time, in global time(resolution, minutes) that merc will leave, or if its a M.E.R.C. merc it will be set to -1. -2 for NPC and player generated
	INT32 iStartContractTime;
	INT32 iTotalContractLength; // total time of AIM mercs contract or the time since last paid for a M.E.R.C. merc
	INT32 iNextActionSpecialData; // AI special action data record for the next action
	UINT8 ubWhatKindOfMercAmI; //Set to the type of character it is
	INT8 bAssignment; // soldiers current assignment
	BOOLEAN fForcedToStayAwake; // forced by player to stay awake, reset to false, the moment they are set to rest or sleep
	INT8 bTrainStat; // current stat soldier is training
	SGPSector sSector; // position on the Stategic Map
	INT32 iVehicleId; // the id of the vehicle the char is in
	PathSt* pMercPath; // Path Structure
	UINT8 fHitByGasFlags; // flags
	UINT16 usMedicalDeposit; // is there a medical deposit on merc
	UINT16 usLifeInsurance; // is there life insurance taken out on merc

	INT32 iStartOfInsuranceContract;
	UINT32 uiLastAssignmentChangeMin; // timestamp of last assignment change in minutes
	INT32 iTotalLengthOfInsuranceContract;

	UINT8 ubSoldierClass; //admin, elite, troop (creature types?)
	UINT8 ubAPsLostToSuppression;
	BOOLEAN fChangingStanceDueToSuppression;
	SOLDIERTYPE* suppressor;

	UINT8 ubCivilianGroup;

	// time changes...when a stat was changed according to GetJA2Clock();
	UINT32 uiChangeLevelTime;
	UINT32 uiChangeHealthTime;
	UINT32 uiChangeStrengthTime;
	UINT32 uiChangeDexterityTime;
	UINT32 uiChangeAgilityTime;
	UINT32 uiChangeWisdomTime;
	UINT32 uiChangeLeadershipTime;
	UINT32 uiChangeMarksmanshipTime;
	UINT32 uiChangeExplosivesTime;
	UINT32 uiChangeMedicalTime;
	UINT32 uiChangeMechanicalTime;

	UINT32 uiUniqueSoldierIdValue; // the unique value every instance of a soldier gets - 1 is the first valid value
	INT8 bBeingAttackedCount; // Being attacked counter

	INT8 bNewItemCount[ NUM_INV_SLOTS ];
	INT8 bNewItemCycleCount[ NUM_INV_SLOTS ];
	BOOLEAN fCheckForNewlyAddedItems;
	INT8 bEndDoorOpenCode;

	UINT8 ubScheduleID;
	INT16 sEndDoorOpenCodeData;
	TIMECOUNTER NextTileCounter;
	BOOLEAN fBlockedByAnotherMerc;
	INT8 bBlockedByAnotherMercDirection;
	UINT16 usAttackingWeapon;
	SOLDIERTYPE* target;
	WeaponModes bWeaponMode;
	INT8 bAIScheduleProgress;
	INT16 sOffWorldGridNo;
	ANITILE* pAniTile;
	INT8 bCamo;
	INT16 sAbsoluteFinalDestination;
	UINT8 ubHiResDirection;
	UINT8 ubLastFootPrintSound;
	INT8 bVehicleID;
	INT8 fPastXDest;
	INT8 fPastYDest;
	INT8 bMovementDirection;
	INT16 sOldGridNo;
	UINT16 usDontUpdateNewGridNoOnMoveAnimChange;
	INT16 sBoundingBoxWidth;
	INT16 sBoundingBoxHeight;
	INT16 sBoundingBoxOffsetX;
	INT16 sBoundingBoxOffsetY;
	UINT32 uiTimeSameBattleSndDone;
	INT8 bOldBattleSnd;
	BOOLEAN fContractPriceHasIncreased;
	UINT32 uiBurstSoundID;
	BOOLEAN fFixingSAMSite;
	BOOLEAN fFixingRobot;
	INT8 bSlotItemTakenFrom;
	BOOLEAN fSignedAnotherContract;
	SOLDIERTYPE* auto_bandaging_medic;
	BOOLEAN fDontChargeTurningAPs;
	SOLDIERTYPE* robot_remote_holder;
	UINT32 uiTimeOfLastContractUpdate;
	INT8 bTypeOfLastContract;
	INT8 bTurnsCollapsed;
	INT8 bSleepDrugCounter;
	UINT8 ubMilitiaKills;

	INT8 bFutureDrugEffect[2]; // value to represent effect of a needle
	INT8 bDrugEffectRate[2]; // represents rate of increase and decrease of effect
	INT8 bDrugEffect[2]; // value that affects AP & morale calc ( -ve is poorly )
	INT8 bDrugSideEffectRate[2]; // duration of negative AP and morale effect
	INT8 bDrugSideEffect[2]; // duration of negative AP and morale effect

	INT8 bBlindedCounter;
	BOOLEAN fMercCollapsedFlag;
	BOOLEAN fDoneAssignmentAndNothingToDoFlag;
	BOOLEAN fMercAsleep;
	BOOLEAN fDontChargeAPsForStanceChange;

	UINT8 ubTurnsUntilCanSayHeardNoise;
	UINT16 usQuoteSaidExtFlags;

	UINT16 sContPathLocation;
	INT8 bGoodContPath;
	INT8 bNoiseLevel;
	INT8 bRegenerationCounter;
	INT8 bRegenBoostersUsedToday;
	INT8 bNumPelletsHitBy;
	INT16 sSkillCheckGridNo;
	UINT8 ubLastEnemyCycledID;

	UINT8 ubPrevSectorID;
	UINT8 ubNumTilesMovesSinceLastForget;
	INT8 bTurningIncrement;
	UINT32 uiBattleSoundID;

	BOOLEAN fSoldierWasMoving;
	BOOLEAN fSayAmmoQuotePending;
	UINT16 usValueGoneUp;

	UINT8 ubNumLocateCycles;
	UINT8 ubDelayedMovementFlags;
	BOOLEAN fMuzzleFlash;
	const SOLDIERTYPE* CTGTTarget;

	TIMECOUNTER PanelAnimateCounter;

	INT8 bCurrentCivQuote;
	INT8 bCurrentCivQuoteDelta;
	UINT8 ubMiscSoldierFlags;
	UINT8 ubReasonCantFinishMove;

	INT16 sLocationOfFadeStart;
	UINT8 bUseExitGridForReentryDirection;

	UINT32 uiTimeSinceLastSpoke;
	UINT8 ubContractRenewalQuoteCode;
	INT16 sPreTraversalGridNo;
	UINT32 uiXRayActivatedTime;
	INT8 bTurningFromUI;
	INT8 bPendingActionData5;

	INT8 bDelayedStrategicMoraleMod;
	UINT8 ubDoorOpeningNoise;

	UINT8 ubLeaveHistoryCode;
	BOOLEAN fDontUnsetLastTargetFromTurn;
	INT8 bOverrideMoveSpeed;
	BOOLEAN fUseMoverrideMoveSpeed;

	UINT32 uiTimeSoldierWillArrive;
	BOOLEAN fUseLandingZoneForArrival;
	BOOLEAN fFallClockwise;
	INT8 bVehicleUnderRepairID;
	INT32 iTimeCanSignElsewhere;
	INT8 bHospitalPriceModifier;
	UINT32 uiStartTimeOfInsuranceContract;
	BOOLEAN fRTInNonintAnim;
	BOOLEAN fDoingExternalDeath;
	INT8 bCorpseQuoteTolerance;
	INT32 iPositionSndID;
	UINT32 uiTuringSoundID;
	UINT8 ubLastDamageReason;
	BOOLEAN fComplainedThatTired;
	INT16 sLastTwoLocations[2];
	INT32 uiTimeSinceLastBleedGrunt;
};

#define BASE_FOR_EACH_SOLDIER_INV_SLOT(type, iter, soldier) \
	for (type* iter = (soldier).inv, * const iter##__end = endof((soldier).inv); iter != iter##__end; ++iter)
#define FOR_EACH_SOLDIER_INV_SLOT(iter, soldier) \
	BASE_FOR_EACH_SOLDIER_INV_SLOT(OBJECTTYPE,       iter, soldier)
#define CFOR_EACH_SOLDIER_INV_SLOT(iter, soldier) \
	BASE_FOR_EACH_SOLDIER_INV_SLOT(OBJECTTYPE const, iter, soldier)

#define HEALTH_INCREASE					0x0001
#define STRENGTH_INCREASE				0x0002
#define DEX_INCREASE					0x0004
#define AGIL_INCREASE					0x0008
#define WIS_INCREASE					0x0010
#define LDR_INCREASE					0x0020

#define MRK_INCREASE					0x0040
#define MED_INCREASE					0x0080
#define EXP_INCREASE					0x0100
#define MECH_INCREASE					0x0200

#define LVL_INCREASE					0x0400


// TYPEDEFS FOR ANIMATION PROFILES
struct ANIM_PROF_TILE
{
	UINT16 usTileFlags;
	INT8   bTileX;
	INT8   bTileY;
};

struct ANIM_PROF_DIR
{
	UINT8 ubNumTiles;
	ANIM_PROF_TILE *pTiles;
};

struct ANIM_PROF
{
	ANIM_PROF_DIR Dirs[8];
};


struct PaletteReplacementType
{
	UINT8            ubType;
	ST::string       ID;
	UINT8            ubPaletteSize;
	SGPPaletteEntry* rgb;
};


// VARIABLES FOR PALETTE REPLACEMENTS FOR HAIR, ETC
extern UINT8*                  gubpNumReplacementsPerRange;
extern PaletteReplacementType* gpPalRep;

extern UINT8 bHealthStrRanges[];


void DeleteSoldier(SOLDIERTYPE&);
void DeleteSoldierLight(SOLDIERTYPE*);

void CreateSoldierCommon(SOLDIERTYPE&);


// Soldier Management functions, called by Event Pump.c
void EVENT_InitNewSoldierAnim(SOLDIERTYPE*, UINT16 new_state, UINT16 starting_ani_code, BOOLEAN force);

void ChangeSoldierState(SOLDIERTYPE* pSoldier, UINT16 usNewState, UINT16 usStartingAniCode, BOOLEAN fForce);

enum SetSoldierPosFlags
{
	SSP_NONE          = 0,
	SSP_NO_DEST       = 1U << 0,
	SSP_NO_FINAL_DEST = 1U << 1,
	SSP_FORCE_DELETE  = 1U << 2
};
ENUM_BITSET(SetSoldierPosFlags)

void EVENT_SetSoldierPosition(SOLDIERTYPE* s, GridNo gridno, SetSoldierPosFlags flags);
void EVENT_SetSoldierPositionNoCenter(SOLDIERTYPE* s, GridNo gridno, SetSoldierPosFlags flags);
void EVENT_SetSoldierPositionXY(SOLDIERTYPE* s, FLOAT dNewXPos, FLOAT dNewYPos, SetSoldierPosFlags flags);

void EVENT_GetNewSoldierPath( SOLDIERTYPE *pSoldier, UINT16 sDestGridNo, UINT16 usMovementAnim );
BOOLEAN EVENT_InternalGetNewSoldierPath( SOLDIERTYPE *pSoldier, UINT16 sDestGridNo, UINT16 usMovementAnim, BOOLEAN fFromUI, BOOLEAN fForceRestart );

void EVENT_SetSoldierDirection( SOLDIERTYPE *pSoldier, UINT16	usNewDirection );
void EVENT_SetSoldierDesiredDirection( SOLDIERTYPE *pSoldier, UINT16	usNewDirection );
void EVENT_SetSoldierDesiredDirectionForward(SOLDIERTYPE* s, UINT16 new_direction);
void EVENT_FireSoldierWeapon( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo );
void EVENT_SoldierGotHit(SOLDIERTYPE* pSoldier, UINT16 usWeaponIndex, INT16 sDamage, INT16 sBreathLoss, UINT16 bDirection, UINT16 sRange, SOLDIERTYPE* att, UINT8 ubSpecial, UINT8 ubHitLocation, INT16 sLocationGrid);
void EVENT_SoldierBeginBladeAttack( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );
void EVENT_SoldierBeginPunchAttack( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );
void EVENT_SoldierBeginFirstAid( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );
void EVENT_StopMerc(SOLDIERTYPE*);
void EVENT_StopMerc(SOLDIERTYPE*, GridNo, INT8 direction);
void EVENT_SoldierBeginCutFence( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );
void EVENT_SoldierBeginRepair(SOLDIERTYPE&, GridNo, UINT8 direction);
void EVENT_SoldierBeginRefuel( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );


BOOLEAN SoldierReadyWeapon(SOLDIERTYPE* pSoldier, GridNo tgt_pos, BOOLEAN fEndReady);
void SetSoldierHeight(SOLDIERTYPE*, FLOAT new_height);
void BeginSoldierClimbUpRoof( SOLDIERTYPE *pSoldier );
void BeginSoldierClimbDownRoof(SOLDIERTYPE*);
void BeginSoldierClimbFence(SOLDIERTYPE*);
void BeginSoldierClimbWindow(SOLDIERTYPE*);

BOOLEAN CheckSoldierHitRoof( SOLDIERTYPE *pSoldier );
void BeginSoldierGetup( SOLDIERTYPE *pSoldier );

// Soldier Management functions called by Overhead.c
BOOLEAN ConvertAniCodeToAniFrame( SOLDIERTYPE *pSoldier, UINT16 usAniFrame );
void TurnSoldier( SOLDIERTYPE *pSold);
void EVENT_BeginMercTurn(SOLDIERTYPE&);
void ChangeSoldierStance( SOLDIERTYPE *pSoldier, UINT8 ubDesiredStance );
void ModifySoldierAniSpeed( SOLDIERTYPE *pSoldier );
void StopSoldier( SOLDIERTYPE *pSoldier );
UINT8 SoldierTakeDamage(SOLDIERTYPE* pSoldier, INT16 sLifeDeduct, INT16 sBreathLoss, UINT8 ubReason, SOLDIERTYPE* attacker);
void ReviveSoldier( SOLDIERTYPE *pSoldier );


// Palette functions for soldiers
void  CreateSoldierPalettes(SOLDIERTYPE&);
UINT8 GetPaletteRepIndexFromID(const ST::string& pal_rep);
void SetPaletteReplacement(SGPPaletteEntry* p8BPPPalette, const ST::string& aPalRep);
void  LoadPaletteData(void);
void  DeletePaletteData(void);

// UTILITY FUNCTUIONS
void MoveMerc( SOLDIERTYPE *pSoldier, FLOAT dMovementChange, FLOAT dAngle, BOOLEAN fCheckRange );
void MoveMercFacingDirection( SOLDIERTYPE *pSoldier, BOOLEAN fReverse, FLOAT dMovementDist );
INT16 GetDirectionFromGridNo(INT16 sGridNo, const SOLDIERTYPE* pSoldier);
UINT8 atan8( INT16 sXPos, INT16 sYPos, INT16 sXPos2, INT16 sYPos2 );
INT8 CalcActionPoints(const SOLDIERTYPE*);
INT16 GetDirectionToGridNoFromGridNo( INT16 sGridNoDest, INT16 sGridNoSrc );
void ReleaseSoldiersAttacker( SOLDIERTYPE *pSoldier );
BOOLEAN MercInWater(const SOLDIERTYPE* pSoldier);
UINT16 GetMoveStateBasedOnStance(const SOLDIERTYPE*, UINT8 ubStanceHeight);
void SoldierGotoStationaryStance( SOLDIERTYPE *pSoldier );
void ReCreateSoldierLight(SOLDIERTYPE*);


void    MakeCharacterDialogueEventDoBattleSound(SOLDIERTYPE& s, BattleSound, UINT32 delay);
BOOLEAN DoMercBattleSound(SOLDIERTYPE*, BattleSound);
BOOLEAN InternalDoMercBattleSound(SOLDIERTYPE*, BattleSound, INT8 bSpecialCode);


UINT32 SoldierDressWound( SOLDIERTYPE *pSoldier, SOLDIERTYPE *pVictim, INT16 sKitPts, INT16 sStatus );
void ReceivingSoldierCancelServices( SOLDIERTYPE *pSoldier );
void GivingSoldierCancelServices( SOLDIERTYPE *pSoldier );
void InternalGivingSoldierCancelServices( SOLDIERTYPE *pSoldier, BOOLEAN fPlayEndAnim );


// WRAPPERS FOR SOLDIER EVENTS
void SendGetNewSoldierPathEvent(SOLDIERTYPE*, UINT16 sDestGridNo);
void SendSoldierSetDesiredDirectionEvent(const SOLDIERTYPE* pSoldier, UINT16 usDesiredDirection);
void SendBeginFireWeaponEvent( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo );

void HaultSoldierFromSighting( SOLDIERTYPE *pSoldier, BOOLEAN fFromSightingEnemy );
void ReLoadSoldierAnimationDueToHandItemChange( SOLDIERTYPE *pSoldier, UINT16 usOldItem, UINT16 usNewItem );

bool CheckForBreathCollapse(SOLDIERTYPE&);

static inline BOOLEAN IsOnCivTeam(const SOLDIERTYPE* const s)
{
	return s->bTeam == CIV_TEAM;
}

#define PTR_CROUCHED					(gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_CROUCH)
#define PTR_STANDING					(gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_STAND)
#define PTR_PRONE					(gAnimControl[ pSoldier->usAnimState ].ubHeight == ANIM_PRONE)


void EVENT_SoldierBeginGiveItem( SOLDIERTYPE *pSoldier );

void DoNinjaAttack( SOLDIERTYPE *pSoldier );

BOOLEAN InternalSoldierReadyWeapon( SOLDIERTYPE *pSoldier, UINT8 sFacingDir, BOOLEAN fEndReady );

void RemoveSoldierFromGridNo(SOLDIERTYPE&);

void PositionSoldierLight( SOLDIERTYPE *pSoldier );

void EVENT_InternalSetSoldierDestination( SOLDIERTYPE *pSoldier, UINT16	usNewDirection, BOOLEAN fFromMove, UINT16 usAnimState );

void ChangeToFallbackAnimation( SOLDIERTYPE *pSoldier, INT8 bDirection );

void EVENT_SoldierBeginKnifeThrowAttack( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );
void EVENT_SoldierBeginUseDetonator( SOLDIERTYPE *pSoldier );
void EVENT_SoldierBeginDropBomb( SOLDIERTYPE *pSoldier );
void EVENT_SoldierEnterVehicle(SOLDIERTYPE&, GridNo);


void SetSoldierCowerState( SOLDIERTYPE *pSoldier, BOOLEAN fOn );

BOOLEAN PlayerSoldierStartTalking( SOLDIERTYPE *pSoldier, UINT8 ubTargetID, BOOLEAN fValidate );

void CalcNewActionPoints( SOLDIERTYPE *pSoldier );

BOOLEAN InternalIsValidStance(const SOLDIERTYPE* pSoldier, INT8 bDirection, INT8 bNewStance);

void AdjustNoAPToFinishMove( SOLDIERTYPE *pSoldier, BOOLEAN fSet );


void UpdateRobotControllerGivenController( SOLDIERTYPE *pSoldier );
void UpdateRobotControllerGivenRobot( SOLDIERTYPE *pSoldier );
const SOLDIERTYPE *GetRobotController( const SOLDIERTYPE *pSoldier );
BOOLEAN CanRobotBeControlled(const SOLDIERTYPE* pSoldier);
BOOLEAN ControllingRobot(const SOLDIERTYPE* s);

void EVENT_SoldierBeginReloadRobot( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection, UINT8 ubMercSlot );

void EVENT_SoldierBeginTakeBlood( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );

void EVENT_SoldierBeginAttachCan( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubDirection );

void PickDropItemAnimation( SOLDIERTYPE *pSoldier );

bool IsValidSecondHandShot(SOLDIERTYPE const*);
bool IsValidSecondHandShotForReloadingPurposes(SOLDIERTYPE const*);

void CrowsFlyAway( UINT8 ubTeam );

void DebugValidateSoldierData(void);

void BeginTyingToFall( SOLDIERTYPE *pSoldier );

void SetSoldierAsUnderAiControl( SOLDIERTYPE *pSoldier );
void HandlePlayerTogglingLightEffects( BOOLEAN fToggleValue );

void HandleSystemNewAISituation(SOLDIERTYPE*);
void SetSoldierAniSpeed(SOLDIERTYPE* pSoldier);
void PlaySoldierFootstepSound(SOLDIERTYPE* pSoldier);
void PlayStealthySoldierFootstepSound(SOLDIERTYPE* pSoldier);

// DO NOT CALL UNLESS THROUGH EVENT_SetSoldierPosition
UINT16 PickSoldierReadyAnimation(SOLDIERTYPE* pSoldier, BOOLEAN fEndReady);

extern BOOLEAN gfGetNewPathThroughPeople;

void FlashSoldierPortrait(SOLDIERTYPE*);

static inline bool IsWearingHeadGear(SOLDIERTYPE const& s, UINT16 const item)
{
	return s.inv[HEAD1POS].usItem == item || s.inv[HEAD2POS].usItem == item;
}

#endif
