#pragma once

#include "TileDat.h"
#include "Json.h"

class TilesetTileIndexModel {
	public:
		constexpr TilesetTileIndexModel(TileTypeDefines type, uint16_t subIndexArg) noexcept
			: tileType{ type }, subIndex{ subIndexArg }
		{
		}

		TileTypeDefines tileType;
		uint16_t subIndex;

		static TilesetTileIndexModel deserialize(const JsonValue &json);
		JsonValue serialize() const;
};
