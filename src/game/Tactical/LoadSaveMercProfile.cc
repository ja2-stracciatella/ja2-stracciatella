#include "Debug.h"
#include "FileMan.h"
#include "LoadSaveData.h"
#include "LoadSaveMercProfile.h"
#include "Tactical_Save.h"

#include <string_theory/string>

#include <stdexcept>


/** Calculates soldier profile checksum. */
UINT32 SoldierProfileChecksum(MERCPROFILESTRUCT const& p)
{
	UINT32 sum = 1;

	sum += 1 + p.bLife;
	sum *= 1 + p.bLifeMax;
	sum += 1 + p.bAgility;
	sum *= 1 + p.bDexterity;
	sum += 1 + p.bStrength;
	sum *= 1 + p.bMarksmanship;
	sum += 1 + p.bMedical;
	sum *= 1 + p.bMechanical;
	sum += 1 + p.bExplosive;
	sum *= 1 + p.bExpLevel;

	FOR_EACH(UINT16 const, i, p.inv)        sum += *i;
	FOR_EACH(UINT8  const, i, p.bInvNumber) sum += *i;

	return sum;
}


/**
* Extract merc profile from the binary data.
* @param encodingCorrection Perform encoding correction - it is necessary for loading strings from the game data files. */
void ExtractMercProfile(BYTE const* const Src, MERCPROFILESTRUCT& p, bool stracLinuxFormat, UINT32 *checksum,
			const IEncodingCorrector *fixer)
{
	const BYTE* S = Src;

	if(stracLinuxFormat)
	{
		DataReader reader(S);
		p.zName = reader.readUTF32(NAME_LENGTH);
		p.zNickname = reader.readUTF32(NICKNAME_LENGTH);
		S += reader.getConsumed();
	}
	else
	{
		DataReader reader(S);
		p.zName = reader.readUTF16(NAME_LENGTH, fixer);
		p.zNickname = reader.readUTF16(NICKNAME_LENGTH, fixer);
		S += reader.getConsumed();
	}

	EXTR_SKIP(S, 28)
	EXTR_U8(S, p.ubFaceIndex)
	EXTR_STR(S, p.PANTS, lengthof(p.PANTS))
	EXTR_STR(S, p.VEST, lengthof(p.VEST))
	EXTR_STR(S, p.SKIN, lengthof(p.SKIN))
	EXTR_STR(S, p.HAIR, lengthof(p.HAIR))
	EXTR_I8(S, p.bSex)
	EXTR_I8(S, p.bArmourAttractiveness)
	EXTR_U8(S, p.ubMiscFlags2)
	EXTR_I8(S, p.bEvolution)
	EXTR_U8(S, p.ubMiscFlags)
	EXTR_U8(S, p.bSexist)
	EXTR_I8(S, p.bLearnToHate)
	EXTR_SKIP(S, 2)
	EXTR_U8(S, p.ubQuoteRecord)
	EXTR_I8(S, p.bDeathRate)
	EXTR_SKIP(S, 2)
	EXTR_I16(S, p.sExpLevelGain)
	EXTR_I16(S, p.sLifeGain)
	EXTR_I16(S, p.sAgilityGain)
	EXTR_I16(S, p.sDexterityGain)
	EXTR_I16(S, p.sWisdomGain)
	EXTR_I16(S, p.sMarksmanshipGain)
	EXTR_I16(S, p.sMedicalGain)
	EXTR_I16(S, p.sMechanicGain)
	EXTR_I16(S, p.sExplosivesGain)
	EXTR_U8(S, p.ubBodyType)
	EXTR_I8(S, p.bMedical)
	EXTR_U16(S, p.usEyesX)
	EXTR_U16(S, p.usEyesY)
	EXTR_U16(S, p.usMouthX)
	EXTR_U16(S, p.usMouthY)
	EXTR_SKIP(S, 10)
	EXTR_U32(S, p.uiBlinkFrequency)
	EXTR_U32(S, p.uiExpressionFrequency)
	EXTR_U16(S, p.sSectorX)
	EXTR_U16(S, p.sSectorY)
	EXTR_U32(S, p.uiDayBecomesAvailable)
	EXTR_I8(S, p.bStrength)
	EXTR_I8(S, p.bLifeMax)
	EXTR_I8(S, p.bExpLevelDelta)
	EXTR_I8(S, p.bLifeDelta)
	EXTR_I8(S, p.bAgilityDelta)
	EXTR_I8(S, p.bDexterityDelta)
	EXTR_I8(S, p.bWisdomDelta)
	EXTR_I8(S, p.bMarksmanshipDelta)
	EXTR_I8(S, p.bMedicalDelta)
	EXTR_I8(S, p.bMechanicDelta)
	EXTR_I8(S, p.bExplosivesDelta)
	EXTR_I8(S, p.bStrengthDelta)
	EXTR_I8(S, p.bLeadershipDelta)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, p.usKills)
	EXTR_U16(S, p.usAssists)
	EXTR_U16(S, p.usShotsFired)
	EXTR_U16(S, p.usShotsHit)
	EXTR_U16(S, p.usBattlesFought)
	EXTR_U16(S, p.usTimesWounded)
	EXTR_U16(S, p.usTotalDaysServed)
	EXTR_I16(S, p.sLeadershipGain)
	EXTR_I16(S, p.sStrengthGain)
	EXTR_U32(S, p.uiBodyTypeSubFlags)
	EXTR_I16(S, p.sSalary)
	EXTR_I8(S, p.bLife)
	EXTR_I8(S, p.bDexterity)
	EXTR_I8(S, p.bPersonalityTrait)
	EXTR_I8(S, p.bSkillTrait)
	EXTR_I8(S, p.bReputationTolerance)
	EXTR_I8(S, p.bExplosive)
	EXTR_I8(S, p.bSkillTrait2)
	EXTR_I8(S, p.bLeadership)
	EXTR_I8A(S, p.bBuddy, lengthof(p.bBuddy))
	EXTR_I8A(S, p.bHated, lengthof(p.bHated))
	EXTR_I8(S, p.bExpLevel)
	EXTR_I8(S, p.bMarksmanship)
	EXTR_SKIP(S, 1)
	EXTR_I8(S, p.bWisdom)
	EXTR_SKIP(S, 2)
	EXTR_U8A(S, p.bInvStatus, lengthof(p.bInvStatus))
	EXTR_U8A(S, p.bInvNumber, lengthof(p.bInvNumber))
	EXTR_U16A(S, p.usApproachFactor, lengthof(p.usApproachFactor))
	EXTR_I8(S, p.bMainGunAttractiveness)
	EXTR_I8(S, p.bAgility)
	EXTR_BOOL(S, p.fUseProfileInsertionInfo)
	EXTR_SKIP(S, 1)
	EXTR_I16(S, p.sGridNo)
	EXTR_U8(S, p.ubQuoteActionID)
	EXTR_I8(S, p.bMechanical)
	EXTR_U8(S, p.ubInvUndroppable)
	EXTR_U8A(S, p.ubRoomRangeStart, lengthof(p.ubRoomRangeStart))
	EXTR_SKIP(S, 1)
	EXTR_U16A(S, p.inv, lengthof(p.inv))
	EXTR_SKIP(S, 20)
	EXTR_U16A(S, p.usStatChangeChances, lengthof(p.usStatChangeChances))
	EXTR_U16A(S, p.usStatChangeSuccesses, lengthof(p.usStatChangeSuccesses))
	EXTR_U8(S, p.ubStrategicInsertionCode)
	EXTR_U8A(S, p.ubRoomRangeEnd, lengthof(p.ubRoomRangeEnd))
	EXTR_SKIP(S, 4)
	EXTR_U8(S, p.ubLastQuoteSaid)
	EXTR_I8(S, p.bRace)
	EXTR_I8(S, p.bNationality)
	EXTR_I8(S, p.bAppearance)
	EXTR_I8(S, p.bAppearanceCareLevel)
	EXTR_I8(S, p.bRefinement)
	EXTR_I8(S, p.bRefinementCareLevel)
	EXTR_I8(S, p.bHatedNationality)
	EXTR_I8(S, p.bHatedNationalityCareLevel)
	EXTR_I8(S, p.bRacist)
	EXTR_SKIP(S, 1)
	EXTR_U32(S, p.uiWeeklySalary)
	EXTR_U32(S, p.uiBiWeeklySalary)
	EXTR_I8(S, p.bMedicalDeposit)
	EXTR_I8(S, p.bAttitude)
	EXTR_SKIP(S, 2)
	EXTR_U16(S, p.sMedicalDepositAmount)
	EXTR_I8(S, p.bLearnToLike)
	EXTR_U8A(S, p.ubApproachVal, lengthof(p.ubApproachVal))
	EXTR_U8A(S, *p.ubApproachMod, sizeof(p.ubApproachMod) / sizeof(**p.ubApproachMod))
	EXTR_I8(S, p.bTown)
	EXTR_I8(S, p.bTownAttachment)
	EXTR_SKIP(S, 1)
	EXTR_U16(S, p.usOptionalGearCost)
	EXTR_I8A(S, p.bMercOpinion, lengthof(p.bMercOpinion))
	EXTR_I8(S, p.bApproached)
	EXTR_I8(S, p.bMercStatus)
	EXTR_I8A(S, p.bHatedTime, lengthof(p.bHatedTime))
	EXTR_I8(S, p.bLearnToLikeTime)
	EXTR_I8(S, p.bLearnToHateTime)
	EXTR_I8A(S, p.bHatedCount, lengthof(p.bHatedCount))
	EXTR_I8(S, p.bLearnToLikeCount)
	EXTR_I8(S, p.bLearnToHateCount)
	EXTR_U8(S, p.ubLastDateSpokenTo)
	EXTR_U8(S, p.bLastQuoteSaidWasSpecial)
	EXTR_I8(S, p.bSectorZ)
	EXTR_U16(S, p.usStrategicInsertionData)
	EXTR_I8(S, p.bFriendlyOrDirectDefaultResponseUsedRecently)
	EXTR_I8(S, p.bRecruitDefaultResponseUsedRecently)
	EXTR_I8(S, p.bThreatenDefaultResponseUsedRecently)
	EXTR_I8(S, p.bNPCData)
	EXTR_I32(S, p.iBalance)
	EXTR_SKIP(S, 2)
	EXTR_U8(S, p.ubCivilianGroup)
	EXTR_U8(S, p.ubNeedForSleep)
	EXTR_U32(S, p.uiMoney)
	EXTR_I8(S, p.bNPCData2)
	EXTR_U8(S, p.ubMiscFlags3)
	EXTR_U8(S, p.ubDaysOfMoraleHangover)
	EXTR_U8(S, p.ubNumTimesDrugUseInLifetime)
	EXTR_U32(S, p.uiPrecedentQuoteSaid)
	EXTR_U32(S, *checksum)
	EXTR_I16(S, p.sPreCombatGridNo)
	EXTR_U8(S, p.ubTimeTillNextHatedComplaint)
	EXTR_U8(S, p.ubSuspiciousDeath)
	EXTR_I32(S, p.iMercMercContractLength)
	EXTR_U32(S, p.uiTotalCostToDate)
	EXTR_SKIP(S, 4)
	if(stracLinuxFormat)
	{
		Assert(S == Src + MERC_PROFILE_SIZE_STRAC_LINUX);
	}
	else
	{
		Assert(S == Src + MERC_PROFILE_SIZE);
	}
}


