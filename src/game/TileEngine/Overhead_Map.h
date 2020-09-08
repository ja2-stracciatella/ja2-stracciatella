#ifndef __OVERHEADMAP_H
#define __OVERHEADMAP_H

#include "JA2Types.h"
#include "Types.h"
#include "World_Tileset_Enums.h"


void InitNewOverheadDB(TileSetID);
void RenderOverheadMap( INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS, BOOLEAN fFromMapUtility );


void HandleOverheadMap(void);
BOOLEAN InOverheadMap(void);
void GoIntoOverheadMap(void);
void KillOverheadMap(void);

void CalculateRestrictedMapCoords( INT8 bDirection, INT16 *psX1, INT16 *psY1, INT16 *psX2, INT16 *psY2, INT16 sEndXS, INT16 sEndYS );

void TrashOverheadMap(void);

GridNo GetOverheadMouseGridNo(void);

extern BOOLEAN gfOverheadMapDirty;

#endif
