#ifndef __SOLDIER_FIND_H
#define __SOLDIER_FIND_H

#include "JA2Types.h"


#define FIND_SOLDIER_GRIDNO		0x000000004
#define FIND_SOLDIER_SAMELEVEL		0x000000008
#define FIND_SOLDIER_BEGINSTACK	0x000000040

// RETURN FLAGS FOR FINDSOLDIER
enum SoldierFindFlags
{
	NO_MERC          = 0,
	SELECTED_MERC    = 0x000000002,
	OWNED_MERC       = 0x000000004,
	ENEMY_MERC       = 0x000000008,
	UNCONSCIOUS_MERC = 0x000000020,
	DEAD_MERC        = 0x000000040,
	VISIBLE_MERC     = 0x000000080,
	NOINTERRUPT_MERC = 0x000000200,
	NEUTRAL_MERC     = 0x000000400
};
ENUM_BITSET(SoldierFindFlags)


#define FINDSOLDIERSAMELEVEL(l)	(FIND_SOLDIER_SAMELEVEL | (l) << 16)

SOLDIERTYPE* FindSoldier(GridNo, UINT32 flags);

bool IsOwnedMerc(SOLDIERTYPE const&);
SoldierFindFlags GetSoldierFindFlags(SOLDIERTYPE const&);

BOOLEAN CycleSoldierFindStack( UINT16 usMapPos );

bool GridNoOnScreen(GridNo);

BOOLEAN SoldierOnScreen(const SOLDIERTYPE* s);
BOOLEAN SoldierLocationRelativeToScreen(INT16 sGridNo, INT8* pbDirection, UINT32* puiScrollFlags);
void GetSoldierScreenPos(const SOLDIERTYPE* pSoldier, INT16* psScreenX, INT16* psScreenY);
void GetSoldierTRUEScreenPos(const SOLDIERTYPE* pSoldier, INT16* psScreenX, INT16* psScreenY);
BOOLEAN IsPointInSoldierBoundingBox( SOLDIERTYPE *pSoldier, INT16 sX, INT16 sY );
UINT16 FindRelativeSoldierPosition(const SOLDIERTYPE* pSoldier, INT16 sX, INT16 sY);

void GetGridNoScreenPos( INT16 sGridNo, UINT8 ubLevel, INT16 *psScreenX, INT16 *psScreenY );

BOOLEAN IsValidTargetMerc(const SOLDIERTYPE* s);

#endif
