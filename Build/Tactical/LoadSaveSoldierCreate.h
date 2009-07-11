#ifndef LOADSAVESOLIDERCREATE_H
#define LOADSAVESOLIDERCREATE_H

#include "Soldier_Create.h"


void ExtractSoldierCreateFromFileUTF16(HWFILE, SOLDIERCREATE_STRUCT*);
void ExtractSoldierCreateFromFile(HWFILE, SOLDIERCREATE_STRUCT*);

void InjectSoldierCreateIntoFileUTF16(HWFILE, SOLDIERCREATE_STRUCT const*);
void InjectSoldierCreateIntoFile(HWFILE, SOLDIERCREATE_STRUCT const*);

#endif
