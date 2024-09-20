#include "enums.h"
#include "Exceptions.h"
#include "MercProfile.h"
#include "MercProfileInfo.h"
#include "Soldier_Profile_Type.h"
#include "TownModel.h"

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

void MercProfile::deserializeStructRelations(MERCPROFILESTRUCT* prof, const JsonObject& json, const ContentManager* contentManager)
{
	for (auto& element : json["relations"].toVec()) {
		JsonObject r = element.toObject();
		uint8_t idx = contentManager->getMercProfileInfoByName(r.GetString("target"))->profileID;
		if (idx >= NUM_RECRUITABLE) {
			throw DataError(ST::format("{}'s profile id must be less than {}", r.GetString("target"), NUM_RECRUITABLE));
		}
		prof->bMercOpinion[idx] = r.GetInt("opinion");
		if (r.getOptionalBool("friend1")) {
			prof->bBuddy[BUDDY_SLOT1] = idx;
		}
		else if (r.getOptionalBool("friend2")) {
			prof->bBuddy[BUDDY_SLOT2] = idx;
		}
		else if (r.getOptionalBool("eventualFriend")) {
			prof->bLearnToLike = idx;
			prof->bLearnToLikeTime = r.getOptionalUInt("resistanceToBefriending");
		}
		else if (r.getOptionalBool("enemy1")) {
			prof->bHated[HATED_SLOT1] = idx;
			prof->bHatedTime[HATED_SLOT1] = r.getOptionalUInt("tolerance");
		}
		else if (r.getOptionalBool("enemy2")) {
			prof->bHated[HATED_SLOT2] = idx;
			prof->bHatedTime[HATED_SLOT2] = r.getOptionalUInt("tolerance");
		}
		else if (r.getOptionalBool("eventualEnemy")) {
			prof->bLearnToHate = idx;
			prof->bLearnToHateTime = r.getOptionalUInt("resistanceToMakingEnemy");
		}
	}
}

