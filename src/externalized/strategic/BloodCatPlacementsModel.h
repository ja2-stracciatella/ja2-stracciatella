#pragma once

#include "Campaign_Types.h"
#include "Json.h"

/**
 * The number of BloodCat placements (for ambush) per sector
 */
class BloodCatPlacementsModel
{
public:
	BloodCatPlacementsModel(uint8_t sectorId_, int8_t bloodCatPlacements_);

	static BloodCatPlacementsModel* deserialize(const JsonValue& obj);

	uint8_t sectorId;
	int8_t bloodCatPlacements;
};

