#include "Debug.h"
#include "LoadSaveData.h"
#include "LoadSaveObjectType.h"
#include "LoadSaveRealObject.h"
#include "Overhead.h"
#include "SGPFile.h"


void ExtractRealObjectFromFile(HWFILE const file, REAL_OBJECT* const o)
{
	BYTE data[256];
	file->read(data, sizeof(data));

	DataReader d{data};
	EXTR_BOOL(d, o->fAllocated)
	EXTR_BOOL(d, o->fAlive)
	EXTR_BOOL(d, o->fApplyFriction)
	EXTR_SKIP(d, 2)
	EXTR_BOOL(d, o->fVisible)
	EXTR_BOOL(d, o->fInWater)
	EXTR_BOOL(d, o->fTestObject)
	EXTR_BOOL(d, o->fTestEndedWithCollision)
	EXTR_BOOL(d, o->fTestPositionNotSet)
	EXTR_SKIP(d, 2)
	EXTR_FLOAT(d, o->TestZTarget)
	EXTR_SKIP(d, 4) // previously o->OneOverMass
	EXTR_FLOAT(d, o->AppliedMu)
	EXTR_VEC3(d, o->Position)
	EXTR_VEC3(d, o->TestTargetPosition)
	EXTR_VEC3(d, o->OldPosition)
	EXTR_VEC3(d, o->Velocity)
	EXTR_VEC3(d, o->OldVelocity)
	EXTR_VEC3(d, o->InitialForce)
	EXTR_VEC3(d, o->Force)
	EXTR_VEC3(d, o->CollisionNormal)
	EXTR_VEC3(d, o->CollisionVelocity)
	EXTR_FLOAT(d, o->CollisionElasticity)
	EXTR_I16(d, o->sGridNo)
	EXTR_SKIP(d, 6)
	EXTR_PTR(d, o->pNode)
	EXTR_PTR(d, o->pShadow)
	EXTR_I16(d, o->sConsecutiveCollisions)
	EXTR_I16(d, o->sConsecutiveZeroVelocityCollisions)
	EXTR_I32(d, o->iOldCollisionCode)
	EXTR_FLOAT(d, o->dLifeLength)
	EXTR_FLOAT(d, o->dLifeSpan)
	ExtractObject(d, &o->Obj);
	EXTR_BOOL(d, o->fFirstTimeMoved)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, o->sFirstGridNo)
	EXTR_SOLDIER(d, o->owner)
	EXTR_U8(d, o->ubActionCode)
	EXTR_SKIP(d, 2)
	EXTR_SOLDIER(d, o->target)
	EXTR_SKIP(d, 3)
	EXTR_BOOL(d, o->fDropItem)
	EXTR_SKIP(d, 3)
	EXTR_U32(d, o->uiNumTilesMoved)
	EXTR_BOOL(d, o->fCatchGood)
	EXTR_BOOL(d, o->fAttemptedCatch)
	EXTR_BOOL(d, o->fCatchAnimOn)
	EXTR_BOOL(d, o->fCatchCheckDone)
	EXTR_BOOL(d, o->fEndedWithCollisionPositionSet)
	EXTR_SKIP(d, 3)
	EXTR_VEC3(d, o->EndedWithCollisionPosition)
	EXTR_BOOL(d, o->fHaveHitGround)
	EXTR_BOOL(d, o->fPotentialForDebug)
	EXTR_I16(d, o->sLevelNodeGridNo)
	EXTR_U32(d, o->uiSoundID)
	EXTR_U8(d, o->ubLastTargetTakenDamage)
	EXTR_SKIP(d, 3)
	Assert(d.getConsumed() == lengthof(data));
}


void InjectRealObjectIntoFile(HWFILE const file, REAL_OBJECT const* const o)
{
	BYTE data[256];

	DataWriter d{data};
	INJ_BOOL(d, o->fAllocated)
	INJ_BOOL(d, o->fAlive)
	INJ_BOOL(d, o->fApplyFriction)
	INJ_SKIP(d, 2)
	INJ_BOOL(d, o->fVisible)
	INJ_BOOL(d, o->fInWater)
	INJ_BOOL(d, o->fTestObject)
	INJ_BOOL(d, o->fTestEndedWithCollision)
	INJ_BOOL(d, o->fTestPositionNotSet)
	INJ_SKIP(d, 2)
	INJ_FLOAT(d, o->TestZTarget)
	INJ_FLOAT(d, 1.0f / 60.0f) // previously oneOverMass
	INJ_FLOAT(d, o->AppliedMu)
	INJ_VEC3(d, o->Position)
	INJ_VEC3(d, o->TestTargetPosition)
	INJ_VEC3(d, o->OldPosition)
	INJ_VEC3(d, o->Velocity)
	INJ_VEC3(d, o->OldVelocity)
	INJ_VEC3(d, o->InitialForce)
	INJ_VEC3(d, o->Force)
	INJ_VEC3(d, o->CollisionNormal)
	INJ_VEC3(d, o->CollisionVelocity)
	INJ_FLOAT(d, o->CollisionElasticity)
	INJ_I16(d, o->sGridNo)
	INJ_SKIP(d, 6)
	INJ_PTR(d, o->pNode)
	INJ_PTR(d, o->pShadow)
	INJ_I16(d, o->sConsecutiveCollisions)
	INJ_I16(d, o->sConsecutiveZeroVelocityCollisions)
	INJ_I32(d, o->iOldCollisionCode)
	INJ_FLOAT(d, o->dLifeLength)
	INJ_FLOAT(d, o->dLifeSpan)
	InjectObject(d, &o->Obj);
	INJ_BOOL(d, o->fFirstTimeMoved)
	INJ_SKIP(d, 1)
	INJ_I16(d, o->sFirstGridNo)
	INJ_SOLDIER(d, o->owner)
	INJ_U8(d, o->ubActionCode)
	INJ_SKIP(d, 2)
	INJ_SOLDIER(d, o->target)
	INJ_SKIP(d, 3)
	INJ_BOOL(d, o->fDropItem)
	INJ_SKIP(d, 3)
	INJ_U32(d, o->uiNumTilesMoved)
	INJ_BOOL(d, o->fCatchGood)
	INJ_BOOL(d, o->fAttemptedCatch)
	INJ_BOOL(d, o->fCatchAnimOn)
	INJ_BOOL(d, o->fCatchCheckDone)
	INJ_BOOL(d, o->fEndedWithCollisionPositionSet)
	INJ_SKIP(d, 3)
	INJ_VEC3(d, o->EndedWithCollisionPosition)
	INJ_BOOL(d, o->fHaveHitGround)
	INJ_BOOL(d, o->fPotentialForDebug)
	INJ_I16(d, o->sLevelNodeGridNo)
	INJ_U32(d, o->uiSoundID)
	INJ_U8(d, o->ubLastTargetTakenDamage)
	INJ_SKIP(d, 3)
	Assert(d.getConsumed() == lengthof(data));

	file->write(data, sizeof(data));
}
