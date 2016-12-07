#ifndef __CAMPAIGN_H
#define __CAMPAIGN_H

#include "JA2Types.h"
#include "Soldier_Profile_Type.h"


enum StatKind
{
	SALARYAMT  =  0,
	HEALTHAMT  =  1,
	AGILAMT    =  2,
	DEXTAMT    =  3,
	WISDOMAMT  =  4,
	MEDICALAMT =  5,
	EXPLODEAMT =  6,
	MECHANAMT  =  7,
	MARKAMT    =  8,
	EXPERAMT   =  9,
	STRAMT     = 10,
	LDRAMT     = 11
};

template<typename T> static inline StatKind operator +(StatKind const s, T const delta)
{
	return static_cast<StatKind>((int)s + delta);
}

static inline StatKind operator ++(StatKind& s)
{
	return s = s + 1;
}

#define FIRST_CHANGEABLE_STAT		HEALTHAMT
#define LAST_CHANGEABLE_STAT		LDRAMT
#define CHANGEABLE_STAT_COUNT		( LDRAMT - HEALTHAMT + 1 )

#define MAX_STAT_VALUE 100			// for stats and skills
#define MAXEXPLEVEL     10      // maximum merc experience level

#define SKILLS_SUBPOINTS_TO_IMPROVE		25
#define ATTRIBS_SUBPOINTS_TO_IMPROVE	50
#define LEVEL_SUBPOINTS_TO_IMPROVE		350    // per current level!	(Can't go over 6500, 10x must fit in USHORT!)

#define WORKIMPROVERATE  2      // increase to make working  mercs improve more
#define TRAINIMPROVERATE 2      // increase to make training mercs improve more

#define SALARY_CHANGE_PER_LEVEL 1.25		// Mercs salary is multiplied by this
#define MAX_DAILY_SALARY        30000		// must fit into an INT16 (32k)
#define MAX_LARGE_SALARY				500000	// no limit, really

// training cap: you can't train any stat/skill beyond this value
#define TRAINING_RATING_CAP	85

// stat change causes
enum StatChangeCause
{
	FROM_SUCCESS  = 0,
	FROM_TRAINING = 1,
	FROM_FAILURE  = 2
};

// types of experience bonus awards
enum
{
	EXP_BONUS_MINIMUM,
	EXP_BONUS_SMALL,
	EXP_BONUS_AVERAGE,
	EXP_BONUS_LARGE,
	EXP_BONUS_MAXIMUM,
	NUM_EXP_BONUS_TYPES,
};


void StatChange(SOLDIERTYPE&, StatKind, UINT16 n_chances, StatChangeCause);

void HandleUnhiredMercImprovement(MERCPROFILESTRUCT&);
void HandleUnhiredMercDeaths( INT32 iProfileID );

UINT8 CurrentPlayerProgressPercentage(void);
UINT8 HighestPlayerProgressPercentage(void);

void HourlyProgressUpdate(void);

void HandleAnyStatChangesAfterAttack( void );

void AwardExperienceBonusToActiveSquad( UINT8 ubExpBonusType );

void BuildStatChangeString(wchar_t* wString, size_t Length, wchar_t const* wName, BOOLEAN fIncrease, INT16 sPtsChanged, StatKind);

void MERCMercWentUpALevelSendEmail( UINT8 ubMercMercIdValue );

#ifdef JA2TESTVERSION
void TestDumpStatChanges(void);
#endif

#endif
