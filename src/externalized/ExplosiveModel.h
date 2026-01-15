#pragma once

#include "ExplosionAnimationModel.h"
#include "ExplosiveCalibreModel.h"
#include "ItemModel.h"
#include "SmokeEffectModel.h"
#include "Views.h"

struct ExplosiveBlastEffect
{
	uint8_t damage;
	uint8_t radius;
	uint8_t structuralDamageDisivor;
};

struct ExplosiveStunEffect
{
	uint8_t breathDamage;
	uint8_t radius;
};

struct ExplosiveLightEffect
{
	uint8_t radius;
	uint8_t duration;
};

struct ExplosiveSmokeEffect
{
	const SmokeEffectModel* smokeEffect;
	uint8_t initialRadius;
	uint8_t maxRadius;
	uint8_t duration;
};

struct ExplosiveModel : public ItemModel
{
	static constexpr const char* ENTITY_NAME = "Explosive";

	ExplosiveModel(
		uint16_t itemIndex,
		ST::string&& internalName,
		ST::string&& shortName,
		ST::string&& name,
		ST::string&& description,
		ST::string&& bobbyRaysName,
		ST::string&& bobbyRaysDescription,
		uint32_t  itemClass,
		ItemCursor cursor,
		InventoryGraphicsModel&& inventoryGraphics,
		TilesetTileIndexModel&& tileGraphic,
		uint8_t weight,
		uint8_t perPocket,
		uint16_t price,
		uint8_t coolness,
		int8_t reliability,
		int8_t repairEase,
		uint32_t flags,
		uint8_t noise,
		uint8_t volatility,
		bool isPressureTriggered,
		const ExplosiveBlastEffect *blastEffect,
		const ExplosiveStunEffect *stunEffect,
		const ExplosiveSmokeEffect *smokeEffect,
		const ExplosiveLightEffect *lightEffect,
		const ExplosiveCalibreModel *calibre,
		const ExplosionAnimationModel *animation
	);
	~ExplosiveModel();

	virtual const ExplosiveModel* asExplosive() const { return this; }

	static std::unique_ptr<ExplosiveModel> deserialize(const JsonValue &json,
						const std::vector<const ExplosiveCalibreModel*> &explosiveCalibres,
						const std::vector<const SmokeEffectModel*> &smokeEffects,
						const std::vector<const ExplosionAnimationModel*> &animations,
						TranslatableString::Loader& stringLoader);

	const ExplosiveBlastEffect* getBlastEffect() const;
	const ExplosiveStunEffect* getStunEffect() const;
	const ExplosiveLightEffect* getLightEffect() const;
	const ExplosiveSmokeEffect* getSmokeEffect() const;

	uint8_t getSafetyMargin() const;
	uint8_t getNoise() const;
	uint8_t getVolatility() const;
	bool isPressureTriggered() const;
	const ExplosionAnimationModel* getAnimation() const;

	bool isLaunchable() const;
	const ExplosiveCalibreModel* getExplosiveCalibre() const;

	protected:
		uint8_t noise;
		uint8_t volatility;
		bool pressureActivated;
		const ExplosiveBlastEffect *blastEffect;
		const ExplosiveStunEffect *stunEffect;
		const ExplosiveSmokeEffect *smokeEffect;
		const ExplosiveLightEffect *lightEffect;
		const ExplosionAnimationModel *animation;
		const ExplosiveCalibreModel *calibre;
};

class ExplosivesContainer : public Containers::Views::Named<uint16_t, ExplosiveModel, ItemModel> {
	public:
		ExplosivesContainer() = default;
		ExplosivesContainer(const ItemsContainer& items) :
			Containers::Views::Named<uint16_t, ExplosiveModel, ItemModel>(items.begin(), items.end(), [](const ItemModel* entity) { return entity->asExplosive(); }) {}
};
