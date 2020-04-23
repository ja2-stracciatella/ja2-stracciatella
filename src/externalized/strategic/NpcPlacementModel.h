#pragma once

#include "JsonObject.h"
#include <vector>

/**
 * Placements of randomly-placed NPCs
 */
class NpcPlacementModel
{
public:
	NpcPlacementModel(uint8_t profileId_, std::vector<uint8_t> sectorIds_);

	static NpcPlacementModel* deserialize(const rapidjson::Value& element);

	const uint8_t profileId;
	const std::vector<uint8_t> sectorIds;
};