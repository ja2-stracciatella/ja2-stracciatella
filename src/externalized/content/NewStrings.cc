#include "NewStrings.h"

#include "Cheats.h"
#include "GameInstance.h"
#include "Drugs_And_Alcohol.h"
#include "Items.h"
#include "MercProfile.h"
#include "Morale.h"
#include "SkillCheck.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Strategic_Town_Loyalty.h"
#include "Text.h"
#include "WeaponModels.h"

#define demarcationStrings                                  (g_tooltipsRes->demarcationStrings)
#define booleanStrings                                      (g_tooltipsRes->booleanStrings)
#define segmentHeaderStrings                                (g_tooltipsRes->segmentHeaderStrings)
#define effectiveStatStrings                                (g_tooltipsRes->effectiveStatStrings)
#define skillPossessionStrings                              (g_tooltipsRes->skillPossessionStrings)
#define personalityTraitStrings                             (g_tooltipsRes->personalityTraitStrings)
#define toolModifierStrings                                 (g_tooltipsRes->toolModifierStrings)
#define statusStrings                                       (g_tooltipsRes->statusStrings)
#define lockStrings                                         (g_tooltipsRes->lockStrings)
#define vitalSignStrings                                    (g_tooltipsRes->vitalSignStrings)
#define rangeStrings                                        (g_tooltipsRes->rangeStrings)
#define trapStrings                                         (g_tooltipsRes->trapStrings)
#define attemptStrings                                      (g_tooltipsRes->attemptStrings)
#define commentStrings                                      (g_tooltipsRes->commentStrings)
#define possessiveStrings                                   (g_tooltipsRes->possessiveStrings)
#define drugStrings                                         (g_tooltipsRes->drugStrings)
#define inventoryStrings                                    (g_tooltipsRes->inventoryStrings)
#define locationStrings                                     (g_tooltipsRes->locationStrings)
#define dialogueStrings                                     (g_tooltipsRes->dialogueStrings)

#define tab demarcationStrings[SINGLE_TAB]

enum
{
	STR_BOOL_GREEN_NO,
	STR_BOOL_RED_NO,
	STR_BOOL_GREEN_YES,
	STR_BOOL_RED_YES,
	STR_BOOL_NONE_TRUE,
	STR_BOOL_ONE_TRUE,
};

enum
{
	SINGLE_TAB,
	DIVIDER,
};

enum
{
	HEADER_CHANCE,
	HEADER_MODIFIED_BY,
	HEADER_BONUSES,
	HEADER_PENALTIES,
	HEADER_TRAP_DETECTION,
};

enum
{
	MOD_LOCKSMITH_STATUS,
	MOD_CROWBAR,
};

enum
{
	STR_STATUS_HIDDEN,
	STR_STATUS_HIDDEN_PLAIN,
	STR_STATUS_N_SLASH_A,
	STR_STATUS_UNUSABLE,
	STR_STATUS_IMPOSSIBLE,
	STR_STATUS_UNTRAPPED,
};

enum
{
	LOCK_DIFFICULTY,
	LOCK_IS_ELECTRONIC,
	LOCK_DAMAGE,
	LOCK_SMASH_DIFFICULTY,
};

enum
{
	VITAL_HEALTH,
	VITAL_FATIGUE,
	VITAL_MORALE,
	VITAL_IN_SHOCK,
	VITAL_SEVERE_BLEEDING,
	VITAL_FIRST_AID,
};

enum
{
	RANGE_NORMAL,
	RANGE_EFFECTIVE,
	RANGE_NOT_RANGE,
};

enum
{
	TRAP_PERCEPTION,
	TRAP_PASSIVE_DETECTION,
	TRAP_ACTIVE_DETECTION,
	TRAP_IF_ACTIVE_DETECTION,
	TRAP_EXPLOSIVE,
	TRAP_ELECTRONIC,
	TRAP_IS_ELECTRONIC,
	TRAP_LEVEL,
};

enum
{
	ATTEMPTS_MADE_VS_NEEDED,
	ATTEMPTS_MANDATORY,
};

enum
{
	COMMENT_ZERO_EXPLOSIVES,
	COMMENT_ZERO_MEDICAL,
	COMMENT_ZERO_MECHANICAL,
	COMMENT_EFFECTIVELY_ZERO,
	COMMENT_EFFECTIVELY_99,
	COMMENT_MORE_ATTEMPTS,
	COMMENT_TOOL_NOT_FOUND,
	COMMENT_TOOL_UNUSABLE,
	COMMENT_LESS_THAN,
	COMMENT_MORE_THAN,
};

enum
{
	POSSESSIVE_HAS_NO,
	POSSESSIVE_HAS,
	POSSESSIVE_HAS_EXPERT,
};

enum
{
	INV_OVERBURDENED,
};

enum
{
	LOC_UNDERGROUND,
	LOC_TOWN_LOYALTY,
	LOC_TOWN_ATTACHMENT,
};

enum
{
	DIAL_EFFECTIVENESS,
	DIAL_FRIENDLY,
	DIAL_DIRECT,
	DIAL_THREATEN,
	DIAL_OPINION,
	DIAL_GUN_DEADLINESS,
	DIAL_APPROACH_SKILL,
	DIAL_SUSCEPTIBILITY,
	DIAL_WEAPON_SALES,
	DIAL_FAST_DRAW,
	DIAL_NOMINAL_LEADERSHIP,
	DIAL_POSSESIVE_EFF_EXPLEVEL,
};

struct SKILLCHECKINFO
{
	bool isSufficientlyAttempted{};
	ST::string attemptsMade{ "^0" };
	ST::string attemptsNeeded{};
	ST::string attemptsFromWisdom{};
	bool isMechSkillRequiredAndZero{}, isExplSkillRequiredAndZero{}, isMedSkillRequiredAndZero{};
	bool is30PctMinRequired{ true }; // some skill checks have a 30 percent minimum for the success chance, some don't
};

