#pragma once

#include "ItemModel.h"
#include "ItemSystem.h"
#include "rapidjson/document.h"

#include <map>

class MapItemReplacementModel
{
public:
	static std::map<ItemId, ItemId> deserialize(const rapidjson::Document* doc, const ItemSystem* items);
};
