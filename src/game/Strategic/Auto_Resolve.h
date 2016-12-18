#ifndef __AUTO_RESOLVE_H
#define __AUTO_RESOLVE_H

#include "game/Tactical/Item_Types.h"
#include "game/JA2Types.h"
#include "game/ScreenIDs.h"


void EnterAutoResolveMode( UINT8 ubSectorX, UINT8 ubSectorY );

// is the autoresolve active?
BOOLEAN IsAutoResolveActive( void );

void EliminateAllEnemies( UINT8 ubSectorX, UINT8 ubSectorY );

void ConvertTacticalBattleIntoStrategicAutoResolveBattle(void);

UINT8 GetAutoResolveSectorID(void);

extern BOOLEAN gfTransferTacticalOppositionToAutoResolve;

//Returns TRUE if autoresolve is active or a sector is loaded.
BOOLEAN GetCurrentBattleSectorXYZ( INT16 *psSectorX, INT16 *psSectorY, INT16 *psSectorZ );

UINT32 VirtualSoldierDressWound(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pVictim, OBJECTTYPE* pKit, INT16 sKitPts, INT16 sStatus);

//Returns TRUE if a battle is happening ONLY
BOOLEAN GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(INT16* psSectorX, INT16* psSectorY, INT16* psSectorZ);

ScreenID AutoResolveScreenHandle(void);

#endif
