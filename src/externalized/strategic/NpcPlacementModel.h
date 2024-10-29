#pragma once

#include "Json.h"
#include "MercSystem.h"
#include <vector>

/**
 * Placements of randomly-placed NPCs
 */
class NpcPlacementModel
{
public:
	NpcPlacementModel(uint8_t profileId_, std::vector<uint8_t>&& sectorIds_,
		bool isPlacedAtStart_, bool useAlternateMap_, bool isSciFiOnly_);

	static NpcPlacementModel* deserialize(const JsonValue& element, const MercSystem* mercSystem);

	// Randomly pick one from the possible sectors. Returns -1 if sectorIds is empty
	int16_t pickPlacementSector() const;

	const uint8_t profileId;
	const std::vector<uint8_t> sectorIds;
	const bool isPlacedAtStart;
	const bool useAlternateMap;
	const bool isSciFiOnly;
};
