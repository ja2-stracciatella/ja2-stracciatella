#ifndef __MS_I_TOWNMINE_INFO_H
#define __MS_I_TOWNMINE_INFO_H

#include "JA2Types.h"

extern BOOLEAN fShowTownInfo;

// display the box
void DisplayTownInfo( INT16 sMapX, INT16 sMapY, INT8 bMapZ );

// create or destroy the town info box..should only be directly called the exit code for mapscreen
void CreateDestroyTownInfoBox( void );

extern PopUpBox* ghTownMineBox;

#endif
