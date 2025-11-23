#pragma once

#include "Keys.h"

enum NewStrings
{
	NS_SOLDIER_EQUIPS_ITEM,
	NS_SOLDIER_REMOVES_ITEM,
	NS_SOMEONE_ELSE_IN_SECTOR,
	NS_INVALID_DROPOFF_SECTOR,
	NS_DIAGONALITY_WARNING,

	NS__LAST
};

enum Attributes
{
	ATTR_AGILITY,
	ATTR_DEXTERITY,
	ATTR_STRENGTH,
	ATTR_LEADERSHIP,
	ATTR_WISDOM,
	ATTR_EXPLEVEL,
	ATTR_MARKSMANSHIP,
	ATTR_EXPLOSIVES,
	ATTR_MECHANICAL,
	ATTR_MEDICAL,
	NUM_ATTRIBUTES
};
// door menu
const ST::string GetModifiersForLockExam(SOLDIERTYPE* const s, DOOR* const d);
const ST::string GetModifiersForLockForceOpen(SOLDIERTYPE* const s, DOOR* const d, bool isCrowbarUsed);
const ST::string GetModifiersForLockUntrap(SOLDIERTYPE* const s, DOOR* const d);
const ST::string GetModifiersForLockPicking(SOLDIERTYPE* const s, DOOR* const d);
const ST::string GetModifiersForLockBlowUp(SOLDIERTYPE* const s);
// attribute list
const ST::string GetModifiersForEffectiveAttributes(SOLDIERTYPE* const s, Attributes const attrName);
