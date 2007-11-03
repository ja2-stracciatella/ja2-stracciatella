#ifndef __SOLDIER_FIND_H
#define __SOLDIER_FIND_H

#include "Soldier_Control.h"


#define			FIND_SOLDIER_FULL				0x000000002
#define			FIND_SOLDIER_GRIDNO			0x000000004
#define			FIND_SOLDIER_SAMELEVEL	0x000000008
#define			FIND_SOLDIER_SELECTIVE	0x000000020
#define			FIND_SOLDIER_BEGINSTACK	0x000000040

// RETURN FLAGS FOR FINDSOLDIER
#define			SELECTED_MERC				0x000000002
#define			OWNED_MERC					0x000000004
#define			ENEMY_MERC					0x000000008
#define     UNCONSCIOUS_MERC		0x000000020
#define			DEAD_MERC						0x000000040
#define			VISIBLE_MERC				0x000000080
#define			ONDUTY_MERC					0x000000100
#define			NOINTERRUPT_MERC		0x000000200
#define			NEUTRAL_MERC				0x000000400


#define			FINDSOLDIERSAMELEVEL( l )		(	( ( FIND_SOLDIER_FULL | FIND_SOLDIER_SAMELEVEL ) | ( l << 16 ) ) )

#define			FINDSOLDIERSELECTIVESAMELEVEL( l )		(	( ( FIND_SOLDIER_SELECTIVE | FIND_SOLDIER_SAMELEVEL ) | ( l << 16 ) ) )


BOOLEAN FindSoldierFromMouse( UINT16 *pusSoldierIndex, UINT32 *pMercFlags );
BOOLEAN FindSoldier( INT16 sGridNo, UINT16 *pusSoldierIndex, UINT32 *pMercFlags , UINT32 uiFlags );
SOLDIERTYPE * SimpleFindSoldier( INT16 sGridNo, INT8 bLevel );

BOOLEAN CycleSoldierFindStack( UINT16 usMapPos );

BOOLEAN GridNoOnScreen( INT16 sGridNo );

BOOLEAN SoldierOnScreen(const SOLDIERTYPE* s);
BOOLEAN SoldierLocationRelativeToScreen( INT16 sGridNo, UINT16 usReasonID, INT8 *pbDirection, UINT32 *puiScrollFlags );
void GetSoldierScreenPos(const SOLDIERTYPE* pSoldier, INT16* psScreenX, INT16* psScreenY);
void GetSoldierAnimDims( SOLDIERTYPE *pSoldier, INT16 *psHeight, INT16 *psWidth );
void GetSoldierAnimOffsets(const SOLDIERTYPE* pSoldier, INT16* sOffsetX, INT16* sOffsetY);
void GetSoldierTRUEScreenPos(const SOLDIERTYPE* pSoldier, INT16* psScreenX, INT16* psScreenY);
BOOLEAN IsPointInSoldierBoundingBox( SOLDIERTYPE *pSoldier, INT16 sX, INT16 sY );
BOOLEAN FindRelativeSoldierPosition( SOLDIERTYPE *pSoldier, UINT16 *usFlags, INT16 sX, INT16 sY );

UINT8 QuickFindSoldier( INT16 sGridNo );
void GetGridNoScreenPos( INT16 sGridNo, UINT8 ubLevel, INT16 *psScreenX, INT16 *psScreenY );



#endif
