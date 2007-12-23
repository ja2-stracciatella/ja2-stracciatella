#ifndef LOADSAVESOLDIERTYPE_H
#define LOADSAVESOLDIERTYPE_H

#include "JA2Types.h"


void ExtractSoldierType(const BYTE* Src, SOLDIERTYPE* Soldier);

void InjectSoldierType(BYTE* Dst, const SOLDIERTYPE* Soldier);

#endif
