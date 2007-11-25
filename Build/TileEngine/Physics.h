#ifndef __PHYSICS_H
#define	__PHYSICS_H

#include "Phys_Math.h"
#include "WorldDef.h"
#include "Soldier_Control.h"


extern UINT32 guiNumObjectSlots;

typedef struct
{
	BOOLEAN			fAllocated;
	BOOLEAN			fAlive;
	BOOLEAN			fApplyFriction;
	BOOLEAN			fVisible;
	BOOLEAN			fInWater;
	BOOLEAN			fTestObject;
	BOOLEAN			fTestEndedWithCollision;
	BOOLEAN			fTestPositionNotSet;

	real				TestZTarget;
	real				OneOverMass;
	real				AppliedMu;

	vector_3		Position;
	vector_3		TestTargetPosition;
	vector_3		OldPosition;
	vector_3		Velocity;
	vector_3		OldVelocity;
	vector_3		InitialForce;
	vector_3		Force;
	vector_3		CollisionNormal;
	vector_3		CollisionVelocity;
	real				CollisionElasticity;

	INT16				sGridNo;
	LEVELNODE		*pNode;
	LEVELNODE   *pShadow;

	INT16				sConsecutiveCollisions;
	INT16				sConsecutiveZeroVelocityCollisions;
	INT32				iOldCollisionCode;

	FLOAT				dLifeLength;
	FLOAT				dLifeSpan;
	OBJECTTYPE	Obj;
	SOLDIERTYPE* owner;
	INT16				sFirstGridNo;
	BOOLEAN			fFirstTimeMoved;
	UINT8				ubActionCode;
	UINT32			uiActionData;
	BOOLEAN			fDropItem;
	UINT32			uiNumTilesMoved;
	BOOLEAN			fCatchGood;
	BOOLEAN			fAttemptedCatch;
	BOOLEAN			fCatchAnimOn;
	BOOLEAN			fCatchCheckDone;
	BOOLEAN			fEndedWithCollisionPositionSet;
	vector_3		EndedWithCollisionPosition;
	BOOLEAN			fHaveHitGround;
  BOOLEAN     fPotentialForDebug;
  INT16       sLevelNodeGridNo;
  INT32       iSoundID;
  UINT8       ubLastTargetTakenDamage;
} REAL_OBJECT;


// OBJECT LIST STUFF
REAL_OBJECT* CreatePhysicalObject(const OBJECTTYPE* pGameObj, real dLifeLength, real xPos, real yPos, real zPos, real xForce, real yForce, real zForce, SOLDIERTYPE* owner, UINT8 ubActionCode, UINT32 uiActionData);
void RemoveAllPhysicsObjects( );


FLOAT CalculateLaunchItemAngle( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubHeight, real dForce, OBJECTTYPE *pItem, INT16 *psGridNo );

BOOLEAN CalculateLaunchItemChanceToGetThrough(const SOLDIERTYPE* pSoldier, const OBJECTTYPE* pItem, INT16 sGridNo, UINT8 ubLevel, INT16 sEndZ, INT16* psFinalGridNo, BOOLEAN fArmed, INT8* pbLevel, BOOLEAN fFromUI);

void CalculateLaunchItemParamsForThrow( SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubLevel, INT16 sZPos, OBJECTTYPE *pItem, INT8 bMissBy, UINT8 ubActionCode, UINT32 uiActionData );





// SIMULATE WORLD
void SimulateWorld(  );


BOOLEAN	SavePhysicsTableToSaveGameFile( HWFILE hFile );

BOOLEAN	LoadPhysicsTableFromSavedGameFile( HWFILE hFile );


#endif
