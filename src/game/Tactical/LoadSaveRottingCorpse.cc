#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveRottingCorpse.h"
#include "LoadSaveData.h"


void ExtractRottingCorpseFromFile(HWFILE const file, ROTTING_CORPSE_DEFINITION* const c)
{
	BYTE data[160];
	FileRead(file, data, sizeof(data));

	DataReader d{data};
	EXTR_U8(d, c->ubType)
	EXTR_U8(d, c->ubBodyType)
	EXTR_I16(d, c->sGridNo)
	EXTR_SKIP(d, 8)
	EXTR_I16(d, c->sHeightAdjustment)
	EXTR_STR(d, c->HeadPal, lengthof(c->HeadPal))
	EXTR_STR(d, c->PantsPal, lengthof(c->PantsPal))
	EXTR_STR(d, c->VestPal, lengthof(c->VestPal))
	EXTR_STR(d, c->SkinPal, lengthof(c->SkinPal))
	EXTR_I8(d, c->bDirection)
	EXTR_SKIP(d, 1)
	EXTR_U32(d, c->uiTimeOfDeath)
	EXTR_U16(d, c->usFlags)
	EXTR_I8(d, c->bLevel)
	EXTR_I8(d, c->bVisible)
	EXTR_I8(d, c->bNumServicingCrows)
	EXTR_U8(d, c->ubProfile)
	EXTR_BOOL(d, c->fHeadTaken)
	EXTR_U8(d, c->ubAIWarningValue)
	EXTR_SKIP(d, 12)
	Assert(d.getConsumed() == lengthof(data));
}


void InjectRottingCorpseIntoFile(HWFILE const file, ROTTING_CORPSE_DEFINITION const* const c)
{
	BYTE data[160];

	DataWriter d{data};
	INJ_U8(d, c->ubType)
	INJ_U8(d, c->ubBodyType)
	INJ_I16(d, c->sGridNo)
	INJ_SKIP(d, 8)
	INJ_I16(d, c->sHeightAdjustment)
	INJ_STR(d, c->HeadPal, lengthof(c->HeadPal))
	INJ_STR(d, c->PantsPal, lengthof(c->PantsPal))
	INJ_STR(d, c->VestPal, lengthof(c->VestPal))
	INJ_STR(d, c->SkinPal, lengthof(c->SkinPal))
	INJ_I8(d, c->bDirection)
	INJ_SKIP(d, 1)
	INJ_U32(d, c->uiTimeOfDeath)
	INJ_U16(d, c->usFlags)
	INJ_I8(d, c->bLevel)
	INJ_I8(d, c->bVisible)
	INJ_I8(d, c->bNumServicingCrows)
	INJ_U8(d, c->ubProfile)
	INJ_BOOL(d, c->fHeadTaken)
	INJ_U8(d, c->ubAIWarningValue)
	INJ_SKIP(d, 12)
	Assert(d.getConsumed() == lengthof(data));

	FileWrite(file, data, sizeof(data));
}
