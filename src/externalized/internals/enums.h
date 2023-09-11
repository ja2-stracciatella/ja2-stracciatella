#pragma once

#include "game/TacticalAI/NPC.h"

namespace Internals
{
	/** Return approach name or NULL if not found. */
	const char* getApproachName(enum Approach approach);

	/** Get action name. */
	const char* getActionName(UINT8 action);

	/** Get animation name. */
	const char* getAnimationName(UINT16 animation);
}
