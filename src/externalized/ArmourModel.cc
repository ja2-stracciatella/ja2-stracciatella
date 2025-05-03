#include "ArmourModel.h"
#include "Exceptions.h"
#include "Item_Types.h"
#include "Weapons.h"
#include <cstdint>

uint8_t deserializeArmourClass(const ST::string& armourClass) {
	if (armourClass == "HELMET") return ARMOURCLASS_HELMET;
	if (armourClass == "VEST") return ARMOURCLASS_VEST;
	if (armourClass == "LEGGINGS") return ARMOURCLASS_LEGGINGS;
	if (armourClass == "PLATE") return ARMOURCLASS_PLATE;
	if (armourClass == "CREATURE") return ARMOURCLASS_MONST;
	throw DataError(ST::format("Unknown armour class '{}'", armourClass));
}

ArmourModel::ArmourModel(
			uint16_t itemIndex,
			ST::string&& internalName,
			ST::string&& shortName,
			ST::string&& name,
			ST::string&& description,
			InventoryGraphicsModel&& inventoryGraphics,
			TilesetTileIndexModel&& tileGraphic,
			uint8_t weight,
			uint8_t perPocket,
			uint16_t price,
			uint8_t coolness,
			int8_t reliability,
			int8_t repairEase,
			uint32_t flags,
			uint8_t armourClass,
			uint8_t protection,
			uint8_t degradePercentage
	) : ItemModel(itemIndex, std::move(internalName), std::move(shortName), std::move(name), std::move(description), IC_ARMOUR, 0, INVALIDCURS, std::move(inventoryGraphics), std::move(tileGraphic), weight, perPocket, price, coolness, reliability, repairEase, flags) {
	this->armourClass = armourClass;
	this->protection = protection;
	this->degradePercentage = degradePercentage;
}

ArmourModel* ArmourModel::deserialize(const JsonValue &json, const BinaryData& vanillaItemStrings) {
	auto obj = json.toObject();
	ItemModel::InitData const initData{ obj, vanillaItemStrings };

	int itemIndex = obj.GetInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	auto shortName = ItemModel::deserializeShortName(initData);
	auto name = ItemModel::deserializeName(initData);
	auto description = ItemModel::deserializeDescription(initData);
	auto flags = ItemModel::deserializeFlags(obj);
	auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);
	auto armourClass = deserializeArmourClass(obj.GetString("armourClass"));
	auto protection = obj.GetUInt("protection");
	auto degradePercentage = obj.GetUInt("degradePercentage");

	return new ArmourModel(
		itemIndex,
		std::move(internalName),
		std::move(shortName),
		std::move(name),
		std::move(description),
		std::move(inventoryGraphics),
		std::move(tileGraphic),
		obj.GetUInt("ubWeight"),
		obj.GetUInt("ubPerPocket"),
		obj.GetUInt("usPrice"),
		obj.GetUInt("ubCoolness"),
		obj.GetInt("bReliability"),
		obj.GetInt("bRepairEase"),
		flags,
		armourClass,
		protection,
		degradePercentage
	);
}

uint8_t ArmourModel::getArmourClass() const {
	return armourClass;
}

uint8_t ArmourModel::getProtection() const {
	return protection;
}

uint8_t ArmourModel::getDegradePercentage() const {
	return degradePercentage;
}
