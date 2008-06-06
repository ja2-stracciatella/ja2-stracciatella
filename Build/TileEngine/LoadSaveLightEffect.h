#ifndef LOADSAVELIGHTEFFECT_H
#define LOADSAVELIGHTEFFECT_H

#include "LightEffects.h"


void ExtractLightEffectFromFile(HWFILE, LIGHTEFFECT* l);
void InjectLightEffectIntoFile(HWFILE, LIGHTEFFECT const* l);

#endif
