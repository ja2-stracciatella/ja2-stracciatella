#ifndef __SMOOTH_H
#define __SMOOTH_H


//   Area (pointer to SGP rect) +
//      Location to check-+--|  |       |---- Check left and right edges -----|    |---- Check top and bottom edges -----|
#define IsLocationInArea( x, y, r )		( ((x) >= r->iLeft) && ((x) <= r->iRight) && ((y) >= r->iTop) && ((y) <= r->iBottom) )


void SmoothAllTerrainWorld( void );
void SmoothTerrain(int gridno, int origType, UINT16 *piNewTile, BOOLEAN fForceSmooth  );

void SmoothTerrainRadius( UINT32 iMapIndex, UINT32 uiCheckType, UINT8 ubRadius, BOOLEAN fForceSmooth );
void SmoothAllTerrainTypeRadius( UINT32 iMapIndex, UINT8 ubRadius, BOOLEAN fForceSmooth );

#endif
