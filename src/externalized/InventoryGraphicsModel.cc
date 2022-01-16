#include "InventoryGraphicsModel.h"

InventoryGraphicsModel::InventoryGraphicsModel(SubImageModel small_, ST::string big_) : small(small_), big(big_) {
}

InventoryGraphicsModel InventoryGraphicsModel::deserialize(JsonObjectReader &obj) {
    auto& smallSource = obj.GetValue("small");
    ST::string big = obj.GetString("big");
	JsonObjectReader smallReader(smallSource);
    return InventoryGraphicsModel(SubImageModel::deserialize(smallReader), big);
}

JsonObject InventoryGraphicsModel::serialize(rapidjson::Document::AllocatorType& allocator) const {
    JsonObject v(allocator);

    auto s = small.serialize(allocator);
    v.AddMember("small", s.getValue());
    v.AddMember("big", big);

    return v;
}