/** Extract IMP merc profile from file.
* If saved checksum is not correct, exception will be thrown. */
void ExtractImpProfileFromFile(SGPFile *hFile, INT32 *iProfileId, INT32 *iPortraitNumber, MERCPROFILESTRUCT& p)
{
	UINT32 fileSize = FileGetSize(hFile);

	// read in the profile
	FileRead(hFile, iProfileId, sizeof(INT32));

	// read in the portrait
	FileRead(hFile, iPortraitNumber, sizeof(INT32));

	// read in the profile
	// not checking the checksum
	UINT32 checksum;
	if(fileSize >= MERC_PROFILE_SIZE_STRAC_LINUX)
	{
		std::vector<BYTE> data(MERC_PROFILE_SIZE_STRAC_LINUX);
		FileRead(hFile, data.data(), MERC_PROFILE_SIZE_STRAC_LINUX);
		ExtractMercProfile(data.data(), p, true, &checksum, NULL);
	}
	else
	{
		std::vector<BYTE> data(MERC_PROFILE_SIZE);
		FileRead(hFile, data.data(), MERC_PROFILE_SIZE);
		ExtractMercProfile(data.data(), p, false, &checksum, NULL);
	}
}


void InjectMercProfile(BYTE* const Dst, MERCPROFILESTRUCT const& p)
{
	BYTE* D = Dst;

	{
		DataWriter writer(D);
		writer.writeUTF16(p.zName, NAME_LENGTH);
		writer.writeUTF16(p.zNickname, NICKNAME_LENGTH);
		D += writer.getConsumed();
	}
	INJ_SKIP(D, 28)
	INJ_U8(D, p.ubFaceIndex)
	INJ_STR(D, p.PANTS, lengthof(p.PANTS))
	INJ_STR(D, p.VEST, lengthof(p.VEST))
	INJ_STR(D, p.SKIN, lengthof(p.SKIN))
	INJ_STR(D, p.HAIR, lengthof(p.HAIR))
	INJ_I8(D, p.bSex)
	INJ_I8(D, p.bArmourAttractiveness)
	INJ_U8(D, p.ubMiscFlags2)
	INJ_I8(D, p.bEvolution)
	INJ_U8(D, p.ubMiscFlags)
	INJ_U8(D, p.bSexist)
	INJ_I8(D, p.bLearnToHate)
	INJ_SKIP(D, 2)
	INJ_U8(D, p.ubQuoteRecord)
	INJ_I8(D, p.bDeathRate)
	INJ_SKIP(D, 2)
	INJ_I16(D, p.sExpLevelGain)
	INJ_I16(D, p.sLifeGain)
	INJ_I16(D, p.sAgilityGain)
	INJ_I16(D, p.sDexterityGain)
	INJ_I16(D, p.sWisdomGain)
	INJ_I16(D, p.sMarksmanshipGain)
	INJ_I16(D, p.sMedicalGain)
	INJ_I16(D, p.sMechanicGain)
	INJ_I16(D, p.sExplosivesGain)
	INJ_U8(D, p.ubBodyType)
	INJ_I8(D, p.bMedical)
	INJ_U16(D, p.usEyesX)
	INJ_U16(D, p.usEyesY)
	INJ_U16(D, p.usMouthX)
	INJ_U16(D, p.usMouthY)
	INJ_SKIP(D, 10)
	INJ_U32(D, p.uiBlinkFrequency)
	INJ_U32(D, p.uiExpressionFrequency)
	INJ_U16(D, p.sSectorX)
	INJ_U16(D, p.sSectorY)
	INJ_U32(D, p.uiDayBecomesAvailable)
	INJ_I8(D, p.bStrength)
	INJ_I8(D, p.bLifeMax)
	INJ_I8(D, p.bExpLevelDelta)
	INJ_I8(D, p.bLifeDelta)
	INJ_I8(D, p.bAgilityDelta)
	INJ_I8(D, p.bDexterityDelta)
	INJ_I8(D, p.bWisdomDelta)
	INJ_I8(D, p.bMarksmanshipDelta)
	INJ_I8(D, p.bMedicalDelta)
	INJ_I8(D, p.bMechanicDelta)
	INJ_I8(D, p.bExplosivesDelta)
	INJ_I8(D, p.bStrengthDelta)
	INJ_I8(D, p.bLeadershipDelta)
	INJ_SKIP(D, 1)
	INJ_U16(D, p.usKills)
	INJ_U16(D, p.usAssists)
	INJ_U16(D, p.usShotsFired)
	INJ_U16(D, p.usShotsHit)
	INJ_U16(D, p.usBattlesFought)
	INJ_U16(D, p.usTimesWounded)
	INJ_U16(D, p.usTotalDaysServed)
	INJ_I16(D, p.sLeadershipGain)
	INJ_I16(D, p.sStrengthGain)
	INJ_U32(D, p.uiBodyTypeSubFlags)
	INJ_I16(D, p.sSalary)
	INJ_I8(D, p.bLife)
	INJ_I8(D, p.bDexterity)
	INJ_I8(D, p.bPersonalityTrait)
	INJ_I8(D, p.bSkillTrait)
	INJ_I8(D, p.bReputationTolerance)
	INJ_I8(D, p.bExplosive)
	INJ_I8(D, p.bSkillTrait2)
	INJ_I8(D, p.bLeadership)
	INJ_I8A(D, p.bBuddy, lengthof(p.bBuddy))
	INJ_I8A(D, p.bHated, lengthof(p.bHated))
	INJ_I8(D, p.bExpLevel)
	INJ_I8(D, p.bMarksmanship)
	INJ_SKIP(D, 1)
	INJ_I8(D, p.bWisdom)
	INJ_SKIP(D, 2)
	INJ_U8A(D, p.bInvStatus, lengthof(p.bInvStatus))
	INJ_U8A(D, p.bInvNumber, lengthof(p.bInvNumber))
	INJ_U16A(D, p.usApproachFactor, lengthof(p.usApproachFactor))
	INJ_I8(D, p.bMainGunAttractiveness)
	INJ_I8(D, p.bAgility)
	INJ_BOOL(D, p.fUseProfileInsertionInfo)
	INJ_SKIP(D, 1)
	INJ_I16(D, p.sGridNo)
	INJ_U8(D, p.ubQuoteActionID)
	INJ_I8(D, p.bMechanical)
	INJ_U8(D, p.ubInvUndroppable)
	INJ_U8A(D, p.ubRoomRangeStart, lengthof(p.ubRoomRangeStart))
	INJ_SKIP(D, 1)
	INJ_U16A(D, p.inv, lengthof(p.inv))
	INJ_SKIP(D, 20)
	INJ_U16A(D, p.usStatChangeChances, lengthof(p.usStatChangeChances))
	INJ_U16A(D, p.usStatChangeSuccesses, lengthof(p.usStatChangeSuccesses))
	INJ_U8(D, p.ubStrategicInsertionCode)
	INJ_U8A(D, p.ubRoomRangeEnd, lengthof(p.ubRoomRangeEnd))
	INJ_SKIP(D, 4)
	INJ_U8(D, p.ubLastQuoteSaid)
	INJ_I8(D, p.bRace)
	INJ_I8(D, p.bNationality)
	INJ_I8(D, p.bAppearance)
	INJ_I8(D, p.bAppearanceCareLevel)
	INJ_I8(D, p.bRefinement)
	INJ_I8(D, p.bRefinementCareLevel)
	INJ_I8(D, p.bHatedNationality)
	INJ_I8(D, p.bHatedNationalityCareLevel)
	INJ_I8(D, p.bRacist)
	INJ_SKIP(D, 1)
	INJ_U32(D, p.uiWeeklySalary)
	INJ_U32(D, p.uiBiWeeklySalary)
	INJ_I8(D, p.bMedicalDeposit)
	INJ_I8(D, p.bAttitude)
	INJ_SKIP(D, 2)
	INJ_U16(D, p.sMedicalDepositAmount)
	INJ_I8(D, p.bLearnToLike)
	INJ_U8A(D, p.ubApproachVal, lengthof(p.ubApproachVal))
	INJ_U8A(D, *p.ubApproachMod, sizeof(p.ubApproachMod) / sizeof(**p.ubApproachMod))
	INJ_I8(D, p.bTown)
	INJ_I8(D, p.bTownAttachment)
	INJ_SKIP(D, 1)
	INJ_U16(D, p.usOptionalGearCost)
	INJ_I8A(D, p.bMercOpinion, lengthof(p.bMercOpinion))
	INJ_I8(D, p.bApproached)
	INJ_I8(D, p.bMercStatus)
	INJ_I8A(D, p.bHatedTime, lengthof(p.bHatedTime))
	INJ_I8(D, p.bLearnToLikeTime)
	INJ_I8(D, p.bLearnToHateTime)
	INJ_I8A(D, p.bHatedCount, lengthof(p.bHatedCount))
	INJ_I8(D, p.bLearnToLikeCount)
	INJ_I8(D, p.bLearnToHateCount)
	INJ_U8(D, p.ubLastDateSpokenTo)
	INJ_U8(D, p.bLastQuoteSaidWasSpecial)
	INJ_I8(D, p.bSectorZ)
	INJ_U16(D, p.usStrategicInsertionData)
	INJ_I8(D, p.bFriendlyOrDirectDefaultResponseUsedRecently)
	INJ_I8(D, p.bRecruitDefaultResponseUsedRecently)
	INJ_I8(D, p.bThreatenDefaultResponseUsedRecently)
	INJ_I8(D, p.bNPCData)
	INJ_I32(D, p.iBalance)
	INJ_SKIP(D, 2)
	INJ_U8(D, p.ubCivilianGroup)
	INJ_U8(D, p.ubNeedForSleep)
	INJ_U32(D, p.uiMoney)
	INJ_I8(D, p.bNPCData2)
	INJ_U8(D, p.ubMiscFlags3)
	INJ_U8(D, p.ubDaysOfMoraleHangover)
	INJ_U8(D, p.ubNumTimesDrugUseInLifetime)
	INJ_U32(D, p.uiPrecedentQuoteSaid)
	UINT32 const checksum = SoldierProfileChecksum(p);
	INJ_U32(D, checksum)
	INJ_I16(D, p.sPreCombatGridNo)
	INJ_U8(D, p.ubTimeTillNextHatedComplaint)
	INJ_U8(D, p.ubSuspiciousDeath)
	INJ_I32(D, p.iMercMercContractLength)
	INJ_U32(D, p.uiTotalCostToDate)
	INJ_SKIP(D, 4)
	Assert(D == Dst + 716);
}


void InjectMercProfileIntoFile(HWFILE const f, MERCPROFILESTRUCT const& p)
{
	BYTE Data[716];
	InjectMercProfile(Data, p);
	FileWrite(f, Data, sizeof(Data));
}


/** Load raw merc profiles.
* @param f Open file with profile data.
* @param numProfiles Number of profiles to load
* @param profiles Array for storing profile data */
void LoadRawMercProfiles(HWFILE const f, int numProfiles, MERCPROFILESTRUCT *profiles, const IEncodingCorrector *fixer)
{
	for (int i = 0; i != numProfiles; ++i)
	{
		BYTE data[MERC_PROFILE_SIZE];
		JA2EncryptedFileRead(f, data, sizeof(data));
		UINT32 checksum;
		ExtractMercProfile(data, profiles[i], false, &checksum, fixer);
		// not checking the checksum
	}
}
