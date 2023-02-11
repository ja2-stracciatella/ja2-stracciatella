#pragma once

#include "Campaign_Types.h"
#include "Json.h"

/**
 * Numbers of bloodcats spawning at designated sectors, by game difficulty
 */
class BloodCatSpawnsModel
{
public:
	BloodCatSpawnsModel(uint8_t sectorId_,
		bool isLair_, bool isArena_,
		int8_t bloodCatsEasy_, int8_t bloodCatsMedium_, int8_t bloodCatsHard_);

	int8_t getSpawnsByDifficulty(uint8_t difficultyLevel) const;
	static BloodCatSpawnsModel* deserialize(const JsonValue& obj);

	const uint8_t sectorId;
	const bool isLair;
	const bool isArena;
	const int8_t bloodCatsSpawnsEasy;
	const int8_t bloodCatsSpawnsMedium;
	const int8_t bloodCatsSpawnsHard;
};
