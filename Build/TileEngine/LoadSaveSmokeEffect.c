#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveSmokeEffect.h"
#include "Overhead.h"
#include "SmokeEffects.h"


BOOLEAN ExtractSmokeEffectFromFile(const HWFILE file, SMOKEEFFECT* const s)
{
	BYTE data[16];
	if (FileRead(file, data, sizeof(data))) return FALSE;

	const BYTE* d = data;
	EXTR_I16(d, s->sGridNo)
	EXTR_U8(d, s->ubDuration)
	EXTR_U8(d, s->ubRadius)
	EXTR_U8(d, s->bFlags)
	EXTR_I8(d, s->bAge)
	EXTR_BOOL(d, s->fAllocated)
	EXTR_I8(d, s->bType)
	EXTR_U16(d, s->usItem)
	EXTR_SOLDIER(d, s->owner)
	EXTR_SKIP(d, 1)
	EXTR_U32(d, s->uiTimeOfLastUpdate)
	Assert(d == endof(data));

	return TRUE;
}


BOOLEAN InjectSmokeEffectIntoFile(const HWFILE file, const SMOKEEFFECT* s)
{
	BYTE data[16];

	BYTE* d = data;
	INJ_I16(d, s->sGridNo)
	INJ_U8(d, s->ubDuration)
	INJ_U8(d, s->ubRadius)
	INJ_U8(d, s->bFlags)
	INJ_I8(d, s->bAge)
	INJ_BOOL(d, s->fAllocated)
	INJ_I8(d, s->bType)
	INJ_U16(d, s->usItem)
	INJ_SOLDIER(d, s->owner)
	INJ_SKIP(d, 1)
	INJ_U32(d, s->uiTimeOfLastUpdate)
	Assert(d == endof(data));

	return FileWrite(file, data, sizeof(data));
}

