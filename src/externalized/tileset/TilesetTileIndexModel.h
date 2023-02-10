#pragma once

#include "TileDat.h"
#include "Json.h"

#include <string_theory/string>

class TilesetTileIndexModel {
	public:
		TilesetTileIndexModel(TileTypeDefines type, uint16_t subIndex);

		TileTypeDefines tileType;
		uint16_t subIndex;

		static TilesetTileIndexModel deserialize(const JsonValue &json);
		JsonValue serialize() const;
};
