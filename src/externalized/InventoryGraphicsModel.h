#pragma once

#include "GraphicModel.h"

#include <string_theory/string>

class InventoryGraphicsModel {
	public:
		InventoryGraphicsModel(GraphicModel small, GraphicModel big);

		GraphicModel small;
		GraphicModel big;

		static InventoryGraphicsModel deserialize(const JsonValue &json);
		JsonValue serialize() const;
};
