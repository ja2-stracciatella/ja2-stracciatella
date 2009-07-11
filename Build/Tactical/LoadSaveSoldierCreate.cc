#include "Debug.h"
#include "LoadSaveSoldierCreate.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveObjectType.h"


void ExtractSoldierCreateFromFileUTF16(HWFILE const f, SOLDIERCREATE_STRUCT* const c)
{
	BYTE data[1040];
	FileRead(f, data, sizeof(data));

	BYTE const* d = data;
	EXTR_BOOL(d, c->fStatic)
	EXTR_U8(d, c->ubProfile)
	EXTR_SKIP(d, 2)
	EXTR_BOOL(d, c->fCopyProfileItemsOver)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, c->sSectorX)
	EXTR_I16(d, c->sSectorY)
	EXTR_I8(d, c->bDirection)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, c->sInsertionGridNo)
	EXTR_I8(d, c->bTeam)
	EXTR_I8(d, c->bBodyType)
	EXTR_I8(d, c->bAttitude)
	EXTR_I8(d, c->bOrders)
	EXTR_I8(d, c->bLifeMax)
	EXTR_I8(d, c->bLife)
	EXTR_I8(d, c->bAgility)
	EXTR_I8(d, c->bDexterity)
	EXTR_I8(d, c->bExpLevel)
	EXTR_I8(d, c->bMarksmanship)
	EXTR_I8(d, c->bMedical)
	EXTR_I8(d, c->bMechanical)
	EXTR_I8(d, c->bExplosive)
	EXTR_I8(d, c->bLeadership)
	EXTR_I8(d, c->bStrength)
	EXTR_I8(d, c->bWisdom)
	EXTR_I8(d, c->bMorale)
	EXTR_I8(d, c->bAIMorale)
	for (size_t i = 0; i < lengthof(c->Inv); i++)
	{
		d = ExtractObject(d, &c->Inv[i]);
	}
	EXTR_STR(d, c->HeadPal, lengthof(c->HeadPal))
	EXTR_STR(d, c->PantsPal, lengthof(c->PantsPal))
	EXTR_STR(d, c->VestPal, lengthof(c->VestPal))
	EXTR_STR(d, c->SkinPal, lengthof(c->SkinPal))
	EXTR_SKIP(d, 30)
	EXTR_I16A(d, c->sPatrolGrid, lengthof(c->sPatrolGrid))
	EXTR_I8(d, c->bPatrolCnt)
	EXTR_BOOL(d, c->fVisible)
	EXTR_WSTR16(d, c->name, lengthof(c->name))
	EXTR_U8(d, c->ubSoldierClass)
	EXTR_BOOL(d, c->fOnRoof)
	EXTR_I8(d, c->bSectorZ)
	EXTR_SKIP(d, 6)
	EXTR_U8(d, c->ubCivilianGroup)
	EXTR_SKIP(d, 1)
	EXTR_U8(d, c->ubScheduleID)
	EXTR_BOOL(d, c->fUseGivenVehicle)
	EXTR_I8(d, c->bUseGivenVehicleID)
	EXTR_BOOL(d, c->fHasKeys)
	EXTR_SKIP(d, 117)
	Assert(d == endof(data));
}


static void ExtractSoldierCreate(const BYTE* const data, SOLDIERCREATE_STRUCT* const c)
{
	const BYTE* d = data;
	EXTR_BOOL(d, c->fStatic)
	EXTR_U8(d, c->ubProfile)
	EXTR_SKIP(d, 2)
	EXTR_BOOL(d, c->fCopyProfileItemsOver)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, c->sSectorX)
	EXTR_I16(d, c->sSectorY)
	EXTR_I8(d, c->bDirection)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, c->sInsertionGridNo)
	EXTR_I8(d, c->bTeam)
	EXTR_I8(d, c->bBodyType)
	EXTR_I8(d, c->bAttitude)
	EXTR_I8(d, c->bOrders)
	EXTR_I8(d, c->bLifeMax)
	EXTR_I8(d, c->bLife)
	EXTR_I8(d, c->bAgility)
	EXTR_I8(d, c->bDexterity)
	EXTR_I8(d, c->bExpLevel)
	EXTR_I8(d, c->bMarksmanship)
	EXTR_I8(d, c->bMedical)
	EXTR_I8(d, c->bMechanical)
	EXTR_I8(d, c->bExplosive)
	EXTR_I8(d, c->bLeadership)
	EXTR_I8(d, c->bStrength)
	EXTR_I8(d, c->bWisdom)
	EXTR_I8(d, c->bMorale)
	EXTR_I8(d, c->bAIMorale)
	for (size_t i = 0; i < lengthof(c->Inv); i++)
	{
		d = ExtractObject(d, &c->Inv[i]);
	}
	EXTR_STR(d, c->HeadPal, lengthof(c->HeadPal))
	EXTR_STR(d, c->PantsPal, lengthof(c->PantsPal))
	EXTR_STR(d, c->VestPal, lengthof(c->VestPal))
	EXTR_STR(d, c->SkinPal, lengthof(c->SkinPal))
	EXTR_SKIP(d, 30)
	EXTR_I16A(d, c->sPatrolGrid, lengthof(c->sPatrolGrid))
	EXTR_I8(d, c->bPatrolCnt)
	EXTR_BOOL(d, c->fVisible)
