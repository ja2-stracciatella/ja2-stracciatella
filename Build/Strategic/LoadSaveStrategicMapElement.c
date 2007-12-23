#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveStrategicMapElement.h"
#include "LoadSaveData.h"


BOOLEAN ExtractStrategicMapElementFromFile(const HWFILE file, StrategicMapElement* const e)
{
	BYTE data[41];
	if (!FileRead(file, data, sizeof(data))) return FALSE;

	const BYTE* d = data;
	EXTR_SKIP(d, 8)
	EXTR_U8A(d, e->uiBadFootSector, lengthof(e->uiBadFootSector))
	EXTR_U8A(d, e->uiBadVehicleSector, lengthof(e->uiBadVehicleSector))
	EXTR_I8(d, e->bNameId)
	EXTR_BOOL(d, e->fEnemyControlled)
	EXTR_BOOL(d, e->fEnemyAirControlled)
	EXTR_SKIP(d, 1)
	EXTR_I8(d, e->bSAMCondition)
	EXTR_SKIP(d, 20)
	Assert(d == endof(data));

	return TRUE;
}


BOOLEAN InjectStrategicMapElementIntoFile(const HWFILE file, const StrategicMapElement* const e)
{
	BYTE data[41];

	BYTE* d = data;
	INJ_SKIP(d, 8)
	INJ_U8A(d, e->uiBadFootSector, lengthof(e->uiBadFootSector))
	INJ_U8A(d, e->uiBadVehicleSector, lengthof(e->uiBadVehicleSector))
	INJ_I8(d, e->bNameId)
	INJ_BOOL(d, e->fEnemyControlled)
	INJ_BOOL(d, e->fEnemyAirControlled)
	INJ_SKIP(d, 1)
	INJ_I8(d, e->bSAMCondition)
	INJ_SKIP(d, 20)
	Assert(d == endof(data));

	return FileWrite(file, data, sizeof(data));
}
