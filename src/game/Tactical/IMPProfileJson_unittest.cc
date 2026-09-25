#include "gtest/gtest.h"

#include "Animation_Data.h"
#include "DefaultContentManagerUT.h"
#include "Exceptions.h"
#include "IMPProfileJson.h"
#include "Json.h"

#include <cstring>
#include <functional>
#include <limits>
#include <set>
#include <type_traits>

namespace {

using IMPProfileJsonTest = DefaultContentManagerUT::BaseTest;

/* The fields of MERCPROFILESTRUCT written under their own names, listed here
 * again rather than taken from the serializer, so that a field the serializer
 * forgets, or writes under another name, fails these tests. A field added to
 * the struct belongs in one of these lists. */
#define IMP_PROFILE_INTEGERS(X) \
	X(bSex) X(ubCivilianGroup) X(ubMiscFlags) X(ubMiscFlags2) X(ubMiscFlags3) \
	X(ubBodyType) X(uiBodyTypeSubFlags) X(ubFaceIndex) X(usEyesX) X(usEyesY) \
	X(usMouthX) X(usMouthY) X(uiBlinkFrequency) X(uiExpressionFrequency) X(ubVoiceId) \
	X(bEvolution) X(bLifeMax) X(bLife) X(bAgility) X(bDexterity) X(bStrength) \
	X(bLeadership) X(bWisdom) X(bExpLevel) X(bMarksmanship) X(bExplosive) \
	X(bMechanical) X(bMedical) X(ubNeedForSleep) \
	X(sLifeGain) X(bLifeDelta) X(sAgilityGain) X(bAgilityDelta) X(sDexterityGain) \
	X(bDexterityDelta) X(sStrengthGain) X(bStrengthDelta) X(sLeadershipGain) \
	X(bLeadershipDelta) X(sWisdomGain) X(bWisdomDelta) X(sExpLevelGain) \
	X(bExpLevelDelta) X(sMarksmanshipGain) X(bMarksmanshipDelta) X(sExplosivesGain) \
	X(bExplosivesDelta) X(sMechanicGain) X(bMechanicDelta) X(sMedicalGain) X(bMedicalDelta) \
	X(bPersonalityTrait) X(bSkillTrait) X(bSkillTrait2) X(bAttitude) X(bSexist) \
	X(bMercStatus) X(bReputationTolerance) X(bDeathRate) X(uiDayBecomesAvailable) \
	X(sSalary) X(uiWeeklySalary) X(uiBiWeeklySalary) X(bMedicalDeposit) \
	X(sMedicalDepositAmount) X(iMercMercContractLength) X(usOptionalGearCost) \
	X(ubSuspiciousDeath) X(ubDaysOfMoraleHangover) \
	X(sGridNo) X(sPreCombatGridNo) X(ubStrategicInsertionCode) X(usStrategicInsertionData) \
	X(fUseProfileInsertionInfo) X(bTown) X(bTownAttachment) \
	X(bLearnToLike) X(bLearnToLikeCount) X(bLearnToLikeTime) X(bLearnToHate) \
	X(bLearnToHateCount) X(bLearnToHateTime) X(ubTimeTillNextHatedComplaint) \
	X(ubInvUndroppable) X(uiMoney) X(bArmourAttractiveness) X(bMainGunAttractiveness) \
	X(iBalance) X(ubNumTimesDrugUseInLifetime) X(bNPCData) X(bNPCData2) \
	X(ubQuoteRecord) X(ubLastQuoteSaid) X(uiPrecedentQuoteSaid) X(bLastQuoteSaidWasSpecial) \
	X(ubLastDateSpokenTo) X(ubQuoteActionID) X(bFriendlyOrDirectDefaultResponseUsedRecently) \
	X(bRecruitDefaultResponseUsedRecently) X(bThreatenDefaultResponseUsedRecently) \
	X(bApproached) \
	X(usKills) X(usAssists) X(usShotsFired) X(usShotsHit) X(usBattlesFought) \
	X(usTimesWounded) X(usTotalDaysServed) X(uiTotalCostToDate) \
	X(bSectorZ) X(bRace) X(bRacist) X(bNationality) X(bAppearance) X(bAppearanceCareLevel) \
	X(bRefinement) X(bRefinementCareLevel) X(bHatedNationality) X(bHatedNationalityCareLevel)

#define IMP_PROFILE_ARRAYS(X) \
	X(usStatChangeChances) X(usStatChangeSuccesses) X(ubRoomRangeStart) X(ubRoomRangeEnd) \
	X(bBuddy) X(bHated) X(bHatedCount) X(bHatedTime) X(bMercOpinion) \
	X(bInvNumber) X(bInvStatus) X(usApproachFactor) X(ubApproachVal)

#define IMP_PROFILE_STRINGS(X) X(zName) X(zNickname) X(PANTS) X(VEST) X(SKIN) X(HAIR)

// Written in a shape of their own: a nested object, item names, an array of arrays.
#define IMP_PROFILE_OTHERS(X) X(sSector) X(inv) X(ubApproachMod)


/* The fields a profile may not hold just any value in, set to values a player
 * could have made. */
void MakeValid(MERCPROFILESTRUCT& p)
{
	p.zName = "Ned Lee";
	p.zNickname = "Needle";
	p.PANTS = "BLUEPANTS";
	p.VEST = "GREYVEST";
	p.SKIN = "PINKSKIN";
	p.HAIR = "BROWNHEAD";
	p.bSex = FEMALE;
	p.ubBodyType = REGFEMALE;
	p.bLifeMax = 91;
	p.bLife = 77;
	p.bAgility = 82;
	p.bDexterity = 83;
	p.bStrength = 84;
	p.bLeadership = 85;
	p.bWisdom = 86;
	p.bExpLevel = 4;
	p.bMarksmanship = 87;
	p.bExplosive = 0;
	p.bMechanical = 100;
	p.bMedical = 55;
	p.bEvolution = DEVOLVE;
	p.bPersonalityTrait = NERVOUS;
	p.bSkillTrait = AUTO_WEAPS;
	p.bSkillTrait2 = CAMOUFLAGED;
	p.bAttitude = ATT_COWARD;
	p.bSexist = GENTLEMAN;
	p.ubVoiceId = 51;
	for (size_t i = 0; i != lengthof(p.bBuddy); ++i)
	{
		p.bBuddy[i] = static_cast<INT8>(i == 0 ? -1 : 10 + i);
		p.bHated[i] = static_cast<INT8>(i == 0 ? -1 : 20 + i);
	}
	p.bLearnToLike = 33;
	p.bLearnToHate = -1;
	UINT16 const items[] = { GLOCK_17, CLIP9_15, FIRSTAIDKIT, NOTHING, SPECTRA_VEST_18 };
	for (size_t i = 0; i != lengthof(p.inv); ++i)
	{
		p.inv[i] = items[i % lengthof(items)];
		p.bInvNumber[i] = static_cast<UINT8>(i % (MAX_OBJECTS_PER_SLOT + 1));
		p.bInvStatus[i] = static_cast<UINT8>(100 - i);
	}
}

/* Gives every integer of the profile a value of its own, so that a field read
 * back from another's key shows, then makes it valid. */
MERCPROFILESTRUCT DistinctProfile()
{
	MERCPROFILESTRUCT p;
	int seed = 1;
	auto const next = [&seed]() { return seed++ * 37 + 11; };
#define FILL_INTEGER(f) p.f = static_cast<std::remove_reference_t<decltype(p.f)>>(next());
	IMP_PROFILE_INTEGERS(FILL_INTEGER)
#undef FILL_INTEGER
#define FILL_ARRAY(f) for (auto& e : p.f) e = static_cast<std::remove_reference_t<decltype(e)>>(next());
	IMP_PROFILE_ARRAYS(FILL_ARRAY)
#undef FILL_ARRAY
	for (auto& row : p.ubApproachMod)
	{
		for (auto& e : row) e = static_cast<UINT8>(next());
	}
	p.sSector = SGPSector(9, 1, 2);
	p.impSlotState = IMPSlotState::TAKEN;
	MakeValid(p);
	return p;
}

void ExpectSameProfile(MERCPROFILESTRUCT const& a, MERCPROFILESTRUCT const& b)
{
#define CMP_SCALAR(f) EXPECT_EQ(+a.f, +b.f) << #f;
	IMP_PROFILE_INTEGERS(CMP_SCALAR)
#undef CMP_SCALAR
#define CMP_STRING(f) EXPECT_EQ(a.f, b.f) << #f;
	IMP_PROFILE_STRINGS(CMP_STRING)
#undef CMP_STRING
#define CMP_ARRAY(f) for (size_t i = 0; i != lengthof(a.f); ++i) EXPECT_EQ(+a.f[i], +b.f[i]) << #f "[" << i << "]";
	IMP_PROFILE_ARRAYS(CMP_ARRAY)
	CMP_ARRAY(inv)
#undef CMP_ARRAY
	for (size_t i = 0; i != 3; ++i)
	{
		for (size_t j = 0; j != 4; ++j)
		{
			EXPECT_EQ(a.ubApproachMod[i][j], b.ubApproachMod[i][j]) << "ubApproachMod[" << i << "][" << j << "]";
		}
	}
	EXPECT_EQ(a.sSector, b.sSector);
}

void ExpectSameObject(OBJECTTYPE const& a, OBJECTTYPE const& b, size_t const slot)
{
	// Every byte counts: whatever the union holds for this kind of item was
	// written, and the rest is zero both times.
	EXPECT_EQ(0, memcmp(&a, &b, sizeof(OBJECTTYPE))) << "slot " << slot;
}

struct EmptyInventory
{
	OBJECTTYPE inv[NUM_INV_SLOTS]{};
};

OBJECTTYPE Object(UINT16 const item, UINT8 const count = 1)
{
	OBJECTTYPE o{};
	o.usItem = item;
	o.ubNumberOfObjects = count;
	return o;
}

/* Carries one of each kind of object the file tells apart. */
EmptyInventory MixedInventory()
{
	EmptyInventory e;
	auto& inv = e.inv;

	OBJECTTYPE& gun = inv[HANDPOS] = Object(GLOCK_17);
	gun.bGunStatus = 93;
	gun.ubGunAmmoType = 1;
	gun.ubGunShotsLeft = 12;
	gun.usGunAmmoItem = CLIP9_15;
	gun.bGunAmmoStatus = 100;
	gun.usAttachItem[0] = SILENCER;
	gun.bAttachStatus[0] = 88;
	gun.usAttachItem[1] = LASERSCOPE;
	gun.bAttachStatus[1] = 71;
	gun.fFlags = 4;
	gun.ubImprintID = 52;
	gun.fUsed = 1;

	OBJECTTYPE& ammo = inv[BIGPOCK1POS] = Object(CLIP9_15, 3);
	ammo.ubShotsLeft[0] = 15;
	ammo.ubShotsLeft[1] = 7;
	ammo.ubShotsLeft[2] = 1;

	OBJECTTYPE& kits = inv[SMALLPOCK3POS] = Object(FIRSTAIDKIT, 2);
	kits.bStatus[0] = 100;
	kits.bStatus[1] = 42;

	OBJECTTYPE& key = inv[SMALLPOCK5POS] = Object(KEY_1);
	key.bKeyStatus[0] = 100;
	key.ubKeyID = 17;

	OBJECTTYPE& money = inv[SMALLPOCK8POS] = Object(MONEY);
	money.bMoneyStatus = 100;
	money.uiMoneyAmount = 4000000000u;

	OBJECTTYPE& vest = inv[VESTPOS] = Object(SPECTRA_VEST_18);
	vest.bStatus[0] = 64;
	vest.usAttachItem[0] = CERAMIC_PLATES;
	vest.bAttachStatus[0] = 50;
	vest.ubMission = 3;
	vest.bTrap = 2;
	vest.ubWeight = 9;
	return e;
}

JsonObject Root(ST::string const& json)
{
	return JsonValue::deserialize(json).toObject();
}

ST::string ValidJson()
{
	return SerializeIMPProfile(DistinctProfile(), MixedInventory().inv);
}

/* Changes one key of the profile a valid file holds. */
ST::string WithProfileKey(char const* const key, JsonValue value)
{
	JsonObject root = Root(ValidJson());
	JsonObject profile = root["profile"].toObject();
	profile.set(key, std::move(value));
	root.set("profile", profile.toValue());
	return root.toValue().serialize();
}

/* Changes the one item of a file holding just that item. */
ST::string WithObject(std::function<void(JsonObject&)> const& change, OBJECTTYPE const& o, InvSlotPos const slot = HANDPOS)
{
	EmptyInventory e;
	e.inv[slot] = o;
	JsonObject root = Root(SerializeIMPProfile(DistinctProfile(), e.inv));
	JsonObject object = root["inventory"].toVec().at(0).toObject();
	change(object);
	JsonArray inventory;
	inventory.push(object.toValue());
	root.set("inventory", inventory.toValue());
	return root.toValue().serialize();
}

ST::string Replace(ST::string const& s, char const* const from, char const* const to)
{
	EXPECT_TRUE(s.contains(from)) << from;
	return s.replace(from, to);
}

void ExpectRejected(ST::string const& json, char const* const message)
{
	MERCPROFILESTRUCT p;
	EmptyInventory e;
	try
	{
		DeserializeIMPProfile(json, p, e.inv);
		ADD_FAILURE() << "accepted, expected: " << message;
	}
	catch (DataError const& err)
	{
		EXPECT_TRUE(ST::string(err.what()).contains(message)) << "got: " << err.what() << "\nexpected: " << message;
	}
	catch (std::exception const& err)
	{
		ADD_FAILURE() << "not a DataError: " << err.what();
	}
}

}


