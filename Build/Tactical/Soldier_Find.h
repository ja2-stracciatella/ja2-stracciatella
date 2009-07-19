#ifndef __SOLDIER_FIND_H
#define __SOLDIER_FIND_H

#include "JA2Types.h"


#define			FIND_SOLDIER_GRIDNO			0x000000004
#define			FIND_SOLDIER_SAMELEVEL	0x000000008
#define			FIND_SOLDIER_BEGINSTACK	0x000000040

// RETURN FLAGS FOR FINDSOLDIER
#define			SELECTED_MERC				0x000000002
#define			OWNED_MERC					0x000000004
#define			ENEMY_MERC					0x000000008
#define     UNCONSCIOUS_MERC		0x000000020
#define			DEAD_MERC						0x000000040
#define			VISIBLE_MERC				0x000000080
#define			NOINTERRUPT_MERC		0x000000200
#define			NEUTRAL_MERC				0x000000400


#define			FINDSOLDIERSAMELEVEL(l) (FIND_SOLDIER_SAMELEVEL | (l) << 16)


SOLDIERTYPE* FindSoldierFromMouse(void);
SOLDIERTYPE* FindSoldier(INT16 sGridNo, UINT32 uiFlags);

bool IsOwnedMerc(SOLDIERTYPE const&);
UINT32 GetSoldierFindFlags(const SOLDIERTYPE* s);

BOOLEAN CycleSoldierFindStack( UINT16 usMapPos );

BOOLEAN GridNoOnScreen( INT16 sGridNo );

BOOLEAN SoldierOnScreen(const SOLDIERTYPE* s);
BOOLEAN SoldierLocationRelativeToScreen(INT16 sGridNo, INT8* pbDirection, UINT32* puiScrollFlags);
void GetSoldierScreenPos(const SOLDIERTYPE* pSoldier, INT16* psScreenX, INT16* psScreenY);
void GetSoldierTRUEScreenPos(const SOLDIERTYPE* pSoldier, INT16* psScreenX, INT16* psScreenY);
BOOLEAN IsPointInSoldierBoundingBox( SOLDIERTYPE *pSoldier, INT16 sX, INT16 sY );
UINT16 FindRelativeSoldierPosition(const SOLDIERTYPE* pSoldier, INT16 sX, INT16 sY);

void GetGridNoScreenPos( INT16 sGridNo, UINT8 ubLevel, INT16 *psScreenX, INT16 *psScreenY );

BOOLEAN IsValidTargetMerc(const SOLDIERTYPE* s);

#endif
