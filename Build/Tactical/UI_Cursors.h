#ifndef __UI_CURSORS_H
#define __UI_CURSORS_H

#include "Interface_Cursors.h"


#define REFINE_PUNCH_1				0
#define REFINE_PUNCH_2				6

#define REFINE_KNIFE_1				0
#define REFINE_KNIFE_2				6


UICursorID GetProperItemCursor(SOLDIERTYPE*, UINT16 usMapPos, BOOLEAN fActivated);

void HandleLeftClickCursor( SOLDIERTYPE *pSoldier );
void HandleRightClickAdjustCursor( SOLDIERTYPE *pSoldier, INT16 usMapPos );

ItemCursor GetActionModeCursor(SOLDIERTYPE const*);

void HandleUICursorRTFeedback( SOLDIERTYPE *pSoldier );

BOOLEAN GetMouseRecalcAndShowAPFlags( UINT32 *puiCursorFlags, BOOLEAN *pfShowAPs );

#endif
