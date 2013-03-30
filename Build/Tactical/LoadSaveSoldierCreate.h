#ifndef LOADSAVESOLIDERCREATE_H
#define LOADSAVESOLIDERCREATE_H

#include "Soldier_Create.h"


void ExtractSoldierCreateFromFile(HWFILE, SOLDIERCREATE_STRUCT*, bool stracLinuxFormat);
void InjectSoldierCreateIntoFile(HWFILE, SOLDIERCREATE_STRUCT const*);

#endif
