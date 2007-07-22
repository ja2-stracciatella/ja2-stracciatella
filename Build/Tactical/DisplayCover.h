#ifndef _DISPLAY_COVER__H_
#define _DISPLAY_COVER__H_

#include "Types.h"
#include "Soldier_Control.h"

void	DisplayCoverOfSelectedGridNo( );
void	RemoveCoverOfSelectedGridNo();

void DisplayRangeToTarget( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo );


void RemoveVisibleGridNoAtSelectedGridNo();
void DisplayGridNoVisibleToSoldierGrid( );

void ChangeSizeOfDisplayCover( INT32 iNewSize );

void ChangeSizeOfLOS( INT32 iNewSize );

#endif
