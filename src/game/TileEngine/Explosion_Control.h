#ifndef _EXPLOSION_CONTROL_H
#define _EXPLOSION_CONTROL_H

#include "JA2Types.h"
#include "ExplosiveModel.h"
#include "SmokeEffectModel.h"
#include "Weapons.h"
#include "Observable.h"

#define MAX_DISTANCE_EXPLOSIVE_CAN_DESTROY_STRUCTURES 2


struct EXPLOSIONTYPE
{
	SOLDIERTYPE* owner;
	UINT8        ubTypeID;

	UINT16       usItem;

	INT16        sX;      // World X ( optional )
	INT16        sY;      // World Y ( optional )
	INT16        sZ;      // World Z ( optional )
	INT16        sGridNo; // World GridNo
	INT8         bLevel;  // World level

	BOOLEAN							fAllocated;
	INT16								sCurrentFrame;
	LIGHT_SPRITE* light;
};

struct ExplosionQueueElement
{
	UINT32	uiWorldBombIndex;
	UINT32	uiTimeStamp;
	UINT8		fExists;
};


#define ERASE_SPREAD_EFFECT					2
#define BLOOD_SPREAD_EFFECT					3
#define REDO_SPREAD_EFFECT					4


extern UINT8 gubElementsOnExplosionQueue;
extern BOOLEAN gfExplosionQueueActive;


extern Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT32, BOOLEAN_S*> BeforeStructureDamaged;
extern Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT8, BOOLEAN> OnStructureDamaged;

void IgniteExplosion(SOLDIERTYPE* owner, INT16 z, INT16 sGridNo, UINT16 item, INT8 level);
void IgniteExplosionXY(SOLDIERTYPE* owner, INT16 sX, INT16 sY, INT16 sZ, INT16 sGridNo, UINT16 usItem, INT8 bLevel);
void InternalIgniteExplosion(SOLDIERTYPE* owner, INT16 sX, INT16 sY, INT16 sZ, INT16 sGridNo, UINT16 usItem, BOOLEAN fLocate, INT8 bLevel);


void SpreadEffect(INT16 sGridNo, UINT8 ubRadius, UINT16 usItem, SOLDIERTYPE* owner, BOOLEAN fSubsequent, INT8 bLevel, const SMOKEEFFECT* s);
void SpreadEffectSmoke(const SMOKEEFFECT* s, BOOLEAN subsequent, INT8 level);

void DecayBombTimers( void );
void SetOffBombsByFrequency(SOLDIERTYPE* s, INT8 bFrequency);
BOOLEAN SetOffBombsInGridNo(SOLDIERTYPE* s, INT16 sGridNo, BOOLEAN fAllBombs, INT8 bLevel);
void ActivateSwitchInGridNo(SOLDIERTYPE* s, INT16 sGridNo);
void SetOffPanicBombs(SOLDIERTYPE* s, INT8 bPanicTrigger);

void UpdateExplosionFrame(EXPLOSIONTYPE* e, INT16 sCurrentFrame);
void RemoveExplosionData(EXPLOSIONTYPE* e);

void SaveExplosionTableToSaveGameFile(HWFILE);
void LoadExplosionTableFromSavedGameFile(HWFILE);

BOOLEAN ActiveTimedBombExists( void );
void RemoveAllActiveTimedBombs( void );

#define GASMASK_MIN_STATUS 70

BOOLEAN DishOutGasDamage(SOLDIERTYPE* pSoldier, const SmokeEffectModel* smokeEffect, INT16 sSubsequent, BOOLEAN fRecompileMovementCosts, SOLDIERTYPE* owner);

void HandleExplosionQueue();

#endif
