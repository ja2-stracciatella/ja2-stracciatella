#ifndef __UI_CURSORS_H
#define __UI_CURSORS_H


#define REFINE_PUNCH_1				0
#define REFINE_PUNCH_2				6

#define REFINE_KNIFE_1				0
#define REFINE_KNIFE_2				6


UINT8	GetProperItemCursor( UINT8 ubSoldierID, UINT16 ubItemIndex, UINT16 usMapPos, BOOLEAN fActivated );
void DetermineCursorBodyLocation( UINT8 ubSoldierID, BOOLEAN fDisplay, BOOLEAN fRecalc );

void HandleLeftClickCursor( SOLDIERTYPE *pSoldier );
void HandleRightClickAdjustCursor( SOLDIERTYPE *pSoldier, INT16 usMapPos );

UINT8 GetActionModeCursor( SOLDIERTYPE *pSoldier );

BOOLEAN gfCannotGetThrough;

void HandleUICursorRTFeedback( SOLDIERTYPE *pSoldier );
void HandleEndConfirmCursor( SOLDIERTYPE *pSoldier );

BOOLEAN GetMouseRecalcAndShowAPFlags( UINT32 *puiCursorFlags, BOOLEAN *pfShowAPs );


#endif
