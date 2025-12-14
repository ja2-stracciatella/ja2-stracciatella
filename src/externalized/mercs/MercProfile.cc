#include "TranslatableString.h"
#include "enums.h"
#include "Exceptions.h"
#include "MercProfile.h"
#include "MercProfileInfo.h"
#include "Soldier_Profile_Type.h"
#include "TownModel.h"
#include <memory>

MercProfile::MercProfile(ProfileID profileID) : m_profileID(profileID)
{
	if (m_profileID >= NUM_PROFILES)
	{
		ST::string err = ST::format("invalid m_profileID '{}'", m_profileID);
		throw std::out_of_range(err.c_str());
	}
	m_profile = &gMercProfiles[profileID];
}

MercProfile::MercProfile(const MercProfileInfo* info)
	: MercProfile(info->profileID)
{
}

MercProfile::MercProfile(MERCPROFILESTRUCT& p)
	: MercProfile(static_cast<ProfileID>(&p - gMercProfiles))
{
}

ProfileID MercProfile::getID() const
{
	return m_profileID;
}

bool MercProfile::isRecruited() const
{
	return m_profile->ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED;
}

bool MercProfile::isForcedNPCQuote() const
{
	return m_profile->ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE;
}

const MercProfileInfo& MercProfile::getInfo() const
{
	return *(MercProfileInfo::load(m_profileID));
}

bool MercProfile::isAIMMerc() const
{
	return getInfo().mercType == MercType::AIM;
}

bool MercProfile::isMERCMerc() const
{
	return getInfo().mercType == MercType::MERC;
}

bool MercProfile::isIMPMerc() const
{
	return getInfo().mercType == MercType::IMP;
}

bool MercProfile::isRPC() const
{
	return getInfo().mercType == MercType::RPC;
}
bool MercProfile::isNPC() const
{
	return getInfo().mercType == MercType::NPC;
}

bool MercProfile::isNPCorRPC() const
{
	return isNPC() || isRPC();
}

bool MercProfile::isVehicle() const
{
	return getInfo().mercType == MercType::VEHICLE;
}

bool MercProfile::isPlayerMerc() const
{
	return isAIMMerc() || isMERCMerc() || isIMPMerc();
}

MERCPROFILESTRUCT &MercProfile::getStruct() const
{
	return *m_profile;
}

MercProfile::operator MERCPROFILESTRUCT&() const
{
	return *m_profile;
}

void MercProfile::deserializeStructRelations(const MERCPROFILESTRUCT* binaryProf, MERCPROFILESTRUCT* prof, const JsonObject& json, const ContentManager* contentManager)
{
	JsonObject jOpinions[NUM_RECRUITABLE];
	if (json.has("relations")) {
		for (auto& element : json["relations"].toVec()) {
			JsonObject r = element.toObject();
			ProfileID idx = contentManager->getMercProfileInfoByName(r.GetString("target"))->profileID;
			if (idx >= NUM_RECRUITABLE) {
				throw DataError(ST::format("{}'s profile id must be less than {}", r.GetString("target"), NUM_RECRUITABLE));
			}
			jOpinions[idx] = std::move(r);
		}
	}
	for (ProfileID idx = 0; idx < NUM_RECRUITABLE; idx++) {
		prof->bMercOpinion[idx] = jOpinions[idx].getOptionalInt("opinion", binaryProf->bMercOpinion[idx]);
		if (jOpinions[idx].getOptionalBool("friend1", binaryProf->bBuddy[BUDDY_SLOT1] == idx)) {
			prof->bBuddy[BUDDY_SLOT1] = idx;
		}
		else if (jOpinions[idx].getOptionalBool("friend2", binaryProf->bBuddy[BUDDY_SLOT2] == idx)) {
			prof->bBuddy[BUDDY_SLOT2] = idx;
		}
		else if (jOpinions[idx].getOptionalBool("eventualFriend", binaryProf->bLearnToLike == idx)) {
			prof->bLearnToLike = idx;
			prof->bLearnToLikeTime = jOpinions[idx].getOptionalUInt("resistanceToBefriending", binaryProf->bLearnToLikeTime);
		}
		else if (jOpinions[idx].getOptionalBool("enemy1", binaryProf->bHated[HATED_SLOT1] == idx)) {
			prof->bHated[HATED_SLOT1] = idx;
			prof->bHatedTime[HATED_SLOT1] = jOpinions[idx].getOptionalUInt("tolerance", binaryProf->bHatedTime[HATED_SLOT1]);
		}
		else if (jOpinions[idx].getOptionalBool("enemy2", binaryProf->bHated[HATED_SLOT2] == idx)) {
			prof->bHated[HATED_SLOT2] = idx;
			prof->bHatedTime[HATED_SLOT2] = jOpinions[idx].getOptionalUInt("tolerance", binaryProf->bHatedTime[HATED_SLOT2]);
		}
		else if (jOpinions[idx].getOptionalBool("eventualEnemy", binaryProf->bLearnToHate == idx)) {
			prof->bLearnToHate = idx;
			prof->bLearnToHateTime = jOpinions[idx].getOptionalUInt("resistanceToMakingEnemy", binaryProf->bLearnToHateTime);
		}
	}
}

