#pragma once

#include "GraphicModel.h"
#include "JsonObject.h"

#include <string_theory/string>

class InventoryGraphicsModel {
	public:
		InventoryGraphicsModel(GraphicModel small, GraphicModel big);

		GraphicModel small;
		GraphicModel big;

		static InventoryGraphicsModel deserialize(JsonObjectReader &obj);
		JsonObject serialize(rapidjson::Document::AllocatorType& allocator) const;
};