// Round trips

TEST_F(IMPProfileJsonTest, RoundTripKeepsEveryProfileField)
{
	MERCPROFILESTRUCT const written = DistinctProfile();
	EmptyInventory e;
	MERCPROFILESTRUCT read;
	DeserializeIMPProfile(SerializeIMPProfile(written, e.inv), read, e.inv);
	ExpectSameProfile(written, read);
}

TEST_F(IMPProfileJsonTest, EveryFieldIsWrittenUnderItsOwnName)
{
	MERCPROFILESTRUCT const p = DistinctProfile();
	JsonObject const profile = Root(SerializeIMPProfile(p, EmptyInventory().inv))["profile"].toObject();

	std::set<ST::string> expected;
#define ADD_KEY(f) expected.insert(#f);
	IMP_PROFILE_INTEGERS(ADD_KEY)
	IMP_PROFILE_ARRAYS(ADD_KEY)
	IMP_PROFILE_STRINGS(ADD_KEY)
	IMP_PROFILE_OTHERS(ADD_KEY)
#undef ADD_KEY
	auto const keys = profile.keys();
	EXPECT_EQ(std::set<ST::string>(keys.begin(), keys.end()), expected);
	EXPECT_EQ(keys.size(), expected.size()) << "a key is written twice";

#define CHECK_INTEGER(f) EXPECT_EQ(profile.GetValue(#f).toDouble(), static_cast<double>(p.f)) << #f;
	IMP_PROFILE_INTEGERS(CHECK_INTEGER)
#undef CHECK_INTEGER
#define CHECK_ARRAY(f) { \
		auto const a = profile.GetValue(#f).toVec(); \
		ASSERT_EQ(a.size(), lengthof(p.f)) << #f; \
		for (size_t i = 0; i != a.size(); ++i) EXPECT_EQ(a[i].toDouble(), static_cast<double>(p.f[i])) << #f "[" << i << "]"; }
	IMP_PROFILE_ARRAYS(CHECK_ARRAY)
#undef CHECK_ARRAY
#define CHECK_STRING(f) EXPECT_EQ(profile.GetString(#f), p.f) << #f;
	IMP_PROFILE_STRINGS(CHECK_STRING)
#undef CHECK_STRING

	JsonObject const sector = profile["sSector"].toObject();
	EXPECT_EQ(sector.GetInt("x"), 9);
	EXPECT_EQ(sector.GetInt("y"), 1);
	EXPECT_EQ(sector.GetInt("z"), 2);

	auto const mod = profile["ubApproachMod"].toVec();
	ASSERT_EQ(mod.size(), 3u);
	for (size_t i = 0; i != 3; ++i)
	{
		auto const row = mod[i].toVec();
		ASSERT_EQ(row.size(), 4u);
		for (size_t j = 0; j != 4; ++j) EXPECT_EQ(row[j].toUInt(), p.ubApproachMod[i][j]);
	}
}

TEST_F(IMPProfileJsonTest, ProfileInventoryIsWrittenAsItemNames)
{
	MERCPROFILESTRUCT const p = DistinctProfile();
	auto const inv = Root(SerializeIMPProfile(p, EmptyInventory().inv))["profile"].toObject()["inv"].toVec();
	ASSERT_EQ(inv.size(), lengthof(p.inv));
	EXPECT_EQ(inv[0].toString(), "GLOCK_17");
	EXPECT_EQ(inv[1].toString(), "CLIP9_15");
	EXPECT_EQ(inv[2].toString(), "FIRSTAIDKIT");
	EXPECT_EQ(inv[3].toString(), "NOTHING");
	EXPECT_EQ(inv[4].toString(), "SPECTRA_VEST_18");
}

TEST_F(IMPProfileJsonTest, IntegersKeepTheirWholeRange)
{
	// the smallest and the largest value of every type, on fields the game
	// takes as they are
	for (bool const largest : { false, true })
	{
		MERCPROFILESTRUCT p = DistinctProfile();
#define EXTREME(f) { using T = std::remove_reference_t<decltype(p.f)>; \
		p.f = largest ? std::numeric_limits<T>::max() : std::numeric_limits<T>::min(); }
		EXTREME(uiBodyTypeSubFlags) EXTREME(usEyesX) EXTREME(uiBlinkFrequency)
		EXTREME(sLifeGain) EXTREME(bLifeDelta) EXTREME(bMercStatus) EXTREME(uiDayBecomesAvailable)
		EXTREME(sSalary) EXTREME(iMercMercContractLength) EXTREME(iBalance) EXTREME(uiMoney)
		EXTREME(uiPrecedentQuoteSaid) EXTREME(uiTotalCostToDate) EXTREME(sGridNo)
		EXTREME(ubQuoteRecord) EXTREME(usKills) EXTREME(bNPCData)
#undef EXTREME
		for (auto& e : p.bMercOpinion) e = largest ? 127 : -128;
		p.sSector = largest ? SGPSector(32767, 32767, 127) : SGPSector(-32768, -32768, -128);

		MERCPROFILESTRUCT read;
		EmptyInventory e;
		DeserializeIMPProfile(SerializeIMPProfile(p, e.inv), read, e.inv);
		ExpectSameProfile(p, read);
	}
}

