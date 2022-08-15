#ifndef REAL_TIME_INPUT_H
#define REAL_TIME_INPUT_H

#include "Handle_UI.h"
#include "Types.h"

extern BOOLEAN gfBeginBurstSpreadTracking;

extern BOOLEAN gfRTClickLeftHoldIntercepted;

void GetRTMouseButtonInput(UIEventKind* puiNewEvent);
void GetRTMousePositionInput(UIEventKind* puiNewEvent);
void TacticalViewPortTouchCallbackRT(MOUSE_REGION* region, UINT32 reason);

#endif
