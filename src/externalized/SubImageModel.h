#pragma once

#include "JsonObject.h"

#include <string_theory/string>

class SubImageModel {
    public:
        SubImageModel(const ST::string path_, const uint8_t subImageIndex);

        const ST::string& getPath() const;
        uint8_t getSubImageIndex() const;

        static SubImageModel deserialize(JsonObjectReader &obj);
        JsonObject serialize(rapidjson::Document::AllocatorType& allocator) const;
    private:
        ST::string path;
        uint8_t subImageIndex;
};