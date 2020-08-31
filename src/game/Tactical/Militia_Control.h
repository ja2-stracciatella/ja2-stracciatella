#ifndef __MILITIA_CONTROL_H
#define __MILITIA_CONTROL_H

#include "Types.h"

void PrepareMilitiaForTactical(void);
void ResetMilitia(void);
void HandleMilitiaPromotions();

extern BOOLEAN gfStrategicMilitiaChangesMade;

#endif
