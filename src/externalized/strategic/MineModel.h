#pragma once

#include "JsonObject.h"
#include <array>
#include <rapidjson/document.h>
#include <string>
#include <vector>

class MineModel
{
public:
	MineModel(const uint8_t mineId, const uint8_t entranceSector_, const uint8_t associatedTownId_, const uint8_t mineType_, const uint16_t minimumMineProduction_,
		const bool headMinerAssigned_, const bool noDepletion_, const bool delayDepletion_,
		const std::vector<std::array<uint8_t, 2>> mineSectors_,
		const int16_t faceDisplayYOffset);
 
	bool isAbandoned() const;

	const uint8_t mineId;
	const uint8_t entranceSector;
	const uint8_t associatedTownId;
	const uint8_t mineType;
	const uint16_t minimumMineProduction;
	const bool headMinerAssigned;
	const bool noDepletion;
	const bool delayDepletion;

	const std::vector<std::array<uint8_t, 2>> mineSectors;

	const int16_t faceDisplayYOffset;
	
	static MineModel* deserialize(uint8_t index, const rapidjson::Value& json);
	static void validateData(std::vector<const MineModel*>& models);
};
