#include "ExplosiveModel.h"

#include "Exceptions.h"
#include "Weapons.h"

uint32_t deserializeItemClass(const ST::string& s) {
	if (s == "GRENADE") {
		return IC_GRENADE;
	}
	if (s == "BOMB") {
		return IC_BOMB;
	}
	throw DataError(ST::format("Item class '{}' not allowed for explosives", s));
}

ItemCursor deserializeItemCursor(const ST::string& s) {
	if (s == "INVALID") {
		return INVALIDCURS;
	}
	if (s == "QUEST") {
		return QUESTCURS;
	}
	if (s == "PUNCH") {
		return PUNCHCURS;
	}
	if (s == "TARGET") {
		return TARGETCURS;
	}
	if (s == "KNIFE") {
		return KNIFECURS;
	}
	if (s == "AID") {
		return AIDCURS;
	}
	if (s == "TOSS") {
		return TOSSCURS;
	}
	if (s == "MINE") {
		return MINECURS;
	}
	if (s == "LPICK") {
		return LPICKCURS;
	}
	if (s == "MDETECT") {
		return MDETECTCURS;
	}
	if (s == "CROWBAR") {
		return CROWBARCURS;
	}
	if (s == "SURVCAM") {
		return SURVCAMCURS;
	}
	if (s == "CAMERA") {
		return CAMERACURS;
	}
	if (s == "KEY") {
		return KEYCURS;
	}
	if (s == "SAW") {
		return SAWCURS;
	}
	if (s == "WIRECUT") {
		return WIRECUTCURS;
	}
	if (s == "REMOTE") {
		return REMOTECURS;
	}
	if (s == "BOMB") {
		return BOMBCURS;
	}
	if (s == "REPAIR") {
		return REPAIRCURS;
	}
	if (s == "TRAJECTORY") {
		return TRAJECTORYCURS;
	}
	if (s == "JAR") {
		return JARCURS;
	}
	if (s == "TINCAN") {
		return TINCANCURS;
	}
	if (s == "REFUEL") {
		return REFUELCURS;
	}
	throw DataError(ST::format("Unknown cursor '{}'", s));
}

ExplosiveModel::ExplosiveModel(
			uint16_t itemIndex,
			ST::string&& internalName,
			ST::string&& shortName,
			ST::string&& name,
			ST::string&& description,
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
			const ExplosiveCalibreModel* calibre,
			const ExplosionAnimationModel *animation
	) : ItemModel(itemIndex, std::move(internalName), std::move(shortName), std::move(name), std::move(description), itemClass, 0, cursor, std::move(inventoryGraphics), std::move(tileGraphic), weight, perPocket, price, coolness, reliability, repairEase, flags) {
	this->noise = noise;
	this->volatility = volatility;
	this->pressureActivated = isPressureTriggered;
	this->blastEffect = blastEffect;
	this->stunEffect = stunEffect;
	this->smokeEffect = smokeEffect;
	this->lightEffect = lightEffect;
	this->calibre = calibre;
	this->animation = animation;
}

ExplosiveModel::~ExplosiveModel() {
	delete blastEffect;
	delete stunEffect;
	delete smokeEffect;
	delete lightEffect;
}

