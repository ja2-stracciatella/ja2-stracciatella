#pragma once

#include "Keys.h"
#include "Soldier_Profile_Type.h"
#include "NPC.h"

enum NewStrings
{
	NS_SOLDIER_EQUIPS_ITEM,
	NS_SOLDIER_REMOVES_ITEM,
	NS_SOMEONE_ELSE_IN_SECTOR,
	NS_INVALID_DROPOFF_SECTOR,
	NS_DIAGONALITY_WARNING,

	NS__LAST
};

// door menu
const ST::string GetModifiersForLockExam(SOLDIERTYPE* const s, DOOR* const d);
const ST::string GetModifiersForLockForceOpen(SOLDIERTYPE* const s, DOOR* const d, bool isCrowbarUsed);
const ST::string GetModifiersForLockUntrap(SOLDIERTYPE* const s, DOOR* const d);
const ST::string GetModifiersForLockPicking(SOLDIERTYPE* const s, DOOR* const d);
const ST::string GetModifiersForLockBlowUp(SOLDIERTYPE* const s);
// attribute list
const ST::string GetModifiersForEffectiveAttributes(SOLDIERTYPE* const s, Attributes const attrName);
// dialogue menu
const ST::string GetModifiersForDialogue(SOLDIERTYPE* const playerChar, SOLDIERTYPE* const npcChar, Approach const apprName);
