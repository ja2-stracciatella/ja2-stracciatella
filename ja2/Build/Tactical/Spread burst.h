#ifndef _SPREAD_BURST_H
#define _SPREAD_BURST_H

#include "soldier control.h"

void ResetBurstLocations( );
void AccumulateBurstLocation( INT16 sGridNo );
void PickBurstLocations( SOLDIERTYPE *pSoldier );
void AIPickBurstLocations( SOLDIERTYPE *pSoldier, INT8 bTargets, SOLDIERTYPE *pTargets[5] );

void RenderAccumulatedBurstLocations( );


#endif
