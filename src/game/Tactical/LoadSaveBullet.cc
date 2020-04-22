#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveBullet.h"
#include "LoadSaveData.h"
#include "Overhead.h"


void ExtractBulletFromFile(HWFILE const file, BULLET* const b)
{
	BYTE src[128];
	DataReader s{src};

	FileRead(file, src, sizeof(src));

	EXTR_SKIP(s, 4)
	EXTR_SOLDIER(s, b->pFirer)
	EXTR_SOLDIER(s, b->target)
	EXTR_I8(s, b->bStartCubesAboveLevelZ)
	EXTR_I8(s, b->bEndCubesAboveLevelZ)
	EXTR_U32(s, b->sGridNo)
	EXTR_SKIP(s, 2)
	EXTR_U16(s, b->usLastStructureHit)
	EXTR_I32(s, b->qCurrX)
	EXTR_I32(s, b->qCurrY)
	EXTR_I32(s, b->qCurrZ)
	EXTR_I32(s, b->qIncrX)
	EXTR_I32(s, b->qIncrY)
	EXTR_I32(s, b->qIncrZ)
	EXTR_DOUBLE(s, b->ddHorizAngle)
	EXTR_I32(s, b->iCurrTileX)
	EXTR_I32(s, b->iCurrTileY)
	EXTR_I8(s, b->bLOSIndexX)
	EXTR_I8(s, b->bLOSIndexY)
	EXTR_BOOL(s, b->fCheckForRoof)
	EXTR_SKIP(s, 1)
	EXTR_I32(s, b->iCurrCubesZ)
	EXTR_I32(s, b->iLoop)
	EXTR_BOOL(s, b->fAllocated)
	EXTR_BOOL(s, b->fToDelete)
	EXTR_BOOL(s, b->fLocated)
	EXTR_BOOL(s, b->fReal)
	EXTR_BOOL(s, b->fAimed)
	EXTR_SKIP(s, 3)
	EXTR_U32(s, b->uiLastUpdate)
	EXTR_U8(s, b->ubTilesPerUpdate)
	EXTR_SKIP(s, 1)
	EXTR_U16(s, b->usClockTicksPerUpdate)
	EXTR_SKIP(s, 4)
	EXTR_U32(s, b->sTargetGridNo)
	EXTR_I16(s, b->sHitBy)
	EXTR_SKIP(s, 2)
	EXTR_I32(s, b->iImpact)
	EXTR_I32(s, b->iImpactReduction)
	EXTR_I32(s, b->iRange)
	EXTR_I32(s, b->iDistanceLimit)
	EXTR_U16(s, b->usFlags)
	EXTR_SKIP(s, 2)
	EXTR_PTR(s, b->pAniTile)
	EXTR_PTR(s, b->pShadowAniTile)
	EXTR_U8(s, b->ubItemStatus)
	EXTR_SKIP(s, 3)
	Assert(s.getConsumed() == lengthof(src));
}


void InjectBulletIntoFile(HWFILE const file, BULLET const* b)
{
	BYTE dst[128];
	DataWriter d{dst};

	INJ_SKIP(d, 4)
	INJ_SOLDIER(d, b->pFirer)
	INJ_SOLDIER(d, b->target)
	INJ_I8(d, b->bStartCubesAboveLevelZ)
	INJ_I8(d, b->bEndCubesAboveLevelZ)
	INJ_U32(d, b->sGridNo)
	INJ_SKIP(d, 2)
	INJ_U16(d, b->usLastStructureHit)
	INJ_I32(d, b->qCurrX)
	INJ_I32(d, b->qCurrY)
	INJ_I32(d, b->qCurrZ)
	INJ_I32(d, b->qIncrX)
	INJ_I32(d, b->qIncrY)
	INJ_I32(d, b->qIncrZ)
	INJ_DOUBLE(d, b->ddHorizAngle)
	INJ_I32(d, b->iCurrTileX)
	INJ_I32(d, b->iCurrTileY)
	INJ_I8(d, b->bLOSIndexX)
	INJ_I8(d, b->bLOSIndexY)
	INJ_BOOL(d, b->fCheckForRoof)
	INJ_SKIP(d, 1)
	INJ_I32(d, b->iCurrCubesZ)
	INJ_I32(d, b->iLoop)
	INJ_BOOL(d, b->fAllocated)
	INJ_BOOL(d, b->fToDelete)
	INJ_BOOL(d, b->fLocated)
	INJ_BOOL(d, b->fReal)
	INJ_BOOL(d, b->fAimed)
	INJ_SKIP(d, 3)
	INJ_U32(d, b->uiLastUpdate)
	INJ_U8(d, b->ubTilesPerUpdate)
	INJ_SKIP(d, 1)
	INJ_U16(d, b->usClockTicksPerUpdate)
	INJ_SKIP(d, 4)
	INJ_U32(d, b->sTargetGridNo)
	INJ_I16(d, b->sHitBy)
	INJ_SKIP(d, 2)
	INJ_I32(d, b->iImpact)
	INJ_I32(d, b->iImpactReduction)
	INJ_I32(d, b->iRange)
	INJ_I32(d, b->iDistanceLimit)
	INJ_U16(d, b->usFlags)
	INJ_SKIP(d, 2)
	INJ_PTR(d, b->pAniTile)
	INJ_PTR(d, b->pShadowAniTile)
	INJ_U8(d, b->ubItemStatus)
	INJ_SKIP(d, 3)
	Assert(d.getConsumed() == lengthof(dst));

	FileWrite(file, dst, sizeof(dst));
}
