#pragma once

#include "Campaign_Types.h"
#include "Strategic_AI.h"
#include "Json.h"

#include <string_theory/string>


class ArmyCompositionModel
{
public:
	ArmyCompositionModel(uint8_t compositionId_, ST::string name_, int8_t priority_,
		int8_t adminPercentage_, int8_t elitePercentage_, int8_t troopPercentage_,
		int8_t desiredPopulation_, int8_t startPopulation_
	);

	ARMY_COMPOSITION toArmyComposition() const;

	static std::vector<const ArmyCompositionModel*> deserialize(const JsonValue& root);

	static void validateData(const std::vector<const ArmyCompositionModel*> compositions);

	/** Validate the ARMY_COMPOSITION entries loaded from a saved game */
	static void validateLoadedData(const std::vector<ARMY_COMPOSITION>& armyCompositions);

	uint8_t compositionId;
	ST::string name;
	int8_t priority;
	int8_t adminPercentage;
	int8_t elitePercentage;
	int8_t troopPercentage;
	int8_t desiredPopulation;
	int8_t startPopulation;
};
