#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "Types.h"
#include <string_theory/string>


BOOLEAN CreateSGPPaletteFromCOLFile(SGPPaletteEntry* pal, const char* col_file);

void DisplayPaletteRep(const ST::string& aPalRep, UINT8 ubXPos, UINT8 ubYPos, SGPVSurface* dst);

#endif