ExplosiveModel* ExplosiveModel::deserialize(
	const JsonValue &json,
	const std::vector<const ExplosiveCalibreModel*> &explosiveCalibres,
	const std::vector<const SmokeEffectModel*> &smokeEffects,
	const std::vector<const ExplosionAnimationModel*> &animations,
	const BinaryData& vanillaItemStrings
) {
	auto obj = json.toObject();
	ItemModel::InitData const initData{ obj, vanillaItemStrings };

	int itemIndex = obj.GetInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	auto shortName = ItemModel::deserializeShortName(initData);
	auto name = ItemModel::deserializeName(initData);
	auto description = ItemModel::deserializeDescription(initData);
	auto itemClass = deserializeItemClass(obj.GetString("itemClass"));
	auto flags = ItemModel::deserializeFlags(obj);
	auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);
	auto cursor = deserializeItemCursor(obj.GetString("cursor"));

	auto calibreStr = obj.getOptionalString("calibre");
	const ExplosiveCalibreModel* calibre = nullptr;
	if (calibreStr != "") {
		auto calibreIt = std::find_if(explosiveCalibres.begin(), explosiveCalibres.end(), [&calibreStr](const ExplosiveCalibreModel* item) -> bool {
			return item->getName() == calibreStr;
		});
		if (calibreIt == explosiveCalibres.end()) {
			throw DataError(ST::format("Could not find explosive calibre '{}'", calibreStr));
		}
		calibre = *calibreIt;
	}

	auto animationStr = obj.getOptionalString("animation");
	const ExplosionAnimationModel* animation = nullptr;
	if (animationStr != "") {
		auto animationIt = std::find_if(animations.begin(), animations.end(), [&animationStr](const ExplosionAnimationModel* item) -> bool {
			return item->getName() == animationStr;
		});
		if (animationIt == animations.end()) {
			throw DataError(ST::format("Could not find explosion animation '{}'", animationStr));
		}
		animation = *animationIt;
	}

	ExplosiveBlastEffect* blastEffect = nullptr;
	if (obj.has("blastEffect")) {
		auto bObj = obj.GetValue("blastEffect").toObject();
		blastEffect = new ExplosiveBlastEffect();
		blastEffect->damage = bObj.GetUInt("damage");
		blastEffect->radius = bObj.GetUInt("radius");
		blastEffect->structuralDamageDisivor = bObj.getOptionalUInt("structuralDamageDisivor");
	}

	ExplosiveStunEffect* stunEffect = nullptr;
	if (obj.has("stunEffect")) {
		auto bObj = obj.GetValue("stunEffect").toObject();
		stunEffect = new ExplosiveStunEffect();
		stunEffect->breathDamage = bObj.GetUInt("breathDamage");
		stunEffect->radius = bObj.GetUInt("radius");
	}

	ExplosiveSmokeEffect* smokeEffect = nullptr;
	if (obj.has("smokeEffect")) {
		auto bObj = obj.GetValue("smokeEffect").toObject();
		smokeEffect = new ExplosiveSmokeEffect();
		smokeEffect->initialRadius = bObj.GetUInt("initialRadius");
		smokeEffect->maxRadius = bObj.GetUInt("maxRadius");
		smokeEffect->duration = bObj.GetUInt("duration");

		auto typeStr = bObj.getOptionalString("type");
		auto smokeEffectIt = std::find_if(smokeEffects.begin(), smokeEffects.end(), [&typeStr](const SmokeEffectModel* item) -> bool {
			return item->getName() == typeStr;
		});
		if (smokeEffectIt == smokeEffects.end()) {
			throw DataError(ST::format("Could not find smoke effect '{}'", typeStr));
		}
		smokeEffect->smokeEffect = *smokeEffectIt;
	}

	ExplosiveLightEffect* lightEffect = nullptr;
	if (obj.has("lightEffect")) {
		auto bObj = obj.GetValue("lightEffect").toObject();
		lightEffect = new ExplosiveLightEffect();
		lightEffect->radius = bObj.GetUInt("radius");
		lightEffect->duration = bObj.GetUInt("duration");
	}

	return new ExplosiveModel(
		itemIndex,
		std::move(internalName),
		std::move(shortName),
		std::move(name),
		std::move(description),
		itemClass,
		cursor,
		std::move(inventoryGraphics),
		std::move(tileGraphic),
		obj.GetUInt("ubWeight"),
		obj.GetUInt("ubPerPocket"),
		obj.GetUInt("usPrice"),
		obj.GetUInt("ubCoolness"),
		obj.GetInt("bReliability"),
		obj.GetInt("bRepairEase"),
		flags,
		obj.GetUInt("noise"),
		obj.GetUInt("volatility"),
		obj.getOptionalBool("isPressureTriggered"),
		blastEffect,
		stunEffect,
		smokeEffect,
		lightEffect,
		calibre,
		animation
	);
}

const ExplosiveBlastEffect* ExplosiveModel::getBlastEffect() const {
	return blastEffect;
}

const ExplosiveStunEffect* ExplosiveModel::getStunEffect() const {
	return stunEffect;
}

const ExplosiveSmokeEffect* ExplosiveModel::getSmokeEffect() const {
	return smokeEffect;
}

const ExplosiveLightEffect* ExplosiveModel::getLightEffect() const {
	return lightEffect;
}

uint8_t ExplosiveModel::getNoise() const {
	return noise;
}

uint8_t ExplosiveModel::getSafetyMargin() const {
	uint8_t margin = 0;
	if (blastEffect && blastEffect->radius > margin) {
		margin = blastEffect->radius;
	}
	if (stunEffect && stunEffect->radius > margin) {
		margin = stunEffect->radius;
	}
	if (smokeEffect && smokeEffect->maxRadius > margin) {
		margin = smokeEffect->maxRadius;
	}
	if (lightEffect && lightEffect->radius / 2 > margin) {
		margin = lightEffect->radius / 2;
	}
	return margin;
}

uint8_t ExplosiveModel::getVolatility() const {
	return volatility;
}

bool ExplosiveModel::isPressureTriggered() const {
	return pressureActivated;
}

bool ExplosiveModel::isLaunchable() const {
	return calibre;
}

const ExplosiveCalibreModel* ExplosiveModel::getExplosiveCalibre() const {
	return calibre;
}

const ExplosionAnimationModel* ExplosiveModel::getAnimation() const {
	return animation;
}
