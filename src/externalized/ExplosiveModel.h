#pragma once

#include "ExplosionAnimationModel.h"
#include "ItemModel.h"
#include "ExplosiveCalibreModel.h"

struct ExplosiveModel : ItemModel
{
	ExplosiveModel(
		uint16_t itemIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint32_t  itemClass,
		ItemCursor cursor,
		InventoryGraphicsModel inventoryGraphics,
		TilesetTileIndexModel tileGraphic,
		uint8_t weight,
		uint8_t perPocket,
		uint16_t price,
		uint8_t coolness,
		int8_t reliability,
		int8_t repairEase,
		uint32_t flags,
		uint8_t damage,
		uint8_t stunDamage,
		uint8_t radius,
		uint8_t noise,
		uint8_t volatility,
		uint8_t type,
		const ExplosiveCalibreModel *calibre,
		const ExplosionAnimationModel *animation
	);

	virtual const ExplosiveModel* asExplosive() const { return this; }

	static ExplosiveModel* deserialize(const JsonValue &json,
						const std::vector<const ExplosiveCalibreModel*> &explosiveCalibres,
						const std::vector<const ExplosionAnimationModel*> &animations,
						const BinaryData& vanillaItemStrings);

	uint8_t getDamage() const;
	uint8_t getStunDamage() const;
	uint8_t getRadius() const;
	uint8_t getNoise() const;
	uint8_t getVolatility() const;
	uint8_t getType() const;
	const ExplosionAnimationModel* getAnimation() const;

	bool isLaunchable() const;
	const ExplosiveCalibreModel* getExplosiveCalibre() const;

	protected:
		uint8_t damage;
		uint8_t stunDamage;
		uint8_t radius;
		uint8_t noise;
		uint8_t volatiltiy;
		uint8_t type;
		const ExplosionAnimationModel *animation;
		const ExplosiveCalibreModel *calibre;
};
