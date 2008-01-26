#include "Debug.h"
#include "LoadSaveSoldierCreate.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveObjectType.h"


const BYTE* ExtractSoldierCreateUTF16(const BYTE* Src, SOLDIERCREATE_STRUCT* Soldier)
{
	const BYTE* S = Src;

	EXTR_BOOL(S, Soldier->fStatic)
	EXTR_U8(S, Soldier->ubProfile)
	EXTR_BOOL(S, Soldier->fPlayerMerc)
	EXTR_SKIP(S, 1)
	EXTR_BOOL(S, Soldier->fCopyProfileItemsOver)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Soldier->sSectorX)
	EXTR_I16(S, Soldier->sSectorY)
	EXTR_I8(S, Soldier->bDirection)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Soldier->sInsertionGridNo)
	EXTR_I8(S, Soldier->bTeam)
	EXTR_I8(S, Soldier->bBodyType)
	EXTR_I8(S, Soldier->bAttitude)
	EXTR_I8(S, Soldier->bOrders)
	EXTR_I8(S, Soldier->bLifeMax)
	EXTR_I8(S, Soldier->bLife)
	EXTR_I8(S, Soldier->bAgility)
	EXTR_I8(S, Soldier->bDexterity)
	EXTR_I8(S, Soldier->bExpLevel)
	EXTR_I8(S, Soldier->bMarksmanship)
	EXTR_I8(S, Soldier->bMedical)
	EXTR_I8(S, Soldier->bMechanical)
	EXTR_I8(S, Soldier->bExplosive)
	EXTR_I8(S, Soldier->bLeadership)
	EXTR_I8(S, Soldier->bStrength)
	EXTR_I8(S, Soldier->bWisdom)
	EXTR_I8(S, Soldier->bMorale)
	EXTR_I8(S, Soldier->bAIMorale)
	for (size_t i = 0; i < lengthof(Soldier->Inv); i++)
	{
		S = ExtractObject(S, &Soldier->Inv[i]);
	}
	EXTR_STR(S, Soldier->HeadPal, lengthof(Soldier->HeadPal))
	EXTR_STR(S, Soldier->PantsPal, lengthof(Soldier->PantsPal))
	EXTR_STR(S, Soldier->VestPal, lengthof(Soldier->VestPal))
	EXTR_STR(S, Soldier->SkinPal, lengthof(Soldier->SkinPal))
	EXTR_STR(S, Soldier->MiscPal, lengthof(Soldier->MiscPal))
	EXTR_I16A(S, Soldier->sPatrolGrid, lengthof(Soldier->sPatrolGrid))
	EXTR_I8(S, Soldier->bPatrolCnt)
	EXTR_BOOL(S, Soldier->fVisible)
	EXTR_WSTR16(S, Soldier->name, lengthof(Soldier->name))
	EXTR_U8(S, Soldier->ubSoldierClass)
	EXTR_BOOL(S, Soldier->fOnRoof)
	EXTR_I8(S, Soldier->bSectorZ)
	EXTR_SKIP(S, 1)
	EXTR_PTR(S, Soldier->pExistingSoldier)
	EXTR_SKIP(S, 1)
	EXTR_U8(S, Soldier->ubCivilianGroup)
	EXTR_BOOL(S, Soldier->fKillSlotIfOwnerDies)
	EXTR_U8(S, Soldier->ubScheduleID)
	EXTR_BOOL(S, Soldier->fUseGivenVehicle)
	EXTR_I8(S, Soldier->bUseGivenVehicleID)
	EXTR_BOOL(S, Soldier->fHasKeys)
	EXTR_SKIP(S, 117)

	Assert(S == Src + 1040);
	return S;
}


BOOLEAN ExtractSoldierCreateFromFileUTF16(HWFILE File, SOLDIERCREATE_STRUCT* Soldier)
{
	BYTE Data[1040];
	BOOLEAN Ret = FileRead(File, Data, sizeof(Data));
	if (Ret) ExtractSoldierCreateUTF16(Data, Soldier);
	return Ret;
}


