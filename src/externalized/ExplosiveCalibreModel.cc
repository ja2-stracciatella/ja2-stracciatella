#include "ExplosiveCalibreModel.h"

ExplosiveCalibreModel::ExplosiveCalibreModel(uint16_t id, ST::string name) {
	this->id = id;
	this->name = name;
}

ExplosiveCalibreModel* ExplosiveCalibreModel::deserialize(uint16_t id, const JsonValue &json) {
	auto obj = json.toObject();
	auto name = obj.GetString("internalName");

	return new ExplosiveCalibreModel(id, std::move(name));
}

uint16_t ExplosiveCalibreModel::getID() const {
	return id;
}

const ST::string& ExplosiveCalibreModel::getName() const {
	return name;
}
