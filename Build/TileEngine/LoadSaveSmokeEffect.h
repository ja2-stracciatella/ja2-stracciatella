#ifndef LOADSAVESMOKEEFFECT_H
#define LOADSAVESMOKEEFFECT_H

#include "JA2Types.h"


BOOLEAN ExtractSmokeEffectFromFile(HWFILE file, SMOKEEFFECT* s);
BOOLEAN InjectSmokeEffectIntoFile(HWFILE file, const SMOKEEFFECT* s);

#endif
