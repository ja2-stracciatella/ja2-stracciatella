#include "PatrolGroupModel.h"
#include "Campaign_Types.h"
#include "JsonUtility.h"

static void ReadPatrolPoints(const JsonValue& root, uint8_t (&points)[4])
{
	auto i = 0;
	for (const auto& sector : root.toVec())
	{
		points[i] = JsonUtility::parseSectorID(sector);
		i++;
	}
}

PATROL_GROUP PatrolGroupModel::deserialize(const JsonValue& root)
{
	auto reader = root.toObject();
	PATROL_GROUP g{};
	g.bSize = static_cast<INT8>(reader.GetInt("size"));
	g.bPriority = static_cast<INT8>(reader.GetInt("priority"));
	g.bFillPermittedAfterDayMod100 = -1;

	ReadPatrolPoints(reader["points"], g.ubSectorID);

	return g;
}

void PatrolGroupModel::validateData(const std::vector<PATROL_GROUP>& patrolGroups)
{
	if (patrolGroups.size() > SAVED_PATROL_GROUPS)
	{
		SLOGE("There cannot be more than {} Patrol Groups", SAVED_PATROL_GROUPS);
		throw std::runtime_error("Too many Patrol Groups");
	}
}
