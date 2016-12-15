#ifndef LOADSAVEBASICSOLDIERCREATESTRUCT_H
#define LOADSAVEBASICSOLDIERCREATESTRUCT_H

#include "game/JA2Types.h"


void ExtractBasicSoldierCreateStructFromFile(HWFILE, BASIC_SOLDIERCREATE_STRUCT&);
void InjectBasicSoldierCreateStructIntoFile(HWFILE, BASIC_SOLDIERCREATE_STRUCT const&);

#endif
