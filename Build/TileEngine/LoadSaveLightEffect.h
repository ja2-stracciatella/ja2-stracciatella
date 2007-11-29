#ifndef LOADSAVELIGHTEFFECT_H
#define LOADSAVELIGHTEFFECT_H

#include "LightEffects.h"


BOOLEAN ExtractLightEffectFromFile(HWFILE file, LIGHTEFFECT* l);
BOOLEAN InjectLightEffectIntoFile(HWFILE file, const LIGHTEFFECT* l);

#endif
