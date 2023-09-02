#ifndef __RENDER_FUN_H
#define __RENDER_FUN_H

#include "JA2Types.h"
#include "WorldDef.h"

#define NO_ROOM		0
#define MAX_ROOMS	250


extern UINT8 gubWorldRoomHidden[MAX_ROOMS];
extern UINT8 gubWorldRoomInfo[WORLD_MAX];


void InitRoomDatabase(void);

void RemoveRoomRoof( UINT16 sGridNo, UINT8 bRoomNum, SOLDIERTYPE *pSoldier );

UINT8 GetRoom(UINT16 gridno);
BOOLEAN InAHiddenRoom( UINT16 sGridNo, UINT8 *pubRoomNo );

void SetGridNoRevealedFlag( UINT16 sGridNo );

void ExamineGridNoForSlantRoofExtraGraphic(GridNo);

void SetRecalculateWireFrameFlagRadius(GridNo pos, INT16 sRadius);

#endif
