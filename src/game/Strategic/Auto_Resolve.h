#ifndef __AUTO_RESOLVE_H
#define __AUTO_RESOLVE_H

#include "ScreenIDs.h"
#include "Types.h"
struct OBJECTTYPE;
struct SOLDIERTYPE;


void EnterAutoResolveMode(const SGPSector& ubSector);

// is the autoresolve active?
BOOLEAN IsAutoResolveActive( void );

void EliminateAllEnemies(const SGPSector& ubSector);

void ConvertTacticalBattleIntoStrategicAutoResolveBattle(void);

UINT8 GetAutoResolveSectorID(void);

extern BOOLEAN gfTransferTacticalOppositionToAutoResolve;

//Returns TRUE if autoresolve is active or a sector is loaded.
BOOLEAN GetCurrentBattleSectorXYZ(SGPSector& psSector);

UINT32 VirtualSoldierDressWound(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pVictim, OBJECTTYPE* pKit, INT16 sKitPts, INT16 sStatus);

//Returns TRUE if a battle is happening ONLY
BOOLEAN GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(SGPSector& psSector);

ScreenID AutoResolveScreenHandle(void);

#endif
