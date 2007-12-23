#ifndef LOADSAVESECTORINFO_H
#define LOADSAVESECTORINFO_H

#include "Campaign_Types.h"


BOOLEAN ExtractSectorInfoFromFile(HWFILE, SECTORINFO*);
BOOLEAN InjectSectorInfoIntoFile(HWFILE, const SECTORINFO*);

#endif
