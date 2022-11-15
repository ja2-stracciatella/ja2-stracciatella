#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveSmokeEffect.h"
#include "Overhead.h"
#include "SmokeEffects.h"


void ExtractSmokeEffectFromFile(HWFILE const file, SMOKEEFFECT* const s)
{
	BYTE data[16];
	file->read(data, sizeof(data));

	DataReader d{data};
	EXTR_I16(d, s->sGridNo)
	EXTR_U8(d, s->ubDuration)
	EXTR_U8(d, s->ubRadius)
	EXTR_U8(d, s->bFlags)
	EXTR_I8(d, s->bAge)
	EXTR_BOOL(d, s->fAllocated)
	EXTR_I8(d, s->bType)
	s->usItem = d.read<ItemId>();
	EXTR_SOLDIER(d, s->owner)
	EXTR_SKIP(d, 1)
	EXTR_U32(d, s->uiTimeOfLastUpdate)
	Assert(d.getConsumed() == lengthof(data));
}


void InjectSmokeEffectIntoFile(HWFILE const file, SMOKEEFFECT const* const s)
{
	BYTE data[16];

	DataWriter d{data};
	INJ_I16(d, s->sGridNo)
	INJ_U8(d, s->ubDuration)
	INJ_U8(d, s->ubRadius)
	INJ_U8(d, s->bFlags)
	INJ_I8(d, s->bAge)
	INJ_BOOL(d, s->fAllocated)
	INJ_I8(d, s->bType)
	d.write<ItemId>(s->usItem);
	INJ_SOLDIER(d, s->owner)
	INJ_SKIP(d, 1)
	INJ_U32(d, s->uiTimeOfLastUpdate)
	Assert(d.getConsumed() == lengthof(data));

	file->write(data, sizeof(data));
}
