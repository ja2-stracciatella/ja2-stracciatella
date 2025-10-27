#ifndef SOLDIER_MACROS_H
#define SOLDIER_MACROS_H

// MACROS FOR EASIER SOLDIER CONTROL
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Assignments.h"
#include "Overhead_Types.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "TeamTurns.h"

static inline bool RPC_RECRUITED(SOLDIERTYPE const* const s)
{
	UINT8 const pid = s->ubProfile;
	return pid != NO_PROFILE && GetProfile(pid).ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED;
}

static inline bool AM_AN_EPC(SOLDIERTYPE const* const s)
{
	UINT8 const pid = s->ubProfile;
	return pid != NO_PROFILE && GetProfile(pid).ubMiscFlags & PROFILE_MISC_FLAG_EPCACTIVE;
}

static inline bool AM_A_ROBOT(SOLDIERTYPE const* const s)
{
	UINT8 const pid = s->ubProfile;
	return pid != NO_PROFILE && GetProfile(pid).ubBodyType == ROBOTNOWEAPON;
}

static inline bool IsMechanical(SOLDIERTYPE const& s)
{
	return s.uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(&s);
}

constexpr bool IsHostileToOurTeam(SOLDIERTYPE const& s) noexcept
{
	return !s.bNeutral && s.bSide != Side::FRIENDLY;
}

static inline bool OK_ENEMY_MERC(SOLDIERTYPE const* const s)
{
	return IsHostileToOurTeam(*s) && s->bLife >= OKLIFE;
}

static inline AnimationHeight GetStance(SOLDIERTYPE const& s)
{
	return static_cast<AnimationHeight>(gAnimControl[s.usAnimState].ubEndHeight);
}

// Checks if our guy can be controllable .... checks bInSector, team, on duty, etc...
static inline BOOLEAN OkControllableMerc(const SOLDIERTYPE* const s)
{
	Assert(s->bActive);
	return s->bLife >= OKLIFE &&
		s->bInSector &&
		s->bTeam == OUR_TEAM &&
		s->bAssignment < ON_DUTY;
}
#define OK_CONTROLLABLE_MERC(s)	((s)->bActive && OkControllableMerc((s)))

// Check if our guy can be selected and is not in a position where our team has an interupt and he does not have one
#define OK_INTERRUPT_MERC(p)	(!INTERRUPT_QUEUED || !(p)->bMoved)

#define CREATURE_OR_BLOODCAT(p)	((p)->uiStatusFlags & SOLDIER_MONSTER || (p)->ubBodyType == BLOODCAT)

#define TANK(p)			((p)->ubBodyType == TANK_NE || (p)->ubBodyType == TANK_NW)

#define OK_ENTERABLE_VEHICLE(p)	((p)->uiStatusFlags & SOLDIER_VEHICLE && !TANK((p)) && (p)->bLife >= OKLIFE)

#endif
