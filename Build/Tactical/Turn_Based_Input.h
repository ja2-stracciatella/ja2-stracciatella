#ifndef TURN_BASED_INPUT_H
#define TURN_BASED_INPUT_H

#include "Soldier_Control.h"
#include "Types.h"

extern UINT32 guiUITargetSoldierId;

BOOLEAN ConfirmActionCancel(UINT16 usMapPos, UINT16 usOldMapPos);
BOOLEAN HandleUIReloading(SOLDIERTYPE* pSoldier);

#endif
