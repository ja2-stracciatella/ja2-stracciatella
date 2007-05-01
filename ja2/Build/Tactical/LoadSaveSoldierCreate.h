#ifndef LOADSAVESOLIDERCREATE_H
#define LOADSAVESOLIDERCREATE_H

#include "Soldier_Create.h"


const BYTE* ExtractSoldierCreateUTF16(const BYTE* Src, SOLDIERCREATE_STRUCT* Soldier);
BOOLEAN ExtractSoldierCreateFromFileUTF16(HWFILE File, SOLDIERCREATE_STRUCT* Soldier);
BOOLEAN ExtractSoldierCreateFromFile(HWFILE File, SOLDIERCREATE_STRUCT* Soldier);

BOOLEAN InjectSoldierCreateIntoFileUTF16(HWFILE File, const SOLDIERCREATE_STRUCT* Soldier);
BOOLEAN InjectSoldierCreateIntoFile(HWFILE File, const SOLDIERCREATE_STRUCT* Soldier);

#endif
