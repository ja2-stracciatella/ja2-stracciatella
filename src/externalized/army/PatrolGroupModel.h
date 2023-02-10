#pragma once

#include "Strategic_AI.h"
#include "Json.h"

#include <vector>

class PatrolGroupModel
{
public:
	static PATROL_GROUP deserialize(const JsonValue& root);
	static void validateData(const std::vector<PATROL_GROUP>& patrolGroups);
};
