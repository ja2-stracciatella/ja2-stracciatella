#ifndef LOADSAVESTRATEGICMAPELEMENT_H
#define LOADSAVESTRATEGICMAPELEMENT_H

#include "Strategic.h"


BOOLEAN ExtractStrategicMapElementFromFile(HWFILE, StrategicMapElement*);
BOOLEAN InjectStrategicMapElementIntoFile(HWFILE, const StrategicMapElement*);

#endif