#ifdef _WIN32 // XXX HACK000A
	EXTR_WSTR16(d, c->name, lengthof(c->name))
#else
	EXTR_WSTR32(d, c->name, lengthof(c->name))
#endif
	EXTR_U8(d, c->ubSoldierClass)
	EXTR_BOOL(d, c->fOnRoof)
	EXTR_I8(d, c->bSectorZ)
	EXTR_SKIP(d, 6)
	EXTR_U8(d, c->ubCivilianGroup)
	EXTR_SKIP(d, 1)
	EXTR_U8(d, c->ubScheduleID)
	EXTR_BOOL(d, c->fUseGivenVehicle)
	EXTR_I8(d, c->bUseGivenVehicleID)
	EXTR_BOOL(d, c->fHasKeys)
	EXTR_SKIP(d, 117)
#ifdef _WIN32 // XXX HACK000A
	Assert(d == data + 1040);
#else
	Assert(d == data + 1060);
#endif
}


void ExtractSoldierCreateFromFile(HWFILE const f, SOLDIERCREATE_STRUCT* const c)
{
#ifdef _WIN32 // XXX HACK000A
	BYTE data[1040];
#else
	BYTE data[1060];
#endif
	FileRead(f, data, sizeof(data));
	ExtractSoldierCreate(data, c);
}


static void InjectSoldierCreateUTF16(BYTE* const data, const SOLDIERCREATE_STRUCT* const c)
{
	BYTE* d = data;
	INJ_BOOL(d, c->fStatic)
	INJ_U8(d, c->ubProfile)
	INJ_SKIP(d, 2)
	INJ_BOOL(d, c->fCopyProfileItemsOver)
	INJ_SKIP(d, 1)
	INJ_I16(d, c->sSectorX)
	INJ_I16(d, c->sSectorY)
	INJ_I8(d, c->bDirection)
	INJ_SKIP(d, 1)
	INJ_I16(d, c->sInsertionGridNo)
	INJ_I8(d, c->bTeam)
	INJ_I8(d, c->bBodyType)
	INJ_I8(d, c->bAttitude)
	INJ_I8(d, c->bOrders)
	INJ_I8(d, c->bLifeMax)
	INJ_I8(d, c->bLife)
	INJ_I8(d, c->bAgility)
	INJ_I8(d, c->bDexterity)
	INJ_I8(d, c->bExpLevel)
	INJ_I8(d, c->bMarksmanship)
	INJ_I8(d, c->bMedical)
	INJ_I8(d, c->bMechanical)
	INJ_I8(d, c->bExplosive)
	INJ_I8(d, c->bLeadership)
	INJ_I8(d, c->bStrength)
	INJ_I8(d, c->bWisdom)
	INJ_I8(d, c->bMorale)
	INJ_I8(d, c->bAIMorale)
	for (size_t i = 0; i < lengthof(c->Inv); i++)
	{
		d = InjectObject(d, &c->Inv[i]);
	}
	INJ_STR(d, c->HeadPal, lengthof(c->HeadPal))
	INJ_STR(d, c->PantsPal, lengthof(c->PantsPal))
	INJ_STR(d, c->VestPal, lengthof(c->VestPal))
	INJ_STR(d, c->SkinPal, lengthof(c->SkinPal))
	INJ_SKIP(d, 30)
	INJ_I16A(d, c->sPatrolGrid, lengthof(c->sPatrolGrid))
	INJ_I8(d, c->bPatrolCnt)
	INJ_BOOL(d, c->fVisible)
	INJ_WSTR16(d, c->name, lengthof(c->name))
	INJ_U8(d, c->ubSoldierClass)
	INJ_BOOL(d, c->fOnRoof)
	INJ_I8(d, c->bSectorZ)
	INJ_SKIP(d, 6)
	INJ_U8(d, c->ubCivilianGroup)
	INJ_SKIP(d, 1)
	INJ_U8(d, c->ubScheduleID)
	INJ_BOOL(d, c->fUseGivenVehicle)
	INJ_I8(d, c->bUseGivenVehicleID)
	INJ_BOOL(d, c->fHasKeys)
	INJ_SKIP(d, 117)
	Assert(d == data + 1040);
}


