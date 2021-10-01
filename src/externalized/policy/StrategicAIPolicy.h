#pragma once

#include "GameSettings.h"
#include "GameInstance.h"
#include <array>
#include <stdint.h>
#include <string_theory/string>

// gets the Strategic AI policy value for the current game difficulty
#define saipolicy(element) ( GCM->getStrategicAIPolicy()->element )
#define saipolicy_by_diff(element) ( saipolicy(element)[gGameOptions.ubDifficultyLevel - 1] )

// Refer to strategic-ai-policy.json for the definition of each value
class StrategicAIPolicy
{
public:
	std::array<unsigned int, NUM_DIF_LEVELS>    queens_pool_of_troops;
	std::array<unsigned int, NUM_DIF_LEVELS>    initial_garrison_percentages;
	std::array<unsigned int, NUM_DIF_LEVELS>    min_enemy_group_size;
	std::array<unsigned int, NUM_DIF_LEVELS>    enemy_starting_alert_level;
	std::array<unsigned int, NUM_DIF_LEVELS>    enemy_starting_alert_decay;
	std::array<unsigned int, NUM_DIF_LEVELS>    time_evaluate_in_minutes;
	std::array<int,          NUM_DIF_LEVELS>    time_evaluate_variance;
	std::array<unsigned int, NUM_DIF_LEVELS>    grace_period_in_hours;
	std::array<unsigned int, NUM_DIF_LEVELS>    patrol_grace_period_in_days;
	std::array<unsigned int, NUM_DIF_LEVELS>    num_aware_battles;

	bool refill_defeated_patrol_groups;
};
