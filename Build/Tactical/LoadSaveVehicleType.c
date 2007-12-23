#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveVehicleType.h"
#include "LoadSaveData.h"
#include "Soldier_Profile.h"


BOOLEAN ExtractVehicleTypeFromFile(const HWFILE file, VEHICLETYPE* const v, const UINT32 savegame_version)
{
	BYTE data[128];
	if (!FileRead(file, data, sizeof(data))) return FALSE;

	const BYTE* d = data;
	EXTR_PTR(d, v->pMercPath)
	EXTR_U8(d, v->ubMovementGroup)
	EXTR_U8(d, v->ubVehicleType)
	EXTR_I16(d, v->sSectorX)
	EXTR_I16(d, v->sSectorY)
	EXTR_I16(d, v->sSectorZ)
	EXTR_BOOL(d, v->fBetweenSectors)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, v->sGridNo);
	const ProfileID noone = (savegame_version < 86 ? 0 : NO_PROFILE);
	/* The ProfileID of the passengers gets stored, not their SoldierID */
	for (SOLDIERTYPE** i = v->pPassengers; i != endof(v->pPassengers); ++i)
	{
		ProfileID id;
		EXTR_U8(d, id)
		EXTR_SKIP(d, 3)
		*i = (id == noone ? NULL : FindSoldierByProfileID(id, FALSE));
	}
	EXTR_U8(d, v->ubDriver)
	EXTR_SKIP(d, 1)
	EXTR_I16A(d, v->sInternalHitLocations, lengthof(v->sInternalHitLocations))
	EXTR_I16(d, v->sArmourType)
	EXTR_I16A(d, v->sExternalArmorLocationsStatus, lengthof(v->sExternalArmorLocationsStatus))
	EXTR_I16A(d, v->sCriticalHits, lengthof(v->sCriticalHits))
	EXTR_SKIP(d, 2)
	EXTR_I32(d, v->iOnSound)
	EXTR_I32(d, v->iOffSound)
	EXTR_I32(d, v->iMoveSound)
	EXTR_I32(d, v->iOutOfSound)
	EXTR_BOOL(d, v->fFunctional)
	EXTR_BOOL(d, v->fDestroyed)
	EXTR_SKIP(d, 2)
	EXTR_I32(d, v->iMovementSoundID)
	EXTR_U8(d, v->ubProfileID)
	EXTR_BOOL(d, v->fValid)
	EXTR_SKIP(d, 2)
	Assert(d == endof(data));

	return TRUE;
}


BOOLEAN InjectVehicleTypeIntoFile(const HWFILE file, const VEHICLETYPE* const v)
{
	BYTE data[128];

	BYTE* d = data;
	INJ_PTR(d, v->pMercPath)
	INJ_U8(d, v->ubMovementGroup)
	INJ_U8(d, v->ubVehicleType)
	INJ_I16(d, v->sSectorX)
	INJ_I16(d, v->sSectorY)
	INJ_I16(d, v->sSectorZ)
	INJ_BOOL(d, v->fBetweenSectors)
	INJ_SKIP(d, 1)
	INJ_I16(d, v->sGridNo);
	/* The ProfileID of the passengers gets stored, not their SoldierID */
	for (SOLDIERTYPE*const * i = v->pPassengers; i != endof(v->pPassengers); ++i)
	{
		const ProfileID id = (*i == NULL ? NO_PROFILE : (*i)->ubProfile);
		INJ_U8(d, id)
		INJ_SKIP(d, 3)
	}
	INJ_U8(d, v->ubDriver)
	INJ_SKIP(d, 1)
	INJ_I16A(d, v->sInternalHitLocations, lengthof(v->sInternalHitLocations))
	INJ_I16(d, v->sArmourType)
	INJ_I16A(d, v->sExternalArmorLocationsStatus, lengthof(v->sExternalArmorLocationsStatus))
	INJ_I16A(d, v->sCriticalHits, lengthof(v->sCriticalHits))
	INJ_SKIP(d, 2)
	INJ_I32(d, v->iOnSound)
	INJ_I32(d, v->iOffSound)
	INJ_I32(d, v->iMoveSound)
	INJ_I32(d, v->iOutOfSound)
	INJ_BOOL(d, v->fFunctional)
	INJ_BOOL(d, v->fDestroyed)
	INJ_SKIP(d, 2)
	INJ_I32(d, v->iMovementSoundID)
	INJ_U8(d, v->ubProfileID)
	INJ_BOOL(d, v->fValid)
	INJ_SKIP(d, 2)
	Assert(d == endof(data));

	return FileWrite(file, data, sizeof(data));
}
