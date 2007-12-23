#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveSectorInfo.h"
#include "LoadSaveData.h"


BOOLEAN ExtractSectorInfoFromFile(const HWFILE file, SECTORINFO* const s)
{
	BYTE data[116];
	if (!FileRead(file, data, sizeof(data))) return FALSE;

	const BYTE* d = data;
	EXTR_U32(d, s->uiFlags)
	EXTR_U8(d, s->ubInvestigativeState)
	EXTR_U8(d, s->ubGarrisonID)
	EXTR_I8(d, s->ubPendingReinforcements)
	EXTR_BOOL(d, s->fMilitiaTrainingPaid)
	EXTR_U8(d, s->ubMilitiaTrainingPercentDone)
	EXTR_U8(d, s->ubMilitiaTrainingHundredths)
	EXTR_BOOLA(d, s->fPlayer, lengthof(s->fPlayer))
	EXTR_U8(d, s->ubNumTroops)
	EXTR_U8(d, s->ubNumElites)
	EXTR_U8(d, s->ubNumAdmins)
	EXTR_U8(d, s->ubNumCreatures)
	EXTR_U8(d, s->ubTroopsInBattle)
	EXTR_U8(d, s->ubElitesInBattle)
	EXTR_U8(d, s->ubAdminsInBattle)
	EXTR_U8(d, s->ubCreaturesInBattle)
	EXTR_I8(d, s->bLastKnownEnemies_UNUSED)
	EXTR_SKIP(d, 1)
	EXTR_U32(d, s->ubDayOfLastCreatureAttack)
	EXTR_U32(d, s->uiFacilitiesFlags)
	EXTR_U8A(d, s->ubTraversability, lengthof(s->ubTraversability))
	EXTR_I8(d, s->bNameId)
	EXTR_I8(d, s->bUSUSED)
	EXTR_I8(d, s->bBloodCats)
	EXTR_I8(d, s->bBloodCatPlacements)
	EXTR_I8(d, s->UNUSEDbSAMCondition)
	EXTR_U8(d, s->ubTravelRating)
	EXTR_U8A(d, s->ubNumberOfCivsAtLevel, lengthof(s->ubNumberOfCivsAtLevel))
	EXTR_U16(d, s->usUNUSEDMilitiaLevels)
	EXTR_U8(d, s->ubUNUSEDNumberOfJoeBlowCivilians)
	EXTR_SKIP(d, 3)
	EXTR_U32(d, s->uiTimeCurrentSectorWasLastLoaded)
	EXTR_U8(d, s->ubUNUSEDNumberOfEnemiesThoughtToBeHere)
	EXTR_SKIP(d, 3)
	EXTR_U32(d, s->uiTimeLastPlayerLiberated)
	EXTR_BOOL(d, s->fSurfaceWasEverPlayerControlled)
	EXTR_U8(d, s->bFiller1)
	EXTR_U8(d, s->bFiller2)
	EXTR_U8(d, s->bFiller3)
	EXTR_U32(d, s->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer)
	EXTR_I8A(d, s->bPadding, lengthof(s->bPadding))
	EXTR_SKIP(d, 3)
	Assert(d == endof(data));

	return TRUE;
}


BOOLEAN InjectSectorInfoIntoFile(const HWFILE file, const SECTORINFO* const s)
{
	BYTE data[116];

	BYTE* d = data;
	INJ_U32(d, s->uiFlags)
	INJ_U8(d, s->ubInvestigativeState)
	INJ_U8(d, s->ubGarrisonID)
	INJ_I8(d, s->ubPendingReinforcements)
	INJ_BOOL(d, s->fMilitiaTrainingPaid)
	INJ_U8(d, s->ubMilitiaTrainingPercentDone)
	INJ_U8(d, s->ubMilitiaTrainingHundredths)
	INJ_BOOLA(d, s->fPlayer, lengthof(s->fPlayer))
	INJ_U8(d, s->ubNumTroops)
	INJ_U8(d, s->ubNumElites)
	INJ_U8(d, s->ubNumAdmins)
	INJ_U8(d, s->ubNumCreatures)
	INJ_U8(d, s->ubTroopsInBattle)
	INJ_U8(d, s->ubElitesInBattle)
	INJ_U8(d, s->ubAdminsInBattle)
	INJ_U8(d, s->ubCreaturesInBattle)
	INJ_I8(d, s->bLastKnownEnemies_UNUSED)
	INJ_SKIP(d, 1)
	INJ_U32(d, s->ubDayOfLastCreatureAttack)
	INJ_U32(d, s->uiFacilitiesFlags)
	INJ_U8A(d, s->ubTraversability, lengthof(s->ubTraversability))
	INJ_I8(d, s->bNameId)
	INJ_I8(d, s->bUSUSED)
	INJ_I8(d, s->bBloodCats)
	INJ_I8(d, s->bBloodCatPlacements)
	INJ_I8(d, s->UNUSEDbSAMCondition)
	INJ_U8(d, s->ubTravelRating)
	INJ_U8A(d, s->ubNumberOfCivsAtLevel, lengthof(s->ubNumberOfCivsAtLevel))
	INJ_U16(d, s->usUNUSEDMilitiaLevels)
	INJ_U8(d, s->ubUNUSEDNumberOfJoeBlowCivilians)
	INJ_SKIP(d, 3)
	INJ_U32(d, s->uiTimeCurrentSectorWasLastLoaded)
	INJ_U8(d, s->ubUNUSEDNumberOfEnemiesThoughtToBeHere)
	INJ_SKIP(d, 3)
	INJ_U32(d, s->uiTimeLastPlayerLiberated)
	INJ_BOOL(d, s->fSurfaceWasEverPlayerControlled)
	INJ_U8(d, s->bFiller1)
	INJ_U8(d, s->bFiller2)
	INJ_U8(d, s->bFiller3)
	INJ_U32(d, s->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer)
	INJ_I8A(d, s->bPadding, lengthof(s->bPadding))
	INJ_SKIP(d, 3)
	Assert(d == endof(data));

	return FileWrite(file, data, sizeof(data));
}
