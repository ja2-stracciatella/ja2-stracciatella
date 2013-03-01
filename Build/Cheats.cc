#include "Cheats.h"

#include "GameRes.h"

UINT8			gubCheatLevel;

/** Reset cheat level to initial value. */
void resetCheatLevelToInitialValue()
{
  if(isGermanVersion())
  {
	#ifdef JA2TESTVERSION
		#define						STARTING_CHEAT_LEVEL						7
	#elif defined JA2BETAVERSION
		#define						STARTING_CHEAT_LEVEL						5
	#else
		#define						STARTING_CHEAT_LEVEL						0
	#endif
    gubCheatLevel = STARTING_CHEAT_LEVEL;
  }
  else
  {
	#ifdef JA2TESTVERSION
		#define						STARTING_CHEAT_LEVEL						6
	#elif defined JA2BETAVERSION
		#define						STARTING_CHEAT_LEVEL						3
	#else
		#define						STARTING_CHEAT_LEVEL						0
	#endif
    gubCheatLevel = STARTING_CHEAT_LEVEL;
  }
}

/** Get cheat code. */
const char * getCheatCode()
{
  return isGermanVersion() ? "iguana" : "gabbi";
}
