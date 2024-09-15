#pragma once

#include "Animation_Data.h"
#include "game/TacticalAI/NPC.h"
#include "MapScreen.h"
#include "MercProfileInfo.h"
#include "Overhead_Types.h"
#include "Soldier_Control.h"
#include "Soldier_Profile_Type.h"
#include "Quests.h"

namespace Internals
{
	/** Return approach name or NULL if not found. */
	const char* getApproachName(enum Approach approach);
	Approach getApproachEnumFromString(const ST::string& s);

	/** Get action name. */
	const char* getActionName(UINT8 action);

	/** Get animation name. */
	const char* getAnimationName(UINT16 animation);

	/** Get inventory slot name. */
	const char* getInventorySlotName(enum InvSlotPos slot);
	InvSlotPos getInventorySlotEnumFromString(const ST::string& s);

	/** Get civilian group name. */
	const char* getCivilianGroupName(enum CivilianGroup group);
	CivilianGroup getCivilianGroupEnumFromString(const ST::string& s);

	/** Get body type name. */
	const char* getBodyTypeName(enum SoldierBodyType group);
	SoldierBodyType getBodyTypeEnumFromString(const ST::string& s);

	/** Get attitude name. */
	const char* getAttitudeName(enum Attitudes attitude);
	Attitudes getAttitudeEnumFromString(const ST::string& s);

	/** Get personality trait name. */
	const char* getPersonalityTraitName(enum PersonalityTrait trait);
	PersonalityTrait getPersonalityTraitEnumFromString(const ST::string& s);

	/** Get skill trait name. */
	const char* getSkillTraitName(enum SkillTrait trait);
	SkillTrait getSkillTraitEnumFromString(const ST::string& s);

	const char* getMercTypeName(enum MercType mercType);
	MercType getMercTypeEnumFromString(const ST::string& s);

	Quests getQuestEnumFromString(const ST::string& s);
}
