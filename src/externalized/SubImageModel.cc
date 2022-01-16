#include "SubImageModel.h"

SubImageModel::SubImageModel(const ST::string path_, const uint8_t subImageIndex_) : path(path_), subImageIndex(subImageIndex_) {
}

const ST::string& SubImageModel::getPath() const {
    return this->path;
}

uint8_t SubImageModel::getSubImageIndex() const {
    return this->subImageIndex;
}

SubImageModel SubImageModel::deserialize(JsonObjectReader &obj) {
    return SubImageModel(
        obj.GetString("path"),
        obj.GetUInt("subImageIndex")
    );
}

JsonObject SubImageModel::serialize(rapidjson::Document::AllocatorType& allocator) const {
    JsonObject v(allocator);

    v.AddMember("path", path);
    v.AddMember("subImageIndex", subImageIndex);

    return v;
}