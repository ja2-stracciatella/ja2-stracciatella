#include "Cheats.h"

#include "GameRes.h"

UINT8			gubCheatLevel;

/** Reset cheat level to initial value. */
void resetCheatLevelToInitialValue()
{
	#define	STARTING_CHEAT_LEVEL	0
	gubCheatLevel = STARTING_CHEAT_LEVEL;
}

/** Get cheat code. */
const char * getCheatCode()
{
  return isGermanVersion() ? "iguana" : "gabbi";
}