std::unique_ptr<MERCPROFILESTRUCT> MercProfile::deserializeStruct(const JsonValue& json, const ContentManager* contentManager)
{
	JsonObject r = json.toObject();
	std::unique_ptr<MERCPROFILESTRUCT> prof = std::make_unique<MERCPROFILESTRUCT>();

	prof->zName = r.getOptionalString("fullName");
	prof->zNickname = r.getOptionalString("nickname");
	if (r.getOptionalString("sex") == "F") {
		prof->bSex = Sexes::FEMALE;
	}
	prof->ubCivilianGroup = Internals::getCivilianGroupEnumFromString(r.getOptionalString("civilianGroup"));
	if (r.getOptionalBool("isGoodGuy")) {
		prof->ubMiscFlags |= PROFILE_MISC_FLAG3_GOODGUY;
	}

	prof->ubBodyType = Internals::getBodyTypeEnumFromString(r.getOptionalString("bodyType"));
	ST::string jAnimFlag = r.getOptionalString("bodyTypeSubstitution");
	if (jAnimFlag == "SUB_ANIM_BIGGUYSHOOT2") {
		prof->uiBodyTypeSubFlags |= SUB_ANIM_BIGGUYSHOOT2;
	}
	else if (jAnimFlag == "SUB_ANIM_BIGGUYTHREATENSTANCE") {
		prof->uiBodyTypeSubFlags |= SUB_ANIM_BIGGUYTHREATENSTANCE;
	}
	if (r.has("face")) {
		JsonObject portrait = r["face"].toObject();
		std::vector<JsonValue> eyesXY = portrait["eyesXY"].toVec();
		std::vector<JsonValue> mouthXY = portrait["mouthXY"].toVec();
		prof->usEyesX = eyesXY[0].toInt();
		prof->usEyesY = eyesXY[1].toInt();
		prof->usMouthX = mouthXY[0].toInt();
		prof->usMouthY = mouthXY[1].toInt();
		prof->uiBlinkFrequency = portrait.getOptionalUInt("blinkFrequency", 3000);
		prof->uiExpressionFrequency = portrait.getOptionalUInt("expressionFrequency", 2000);
	}
	prof->SKIN = r.getOptionalString("skinColor");
	prof->HAIR = r.getOptionalString("hairColor");
	prof->VEST = r.getOptionalString("vestColor");
	prof->PANTS = r.getOptionalString("pantsColor");

	ST::string jSexismMode = r.getOptionalString("sexismMode");
	if (jSexismMode == "GENTLEMAN") {
		prof->bSexist = SexistLevels::GENTLEMAN;
	}
	else if (jSexismMode == "SOMEWHAT_SEXIST") {
		prof->bSexist = SexistLevels::SOMEWHAT_SEXIST;
	}
	else if (jSexismMode == "VERY_SEXIST") {
		prof->bSexist = SexistLevels::VERY_SEXIST;
	}

	if (r.has("stats")) {
		JsonObject stats = r["stats"].toObject();
		ST::string jEvolution = stats.getOptionalString("evolution");
		if (jEvolution == "NONE") {
			prof->bEvolution = CharacterEvolution::NO_EVOLUTION;
		}
		else if (jEvolution == "REVERSED") {
			prof->bEvolution = CharacterEvolution::DEVOLVE;
		}
		prof->bLifeMax = stats.getOptionalUInt("health", 15);
		prof->bLife = stats.getOptionalUInt("health", 15);
		prof->bAgility = stats.getOptionalUInt("agility", 1);
		prof->bDexterity = stats.getOptionalUInt("dexterity", 1);
		prof->bStrength = stats.getOptionalUInt("strength", 1);
		prof->bLeadership = stats.getOptionalUInt("leadership", 1);
		prof->bWisdom = stats.getOptionalUInt("wisdom", 1);
		prof->bExpLevel = stats.getOptionalUInt("experience", 1);
		prof->bMarksmanship = stats.getOptionalUInt("marksmanship");
		prof->bMedical = stats.getOptionalUInt("medical");
		prof->bExplosive = stats.getOptionalUInt("explosive");
		prof->bMechanical = stats.getOptionalUInt("mechanical");
		prof->ubNeedForSleep = stats.getOptionalUInt("sleepiness", 7);
	}
	prof->bPersonalityTrait = Internals::getPersonalityTraitEnumFromString(r.getOptionalString("personalityTrait"));
	prof->bSkillTrait = Internals::getSkillTraitEnumFromString(r.getOptionalString("skillTrait"));
	prof->bSkillTrait2 = Internals::getSkillTraitEnumFromString(r.getOptionalString("skillTrait2"));
	prof->bAttitude = Internals::getAttitudeEnumFromString(r.getOptionalString("attitude"));

	ST::string jSector = r.getOptionalString("sector");
	if (!jSector.empty()) {
		std::vector<ST::string> jLong = jSector.split("-");
		ST::string jShort = jLong[0];
		prof->sSector = SGPSector::FromShortString(jShort);
		if (jLong.size() == 2) {
			prof->sSector.z = jLong[1].to_int();
		}
	}

	ST::string jTown = r.getOptionalString("town");
	if (!jTown.empty()) {
		prof->bTown = contentManager->getTownByName(jTown)->townId;
	}
	prof->bTownAttachment = r.getOptionalUInt("townAttachment");
	if (r.getOptionalBool("isTownIndifferentIfDead")) {
		prof->ubMiscFlags3 |= PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH;
	}
	if (r.has("ownedRooms")) {
		JsonObject rooms = r["ownedRooms"].toObject();
		if (rooms.has("range1")) {
			std::vector<JsonValue> range1 = rooms["range1"].toVec();
			prof->ubRoomRangeStart[0] = range1[0].toInt();
			prof->ubRoomRangeEnd[0] = range1[1].toInt();
		}
		if (rooms.has("range2")) {
			std::vector<JsonValue> range2 = rooms["range2"].toVec();
			prof->ubRoomRangeStart[1] = range2[0].toInt();
			prof->ubRoomRangeEnd[1] = range2[1].toInt();
		}
	}

	prof->bReputationTolerance = r.getOptionalUInt("toleranceForPlayersReputation", 101);
	prof->bDeathRate = r.getOptionalUInt("toleranceForPlayersDeathRate", 101);
	if (r.has("contract")) {
		JsonObject c = r["contract"].toObject();
		prof->sSalary = c.getOptionalUInt("dailySalary");
		prof->uiWeeklySalary = c.getOptionalUInt("weeklySalary");
		prof->uiBiWeeklySalary = c.getOptionalUInt("biWeeklySalary");
		prof->bMedicalDeposit = c.getOptionalBool("isMedicalDepositRequired");
	}

	if (r.has("inventory")) {
		for (auto& element : r.GetValue("inventory").toVec()) {
			JsonObject s = element.toObject();
			InvSlotPos slotIdx = Internals::getInventorySlotEnumFromString(s.GetString("slot"));
			prof->inv[slotIdx] = contentManager->getItemByName(s.GetString("item"))->getItemIndex();
			prof->bInvNumber[slotIdx] = s.getOptionalUInt("quantity", 1);
			prof->bInvStatus[slotIdx] = s.getOptionalUInt("status", 100);
			if (s.getOptionalBool("isUndroppable")) {
				prof->ubInvUndroppable |= gubItemDroppableFlag[slotIdx];
			}
		}
	}
	prof->uiMoney = r.getOptionalUInt("money");

	if (r.has("dialogue")) {
		for (auto& element : r.GetValue("dialogue").toVec()) {
			JsonObject a = element.toObject();
			Approach jApproach = Internals::getApproachEnumFromString(a.GetString("approach"));
			if (jApproach > 0 && jApproach < 5) {
				prof->usApproachFactor[jApproach - 1] = a.getOptionalUInt("effectiveness");
				prof->ubApproachVal[jApproach - 1] = a.getOptionalUInt("desireToTalk");
				prof->ubApproachMod[APPROACH_FRIENDLY - 1][jApproach - 1] = a.getOptionalUInt("friendlyMod");
				prof->ubApproachMod[APPROACH_DIRECT - 1][jApproach - 1] = a.getOptionalUInt("directMod");
				prof->ubApproachMod[APPROACH_THREATEN - 1][jApproach - 1] = a.getOptionalUInt("threatenMod");
			}
		}
	}

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
