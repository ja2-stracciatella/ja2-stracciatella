#ifndef LOADSAVEEXPLOSIONTYPE_H
#define LOADSAVEEXPLOSIONTYPE_H

#include "Explosion_Control.h"


BOOLEAN ExtractExplosionTypeFromFile(HWFILE file, EXPLOSIONTYPE* e);
BOOLEAN InjectExplosionTypeIntoFile(HWFILE file, const EXPLOSIONTYPE* e);

#endif