static void ExtractSoldierCreate(const BYTE* Src, SOLDIERCREATE_STRUCT* Soldier)
{
	const BYTE* S = Src;

	EXTR_BOOL(S, Soldier->fStatic)
	EXTR_U8(S, Soldier->ubProfile)
	EXTR_BOOL(S, Soldier->fPlayerMerc)
	EXTR_SKIP(S, 1)
	EXTR_BOOL(S, Soldier->fCopyProfileItemsOver)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Soldier->sSectorX)
	EXTR_I16(S, Soldier->sSectorY)
	EXTR_I8(S, Soldier->bDirection)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, Soldier->sInsertionGridNo)
	EXTR_I8(S, Soldier->bTeam)
	EXTR_I8(S, Soldier->bBodyType)
	EXTR_I8(S, Soldier->bAttitude)
	EXTR_I8(S, Soldier->bOrders)
	EXTR_I8(S, Soldier->bLifeMax)
	EXTR_I8(S, Soldier->bLife)
	EXTR_I8(S, Soldier->bAgility)
	EXTR_I8(S, Soldier->bDexterity)
	EXTR_I8(S, Soldier->bExpLevel)
	EXTR_I8(S, Soldier->bMarksmanship)
	EXTR_I8(S, Soldier->bMedical)
	EXTR_I8(S, Soldier->bMechanical)
	EXTR_I8(S, Soldier->bExplosive)
	EXTR_I8(S, Soldier->bLeadership)
	EXTR_I8(S, Soldier->bStrength)
	EXTR_I8(S, Soldier->bWisdom)
	EXTR_I8(S, Soldier->bMorale)
	EXTR_I8(S, Soldier->bAIMorale)
	for (size_t i = 0; i < lengthof(Soldier->Inv); i++)
	{
		S = ExtractObject(S, &Soldier->Inv[i]);
	}
	EXTR_STR(S, Soldier->HeadPal, lengthof(Soldier->HeadPal))
	EXTR_STR(S, Soldier->PantsPal, lengthof(Soldier->PantsPal))
	EXTR_STR(S, Soldier->VestPal, lengthof(Soldier->VestPal))
	EXTR_STR(S, Soldier->SkinPal, lengthof(Soldier->SkinPal))
	EXTR_STR(S, Soldier->MiscPal, lengthof(Soldier->MiscPal))
	EXTR_I16A(S, Soldier->sPatrolGrid, lengthof(Soldier->sPatrolGrid))
	EXTR_I8(S, Soldier->bPatrolCnt)
	EXTR_BOOL(S, Soldier->fVisible)
#ifdef _WIN32 // XXX HACK000A
	EXTR_WSTR16(S, Soldier->name, lengthof(Soldier->name))
#else
	EXTR_WSTR32(S, Soldier->name, lengthof(Soldier->name))
#endif
	EXTR_U8(S, Soldier->ubSoldierClass)
	EXTR_BOOL(S, Soldier->fOnRoof)
	EXTR_I8(S, Soldier->bSectorZ)
	EXTR_SKIP(S, 1)
	EXTR_PTR(S, Soldier->pExistingSoldier)
	EXTR_SKIP(S, 1)
	EXTR_U8(S, Soldier->ubCivilianGroup)
	EXTR_BOOL(S, Soldier->fKillSlotIfOwnerDies)
	EXTR_U8(S, Soldier->ubScheduleID)
	EXTR_BOOL(S, Soldier->fUseGivenVehicle)
	EXTR_I8(S, Soldier->bUseGivenVehicleID)
	EXTR_BOOL(S, Soldier->fHasKeys)
	EXTR_SKIP(S, 117)

#ifdef _WIN32 // XXX HACK000A
	Assert(S == Src + 1040);
#else
	Assert(S == Src + 1060);
#endif
}


BOOLEAN ExtractSoldierCreateFromFile(HWFILE File, SOLDIERCREATE_STRUCT* Soldier)
{
#ifdef _WIN32 // XXX HACK000A
	BYTE Data[1040];
#else
	BYTE Data[1060];
#endif
	BOOLEAN Ret = FileRead(File, Data, sizeof(Data));
	if (Ret) ExtractSoldierCreate(Data, Soldier);
	return Ret;
}


