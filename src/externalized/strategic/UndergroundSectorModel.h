#pragma once

#include "Campaign_Types.h"
#include "GameSettings.h"
#include "Json.h"

#include <array>

const char DIRECTION_NORTH = 'N';
const char DIRECTION_SOUTH = 'S';
const char DIRECTION_EAST = 'E';
const char DIRECTION_WEST = 'W';

// Definitions of underground sectors, useed by Campaign_Init. During init the data are converted to UNDERGROUND_SECTORINFO structures.
class UndergroundSectorModel
{
public:
	UndergroundSectorModel(
		uint8_t sectorId_, uint8_t sectorZ_, uint8_t adjacentSectors_,
		std::array<uint8_t, NUM_DIF_LEVELS> numTroops_, std::array<uint8_t, NUM_DIF_LEVELS> numElites_, std::array<uint8_t, NUM_DIF_LEVELS> numCreatures_,
		std::array<uint8_t, NUM_DIF_LEVELS> numTroopsVariance_, std::array<uint8_t, NUM_DIF_LEVELS> numElitesVariance_, std::array<uint8_t, NUM_DIF_LEVELS> numCreaturesVariance_
	);
	UNDERGROUND_SECTORINFO* createUndergroundSectorInfo(uint8_t difficultyLevel) const;

	static UndergroundSectorModel* deserialize(const JsonValue& obj);
	static void validateData(const std::vector<const UndergroundSectorModel*>& ugSectors);

	uint8_t sectorId;
	uint8_t sectorZ;
	uint8_t	adjacentSectors;

protected:
	std::array<uint8_t, NUM_DIF_LEVELS> numTroops;
	std::array<uint8_t, NUM_DIF_LEVELS> numElites;
	std::array<uint8_t, NUM_DIF_LEVELS> numCreatures;

	std::array<uint8_t, NUM_DIF_LEVELS> numTroopsVariance;
	std::array<uint8_t, NUM_DIF_LEVELS> numElitesVariance;
	std::array<uint8_t, NUM_DIF_LEVELS> numCreaturesVariance;
};