TEST_F(IMPProfileJsonTest, NamesKeepCharactersOutsideAscii)
{
	MERCPROFILESTRUCT p = DistinctProfile();
	p.zName = "Łukasz Ñoño \"Ölmütze\" 猫";
	p.zNickname = "Żółw 猫";
	MERCPROFILESTRUCT read;
	EmptyInventory e;
	DeserializeIMPProfile(SerializeIMPProfile(p, e.inv), read, e.inv);
	EXPECT_EQ(read.zName, p.zName);
	EXPECT_EQ(read.zNickname, p.zNickname);
}

TEST_F(IMPProfileJsonTest, SlotStateIsNotWrittenAndReadsAsFree)
{
	MERCPROFILESTRUCT p = DistinctProfile();
	p.impSlotState = IMPSlotState::TAKEN;
	ST::string const json = SerializeIMPProfile(p, EmptyInventory().inv);
	EXPECT_FALSE(Root(json)["profile"].toObject().has("impSlotState"));

	MERCPROFILESTRUCT read;
	read.impSlotState = IMPSlotState::TAKEN;
	EmptyInventory e;
	DeserializeIMPProfile(json, read, e.inv);
	EXPECT_EQ(read.impSlotState, IMPSlotState::FREE);
}

TEST_F(IMPProfileJsonTest, FileCarriesTheVersion)
{
	EXPECT_EQ(Root(ValidJson()).GetInt("version"), IMP_PROFILE_JSON_VERSION);
}