void InjectSoldierCreateIntoFileUTF16(HWFILE const f, SOLDIERCREATE_STRUCT const* const c)
{
	BYTE data[1040];
	InjectSoldierCreateUTF16(data, c);
	FileWrite(f, data, sizeof(data));
}


static void InjectSoldierCreate(BYTE* const data, const SOLDIERCREATE_STRUCT* const c)
{
	BYTE* d = data;
	INJ_BOOL(d, c->fStatic)
	INJ_U8(d, c->ubProfile)
	INJ_SKIP(d, 2)
	INJ_BOOL(d, c->fCopyProfileItemsOver)
	INJ_SKIP(d, 1)
	INJ_I16(d, c->sSectorX)
	INJ_I16(d, c->sSectorY)
	INJ_I8(d, c->bDirection)
	INJ_SKIP(d, 1)
	INJ_I16(d, c->sInsertionGridNo)
	INJ_I8(d, c->bTeam)
	INJ_I8(d, c->bBodyType)
	INJ_I8(d, c->bAttitude)
	INJ_I8(d, c->bOrders)
	INJ_I8(d, c->bLifeMax)
	INJ_I8(d, c->bLife)
	INJ_I8(d, c->bAgility)
	INJ_I8(d, c->bDexterity)
	INJ_I8(d, c->bExpLevel)
	INJ_I8(d, c->bMarksmanship)
	INJ_I8(d, c->bMedical)
	INJ_I8(d, c->bMechanical)
	INJ_I8(d, c->bExplosive)
	INJ_I8(d, c->bLeadership)
	INJ_I8(d, c->bStrength)
	INJ_I8(d, c->bWisdom)
	INJ_I8(d, c->bMorale)
	INJ_I8(d, c->bAIMorale)
	for (size_t i = 0; i < lengthof(c->Inv); i++)
	{
		d = InjectObject(d, &c->Inv[i]);
	}
	INJ_STR(d, c->HeadPal, lengthof(c->HeadPal))
	INJ_STR(d, c->PantsPal, lengthof(c->PantsPal))
	INJ_STR(d, c->VestPal, lengthof(c->VestPal))
	INJ_STR(d, c->SkinPal, lengthof(c->SkinPal))
	INJ_SKIP(d, 30)
	INJ_I16A(d, c->sPatrolGrid, lengthof(c->sPatrolGrid))
	INJ_I8(d, c->bPatrolCnt)
	INJ_BOOL(d, c->fVisible)
#ifdef _WIN32 // XXX HACK000A
	INJ_WSTR16(d, c->name, lengthof(c->name))
#else
	INJ_WSTR32(d, c->name, lengthof(c->name))
#endif
	INJ_U8(d, c->ubSoldierClass)
	INJ_BOOL(d, c->fOnRoof)
	INJ_I8(d, c->bSectorZ)
	INJ_SKIP(d, 6)
	INJ_U8(d, c->ubCivilianGroup)
	INJ_SKIP(d, 1)
	INJ_U8(d, c->ubScheduleID)
	INJ_BOOL(d, c->fUseGivenVehicle)
	INJ_I8(d, c->bUseGivenVehicleID)
	INJ_BOOL(d, c->fHasKeys)
	INJ_SKIP(d, 117)
#ifdef _WIN32 // XXX HACK000A
	Assert(d == data + 1040);
#else
	Assert(d == data + 1060);
#endif
}


void InjectSoldierCreateIntoFile(HWFILE const f, SOLDIERCREATE_STRUCT const* const c)
{
#ifdef _WIN32 // XXX HACK000A
	BYTE data[1040];
#else
	BYTE data[1060];
#endif
	InjectSoldierCreate(data, c);
	FileWrite(f, data, sizeof(data));
}
