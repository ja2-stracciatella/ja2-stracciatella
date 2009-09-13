#ifndef STRATEGIC_MOVEMENT_COSTS_H
#define STRATEGIC_MOVEMENT_COSTS_H

#include "Types.h"


void InitStrategicMovementCosts();

UINT8 GetTraversability(INT16 sStartSector, INT16 sEndSector);

bool SectorIsPassable(INT16 sSector);

#endif
