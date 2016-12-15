#ifndef LOADSAVESOLDIERTYPE_H
#define LOADSAVESOLDIERTYPE_H

#include "game/JA2Types.h"


void ExtractSoldierType(const BYTE* Src, SOLDIERTYPE* Soldier, bool stracLinuxFormat, UINT32 uiSavedGameVersion);

void InjectSoldierType(BYTE* Dst, const SOLDIERTYPE* Soldier);

#endif
