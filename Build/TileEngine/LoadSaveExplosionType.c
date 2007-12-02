#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveExplosionType.h"
#include "LoadSaveData.h"
#include "Overhead.h"


BOOLEAN ExtractExplosionTypeFromFile(const HWFILE file, EXPLOSIONTYPE* const e)
{
	BYTE src[36];
	const BYTE* s = src;

	if (FileRead(file, src, sizeof(src))) return FALSE;

	EXTR_SKIP(s, 4)
	EXTR_SOLDIER(s, e->Params.owner)
	EXTR_U8(s, e->Params.ubTypeID)
	EXTR_U16(s, e->Params.usItem)
	EXTR_U16(s, e->Params.sX)
	EXTR_U16(s, e->Params.sY)
	EXTR_U16(s, e->Params.sZ)
	EXTR_U16(s, e->Params.sGridNo)
	EXTR_SKIP(s, 1)
	EXTR_I8(s, e->Params.bLevel)
	EXTR_SKIP(s, 2)
	EXTR_BOOL(s, e->fAllocated)
	EXTR_SKIP(s, 1)
	EXTR_I16(s, e->sCurrentFrame)
	EXTR_SKIP(s, 12)
	Assert(s == endof(src));

	e->light = NULL;

	return TRUE;
}


BOOLEAN InjectExplosionTypeIntoFile(const HWFILE file, const EXPLOSIONTYPE* e)
{
	BYTE dst[36];
	BYTE* d = dst;

	INJ_SKIP(d, 4)
	INJ_SOLDIER(d, e->Params.owner)
	INJ_U8(d, e->Params.ubTypeID)
	INJ_U16(d, e->Params.usItem)
	INJ_U16(d, e->Params.sX)
	INJ_U16(d, e->Params.sY)
	INJ_U16(d, e->Params.sZ)
	INJ_U16(d, e->Params.sGridNo)
	INJ_SKIP(d, 1)
	INJ_I8(d, e->Params.bLevel)
	INJ_SKIP(d, 2)
	INJ_BOOL(d, e->fAllocated)
	INJ_SKIP(d, 1)
	INJ_I16(d, e->sCurrentFrame)
	INJ_SKIP(d, 12)
	Assert(d == endof(dst));

	return FileWrite(file, dst, sizeof(dst));
}

