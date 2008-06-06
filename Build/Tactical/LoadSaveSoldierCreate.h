#ifndef LOADSAVESOLIDERCREATE_H
#define LOADSAVESOLIDERCREATE_H

#include "Soldier_Create.h"


const BYTE* ExtractSoldierCreateUTF16(const BYTE* Src, SOLDIERCREATE_STRUCT* Soldier);
BOOLEAN ExtractSoldierCreateFromFileUTF16(HWFILE File, SOLDIERCREATE_STRUCT* Soldier);
void        ExtractSoldierCreateFromFile(HWFILE, SOLDIERCREATE_STRUCT*);

BOOLEAN InjectSoldierCreateIntoFileUTF16(HWFILE File, const SOLDIERCREATE_STRUCT* Soldier);
void    InjectSoldierCreateIntoFile(HWFILE, SOLDIERCREATE_STRUCT const*);

#endif
