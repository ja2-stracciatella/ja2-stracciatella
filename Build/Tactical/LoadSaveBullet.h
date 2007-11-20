#ifndef LOADSAVEBULLET_H
#define LOADSAVEBULLET_H

#include "Bullets.h"


BOOLEAN ExtractBulletFromFile(HWFILE file, BULLET* b);
BOOLEAN InjectBulletIntoFile(HWFILE file, const BULLET* b);

#endif
