#ifndef LOADSAVEVEHICLETYPE_H
#define LOADSAVEVEHICLETYPE_H

#include "Vehicles.h"


BOOLEAN ExtractVehicleTypeFromFile(HWFILE file, VEHICLETYPE* v, UINT32 savegame_version);
BOOLEAN InjectVehicleTypeIntoFile(HWFILE file, const VEHICLETYPE* v);

#endif
