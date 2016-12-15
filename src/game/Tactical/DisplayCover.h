#ifndef _DISPLAY_COVER__H_
#define _DISPLAY_COVER__H_

#include "game/JA2Types.h"


void DisplayCoverOfSelectedGridNo(void);
void RemoveCoverOfSelectedGridNo(void);

void DisplayRangeToTarget(const SOLDIERTYPE* pSoldier, INT16 sTargetGridNo);


void RemoveVisibleGridNoAtSelectedGridNo(void);
void DisplayGridNoVisibleToSoldierGrid(void);

void ChangeSizeOfDisplayCover( INT32 iNewSize );

void ChangeSizeOfLOS( INT32 iNewSize );

#endif
