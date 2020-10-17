#pragma once

#include "Strategic_AI.h"

#include "rapidjson/document.h"
#include <vector>

class PatrolGroupModel
{
public:
	static PATROL_GROUP deserialize(const rapidjson::Value& val);
	static void validateData(const std::vector<PATROL_GROUP>& patrolGroups);
};
