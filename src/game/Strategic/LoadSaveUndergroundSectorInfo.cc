#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveUndergroundSectorInfo.h"
#include "LoadSaveData.h"


void ExtractUndergroundSectorInfoFromFile(HWFILE const file, UNDERGROUND_SECTORINFO* const u)
{
	BYTE data[72];
	FileRead(file, data, sizeof(data));

	DataReader d{data};
	EXTR_U32(d, u->uiFlags)
	EXTR_U8(d, u->ubSectorX)
	EXTR_U8(d, u->ubSectorY)
	EXTR_U8(d, u->ubSectorZ)
	EXTR_U8(d, u->ubNumElites)
	EXTR_U8(d, u->ubNumTroops)
	EXTR_U8(d, u->ubNumAdmins)
	EXTR_U8(d, u->ubNumCreatures)
	EXTR_SKIP(d, 5)
	EXTR_U32(d, u->uiTimeCurrentSectorWasLastLoaded)
	EXTR_PTR(d, u->next)
	EXTR_U8(d, u->ubAdjacentSectors)
	EXTR_U8(d, u->ubCreatureHabitat)
	EXTR_U8(d, u->ubElitesInBattle)
	EXTR_U8(d, u->ubTroopsInBattle)
	EXTR_U8(d, u->ubAdminsInBattle)
	EXTR_U8(d, u->ubCreaturesInBattle)
	EXTR_SKIP(d, 2)
	EXTR_U32(d, u->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer)
	EXTR_SKIP(d, 36)
	Assert(d.getConsumed() == lengthof(data));
}


void InjectUndergroundSectorInfoIntoFile(HWFILE const file, UNDERGROUND_SECTORINFO const* const u)
{
	BYTE data[72];
	DataWriter d{data};
	INJ_U32(d, u->uiFlags)
	INJ_U8(d, u->ubSectorX)
	INJ_U8(d, u->ubSectorY)
	INJ_U8(d, u->ubSectorZ)
	INJ_U8(d, u->ubNumElites)
	INJ_U8(d, u->ubNumTroops)
	INJ_U8(d, u->ubNumAdmins)
	INJ_U8(d, u->ubNumCreatures)
	INJ_SKIP(d, 5)
	INJ_U32(d, u->uiTimeCurrentSectorWasLastLoaded)
	INJ_PTR(d, u->next)
	INJ_U8(d, u->ubAdjacentSectors)
	INJ_U8(d, u->ubCreatureHabitat)
	INJ_U8(d, u->ubElitesInBattle)
	INJ_U8(d, u->ubTroopsInBattle)
	INJ_U8(d, u->ubAdminsInBattle)
	INJ_U8(d, u->ubCreaturesInBattle)
	INJ_SKIP(d, 2)
	INJ_U32(d, u->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer)
	INJ_SKIP(d, 36)
	Assert(d.getConsumed() == lengthof(data));

	FileWrite(file, data, sizeof(data));
}
