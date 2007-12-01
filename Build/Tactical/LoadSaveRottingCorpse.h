#ifndef LOADSAVEROTTINGCORPSE_H
#define LOADSAVEROTTINGCORPSE_H

#include "Rotting_Corpses.h"


BOOLEAN ExtractRottingCorpseFromFile(HWFILE file, ROTTING_CORPSE_DEFINITION* b);
BOOLEAN InjectRottingCorpseIntoFile(HWFILE file, const ROTTING_CORPSE_DEFINITION* b);

#endif
