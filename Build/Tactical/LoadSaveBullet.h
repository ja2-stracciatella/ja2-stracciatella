#ifndef LOADSAVEBULLET_H
#define LOADSAVEBULLET_H

#include "Bullets.h"


void ExtractBulletFromFile(HWFILE, BULLET*);
void InjectBulletIntoFile(HWFILE, const BULLET*);

#endif
