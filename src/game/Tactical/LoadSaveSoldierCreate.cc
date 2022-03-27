#include "Debug.h"
#include "LoadSaveSoldierCreate.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveObjectType.h"

#include "Logger.h"

#include <string_theory/string>


UINT16 CalcSoldierCreateCheckSum(const SOLDIERCREATE_STRUCT* const s)
{
	return
		s->bLife            *  7 +
		s->bLifeMax         *  8 -
		s->bAgility         *  2 +
		s->bDexterity       *  1 +
		s->bExpLevel        *  5 -
		s->bMarksmanship    *  9 +
		s->bMedical         * 10 +
		s->bMechanical      *  3 +
		s->bExplosive       *  4 +
		s->bLeadership      *  5 +
		s->bStrength        *  7 +
		s->bWisdom          * 11 +
		s->bMorale          *  7 +
		s->bAIMorale        *  3 -
		s->bBodyType        *  7 +
		4                   *  6 +
		s->sSector.x        *  7 -
		s->ubSoldierClass   *  4 +
		s->bTeam            *  7 +
		s->bDirection       *  5 +
		s->fOnRoof          * 17 +
		s->sInsertionGridNo *  1 +
		3;
}


static void ExtractSoldierCreate(const BYTE* const data, SOLDIERCREATE_STRUCT* const c, bool stracLinuxFormat)
{
	DataReader d{data};
	EXTR_BOOL(d, c->fStatic)
	EXTR_U8(d, c->ubProfile)
	EXTR_SKIP(d, 2)
	EXTR_BOOL(d, c->fCopyProfileItemsOver)
	EXTR_SKIP(d, 1)
	EXTR_I16(d, c->sSector.x)
	EXTR_I16(d, c->sSector.y)
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
		ExtractObject(d, &c->Inv[i]);
	}
	c->HeadPal = d.readUTF8(PaletteRepID_LENGTH, ST::substitute_invalid);
	c->PantsPal = d.readUTF8(PaletteRepID_LENGTH, ST::substitute_invalid);
	c->VestPal = d.readUTF8(PaletteRepID_LENGTH, ST::substitute_invalid);
	c->SkinPal = d.readUTF8(PaletteRepID_LENGTH, ST::substitute_invalid);
	EXTR_SKIP(d, 30)
	EXTR_I16A(d, c->sPatrolGrid, lengthof(c->sPatrolGrid))
	EXTR_I8(d, c->bPatrolCnt)
	EXTR_BOOL(d, c->fVisible);
	if(stracLinuxFormat)
	{
		c->name = d.readUTF32(SOLDIERTYPE_NAME_LENGTH);
	}
	else
	{
		c->name = d.readUTF16(SOLDIERTYPE_NAME_LENGTH);
	}
	EXTR_U8(d, c->ubSoldierClass)
	EXTR_BOOL(d, c->fOnRoof)
	EXTR_I8(d, c->sSector.z)
	EXTR_SKIP(d, 6)
	EXTR_U8(d, c->ubCivilianGroup)
	EXTR_SKIP(d, 1)
	EXTR_U8(d, c->ubScheduleID)
	EXTR_BOOL(d, c->fUseGivenVehicle)
	EXTR_I8(d, c->bUseGivenVehicleID)
	EXTR_BOOL(d, c->fHasKeys)
	EXTR_SKIP(d, 117)
	if(stracLinuxFormat)
	{
		Assert(d.getConsumed() == 1060);
	}
	else
	{
		Assert(d.getConsumed() == 1040);
	}
}


void ExtractSoldierCreateFromFile(HWFILE const f, SOLDIERCREATE_STRUCT* const c, bool stracLinuxFormat)
{
	if(stracLinuxFormat)
	{
		BYTE data[1060];
		f->read(data, sizeof(data));
		ExtractSoldierCreate(data, c, stracLinuxFormat);
	}
	else
	{
		BYTE data[1040];
		f->read(data, sizeof(data));
		ExtractSoldierCreate(data, c, stracLinuxFormat);
	}
}

/**
* Load SOLDIERCREATE_STRUCT structure and checksum from the file and guess the
* format the structure was saved in (vanilla windows format or stracciatella linux format). */
void ExtractSoldierCreateFromFileWithChecksumAndGuess(HWFILE f, SOLDIERCREATE_STRUCT* c, UINT16 *checksum)
{
	// First trying to load the windows format.
	// If checksum doesn't match, trying to load linux format.

	const INT32 pos = f->pos();
	ExtractSoldierCreateFromFile(f, c, false);
	f->read(checksum, 2);

	UINT16 const fresh_checksum = CalcSoldierCreateCheckSum(c);
	if(*checksum != fresh_checksum)
	{
		SLOGI("trying SOLDIERCREATE_STRUCT in linux format");

		// trying linux format
		// not validating the checksum - it will be the job of the caller
		f->seek(pos, FILE_SEEK_FROM_START);
		ExtractSoldierCreateFromFile(f, c, true);
		f->read(checksum, 2);
	}
}

static void InjectSoldierCreate(BYTE* const data, const SOLDIERCREATE_STRUCT* const c)
{
	DataWriter d{data};
	INJ_BOOL(d, c->fStatic)
	INJ_U8(d, c->ubProfile)
	INJ_SKIP(d, 2)
	INJ_BOOL(d, c->fCopyProfileItemsOver)
	INJ_SKIP(d, 1)
	INJ_I16(d, c->sSector.x)
	INJ_I16(d, c->sSector.y)
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
		InjectObject(d, &c->Inv[i]);
	}
	d.writeUTF8(c->HeadPal, PaletteRepID_LENGTH);
	d.writeUTF8(c->PantsPal, PaletteRepID_LENGTH);
	d.writeUTF8(c->VestPal, PaletteRepID_LENGTH);
	d.writeUTF8(c->SkinPal, PaletteRepID_LENGTH);
	INJ_SKIP(d, 30)
	INJ_I16A(d, c->sPatrolGrid, lengthof(c->sPatrolGrid))
	INJ_I8(d, c->bPatrolCnt)
	INJ_BOOL(d, c->fVisible)
	d.writeUTF16(c->name, SOLDIERTYPE_NAME_LENGTH);
	INJ_U8(d, c->ubSoldierClass)
	INJ_BOOL(d, c->fOnRoof)
	INJ_I8(d, c->sSector.z)
	INJ_SKIP(d, 6)
	INJ_U8(d, c->ubCivilianGroup)
	INJ_SKIP(d, 1)
	INJ_U8(d, c->ubScheduleID)
	INJ_BOOL(d, c->fUseGivenVehicle)
	INJ_I8(d, c->bUseGivenVehicleID)
	INJ_BOOL(d, c->fHasKeys)
	INJ_SKIP(d, 117)
	Assert(d.getConsumed() == 1040);
}


void InjectSoldierCreateIntoFile(HWFILE const f, SOLDIERCREATE_STRUCT const* const c)
{
	BYTE data[1040];
	InjectSoldierCreate(data, c);
	f->write(data, sizeof(data));
}
