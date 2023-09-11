#ifndef __SOLDIER_ANI_H
#define __SOLDIER_ANI_H

#include "JA2Types.h"

BOOLEAN AdjustToNextAnimationFrame( SOLDIERTYPE *pSoldier );

void CheckForAndHandleSoldierDeath(SOLDIERTYPE* pSoldier, BOOLEAN* pfMadeCorpse);

BOOLEAN CheckForAndHandleSoldierDyingNotFromHit( SOLDIERTYPE *pSoldier );

BOOLEAN HandleSoldierDeath( SOLDIERTYPE *pSoldier , BOOLEAN *pfMadeCorpse );

BOOLEAN OKFallDirection( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bLevel, INT8 bTestDirection, UINT16 usAnimState );

void HandleCheckForDeathCommonCode(SOLDIERTYPE* pSoldier);

void KickOutWheelchair( SOLDIERTYPE *pSoldier );

void HandlePlayerTeamMemberDeathAfterSkullAnimation(SOLDIERTYPE* pSoldier);
void HandleKilledQuote(SOLDIERTYPE* pKilledSoldier, SOLDIERTYPE* pKillerSoldier, INT16 sGridNo, INT8 bLevel);

extern const SOLDIERTYPE* gLastMercTalkedAboutKilling;

#endif
