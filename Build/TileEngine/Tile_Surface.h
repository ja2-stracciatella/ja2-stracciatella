#ifndef _TILE_SURFACE_H
#define _TILE_SURFACE_H

#include "WorldDef.h"


extern TILE_IMAGERY* gTileSurfaceArray[NUMBEROFTILETYPES];


TILE_IMAGERY* LoadTileSurface(const char* cFilename);

void DeleteTileSurface(TILE_IMAGERY* pTileSurf);

void SetRaisedObjectFlag(char const* filename, TILE_IMAGERY*);

#endif