TEST_F(IMPProfileJsonTest, RoundTripKeepsEveryKindOfObject)
{
	EmptyInventory const written = MixedInventory();
	MERCPROFILESTRUCT p;
	EmptyInventory read;
	// garbage in the slots that are empty in the file
	for (auto& o : read.inv) o = Object(CROWBAR);
	DeserializeIMPProfile(SerializeIMPProfile(DistinctProfile(), written.inv), p, read.inv);
	for (size_t i = 0; i != NUM_INV_SLOTS; ++i) ExpectSameObject(written.inv[i], read.inv[i], i);
}

TEST_F(IMPProfileJsonTest, InventoryIsWrittenBySlotAndItemName)
{
	auto const inventory = Root(ValidJson())["inventory"].toVec();
	ASSERT_EQ(inventory.size(), 6u) << "empty slots are left out";

	JsonObject const vest = inventory[0].toObject();
	EXPECT_EQ(vest.GetString("slot"), "VESTPOS");
	EXPECT_EQ(vest.GetString("item"), "SPECTRA_VEST_18");

	JsonObject const gun = inventory[1].toObject();
	EXPECT_EQ(gun.GetString("slot"), "HANDPOS");
	EXPECT_EQ(gun.GetString("item"), "GLOCK_17");
	EXPECT_EQ(gun.GetInt("status"), 93);
	EXPECT_EQ(gun.GetString("ammoItem"), "CLIP9_15");
	EXPECT_EQ(gun.GetInt("shotsLeft"), 12);
	auto const attachments = gun["attachments"].toVec();
	ASSERT_EQ(attachments.size(), 2u);
	EXPECT_EQ(attachments[0].toObject().GetString("item"), "SILENCER");
	EXPECT_EQ(attachments[1].toObject().GetString("item"), "LASERSCOPE");

	JsonObject const ammo = inventory[2].toObject();
	EXPECT_EQ(ammo.GetString("slot"), "BIGPOCK1POS");
	EXPECT_EQ(ammo.GetInt("count"), 3);
	EXPECT_EQ(ammo["shotsLeft"].toVec().size(), 3u) << "one value per object in the stack";

	JsonObject const money = inventory[5].toObject();
	EXPECT_EQ(money.GetString("slot"), "SMALLPOCK8POS");
	EXPECT_EQ(money.GetUInt("amount"), 4000000000u);
}