struct TOOLINFO
{
	ITEMDEFINE name{};
	bool mustBeUsable{};
	int32_t modifier{};
	ST::string coloredModifier{};
	ST::string chanceComment{};
};

struct TRAITINFO
{
	SkillTrait name{};
	int32_t modifier{};
	ST::string coloredModifier{};
	ST::string possession{};
};

static const ST::string ColorCodeModifier(const ST::string fmt, const int32_t number, const bool isGreenIfZeroOrGreater = true)
{
	ST::string plain = ST::format(fmt.c_str(), number);
	ST::string colored;

	if (number == 0) {
		plain = plain.replace("+", "");
	}

	ST::string controlChar = isGreenIfZeroOrGreater ? "^" : "~";
	if (number < 0 && isGreenIfZeroOrGreater) {
		controlChar = "~";
	}

	for (size_t i = 0; i < plain.size(); i++) {
		colored += controlChar;
		colored += plain[i];
	}

	return colored;
}

static void GetTraitModifier(TRAITINFO& traitInf, SOLDIERTYPE* const soldier, const SkillTrait trait, const SkillChecks skill = NO_CHECK)
{
	bool isExpert{};
	int8_t traitLvl = gbSkillTraitBonus[trait] * NUM_SKILL_TRAITS(soldier, trait);
	if (traitLvl == 0) {
		traitInf.possession = possessiveStrings[POSSESSIVE_HAS_NO];
	} else if (traitLvl == gbSkillTraitBonus[trait]) {
		traitInf.possession = possessiveStrings[POSSESSIVE_HAS];
	} else if (traitLvl == gbSkillTraitBonus[trait] * 2) {
		traitInf.possession = possessiveStrings[POSSESSIVE_HAS_EXPERT];
		isExpert = true;
	}

	if (trait == LOCKPICKING) {
		traitInf.modifier = traitLvl;
		if (isExpert) {
			traitInf.coloredModifier = "^+^2^x" + ColorCodeModifier("{d}%", traitLvl / 2);
		} else {
			traitInf.coloredModifier = ColorCodeModifier("{+d}%", traitLvl);
		}
	} else if (trait == ELECTRONICS) {
		if (traitLvl == 0) {
			if (skill == LOCKPICKING_CHECK) {
				traitInf.modifier = traitInf.modifier / 2;
			} else if (skill == DISARM_ELECTRONIC_TRAP_CHECK) {
				traitInf.modifier = traitInf.modifier / 4;
			}
		} else {
			traitInf.modifier = 0;
		}
		traitInf.coloredModifier += ST::format("{}", ColorCodeModifier("{d}%", -traitInf.modifier));
	}
}

static void GetToolModifier(TOOLINFO& reqTool, SOLDIERTYPE* const soldier)
{
	int8_t toolSlot = FindObj(soldier, reqTool.name);
	if (toolSlot == NO_SLOT) {
		reqTool.coloredModifier = statusStrings[STR_STATUS_N_SLASH_A];
		reqTool.chanceComment = commentStrings[COMMENT_TOOL_NOT_FOUND];
	} else if (reqTool.mustBeUsable) {
		if (int8_t usableToolSlot = FindUsableObj(soldier, reqTool.name); usableToolSlot == NO_SLOT) {
			reqTool.coloredModifier = statusStrings[STR_STATUS_UNUSABLE];
			reqTool.chanceComment = commentStrings[COMMENT_TOOL_UNUSABLE];
			return;
		} else {
			toolSlot = usableToolSlot;
		}
	}
	if (toolSlot != NO_SLOT) {
		if (reqTool.name == LOCKSMITHKIT) {
			int32_t baseMod = reqTool.modifier;
			reqTool.modifier = baseMod * soldier->inv[toolSlot].bStatus[0] / 100;
			reqTool.coloredModifier = ColorCodeModifier("{d}%", reqTool.modifier - baseMod);
		} else if (reqTool.name == CROWBAR) {
			reqTool.modifier = 20;
			reqTool.coloredModifier = ColorCodeModifier("{+d}%", reqTool.modifier);
		}
	}
}

static const ST::string GetChanceOfSuccessComment(SKILLCHECKINFO& inf, TOOLINFO& reqTool, const int8_t chance)
{
	ST::string chanceColored{}, comments{};
	bool isImpossible{};

	if (inf.isExplSkillRequiredAndZero) {
		isImpossible = true;
		comments += "\n" + tab + commentStrings[COMMENT_ZERO_EXPLOSIVES];
	}
	if (inf.isMedSkillRequiredAndZero) {
		isImpossible = true;
		comments += "\n" + tab + commentStrings[COMMENT_ZERO_MEDICAL];
	}
	if (inf.isMechSkillRequiredAndZero) {
		isImpossible = true;
		comments += "\n" + tab + commentStrings[COMMENT_ZERO_MECHANICAL];
	}
	if (!reqTool.chanceComment.empty()) {
		isImpossible = true;
		comments += "\n" + tab + reqTool.chanceComment;
	}

	if (!isImpossible) {
		if (inf.isSufficientlyAttempted) {
			if (inf.is30PctMinRequired && chance > 0 && chance < 30) {
				chanceColored = ST::format("{d}%", chance);
				comments += commentStrings[COMMENT_EFFECTIVELY_ZERO];
			} else if (chance > 99) {
				chanceColored = ST::format("{d}%", chance);
				comments += commentStrings[COMMENT_EFFECTIVELY_99];
			} else {
				chanceColored = ColorCodeModifier("{d}%", chance);
			}
		} else {
			chanceColored = statusStrings[STR_STATUS_HIDDEN];
			comments += commentStrings[COMMENT_MORE_ATTEMPTS];
		}
	} else {
		chanceColored = statusStrings[STR_STATUS_IMPOSSIBLE];
	}

	return chanceColored+comments;
}

