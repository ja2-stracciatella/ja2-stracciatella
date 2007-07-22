#ifndef __OVERHEADMAP_H
#define __OVERHEADMAP_H

void InitNewOverheadDB( UINT8 ubTilesetID );
void RenderOverheadMap( INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS, BOOLEAN fFromMapUtility );


void HandleOverheadMap( );
BOOLEAN InOverheadMap( );
void GoIntoOverheadMap( );
void KillOverheadMap();

void CalculateRestrictedMapCoords( INT8 bDirection, INT16 *psX1, INT16 *psY1, INT16 *psX2, INT16 *psY2, INT16 sEndXS, INT16 sEndYS );

void TrashOverheadMap( );

#endif
