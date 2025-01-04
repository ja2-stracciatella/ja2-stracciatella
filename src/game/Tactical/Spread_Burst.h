#ifndef _SPREAD_BURST_H
#define _SPREAD_BURST_H

#include "JA2Types.h"


void ResetBurstLocations(void);
void AccumulateBurstLocation( INT16 sGridNo );
void PickBurstLocations( SOLDIERTYPE *pSoldier );
void AIPickBurstLocations( SOLDIERTYPE *pSoldier, INT8 bTargets, SOLDIERTYPE *pTargets[5] );

void RenderAccumulatedBurstLocations(void);

void DeleteSpreadBurstGraphics();

#endif
