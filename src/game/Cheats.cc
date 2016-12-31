#include "Cheats.h"

#include "GameRes.h"

UINT8			gubCheatLevel;

/** Get cheat code. */
const char * getCheatCode()
{
  return isGermanVersion() ? "iguana" : "gabbi";
}
