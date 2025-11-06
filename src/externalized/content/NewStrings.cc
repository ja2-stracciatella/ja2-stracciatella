#include "Items.h"
#include "Morale.h"
#include "NewStrings.h"
#include "SkillCheck.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Text.h"

#define demarcationStrings                                  (g_popupsRes->demarcationStrings)
#define booleanStrings                                      (g_popupsRes->booleanStrings)
#define segmentHeaderStrings                                (g_popupsRes->segmentHeaderStrings)
#define effectiveStatStrings                                (g_popupsRes->effectiveStatStrings)
#define skillPossessionStrings                              (g_popupsRes->skillPossessionStrings)
#define toolModifierStrings                                 (g_popupsRes->toolModifierStrings)
#define statusStrings                                       (g_popupsRes->statusStrings)
#define lockStrings                                         (g_popupsRes->lockStrings)
#define vitalSignStrings                                    (g_popupsRes->vitalSignStrings)
#define rangeStrings                                        (g_popupsRes->rangeStrings)
#define trapStrings                                         (g_popupsRes->trapStrings)
#define attemptStrings                                      (g_popupsRes->attemptStrings)
#define commentStrings                                      (g_popupsRes->commentStrings)
#define possessiveStrings                                   (g_popupsRes->possessiveStrings)

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
	EFF_HEALTH,
	ATTR_AGILITY,
	ATTR_DEXTERITY,
	ATTR_STRENGTH,
	ATTR_LEADERSHIP,
	ATTR_WISDOM,
	ATTR_EXPLEVEL,
	ATTR_MARKSMANSHIP,
	ATTR_MECHANICAL,
	ATTR_EXPLOSIVES,
	ATTR_MEDICAL,
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
	STR_STATUS_N_SLASH_A,
	STR_STATUS_UNUSABLE,
	STR_STATUS_IMPOSSIBLE,
	STR_STATUS_UNTRAPPED,
	STR_STATUS_TRAPPED,
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
	VITAL_FATIGUE,
	VITAL_IN_SHOCK,
	VITAL_SEVERE_BLEEDING,
	VITAL_MORALE
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
	COMMENT_TOOL_UNUSABEL,
};

enum
{
	POSSESSIVE_HAS_NO,
	POSSESSIVE_HAS,
	POSSESSIVE_HAS_EXPERT,
};

struct SKILLCHECKINFO
{
	bool isSufficientlyAttempted{};
	ST::string attemptsMade{ "^0" };
	ST::string attemptsNeeded{};
	ST::string attemptsFromWisdom{};
	bool isMechSkillRequiredAndZero{}, isExplSkillRequiredAndZero{}, isMedSkillRequiredAndZero{};
	bool isChanceRevealed{};
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
			reqTool.chanceComment = commentStrings[COMMENT_TOOL_UNUSABEL];
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
			inf.isChanceRevealed = true;
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
	ST::string trapPerception, trapLvlColored;
	if (d->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN) {
		trapPerception = statusStrings[STR_STATUS_HIDDEN];
	} else if (d->bPerceivedTrapped == DOOR_PERCEIVED_UNTRAPPED) {
		trapPerception = statusStrings[STR_STATUS_UNTRAPPED];
	} else if (d->bPerceivedTrapped == DOOR_PERCEIVED_TRAPPED) {
		trapPerception = statusStrings[STR_STATUS_TRAPPED];
	}
	trapLvlColored = skillCheckInf.isSufficientlyAttempted ? ColorCodeModifier("{d}", d->ubTrapLevel, false) : statusStrings[STR_STATUS_HIDDEN];

	ST::string result = st_format_printf("\n" + trapStrings[TRAP_PERCEPTION], trapPerception);
	if (d->bPerceivedTrapped == DOOR_PERCEIVED_TRAPPED) {
		result += st_format_printf("\n" + TacticalStr[DOOR_LOCK_DESCRIPTION_STR], GetTrapName(*d)); // "The lock has a silent alarm."
	}

	result += "\n" + demarcationStrings[DIVIDER];
	result += "\n" + segmentHeaderStrings[HEADER_TRAP_DETECTION]; // "Detection Level vs. Trap Level:"

	auto getRangeStr = [](int32_t lBound, int32_t uBound) -> const ST::string {
		ST::string result{};
		if (lBound != uBound) {
			ST::string const lBoundStr = lBound < 1 ? ST::format("{}", lBound) : ColorCodeModifier("{d}", lBound);
			ST::string const uBoundStr = lBound < 1 ? ST::format("{}", uBound) : ColorCodeModifier("{d}", uBound);
			result = st_format_printf(rangeStrings[RANGE_NORMAL], lBoundStr, uBoundStr);
			if (lBound < 1) {
				if (uBound > 1) {
					result += st_format_printf(rangeStrings[RANGE_EFFECTIVE], ColorCodeModifier("{d}", uBound));
				} else {
					result += rangeStrings[RANGE_NOT_RANGE];
				}
			}
		} else {
			result = ColorCodeModifier("{d}", lBound);
			if (lBound < 1) {
				result += rangeStrings[RANGE_NOT_RANGE];
			}
		}
		return result;
	};

	result += st_format_printf("\n" + tab + trapStrings[TRAP_PASSIVE_DETECTION], getRangeStr(lowerBound,     upperBound),           trapLvlColored);
	result += st_format_printf("\n" + tab + trapStrings[TRAP_ACTIVE_DETECTION],  getRangeStr(lowerBound + 1, upperBound + randMod), trapLvlColored);

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

	bool const isPerceivedTrapped = d->bPerceivedTrapped == DOOR_PERCEIVED_TRAPPED;
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
		CompareSkillAttempts(skillCheckInf, s, SMASH_DOOR_CHECK);
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
	ST::string const lockDifficulty = skillCheckInf.isChanceRevealed ? ColorCodeModifier("{d}%", lockDiffMod - fatigueMod) : statusStrings[STR_STATUS_HIDDEN];
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
