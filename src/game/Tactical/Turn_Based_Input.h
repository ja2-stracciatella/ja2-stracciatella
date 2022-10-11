#ifndef TURN_BASED_INPUT_H
#define TURN_BASED_INPUT_H

#include "JA2Types.h"
#include "MouseSystem.h"


extern const SOLDIERTYPE* gUITargetSoldier;

INT8    HandleMoveModeInteractiveClick(UINT16 usMapPos);
BOOLEAN HandleUIReloading(SOLDIERTYPE* pSoldier);
void TacticalViewPortTouchCallbackTB(MOUSE_REGION* region, UINT32 reason);

#endif
