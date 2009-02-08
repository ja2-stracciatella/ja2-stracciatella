#ifndef __SKILL_CHECK_H
#define __SKILL_CHECK_H

#include "JA2Types.h"


void ReducePointsForFatigue(const SOLDIERTYPE* s, UINT16* pusPoints);
extern INT32 GetSkillCheckPenaltyForFatigue( SOLDIERTYPE *pSoldier, INT32 iSkill );
extern INT32 SkillCheck( SOLDIERTYPE *pSoldier, INT8 bReason, INT8 bDifficulty );
extern INT8 CalcTrapDetectLevel( SOLDIERTYPE *pSoldier, BOOLEAN fExamining );


INT8 EffectiveStrength(    const SOLDIERTYPE* s);
INT8 EffectiveWisdom(      const SOLDIERTYPE* s);
INT8 EffectiveAgility(     const SOLDIERTYPE* s);
INT8 EffectiveMechanical(  const SOLDIERTYPE* s);
INT8 EffectiveExplosive(   const SOLDIERTYPE* s);
INT8 EffectiveLeadership(  const SOLDIERTYPE* s);
INT8 EffectiveMarksmanship(const SOLDIERTYPE* s);
INT8 EffectiveDexterity(   const SOLDIERTYPE* s);
INT8 EffectiveExpLevel(    const SOLDIERTYPE* s);
INT8 EffectiveMedical(     const SOLDIERTYPE* s);


enum SkillChecks
{
	NO_CHECK = 0,
	LOCKPICKING_CHECK,
	ELECTRONIC_LOCKPICKING_CHECK,
	ATTACHING_DETONATOR_CHECK,
	ATTACHING_REMOTE_DETONATOR_CHECK,
	PLANTING_BOMB_CHECK,
	PLANTING_REMOTE_BOMB_CHECK,
	OPEN_WITH_CROWBAR,
	SMASH_DOOR_CHECK,
	DISARM_TRAP_CHECK,
	UNJAM_GUN_CHECK,
	NOTICE_DART_CHECK,
	LIE_TO_QUEEN_CHECK,
	ATTACHING_SPECIAL_ITEM_CHECK,
	ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK,
	DISARM_ELECTRONIC_TRAP_CHECK
};

#endif
