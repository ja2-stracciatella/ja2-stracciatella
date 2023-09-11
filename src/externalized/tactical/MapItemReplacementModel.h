#pragma once

#include "ItemSystem.h"
#include "Json.h"

#include <map>

class MapItemReplacementModel
{
public:
	static std::map<uint16_t, uint16_t> deserialize(const JsonValue& doc, const ItemSystem* items);
};
