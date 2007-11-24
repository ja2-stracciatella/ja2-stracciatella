#ifndef LOADSAVEREALOBJECT_H
#define LOADSAVEREALOBJECT_H

#include "Physics.h"


BOOLEAN ExtractRealObjectFromFile(HWFILE file, REAL_OBJECT* o);
BOOLEAN InjectRealObjectIntoFile(HWFILE file, const REAL_OBJECT* o);

#endif
