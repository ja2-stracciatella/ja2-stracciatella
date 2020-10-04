#pragma once

#include "JA2Types.h"
#include <map>
#include <string_theory/string>
#include <rapidjson/document.h>

enum class MercType
{
	NOT_USED,
	AIM,
	MERC,
	IMP,
	RPC,
	NPC,
	VEHICLE
};

// Read-only data supplementary to the merc profiles
class MercProfileInfo
{
public:
	const uint8_t profileID;
	const ST::string internalName;
	const MercType mercType;

	// A function to provide MercProfileInfo by given ProfileIDs. This is to
	// avoid a circular reference to ContentManager.
	// This function must be initialized at init
	static std::function<const MercProfileInfo *(ProfileID)> load;
	static MercProfileInfo* deserialize(const rapidjson::Value& json);
	static void validateData(const std::map<uint8_t, const MercProfileInfo*>& models);

protected:
	MercProfileInfo(uint8_t profileID_, const char* internalName_, MercType mercType_);
};
