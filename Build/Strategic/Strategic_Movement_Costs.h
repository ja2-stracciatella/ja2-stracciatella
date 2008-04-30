#ifndef STRATEGIC_MOVEMENT_COSTS_H
#define STRATEGIC_MOVEMENT_COSTS_H

#include "Types.h"


void InitStrategicMovementCosts(void);

UINT8 GetTraversability(INT16 sStartSector, INT16 sEndSector);

BOOLEAN SectorIsImpassable(INT16 sSector);

#endif