std::unique_ptr<MERCPROFILESTRUCT> MercProfile::deserializeStruct(const MERCPROFILESTRUCT* binaryProf, TranslatableString::Loader& stringLoader, const JsonObject& r, const ContentManager* contentManager)
{
	std::unique_ptr<MERCPROFILESTRUCT> prof = std::make_unique<MERCPROFILESTRUCT>();

	prof->zName = TranslatableString::Utils::resolveOptionalProperty(stringLoader, r, "fullName", std::make_unique<TranslatableString::Untranslated>(binaryProf->zName));
	prof->zNickname = TranslatableString::Utils::resolveOptionalProperty(stringLoader, r, "nickname", std::make_unique<TranslatableString::Untranslated>(binaryProf->zNickname));

	if (r.has("sex")) {
		prof->bSex = r.GetString("sex") == "F" ? Sexes::FEMALE : Sexes::MALE;
	}
	else {
		prof->bSex = binaryProf->bSex;
	}

	auto stringForEnum{ r.getOptionalString("civilianGroup") };
	prof->ubCivilianGroup = stringForEnum.empty() ? binaryProf->ubCivilianGroup : Internals::getCivilianGroupEnumFromString(stringForEnum);

	if (r.getOptionalBool("isGoodGuy", binaryProf->ubMiscFlags3 & PROFILE_MISC_FLAG3_GOODGUY)) {
		prof->ubMiscFlags3 |= PROFILE_MISC_FLAG3_GOODGUY;
	}

	stringForEnum = r.getOptionalString("bodyType");
	prof->ubBodyType = stringForEnum.empty() ? binaryProf->ubBodyType : Internals::getBodyTypeEnumFromString(stringForEnum);

	ST::string jAnimFlag = r.getOptionalString("bodyTypeSubstitution");
	if (jAnimFlag.empty()) {
		prof->uiBodyTypeSubFlags = binaryProf->uiBodyTypeSubFlags;
	}
	else if (jAnimFlag == "SUB_ANIM_BIGGUYSHOOT2") {
		prof->uiBodyTypeSubFlags |= SUB_ANIM_BIGGUYSHOOT2;
	}
	else if (jAnimFlag == "SUB_ANIM_BIGGUYTHREATENSTANCE") {
		prof->uiBodyTypeSubFlags |= SUB_ANIM_BIGGUYTHREATENSTANCE;
	}

	JsonObject portrait;
	if (r.has("face")) {
		portrait = r["face"].toObject();
		std::vector<JsonValue> eyesXY = portrait["eyesXY"].toVec();
		std::vector<JsonValue> mouthXY = portrait["mouthXY"].toVec();
		prof->usEyesX = eyesXY[0].toInt();
		prof->usEyesY = eyesXY[1].toInt();
		prof->usMouthX = mouthXY[0].toInt();
		prof->usMouthY = mouthXY[1].toInt();
	}
	else {
		prof->usEyesX = binaryProf->usEyesX;
		prof->usEyesY = binaryProf->usEyesY;
		prof->usMouthX = binaryProf->usMouthX;
		prof->usMouthY = binaryProf->usMouthY;
	}
	prof->uiBlinkFrequency = portrait.getOptionalUInt("blinkFrequency", binaryProf->uiBlinkFrequency);
	prof->uiExpressionFrequency = portrait.getOptionalUInt("expressionFrequency", binaryProf->uiExpressionFrequency);

	prof->SKIN = r.getOptionalString("skinColor", binaryProf->SKIN);
	prof->HAIR = r.getOptionalString("hairColor", binaryProf->HAIR);
	prof->VEST = r.getOptionalString("vestColor", binaryProf->VEST);
	prof->PANTS = r.getOptionalString("pantsColor", binaryProf->PANTS);

	ST::string jSexismMode = r.getOptionalString("sexismMode");
	if (jSexismMode.empty()) {
		prof->bSexist = binaryProf->bSexist;
	}
	else if (jSexismMode == "GENTLEMAN") {
		prof->bSexist = SexistLevels::GENTLEMAN;
	}
	else if (jSexismMode == "SOMEWHAT_SEXIST") {
		prof->bSexist = SexistLevels::SOMEWHAT_SEXIST;
	}
	else if (jSexismMode == "VERY_SEXIST") {
		prof->bSexist = SexistLevels::VERY_SEXIST;
	}

	JsonObject stats;
	if (r.has("stats")) {
		stats = r["stats"].toObject();
	}
	ST::string jEvolution = stats.getOptionalString("evolution");
	if (jEvolution == "NONE") {
		prof->bEvolution = CharacterEvolution::NO_EVOLUTION;
	}
	else if (jEvolution == "REVERSED") {
		prof->bEvolution = CharacterEvolution::DEVOLVE;
	}
	prof->bLifeMax = stats.getOptionalUInt("health", binaryProf->bLifeMax);
	prof->bLife = stats.getOptionalUInt("health", binaryProf->bLife);
	prof->bAgility = stats.getOptionalUInt("agility", binaryProf->bAgility);
	prof->bDexterity = stats.getOptionalUInt("dexterity", binaryProf->bDexterity);
	prof->bStrength = stats.getOptionalUInt("strength", binaryProf->bStrength);
	prof->bLeadership = stats.getOptionalUInt("leadership", binaryProf->bLeadership);
	prof->bWisdom = stats.getOptionalUInt("wisdom", binaryProf->bWisdom);
	prof->bExpLevel = stats.getOptionalUInt("experience", binaryProf->bExpLevel);
	prof->bMarksmanship = stats.getOptionalUInt("marksmanship", binaryProf->bMarksmanship);
	prof->bMedical = stats.getOptionalUInt("medical", binaryProf->bMedical);
	prof->bExplosive = stats.getOptionalUInt("explosive", binaryProf->bExplosive);
	prof->bMechanical = stats.getOptionalUInt("mechanical", binaryProf->bMechanical);
	prof->ubNeedForSleep = stats.getOptionalUInt("sleepiness", binaryProf->ubNeedForSleep);

	stringForEnum = r.getOptionalString("personalityTrait");
	prof->bPersonalityTrait = stringForEnum.empty() ? binaryProf->bPersonalityTrait : Internals::getPersonalityTraitEnumFromString(stringForEnum);

	stringForEnum = r.getOptionalString("skillTrait");
	prof->bSkillTrait = stringForEnum.empty() ? binaryProf->bSkillTrait : Internals::getSkillTraitEnumFromString(stringForEnum);

	stringForEnum = r.getOptionalString("skillTrait2");
	prof->bSkillTrait2 = stringForEnum.empty() ? binaryProf->bSkillTrait2 : Internals::getSkillTraitEnumFromString(stringForEnum);

	stringForEnum = r.getOptionalString("attitude");
	prof->bAttitude = stringForEnum.empty() ? binaryProf->bAttitude : Internals::getAttitudeEnumFromString(stringForEnum);

	ST::string jSector = r.getOptionalString("sector");
	if (jSector.empty()) {
		prof->sSector = binaryProf->sSector;
	}
	else {
		std::vector<ST::string> jLong = jSector.split("-");
		ST::string jShort = jLong[0];
		prof->sSector = SGPSector::FromShortString(jShort);
		if (jLong.size() == 2) {
			prof->sSector.z = jLong[1].to_int();
		}
	}

	ST::string jTown = r.getOptionalString("town");
	if (jTown.empty()) {
		prof->bTown = binaryProf->bTown;
	}
	else {
		prof->bTown = contentManager->getTownByName(jTown)->townId;
	}

	prof->bTownAttachment = r.getOptionalUInt("townAttachment", binaryProf->bTownAttachment);

	if (r.getOptionalBool("isTownIndifferentIfDead", binaryProf->ubMiscFlags3 & PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH)) {
		prof->ubMiscFlags3 |= PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH;
	}

	if (r.has("ownedRooms")) {
		JsonObject rooms = r["ownedRooms"].toObject();
		if (rooms.has("range1")) {
			std::vector<JsonValue> range1 = rooms["range1"].toVec();
			prof->ubRoomRangeStart[0] = range1[0].toInt();
			prof->ubRoomRangeEnd[0] = range1[1].toInt();
		}
		else {
			prof->ubRoomRangeStart[0] = binaryProf->ubRoomRangeStart[0];
			prof->ubRoomRangeEnd[0] = binaryProf->ubRoomRangeEnd[0];
		}
		if (rooms.has("range2")) {
			std::vector<JsonValue> range2 = rooms["range2"].toVec();
			prof->ubRoomRangeStart[1] = range2[0].toInt();
			prof->ubRoomRangeEnd[1] = range2[1].toInt();
		}
		else {
			prof->ubRoomRangeStart[1] = binaryProf->ubRoomRangeStart[1];
			prof->ubRoomRangeEnd[1] = binaryProf->ubRoomRangeEnd[1];
		}
	}
	else {
		prof->ubRoomRangeStart[0] = binaryProf->ubRoomRangeStart[0];
		prof->ubRoomRangeEnd[0] = binaryProf->ubRoomRangeEnd[0];
		prof->ubRoomRangeStart[1] = binaryProf->ubRoomRangeStart[1];
		prof->ubRoomRangeEnd[1] = binaryProf->ubRoomRangeEnd[1];
	}

	prof->bReputationTolerance = r.getOptionalUInt("toleranceForPlayersReputation", binaryProf->bReputationTolerance);
	prof->bDeathRate = r.getOptionalUInt("toleranceForPlayersDeathRate", binaryProf->bDeathRate);

	JsonObject c;
	if (r.has("contract")) {
		c = r["contract"].toObject();
	}
	prof->sSalary = c.getOptionalUInt("dailySalary", binaryProf->sSalary);
	prof->uiWeeklySalary = c.getOptionalUInt("weeklySalary", binaryProf->uiWeeklySalary);
	prof->uiBiWeeklySalary = c.getOptionalUInt("biWeeklySalary", binaryProf->uiBiWeeklySalary);
	prof->bMedicalDeposit = c.getOptionalBool("isMedicalDepositRequired", binaryProf->bMedicalDeposit);

	if (r.has("inventory")) {
		int slotIdx = 0;
		for (auto& element : r.GetValue("inventory").toVec()) {
			JsonObject jSlot = element.toObject();
			if (jSlot.has("slot")) {
				slotIdx = Internals::getInventorySlotEnumFromString(jSlot.GetString("slot"));
			}
			ST::string itemName = jSlot.GetString("item");
			prof->inv[slotIdx] = contentManager->getItemByName(itemName)->getItemIndex();
			prof->bInvNumber[slotIdx] = jSlot.getOptionalUInt("quantity", 1);
			prof->bInvStatus[slotIdx] = jSlot.getOptionalUInt("status", 100);
			if (jSlot.getOptionalBool("isUndroppable")) {
				prof->ubInvUndroppable |= gubItemDroppableFlag[slotIdx];
			}
			slotIdx++;
		}
	} else {
		std::copy(std::begin(binaryProf->inv), std::end(binaryProf->inv), std::begin(prof->inv));
		std::copy(std::begin(binaryProf->bInvNumber), std::end(binaryProf->bInvNumber), std::begin(prof->bInvNumber));
		std::copy(std::begin(binaryProf->bInvStatus), std::end(binaryProf->bInvStatus), std::begin(prof->bInvStatus));
		prof->ubInvUndroppable = binaryProf->ubInvUndroppable;
	}

	prof->uiMoney = r.getOptionalUInt("money", binaryProf->uiMoney);

	JsonObject jApproaches[APPROACH_RECRUIT];
	if (r.has("dialogue")) {
		for (auto& element : r.GetValue("dialogue").toVec()) {
			JsonObject jAppr = element.toObject();
			Approach apprIdx = Internals::getApproachEnumFromString(jAppr.GetString("approach"));
			jApproaches[apprIdx - 1] = std::move(jAppr);
		}
	}
	for (size_t apprIdx = 0; apprIdx < lengthof(jApproaches); apprIdx++) {
		prof->usApproachFactor[apprIdx] = jApproaches[apprIdx].getOptionalUInt("effectiveness", binaryProf->usApproachFactor[apprIdx]);
		prof->ubApproachVal[apprIdx] = jApproaches[apprIdx].getOptionalUInt("desireToTalk", binaryProf->ubApproachVal[apprIdx]);
		prof->ubApproachMod[APPROACH_FRIENDLY - 1][apprIdx] = jApproaches[apprIdx].getOptionalUInt("friendlyMod", binaryProf->ubApproachMod[APPROACH_FRIENDLY - 1][apprIdx]);
		prof->ubApproachMod[APPROACH_DIRECT - 1][apprIdx] = jApproaches[apprIdx].getOptionalUInt("directMod", binaryProf->ubApproachMod[APPROACH_DIRECT - 1][apprIdx]);
		prof->ubApproachMod[APPROACH_THREATEN - 1][apprIdx] = jApproaches[apprIdx].getOptionalUInt("threatenMod", binaryProf->ubApproachMod[APPROACH_THREATEN - 1][apprIdx]);
	}

	std::copy(std::begin(binaryProf->bMercOpinion), std::end(binaryProf->bMercOpinion), std::begin(prof->bMercOpinion));
	std::copy(std::begin(binaryProf->bBuddy), std::end(binaryProf->bBuddy), std::begin(prof->bBuddy));
	std::copy(std::begin(binaryProf->bHated), std::end(binaryProf->bHated), std::begin(prof->bHated));
	prof->bLearnToLike = binaryProf->bLearnToLike;
	prof->bLearnToLikeTime = binaryProf->bLearnToLikeTime;
	prof->bLearnToHate = binaryProf->bLearnToHate;
	prof->bLearnToHateTime = binaryProf->bLearnToHateTime;

	return prof;
}

