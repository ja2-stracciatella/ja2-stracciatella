#ifndef MEDICAL_H
#define MEDICAL_H

#include "game/JA2Types.h"


// Can this soldier autobandage others in sector
BOOLEAN CanCharacterAutoBandageTeammate(const SOLDIERTYPE* s);

// Can this grunt be bandaged by a teammate?
BOOLEAN CanCharacterBeAutoBandagedByTeammate(const SOLDIERTYPE* s);

#endif
