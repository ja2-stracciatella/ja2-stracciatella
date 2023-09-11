#ifndef __SOLDER_PROFILE_TYPE_H
#define __SOLDER_PROFILE_TYPE_H

#include "Types.h"
#include <string_theory/string>


#define NUM_PROFILES						170

#define NAME_LENGTH						30
#define NICKNAME_LENGTH					10


//ONLY HAVE 8 MISC FLAGS.. SHOULD BE ENOUGH
#define PROFILE_MISC_FLAG_RECRUITED				0x01
#define PROFILE_MISC_FLAG_HAVESEENCREATURE			0x02
#define PROFILE_MISC_FLAG_FORCENPCQUOTE			0x04
#define PROFILE_MISC_FLAG_WOUNDEDBYPLAYER			0x08
#define PROFILE_MISC_FLAG_TEMP_NPC_QUOTE_DATA_EXISTS		0x10
#define PROFILE_MISC_FLAG_SAID_HOSTILE_QUOTE			0x20
#define PROFILE_MISC_FLAG_EPCACTIVE				0x40
#define PROFILE_MISC_FLAG_ALREADY_USED_ITEMS			0x80 //The player has already purchased the mercs items.

#define PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR			0x01
#define PROFILE_MISC_FLAG2_LEFT_COUNTRY			0x02
#define PROFILE_MISC_FLAG2_BANDAGED_TODAY			0x04
#define PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE		0x08
#define PROFILE_MISC_FLAG2_NEEDS_TO_SAY_HOSTILE_QUOTE		0x10
#define PROFILE_MISC_FLAG2_MARRIED_TO_HICKS			0x20
#define PROFILE_MISC_FLAG2_ASKED_BY_HICKS			0x40

#define PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM	0x01 // In the aimscreen, the merc was away and the player left a message
#define PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE		0x02
#define PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE		0x04 // player's had a chance to hire this merc
#define PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL		0x08

#define PROFILE_MISC_FLAG3_NPC_PISSED_OFF			0x10
#define PROFILE_MISC_FLAG3_MERC_MERC_IS_DEAD_AND_QUOTE_SAID	0x20 // In the merc site, the merc has died and Speck quote for the dead merc has been said

#define PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH	0x40
#define PROFILE_MISC_FLAG3_GOODGUY				0x80
//
// The following variables are used with the 'bMercStatus' variable
//
//

//Merc is ready
#define MERC_OK						0

//if the merc doesnt have a EDT file
#define MERC_HAS_NO_TEXT_FILE					-1

//used in the aim video conferencing screen
#define MERC_ANNOYED_BUT_CAN_STILL_CONTACT			-2
#define MERC_ANNOYED_WONT_CONTACT				-3
#define MERC_HIRED_BUT_NOT_ARRIVED_YET				-4

//self explanatory
#define MERC_IS_DEAD						-5

//set when the merc is returning home.  A delay for 1,2 or 3 days
#define MERC_RETURNING_HOME					-6

// used when merc starts game on assignment, goes on assignment later, or leaves to go on another contract
#define MERC_WORKING_ELSEWHERE					-7

//When the merc was fired, they were a POW, make sure they dont show up in AIM, or MERC as available
#define MERC_FIRED_AS_A_POW					-8


// the values for categories of stats
#define SUPER_STAT_VALUE					80
#define NEEDS_TRAINING_STAT_VALUE				50
#define NO_CHANCE_IN_HELL_STAT_VALUE				40

#define SUPER_SKILL_VALUE					80
#define NEEDS_TRAINING_SKILL_VALUE				50
#define NO_CHANCE_IN_HELL_SKILL_VALUE				0


enum SkillTrait
{
	NO_SKILLTRAIT = 0,
	LOCKPICKING,
	HANDTOHAND,
	ELECTRONICS,
	NIGHTOPS,
	THROWING,
	TEACHING,
	HEAVY_WEAPS,
	AUTO_WEAPS,
	STEALTHY,
	AMBIDEXT,
	THIEF,
	MARTIALARTS,
	KNIFING,
	ONROOF,
	CAMOUFLAGED,
	NUM_SKILLTRAITS
};