static void InjectSoldierCreateUTF16(BYTE* Dst, const SOLDIERCREATE_STRUCT* Soldier)
{
	BYTE* D = Dst;

	INJ_BOOL(D, Soldier->fStatic)
	INJ_U8(D, Soldier->ubProfile)
	INJ_BOOL(D, Soldier->fPlayerMerc)
	INJ_SKIP(D, 1)
	INJ_BOOL(D, Soldier->fCopyProfileItemsOver)
	INJ_SKIP(D, 1)
	INJ_I16(D, Soldier->sSectorX)
	INJ_I16(D, Soldier->sSectorY)
	INJ_I8(D, Soldier->bDirection)
	INJ_SKIP(D, 1)
	INJ_I16(D, Soldier->sInsertionGridNo)
	INJ_I8(D, Soldier->bTeam)
	INJ_I8(D, Soldier->bBodyType)
	INJ_I8(D, Soldier->bAttitude)
	INJ_I8(D, Soldier->bOrders)
	INJ_I8(D, Soldier->bLifeMax)
	INJ_I8(D, Soldier->bLife)
	INJ_I8(D, Soldier->bAgility)
	INJ_I8(D, Soldier->bDexterity)
	INJ_I8(D, Soldier->bExpLevel)
	INJ_I8(D, Soldier->bMarksmanship)
	INJ_I8(D, Soldier->bMedical)
	INJ_I8(D, Soldier->bMechanical)
	INJ_I8(D, Soldier->bExplosive)
	INJ_I8(D, Soldier->bLeadership)
	INJ_I8(D, Soldier->bStrength)
	INJ_I8(D, Soldier->bWisdom)
	INJ_I8(D, Soldier->bMorale)
	INJ_I8(D, Soldier->bAIMorale)
	for (size_t i = 0; i < lengthof(Soldier->Inv); i++)
	{
		D = InjectObject(D, &Soldier->Inv[i]);
	}
	INJ_STR(D, Soldier->HeadPal, lengthof(Soldier->HeadPal))
	INJ_STR(D, Soldier->PantsPal, lengthof(Soldier->PantsPal))
	INJ_STR(D, Soldier->VestPal, lengthof(Soldier->VestPal))
	INJ_STR(D, Soldier->SkinPal, lengthof(Soldier->SkinPal))
	INJ_STR(D, Soldier->MiscPal, lengthof(Soldier->MiscPal))
	INJ_I16A(D, Soldier->sPatrolGrid, lengthof(Soldier->sPatrolGrid))
	INJ_I8(D, Soldier->bPatrolCnt)
	INJ_BOOL(D, Soldier->fVisible)
	INJ_WSTR16(D, Soldier->name, lengthof(Soldier->name))
	INJ_U8(D, Soldier->ubSoldierClass)
	INJ_BOOL(D, Soldier->fOnRoof)
	INJ_I8(D, Soldier->bSectorZ)
	INJ_SKIP(D, 1)
	INJ_PTR(D, Soldier->pExistingSoldier)
	INJ_SKIP(D, 1)
	INJ_U8(D, Soldier->ubCivilianGroup)
	INJ_BOOL(D, Soldier->fKillSlotIfOwnerDies)
	INJ_U8(D, Soldier->ubScheduleID)
	INJ_BOOL(D, Soldier->fUseGivenVehicle)
	INJ_I8(D, Soldier->bUseGivenVehicleID)
	INJ_BOOL(D, Soldier->fHasKeys)
	INJ_SKIP(D, 117)

	Assert(D == Dst + 1040);
}


BOOLEAN InjectSoldierCreateIntoFileUTF16(HWFILE File, const SOLDIERCREATE_STRUCT* Soldier)
{
	BYTE Data[1040];
	InjectSoldierCreateUTF16(Data, Soldier);
	return FileWrite(File, Data, sizeof(Data));
}


