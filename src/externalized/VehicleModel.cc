#include "VehicleModel.h"
#include "Exceptions.h"
#include "ItemModel.h"
#include "ItemSystem.h"
#include "JsonObject.h"
#include "Vehicles.h"

VehicleModel::VehicleModel(ST::string enterSound, ST::string movementSound,
                           ProfileID profile, VehicleMovementType movementType,
                           ItemId armourType, uint8_t seats_)
    : enter_sound(enterSound), move_sound(movementSound), profile(profile),
      movement_type(movementType), armour_type(armourType), seats(seats_) {}

static ProfileID toProfileID(unsigned int profileID)
{
	if (profileID >= UINT8_MAX)
	{
		throw DataError(ST::format("{} is not a valid ProfileID", profileID));
	}
	return static_cast<ProfileID>(profileID);
}

static VehicleMovementType toMovementType(const std::string& typeName)
{
	if (typeName == "FOOT") return FOOT;
	if (typeName == "CAR") return CAR;
	if (typeName == "TRUCK") return TRUCK;
	if (typeName == "TRACKED") return TRACKED;
	if (typeName == "AIR") return AIR;

	throw DataError(ST::format("'{}' is not a valid vehicle movement type", typeName));
}

const VehicleModel* VehicleModel::deserialize(JsonObjectReader &obj, const ItemSystem* itemSystem, const MercSystem* mercSystem)
{
	ST::string profile = obj.GetString("profile");
	auto mercProfile = mercSystem->getMercProfileInfoByName(profile);
	if (mercProfile == NULL) {
		throw DataError(ST::format("'{}' does not refer to a valid profile", profile));
	}
	if (mercProfile->mercType != MercType::VEHICLE && mercProfile->mercType != MercType::NOT_USED) {
		throw DataError(ST::format("Profile '{}' does not refer to a vehicle or unused profile", profile));
	}

	ST::string armourName = obj.GetString("armourType");
	const ItemModel* armour = itemSystem->getItemByName(armourName);
	if (!armour || !(armour->isArmour()))
	{
		throw DataError(ST::format("'{}' does not refer to an armour item", armourName));
	}

	unsigned int numSeats = obj.GetUInt("seats");
	if (numSeats > MAX_PASSENGERS_IN_VEHICLE)
	{
		// limited by struct VEHICLETYPE
		throw DataError(ST::format("Vehicles cannot have more than {} seats", MAX_PASSENGERS_IN_VEHICLE));
	}

	return new VehicleModel(
		obj.GetString("enterSound"),
		obj.GetString("moveSound"),
		mercProfile->profileID,
		toMovementType(obj.GetString("movementType")),
		armour->getItemIndex(),
		numSeats
		);
}

void VehicleModel::validateData(const std::vector<const VehicleModel*>& vehicles)
{
	if (vehicles.size() < NUMBER_OF_TYPES_OF_VEHICLES)
	{
		// the code has a lot of references to HELICOPTER(5)
		throw DataError(ST::format("There must be at least {} vehicle types", NUMBER_OF_TYPES_OF_VEHICLES));
	}
}
