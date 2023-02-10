#include "DefaultStrategicAIPolicy.h"
#include "GameSettings.h"

#include <array>
#include <string_theory/string>

static std::array<int, NUM_DIF_LEVELS> ReadSAIPolicyInt(const JsonObject& r, const ST::string& key)
{
	return std::array<int, NUM_DIF_LEVELS> {
		r.GetInt((key + "_easy").c_str()),
		r.GetInt((key + "_medium").c_str()),
		r.GetInt((key + "_hard"  ).c_str())
	};
}

static std::array<unsigned int, NUM_DIF_LEVELS> ReadSAIPolicyUInt(const JsonObject& r, const ST::string& key)
{
	return std::array<unsigned int, NUM_DIF_LEVELS> {
		r.GetUInt((key + "_easy").c_str()),
		r.GetUInt((key + "_medium").c_str()),
		r.GetUInt((key + "_hard"  ).c_str())
	};
}

DefaultStrategicAIPolicy::DefaultStrategicAIPolicy(const JsonValue& json)
{
	auto r = json.toObject();

	queens_pool_of_troops        = ReadSAIPolicyUInt(r, "queens_pool_of_troops");
	initial_garrison_percentages = ReadSAIPolicyUInt(r, "initial_garrison_percentages");
	min_enemy_group_size         = ReadSAIPolicyUInt(r, "min_enemy_group_size");
	enemy_starting_alert_level   = ReadSAIPolicyUInt(r, "enemy_starting_alert_level");
	enemy_starting_alert_decay   = ReadSAIPolicyUInt(r, "enemy_starting_alert_decay");
	time_evaluate_in_minutes     = ReadSAIPolicyUInt(r, "time_evaluate_in_minutes");
	time_evaluate_variance       = ReadSAIPolicyInt (r, "time_evaluate_variance");
	grace_period_in_hours        = ReadSAIPolicyUInt(r, "grace_period_in_hours");
	patrol_grace_period_in_days  = ReadSAIPolicyUInt(r, "patrol_grace_period_in_days");
	num_aware_battles            = ReadSAIPolicyUInt(r, "num_aware_battles");

	refill_defeated_patrol_groups = r.getOptionalBool("refill_defeated_patrol_groups", false);
}