static void CompareSkillAttempts(SKILLCHECKINFO& inf, SOLDIERTYPE* const soldier, SkillChecks skill)
{
	int8_t attemptsNeeded = 8 - ceil(static_cast<float>(EffectiveWisdom(soldier)) / 20);
	if (soldier->sGridNo == soldier->sSkillCheckGridNo) {
		if (soldier->bLastSkillCheck == skill) {
			inf.isSufficientlyAttempted = soldier->ubSkillCheckAttempts >= attemptsNeeded;
			inf.attemptsMade = ColorCodeModifier("{d}", soldier->ubSkillCheckAttempts);
		}
	}
	inf.attemptsNeeded = ST::format("~{}", attemptsNeeded);
	inf.attemptsFromWisdom = ST::format("~+~{}", attemptsNeeded - 2);
}

static const ST::string GetModifiersForSkillAttempts(SKILLCHECKINFO& inf)
{
	ST::string result;
	result += "\n" + demarcationStrings[DIVIDER];
	result += st_format_printf("\n" + attemptStrings[ATTEMPTS_MADE_VS_NEEDED], inf.attemptsMade, inf.attemptsNeeded);
	result +=                  "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	result +=                  "\n" + tab + attemptStrings[ATTEMPTS_MANDATORY];
	result += st_format_printf("\n" + tab + effectiveStatStrings[ATTR_WISDOM], inf.attemptsFromWisdom);
	return result;
}

static const ST::string GetRangeString(const int32_t lBound, const int32_t uBound, const bool isGreenIfZeroOrGreater = true, const bool isPlusSigned = false)
{
	ST::string result;
	ST::string fmt = isPlusSigned ? "{+d}" : "{d}";
	if (lBound != uBound) {
		ST::string const lBoundStr = lBound < 1 ? ST::format("{}", lBound) : ColorCodeModifier(fmt, lBound, isGreenIfZeroOrGreater);
		ST::string const uBoundStr = lBound < 1 ? ST::format("{}", uBound) : ColorCodeModifier(fmt, uBound, isGreenIfZeroOrGreater);
		result = st_format_printf(rangeStrings[RANGE_NORMAL], lBoundStr, uBoundStr);
		if (lBound < 1) {
			if (uBound > 1) {
				result += st_format_printf(rangeStrings[RANGE_EFFECTIVE], ColorCodeModifier(fmt, uBound, isGreenIfZeroOrGreater));
			} else {
				result += rangeStrings[RANGE_NOT_RANGE];
			}
		}
	} else {
		result = ColorCodeModifier(fmt, lBound, isGreenIfZeroOrGreater);
		if (lBound < 1) {
			result += rangeStrings[RANGE_NOT_RANGE];
		}
	}
	return result;
};

const ST::string GetModifiersForLockExam(SOLDIERTYPE* const s, DOOR* const d)
{
	SKILLCHECKINFO skillCheckInf{};

	int32_t const expLvlMod = EffectiveExpLevel(s);
	int32_t const explMod = EffectiveExplosive(s) / 40;
	int32_t const wisdMod = (100 - EffectiveWisdom(s)) / 20;
	int32_t const detectionLvl = expLvlMod + explMod - wisdMod;
	int32_t const randMod = detectionLvl / 3 + 2;

	int32_t lowerBound = detectionLvl;
	int32_t upperBound = detectionLvl;
	bool const isShockedOrBleeding = s->bBleeding > 20 || s->bShock > 1;
	if (isShockedOrBleeding) {
		lowerBound += -3;
		upperBound += -1;
	}

	CompareSkillAttempts(skillCheckInf, s, d->ubTrapID == EXPLOSION ? DISARM_TRAP_CHECK : DISARM_ELECTRONIC_TRAP_CHECK);
	ST::string const trapLvlColored = skillCheckInf.isSufficientlyAttempted ? ColorCodeModifier("{d}", d->ubTrapLevel, false) : statusStrings[STR_STATUS_HIDDEN];
	
	ST::string result;
	if (d->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN) {
		result = st_format_printf("\n" + trapStrings[TRAP_PERCEPTION], statusStrings[STR_STATUS_HIDDEN]);
	} else if (d->bPerceivedTrapped == DOOR_PERCEIVED_UNTRAPPED) {
		result = st_format_printf("\n" + trapStrings[TRAP_PERCEPTION], statusStrings[STR_STATUS_UNTRAPPED]);
	}

	result += "\n" + demarcationStrings[DIVIDER];
	result += "\n" + segmentHeaderStrings[HEADER_TRAP_DETECTION]; // "Detection Level vs. Trap Level:"

	result += st_format_printf("\n" + tab + trapStrings[TRAP_PASSIVE_DETECTION], GetRangeString(lowerBound,     upperBound),           trapLvlColored);
	result += st_format_printf("\n" + tab + trapStrings[TRAP_ACTIVE_DETECTION],  GetRangeString(lowerBound + 1, upperBound + randMod), trapLvlColored);

	result +=                  "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	result +=                  "\n" + tab + segmentHeaderStrings[HEADER_BONUSES];
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLEVEL], ColorCodeModifier("{+d}", expLvlMod));
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLOSIVES], ColorCodeModifier("{+d}", explMod));
	result += st_format_printf("\n" + tab + tab + trapStrings[TRAP_IF_ACTIVE_DETECTION], ColorCodeModifier("{+d}", randMod));
	result +=                  "\n" + tab + segmentHeaderStrings[HEADER_PENALTIES];
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_WISDOM], ColorCodeModifier("{d}", -wisdMod));
	ST::string const boolStr = isShockedOrBleeding ? booleanStrings[STR_BOOL_ONE_TRUE] : booleanStrings[STR_BOOL_NONE_TRUE];
	ST::string const defaultRange = st_format_printf(rangeStrings[RANGE_NORMAL], "~-~1", "~-~3");
	result += st_format_printf("\n" + tab + tab + boolStr, isShockedOrBleeding ? defaultRange : "^0");
	ST::string const bleedStr = s->bBleeding > 20 ? booleanStrings[STR_BOOL_RED_YES] : booleanStrings[STR_BOOL_GREEN_NO];
	result += st_format_printf("\n" + tab + tab + tab + vitalSignStrings[VITAL_SEVERE_BLEEDING], bleedStr);
	ST::string const shockStr = s->bShock > 1 ? booleanStrings[STR_BOOL_RED_YES] : booleanStrings[STR_BOOL_GREEN_NO];
	result += st_format_printf("\n" + tab + tab + tab + vitalSignStrings[VITAL_IN_SHOCK], shockStr);
	return result;
}

