#include "GraphicModel.h"
#include <utility>

GraphicModel::GraphicModel(ST::string path_, uint16_t subImageIndex_)
	: path(std::move(path_)), subImageIndex(subImageIndex_)
{
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
