#pragma once

#include "ItemModel.h"
#include "Views.h"

#include <cstdint>

struct ArmourModel : public ItemModel
{
	static constexpr const char* ENTITY_NAME = "Armour";

	ArmourModel(
		uint16_t itemIndex,
		ST::string&& internalName,
		ST::string&& shortName,
		ST::string&& name,
		ST::string&& description,
		ST::string&& bobbyRaysName,
		ST::string&& bobbyRaysDescription,
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
		uint8_t explosivesProtection,
		uint8_t degradePercentage,
		bool ignoreForMaxProtection
	);

	virtual const ArmourModel* asArmour() const override { return this; }

	static std::unique_ptr<ArmourModel> deserialize(const JsonValue &json, TranslatableString::Loader& stringLoader);

	bool isIgnoredForMaxProtection() const;
	uint8_t getArmourClass() const;
	uint8_t getProtection() const;
	uint8_t getExplosivesProtection() const;
	uint8_t getDegradePercentage() const;
	bool canBeAttached(const GamePolicy* policy, const ItemModel* attachment) const override;
	protected:
		uint8_t armourClass;
		uint8_t protection;
		uint8_t explosivesProtection;
		uint8_t degradePercentage;
		bool ignoreForMaxProtection;
};

class ArmoursContainer : public Containers::Views::Named<uint16_t, ArmourModel, ItemModel> {
	public:
		ArmoursContainer() = default;
		ArmoursContainer(const ItemsContainer& items) :
			Containers::Views::Named<uint16_t, ArmourModel, ItemModel>(items.begin(), items.end(), [](const ItemModel* entity) { return entity->asArmour(); }) {}
};
