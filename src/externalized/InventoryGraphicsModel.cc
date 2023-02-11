#include "InventoryGraphicsModel.h"

InventoryGraphicsModel::InventoryGraphicsModel(GraphicModel small_, GraphicModel big_) : small(small_), big(big_) {
}

InventoryGraphicsModel InventoryGraphicsModel::deserialize(const JsonValue& json) {
	auto obj = json.toObject();
	return { GraphicModel::deserialize(obj["small"]), GraphicModel::deserialize(obj["big"]) };
}

JsonValue InventoryGraphicsModel::serialize() const {
	JsonObject v;

	auto s = small.serialize();
	auto b = big.serialize();
	v.set("small", std::move(s));
	v.set("big", std::move(b));

	return v.toValue();
}
