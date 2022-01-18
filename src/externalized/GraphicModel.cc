#include "GraphicModel.h"

GraphicModel::GraphicModel(const ST::string path_, const uint8_t subImageIndex_) : path(path_), subImageIndex(subImageIndex_) {
}

const ST::string& GraphicModel::getPath() const {
	return this->path;
}

uint8_t GraphicModel::getSubImageIndex() const {
	return this->subImageIndex;
}

GraphicModel GraphicModel::deserialize(JsonObjectReader &obj) {
	return GraphicModel(
		obj.GetString("path"),
		obj.getOptionalUInt("subImageIndex", 0)
	);
}

JsonObject GraphicModel::serialize(rapidjson::Document::AllocatorType& allocator) const {
	JsonObject v(allocator);

	v.AddMember("path", path);
	v.AddMember("subImageIndex", subImageIndex);

	return v;
}
