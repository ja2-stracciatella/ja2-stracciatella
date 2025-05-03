#pragma once

#include "ItemModel.h"
#include <cstdint>

struct ArmourModel : ItemModel
{
	ArmourModel(
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
	);

	virtual const ArmourModel* asArmour() const { return this; }

	static ArmourModel* deserialize(const JsonValue &json, const BinaryData& vanillaItemStrings);

	uint8_t getArmourClass() const;
	uint8_t getProtection() const;
	uint8_t getDegradePercentage() const;
	protected:
		uint8_t armourClass;
		uint8_t protection;
		uint8_t degradePercentage;
};
