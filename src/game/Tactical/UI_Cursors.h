#ifndef __UI_CURSORS_H
#define __UI_CURSORS_H

#include "Interface_Cursors.h"


UICursorID GetProperItemCursor(SOLDIERTYPE*, GridNo map_pos, BOOLEAN activated);

void HandleLeftClickCursor( SOLDIERTYPE *pSoldier );
void HandleRightClickAdjustCursor( SOLDIERTYPE *pSoldier, INT16 usMapPos );

ItemCursor GetActionModeCursor(SOLDIERTYPE const*);

void HandleUICursorRTFeedback( SOLDIERTYPE *pSoldier );

BOOLEAN GetMouseRecalcAndShowAPFlags(MouseMoveState*, BOOLEAN* pfShowAPs);

#endif
