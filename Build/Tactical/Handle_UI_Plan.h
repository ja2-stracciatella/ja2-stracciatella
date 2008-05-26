#ifndef __HANDLEUIPLAN_H
#define __HANDLEUIPLAN_H

#include "JA2Types.h"


#define		UIPLAN_ACTION_MOVETO			1
#define		UIPLAN_ACTION_FIRE				2


void    BeginUIPlan(SOLDIERTYPE* pSoldier);
BOOLEAN AddUIPlan( UINT16 sGridNo, UINT8 ubPlanID );
void EndUIPlan(void);
BOOLEAN InUIPlanMode(void);


#endif
