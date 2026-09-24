#include "IMPProfileJson.h"

#include "Animation_Data.h"
#include "ContentManager.h"
#include "Exceptions.h"
#include "GameInstance.h"
#include "ItemModel.h"
#include "Json.h"
#include "Logger.h"
#include "MagazineModel.h"
#include "WeaponModels.h"

#include <magic_enum/magic_enum.hpp>
#include <string_theory/format>

#include <exception>
#include <limits>
#include <set>
#include <type_traits>
#include <vector>


namespace {

/* Every field of MERCPROFILESTRUCT but impSlotState, which says whether a slot
 * is taken in the game that holds the profile, not anything about the
 * character. One list serves writing and reading, so the two cannot drift
 * apart, and the keys are the field names, so a file can be read next to the
 * struct. */
template<typename Visitor, typename Profile>
void VisitProfile(Visitor& v, Profile& p)
{
	v("zName", p.zName);
	v("zNickname", p.zNickname);
	v("bSex", p.bSex);
	v("ubCivilianGroup", p.ubCivilianGroup);
	v("ubMiscFlags", p.ubMiscFlags);
	v("ubMiscFlags2", p.ubMiscFlags2);
	v("ubMiscFlags3", p.ubMiscFlags3);
	v("ubBodyType", p.ubBodyType);
	v("uiBodyTypeSubFlags", p.uiBodyTypeSubFlags);

	v("ubFaceIndex", p.ubFaceIndex);
	v("usEyesX", p.usEyesX);
	v("usEyesY", p.usEyesY);
	v("usMouthX", p.usMouthX);
	v("usMouthY", p.usMouthY);
	v("uiBlinkFrequency", p.uiBlinkFrequency);
	v("uiExpressionFrequency", p.uiExpressionFrequency);
	v("ubVoiceId", p.ubVoiceId);

	v("PANTS", p.PANTS);
	v("VEST", p.VEST);
	v("SKIN", p.SKIN);
	v("HAIR", p.HAIR);

	v("bEvolution", p.bEvolution);
	v("bLifeMax", p.bLifeMax);
	v("bLife", p.bLife);
	v("bAgility", p.bAgility);
	v("bDexterity", p.bDexterity);
	v("bStrength", p.bStrength);
	v("bLeadership", p.bLeadership);
	v("bWisdom", p.bWisdom);
	v("bExpLevel", p.bExpLevel);
	v("bMarksmanship", p.bMarksmanship);
	v("bExplosive", p.bExplosive);
	v("bMechanical", p.bMechanical);
	v("bMedical", p.bMedical);
	v("ubNeedForSleep", p.ubNeedForSleep);

	v("sLifeGain", p.sLifeGain);
	v("bLifeDelta", p.bLifeDelta);
	v("sAgilityGain", p.sAgilityGain);
	v("bAgilityDelta", p.bAgilityDelta);
	v("sDexterityGain", p.sDexterityGain);
	v("bDexterityDelta", p.bDexterityDelta);
	v("sStrengthGain", p.sStrengthGain);
	v("bStrengthDelta", p.bStrengthDelta);
	v("sLeadershipGain", p.sLeadershipGain);
	v("bLeadershipDelta", p.bLeadershipDelta);
	v("sWisdomGain", p.sWisdomGain);
	v("bWisdomDelta", p.bWisdomDelta);
	v("sExpLevelGain", p.sExpLevelGain);
	v("bExpLevelDelta", p.bExpLevelDelta);
	v("sMarksmanshipGain", p.sMarksmanshipGain);
	v("bMarksmanshipDelta", p.bMarksmanshipDelta);
	v("sExplosivesGain", p.sExplosivesGain);
	v("bExplosivesDelta", p.bExplosivesDelta);
	v("sMechanicGain", p.sMechanicGain);
	v("bMechanicDelta", p.bMechanicDelta);
	v("sMedicalGain", p.sMedicalGain);
	v("bMedicalDelta", p.bMedicalDelta);
	v("usStatChangeChances", p.usStatChangeChances);
	v("usStatChangeSuccesses", p.usStatChangeSuccesses);

	v("bPersonalityTrait", p.bPersonalityTrait);
	v("bSkillTrait", p.bSkillTrait);
	v("bSkillTrait2", p.bSkillTrait2);
	v("bAttitude", p.bAttitude);
	v("bSexist", p.bSexist);

	v("bMercStatus", p.bMercStatus);
	v("bReputationTolerance", p.bReputationTolerance);
	v("bDeathRate", p.bDeathRate);
	v("uiDayBecomesAvailable", p.uiDayBecomesAvailable);
	v("sSalary", p.sSalary);
	v("uiWeeklySalary", p.uiWeeklySalary);
	v("uiBiWeeklySalary", p.uiBiWeeklySalary);
	v("bMedicalDeposit", p.bMedicalDeposit);
	v("sMedicalDepositAmount", p.sMedicalDepositAmount);
	v("iMercMercContractLength", p.iMercMercContractLength);
	v("usOptionalGearCost", p.usOptionalGearCost);
	v("ubSuspiciousDeath", p.ubSuspiciousDeath);
	v("ubDaysOfMoraleHangover", p.ubDaysOfMoraleHangover);

	v.sector("sSector", p.sSector);
	v("sGridNo", p.sGridNo);
	v("sPreCombatGridNo", p.sPreCombatGridNo);
	v("ubStrategicInsertionCode", p.ubStrategicInsertionCode);
	v("usStrategicInsertionData", p.usStrategicInsertionData);
	v("fUseProfileInsertionInfo", p.fUseProfileInsertionInfo);
	v("bTown", p.bTown);
	v("bTownAttachment", p.bTownAttachment);
	v("ubRoomRangeStart", p.ubRoomRangeStart);
	v("ubRoomRangeEnd", p.ubRoomRangeEnd);

	v("bBuddy", p.bBuddy);
	v("bHated", p.bHated);
	v("bHatedCount", p.bHatedCount);
	v("bHatedTime", p.bHatedTime);
	v("bLearnToLike", p.bLearnToLike);
	v("bLearnToLikeCount", p.bLearnToLikeCount);
	v("bLearnToLikeTime", p.bLearnToLikeTime);
	v("bLearnToHate", p.bLearnToHate);
	v("bLearnToHateCount", p.bLearnToHateCount);
	v("bLearnToHateTime", p.bLearnToHateTime);
	v("ubTimeTillNextHatedComplaint", p.ubTimeTillNextHatedComplaint);
	v("bMercOpinion", p.bMercOpinion);

	v.items("inv", p.inv);
	v("bInvNumber", p.bInvNumber);
	v("bInvStatus", p.bInvStatus);
	v("ubInvUndroppable", p.ubInvUndroppable);
	v("uiMoney", p.uiMoney);
	v("bArmourAttractiveness", p.bArmourAttractiveness);
	v("bMainGunAttractiveness", p.bMainGunAttractiveness);

	v("iBalance", p.iBalance);
	v("ubNumTimesDrugUseInLifetime", p.ubNumTimesDrugUseInLifetime);
	v("bNPCData", p.bNPCData);
	v("bNPCData2", p.bNPCData2);

	v("ubQuoteRecord", p.ubQuoteRecord);
	v("ubLastQuoteSaid", p.ubLastQuoteSaid);
	v("uiPrecedentQuoteSaid", p.uiPrecedentQuoteSaid);
	v("bLastQuoteSaidWasSpecial", p.bLastQuoteSaidWasSpecial);
	v("ubLastDateSpokenTo", p.ubLastDateSpokenTo);
	v("ubQuoteActionID", p.ubQuoteActionID);
	v("bFriendlyOrDirectDefaultResponseUsedRecently", p.bFriendlyOrDirectDefaultResponseUsedRecently);
	v("bRecruitDefaultResponseUsedRecently", p.bRecruitDefaultResponseUsedRecently);
	v("bThreatenDefaultResponseUsedRecently", p.bThreatenDefaultResponseUsedRecently);

	v("bApproached", p.bApproached);
	v("usApproachFactor", p.usApproachFactor);
	v("ubApproachVal", p.ubApproachVal);
	v("ubApproachMod", p.ubApproachMod);

	v("usKills", p.usKills);
	v("usAssists", p.usAssists);
	v("usShotsFired", p.usShotsFired);
	v("usShotsHit", p.usShotsHit);
	v("usBattlesFought", p.usBattlesFought);
	v("usTimesWounded", p.usTimesWounded);
	v("usTotalDaysServed", p.usTotalDaysServed);
	v("uiTotalCostToDate", p.uiTotalCostToDate);

	v("bSectorZ", p.bSectorZ);
	v("bRace", p.bRace);
	v("bRacist", p.bRacist);
	v("bNationality", p.bNationality);
	v("bAppearance", p.bAppearance);
	v("bAppearanceCareLevel", p.bAppearanceCareLevel);
	v("bRefinement", p.bRefinement);
	v("bRefinementCareLevel", p.bRefinementCareLevel);
	v("bHatedNationality", p.bHatedNationality);
	v("bHatedNationalityCareLevel", p.bHatedNationalityCareLevel);
}


/* How the fields an object shares with other kinds of object are to be
 * understood, as ExtractObject decides it for a saved game. */
enum class ObjectKind { Plain, Ammo, Gun, Key, Money };

ObjectKind KindOf(ItemModel const& item)
{
	switch (item.getItemClass())
	{
		case IC_AMMO:  return ObjectKind::Ammo;
		case IC_GUN:   return ObjectKind::Gun;
		case IC_KEY:   return ObjectKind::Key;
		case IC_MONEY: return ObjectKind::Money;
		default:       return ObjectKind::Plain;
	}
}

/* Items the map uses to mark things, which a character never carries. */
bool IsMapOnlyItem(UINT16 const item)
{
	return item == ACTION_ITEM || item == SWITCH || item == OWNERSHIP;
}


// Writing

JsonValue ToJson(ST::string const& s)
{
	return JsonValue(s);
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
JsonValue ToJson(T const v)
{
	if constexpr (std::is_signed_v<T>) return JsonValue(static_cast<int>(v));
	else return JsonValue(static_cast<unsigned int>(v));
}

template<typename T, size_t N>
JsonValue ToJson(T const (&a)[N])
{
	JsonArray arr;
	for (auto const& e : a) arr.push(ToJson(e));
	return arr.toValue();
}

template<typename T>
JsonValue ToJsonArray(T const* const a, size_t const n)
{
	JsonArray arr;
	for (size_t i = 0; i != n; ++i) arr.push(ToJson(a[i]));
	return arr.toValue();
}

JsonValue ItemToJson(UINT16 const item)
{
	return JsonValue(GCM->getItem(item)->getInternalName());
}


class ProfileWriter
{
public:
	template<typename T>
	void operator()(char const* const key, T const& value)
	{
		obj.set(key, ToJson(value));
	}

	void sector(char const* const key, SGPSector const& s)
	{
		JsonObject o;
		o.set("x", ToJson(s.x));
		o.set("y", ToJson(s.y));
		o.set("z", ToJson(s.z));
		obj.set(key, o.toValue());
	}

	template<size_t N>
	void items(char const* const key, UINT16 const (&ids)[N])
	{
		JsonArray arr;
		for (UINT16 const id : ids) arr.push(ItemToJson(id));
		obj.set(key, arr.toValue());
	}

	JsonObject obj;
};


JsonValue ObjectToJson(InvSlotPos const slot, OBJECTTYPE const& o)
{
	ItemModel const& item = *GCM->getItem(o.usItem);
	UINT8 const n = o.ubNumberOfObjects;

	JsonObject j;
	j.set("slot", JsonValue(ST::string(magic_enum::enum_name(slot))));
	j.set("item", ItemToJson(o.usItem));
	j.set("count", ToJson(n));
	switch (KindOf(item))
	{
		case ObjectKind::Ammo:
			j.set("shotsLeft", ToJsonArray(o.ubShotsLeft, std::min<size_t>(n, lengthof(o.ubShotsLeft))));
			break;

		case ObjectKind::Gun:
			j.set("status", ToJson(o.bGunStatus));
			j.set("ammoType", ToJson(o.ubGunAmmoType));
			j.set("shotsLeft", ToJson(o.ubGunShotsLeft));
			j.set("ammoItem", ItemToJson(o.usGunAmmoItem));
			j.set("ammoStatus", ToJson(o.bGunAmmoStatus));
			break;

		case ObjectKind::Key:
			j.set("status", ToJsonArray(o.bKeyStatus, std::min<size_t>(n, lengthof(o.bKeyStatus))));
			j.set("keyId", ToJson(o.ubKeyID));
			break;

		case ObjectKind::Money:
			j.set("status", ToJson(o.bMoneyStatus));
			j.set("amount", ToJson(o.uiMoneyAmount));
			break;

		case ObjectKind::Plain:
			j.set("status", ToJsonArray(o.bStatus, std::min<size_t>(n, lengthof(o.bStatus))));
			break;
	}

	JsonArray attachments;
	for (size_t i = 0; i != MAX_ATTACHMENTS; ++i)
	{
		if (o.usAttachItem[i] == NOTHING) continue;
		JsonObject a;
		a.set("item", ItemToJson(o.usAttachItem[i]));
		a.set("status", ToJson(o.bAttachStatus[i]));
		attachments.push(a.toValue());
	}
	if (attachments.size() != 0) j.set("attachments", attachments.toValue());

	j.set("flags", ToJson(o.fFlags));
	j.set("mission", ToJson(o.ubMission));
	j.set("trap", ToJson(o.bTrap));
	j.set("imprintId", ToJson(o.ubImprintID));
	j.set("weight", ToJson(o.ubWeight));
	j.set("used", ToJson(o.fUsed));
	return j.toValue();
}


// Reading

[[noreturn]] void Fail(ST::string const& path, ST::string const& what)
{
	throw DataError(ST::format("I.M.P. profile, {}: {}", path, what));
}

template<typename T>
T ToIntegral(JsonValue const& v, ST::string const& path)
{
	if (!v.isInt()) Fail(path, "expected an integer");
	// Every integer that fits one of the profile's types is exact as a double,
	// and a larger one is out of range whatever rounding it had.
	double const d = v.toDouble();
	using L = std::numeric_limits<T>;
	if (d < L::min() || d > L::max())
	{
		Fail(path, ST::format("{} is not in the range {}..{}",
			static_cast<int64_t>(d), static_cast<int64_t>(L::min()), static_cast<int64_t>(L::max())));
	}
	return static_cast<T>(static_cast<int64_t>(d));
}

std::vector<JsonValue> ToArray(JsonValue const& v, ST::string const& path)
{
	if (!v.isVec()) Fail(path, "expected an array");
	return v.toVec();
}

std::vector<JsonValue> ToArray(JsonValue const& v, size_t const length, ST::string const& path)
{
	auto elems = ToArray(v, path);
	if (elems.size() != length)
	{
		Fail(path, ST::format("expected {} elements, got {}", length, elems.size()));
	}
	return elems;
}

void FromJson(JsonValue const& v, ST::string& out, ST::string const& path)
{
	if (!v.isString()) Fail(path, "expected a string");
	out = v.toString();
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void FromJson(JsonValue const& v, T& out, ST::string const& path)
{
	out = ToIntegral<T>(v, path);
}

template<typename T, size_t N>
void FromJson(JsonValue const& v, T (&out)[N], ST::string const& path)
{
	auto const elems = ToArray(v, N, path);
	for (size_t i = 0; i != N; ++i) FromJson(elems[i], out[i], ST::format("{}[{}]", path, i));
}

ItemModel const& ItemFromJson(JsonValue const& v, ST::string const& path)
{
	ST::string name;
	FromJson(v, name, path);
	try
	{
		return *GCM->getItemByName(name);
	}
	catch (NotFoundError const&)
	{
		Fail(path, ST::format("there is no item '{}'", name));
	}
}


/* Reads the keys of one JSON object, and remembers which it took, so that a
 * key nobody asked for, most likely a typo in a file edited by hand, can be
 * pointed out. */
class ObjectReader
{
public:
	ObjectReader(JsonValue const& v, ST::string path) :
		m_path{ std::move(path) }, m_exceptions{ std::uncaught_exceptions() }
	{
		if (!v.isObject()) Fail(m_path, "expected an object");
		m_obj = v.toObject();
	}

	~ObjectReader()
	{
		// Keys not taken yet because reading failed are not unknown.
		if (std::uncaught_exceptions() != m_exceptions) return;
		for (auto const& key : m_obj.keys())
		{
			if (m_taken.count(key) == 0) SLOGW("I.M.P. profile, {}: ignoring unknown key '{}'", m_path, key);
		}
	}

	bool has(char const* const key) const { return m_obj.has(key); }

	ST::string path(char const* const key) const { return ST::format("{}.{}", m_path, key); }

	JsonValue take(char const* const key)
	{
		if (!m_obj.has(key)) Fail(m_path, ST::format("missing key '{}'", key));
		m_taken.insert(key);
		return m_obj.GetValue(key);
	}

	template<typename T>
	void operator()(char const* const key, T& value)
	{
		FromJson(take(key), value, path(key));
	}

	// Leaves the value as it is when the key is missing.
	template<typename T>
	void optional(char const* const key, T& value)
	{
		if (has(key)) (*this)(key, value);
	}

	void sector(char const* const key, SGPSector& s)
	{
		ObjectReader r{ take(key), path(key) };
		r("x", s.x);
		r("y", s.y);
		r("z", s.z);
	}

	template<size_t N>
	void items(char const* const key, UINT16 (&ids)[N])
	{
		auto const p = path(key);
		auto const elems = ToArray(take(key), N, p);
		for (size_t i = 0; i != N; ++i)
		{
			ids[i] = ItemFromJson(elems[i], ST::format("{}[{}]", p, i)).getItemIndex();
		}
	}

	UINT16 item(char const* const key)
	{
		return ItemFromJson(take(key), path(key)).getItemIndex();
	}

private:
	ST::string m_path;
	int m_exceptions;
	JsonObject m_obj;
	std::set<ST::string> m_taken;
};


template<typename T>
void RequireRange(ST::string const& path, T const value, int const min, int const max)
{
	if (value < min || value > max)
	{
		Fail(path, ST::format("{} is not in the range {}..{}", +value, min, max));
	}
}

template<typename T>
void ReadArrayPrefix(ObjectReader& r, char const* const key, T* const out, size_t const n, int const min, int const max)
{
	auto const p = r.path(key);
	auto const elems = ToArray(r.take(key), n, p);
	for (size_t i = 0; i != n; ++i)
	{
		ST::string const ep = ST::format("{}[{}]", p, i);
		FromJson(elems[i], out[i], ep);
		RequireRange(ep, out[i], min, max);
	}
}

template<typename T>
void ReadStatus(ObjectReader& r, char const* const key, T& out)
{
	r(key, out);
	RequireRange(r.path(key), out, 1, 100);
}

/* The profile only reaches the game if it is one a player could have made:
 * values the game looks things up with, or does arithmetic on as a
 * percentage, are in the range it expects. Values that only replay what
 * happened in the game that wrote the file are taken as they are. */
void ValidateProfile(MERCPROFILESTRUCT const& p)
{
	auto const length = [](char const* const key, ST::string const& s, size_t const max, bool const utf16)
	{
		size_t const size = utf16 ? s.to_utf16().size() : s.size();
		if (size > max)
		{
			Fail(ST::format("profile.{}", key), ST::format("'{}' is longer than {}", s, max));
		}
	};
	// what the fixed size records of a saved game have room for
	length("zName", p.zName, NAME_LENGTH - 1, true);
	length("zNickname", p.zNickname, NICKNAME_LENGTH - 1, true);
	length("PANTS", p.PANTS, PaletteRepID_LENGTH - 1, false);
	length("VEST", p.VEST, PaletteRepID_LENGTH - 1, false);
	length("SKIN", p.SKIN, PaletteRepID_LENGTH - 1, false);
	length("HAIR", p.HAIR, PaletteRepID_LENGTH - 1, false);
	if (p.zNickname.empty()) Fail("profile.zNickname", "must not be empty");

	RequireRange("profile.bSex", p.bSex, MALE, FEMALE);
	RequireRange("profile.ubBodyType", p.ubBodyType, REGMALE, REGFEMALE);

	RequireRange("profile.bLifeMax", p.bLifeMax, 1, 100);
	RequireRange("profile.bLife", p.bLife, 1, p.bLifeMax);
	RequireRange("profile.bAgility", p.bAgility, 1, 100);
	RequireRange("profile.bDexterity", p.bDexterity, 1, 100);
	RequireRange("profile.bStrength", p.bStrength, 1, 100);
	RequireRange("profile.bLeadership", p.bLeadership, 1, 100);
	RequireRange("profile.bWisdom", p.bWisdom, 1, 100);
	RequireRange("profile.bMarksmanship", p.bMarksmanship, 0, 100);
	RequireRange("profile.bExplosive", p.bExplosive, 0, 100);
	RequireRange("profile.bMechanical", p.bMechanical, 0, 100);
	RequireRange("profile.bMedical", p.bMedical, 0, 100);
	RequireRange("profile.bExpLevel", p.bExpLevel, 1, 10);

	RequireRange("profile.bEvolution", p.bEvolution, NORMAL_EVOLUTION, DEVOLVE);
	RequireRange("profile.bPersonalityTrait", p.bPersonalityTrait, NO_PERSONALITYTRAIT, PSYCHO);
	RequireRange("profile.bSkillTrait", p.bSkillTrait, NO_SKILLTRAIT, NUM_SKILLTRAITS - 1);
	RequireRange("profile.bSkillTrait2", p.bSkillTrait2, NO_SKILLTRAIT, NUM_SKILLTRAITS - 1);
	RequireRange("profile.bAttitude", p.bAttitude, ATT_NORMAL, NUM_ATTITUDES - 1);
	RequireRange("profile.bSexist", p.bSexist, NOT_SEXIST, GENTLEMAN);

	// Profile IDs the game indexes gMercProfiles with.
	RequireRange("profile.ubVoiceId", p.ubVoiceId, 0, NUM_PROFILES - 1);
	for (size_t i = 0; i != lengthof(p.bBuddy); ++i)
	{
		RequireRange(ST::format("profile.bBuddy[{}]", i), p.bBuddy[i], -1, NUM_PROFILES - 1);
		RequireRange(ST::format("profile.bHated[{}]", i), p.bHated[i], -1, NUM_PROFILES - 1);
	}
	RequireRange("profile.bLearnToLike", p.bLearnToLike, -1, NUM_PROFILES - 1);
	RequireRange("profile.bLearnToHate", p.bLearnToHate, -1, NUM_PROFILES - 1);

	for (size_t i = 0; i != lengthof(p.inv); ++i)
	{
		RequireRange(ST::format("profile.bInvNumber[{}]", i), p.bInvNumber[i], 0, MAX_OBJECTS_PER_SLOT);
		RequireRange(ST::format("profile.bInvStatus[{}]", i), p.bInvStatus[i], 0, 100);
	}
}

void ReadObject(ObjectReader& r, OBJECTTYPE& o)
{
	o = OBJECTTYPE{};
	o.usItem = r.item("item");
	ItemModel const& item = *GCM->getItem(o.usItem);
	if (o.usItem == NOTHING) Fail(r.path("item"), "an empty slot is left out, not written as NOTHING");
	if (IsMapOnlyItem(o.usItem)) Fail(r.path("item"), ST::format("'{}' cannot be carried", item.getInternalName()));

	r("count", o.ubNumberOfObjects);
	RequireRange(r.path("count"), o.ubNumberOfObjects, 1, MAX_OBJECTS_PER_SLOT);
	UINT8 const n = o.ubNumberOfObjects;

	switch (KindOf(item))
	{
		case ObjectKind::Ammo:
			ReadArrayPrefix(r, "shotsLeft", o.ubShotsLeft, n, 0, item.asAmmo()->capacity);
			break;

		case ObjectKind::Gun:
		{
			ReadStatus(r, "status", o.bGunStatus);
			r.optional("ammoType", o.ubGunAmmoType);
			r.optional("shotsLeft", o.ubGunShotsLeft);
			RequireRange(r.path("shotsLeft"), o.ubGunShotsLeft, 0, item.asWeapon()->ubMagSize);
			if (r.has("ammoItem"))
			{
				o.usGunAmmoItem = r.item("ammoItem");
				if (o.usGunAmmoItem != NOTHING && !GCM->getItem(o.usGunAmmoItem)->isAmmo())
				{
					Fail(r.path("ammoItem"), ST::format("'{}' is not ammunition", GCM->getItem(o.usGunAmmoItem)->getInternalName()));
				}
			}
			r.optional("ammoStatus", o.bGunAmmoStatus);
			break;
		}

		case ObjectKind::Key:
			if (n > lengthof(o.bKeyStatus))
			{
				Fail(r.path("count"), ST::format("a stack of keys holds at most {}", lengthof(o.bKeyStatus)));
			}
			ReadArrayPrefix(r, "status", o.bKeyStatus, n, 1, 100);
			r("keyId", o.ubKeyID);
			break;

		case ObjectKind::Money:
			ReadStatus(r, "status", o.bMoneyStatus);
			r("amount", o.uiMoneyAmount);
			break;

		case ObjectKind::Plain:
			ReadArrayPrefix(r, "status", o.bStatus, n, 1, 100);
			break;
	}

	if (r.has("attachments"))
	{
		auto const p = r.path("attachments");
		auto const attachments = ToArray(r.take("attachments"), p);
		if (attachments.size() > MAX_ATTACHMENTS)
		{
			Fail(p, ST::format("an item takes at most {} attachments", MAX_ATTACHMENTS));
		}
		for (size_t i = 0; i != attachments.size(); ++i)
		{
			ObjectReader a{ attachments[i], ST::format("{}[{}]", p, i) };
			o.usAttachItem[i] = a.item("item");
			if (o.usAttachItem[i] == NOTHING) Fail(a.path("item"), "an attachment must not be NOTHING");
			ReadStatus(a, "status", o.bAttachStatus[i]);
		}
	}

	r.optional("flags", o.fFlags);
	r.optional("mission", o.ubMission);
	r.optional("trap", o.bTrap);
	r.optional("imprintId", o.ubImprintID);
	r.optional("weight", o.ubWeight);
	r.optional("used", o.fUsed);
}

}


ST::string SerializeIMPProfile(MERCPROFILESTRUCT const& profile, OBJECTTYPE const (&inv)[NUM_INV_SLOTS])
{
	ProfileWriter w;
	VisitProfile(w, profile);

	JsonArray inventory;
	for (size_t i = 0; i != NUM_INV_SLOTS; ++i)
	{
		OBJECTTYPE const& o = inv[i];
		if (o.usItem == NOTHING) continue;
		if (IsMapOnlyItem(o.usItem) || o.ubNumberOfObjects == 0)
		{
			SLOGW("I.M.P. profile '{}': leaving out item {} in slot {}, a character cannot carry it",
				profile.zNickname, o.usItem, i);
			continue;
		}
		inventory.push(ObjectToJson(static_cast<InvSlotPos>(i), o));
	}

	JsonObject root;
	root.set("version", JsonValue(IMP_PROFILE_JSON_VERSION));
	root.set("profile", w.obj.toValue());
	root.set("inventory", inventory.toValue());
	return root.toValue().serialize(true);
}

void DeserializeIMPProfile(ST::string const& json, MERCPROFILESTRUCT& profile, OBJECTTYPE (&inv)[NUM_INV_SLOTS])
{
	JsonValue const root_value = JsonValue::deserialize(json);
	ObjectReader root{ root_value, "file" };

	int version;
	root("version", version);
	if (version < 1 || version > IMP_PROFILE_JSON_VERSION)
	{
		Fail(root.path("version"), ST::format("{} is not a version this game reads (1..{})", version, IMP_PROFILE_JSON_VERSION));
	}

	MERCPROFILESTRUCT p;
	{
		ObjectReader r{ root.take("profile"), "profile" };
		VisitProfile(r, p);
	}
	ValidateProfile(p);

	OBJECTTYPE items[NUM_INV_SLOTS]{};
	auto const inventory = ToArray(root.take("inventory"), "inventory");
	std::set<InvSlotPos> slots_taken;
	for (size_t i = 0; i != inventory.size(); ++i)
	{
		ObjectReader r{ inventory[i], ST::format("inventory[{}]", i) };
		ST::string slot_name;
		r("slot", slot_name);
		auto const slot = magic_enum::enum_cast<InvSlotPos>(slot_name.to_std_string());
		if (!slot || *slot == NUM_INV_SLOTS) Fail(r.path("slot"), ST::format("there is no slot '{}'", slot_name));
		if (!slots_taken.insert(*slot).second) Fail(r.path("slot"), ST::format("slot '{}' is given twice", slot_name));
		ReadObject(r, items[*slot]);
	}

	profile = p;
	std::copy(std::begin(items), std::end(items), std::begin(inv));
}
