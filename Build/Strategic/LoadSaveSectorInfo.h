#ifndef LOADSAVESECTORINFO_H
#define LOADSAVESECTORINFO_H

#include "Campaign_Types.h"


void ExtractSectorInfoFromFile(HWFILE, SECTORINFO&);
void InjectSectorInfoIntoFile(HWFILE, SECTORINFO const&);

#endif
