#pragma once

#include "ItemModel.h"
#include "ItemSystem.h"
#include "rapidjson/document.h"

#include <map>

class MapItemReplacementModel
{
public:
	static std::map<uint16_t, uint16_t> deserialize(const rapidjson::Document* doc, const ItemSystem* items);
};
