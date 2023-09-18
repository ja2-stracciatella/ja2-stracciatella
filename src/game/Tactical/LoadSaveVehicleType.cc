#include "Assignments.h"
#include "Debug.h"
#include "Isometric_Utils.h"
#include "LoadSaveVehicleType.h"
#include "LoadSaveData.h"
#include "SGPFile.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Vehicles.h"


void ExtractVehicleTypeFromFile(HWFILE const file, VEHICLETYPE* const v, UINT32 const savegame_version)
{
	BYTE data[128];
	file->read(data, sizeof(data));

	DataReader d{data};
	EXTR_PTR(d, v->pMercPath)
	EXTR_U8(d, v->ubMovementGroup)
	EXTR_U8(d, v->ubVehicleType)
	EXTR_I16(d, v->sSector.x)
	EXTR_I16(d, v->sSector.y)
	EXTR_I16(d, v->sSector.z)
	EXTR_BOOL(d, v->fBetweenSectors)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, v->sGridNo);
	const ProfileID noone = (savegame_version < 86 ? 0 : NO_PROFILE);
	// The ProfileID of the passengers gets stored, not their SoldierID
	for (auto & soldier : v->pPassengers)
	{
		ProfileID id;
		EXTR_U8(d, id)
		EXTR_SKIP(d, 3)
		if (id == noone)
		{
			soldier = nullptr;
			continue;
		}

		soldier = FindSoldierByProfileID(id);
		if (soldier == nullptr)
		{
			SLOGE("Soldier with profile id {} supposed to be in vehicle"
			      "but doesn't have a SOLDIERTYPE", id);
			continue;
		}

		if (soldier->bAssignment != Assignments::VEHICLE)
		{
			// Workaround for issue 1885: it is somehow possible for a merc
			// to be aboard the helicopter while having a different assignment.
			SLOGE("{} in vehicle but has wrong assignment {}",
				soldier->name, soldier->bAssignment);
			// We cannot know whether the assignment or the passenger's list
			// is incorrect, so we have to make a guess. If this guy is in
			// the same sector as the vehicle but nowhere on the map we fix
			// his assignment, otherwise we remove him from the vehicle.
			if (soldier->sSector == v->sSector && soldier->sGridNo == NOWHERE)
			{
				soldier->bAssignment = Assignments::VEHICLE;
				soldier->bVehicleID = static_cast<INT8>(VEHICLE2ID(*v));
			}
			else
			{
				soldier = nullptr;
			}
		}
	}
	EXTR_SKIP(d, 61)
	EXTR_BOOL(d, v->fDestroyed)
	EXTR_SKIP(d, 2)
	EXTR_U32(d, v->uiMovementSoundID)
	EXTR_SKIP(d, 1)
	EXTR_BOOL(d, v->fValid)
	EXTR_SKIP(d, 2)
	Assert(d.getConsumed() == lengthof(data));
}


void InjectVehicleTypeIntoFile(HWFILE const file, VEHICLETYPE const* const v)
{
	BYTE data[128];

	DataWriter d{data};
	INJ_PTR(d, v->pMercPath)
	INJ_U8(d, v->ubMovementGroup)
	INJ_U8(d, v->ubVehicleType)
	INJ_I16(d, v->sSector.x)
	INJ_I16(d, v->sSector.y)
	INJ_I16(d, v->sSector.z)
	INJ_BOOL(d, v->fBetweenSectors)
	INJ_SKIP(d, 1)
	INJ_I16(d, v->sGridNo);
	// The ProfileID of the passengers gets stored, not their SoldierID
	FOR_EACH(SOLDIERTYPE* const, i, v->pPassengers)
	{
		const ProfileID id = (*i == NULL ? NO_PROFILE : (*i)->ubProfile);
		INJ_U8(d, id)
		INJ_SKIP(d, 3)
	}
	INJ_SKIP(d, 61)
	INJ_BOOL(d, v->fDestroyed)
	INJ_SKIP(d, 2)
	INJ_U32(d, v->uiMovementSoundID)
	INJ_SKIP(d, 1)
	INJ_BOOL(d, v->fValid)
	INJ_SKIP(d, 2)
	Assert(d.getConsumed() == lengthof(data));

	file->write(data, sizeof(data));
}