const ST::string GetModifiersForLockUntrap(SOLDIERTYPE* const s, DOOR* const d)
{
	SKILLCHECKINFO skillCheckInf{};
	TRAITINFO electrTraitInf{};
	TOOLINFO requiredTool{};

	int32_t const explMod = EffectiveExplosive(s) * 7 / 10;
	int32_t const mechMod = EffectiveMechanical(s) * 7 / 10;
	bool const isMechMoreThanExpl = mechMod >= explMod;
	int32_t const dextMod = EffectiveDexterity(s) * 2 / 10;
	int32_t const expLvlMod = EffectiveExpLevel(s);
	int32_t const wisdMod = (100 - EffectiveWisdom(s)) / 5;

	int32_t chance = dextMod + expLvlMod - wisdMod;

	bool const isPerceivedTrapped = d->bPerceivedTrapped == DOOR_PROVED_TRAPPED;
	bool const isTrapExplosive = d->ubTrapID == EXPLOSION;
	ST::string electrTypeAffirmation;
	if (isTrapExplosive) {
		electrTypeAffirmation = booleanStrings[STR_BOOL_GREEN_NO];
		skillCheckInf.isExplSkillRequiredAndZero = explMod == 0;
		chance += explMod;
	} else {
		electrTypeAffirmation = booleanStrings[STR_BOOL_RED_YES];
		if (explMod == 0 && mechMod == 0 && isPerceivedTrapped) {
			skillCheckInf.isExplSkillRequiredAndZero = true;
			skillCheckInf.isMechSkillRequiredAndZero = true;
		}
		chance += (isMechMoreThanExpl ? mechMod : explMod);
		electrTraitInf.modifier = chance;
		GetTraitModifier(electrTraitInf, s, ELECTRONICS, DISARM_ELECTRONIC_TRAP_CHECK);
		chance -= electrTraitInf.modifier;
	}

	int32_t const fatigueMod = GetSkillCheckPenaltyForFatigue(s, chance);
	int32_t const trapLvlMod = d->ubTrapLevel * 7;
	chance += -fatigueMod - trapLvlMod;

	CompareSkillAttempts(skillCheckInf, s, isTrapExplosive ? DISARM_TRAP_CHECK : DISARM_ELECTRONIC_TRAP_CHECK);

	ST::string result;
	if (isPerceivedTrapped) {
		result = st_format_printf("\n" + segmentHeaderStrings[HEADER_CHANCE], GetChanceOfSuccessComment(skillCheckInf, requiredTool, chance));
	} else {
		result = st_format_printf("\n" + segmentHeaderStrings[HEADER_CHANCE], ST::string());
		result +=                 "\n" + tab + trapStrings[TRAP_EXPLOSIVE] + statusStrings[STR_STATUS_HIDDEN];
		result +=                 "\n" + tab + trapStrings[TRAP_ELECTRONIC] + statusStrings[STR_STATUS_HIDDEN];
		electrTypeAffirmation = statusStrings[STR_STATUS_HIDDEN];
	}
	result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	result += "\n" + tab + segmentHeaderStrings[HEADER_BONUSES];
	if (isPerceivedTrapped) {
		if (isTrapExplosive) {
			result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLOSIVES], ColorCodeModifier("{+d}%", explMod));
		} else {
			if (isMechMoreThanExpl) {
				result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_MECHANICAL], ColorCodeModifier("{+d}%", mechMod));
			} else {
				result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLOSIVES], ColorCodeModifier("{+d}%", explMod));
			}
		}
	} else {
		result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLOSIVES], ColorCodeModifier("{+d}%", explMod));
		if (isMechMoreThanExpl) {
			result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_MECHANICAL], ColorCodeModifier("{+d}%", mechMod));
		}
	}
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_DEXTERITY], ColorCodeModifier("{+d}%", dextMod));
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLEVEL], ColorCodeModifier("{+d}%", expLvlMod));
	result +=                  "\n" + tab + segmentHeaderStrings[HEADER_PENALTIES];
	ST::string const levelStr = skillCheckInf.isSufficientlyAttempted ? ColorCodeModifier("{d}%", -trapLvlMod) : statusStrings[STR_STATUS_HIDDEN];
	result += st_format_printf("\n" + tab + tab + trapStrings[TRAP_LEVEL], levelStr);
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_WISDOM], ColorCodeModifier("{d}%", -wisdMod));
	ST::string const skillStatus = isPerceivedTrapped ? electrTraitInf.coloredModifier : statusStrings[STR_STATUS_HIDDEN];
	result += st_format_printf("\n" + tab + tab + skillPossessionStrings[ELECTRONICS], electrTraitInf.possession, skillStatus);
	result += st_format_printf("\n" + tab + tab + tab + trapStrings[TRAP_IS_ELECTRONIC], electrTypeAffirmation);
	ST::string const fatigue = isPerceivedTrapped || explMod >= mechMod ? ColorCodeModifier("{d}%", -fatigueMod) : statusStrings[STR_STATUS_HIDDEN];
	result += st_format_printf("\n" + tab + tab + vitalSignStrings[VITAL_FATIGUE], fatigue);

	result += GetModifiersForSkillAttempts(skillCheckInf);
	return result;
}