JsonValue MercProfile::serializeStruct(const ContentManager* contentManager) const
{
	MERCPROFILESTRUCT* prof = this->m_profile;
	JsonObject obj;
	// triple digits prepended to field names is a workaround to sort them in convenient order instead of alphabetically
	obj.set("000profileID", this->getInfo().profileID);
	obj.set("001internalName", this->getInfo().internalName);
	obj.set("002type", Internals::getMercTypeName(static_cast<MercType>(this->getInfo().mercType)));
	if (!prof->zName.empty()) {
		obj.set("003fullName", prof->zName);
	}
	if (!prof->zNickname.empty()) {
		obj.set("004nickname", prof->zNickname);
	}
	obj.set("005sex", prof->bSex == Sexes::MALE ? "M" : "F");
	if (prof->ubCivilianGroup > 0) {
		obj.set("006civilianGroup", Internals::getCivilianGroupName(static_cast<CivilianGroup>(prof->ubCivilianGroup)));
	}
	// flags
	if (prof->ubMiscFlags3 & PROFILE_MISC_FLAG3_GOODGUY) {
		obj.set("007isGoodGuy", true);
	}
	// appearance
	obj.set("050bodyType", Internals::getBodyTypeName(static_cast<SoldierBodyType>(prof->ubBodyType)));
	if (prof->uiBodyTypeSubFlags > 0) {
		obj.set("051bodyTypeSubstitution", prof->uiBodyTypeSubFlags & SUB_ANIM_BIGGUYSHOOT2 ? "SUB_ANIM_BIGGUYSHOOT2" : "SUB_ANIM_BIGGUYTHREATENSTANCE");
	}
	if ((prof->usEyesX + prof->usEyesY + prof->usMouthX + prof->usMouthY) != 0) {
		JsonObject portrait;
		JsonArray eyeOffsets;
		eyeOffsets.push(prof->usEyesX);
		eyeOffsets.push(prof->usEyesY);
		portrait.set("000eyesXY", eyeOffsets.toValue());
		JsonArray mouthOffsets;
		mouthOffsets.push(prof->usMouthX);
		mouthOffsets.push(prof->usMouthY);
		portrait.set("001mouthXY", mouthOffsets.toValue());
		if (prof->uiBlinkFrequency != 3000) {
			portrait.set("004blinkFrequency", prof->uiBlinkFrequency);
		}
		if (prof->uiExpressionFrequency != 2000) {
			portrait.set("005expressionFrequency", prof->uiExpressionFrequency);
		}
		obj.set("060face", portrait.toValue());
	}
	if (!prof->SKIN.empty()) {
		obj.set("061skinColor", prof->SKIN);
		obj.set("062hairColor", prof->HAIR);
		obj.set("063vestColor", prof->VEST);
		obj.set("064pantsColor", prof->PANTS);
	}
	// relations
	switch (prof->bSexist)
	{
	case SexistLevels::GENTLEMAN:
		obj.set("074sexismMode", "GENTLEMAN");
		break;
	case SexistLevels::SOMEWHAT_SEXIST:
		obj.set("074sexismMode", "SOMEWHAT_SEXIST");
		break;
	case SexistLevels::VERY_SEXIST:
		obj.set("074sexismMode", "VERY_SEXIST");
		break;
	default:
		break;
	}
	// stats
	JsonObject stats;
	if (prof->bEvolution > CharacterEvolution::NORMAL_EVOLUTION) {
		stats.set("000evolution", prof->bEvolution == CharacterEvolution::NO_EVOLUTION ? "NONE" : "REVERSED");
	}
	stats.set("health", prof->bLifeMax);
	stats.set("agility", prof->bAgility);
	stats.set("dexterity", prof->bDexterity);
	stats.set("strength", prof->bStrength);
	stats.set("leadership", prof->bLeadership);
	stats.set("wisdom", prof->bWisdom);
	stats.set("experience", prof->bExpLevel);
	stats.set("marksmanship", prof->bMarksmanship);
	stats.set("medical", prof->bMedical);
	stats.set("explosive", prof->bExplosive);
	stats.set("mechanical", prof->bMechanical);
	stats.set("sleepiness", prof->ubNeedForSleep);
	obj.set("070stats", stats.toValue());
	// traits
	if (prof->bPersonalityTrait > PersonalityTrait::NO_PERSONALITYTRAIT) {
		obj.set("071personalityTrait", Internals::getPersonalityTraitName(static_cast<PersonalityTrait>(prof->bPersonalityTrait)));
	}
	if (prof->bSkillTrait > SkillTrait::NO_SKILLTRAIT) {
		obj.set("072skillTrait", Internals::getSkillTraitName(static_cast<SkillTrait>(prof->bSkillTrait)));
	}
	if (prof->bSkillTrait2 > SkillTrait::NO_SKILLTRAIT) {
		obj.set("073skillTrait2", Internals::getSkillTraitName(static_cast<SkillTrait>(prof->bSkillTrait2)));
	}
	if (prof->bAttitude > Attitudes::ATT_NORMAL) {
		obj.set("074attitude", Internals::getAttitudeName(static_cast<Attitudes>(prof->bAttitude)));
	}
	// locations
	if (prof->sSector != SGPSector()) {
		if (prof->sSector.z > 0) {
			obj.set("075sector", prof->sSector.AsLongString());
		}
		else obj.set("075sector", prof->sSector.AsShortString());
	}
	if (prof->bTown > 0) {
		obj.set("076town", contentManager->getTown(prof->bTown)->internalName);
	}
	if (prof->bTownAttachment > 0) {
		obj.set("077townAttachment", prof->bTownAttachment);
	}
	if (prof->ubMiscFlags3 & PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH) {
		obj.set("078isTownIndifferentIfDead", true);
	}
	JsonObject roomRange;
	if (prof->ubRoomRangeStart[0] > 0 || prof->ubRoomRangeEnd[0] > 0) {
		JsonArray range1;
		range1.push(prof->ubRoomRangeStart[0]);
		range1.push(prof->ubRoomRangeEnd[0]);
		roomRange.set("range1", range1.toValue());
	}
	if (prof->ubRoomRangeStart[1] > 0 || prof->ubRoomRangeEnd[1] > 0) {
		JsonArray range2;
		range2.push(prof->ubRoomRangeStart[1]);
		range2.push(prof->ubRoomRangeEnd[1]);
		roomRange.set("range2", range2.toValue());
	}
	if ((prof->ubRoomRangeStart[0] + prof->ubRoomRangeEnd[0] + prof->ubRoomRangeStart[1] + prof->ubRoomRangeEnd[1]) > 0) {
		obj.set("079ownedRooms", roomRange.toValue());
	}
	// contract
	obj.set("083toleranceForPlayersReputation", prof->bReputationTolerance);
	obj.set("084toleranceForPlayersDeathRate", prof->bDeathRate);
	if ((prof->sSalary + prof->uiWeeklySalary + prof->uiBiWeeklySalary + prof->bMedicalDeposit) != 0) {
		JsonObject contract;
		contract.set("003dailySalary", prof->sSalary);
		if (prof->uiWeeklySalary > 0) {
			contract.set("004weeklySalary", prof->uiWeeklySalary);
		}
		if (prof->uiBiWeeklySalary > 0) {
			contract.set("005biWeeklySalary", prof->uiBiWeeklySalary);
		}
		contract.set("006isMedicalDepositRequired", prof->bMedicalDeposit ? true : false);
		obj.set("085contract", contract.toValue());
	}
	// possessions
	JsonArray invSlots;
	for (size_t i = 0; i < InvSlotPos::NUM_INV_SLOTS; i++) {
		if (prof->inv[i] > 0) {
			JsonObject invItem;
			const ItemModel* item = contentManager->getItem(prof->inv[i]);
			invItem.set("000slot", Internals::getInventorySlotName(static_cast<InvSlotPos>(i)));
			if (prof->ubInvUndroppable & gubItemDroppableFlag[i]) {
				invItem.set("001isUndroppable", true);
			}
			invItem.set("002item", item->getInternalName());
			if (prof->bInvNumber[i] > 1) {
				invItem.set("003quantity", prof->bInvNumber[i]);
			}
			if (prof->bInvStatus[i] < 100) {
				invItem.set("004status", prof->bInvStatus[i]);
			}
			invSlots.push(invItem.toValue());
		}
	}
	if (invSlots.size() > 0) {
		obj.set("090inventory", invSlots.toValue());
	}
	if (prof->uiMoney > 0) {
		obj.set("093money", prof->uiMoney);
	}

	JsonArray approachFactors;
	bool isEmpty = true;
	for (size_t i = 0; i < lengthof(prof->usApproachFactor); i++) {
		if (prof->usApproachFactor[i] > 0 || prof->ubApproachVal[i] > 0) isEmpty = false;
		JsonObject appr;
		appr.set("000approach", Internals::getApproachName(static_cast<Approach>(i + 1)));
		appr.set("002effectiveness", prof->usApproachFactor[i]);
		if (prof->ubApproachVal[i] > 0) {
			appr.set("003desireToTalk", prof->ubApproachVal[i]);
			appr.set("004friendlyMod", prof->ubApproachMod[0][i]);
			appr.set("005directMod", prof->ubApproachMod[1][i]);
			appr.set("006threatenMod", prof->ubApproachMod[2][i]);
		}
		approachFactors.push(appr.toValue());
	}
	if (!isEmpty) {
		obj.set("150dialogue", approachFactors.toValue());
	}
	return obj.toValue();
}