TEST_F(IMPProfileJsonTest, AttachmentsAreReadIntoTheFirstPositions)
{
	EmptyInventory e;
	OBJECTTYPE& gun = e.inv[HANDPOS] = Object(GLOCK_17);
	gun.bGunStatus = 100;
	gun.usAttachItem[2] = SILENCER;
	gun.bAttachStatus[2] = 90;

	MERCPROFILESTRUCT p;
	EmptyInventory read;
	DeserializeIMPProfile(SerializeIMPProfile(DistinctProfile(), e.inv), p, read.inv);
	EXPECT_EQ(read.inv[HANDPOS].usAttachItem[0], SILENCER);
	EXPECT_EQ(read.inv[HANDPOS].bAttachStatus[0], 90);
	EXPECT_EQ(read.inv[HANDPOS].usAttachItem[2], NOTHING);
}

TEST_F(IMPProfileJsonTest, MapOnlyItemsAreLeftOut)
{
	EmptyInventory e;
	e.inv[BIGPOCK2POS] = Object(ACTION_ITEM);
	e.inv[BIGPOCK3POS] = Object(CROWBAR, 0);
	e.inv[BIGPOCK4POS] = Object(CROWBAR);
	e.inv[BIGPOCK4POS].bStatus[0] = 60;
	auto const inventory = Root(SerializeIMPProfile(DistinctProfile(), e.inv))["inventory"].toVec();
	ASSERT_EQ(inventory.size(), 1u);
	EXPECT_EQ(inventory[0].toObject().GetString("slot"), "BIGPOCK4POS");
}