const ST::string GetModifiersForLockForceOpen(SOLDIERTYPE* const s, DOOR* const d, bool isCrowbarUsed)
{
	LOCK* const l = &LockTable[d->ubLockID];
	SKILLCHECKINFO skillCheckInf{};
	TOOLINFO requiredTool{};

	int32_t const strnMod = EffectiveStrength(s);

	int32_t fatigueMod;
	if (isCrowbarUsed) {
		requiredTool.name = CROWBAR;
		requiredTool.mustBeUsable = true;
		GetToolModifier(requiredTool, s);
		fatigueMod = GetSkillCheckPenaltyForFatigue(s, strnMod + requiredTool.modifier);
	} else {
		fatigueMod = GetSkillCheckPenaltyForFatigue(s, strnMod);
	}

	int32_t const smashDiffMod = l->ubSmashDifficulty;
	int32_t const lockDmgMod = d->bLockDamage;
	int32_t const chance = strnMod + requiredTool.modifier - fatigueMod - smashDiffMod + lockDmgMod;

	CompareSkillAttempts(skillCheckInf, s, isCrowbarUsed ? OPEN_WITH_CROWBAR : SMASH_DOOR_CHECK);

	ST::string result;
	result =  st_format_printf("\n" + segmentHeaderStrings[HEADER_CHANCE], GetChanceOfSuccessComment(skillCheckInf, requiredTool, chance));
	result +=                  "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	result +=                  "\n" + tab + segmentHeaderStrings[HEADER_BONUSES];
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_STRENGTH], ColorCodeModifier("{+d}%", strnMod));
	if (isCrowbarUsed) {
		result += st_format_printf("\n" + tab + tab + toolModifierStrings[MOD_CROWBAR], requiredTool.coloredModifier);
	}
	result += st_format_printf("\n" + tab + tab + lockStrings[LOCK_DAMAGE], ColorCodeModifier("{+d}%", lockDmgMod));
	result +=                  "\n" + tab + segmentHeaderStrings[HEADER_PENALTIES];
	ST::string const smashStr = skillCheckInf.isSufficientlyAttempted ? ColorCodeModifier("{d}%", -smashDiffMod) : statusStrings[STR_STATUS_HIDDEN];
	result += st_format_printf("\n" + tab + tab + lockStrings[LOCK_SMASH_DIFFICULTY], smashStr);
	result += st_format_printf("\n" + tab + tab + vitalSignStrings[VITAL_FATIGUE], ColorCodeModifier("{d}%", -fatigueMod));

	result += GetModifiersForSkillAttempts(skillCheckInf);
	return result;
}

const ST::string GetModifiersForLockPicking(SOLDIERTYPE* const s, DOOR* const d)
{
	LOCK* const l = &LockTable[d->ubLockID];
	SKILLCHECKINFO skillCheckInf{};
	TRAITINFO lockTraitInf{}, electrTraitInf{};
	TOOLINFO requiredTool{};

	int32_t const mechMod = EffectiveMechanical(s);
	skillCheckInf.isMechSkillRequiredAndZero = mechMod == 0;
	int32_t const wisdMod = mechMod * (EffectiveWisdom(s) + 100) / 200;
	int32_t const dextMod = wisdMod * (EffectiveDexterity(s) + 100) / 200;
	int32_t const expLvlMod = dextMod + EffectiveExpLevel(s) * 3;

	GetTraitModifier(lockTraitInf, s, LOCKPICKING);
	int32_t const traitMod = expLvlMod + lockTraitInf.modifier;

	bool const isLockElectronic = l->ubLockType == LOCK_CARD || l->ubLockType == LOCK_ELECTRONIC;
	if (isLockElectronic) {
		electrTraitInf.modifier = traitMod;
	}
	GetTraitModifier(electrTraitInf, s, ELECTRONICS, LOCKPICKING_CHECK);
	int32_t const lockTypeMod = traitMod - electrTraitInf.modifier;

	requiredTool.name = LOCKSMITHKIT;
	requiredTool.modifier = lockTypeMod;
	GetToolModifier(requiredTool, s);

	int32_t const fatigueMod = requiredTool.modifier - (GetSkillCheckPenaltyForFatigue(s, requiredTool.modifier));

	int32_t const lockDiffMod = fatigueMod - l->ubPickDifficulty;

	CompareSkillAttempts(skillCheckInf, s, isLockElectronic ? ELECTRONIC_LOCKPICKING_CHECK : LOCKPICKING_CHECK);

	ST::string result;
	result = st_format_printf("\n" + segmentHeaderStrings[HEADER_CHANCE], GetChanceOfSuccessComment(skillCheckInf, requiredTool, lockDiffMod));
	result +=                 "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	result +=                 "\n" + tab + segmentHeaderStrings[HEADER_BONUSES];
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_MECHANICAL], ColorCodeModifier("{+d}%", mechMod));
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLEVEL], ColorCodeModifier("{+d}%", expLvlMod - dextMod));
	result += st_format_printf("\n" + tab + tab + skillPossessionStrings[LOCKPICKING], lockTraitInf.possession, lockTraitInf.coloredModifier);
	result += "\n" + tab + segmentHeaderStrings[HEADER_PENALTIES];
	ST::string const lockDifficulty = skillCheckInf.isSufficientlyAttempted ? ColorCodeModifier("{d}%", lockDiffMod - fatigueMod) : statusStrings[STR_STATUS_HIDDEN];
	result += st_format_printf("\n" + tab + tab + lockStrings[LOCK_DIFFICULTY], lockDifficulty);
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_WISDOM], ColorCodeModifier("{d}%", wisdMod - mechMod));
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_DEXTERITY], ColorCodeModifier("{d}%", dextMod - wisdMod));
	result += st_format_printf("\n" + tab + tab + skillPossessionStrings[ELECTRONICS], electrTraitInf.possession, electrTraitInf.coloredModifier);
	result += st_format_printf("\n" + tab + tab + tab + lockStrings[LOCK_IS_ELECTRONIC], isLockElectronic ? booleanStrings[STR_BOOL_RED_YES] : booleanStrings[STR_BOOL_GREEN_NO]);
	result += st_format_printf("\n" + tab + tab + toolModifierStrings[MOD_LOCKSMITH_STATUS], requiredTool.coloredModifier);
	result += st_format_printf("\n" + tab + tab + vitalSignStrings[VITAL_FATIGUE], ColorCodeModifier("{d}%", fatigueMod - requiredTool.modifier));

	result += GetModifiersForSkillAttempts(skillCheckInf);
	return result;
}

