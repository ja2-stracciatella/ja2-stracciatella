#pragma once

#include "TileDat.h"
#include "JsonObject.h"

#include <string_theory/string>

class TilesetTileIndexModel {
	public:
		TilesetTileIndexModel(TileTypeDefines type, uint16_t subIndex);

		TileTypeDefines tileType;
		uint16_t subIndex;

		static TilesetTileIndexModel deserialize(JsonObjectReader &obj);
		JsonObject serialize(rapidjson::Document::AllocatorType& allocator) const;
};
