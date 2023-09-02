#ifndef __SMOOTH_H
#define __SMOOTH_H

#include "Types.h"

void SmoothAllTerrainWorld( void );
void SmoothTerrain(int gridno, int origType, UINT16 *piNewTile, BOOLEAN fForceSmooth  );

void SmoothTerrainRadius( UINT32 iMapIndex, UINT32 uiCheckType, UINT8 ubRadius, BOOLEAN fForceSmooth );
void SmoothAllTerrainTypeRadius( UINT32 iMapIndex, UINT8 ubRadius, BOOLEAN fForceSmooth );

#endif