const ST::string GetModifiersForLockBlowUp(SOLDIERTYPE* const s)
{
	SKILLCHECKINFO skillCheckInf{};
	TOOLINFO requiredTool{};

	skillCheckInf.is30PctMinRequired = false;
	skillCheckInf.isSufficientlyAttempted = true;
	requiredTool.name = SHAPED_CHARGE;
	GetToolModifier(requiredTool, s);

	int32_t explMod = EffectiveExplosive(s) * 7;
	int32_t wisdMod = EffectiveWisdom(s) * 2;
	int32_t expLvlMod = EffectiveExpLevel(s) * 10;
	int32_t chance = (explMod + wisdMod + expLvlMod) / 10;
	int32_t skewedUpChance = (chance + 100 * (chance / 25)) / (chance / 25 + 1);
	float const skewnessFactor = static_cast<float>(skewedUpChance) / static_cast<float>(chance);
	explMod = explMod * skewnessFactor / 10;
	wisdMod = wisdMod * skewnessFactor / 10;
	expLvlMod = expLvlMod * skewnessFactor / 10;

	int32_t const fatigueMod = -GetSkillCheckPenaltyForFatigue(s, skewedUpChance);
	skewedUpChance += fatigueMod;
	int32_t const moraleMod = GetMoraleModifier(s);
	skewedUpChance += moraleMod;

	ST::string result;
	result = st_format_printf("\n" + segmentHeaderStrings[HEADER_CHANCE], GetChanceOfSuccessComment(skillCheckInf, requiredTool, skewedUpChance));
	result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	result += "\n" + tab + segmentHeaderStrings[HEADER_BONUSES];
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLOSIVES], ColorCodeModifier("{+d}%", explMod));
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_WISDOM], ColorCodeModifier("{+d}%", wisdMod));
	result += st_format_printf("\n" + tab + tab + effectiveStatStrings[ATTR_EXPLEVEL], ColorCodeModifier("{+d}%", expLvlMod));
	if (moraleMod >= 0) {
		result += st_format_printf("\n" + tab + tab + vitalSignStrings[VITAL_MORALE], ColorCodeModifier("{+d}%", moraleMod));
	}
	result += "\n" + tab + segmentHeaderStrings[HEADER_PENALTIES];
	if (moraleMod < 0) {
		result += st_format_printf("\n" + tab + tab + vitalSignStrings[VITAL_MORALE], ColorCodeModifier("{d}%", moraleMod));
	}
	result += st_format_printf("\n" + tab + tab + vitalSignStrings[VITAL_FATIGUE], ColorCodeModifier("{d}%", fatigueMod));

	return result;
}

