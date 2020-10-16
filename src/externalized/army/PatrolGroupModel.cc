#include "PatrolGroupModel.h"
#include "Campaign_Types.h"
#include "JsonUtility.h"

void ReadPatrolPoints(const rapidjson::Value& arr, uint8_t (&points)[4])
{
	if (!arr.IsArray())
	{
		throw std::runtime_error("Array is expected");
	}
	auto arraySize = arr.Size();
	if (arraySize < 2 || arraySize > 4)
	{
		throw std::runtime_error("Patrol Group must have at least 2 and at most 4 points");
	}
	for (rapidjson::SizeType i = 0; i < arraySize; i++)
	{
		auto sector = arr[i].GetString();
		points[i] = JsonUtility::parseSectorID(sector);
	}
}

PATROL_GROUP PatrolGroupModel::deserialize(const rapidjson::Value& val)
{
	PATROL_GROUP g{};
	g.bSize = static_cast<INT8>(val["size"].GetInt()); 
	g.bPriority = static_cast<INT8>(val["priority"].GetInt());
	g.bFillPermittedAfterDayMod100 = -1;
	ReadPatrolPoints(val["points"], g.ubSectorID);

	return g;
}

void PatrolGroupModel::validateData(std::vector<PATROL_GROUP> patrolGroups)
{
	if (patrolGroups.size() > SAVED_PATROL_GROUPS)
	{
		SLOGE(ST::format("There cannot be more than {} Patrol Groups", SAVED_PATROL_GROUPS));
		throw std::runtime_error("Too many Patrol Groups");
	}
}
