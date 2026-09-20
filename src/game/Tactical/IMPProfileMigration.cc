#include "IMPProfileMigration.h"

#include "Item_Types.h"
#include "Soldier_Control.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace
{

/* An ST::string as a record holds it. The pointer is into the heap of the game
 * that wrote the file and means nothing here; the text is only there when it
 * was short enough to live inside the string itself. */
struct SavedString
{
	uint64_t chars;
	uint64_t size;
	char     text[16];
};
static_assert(sizeof(SavedString) == sizeof(ST::string),
	"a saved string no longer describes the string the game writes");

ST::string Restore(SavedString const& saved)
{
	// Anything longer was on the heap, and the heap was not written down.
	if (saved.size >= sizeof(saved.text)) return ST::string{};
	return ST::string::from_utf8(saved.text, saved.size, ST::substitute_invalid);
}

/* The record as 0.21 wrote it: the palette strings led it, ahead of the
 * character's own particulars. */
struct ProfileV21
{
	SavedString zName;
	SavedString zNickname;
	UINT8 ubFaceIndex;
	SavedString PANTS;
	SavedString VEST;
	SavedString SKIN;
	SavedString HAIR;
	INT8 bSex;
	INT8 bArmourAttractiveness;
	UINT8 ubMiscFlags2;
	INT8 bEvolution;
	UINT8 ubMiscFlags;
	UINT8 bSexist;
	INT8 bLearnToHate;
	UINT8 ubQuoteRecord;
	INT8 bDeathRate;
	INT16 sExpLevelGain;
	INT16 sLifeGain;
	INT16 sAgilityGain;
	INT16 sDexterityGain;
	INT16 sWisdomGain;
	INT16 sMarksmanshipGain;
	INT16 sMedicalGain;
	INT16 sMechanicGain;
	INT16 sExplosivesGain;
	UINT8 ubBodyType;
	INT8 bMedical;
	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;
	UINT32 uiBlinkFrequency;
	UINT32 uiExpressionFrequency;
	SGPSector sSector;
	UINT32 uiDayBecomesAvailable;
	INT8 bStrength;
	INT8 bLifeMax;
	INT8 bExpLevelDelta;
	INT8 bLifeDelta;
	INT8 bAgilityDelta;
	INT8 bDexterityDelta;
	INT8 bWisdomDelta;
	INT8 bMarksmanshipDelta;
	INT8 bMedicalDelta;
	INT8 bMechanicDelta;
	INT8 bExplosivesDelta;
	INT8 bStrengthDelta;
	INT8 bLeadershipDelta;
	UINT16 usKills;
	UINT16 usAssists;
	UINT16 usShotsFired;
	UINT16 usShotsHit;
	UINT16 usBattlesFought;
	UINT16 usTimesWounded;
	UINT16 usTotalDaysServed;
	INT16 sLeadershipGain;
	INT16 sStrengthGain;
	UINT32 uiBodyTypeSubFlags;
	INT16 sSalary;
	INT8 bLife;
	INT8 bDexterity;
	INT8 bPersonalityTrait;
	INT8 bSkillTrait;
	INT8 bReputationTolerance;
	INT8 bExplosive;
	INT8 bSkillTrait2;
	INT8 bLeadership;
	INT8 bBuddy[5];
	INT8 bHated[5];
	INT8 bExpLevel;
	INT8 bMarksmanship;
	INT8 bWisdom;
	UINT8 bInvStatus[19];
	UINT8 bInvNumber[19];
	UINT16 usApproachFactor[4];
	INT8 bMainGunAttractiveness;
	INT8 bAgility;
	BOOLEAN fUseProfileInsertionInfo;
	INT16 sGridNo;
	UINT8 ubQuoteActionID;
	INT8 bMechanical;
	UINT8 ubInvUndroppable;
	UINT8 ubRoomRangeStart[2];
	UINT16 inv[19];
	UINT16 usStatChangeChances[12];
	UINT16 usStatChangeSuccesses[12];
	UINT8 ubStrategicInsertionCode;
	UINT8 ubRoomRangeEnd[2];
	UINT8 ubLastQuoteSaid;
	INT8 bRace;
	INT8 bNationality;
	INT8 bAppearance;
	INT8 bAppearanceCareLevel;
	INT8 bRefinement;
	INT8 bRefinementCareLevel;
	INT8 bHatedNationality;
	INT8 bHatedNationalityCareLevel;
	INT8 bRacist;
	UINT32 uiWeeklySalary;
	UINT32 uiBiWeeklySalary;
	INT8 bMedicalDeposit;
	INT8 bAttitude;
	UINT16 sMedicalDepositAmount;
	INT8 bLearnToLike;
	UINT8 ubApproachVal[4];
	UINT8 ubApproachMod[3][4];
	INT8 bTown;
	INT8 bTownAttachment;
	UINT16 usOptionalGearCost;
	INT8 bMercOpinion[75];
	INT8 bApproached;
	INT8 bMercStatus;
	INT8 bHatedTime[5];
	INT8 bLearnToLikeTime;
	INT8 bLearnToHateTime;
	INT8 bHatedCount[5];
	INT8 bLearnToLikeCount;
	INT8 bLearnToHateCount;
	UINT8 ubLastDateSpokenTo;
	UINT8 bLastQuoteSaidWasSpecial;
	INT8 bSectorZ;
	UINT16 usStrategicInsertionData;
	INT8 bFriendlyOrDirectDefaultResponseUsedRecently;
	INT8 bRecruitDefaultResponseUsedRecently;
	INT8 bThreatenDefaultResponseUsedRecently;
	INT8 bNPCData;
	INT32 iBalance;
	UINT8 ubCivilianGroup;
	UINT8 ubNeedForSleep;
	UINT32 uiMoney;
	INT8 bNPCData2;
	UINT8 ubMiscFlags3;
	UINT8 ubDaysOfMoraleHangover;
	UINT8 ubNumTimesDrugUseInLifetime;
	UINT32 uiPrecedentQuoteSaid;
	INT16 sPreCombatGridNo;
	UINT8 ubTimeTillNextHatedComplaint;
	UINT8 ubSuspiciousDeath;
	INT32 iMercMercContractLength;
	UINT32 uiTotalCostToDate;
};

/* The record as 0.22 wrote it: the portrait fields had arrived and the
 * strings had moved back behind them. */
struct ProfileV22
{
	SavedString zName;
	SavedString zNickname;
	INT8 bSex;
	UINT8 ubCivilianGroup;
	UINT8 ubMiscFlags;
	UINT8 ubMiscFlags2;
	UINT8 ubMiscFlags3;
	UINT8 ubBodyType;
	UINT32 uiBodyTypeSubFlags;
	UINT8 ubFaceIndex;
	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;
	UINT32 uiBlinkFrequency;
	UINT32 uiExpressionFrequency;
	SavedString PANTS;
	SavedString VEST;
	SavedString SKIN;
	SavedString HAIR;
	INT8 bEvolution;
	INT8 bLifeMax;
	INT8 bLife;
	INT8 bAgility;
	INT8 bDexterity;
	INT8 bStrength;
	INT8 bLeadership;
	INT8 bWisdom;
	INT8 bExpLevel;
	INT8 bMarksmanship;
	INT8 bExplosive;
	INT8 bMechanical;
	INT8 bMedical;
	UINT8 ubNeedForSleep;
	INT16 sLifeGain;
	INT8 bLifeDelta;
	INT16 sAgilityGain;
	INT8 bAgilityDelta;
	INT16 sDexterityGain;
	INT8 bDexterityDelta;
	INT16 sStrengthGain;
	INT8 bStrengthDelta;
	INT16 sLeadershipGain;
	INT8 bLeadershipDelta;
	INT16 sWisdomGain;
	INT8 bWisdomDelta;
	INT16 sExpLevelGain;
	INT8 bExpLevelDelta;
	INT16 sMarksmanshipGain;
	INT8 bMarksmanshipDelta;
	INT16 sExplosivesGain;
	INT8 bExplosivesDelta;
	INT16 sMechanicGain;
	INT8 bMechanicDelta;
	INT16 sMedicalGain;
	INT8 bMedicalDelta;
	UINT16 usStatChangeChances[12];
	UINT16 usStatChangeSuccesses[12];
	INT8 bPersonalityTrait;
	INT8 bSkillTrait;
	INT8 bSkillTrait2;
	INT8 bAttitude;
	UINT8 bSexist;
	INT8 bMercStatus;
	INT8 bReputationTolerance;
	INT8 bDeathRate;
	UINT32 uiDayBecomesAvailable;
	INT16 sSalary;
	UINT32 uiWeeklySalary;
	UINT32 uiBiWeeklySalary;
	INT8 bMedicalDeposit;
	UINT16 sMedicalDepositAmount;
	INT32 iMercMercContractLength;
	UINT16 usOptionalGearCost;
	UINT8 ubSuspiciousDeath;
	UINT8 ubDaysOfMoraleHangover;
	SGPSector sSector;
	INT16 sGridNo;
	INT16 sPreCombatGridNo;
	UINT8 ubStrategicInsertionCode;
	UINT16 usStrategicInsertionData;
	BOOLEAN fUseProfileInsertionInfo;
	INT8 bTown;
	INT8 bTownAttachment;
	UINT8 ubRoomRangeStart[2];
	UINT8 ubRoomRangeEnd[2];
	INT8 bBuddy[5];
	INT8 bHated[5];
	INT8 bHatedCount[5];
	INT8 bHatedTime[5];
	INT8 bLearnToLike;
	INT8 bLearnToLikeCount;
	INT8 bLearnToLikeTime;
	INT8 bLearnToHate;
	INT8 bLearnToHateCount;
	INT8 bLearnToHateTime;
	UINT8 ubTimeTillNextHatedComplaint;
	INT8 bMercOpinion[75];
	UINT16 inv[19];
	UINT8 bInvNumber[19];
	UINT8 bInvStatus[19];
	UINT8 ubInvUndroppable;
	UINT32 uiMoney;
	INT8 bArmourAttractiveness;
	INT8 bMainGunAttractiveness;
	INT32 iBalance;
	UINT8 ubNumTimesDrugUseInLifetime;
	INT8 bNPCData;
	INT8 bNPCData2;
	UINT8 ubQuoteRecord;
	UINT8 ubLastQuoteSaid;
	UINT32 uiPrecedentQuoteSaid;
	UINT8 bLastQuoteSaidWasSpecial;
	UINT8 ubLastDateSpokenTo;
	UINT8 ubQuoteActionID;
	INT8 bFriendlyOrDirectDefaultResponseUsedRecently;
	INT8 bRecruitDefaultResponseUsedRecently;
	INT8 bThreatenDefaultResponseUsedRecently;
	INT8 bApproached;
	UINT16 usApproachFactor[4];
	UINT8 ubApproachVal[4];
	UINT8 ubApproachMod[3][4];
	UINT16 usKills;
	UINT16 usAssists;
	UINT16 usShotsFired;
	UINT16 usShotsHit;
	UINT16 usBattlesFought;
	UINT16 usTimesWounded;
	UINT16 usTotalDaysServed;
	UINT32 uiTotalCostToDate;
	INT8 bSectorZ;
	INT8 bRace;
	INT8 bRacist;
	INT8 bNationality;
	INT8 bAppearance;
	INT8 bAppearanceCareLevel;
	INT8 bRefinement;
	INT8 bRefinementCareLevel;
	INT8 bHatedNationality;
	INT8 bHatedNationalityCareLevel;
};

/* The record this version writes, as bytes. The live struct cannot stand in
 * for it: its strings own their storage, and a file holds only what an
 * ST::string keeps inside itself. */
struct ProfileCurrent
{
	SavedString zName;
	SavedString zNickname;
	INT8 bSex;
	UINT8 ubCivilianGroup;
	UINT8 ubMiscFlags;
	UINT8 ubMiscFlags2;
	UINT8 ubMiscFlags3;
	UINT8 ubBodyType;
	UINT32 uiBodyTypeSubFlags;
	UINT8 ubFaceIndex;
	UINT16 usEyesX;
	UINT16 usEyesY;
	UINT16 usMouthX;
	UINT16 usMouthY;
	UINT32 uiBlinkFrequency;
	UINT32 uiExpressionFrequency;
	UINT8 ubVoiceId;
	IMPSlotState impSlotState;
	SavedString PANTS;
	SavedString VEST;
	SavedString SKIN;
	SavedString HAIR;
	INT8 bEvolution;
	INT8 bLifeMax;
	INT8 bLife;
	INT8 bAgility;
	INT8 bDexterity;
	INT8 bStrength;
	INT8 bLeadership;
	INT8 bWisdom;
	INT8 bExpLevel;
	INT8 bMarksmanship;
	INT8 bExplosive;
	INT8 bMechanical;
	INT8 bMedical;
	UINT8 ubNeedForSleep;
	INT16 sLifeGain;
	INT8 bLifeDelta;
	INT16 sAgilityGain;
	INT8 bAgilityDelta;
	INT16 sDexterityGain;
	INT8 bDexterityDelta;
	INT16 sStrengthGain;
	INT8 bStrengthDelta;
	INT16 sLeadershipGain;
	INT8 bLeadershipDelta;
	INT16 sWisdomGain;
	INT8 bWisdomDelta;
	INT16 sExpLevelGain;
	INT8 bExpLevelDelta;
	INT16 sMarksmanshipGain;
	INT8 bMarksmanshipDelta;
	INT16 sExplosivesGain;
	INT8 bExplosivesDelta;
	INT16 sMechanicGain;
	INT8 bMechanicDelta;
	INT16 sMedicalGain;
	INT8 bMedicalDelta;
	UINT16 usStatChangeChances[12];
	UINT16 usStatChangeSuccesses[12];
	INT8 bPersonalityTrait;
	INT8 bSkillTrait;
	INT8 bSkillTrait2;
	INT8 bAttitude;
	UINT8 bSexist;
	INT8 bMercStatus;
	INT8 bReputationTolerance;
	INT8 bDeathRate;
	UINT32 uiDayBecomesAvailable;
	INT16 sSalary;
	UINT32 uiWeeklySalary;
	UINT32 uiBiWeeklySalary;
	INT8 bMedicalDeposit;
	UINT16 sMedicalDepositAmount;
	INT32 iMercMercContractLength;
	UINT16 usOptionalGearCost;
	UINT8 ubSuspiciousDeath;
	UINT8 ubDaysOfMoraleHangover;
	SGPSector sSector;
	INT16 sGridNo;
	INT16 sPreCombatGridNo;
	UINT8 ubStrategicInsertionCode;
	UINT16 usStrategicInsertionData;
	BOOLEAN fUseProfileInsertionInfo;
	INT8 bTown;
	INT8 bTownAttachment;
	UINT8 ubRoomRangeStart[2];
	UINT8 ubRoomRangeEnd[2];
	INT8 bBuddy[5];
	INT8 bHated[5];
	INT8 bHatedCount[5];
	INT8 bHatedTime[5];
	INT8 bLearnToLike;
	INT8 bLearnToLikeCount;
	INT8 bLearnToLikeTime;
	INT8 bLearnToHate;
	INT8 bLearnToHateCount;
	INT8 bLearnToHateTime;
	UINT8 ubTimeTillNextHatedComplaint;
	INT8 bMercOpinion[75];
	UINT16 inv[19];
	UINT8 bInvNumber[19];
	UINT8 bInvStatus[19];
	UINT8 ubInvUndroppable;
	UINT32 uiMoney;
	INT8 bArmourAttractiveness;
	INT8 bMainGunAttractiveness;
	INT32 iBalance;
	UINT8 ubNumTimesDrugUseInLifetime;
	INT8 bNPCData;
	INT8 bNPCData2;
	UINT8 ubQuoteRecord;
	UINT8 ubLastQuoteSaid;
	UINT32 uiPrecedentQuoteSaid;
	UINT8 bLastQuoteSaidWasSpecial;
	UINT8 ubLastDateSpokenTo;
	UINT8 ubQuoteActionID;
	INT8 bFriendlyOrDirectDefaultResponseUsedRecently;
	INT8 bRecruitDefaultResponseUsedRecently;
	INT8 bThreatenDefaultResponseUsedRecently;
	INT8 bApproached;
	UINT16 usApproachFactor[4];
	UINT8 ubApproachVal[4];
	UINT8 ubApproachMod[3][4];
	UINT16 usKills;
	UINT16 usAssists;
	UINT16 usShotsFired;
	UINT16 usShotsHit;
	UINT16 usBattlesFought;
	UINT16 usTimesWounded;
	UINT16 usTotalDaysServed;
	UINT32 uiTotalCostToDate;
	INT8 bSectorZ;
	INT8 bRace;
	INT8 bRacist;
	INT8 bNationality;
	INT8 bAppearance;
	INT8 bAppearanceCareLevel;
	INT8 bRefinement;
	INT8 bRefinementCareLevel;
	INT8 bHatedNationality;
	INT8 bHatedNationalityCareLevel;
};

ProfileV22 ToV22(ProfileV21 const& from)
{
	ProfileV22 to{};
	to.zName = from.zName;
	to.zNickname = from.zNickname;
	to.ubFaceIndex = from.ubFaceIndex;
	to.PANTS = from.PANTS;
	to.VEST = from.VEST;
	to.SKIN = from.SKIN;
	to.HAIR = from.HAIR;
	to.bSex = from.bSex;
	to.bArmourAttractiveness = from.bArmourAttractiveness;
	to.ubMiscFlags2 = from.ubMiscFlags2;
	to.bEvolution = from.bEvolution;
	to.ubMiscFlags = from.ubMiscFlags;
	to.bSexist = from.bSexist;
	to.bLearnToHate = from.bLearnToHate;
	to.ubQuoteRecord = from.ubQuoteRecord;
	to.bDeathRate = from.bDeathRate;
	to.sExpLevelGain = from.sExpLevelGain;
	to.sLifeGain = from.sLifeGain;
	to.sAgilityGain = from.sAgilityGain;
	to.sDexterityGain = from.sDexterityGain;
	to.sWisdomGain = from.sWisdomGain;
	to.sMarksmanshipGain = from.sMarksmanshipGain;
	to.sMedicalGain = from.sMedicalGain;
	to.sMechanicGain = from.sMechanicGain;
	to.sExplosivesGain = from.sExplosivesGain;
	to.ubBodyType = from.ubBodyType;
	to.bMedical = from.bMedical;
	to.usEyesX = from.usEyesX;
	to.usEyesY = from.usEyesY;
	to.usMouthX = from.usMouthX;
	to.usMouthY = from.usMouthY;
	to.uiBlinkFrequency = from.uiBlinkFrequency;
	to.uiExpressionFrequency = from.uiExpressionFrequency;
	to.sSector = from.sSector;
	to.uiDayBecomesAvailable = from.uiDayBecomesAvailable;
	to.bStrength = from.bStrength;
	to.bLifeMax = from.bLifeMax;
	to.bExpLevelDelta = from.bExpLevelDelta;
	to.bLifeDelta = from.bLifeDelta;
	to.bAgilityDelta = from.bAgilityDelta;
	to.bDexterityDelta = from.bDexterityDelta;
	to.bWisdomDelta = from.bWisdomDelta;
	to.bMarksmanshipDelta = from.bMarksmanshipDelta;
	to.bMedicalDelta = from.bMedicalDelta;
	to.bMechanicDelta = from.bMechanicDelta;
	to.bExplosivesDelta = from.bExplosivesDelta;
	to.bStrengthDelta = from.bStrengthDelta;
	to.bLeadershipDelta = from.bLeadershipDelta;
	to.usKills = from.usKills;
	to.usAssists = from.usAssists;
	to.usShotsFired = from.usShotsFired;
	to.usShotsHit = from.usShotsHit;
	to.usBattlesFought = from.usBattlesFought;
	to.usTimesWounded = from.usTimesWounded;
	to.usTotalDaysServed = from.usTotalDaysServed;
	to.sLeadershipGain = from.sLeadershipGain;
	to.sStrengthGain = from.sStrengthGain;
	to.uiBodyTypeSubFlags = from.uiBodyTypeSubFlags;
	to.sSalary = from.sSalary;
	to.bLife = from.bLife;
	to.bDexterity = from.bDexterity;
	to.bPersonalityTrait = from.bPersonalityTrait;
	to.bSkillTrait = from.bSkillTrait;
	to.bReputationTolerance = from.bReputationTolerance;
	to.bExplosive = from.bExplosive;
	to.bSkillTrait2 = from.bSkillTrait2;
	to.bLeadership = from.bLeadership;
	std::memcpy(to.bBuddy, from.bBuddy, sizeof(to.bBuddy));
	std::memcpy(to.bHated, from.bHated, sizeof(to.bHated));
	to.bExpLevel = from.bExpLevel;
	to.bMarksmanship = from.bMarksmanship;
	to.bWisdom = from.bWisdom;
	std::memcpy(to.bInvStatus, from.bInvStatus, sizeof(to.bInvStatus));
	std::memcpy(to.bInvNumber, from.bInvNumber, sizeof(to.bInvNumber));
	std::memcpy(to.usApproachFactor, from.usApproachFactor, sizeof(to.usApproachFactor));
	to.bMainGunAttractiveness = from.bMainGunAttractiveness;
	to.bAgility = from.bAgility;
	to.fUseProfileInsertionInfo = from.fUseProfileInsertionInfo;
	to.sGridNo = from.sGridNo;
	to.ubQuoteActionID = from.ubQuoteActionID;
	to.bMechanical = from.bMechanical;
	to.ubInvUndroppable = from.ubInvUndroppable;
	std::memcpy(to.ubRoomRangeStart, from.ubRoomRangeStart, sizeof(to.ubRoomRangeStart));
	std::memcpy(to.inv, from.inv, sizeof(to.inv));
	std::memcpy(to.usStatChangeChances, from.usStatChangeChances, sizeof(to.usStatChangeChances));
	std::memcpy(to.usStatChangeSuccesses, from.usStatChangeSuccesses, sizeof(to.usStatChangeSuccesses));
	to.ubStrategicInsertionCode = from.ubStrategicInsertionCode;
	std::memcpy(to.ubRoomRangeEnd, from.ubRoomRangeEnd, sizeof(to.ubRoomRangeEnd));
	to.ubLastQuoteSaid = from.ubLastQuoteSaid;
	to.bRace = from.bRace;
	to.bNationality = from.bNationality;
	to.bAppearance = from.bAppearance;
	to.bAppearanceCareLevel = from.bAppearanceCareLevel;
	to.bRefinement = from.bRefinement;
	to.bRefinementCareLevel = from.bRefinementCareLevel;
	to.bHatedNationality = from.bHatedNationality;
	to.bHatedNationalityCareLevel = from.bHatedNationalityCareLevel;
	to.bRacist = from.bRacist;
	to.uiWeeklySalary = from.uiWeeklySalary;
	to.uiBiWeeklySalary = from.uiBiWeeklySalary;
	to.bMedicalDeposit = from.bMedicalDeposit;
	to.bAttitude = from.bAttitude;
	to.sMedicalDepositAmount = from.sMedicalDepositAmount;
	to.bLearnToLike = from.bLearnToLike;
	std::memcpy(to.ubApproachVal, from.ubApproachVal, sizeof(to.ubApproachVal));
	std::memcpy(to.ubApproachMod, from.ubApproachMod, sizeof(to.ubApproachMod));
	to.bTown = from.bTown;
	to.bTownAttachment = from.bTownAttachment;
	to.usOptionalGearCost = from.usOptionalGearCost;
	std::memcpy(to.bMercOpinion, from.bMercOpinion, sizeof(to.bMercOpinion));
	to.bApproached = from.bApproached;
	to.bMercStatus = from.bMercStatus;
	std::memcpy(to.bHatedTime, from.bHatedTime, sizeof(to.bHatedTime));
	to.bLearnToLikeTime = from.bLearnToLikeTime;
	to.bLearnToHateTime = from.bLearnToHateTime;
	std::memcpy(to.bHatedCount, from.bHatedCount, sizeof(to.bHatedCount));
	to.bLearnToLikeCount = from.bLearnToLikeCount;
	to.bLearnToHateCount = from.bLearnToHateCount;
	to.ubLastDateSpokenTo = from.ubLastDateSpokenTo;
	to.bLastQuoteSaidWasSpecial = from.bLastQuoteSaidWasSpecial;
	to.bSectorZ = from.bSectorZ;
	to.usStrategicInsertionData = from.usStrategicInsertionData;
	to.bFriendlyOrDirectDefaultResponseUsedRecently = from.bFriendlyOrDirectDefaultResponseUsedRecently;
	to.bRecruitDefaultResponseUsedRecently = from.bRecruitDefaultResponseUsedRecently;
	to.bThreatenDefaultResponseUsedRecently = from.bThreatenDefaultResponseUsedRecently;
	to.bNPCData = from.bNPCData;
	to.iBalance = from.iBalance;
	to.ubCivilianGroup = from.ubCivilianGroup;
	to.ubNeedForSleep = from.ubNeedForSleep;
	to.uiMoney = from.uiMoney;
	to.bNPCData2 = from.bNPCData2;
	to.ubMiscFlags3 = from.ubMiscFlags3;
	to.ubDaysOfMoraleHangover = from.ubDaysOfMoraleHangover;
	to.ubNumTimesDrugUseInLifetime = from.ubNumTimesDrugUseInLifetime;
	to.uiPrecedentQuoteSaid = from.uiPrecedentQuoteSaid;
	to.sPreCombatGridNo = from.sPreCombatGridNo;
	to.ubTimeTillNextHatedComplaint = from.ubTimeTillNextHatedComplaint;
	to.ubSuspiciousDeath = from.ubSuspiciousDeath;
	to.iMercMercContractLength = from.iMercMercContractLength;
	to.uiTotalCostToDate = from.uiTotalCostToDate;
	return to;
}

MERCPROFILESTRUCT ToProfile(ProfileCurrent const& from)
{
	MERCPROFILESTRUCT to{};
	to.zName = Restore(from.zName);
	to.zNickname = Restore(from.zNickname);
	to.bSex = from.bSex;
	to.ubCivilianGroup = from.ubCivilianGroup;
	to.ubMiscFlags = from.ubMiscFlags;
	to.ubMiscFlags2 = from.ubMiscFlags2;
	to.ubMiscFlags3 = from.ubMiscFlags3;
	to.ubBodyType = from.ubBodyType;
	to.uiBodyTypeSubFlags = from.uiBodyTypeSubFlags;
	to.ubFaceIndex = from.ubFaceIndex;
	to.usEyesX = from.usEyesX;
	to.usEyesY = from.usEyesY;
	to.usMouthX = from.usMouthX;
	to.usMouthY = from.usMouthY;
	to.uiBlinkFrequency = from.uiBlinkFrequency;
	to.uiExpressionFrequency = from.uiExpressionFrequency;
	to.ubVoiceId = from.ubVoiceId;
	to.impSlotState = from.impSlotState;
	to.PANTS = Restore(from.PANTS);
	to.VEST = Restore(from.VEST);
	to.SKIN = Restore(from.SKIN);
	to.HAIR = Restore(from.HAIR);
	to.bEvolution = from.bEvolution;
	to.bLifeMax = from.bLifeMax;
	to.bLife = from.bLife;
	to.bAgility = from.bAgility;
	to.bDexterity = from.bDexterity;
	to.bStrength = from.bStrength;
	to.bLeadership = from.bLeadership;
	to.bWisdom = from.bWisdom;
	to.bExpLevel = from.bExpLevel;
	to.bMarksmanship = from.bMarksmanship;
	to.bExplosive = from.bExplosive;
	to.bMechanical = from.bMechanical;
	to.bMedical = from.bMedical;
	to.ubNeedForSleep = from.ubNeedForSleep;
	to.sLifeGain = from.sLifeGain;
	to.bLifeDelta = from.bLifeDelta;
	to.sAgilityGain = from.sAgilityGain;
	to.bAgilityDelta = from.bAgilityDelta;
	to.sDexterityGain = from.sDexterityGain;
	to.bDexterityDelta = from.bDexterityDelta;
	to.sStrengthGain = from.sStrengthGain;
	to.bStrengthDelta = from.bStrengthDelta;
	to.sLeadershipGain = from.sLeadershipGain;
	to.bLeadershipDelta = from.bLeadershipDelta;
	to.sWisdomGain = from.sWisdomGain;
	to.bWisdomDelta = from.bWisdomDelta;
	to.sExpLevelGain = from.sExpLevelGain;
	to.bExpLevelDelta = from.bExpLevelDelta;
	to.sMarksmanshipGain = from.sMarksmanshipGain;
	to.bMarksmanshipDelta = from.bMarksmanshipDelta;
	to.sExplosivesGain = from.sExplosivesGain;
	to.bExplosivesDelta = from.bExplosivesDelta;
	to.sMechanicGain = from.sMechanicGain;
	to.bMechanicDelta = from.bMechanicDelta;
	to.sMedicalGain = from.sMedicalGain;
	to.bMedicalDelta = from.bMedicalDelta;
	std::memcpy(to.usStatChangeChances, from.usStatChangeChances, sizeof(to.usStatChangeChances));
	std::memcpy(to.usStatChangeSuccesses, from.usStatChangeSuccesses, sizeof(to.usStatChangeSuccesses));
	to.bPersonalityTrait = from.bPersonalityTrait;
	to.bSkillTrait = from.bSkillTrait;
	to.bSkillTrait2 = from.bSkillTrait2;
	to.bAttitude = from.bAttitude;
	to.bSexist = from.bSexist;
	to.bMercStatus = from.bMercStatus;
	to.bReputationTolerance = from.bReputationTolerance;
	to.bDeathRate = from.bDeathRate;
	to.uiDayBecomesAvailable = from.uiDayBecomesAvailable;
	to.sSalary = from.sSalary;
	to.uiWeeklySalary = from.uiWeeklySalary;
	to.uiBiWeeklySalary = from.uiBiWeeklySalary;
	to.bMedicalDeposit = from.bMedicalDeposit;
	to.sMedicalDepositAmount = from.sMedicalDepositAmount;
	to.iMercMercContractLength = from.iMercMercContractLength;
	to.usOptionalGearCost = from.usOptionalGearCost;
	to.ubSuspiciousDeath = from.ubSuspiciousDeath;
	to.ubDaysOfMoraleHangover = from.ubDaysOfMoraleHangover;
	to.sSector = from.sSector;
	to.sGridNo = from.sGridNo;
	to.sPreCombatGridNo = from.sPreCombatGridNo;
	to.ubStrategicInsertionCode = from.ubStrategicInsertionCode;
	to.usStrategicInsertionData = from.usStrategicInsertionData;
	to.fUseProfileInsertionInfo = from.fUseProfileInsertionInfo;
	to.bTown = from.bTown;
	to.bTownAttachment = from.bTownAttachment;
	std::memcpy(to.ubRoomRangeStart, from.ubRoomRangeStart, sizeof(to.ubRoomRangeStart));
	std::memcpy(to.ubRoomRangeEnd, from.ubRoomRangeEnd, sizeof(to.ubRoomRangeEnd));
	std::memcpy(to.bBuddy, from.bBuddy, sizeof(to.bBuddy));
	std::memcpy(to.bHated, from.bHated, sizeof(to.bHated));
	std::memcpy(to.bHatedCount, from.bHatedCount, sizeof(to.bHatedCount));
	std::memcpy(to.bHatedTime, from.bHatedTime, sizeof(to.bHatedTime));
	to.bLearnToLike = from.bLearnToLike;
	to.bLearnToLikeCount = from.bLearnToLikeCount;
	to.bLearnToLikeTime = from.bLearnToLikeTime;
	to.bLearnToHate = from.bLearnToHate;
	to.bLearnToHateCount = from.bLearnToHateCount;
	to.bLearnToHateTime = from.bLearnToHateTime;
	to.ubTimeTillNextHatedComplaint = from.ubTimeTillNextHatedComplaint;
	std::memcpy(to.bMercOpinion, from.bMercOpinion, sizeof(to.bMercOpinion));
	std::memcpy(to.inv, from.inv, sizeof(to.inv));
	std::memcpy(to.bInvNumber, from.bInvNumber, sizeof(to.bInvNumber));
	std::memcpy(to.bInvStatus, from.bInvStatus, sizeof(to.bInvStatus));
	to.ubInvUndroppable = from.ubInvUndroppable;
	to.uiMoney = from.uiMoney;
	to.bArmourAttractiveness = from.bArmourAttractiveness;
	to.bMainGunAttractiveness = from.bMainGunAttractiveness;
	to.iBalance = from.iBalance;
	to.ubNumTimesDrugUseInLifetime = from.ubNumTimesDrugUseInLifetime;
	to.bNPCData = from.bNPCData;
	to.bNPCData2 = from.bNPCData2;
	to.ubQuoteRecord = from.ubQuoteRecord;
	to.ubLastQuoteSaid = from.ubLastQuoteSaid;
	to.uiPrecedentQuoteSaid = from.uiPrecedentQuoteSaid;
	to.bLastQuoteSaidWasSpecial = from.bLastQuoteSaidWasSpecial;
	to.ubLastDateSpokenTo = from.ubLastDateSpokenTo;
	to.ubQuoteActionID = from.ubQuoteActionID;
	to.bFriendlyOrDirectDefaultResponseUsedRecently = from.bFriendlyOrDirectDefaultResponseUsedRecently;
	to.bRecruitDefaultResponseUsedRecently = from.bRecruitDefaultResponseUsedRecently;
	to.bThreatenDefaultResponseUsedRecently = from.bThreatenDefaultResponseUsedRecently;
	to.bApproached = from.bApproached;
	std::memcpy(to.usApproachFactor, from.usApproachFactor, sizeof(to.usApproachFactor));
	std::memcpy(to.ubApproachVal, from.ubApproachVal, sizeof(to.ubApproachVal));
	std::memcpy(to.ubApproachMod, from.ubApproachMod, sizeof(to.ubApproachMod));
	to.usKills = from.usKills;
	to.usAssists = from.usAssists;
	to.usShotsFired = from.usShotsFired;
	to.usShotsHit = from.usShotsHit;
	to.usBattlesFought = from.usBattlesFought;
	to.usTimesWounded = from.usTimesWounded;
	to.usTotalDaysServed = from.usTotalDaysServed;
	to.uiTotalCostToDate = from.uiTotalCostToDate;
	to.bSectorZ = from.bSectorZ;
	to.bRace = from.bRace;
	to.bRacist = from.bRacist;
	to.bNationality = from.bNationality;
	to.bAppearance = from.bAppearance;
	to.bAppearanceCareLevel = from.bAppearanceCareLevel;
	to.bRefinement = from.bRefinement;
	to.bRefinementCareLevel = from.bRefinementCareLevel;
	to.bHatedNationality = from.bHatedNationality;
	to.bHatedNationalityCareLevel = from.bHatedNationalityCareLevel;
	return to;
}
/* The length each record had when it was written is what a file of that era
 * is known by, so these are the load-bearing numbers of the whole business. */
static_assert(sizeof(ProfileV21) == 648);
static_assert(sizeof(ProfileV22) == 664);
static_assert(sizeof(ProfileCurrent) == sizeof(MERCPROFILESTRUCT),
	"the record written today is no longer the struct it is written from");
static_assert(offsetof(ProfileV21, PANTS) == 72);

/* 0.22 moved nothing that came after it: the voice id and the slot state were
 * put in where the palette strings used to begin, and everything from there on
 * was pushed down by as much. */
static_assert(offsetof(ProfileV22, PANTS) == 96);
static_assert(offsetof(ProfileCurrent, ubVoiceId) == 96);
static_assert(offsetof(ProfileCurrent, PANTS) == 104);
static_assert(sizeof(ProfileCurrent) - sizeof(ProfileV22) ==
	offsetof(ProfileCurrent, PANTS) - offsetof(ProfileV22, PANTS));

ProfileCurrent ToCurrent(ProfileV22 const& from)
{
	ProfileCurrent to{};
	BYTE const* const src = reinterpret_cast<BYTE const*>(&from);
	BYTE* const dst = reinterpret_cast<BYTE*>(&to);

	std::memcpy(dst, src, offsetof(ProfileV22, PANTS));
	std::memcpy(dst + offsetof(ProfileCurrent, PANTS), src + offsetof(ProfileV22, PANTS),
		sizeof(ProfileV22) - offsetof(ProfileV22, PANTS));

	/* A character from before the I.M.P. slots spoke with the files named
	 * after the profile it sat in, which is the slot it is about to be put
	 * back into, and it holds no slot until the player confirms it. */
	to.ubVoiceId = 0;
	to.impSlotState = IMPSlotState::FREE;
	return to;
}

}

