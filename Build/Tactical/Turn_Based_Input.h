#ifndef TURN_BASED_INPUT_H
#define TURN_BASED_INPUT_H

#include "JA2Types.h"


extern const SOLDIERTYPE* gUITargetSoldier;

BOOLEAN ConfirmActionCancel(UINT16 usMapPos, UINT16 usOldMapPos);
BOOLEAN HandleUIReloading(SOLDIERTYPE* pSoldier);

#endif