JsonValue MercProfile::serializeStructRelations(const ContentManager* contentManager) const
{
	MERCPROFILESTRUCT *prof = this->m_profile;
	JsonObject obj;
	// triple digits prepended to field names is a workaround to sort them in convenient order instead of alphabetically
	obj.set("000profile", this->getInfo().internalName);
	JsonArray opinions;
	for (uint8_t i = 0; i < lengthof(prof->bMercOpinion); i++) {
		if (prof->bMercOpinion[i] != 0) {
			JsonObject opinion;
			opinion.set("000target", (contentManager->getMercProfileInfo(i))->internalName);
			opinion.set("001opinion", prof->bMercOpinion[i]);
			for (size_t j = BUDDY_SLOT1; j < lengthof(prof->bBuddy); j++) {
				if (prof->bBuddy[j] == i) {
					opinion.set(j + 1 == 1 ? "002friend1" : "002friend2", true);
				}
				if (prof->bHated[j] == i) {
					opinion.set(j + 1 == 1 ? "002enemy1" : "002enemy2", true);
					opinion.set("003tolerance", prof->bHatedTime[j]);
				}
			}
			if (prof->bLearnToHate == i) {
				opinion.set("004eventualEnemy", true);
				opinion.set("005resistanceToMakingEnemy", prof->bLearnToHateTime);
			}
			if (prof->bLearnToLike == i) {
				opinion.set("004eventualFriend", true);
				opinion.set("005resistanceToBefriending", prof->bLearnToLikeTime);
			}
			opinions.push(opinion.toValue());
		}
	}
	obj.set("100relations", opinions.toValue());
	return obj.toValue();
}
