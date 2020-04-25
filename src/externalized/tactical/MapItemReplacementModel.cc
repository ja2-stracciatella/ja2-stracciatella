#include "MapItemReplacementModel.h"

static const ItemModel* GetItemFromValue(const rapidjson::Value& val, const ItemSystem* items)
{
	if (val.IsString())
	{
		return items->getItemByName(val.GetString());
	}
	if (val.IsInt())
	{
		return items->getItem(val.GetInt());
	}

	throw std::runtime_error("Value is not a valid item identifier");
}

std::map<uint16_t, uint16_t> MapItemReplacementModel::deserialize(const rapidjson::Document* doc, const ItemSystem* items)
{
	std::map<uint16_t, uint16_t> mapping;
	for (auto& m : doc->GetArray())
	{
		const ItemModel* fromItem = GetItemFromValue(m["from"], items);
		const ItemModel* toItem = GetItemFromValue(m["to"], items);
		
		if (fromItem == NULL || toItem == NULL)
		{
			throw std::runtime_error("Item not found");
		}
		mapping.insert(std::make_pair(fromItem->getItemIndex(), toItem->getItemIndex()));
	}

	return mapping;
}