static void InjectSoldierCreate(BYTE* Dst, const SOLDIERCREATE_STRUCT* Soldier)
{
	BYTE* D = Dst;

	INJ_BOOL(D, Soldier->fStatic)
	INJ_U8(D, Soldier->ubProfile)
	INJ_BOOL(D, Soldier->fPlayerMerc)
	INJ_SKIP(D, 1)
	INJ_BOOL(D, Soldier->fCopyProfileItemsOver)
	INJ_SKIP(D, 1)
	INJ_I16(D, Soldier->sSectorX)
	INJ_I16(D, Soldier->sSectorY)
	INJ_I8(D, Soldier->bDirection)
	INJ_SKIP(D, 1)
	INJ_I16(D, Soldier->sInsertionGridNo)
	INJ_I8(D, Soldier->bTeam)
	INJ_I8(D, Soldier->bBodyType)
	INJ_I8(D, Soldier->bAttitude)
	INJ_I8(D, Soldier->bOrders)
	INJ_I8(D, Soldier->bLifeMax)
	INJ_I8(D, Soldier->bLife)
	INJ_I8(D, Soldier->bAgility)
	INJ_I8(D, Soldier->bDexterity)
	INJ_I8(D, Soldier->bExpLevel)
	INJ_I8(D, Soldier->bMarksmanship)
	INJ_I8(D, Soldier->bMedical)
	INJ_I8(D, Soldier->bMechanical)
	INJ_I8(D, Soldier->bExplosive)
	INJ_I8(D, Soldier->bLeadership)
	INJ_I8(D, Soldier->bStrength)
	INJ_I8(D, Soldier->bWisdom)
	INJ_I8(D, Soldier->bMorale)
	INJ_I8(D, Soldier->bAIMorale)
	for (size_t i = 0; i < lengthof(Soldier->Inv); i++)
	{
		D = InjectObject(D, &Soldier->Inv[i]);
	}
	INJ_STR(D, Soldier->HeadPal, lengthof(Soldier->HeadPal))
	INJ_STR(D, Soldier->PantsPal, lengthof(Soldier->PantsPal))
	INJ_STR(D, Soldier->VestPal, lengthof(Soldier->VestPal))
	INJ_STR(D, Soldier->SkinPal, lengthof(Soldier->SkinPal))
	INJ_STR(D, Soldier->MiscPal, lengthof(Soldier->MiscPal))
	INJ_I16A(D, Soldier->sPatrolGrid, lengthof(Soldier->sPatrolGrid))
	INJ_I8(D, Soldier->bPatrolCnt)
	INJ_BOOL(D, Soldier->fVisible)
#ifdef _WIN32 // XXX HACK000A
	INJ_WSTR16(D, Soldier->name, lengthof(Soldier->name))
#else
	INJ_WSTR32(D, Soldier->name, lengthof(Soldier->name))
#endif
	INJ_U8(D, Soldier->ubSoldierClass)
	INJ_BOOL(D, Soldier->fOnRoof)
	INJ_I8(D, Soldier->bSectorZ)
	INJ_SKIP(D, 1)
	INJ_PTR(D, Soldier->pExistingSoldier)
	INJ_SKIP(D, 1)
	INJ_U8(D, Soldier->ubCivilianGroup)
	INJ_BOOL(D, Soldier->fKillSlotIfOwnerDies)
	INJ_U8(D, Soldier->ubScheduleID)
	INJ_BOOL(D, Soldier->fUseGivenVehicle)
	INJ_I8(D, Soldier->bUseGivenVehicleID)
	INJ_BOOL(D, Soldier->fHasKeys)
	INJ_SKIP(D, 117)

#ifdef _WIN32 // XXX HACK000A
	Assert(D == Dst + 1040);
#else
	Assert(D == Dst + 1060);
#endif
}


BOOLEAN InjectSoldierCreateIntoFile(HWFILE File, const SOLDIERCREATE_STRUCT* Soldier)
{
#ifdef _WIN32 // XXX HACK000A
	BYTE Data[1040];
#else
	BYTE Data[1060];
#endif
	InjectSoldierCreate(Data, Soldier);
	return FileWrite(File, Data, sizeof(Data));
}