TEST_F(IMPProfileJsonTest, HandWrittenObjectNeedsOnlyItsEssentials)
{
	JsonObject root = Root(SerializeIMPProfile(DistinctProfile(), EmptyInventory().inv));
	root.set("inventory", JsonValue::deserialize(R"([
		{ "slot": "HANDPOS", "item": "GLOCK_17", "count": 1, "status": 80 },
		{ "slot": "SMALLPOCK1POS", "item": "FIRSTAIDKIT", "count": 1, "status": [ 100 ] }
	])"));
	MERCPROFILESTRUCT p;
	EmptyInventory read;
	DeserializeIMPProfile(root.toValue().serialize(), p, read.inv);

	OBJECTTYPE gun = Object(GLOCK_17);
	gun.bGunStatus = 80;
	ExpectSameObject(gun, read.inv[HANDPOS], HANDPOS);
	OBJECTTYPE kit = Object(FIRSTAIDKIT);
	kit.bStatus[0] = 100;
	ExpectSameObject(kit, read.inv[SMALLPOCK1POS], SMALLPOCK1POS);
}

TEST_F(IMPProfileJsonTest, UnknownKeysAreIgnored)
{
	ST::string json = Replace(ValidJson(), "\"version\":", "\"comment\": \"made by hand\", \"version\":");
	json = Replace(json, "\"zName\":", "\"favouriteColour\": \"green\", \"zName\":");
	MERCPROFILESTRUCT p;
	EmptyInventory e;
	DeserializeIMPProfile(json, p, e.inv);
	EXPECT_EQ(p.zNickname, "Needle");
}


// Rejected files

TEST_F(IMPProfileJsonTest, RejectsWhatIsNotAProfile)
{
	ExpectRejected("[]", "file: expected an object");
	ExpectRejected("{}", "missing key 'version'");
	ExpectRejected(R"({ "version": 1, "inventory": [] })", "missing key 'profile'");
	ExpectRejected(Replace(ValidJson(), "\"inventory\":", "\"inventori\":"), "missing key 'inventory'");

	MERCPROFILESTRUCT p;
	EmptyInventory e;
	EXPECT_ANY_THROW(DeserializeIMPProfile("not json", p, e.inv));
	EXPECT_ANY_THROW(DeserializeIMPProfile("", p, e.inv));
}

TEST_F(IMPProfileJsonTest, RejectsVersionsItDoesNotKnow)
{
	ST::string const current = ST::format("\"version\": {}", IMP_PROFILE_JSON_VERSION);
	ExpectRejected(Replace(ValidJson(), current.c_str(), "\"version\": 0"), "0 is not a version this game reads");
	ST::string const next = ST::format("\"version\": {}", IMP_PROFILE_JSON_VERSION + 1);
	ExpectRejected(Replace(ValidJson(), current.c_str(), next.c_str()), "is not a version this game reads");
	ExpectRejected(Replace(ValidJson(), current.c_str(), "\"version\": \"1\""), "version: expected an integer");
}

TEST_F(IMPProfileJsonTest, RejectsAMissingProfileKey)
{
	ExpectRejected(Replace(ValidJson(), "\"bLife\":", "\"bLive\":"), "profile: missing key 'bLife'");
	ExpectRejected(Replace(ValidJson(), "\"sSector\":", "\"sSektor\":"), "profile: missing key 'sSector'");
}

TEST_F(IMPProfileJsonTest, RejectsValuesOfTheWrongType)
{
	ExpectRejected(WithProfileKey("bLife", JsonValue(50.5)), "profile.bLife: expected an integer");
	ExpectRejected(WithProfileKey("bLife", JsonValue(true)), "profile.bLife: expected an integer");
	ExpectRejected(WithProfileKey("bLife", JsonValue("50")), "profile.bLife: expected an integer");
	ExpectRejected(WithProfileKey("zName", JsonValue(5)), "profile.zName: expected a string");
	ExpectRejected(WithProfileKey("bBuddy", JsonValue(5)), "profile.bBuddy: expected an array");
	ExpectRejected(WithProfileKey("sSector", JsonValue(5)), "profile.sSector: expected an object");
	ExpectRejected(WithProfileKey("inv", JsonValue::deserialize("[1]")), "profile.inv: expected 19 elements, got 1");
}

TEST_F(IMPProfileJsonTest, RejectsIntegersOutsideTheirType)
{
	ExpectRejected(WithProfileKey("usKills", JsonValue(-1)), "profile.usKills: -1 is not in the range 0..65535");
	ExpectRejected(WithProfileKey("usKills", JsonValue(65536)), "profile.usKills: 65536 is not in the range 0..65535");
	ExpectRejected(WithProfileKey("bNPCData", JsonValue(128)), "profile.bNPCData: 128 is not in the range -128..127");
	ExpectRejected(WithProfileKey("uiMoney", JsonValue::deserialize("4294967296")), "profile.uiMoney: 4294967296 is not in the range");
	ExpectRejected(WithProfileKey("iBalance", JsonValue::deserialize("-2147483649")), "profile.iBalance: -2147483649 is not in the range");
	ExpectRejected(WithProfileKey("uiMoney", JsonValue::deserialize("18446744073709551615")), "profile.uiMoney: expected an integer");

	JsonArray opinions;
	for (int i = 0; i != 76; ++i) opinions.push(JsonValue(0));
	ExpectRejected(WithProfileKey("bMercOpinion", opinions.toValue()), "profile.bMercOpinion: expected 75 elements, got 76");
	ExpectRejected(WithProfileKey("ubApproachMod", JsonValue::deserialize("[[0,0,0,0],[0,0,0,0],[0,0,0,256]]")),
		"profile.ubApproachMod[2][3]: 256 is not in the range 0..255");
	ExpectRejected(WithProfileKey("sSector", JsonValue::deserialize(R"({"x": 1, "y": 70000, "z": 0})")),
		"profile.sSector.y: 70000 is not in the range -32768..32767");
}

