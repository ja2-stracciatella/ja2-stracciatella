#ifndef _TILE_SURFACE_H
#define _TILE_SURFACE_H

#include "TileDat.h"
#include <string_theory/string>
struct TILE_IMAGERY;

extern TILE_IMAGERY* gTileSurfaceArray[NUMBEROFTILETYPES];


TILE_IMAGERY* LoadTileSurface(ST::string const& cFilename);

void DeleteTileSurface(TILE_IMAGERY* pTileSurf);

void SetRaisedObjectFlag(ST::string const& filename, TILE_IMAGERY*);

#endif
