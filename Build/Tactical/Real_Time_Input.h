#ifndef REAL_TIME_INPUT_H
#define REAL_TIME_INPUT_H

#include "Types.h"

extern BOOLEAN gfBeginBurstSpreadTracking;

extern BOOLEAN gfRTClickLeftHoldIntercepted;

void GetRTMouseButtonInput(UINT32* puiNewEvent);
void GetRTMousePositionInput(UINT32* puiNewEvent);

#endif
