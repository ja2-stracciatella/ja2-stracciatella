#include "MercProfileInfo.h"
#include "JsonObject.h"
#include "Soldier_Control.h"
#include "Soldier_Profile_Type.h"
#include <string_theory/format>
#include <string_theory/string>
#include <algorithm>
#include <utility>


std::function<const MercProfileInfo*(ProfileID)> MercProfileInfo::load = {};

static MercType MercTypeFromString(const std::string& name)
{
	if (name == "NOT_USED")     return MercType::NOT_USED;
	else if (name == "AIM")     return MercType::AIM;
	else if (name == "MERC")    return MercType::MERC;
	else if (name == "IMP")     return MercType::IMP;
	else if (name == "RPC")     return MercType::RPC;
	else if (name == "NPC")     return MercType::NPC;
	else if (name == "VEHICLE") return MercType::VEHICLE;

	ST::string err = ST::format("unsupported MercType: '{}'", name);
	throw std::runtime_error(err.to_std_string());
}

MercProfileInfo::MercProfileInfo(uint8_t profileID_, ST::string internalName_, MercType mercType_, uint8_t weaponSaleModifier_)
	: internalName(std::move(internalName_)), profileID(profileID_), mercType(mercType_), weaponSaleModifier(weaponSaleModifier_)
{
}

MercProfileInfo::MercProfileInfo()
	: internalName(""), profileID(NO_PROFILE), mercType(MercType::NOT_USED), weaponSaleModifier(100)
{
}

MercProfileInfo *MercProfileInfo::deserialize(const rapidjson::Value &json)
{
	JsonObjectReader r(json);
	return new MercProfileInfo(
		r.GetUInt("profileID"),
		r.GetString("internalName"),
		MercTypeFromString(r.GetString("type")),
		std::clamp(r.getOptionalInt("weaponSaleModifier", 100), 10, 180)
		);
}

void MercProfileInfo::validateData(const std::map<uint8_t, const MercProfileInfo*> &models)
{
	if (models.empty())
	{
		throw std::runtime_error("No merc profile info defined");
	}
	if (models.size() > NUM_PROFILES)
	{
		throw std::runtime_error("Too many merc profiles");
	}
	for (auto const& p : models)
	{
		if (p.first != p.second->profileID)
		{
			throw std::logic_error("profileID is not consistent");
		}
		if (p.first >= NUM_PROFILES || p.first == NO_PROFILE)
		{
			throw std::runtime_error("Invalid profileID");
		}
	}

	if (!MercProfileInfo::load)
	{
		throw std::runtime_error("MercProfileInfo::load has not been initialized");
	}
}
