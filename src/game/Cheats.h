#ifndef _CHEATS__H_
#define _CHEATS__H_

#include "Types.h"
#include "GameMode.h"
#include <string_view>

inline UINT8 gubCheatLevel;

bool isGermanVersion();

#define INFORMATION_CHEAT_LEVEL( )	( (gubCheatLevel >= (isGermanVersion() ? 5 : 3)) || GameMode::getInstance()->debugging())
#define CHEATER_CHEAT_LEVEL( )		( gubCheatLevel >= (isGermanVersion() ? 6 : 5) )
#define DEBUG_CHEAT_LEVEL( )		( gubCheatLevel >= (isGermanVersion() ? 7 : 6) || GameMode::getInstance()->debugging())
#define RESET_CHEAT_LEVEL( )		( gubCheatLevel = 0 )

/** Get cheat code. */
inline std::string_view getCheatCode()
{
	return isGermanVersion() ? "iguana" : "gabbi";
}

#endif
