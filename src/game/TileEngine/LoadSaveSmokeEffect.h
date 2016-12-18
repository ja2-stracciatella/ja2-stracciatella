#ifndef LOADSAVESMOKEEFFECT_H
#define LOADSAVESMOKEEFFECT_H

#include "game/JA2Types.h"


void ExtractSmokeEffectFromFile(HWFILE, SMOKEEFFECT*);
void InjectSmokeEffectIntoFile(HWFILE, SMOKEEFFECT const*);

#endif
