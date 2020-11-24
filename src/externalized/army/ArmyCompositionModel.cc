#include "ArmyCompositionModel.h"
#include "Logger.h"
#include <utility>


ArmyCompositionModel::ArmyCompositionModel(uint8_t compositionId_, ST::string name_, int8_t priority_,
		int8_t adminPercentage_, int8_t elitePercentage_, int8_t troopPercentage_,
		int8_t desiredPopulation_, int8_t startPopulation_) :
			compositionId(compositionId_), name(std::move(name_)), priority(priority_),
			adminPercentage(adminPercentage_), elitePercentage(elitePercentage_), troopPercentage(troopPercentage_),
			desiredPopulation(desiredPopulation_), startPopulation(startPopulation_) {}

ARMY_COMPOSITION ArmyCompositionModel::toArmyComposition() const
{
	ARMY_COMPOSITION composition{};
	composition.iReadability = compositionId;
	composition.bPriority = priority;
	composition.bElitePercentage = elitePercentage;
	composition.bTroopPercentage = troopPercentage;
	composition.bAdminPercentage = adminPercentage;
	composition.bDesiredPopulation = desiredPopulation;
	composition.bStartPopulation = startPopulation;

	return composition;
}

std::vector<const ArmyCompositionModel*> ArmyCompositionModel::deserialize(const rapidjson::Document& root)
{
	std::vector<const ArmyCompositionModel*> armyComps;
	for (auto& item : root.GetArray())
	{
		auto obj = JsonObjectReader(item);
		armyComps.push_back(new ArmyCompositionModel(
			obj.GetInt("id"),
			obj.GetString("name"),
			obj.GetInt("priority"),
			obj.GetInt("adminPercentage"),
			obj.GetInt("elitePercentage"),
			obj.GetInt("troopPercentage"),
			obj.GetInt("desiredPopulation"),
			obj.GetInt("startPopulation")
		));
	}
	return armyComps;
}

#define EXPECTS_STR(s) \
	if (comp->name != s) SLOGW(ST::format("Army Composition has an unexpected name. We recommend leaving the default army compositions unchanged. Expected: {}; Actual: {}", comp->name, s));

void ArmyCompositionModel::validateData(const std::vector<const ArmyCompositionModel*> compositions)
{
	if (compositions.size() < NUM_ARMY_COMPOSITIONS || compositions.size() > SAVED_ARMY_COMPOSITIONS)
	{
		// Invalid data will crash the game
		SLOGE(ST::format("Number of Army Composition must be between {} and {}. Got {}.", NUM_ARMY_COMPOSITIONS, SAVED_ARMY_COMPOSITIONS, compositions.size()));
		throw std::runtime_error("Too many or too few Army Compositions");
	}
	for (size_t i = 0; i < compositions.size(); i++)
	{
		// Warn if default compositions are altered. It might work, but no guarantee.
		auto comp = compositions[i];
		if (comp->compositionId != i)
		{
			SLOGW(ST::format("Army Composition has incorrect ID. Expected: {}; Actual: {}", i, comp->compositionId));
		}

		switch (comp->compositionId) // these army-comps are referenced in code
		{
		case QUEEN_DEFENCE: EXPECTS_STR("QUEEN_DEFENCE"); break;
		case MEDUNA_DEFENCE: EXPECTS_STR("MEDUNA_DEFENCE"); break;
		case MEDUNA_SAMSITE: EXPECTS_STR("MEDUNA_SAMSITE"); break;
		case LEVEL1_DEFENCE: EXPECTS_STR("LEVEL1_DEFENCE"); break;
		case LEVEL2_DEFENCE: EXPECTS_STR("LEVEL2_DEFENCE"); break;
		case LEVEL3_DEFENCE: EXPECTS_STR("LEVEL3_DEFENCE"); break;
		case GRUMM_MINE: EXPECTS_STR("GRUMM_MINE"); break;
		case OMERTA_WELCOME_WAGON: EXPECTS_STR("OMERTA_WELCOME_WAGON"); break;
		case TIXA_PRISON: EXPECTS_STR("TIXA_PRISON"); break;
		case ALMA_DEFENCE: EXPECTS_STR("ALMA_DEFENCE"); break;
		case ALMA_MINE: EXPECTS_STR("ALMA_MINE"); break;
		case CAMBRIA_DEFENCE: EXPECTS_STR("CAMBRIA_DEFENCE"); break;
		case CAMBRIA_MINE: EXPECTS_STR("CAMBRIA_MINE"); break;
		case DRASSEN_AIRPORT: EXPECTS_STR("DRASSEN_AIRPORT"); break;
		case DRASSEN_DEFENCE: EXPECTS_STR("DRASSEN_DEFENCE"); break;
		case DRASSEN_MINE: EXPECTS_STR("DRASSEN_MINE"); break;
		case ROADBLOCK: EXPECTS_STR("ROADBLOCK"); break;
		case SANMONA_SMALL: EXPECTS_STR("SANMONA_SMALL"); break;
		}
	}
#undef EXPECTS_STR
}

void ArmyCompositionModel::validateLoadedData(const std::vector<ARMY_COMPOSITION>&armyCompositions)
{
	if (armyCompositions.size() < NUM_ARMY_COMPOSITIONS)
	{
		SLOGW(ST::format("There are too few loaded Army Compositions. Expected: {}; Actual: {}", NUM_ARMY_COMPOSITIONS, armyCompositions.size()));
	}
	for (size_t i = 0; i < armyCompositions.size(); i++)
	{
		// Warn if default compositions are altered. It might work, but no guarantee.
		auto comp = armyCompositions[i];
		if (comp.iReadability != static_cast<int32_t>(i))
		{
			SLOGW(ST::format("Army Composition has incorrect ID. The save might be corrupted. Expected: {}; Actual: {}", i, comp.iReadability));
		}
	}
}
