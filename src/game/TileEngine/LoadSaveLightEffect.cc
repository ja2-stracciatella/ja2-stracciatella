#include "sgp/Debug.h"
#include "sgp/FileMan.h"
#include "sgp/LoadSaveData.h"
#include "LoadSaveLightEffect.h"


void ExtractLightEffectFromFile(HWFILE const file, LIGHTEFFECT* const l)
{
	BYTE data[16];
	FileRead(file, data, sizeof(data));

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

	l->light = NULL;
}


void InjectLightEffectIntoFile(HWFILE const file, const LIGHTEFFECT* const l)
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

	FileWrite(file, data, sizeof(data));
}
