#ifndef LOADSAVESTRATEGICMAPELEMENT_H
#define LOADSAVESTRATEGICMAPELEMENT_H

#include "Strategic.h"


void ExtractStrategicMapElementFromFile(HWFILE, StrategicMapElement*);
void InjectStrategicMapElementIntoFile(HWFILE, StrategicMapElement const*);

#endif
