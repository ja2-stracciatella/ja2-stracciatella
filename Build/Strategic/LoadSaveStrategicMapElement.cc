#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveStrategicMapElement.h"
#include "LoadSaveData.h"


BOOLEAN ExtractStrategicMapElementFromFile(const HWFILE file, StrategicMapElement* const e)
try
{
	BYTE data[41];
	FileRead(file, data, sizeof(data));

	const BYTE* d = data;
	EXTR_SKIP(d, 16)
	EXTR_I8(d, e->bNameId)
	EXTR_BOOL(d, e->fEnemyControlled)
	EXTR_BOOL(d, e->fEnemyAirControlled)
	EXTR_SKIP(d, 1)
	EXTR_I8(d, e->bSAMCondition)
	EXTR_SKIP(d, 20)
	Assert(d == endof(data));

	return TRUE;
}
catch (...) { return FALSE; }


BOOLEAN InjectStrategicMapElementIntoFile(const HWFILE file, const StrategicMapElement* const e)
{
	BYTE data[41];

	BYTE* d = data;
	INJ_SKIP(d, 16)
	INJ_I8(d, e->bNameId)
	INJ_BOOL(d, e->fEnemyControlled)
	INJ_BOOL(d, e->fEnemyAirControlled)
	INJ_SKIP(d, 1)
	INJ_I8(d, e->bSAMCondition)
	INJ_SKIP(d, 20)
	Assert(d == endof(data));

	return FileWrite(file, data, sizeof(data));
}
