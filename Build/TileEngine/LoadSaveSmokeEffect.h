#ifndef LOADSAVESMOKEEFFECT_H
#define LOADSAVESMOKEEFFECT_H

#include "JA2Types.h"


BOOLEAN ExtractSmokeEffectFromFile(const HWFILE file, SMOKEEFFECT* const s);
BOOLEAN InjectSmokeEffectIntoFile(const HWFILE file, const SMOKEEFFECT* s);

#endif
