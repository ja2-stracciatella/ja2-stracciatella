#ifndef __UI_CURSORS_H
#define __UI_CURSORS_H

#include "Handle_UI.h"
#include "Interface_Cursors.h"
#include "Item_Types.h"
#include "JA2Types.h"


UICursorID GetProperItemCursor(SOLDIERTYPE*, GridNo map_pos, BOOLEAN activated);

void HandleLeftClickCursor( SOLDIERTYPE *pSoldier );
void HandleRightClickAdjustCursor( SOLDIERTYPE *pSoldier, INT16 usMapPos );
void HandleWheelAdjustCursor( SOLDIERTYPE *pSoldier, INT16 usMapPos, INT8 bDirection );

ItemCursor GetActionModeCursor(SOLDIERTYPE const*);

void HandleUICursorRTFeedback( SOLDIERTYPE *pSoldier );

BOOLEAN GetMouseRecalcAndShowAPFlags(MouseMoveState*, BOOLEAN* pfShowAPs);

#endif