enum PersonalityTrait
{
	NO_PERSONALITYTRAIT = 0,
	HEAT_INTOLERANT,
	NERVOUS,
	CLAUSTROPHOBIC,
	NONSWIMMER,
	FEAR_OF_INSECTS,
	FORGETFUL,
	PSYCHO
};

#define NERVOUS_RADIUS 10

enum Attitudes
{
	ATT_NORMAL = 0,
	ATT_FRIENDLY,
	ATT_LONER,
	ATT_OPTIMIST,
	ATT_PESSIMIST,
	ATT_AGGRESSIVE,
	ATT_ARROGANT,
	ATT_BIG_SHOT,
	ATT_ASSHOLE,
	ATT_COWARD,
	NUM_ATTITUDES
};

enum Sexes
{
	MALE = 0,
	FEMALE
};

enum SexistLevels
{
	NOT_SEXIST = 0,
	SOMEWHAT_SEXIST,
	VERY_SEXIST,
	GENTLEMAN
};



// training defines for evolution, no stat increase, stat decrease( de-evolve )
enum CharacterEvolution
{
	NORMAL_EVOLUTION = 0,
	NO_EVOLUTION,
	DEVOLVE,
};

#define BUDDY_MERC(prof, bud) ((prof).bBuddy[0] == (bud) || (prof).bBuddy[1] == (bud) || (prof).bBuddy[2] == (bud))
#define HATED_MERC(prof, hat) ((prof).bHated[0] == (hat) || (prof).bHated[1] == (hat) || (prof).bHated[2] == (hat))

#define BUDDY_OPINION						+25
#define HATED_OPINION						-25

struct MERCPROFILESTRUCT
{
	ST::string zName;
	ST::string zNickname;
	UINT8 ubFaceIndex;
	ST::string PANTS;
	ST::string VEST;
	ST::string SKIN;
	ST::string HAIR;
	INT8 bSex;
	INT8 bArmourAttractiveness;
	UINT8 ubMiscFlags2;
	INT8 bEvolution;
	UINT8 ubMiscFlags;
	UINT8 bSexist;
	INT8 bLearnToHate;

	// skills
	UINT8 ubQuoteRecord;
	INT8 bDeathRate;

	INT16 sExpLevelGain;
	INT16 sLifeGain;
	INT16 sAgilityGain;
	INT16 sDexterityGain;
	INT16 sWisdomGain;
	INT16 sMarksmanshipGain;
	INT16 sMedicalGain;
	INT16 sMechanicGain;
	INT16 sExplosivesGain;

	UINT8 ubBodyType;
	INT8 bMedical;

	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;
	UINT32 uiBlinkFrequency;
	UINT32 uiExpressionFrequency;
	SGPSector sSector;

	UINT32 uiDayBecomesAvailable; //day the merc will be available. used with the bMercStatus

	INT8 bStrength;

	INT8 bLifeMax;
	INT8 bExpLevelDelta;
	INT8 bLifeDelta;
	INT8 bAgilityDelta;
	INT8 bDexterityDelta;
	INT8 bWisdomDelta;
	INT8 bMarksmanshipDelta;
	INT8 bMedicalDelta;
	INT8 bMechanicDelta;
	INT8 bExplosivesDelta;
	INT8 bStrengthDelta;
	INT8 bLeadershipDelta;
	UINT16 usKills;
	UINT16 usAssists;
	UINT16 usShotsFired;
	UINT16 usShotsHit;
	UINT16 usBattlesFought;
	UINT16 usTimesWounded;
	UINT16 usTotalDaysServed;

	INT16 sLeadershipGain;
	INT16 sStrengthGain;



	// BODY TYPE SUBSITUTIONS
	UINT32 uiBodyTypeSubFlags;

	INT16 sSalary;
	INT8 bLife;
	INT8 bDexterity; // dexterity (hand coord) value
	INT8 bPersonalityTrait;
	INT8 bSkillTrait;

	INT8 bReputationTolerance;
	INT8 bExplosive;
	INT8 bSkillTrait2;
	INT8 bLeadership;

