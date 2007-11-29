#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveLightEffect.h"


BOOLEAN ExtractLightEffectFromFile(const HWFILE file, LIGHTEFFECT* const l)
{
	BYTE data[16];
	if (!FileRead(file, data, sizeof(data))) return FALSE;

	const BYTE* d = data;
	EXTR_I16(d, l->sGridNo)
	EXTR_U8(d, l->ubDuration)
	EXTR_U8(d, l->bRadius)
	EXTR_I8(d, l->bAge)
	EXTR_BOOL(d, l->fAllocated)
	EXTR_I8(d, l->bType)
	EXTR_SKIP(d, 5)
	EXTR_U32(d, l->uiTimeOfLastUpdate)
	Assert(d == endof(data));

	l->iLight = -1;

	return TRUE;
}


BOOLEAN InjectLightEffectIntoFile(const HWFILE file, const LIGHTEFFECT* const l)
{
	BYTE data[16];

	BYTE* d = data;
	INJ_I16(d, l->sGridNo)
	INJ_U8(d, l->ubDuration)
	INJ_U8(d, l->bRadius)
	INJ_I8(d, l->bAge)
	INJ_BOOL(d, l->fAllocated)
	INJ_I8(d, l->bType)
	INJ_SKIP(d, 5)
	INJ_U32(d, l->uiTimeOfLastUpdate)
	Assert(d == endof(data));

	return FileWrite(file, data, sizeof(data));
}

