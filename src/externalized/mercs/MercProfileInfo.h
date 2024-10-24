#pragma once

#include "JA2Types.h"
#include "Json.h"

#include <map>

enum class MercType : int8_t
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
	// Creates an empty instance, of ID NO_PROFILE and type NOT_USED
	MercProfileInfo();

	const ST::string internalName;
	const uint8_t profileID;
	const MercType mercType;
	const uint8_t weaponSaleModifier;

	// A function to provide MercProfileInfo by given ProfileIDs. This is to
	// avoid a circular reference to ContentManager.
	// This function must be initialized at init
	static std::function<const MercProfileInfo *(ProfileID)> load;
	static MercProfileInfo* deserialize(const JsonObject& json);
	static void validateData(const std::map<uint8_t, const MercProfileInfo*>& models);

protected:
	MercProfileInfo(uint8_t profileID_, ST::string internalName_, MercType mercType_, uint8_t weaponSaleModifier_);
};
