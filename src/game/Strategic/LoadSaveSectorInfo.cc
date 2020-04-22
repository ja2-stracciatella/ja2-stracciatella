#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveSectorInfo.h"
#include "LoadSaveData.h"


void ExtractSectorInfoFromFile(HWFILE const f, SECTORINFO& s)
{
	BYTE data[116];
	FileRead(f, data, sizeof(data));

	DataReader d{data};
	EXTR_U32( d, s.uiFlags)
	EXTR_SKIP(d, 1)
	EXTR_U8(  d, s.ubGarrisonID)
	EXTR_I8(  d, s.ubPendingReinforcements)
	EXTR_BOOL(d, s.fMilitiaTrainingPaid)
	EXTR_U8(  d, s.ubMilitiaTrainingPercentDone)
	EXTR_U8(  d, s.ubMilitiaTrainingHundredths)
	EXTR_SKIP(d, 4)
	EXTR_U8(  d, s.ubNumTroops)
	EXTR_U8(  d, s.ubNumElites)
	EXTR_U8(  d, s.ubNumAdmins)
	EXTR_U8(  d, s.ubNumCreatures)
	EXTR_U8(  d, s.ubTroopsInBattle)
	EXTR_U8(  d, s.ubElitesInBattle)
	EXTR_U8(  d, s.ubAdminsInBattle)
	EXTR_U8(  d, s.ubCreaturesInBattle)
	EXTR_SKIP(d, 2)
	EXTR_U32( d, s.ubDayOfLastCreatureAttack)
	EXTR_U32( d, s.uiFacilitiesFlags)
	EXTR_U8A( d, s.ubTraversability, lengthof(s.ubTraversability))
	EXTR_SKIP(d, 2)
	EXTR_I8(  d, s.bBloodCats)
	EXTR_I8(  d, s.bBloodCatPlacements)
	EXTR_SKIP(d, 1)
	EXTR_U8(  d, s.ubTravelRating)
	EXTR_U8A( d, s.ubNumberOfCivsAtLevel, lengthof(s.ubNumberOfCivsAtLevel))
	EXTR_SKIP(d, 6)
	EXTR_U32( d, s.uiTimeCurrentSectorWasLastLoaded)
	EXTR_SKIP(d, 4)
	EXTR_U32( d, s.uiTimeLastPlayerLiberated)
	EXTR_BOOL(d, s.fSurfaceWasEverPlayerControlled)
	EXTR_SKIP(d, 3)
	EXTR_U32( d, s.uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer)
	EXTR_SKIP(d, 44)
	Assert(d.getConsumed() == lengthof(data));
}


void InjectSectorInfoIntoFile(HWFILE const f, SECTORINFO const& s)
{
	BYTE  data[116];
	DataWriter d{data};
	INJ_U32( d, s.uiFlags)
	INJ_SKIP(d, 1)
	INJ_U8(  d, s.ubGarrisonID)
	INJ_I8(  d, s.ubPendingReinforcements)
	INJ_BOOL(d, s.fMilitiaTrainingPaid)
	INJ_U8(  d, s.ubMilitiaTrainingPercentDone)
	INJ_U8(  d, s.ubMilitiaTrainingHundredths)
	INJ_SKIP(d, 4)
	INJ_U8(  d, s.ubNumTroops)
	INJ_U8(  d, s.ubNumElites)
	INJ_U8(  d, s.ubNumAdmins)
	INJ_U8(  d, s.ubNumCreatures)
	INJ_U8(  d, s.ubTroopsInBattle)
	INJ_U8(  d, s.ubElitesInBattle)
	INJ_U8(  d, s.ubAdminsInBattle)
	INJ_U8(  d, s.ubCreaturesInBattle)
	INJ_SKIP(d, 2)
	INJ_U32( d, s.ubDayOfLastCreatureAttack)
	INJ_U32( d, s.uiFacilitiesFlags)
	INJ_U8A( d, s.ubTraversability, lengthof(s.ubTraversability))
	INJ_SKIP(d, 2)
	INJ_I8(  d, s.bBloodCats)
	INJ_I8(  d, s.bBloodCatPlacements)
	INJ_SKIP(d, 1)
	INJ_U8(  d, s.ubTravelRating)
	INJ_U8A( d, s.ubNumberOfCivsAtLevel, lengthof(s.ubNumberOfCivsAtLevel))
	INJ_SKIP(d, 6)
	INJ_U32( d, s.uiTimeCurrentSectorWasLastLoaded)
	INJ_SKIP(d, 4)
	INJ_U32( d, s.uiTimeLastPlayerLiberated)
	INJ_BOOL(d, s.fSurfaceWasEverPlayerControlled)
	INJ_SKIP(d, 3)
	INJ_U32( d, s.uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer)
	INJ_SKIP(d, 44)
	Assert(d.getConsumed() == lengthof(data));

	FileWrite(f, data, sizeof(data));
}
