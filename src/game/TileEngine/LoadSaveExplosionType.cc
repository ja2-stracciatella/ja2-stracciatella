#include "sgp/Debug.h"
#include "sgp/FileMan.h"
#include "LoadSaveExplosionType.h"
#include "sgp/LoadSaveData.h"
#include "game/Tactical/Overhead.h"


void ExtractExplosionTypeFromFile(HWFILE const file, EXPLOSIONTYPE* const e)
{
	BYTE src[36];
	const BYTE* s = src;

	FileRead(file, src, sizeof(src));

	EXTR_SKIP(s, 4)
	EXTR_SOLDIER(s, e->owner)
	EXTR_U8(s, e->ubTypeID)
	EXTR_U16(s, e->usItem)
	EXTR_U16(s, e->sX)
	EXTR_U16(s, e->sY)
	EXTR_U16(s, e->sZ)
	EXTR_U16(s, e->sGridNo)
	EXTR_SKIP(s, 1)
	EXTR_I8(s, e->bLevel)
	EXTR_SKIP(s, 2)
	EXTR_BOOL(s, e->fAllocated)
	EXTR_SKIP(s, 1)
	EXTR_I16(s, e->sCurrentFrame)
	EXTR_SKIP(s, 12)
	Assert(s == endof(src));

	e->light = NULL;
}


void InjectExplosionTypeIntoFile(HWFILE const file, EXPLOSIONTYPE const* e)
{
	BYTE dst[36];
	BYTE* d = dst;

	INJ_SKIP(d, 4)
	INJ_SOLDIER(d, e->owner)
	INJ_U8(d, e->ubTypeID)
	INJ_U16(d, e->usItem)
	INJ_U16(d, e->sX)
	INJ_U16(d, e->sY)
	INJ_U16(d, e->sZ)
	INJ_U16(d, e->sGridNo)
	INJ_SKIP(d, 1)
	INJ_I8(d, e->bLevel)
	INJ_SKIP(d, 2)
	INJ_BOOL(d, e->fAllocated)
	INJ_SKIP(d, 1)
	INJ_I16(d, e->sCurrentFrame)
	INJ_SKIP(d, 12)
	Assert(d == endof(dst));

	FileWrite(file, dst, sizeof(dst));
}
