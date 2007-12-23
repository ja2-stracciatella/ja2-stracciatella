#ifndef LOADSAVEVEHICLETYPE_H
#define LOADSAVEVEHICLETYPE_H

#include "JA2Types.h"


BOOLEAN ExtractVehicleTypeFromFile(HWFILE file, VEHICLETYPE* v, UINT32 savegame_version);
BOOLEAN InjectVehicleTypeIntoFile(HWFILE file, const VEHICLETYPE* v);

#endif
