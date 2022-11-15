#pragma once
#include "Campaign_Types.h"
#include "JA2Types.h"
#include "Item_Types.h"
#include "Sound_Control.h"
#include "MercSystem.h"
#include <vector>

class JsonObjectReader;
class ItemSystem;

class VehicleModel
{
public:
	VehicleModel(ST::string enterSound, ST::string movementSound, ProfileID profileID, VehicleMovementType movementType, ItemId armourType, uint8_t seats);
	static const VehicleModel* deserialize(JsonObjectReader& obj, const ItemSystem* itemSystem, const MercSystem* mercSystem);
	static void validateData(const std::vector<const VehicleModel*>& vehicles);

	const ST::string   enter_sound;
	const ST::string   move_sound;
	const ProfileID profile;
	const VehicleMovementType   movement_type;
	const ItemId  armour_type;
	const uint8_t   seats;
};
