#ifndef TURN_BASED_INPUT_H
#define TURN_BASED_INPUT_H

#include "game/JA2Types.h"


extern const SOLDIERTYPE* gUITargetSoldier;

INT8    HandleMoveModeInteractiveClick(UINT16 usMapPos);
BOOLEAN HandleUIReloading(SOLDIERTYPE* pSoldier);

#endif