std::optional<IMPProfileLayout> IMPProfileLayoutOfVersion(UINT32 const version)
{
	/* A versioned file is the version, then the record, then the inventory. */
	auto const of = [](IMPProfileFormat format, size_t record)
		-> std::optional<IMPProfileLayout>
	{
		size_t const lead = sizeof(UINT32);
		return IMPProfileLayout{ format, lead, record, lead + record };
	};

	switch (version)
	{
		case 1: return of(IMPProfileFormat::ProfileVersion1, sizeof(ProfileCurrent));
	}
	return std::nullopt;
}

/* Every version up to the one this build writes must be named above. Raising
 * IMP_PROFILE_VERSION without giving it a layout gives way here. */
static_assert(IMP_PROFILE_VERSION == 1,
	"A profile version has been raised. Give it a case in "
	"IMPProfileLayoutOfVersion, freeze the record the version before it wrote "
	"as a struct of its own, and name that struct's layout in "
	"IMPProfileFormat.");

std::optional<IMPProfileLayout> IMPProfileVersionlessLayoutOfSize(size_t const fileSize)
{
	size_t const inventory = sizeof(OBJECTTYPE) * NUM_INV_SLOTS;

	/* A versionless file is the record and the inventory and nothing else, so
	 * the record begins where the file does. */
	auto const of = [&](IMPProfileFormat format, size_t record)
		-> std::optional<IMPProfileLayout>
	{
		if (fileSize != record + inventory) return std::nullopt;
		return IMPProfileLayout{ format, 0, record, record };
	};

	if (auto l = of(IMPProfileFormat::SaveVersion104, sizeof(ProfileCurrent))) return l;
	if (auto l = of(IMPProfileFormat::Release022, sizeof(ProfileV22))) return l;
	if (auto l = of(IMPProfileFormat::Release021, sizeof(ProfileV21))) return l;
	return std::nullopt;
}

MERCPROFILESTRUCT IMPProfileMigrate(IMPProfileFormat const format, BYTE const* const record)
{
	switch (format)
	{
		case IMPProfileFormat::Release021:
		{
			ProfileV21 v21;
			std::memcpy(&v21, record, sizeof(v21));
			return ToProfile(ToCurrent(ToV22(v21)));
		}
		case IMPProfileFormat::Release022:
		{
			ProfileV22 v22;
			std::memcpy(&v22, record, sizeof(v22));
			return ToProfile(ToCurrent(v22));
		}
		case IMPProfileFormat::SaveVersion104:
		case IMPProfileFormat::ProfileVersion1:
		{
			ProfileCurrent current;
			std::memcpy(&current, record, sizeof(current));
			return ToProfile(current);
		}
	}
	return MERCPROFILESTRUCT{};
}