	INT8 bBuddy[5];
	INT8 bHated[5];
	INT8 bExpLevel; // general experience level

	INT8 bMarksmanship;
	INT8 bWisdom;

	UINT8 bInvStatus[19];
	UINT8 bInvNumber[19];
	UINT16 usApproachFactor[4];

	INT8 bMainGunAttractiveness;
	INT8 bAgility; // agility (speed) value

	BOOLEAN fUseProfileInsertionInfo; // Set to various flags, ( contained in TacticalSave.h )
	INT16 sGridNo; // The Gridno the NPC was in before leaving the sector
	UINT8 ubQuoteActionID;
	INT8 bMechanical;

	UINT8 ubInvUndroppable;
	UINT8 ubRoomRangeStart[2];
	UINT16 inv[19];

	UINT16 usStatChangeChances[ 12 ]; // used strictly for balancing, never shown!
	UINT16 usStatChangeSuccesses[ 12 ]; // used strictly for balancing, never shown!

	UINT8 ubStrategicInsertionCode;

	UINT8 ubRoomRangeEnd[2];

	UINT8 ubLastQuoteSaid;

	INT8 bRace;
	INT8 bNationality;
	INT8 bAppearance;
	INT8 bAppearanceCareLevel;
	INT8 bRefinement;
	INT8 bRefinementCareLevel;
	INT8 bHatedNationality;
	INT8 bHatedNationalityCareLevel;
	INT8 bRacist;
	UINT32 uiWeeklySalary;
	UINT32 uiBiWeeklySalary;
	INT8 bMedicalDeposit;
	INT8 bAttitude;
	UINT16 sMedicalDepositAmount;

	INT8 bLearnToLike;
	UINT8 ubApproachVal[4];
	UINT8 ubApproachMod[3][4];
	INT8 bTown;
	INT8 bTownAttachment;
	UINT16 usOptionalGearCost;
	INT8 bMercOpinion[75];
	INT8 bApproached;
	INT8 bMercStatus; //The status of the merc. If negative, see flags at the top of this file. Positive: The number of days the merc is away for. 0: Not hired but ready to be.
	INT8 bHatedTime[5];
	INT8 bLearnToLikeTime;
	INT8 bLearnToHateTime;
	INT8 bHatedCount[5];
	INT8 bLearnToLikeCount;
	INT8 bLearnToHateCount;
	UINT8 ubLastDateSpokenTo;
	UINT8 bLastQuoteSaidWasSpecial;
	INT8 bSectorZ;
	UINT16 usStrategicInsertionData;
	INT8 bFriendlyOrDirectDefaultResponseUsedRecently;
	INT8 bRecruitDefaultResponseUsedRecently;
	INT8 bThreatenDefaultResponseUsedRecently;
	INT8 bNPCData; // NPC specific
	INT32 iBalance;
	UINT8 ubCivilianGroup;
	UINT8 ubNeedForSleep;
	UINT32 uiMoney;
	INT8 bNPCData2; // NPC specific

	UINT8 ubMiscFlags3;

	UINT8 ubDaysOfMoraleHangover; // used only when merc leaves team while having poor morale
	UINT8 ubNumTimesDrugUseInLifetime; // The # times a drug has been used in the player's lifetime...

	// Flags used for the precedent to repeating oneself in Contract negotiations. Used for quote 80 - ~107. Gets reset every day
	UINT32 uiPrecedentQuoteSaid;
	INT16 sPreCombatGridNo;
	UINT8 ubTimeTillNextHatedComplaint;
	UINT8 ubSuspiciousDeath;

	INT32 iMercMercContractLength; //Used for MERC mercs, specifies how many days the merc has gone since last page

	UINT32 uiTotalCostToDate; // The total amount of money that has been paid to the merc for their salary
};


static inline bool HasSkillTrait(MERCPROFILESTRUCT const& p, SkillTrait const skill)
{
	return p.bSkillTrait == skill || p.bSkillTrait2 == skill;
}


#define TIME_BETWEEN_HATED_COMPLAINTS				24

#define SUSPICIOUS_DEATH					1
#define VERY_SUSPICIOUS_DEATH					2

#endif
