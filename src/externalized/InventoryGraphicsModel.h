#pragma once

#include "SubImageModel.h"
#include "JsonObject.h"

#include <string_theory/string>

class InventoryGraphicsModel {
    public:
        InventoryGraphicsModel(SubImageModel small, ST::string big);

        SubImageModel small;
        ST::string    big;

        static InventoryGraphicsModel deserialize(JsonObjectReader &obj);
        JsonObject serialize(rapidjson::Document::AllocatorType& allocator) const;
};