TEST_F(IMPProfileJsonTest, RejectsWhatNoPlayerCouldHaveMade)
{
	struct Case { char const* key; int value; char const* message; };
	for (Case const& c : {
		Case{ "bSex", 2, "profile.bSex: 2 is not in the range 0..1" },
		Case{ "ubBodyType", BIGMALE + 20, "profile.ubBodyType" },
		Case{ "bLifeMax", 0, "profile.bLifeMax: 0 is not in the range 1..100" },
		Case{ "bLife", 92, "profile.bLife: 92 is not in the range 1..91" },
		Case{ "bLife", 0, "profile.bLife: 0 is not in the range" },
		Case{ "bAgility", 101, "profile.bAgility: 101 is not in the range 1..100" },
		Case{ "bWisdom", 0, "profile.bWisdom: 0 is not in the range 1..100" },
		Case{ "bMedical", -1, "profile.bMedical: -1 is not in the range 0..100" },
		Case{ "bExpLevel", 11, "profile.bExpLevel: 11 is not in the range 1..10" },
		Case{ "bEvolution", 3, "profile.bEvolution" },
		Case{ "bPersonalityTrait", PSYCHO + 1, "profile.bPersonalityTrait" },
		Case{ "bSkillTrait", NUM_SKILLTRAITS, "profile.bSkillTrait" },
		Case{ "bSkillTrait2", -1, "profile.bSkillTrait2" },
		Case{ "bAttitude", NUM_ATTITUDES, "profile.bAttitude" },
		Case{ "bSexist", GENTLEMAN + 1, "profile.bSexist" },
		Case{ "ubVoiceId", NUM_PROFILES, "profile.ubVoiceId" },
		Case{ "bLearnToLike", -2, "profile.bLearnToLike" },
		Case{ "bLearnToHate", NUM_PROFILES, "profile.bLearnToHate" },
	})
	{
		SCOPED_TRACE(c.key);
		ExpectRejected(WithProfileKey(c.key, JsonValue(c.value)), c.message);
	}
	ExpectRejected(WithProfileKey("bBuddy", JsonValue::deserialize("[-1, 5, -2, -1, -1]")), "profile.bBuddy[2]: -2 is not in the range -1..169");
	ExpectRejected(WithProfileKey("bInvNumber", JsonValue::deserialize("[9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]")), "profile.bInvNumber[0]");
	ExpectRejected(WithProfileKey("bInvStatus", JsonValue::deserialize("[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,101]")), "profile.bInvStatus[18]");
}

TEST_F(IMPProfileJsonTest, RejectsNamesTheGameHasNoRoomFor)
{
	ExpectRejected(WithProfileKey("zNickname", JsonValue("")), "profile.zNickname: must not be empty");
	ExpectRejected(WithProfileKey("zNickname", JsonValue("Tennnnnnnn")), "profile.zNickname: 'Tennnnnnnn' is longer than 9");
	ExpectRejected(WithProfileKey("zName", JsonValue("A name of thirty characters xx")), "profile.zName");
	ExpectRejected(WithProfileKey("SKIN", JsonValue("A palette name of thirty bytes")), "profile.SKIN");

	// nine characters fit, however many bytes they take
	MERCPROFILESTRUCT p;
	EmptyInventory e;
	DeserializeIMPProfile(WithProfileKey("zNickname", JsonValue("ŻŻŻŻŻŻŻŻŻ")), p, e.inv);
	EXPECT_EQ(p.zNickname, "ŻŻŻŻŻŻŻŻŻ");
}

TEST_F(IMPProfileJsonTest, RejectsItemsTheGameDoesNotHave)
{
	JsonValue inv = JsonValue::deserialize(Root(ValidJson())["profile"].toObject()["inv"].serialize().replace("\"GLOCK_17\"", "\"GLOCK_99\""));
	ExpectRejected(WithProfileKey("inv", std::move(inv)), "profile.inv[0]: there is no item 'GLOCK_99'");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("item", JsonValue("PHASER")); }, Object(CROWBAR)),
		"inventory[0].item: there is no item 'PHASER'");
}

