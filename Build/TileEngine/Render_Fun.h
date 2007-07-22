#ifndef __RENDER_FUN_H
#define __RENDER_FUN_H

#include "WorldDef.h"

#define		NO_ROOM			0
#define   MAX_ROOMS		250


UINT8						gubWorldRoomHidden[ MAX_ROOMS ];
UINT8						gubWorldRoomInfo[ WORLD_MAX ];


BOOLEAN InitRoomDatabase( );

void RemoveRoomRoof( UINT16 sGridNo, UINT8 bRoomNum, SOLDIERTYPE *pSoldier );
BOOLEAN InARoom( UINT16 sGridNo, UINT8 *pubRoomNo );
BOOLEAN InAHiddenRoom( UINT16 sGridNo, UINT8 *pubRoomNo );

void SetGridNoRevealedFlag( UINT16 sGridNo );

void ExamineGridNoForSlantRoofExtraGraphic( UINT16 sCheckGridNo );

void SetRecalculateWireFrameFlagRadius(INT16 sX, INT16 sY, INT16 sRadius);


#endif
