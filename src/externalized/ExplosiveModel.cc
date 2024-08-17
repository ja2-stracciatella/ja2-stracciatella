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

uint8_t deserializeExplosiveType(const ST::string& s) {
	if (s == "NORMAL") {
		return EXPLOSV_NORMAL;
	}
	if (s == "STUN") {
		return EXPLOSV_STUN;
	}
	if (s == "TEARGAS") {
		return EXPLOSV_TEARGAS;
	}
	if (s == "MUSTGAS") {
		return EXPLOSV_MUSTGAS;
	}
	if (s == "FLARE") {
		return EXPLOSV_FLARE;
	}
	if (s == "NOISE") {
		return EXPLOSV_NOISE;
	}
	if (s == "SMOKE") {
		return EXPLOSV_SMOKE;
	}
	if (s == "CREATUREGAS") {
		return EXPLOSV_CREATUREGAS;
	}
	throw DataError(ST::format("Unknown explosive type '{}'", s));
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
			const ExplosionAnimationModel *animation
	) : ItemModel(itemIndex, internalName, shortName, name, description, itemClass, 0, cursor, inventoryGraphics, tileGraphic, weight, perPocket, price, coolness, reliability, repairEase, flags) {
	this->damage = damage;
	this->stunDamage = stunDamage;
	this->radius = radius;
	this->noise = noise;
	this->volatiltiy = volatility;
	this->type = type;
	this->animation = animation;
}

ExplosiveModel* ExplosiveModel::deserialize(const JsonValue &json, const std::vector<const ExplosionAnimationModel*> &animations, const VanillaItemStrings& vanillaItemStrings) {
	auto obj = json.toObject();
	ItemModel::InitData const initData{ obj, vanillaItemStrings };

	int itemIndex = obj.GetInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	auto shortName = ItemModel::deserializeShortName(initData);
	auto name = ItemModel::deserializeName(initData);
	auto description = ItemModel::deserializeDescription(initData);
	auto itemClass = deserializeItemClass(obj.GetString("itemClass"));
	auto type = deserializeExplosiveType(obj.GetString("type"));
	auto flags = ItemModel::deserializeFlags(obj);
	auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);
	auto cursor = deserializeItemCursor(obj.GetString("cursor"));

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

	auto explosive = new ExplosiveModel(
		itemIndex,
		internalName,
		shortName,
		name,
		description,
		itemClass,
		cursor,
		inventoryGraphics,
		tileGraphic,
		obj.GetUInt("ubWeight"),
		obj.GetUInt("ubPerPocket"),
		obj.GetUInt("usPrice"),
		obj.GetUInt("ubCoolness"),
		obj.GetInt("bReliability"),
		obj.GetInt("bRepairEase"),
		flags,
		obj.GetUInt("damage"),
		obj.GetUInt("stunDamage"),
		obj.GetUInt("radius"),
		obj.GetUInt("noise"),
		obj.GetUInt("volatility"),
		type,
		animation
	);

	return explosive;
}

uint8_t ExplosiveModel::getDamage() const {
	return damage;
}

uint8_t ExplosiveModel::getStunDamage() const {
	return stunDamage;
}

uint8_t ExplosiveModel::getRadius() const {
	return radius;
}

uint8_t ExplosiveModel::getNoise() const {
	return noise;
}

uint8_t ExplosiveModel::getVolatility() const {
	return volatiltiy;
}

uint8_t ExplosiveModel::getType() const {
	return type;
}

const ExplosionAnimationModel* ExplosiveModel::getAnimation() const {
	return animation;
}
