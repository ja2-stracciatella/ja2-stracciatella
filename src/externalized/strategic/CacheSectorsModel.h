#pragma once
#include "GameSettings.h"
#include <array>
#include <vector>
#include <rapidjson/document.h>

class CacheSectorsModel
{
public:
	CacheSectorsModel(std::vector<uint8_t> sectors_,
		std::array<uint8_t, NUM_DIF_LEVELS> numTroops_,
		std::array<uint8_t, NUM_DIF_LEVELS> numTroopsVariance_);

	static const CacheSectorsModel* deserialize(const rapidjson::Document& doc);

	// randomly pick one sector from list; returns -1 if the list is empty
	int16_t pickSector() const;

	// randomly generate the number of troops guarding a cache sector, according to game difficulty
	uint8_t getNumTroops(uint8_t difficultyLevel) const;

	// the list of all possible cache sectors
	const std::vector<uint8_t> sectors;

protected:
	const std::array<uint8_t, NUM_DIF_LEVELS> numTroops;
	const std::array<uint8_t, NUM_DIF_LEVELS> numTroopsVariance;
};
