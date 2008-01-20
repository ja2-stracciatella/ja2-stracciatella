#ifndef LOADSAVELIGHTSPRITE_H
#define LOADSAVELIGHTSPRITE_H

#include "Lighting.h"


void ExtractLightSprite(const BYTE** data, UINT32 light_time);
BOOLEAN InjectLightSpriteIntoFile(HWFILE, const LIGHT_SPRITE*);

#endif
