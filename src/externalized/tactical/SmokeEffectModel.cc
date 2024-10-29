#include "SmokeEffectModel.h"

SmokeEffectModel::SmokeEffectModel(
	SmokeEffectID id,
	ST::string&& name,
	ST::string&& graphics,
	ST::string&& dissipatingGraphics,
	ST::string&& staticGraphics,
	uint8_t damage,
	uint8_t breathDamage,
	uint8_t lostVisibilityPerTile,
	uint8_t maxVisibility,
	uint8_t maxVisibilityWhenAffected,
	bool ignoresGasMask,
	bool affectsRobot,
	bool affectsMonsters
) {
	this->id = id;
	this->name = std::move(name);
	this->graphics = std::move(graphics);
	this->dissipatingGraphics = std::move(dissipatingGraphics);
	this->staticGraphics = std::move(staticGraphics);
	this->damage = damage;
	this->breathDamage = breathDamage;
	this->lostVisibilityPerTile = lostVisibilityPerTile;
	this->maxVisibility = maxVisibility;
	this->maxVisibilityWhenAffected = maxVisibilityWhenAffected;
	this->ignoresGasMask = ignoresGasMask;
	this->affectsRobot = affectsRobot;
	this->affectsMonsters = affectsMonsters;
}

SmokeEffectModel* SmokeEffectModel::deserialize(uint16_t index, const JsonValue& value) {
	auto obj = value.toObject();
	auto id = static_cast<SmokeEffectID>(index);
	auto name = obj.GetString("name");
	auto graphics = obj.GetString("graphics");
	auto dissipatingGraphics = obj.GetString("dissipatingGraphics");
	auto staticGraphics = obj.GetString("staticGraphics");
	uint8_t damage = obj.getOptionalUInt("damage");
	uint8_t breathDamage = obj.getOptionalUInt("breathDamage");
	uint8_t lostVisibilityPerTile = obj.getOptionalUInt("lostVisibilityPerTile");
	uint8_t maxVisibility = obj.getOptionalUInt("maxVisibility");
	uint8_t maxVisibilityWhenAffected = obj.getOptionalUInt("maxVisibilityWhenAffected");
	bool ignoresGasMask = obj.getOptionalBool("ignoresGasMask");
	bool affectsRobot = obj.getOptionalBool("affectsRobot");
	bool affectsMonsters = obj.getOptionalBool("affectsMonsters", true);

	return new SmokeEffectModel(
		id,
		std::move(name),
		std::move(graphics),
		std::move(dissipatingGraphics),
		std::move(staticGraphics),
		damage,
		breathDamage,
		lostVisibilityPerTile,
		maxVisibility,
		maxVisibilityWhenAffected,
		ignoresGasMask,
		affectsRobot,
		affectsMonsters
	);
}

SmokeEffectID SmokeEffectModel::getID() const {
	return id;
}

const ST::string& SmokeEffectModel::getName() const {
	return name;
}

const ST::string& SmokeEffectModel::getGraphics() const {
	return graphics;
}

const ST::string& SmokeEffectModel::getDissipatingGraphics() const {
	return dissipatingGraphics;
}

const ST::string& SmokeEffectModel::getStaticGraphics() const {
	return staticGraphics;
}


uint8_t SmokeEffectModel::getDamage() const {
	return damage;
}

uint8_t SmokeEffectModel::getBreathDamage() const {
	return breathDamage;
}

uint8_t SmokeEffectModel::getLostVisibilityPerTile() const {
	return lostVisibilityPerTile;
}

uint8_t SmokeEffectModel::getMaxVisibility() const {
	return maxVisibility;
}

uint8_t SmokeEffectModel::getMaxVisibilityWhenAffected() const {
	return maxVisibilityWhenAffected;
}

bool SmokeEffectModel::dealsAnyDamage() const {
	return damage > 0 || breathDamage > 0;
}

bool SmokeEffectModel::getIgnoresGasMask() const {
	return ignoresGasMask;
}

bool SmokeEffectModel::getAffectsRobot() const {
	return affectsRobot;
}

bool SmokeEffectModel::getAffectsMonsters() const {
	return affectsMonsters;
}
