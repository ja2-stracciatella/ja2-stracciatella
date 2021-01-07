#include "DefaultStrategicAIPolicy.h"

#include "GameSettings.h"
#include "JsonObject.h"
#include <array>
#include <string_theory/string>

template<typename T>
static std::array<T, NUM_DIF_LEVELS> ReadSAIPolicy(const rapidjson::Document& json, const std::string& key)
{
	static_assert(std::is_integral<T>::value);
	return std::array<T, NUM_DIF_LEVELS> {
		json[(key + "_easy"  ).c_str()].Get<T>(),
		json[(key + "_medium").c_str()].Get<T>(),
		json[(key + "_hard"  ).c_str()].Get<T>()
	};
}

DefaultStrategicAIPolicy::DefaultStrategicAIPolicy(rapidjson::Document* json)
{
	queens_pool_of_troops        = ReadSAIPolicy<unsigned int>(*json, "queens_pool_of_troops");
	initial_garrison_percentages = ReadSAIPolicy<unsigned int>(*json, "initial_garrison_percentages");
	min_enemy_group_size         = ReadSAIPolicy<unsigned int>(*json, "min_enemy_group_size");
	enemy_starting_alert_level   = ReadSAIPolicy<unsigned int>(*json, "enemy_starting_alert_level");
	enemy_starting_alert_decay   = ReadSAIPolicy<unsigned int>(*json, "enemy_starting_alert_decay");
	time_evaluate_in_minutes     = ReadSAIPolicy<unsigned int>(*json, "time_evaluate_in_minutes");
	time_evaluate_variance       = ReadSAIPolicy<int>         (*json, "time_evaluate_variance");
	grace_period_in_hours        = ReadSAIPolicy<unsigned int>(*json, "grace_period_in_hours");
	patrol_grace_period_in_days  = ReadSAIPolicy<unsigned int>(*json, "patrol_grace_period_in_days");
	num_aware_battles            = ReadSAIPolicy<unsigned int>(*json, "num_aware_battles");
}

