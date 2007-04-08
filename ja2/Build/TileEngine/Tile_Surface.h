#ifndef _TILE_SURFACE_H
#define _TILE_SURFACE_H

#include "WorldDef.h"

TILE_IMAGERY				*gTileSurfaceArray[ NUMBEROFTILETYPES ];


TILE_IMAGERY *LoadTileSurface( char * cFilename );

void DeleteTileSurface( PTILE_IMAGERY	pTileSurf );

void SetRaisedObjectFlag( char *cFilename, TILE_IMAGERY *pTileSurf );


#endif
