#ifndef LOADSAVEVEHICLETYPE_H
#define LOADSAVEVEHICLETYPE_H

#include "Strategic.h"


BOOLEAN ExtractStrategicMapElementFromFile(HWFILE, StrategicMapElement*);
BOOLEAN InjectStrategicMapElementIntoFile(HWFILE, const StrategicMapElement*);

#endif