const ST::string GetModifiersForEffectiveAttributes(SOLDIERTYPE* const s, Attributes const attrName)
{
	ST::string result, comment, possession, location;
	int32_t effAttr;
	int32_t alcoMod{}, weightMod{}, healthMod, firstAidMod, phobiaMod{};
	int32_t firstAid, drunkLevel;

	auto getAlcoMod = [&s, attrName, &effAttr, &alcoMod, &result](int32_t attrVal) {
		effAttr = attrVal;
		alcoMod = EffectStatForBeingDrunk(s, effAttr) - effAttr;
		effAttr += alcoMod;
		result += st_format_printf(effectiveStatStrings[attrName], ColorCodeModifier("{d}", effAttr, attrVal <= effAttr));
		result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
		result += st_format_printf("\n" + tab + drugStrings[DRUG_TYPE_ALCOHOL], ColorCodeModifier("{+d}", alcoMod));
	};

	switch (attrName) {
		case ATTR_AGILITY:
			getAlcoMod(s->bAgility);
			if (s->sWeightCarriedAtTurnStart > 100) {
				weightMod = (effAttr * 100) / s->sWeightCarriedAtTurnStart - effAttr;
				effAttr += weightMod;
			}
	
			result += st_format_printf("\n" + tab + inventoryStrings[INV_OVERBURDENED], ColorCodeModifier("{+d}", weightMod));
			break;
		case ATTR_DEXTERITY:
			getAlcoMod(s->bDexterity);
			break;
		case ATTR_STRENGTH:
			effAttr = s->bStrength / 2;
			healthMod = effAttr * s->bLife / s->bLifeMax;
			firstAid = s->bLifeMax - s->bLife - s->bBleeding;
			firstAidMod = effAttr * (s->bLife + firstAid / 2) / s->bLifeMax;
			effAttr += firstAidMod;
			if (effAttr < 2) {
				effAttr = 2;
				comment = st_format_printf(". " + commentStrings[COMMENT_LESS_THAN], effAttr);
			}

			result += st_format_printf(effectiveStatStrings[ATTR_STRENGTH] + comment, ColorCodeModifier("{d}", effAttr, s->bStrength <= effAttr));
			result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
			result += st_format_printf("\n" + tab + vitalSignStrings[VITAL_HEALTH], ColorCodeModifier("{+d}", (s->bStrength / 2 + healthMod) - s->bStrength));
			result += st_format_printf("\n" + tab + vitalSignStrings[VITAL_FIRST_AID], ColorCodeModifier("{+d}", firstAidMod - healthMod));
			break;
		case ATTR_LEADERSHIP:
			effAttr = s->bLeadership;
			if (GetDrunkLevel(s) == FEELING_GOOD) {
				effAttr = effAttr * 120 / 100;
				alcoMod = effAttr - s->bLeadership;
			}

			result += st_format_printf(effectiveStatStrings[ATTR_LEADERSHIP], ColorCodeModifier("{d}", effAttr, s->bLeadership <= effAttr));
			result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
			result += st_format_printf("\n" + tab + drugStrings[DRUG_TYPE_ALCOHOL], ColorCodeModifier("{+d}", alcoMod));
			break;
		case ATTR_WISDOM:
			getAlcoMod(s->bWisdom);
			break;
		case ATTR_EXPLEVEL:
			drunkLevel = GetDrunkLevel(s);
			if (drunkLevel == BORDERLINE)
				alcoMod = -1;
			else if (drunkLevel == DRUNK)
				alcoMod = -2;

			if (gMercProfiles[s->ubProfile].bPersonalityTrait == CLAUSTROPHOBIC) {
				possession = possessiveStrings[POSSESSIVE_HAS];
				if (s->bActive && s->bInSector && s->sSector.z > 0) {
					phobiaMod = -1;
				}
			} else {
				possession = possessiveStrings[POSSESSIVE_HAS_NO];
			}

			location = s->sSector.z > 0 ? booleanStrings[STR_BOOL_RED_YES] : booleanStrings[STR_BOOL_GREEN_NO];

			effAttr = s->bExpLevel + alcoMod + phobiaMod;
			if (effAttr < 1) {
				effAttr = 1;
				comment = st_format_printf(". " + commentStrings[COMMENT_LESS_THAN], effAttr);
			}

			result += st_format_printf(effectiveStatStrings[ATTR_EXPLEVEL] + comment, ColorCodeModifier("{d}", effAttr, s->bExpLevel <= effAttr));
			result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
			result += st_format_printf("\n" + tab + drugStrings[DRUG_TYPE_ALCOHOL], ColorCodeModifier("{+d}", alcoMod));
			result += st_format_printf("\n" + tab + personalityTraitStrings[CLAUSTROPHOBIC], possession, ColorCodeModifier("{+d}", phobiaMod));
			result += st_format_printf("\n" + tab + tab + locationStrings[LOC_UNDERGROUND], location);
			break;
		case ATTR_MARKSMANSHIP:
			getAlcoMod(s->bMarksmanship);
			break;
		case ATTR_EXPLOSIVES:
			getAlcoMod(s->bExplosive);
			break;
		case ATTR_MECHANICAL:
			getAlcoMod(s->bMechanical);
			break;
		case ATTR_MEDICAL:
			getAlcoMod(s->bMedical);
			break;
	}

	return result;
}

