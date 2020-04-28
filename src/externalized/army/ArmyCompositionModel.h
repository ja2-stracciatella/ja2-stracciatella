#pragma once

#include "Campaign_Types.h"
#include "Strategic_AI.h"

#include "JsonObject.h"
#include "rapidjson/document.h"


class ArmyCompositionModel
{
public:
	ArmyCompositionModel(uint8_t compositionId_, const char* name_, int8_t priority_,
		int8_t adminPercentage_, int8_t elitePercentage_, int8_t troopPercentage_,
		int8_t desiredPopulation_, int8_t startPopulation_
	);

	ARMY_COMPOSITION toArmyComposition() const;

	static std::vector<const ArmyCompositionModel*> deserialize(const rapidjson::Document& root);

	static void validateData(std::vector<const ArmyCompositionModel*> compositions);

	uint8_t compositionId;
	const char* name;
	int8_t priority;
	int8_t adminPercentage;
	int8_t elitePercentage;
	int8_t troopPercentage;
	int8_t desiredPopulation;
	int8_t startPopulation;
};
