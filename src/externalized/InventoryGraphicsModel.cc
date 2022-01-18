#include "InventoryGraphicsModel.h"

InventoryGraphicsModel::InventoryGraphicsModel(GraphicModel small_, GraphicModel big_) : small(small_), big(big_) {
}

InventoryGraphicsModel InventoryGraphicsModel::deserialize(JsonObjectReader &obj) {
	auto& smallSource = obj.GetValue("small");
	auto& bigSource = obj.GetValue("big");
	JsonObjectReader smallReader(smallSource);
	JsonObjectReader bigReader(bigSource);
	return InventoryGraphicsModel(GraphicModel::deserialize(smallReader), GraphicModel::deserialize(bigReader));
}

JsonObject InventoryGraphicsModel::serialize(rapidjson::Document::AllocatorType& allocator) const {
	JsonObject v(allocator);

	auto s = small.serialize(allocator);
	auto b = big.serialize(allocator);
	v.AddMember("small", s.getValue());
	v.AddMember("big", b.getValue());

	return v;
}
