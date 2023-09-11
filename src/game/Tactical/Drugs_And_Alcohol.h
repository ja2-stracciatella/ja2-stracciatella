#ifndef __DRUGS_AND_ALCOHOL_H
#define __DRUGS_AND_ALCOHOL_H

#include "JA2Types.h"


#define DRUG_TYPE_ADRENALINE		0
#define DRUG_TYPE_ALCOHOL		1
#define NO_DRUG			2
#define NUM_COMPLEX_DRUGS		2
#define DRUG_TYPE_REGENERATION		3

#define SOBER				0
#define FEELING_GOOD			1
#define BORDERLINE			2
#define DRUNK				3
#define HUNGOVER			4

#define REGEN_POINTS_PER_BOOSTER	4
#define LIFE_GAIN_PER_REGEN_POINT	10

BOOLEAN ApplyDrugs( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObject );

void HandleEndTurnDrugAdjustments( SOLDIERTYPE *pSoldier );
void HandleAPEffectDueToDrugs(const SOLDIERTYPE* pSoldier, UINT8* pubPoints);
void HandleBPEffectDueToDrugs( SOLDIERTYPE *pSoldier, INT16 *psPoints );


INT8 GetDrugEffect( SOLDIERTYPE *pSoldier, UINT8 ubDrugType );
INT8 GetDrunkLevel(const SOLDIERTYPE* pSoldier);
INT32 EffectStatForBeingDrunk(const SOLDIERTYPE* pSoldier, INT32 iStat);
BOOLEAN MercUnderTheInfluence(const SOLDIERTYPE* pSoldier);


#endif