const ST::string GetModifiersForDialogue(SOLDIERTYPE* const playerChar, SOLDIERTYPE* const npcChar, Approach const apprName)
{
	uint16_t itemIdx;
	int32_t salesSkill;
	int32_t plrLeadership, plrEffStrength, plrGunDeadliness{};

	bool const isCheater = CHEATER_CHEAT_LEVEL();

	ST::string result, totalApproachEffectiveness;
	if (apprName != APPROACH_BUYSELL) {
		totalApproachEffectiveness = ColorCodeModifier("{d}", CalcDesireToTalk(npcChar->ubProfile, playerChar->ubProfile, apprName));
		result += st_format_printf(dialogueStrings[DIAL_EFFECTIVENESS], isCheater ? totalApproachEffectiveness : statusStrings[STR_STATUS_HIDDEN]);
		result += "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
	}

	switch (apprName) {
		case APPROACH_THREATEN:
			itemIdx = playerChar->inv[HANDPOS].usItem;
			if (GCM->getItem(itemIdx)->isWeapon()) {
				plrGunDeadliness = GCM->getWeapon(itemIdx)->ubDeadliness;
			}
			if (plrGunDeadliness == 0) {
				plrGunDeadliness = -30;
			}
			plrLeadership = EffectiveLeadership(playerChar);
			plrEffStrength = EffectiveStrength(playerChar);

			result += st_format_printf("\n" + tab + effectiveStatStrings[ATTR_LEADERSHIP], ColorCodeModifier("{d}", plrLeadership));
			result += st_format_printf("\n" + tab + effectiveStatStrings[ATTR_STRENGTH], ColorCodeModifier("{d}", plrEffStrength));
			result += st_format_printf("\n" + tab + dialogueStrings[DIAL_GUN_DEADLINESS], ColorCodeModifier("{d}", plrGunDeadliness));
			break;
		case APPROACH_BUYSELL:
			salesSkill = MercProfile{ playerChar->ubProfile }.getInfo().weaponSaleModifier;
			result += st_format_printf(dialogueStrings[DIAL_WEAPON_SALES], ColorCodeModifier("{+d}%", salesSkill - 100));
			return result;
	}

	MERCPROFILESTRUCT const& plrProfile = MercProfile{ playerChar->ubProfile }.getStruct();
	MERCPROFILESTRUCT const& npcProfile = MercProfile{ npcChar->ubProfile }.getStruct();

	ST::string const plrNominalApprSkill = ColorCodeModifier("{+d}%", plrProfile.usApproachFactor[apprName - 1] - 100);
	ST::string const townLoyalty = ColorCodeModifier("{d}", gTownLoyalty[npcProfile.bTown].ubRating);
	ST::string const npcTownAttachment = ColorCodeModifier("{d}", npcProfile.bTownAttachment, false);
	ST::string const npcOpinionOnPlr = ColorCodeModifier("{+d}", npcProfile.bMercOpinion[playerChar->ubProfile]);
	int32_t const susceptibility = npcProfile.ubApproachVal[apprName - 1] - 100;
	ST::string const npcApprSusceptibility = ColorCodeModifier("{+d}%", susceptibility);

	if (apprName != APPROACH_THREATEN) {
		result += st_format_printf("\n" + tab + dialogueStrings[DIAL_NOMINAL_LEADERSHIP], ColorCodeModifier("{d}", plrProfile.bLeadership));
	}
	result += st_format_printf("\n" + tab + dialogueStrings[DIAL_APPROACH_SKILL], plrNominalApprSkill);
	result += st_format_printf("\n" + tab + locationStrings[LOC_TOWN_LOYALTY], townLoyalty);
	result += st_format_printf("\n" + tab + locationStrings[LOC_TOWN_ATTACHMENT], npcProfile.zNickname, isCheater ? npcTownAttachment : statusStrings[STR_STATUS_HIDDEN]);
	result += st_format_printf("\n" + tab + dialogueStrings[DIAL_OPINION], npcProfile.zNickname, plrProfile.zNickname, isCheater ? npcOpinionOnPlr : statusStrings[STR_STATUS_HIDDEN]);

	result += st_format_printf("\n" + tab + dialogueStrings[DIAL_SUSCEPTIBILITY], npcProfile.zNickname, isCheater ? npcApprSusceptibility : statusStrings[STR_STATUS_HIDDEN]);
	if (isCheater && susceptibility == 155) {
		result += st_format_printf("\n" + tab + tab + commentStrings[COMMENT_MORE_THAN], susceptibility);
	}
	int32_t npc1stTimeMod;
	ST::string apprMod;
	for (size_t apprIdx = APPROACH_FRIENDLY; apprIdx < APPROACH_RECRUIT; apprIdx++) {
		if (!(npcProfile.bApproached & gbFirstApproachFlags[apprIdx - 1]))
			continue;
		if (isCheater) {
			npc1stTimeMod = npcProfile.ubApproachMod[apprIdx - 1][apprName - 1] - 100;
			apprMod = ColorCodeModifier("{+d}%", npc1stTimeMod);
		} else {
			apprMod = statusStrings[STR_STATUS_HIDDEN];
		}
		result += st_format_printf("\n" + tab + tab + dialogueStrings[apprIdx], apprMod);
	}

	if (apprName == APPROACH_THREATEN) {
		uint8_t const plrEffExpLvlSmall = EffectiveExpLevel(playerChar) / 2;
		uint8_t const plrEffExpLvlLarge = EffectiveExpLevel(playerChar) - plrEffExpLvlSmall;
		uint8_t npcEffExpLvlSmall = EffectiveExpLevel(npcChar) / 2;
		uint8_t npcEffExpLvlLarge = EffectiveExpLevel(npcChar) - npcEffExpLvlSmall;
		uint8_t threatPoint{};
		if (npcProfile.bApproached & gbFirstApproachFlags[APPROACH_THREATEN - 1]) {
			threatPoint = 1;
		}
		ST::string plrRange = GetRangeString(plrEffExpLvlLarge, plrEffExpLvlSmall + plrEffExpLvlLarge);
		ST::string npcRange;
		if (isCheater) {
			npcRange = GetRangeString(npcEffExpLvlLarge + threatPoint, npcEffExpLvlSmall + npcEffExpLvlLarge + (threatPoint * 2), false);
		} else {
			npcRange = statusStrings[STR_STATUS_HIDDEN];
		}
		result +=                  "\n" + demarcationStrings[DIVIDER];
		result += st_format_printf("\n" + dialogueStrings[DIAL_FAST_DRAW], plrProfile.zNickname, npcProfile.zNickname, plrRange, npcRange);
		result +=                  "\n" + segmentHeaderStrings[HEADER_MODIFIED_BY];
		plrRange = GetRangeString(plrEffExpLvlLarge, plrEffExpLvlSmall + plrEffExpLvlLarge, true, true);
		npcRange = GetRangeString(npcEffExpLvlLarge, npcEffExpLvlSmall + npcEffExpLvlLarge, false, true);
		result += st_format_printf("\n" + tab + dialogueStrings[DIAL_POSSESIVE_EFF_EXPLEVEL], plrProfile.zNickname, plrRange);
		result += st_format_printf("\n" + tab + dialogueStrings[DIAL_POSSESIVE_EFF_EXPLEVEL], npcProfile.zNickname, isCheater ? npcRange : statusStrings[STR_STATUS_HIDDEN]);

		ST::string threatRange = st_format_printf(rangeStrings[RANGE_NORMAL], 1, 2);
		if (threatPoint > 0) {
			threatRange = st_format_printf(rangeStrings[RANGE_NORMAL], "~+~1", "~+~2");
		} else {
			threatRange = "^0";
		}
		result += st_format_printf("\n" + tab + dialogueStrings[DIAL_THREATEN], threatRange);
	}

	return result;
}
