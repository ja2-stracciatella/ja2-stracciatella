#ifndef _EXPLOSION_CONTROL_H
#define _EXPLOSION_CONTROL_H

#include "Weapons.h"

#define MAX_DISTANCE_EXPLOSIVE_CAN_DESTROY_STRUCTURES 2


#define EXPLOSION_FLAG_USEABSPOS		0x00000001
#define EXPLOSION_FLAG_DISPLAYONLY	0x00000002


// Explosion Data
typedef struct
{
	UINT32											uiFlags;

	UINT8												ubOwner;
	UINT8												ubTypeID;

	UINT16											usItem;

	INT16												sX;										// World X ( optional )
	INT16												sY;										// World Y ( optional )
	INT16												sZ;										// World Z ( optional )
	INT16												sGridNo;							// World GridNo
	BOOLEAN											fLocate;
	INT8												bLevel;								// World level
	UINT8												ubUnsed[1];

} EXPLOSION_PARAMS;


typedef struct
{
	EXPLOSION_PARAMS		Params;
	BOOLEAN							fAllocated;
	INT16								sCurrentFrame;
	INT32								iID;
	INT32								iLightID;
	UINT8								ubUnsed[2];
} EXPLOSIONTYPE;
CASSERT(sizeof(EXPLOSIONTYPE) == 36)


enum
{
	NO_BLAST,
	BLAST_1,
	BLAST_2,
	BLAST_3,
	STUN_BLAST,
	WATER_BLAST,
  TARGAS_EXP,
  SMOKE_EXP,
  MUSTARD_EXP,

	NUM_EXP_TYPES

} EXPLOSION_TYPES;

typedef struct
{
	UINT32	uiWorldBombIndex;
	UINT32	uiTimeStamp;
	UINT8		fExists;
} ExplosionQueueElement;
CASSERT(sizeof(ExplosionQueueElement) == 12)


#define	ERASE_SPREAD_EFFECT					2
#define BLOOD_SPREAD_EFFECT					3
#define REDO_SPREAD_EFFECT					4


#define	NUM_EXPLOSION_SLOTS					100
extern	EXPLOSIONTYPE								gExplosionData[ NUM_EXPLOSION_SLOTS ];

extern UINT8 gubElementsOnExplosionQueue;
extern BOOLEAN gfExplosionQueueActive;

void IgniteExplosion( UINT8 ubOwner, INT16 sX, INT16 sY, INT16 sZ, INT16 sGridNo, UINT16 usItem, INT8 bLevel );
void InternalIgniteExplosion( UINT8 ubOwner, INT16 sX, INT16 sY, INT16 sZ, INT16 sGridNo, UINT16 usItem, BOOLEAN fLocate, INT8 bLevel );


void GenerateExplosion( EXPLOSION_PARAMS *pExpParams );

void SpreadEffect( INT16 sGridNo, UINT8 ubRadius, UINT16 usItem, UINT8 ubOwner, BOOLEAN fSubsequent, INT8 bLevel, INT32 iSmokeEffectNum );

void DecayBombTimers( void );
void SetOffBombsByFrequency( UINT8 ubID, INT8 bFrequency );
BOOLEAN SetOffBombsInGridNo( UINT8 ubID, INT16 sGridNo, BOOLEAN fAllBombs, INT8 bLevel );
void ActivateSwitchInGridNo( UINT8 ubID, INT16 sGridNo );
void SetOffPanicBombs(SOLDIERTYPE* s, INT8 bPanicTrigger);

void UpdateExplosionFrame( INT32 iIndex, INT16 sCurrentFrame );
void RemoveExplosionData( INT32 iIndex );

void UpdateAndDamageSAMIfFound( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ, INT16 sGridNo, UINT8 ubDamage );
void UpdateSAMDoneRepair( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ  );


BOOLEAN SaveExplosionTableToSaveGameFile( HWFILE hFile );

BOOLEAN LoadExplosionTableFromSavedGameFile( HWFILE hFile );

BOOLEAN ActiveTimedBombExists( void );
void RemoveAllActiveTimedBombs( void );

#define GASMASK_MIN_STATUS 70

BOOLEAN DishOutGasDamage( SOLDIERTYPE * pSoldier, EXPLOSIVETYPE * pExplosive, INT16 sSubsequent, BOOLEAN fRecompileMovementCosts, INT16 sWoundAmt, INT16 sBreathAmt, UINT8 ubOwner );

void HandleExplosionQueue(void);

#endif
