#ifndef LOADSAVEREALOBJECT_H
#define LOADSAVEREALOBJECT_H

#include "Physics.h"


void ExtractRealObjectFromFile(HWFILE, REAL_OBJECT*);
void InjectRealObjectIntoFile(HWFILE, REAL_OBJECT const*);

#endif
