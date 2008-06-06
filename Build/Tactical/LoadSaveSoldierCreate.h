#ifndef LOADSAVESOLIDERCREATE_H
#define LOADSAVESOLIDERCREATE_H

#include "Soldier_Create.h"


const BYTE* ExtractSoldierCreateUTF16(const BYTE* Src, SOLDIERCREATE_STRUCT* Soldier);
void        ExtractSoldierCreateFromFileUTF16(HWFILE, SOLDIERCREATE_STRUCT*);
void        ExtractSoldierCreateFromFile(HWFILE, SOLDIERCREATE_STRUCT*);

void InjectSoldierCreateIntoFileUTF16(HWFILE, SOLDIERCREATE_STRUCT const*);
void InjectSoldierCreateIntoFile(HWFILE, SOLDIERCREATE_STRUCT const*);

#endif
