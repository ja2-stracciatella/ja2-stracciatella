#include "GraphicModel.h"

GraphicModel::GraphicModel(const ST::string path_, const uint8_t subImageIndex_) : path(path_), subImageIndex(subImageIndex_) {
}

const ST::string& GraphicModel::getPath() const {
	return this->path;
}

uint8_t GraphicModel::getSubImageIndex() const {
	return this->subImageIndex;
}

GraphicModel GraphicModel::deserialize(const JsonValue &json) {
	auto obj = json.toObject();
	return GraphicModel(
		obj.GetString("path"),
		obj.getOptionalUInt("subImageIndex", 0)
	);
}

JsonValue GraphicModel::serialize() const {
	JsonObject v;

	v.set("path", path);
	v.set("subImageIndex", subImageIndex);

	return v.toValue();
}
