#ifndef __MS_I_TOWNMINE_INFO_H
#define __MS_I_TOWNMINE_INFO_H

#include "JA2Types.h"

extern BOOLEAN fShowTownInfo;

// display the box
void DisplayTownInfo(const SGPSector& sMap);

// create or destroy the town info box..should only be directly called the exit code for mapscreen
void CreateDestroyTownInfoBox( void );

extern PopUpBox* ghTownMineBox;

#endif