TEST_F(IMPProfileJsonTest, RejectsBrokenInventory)
{
	OBJECTTYPE crowbar = Object(CROWBAR);
	crowbar.bStatus[0] = 50;

	ExpectRejected(WithObject([](JsonObject& o) { o.set("slot", JsonValue("THIRDHANDPOS")); }, crowbar),
		"inventory[0].slot: there is no slot 'THIRDHANDPOS'");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("slot", JsonValue("NUM_INV_SLOTS")); }, crowbar),
		"there is no slot 'NUM_INV_SLOTS'");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("item", JsonValue("NOTHING")); }, crowbar),
		"an empty slot is left out");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("item", JsonValue("ACTION_ITEM")); }, crowbar),
		"cannot be carried");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("count", JsonValue(0)); }, crowbar),
		"inventory[0].count: 0 is not in the range 1..8");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("count", JsonValue(9)); }, crowbar),
		"inventory[0].count: 9 is not in the range 1..8");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("count", JsonValue(2)); }, crowbar),
		"inventory[0].status: expected 2 elements, got 1");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("status", JsonValue::deserialize("[0]")); }, crowbar),
		"inventory[0].status[0]: 0 is not in the range 1..100");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("status", JsonValue::deserialize("[101]")); }, crowbar),
		"inventory[0].status[0]: 101 is not in the range 1..100");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("attachments", JsonValue::deserialize(
		R"([{"item":"SILENCER","status":1},{"item":"SILENCER","status":1},{"item":"SILENCER","status":1},{"item":"SILENCER","status":1},{"item":"SILENCER","status":1}])")); }, crowbar),
		"inventory[0].attachments: an item takes at most 4 attachments");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("attachments", JsonValue::deserialize(R"([{"item":"NOTHING","status":1}])")); }, crowbar),
		"inventory[0].attachments[0].item: an attachment must not be NOTHING");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("attachments", JsonValue::deserialize(R"([{"item":"SILENCER","status":0}])")); }, crowbar),
		"inventory[0].attachments[0].status: 0 is not in the range 1..100");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("attachments", JsonValue::deserialize(R"([{"item":"SILENCER"}])")); }, crowbar),
		"inventory[0].attachments[0]: missing key 'status'");

	// the same slot twice
	EmptyInventory e;
	e.inv[HANDPOS] = crowbar;
	e.inv[SECONDHANDPOS] = crowbar;
	ST::string const json = SerializeIMPProfile(DistinctProfile(), e.inv);
	ExpectRejected(Replace(json, "\"SECONDHANDPOS\"", "\"HANDPOS\""), "inventory[1].slot: slot 'HANDPOS' is given twice");
}

TEST_F(IMPProfileJsonTest, RejectsBrokenGunsAndAmmunition)
{
	OBJECTTYPE gun = Object(GLOCK_17);
	gun.bGunStatus = 90;
	ExpectRejected(WithObject([](JsonObject& o) { o.set("shotsLeft", JsonValue(16)); }, gun),
		"inventory[0].shotsLeft: 16 is not in the range 0..15");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("ammoItem", JsonValue("CROWBAR")); }, gun),
		"inventory[0].ammoItem: 'CROWBAR' is not ammunition");
	ExpectRejected(WithObject([](JsonObject& o) { o.set("status", JsonValue(0)); }, gun),
		"inventory[0].status: 0 is not in the range 1..100");

	OBJECTTYPE clip = Object(CLIP9_15);
	clip.ubShotsLeft[0] = 15;
	ExpectRejected(WithObject([](JsonObject& o) { o.set("shotsLeft", JsonValue::deserialize("[16]")); }, clip),
		"inventory[0].shotsLeft[0]: 16 is not in the range 0..15");

	OBJECTTYPE money = Object(MONEY);
	money.bMoneyStatus = 100;
	ExpectRejected(WithObject([](JsonObject& o) { o.set("amount", JsonValue(-5)); }, money),
		"inventory[0].amount: -5 is not in the range 0..4294967295");

	OBJECTTYPE key = Object(KEY_1);
	key.bKeyStatus[0] = 100;
	ExpectRejected(WithObject([](JsonObject& o) { o.set("count", JsonValue(7)); }, key),
		"inventory[0].count: a stack of keys holds at most 6");
}

TEST_F(IMPProfileJsonTest, RejectedFileLeavesTheOutputsAlone)
{
	MERCPROFILESTRUCT p;
	p.zNickname = "Untouched";
	p.bLife = 42;
	EmptyInventory e;
	e.inv[HANDPOS] = Object(CROWBAR);

	// fails only at the inventory, after the whole profile was read
	ST::string const json = WithObject([](JsonObject& o) { o.set("count", JsonValue(0)); }, Object(CROWBAR));
	EXPECT_THROW(DeserializeIMPProfile(json, p, e.inv), DataError);
	EXPECT_EQ(p.zNickname, "Untouched");
	EXPECT_EQ(p.bLife, 42);
	EXPECT_EQ(e.inv[HANDPOS].usItem, CROWBAR);
	EXPECT_EQ(e.inv[HANDPOS].ubNumberOfObjects, 1);
}
