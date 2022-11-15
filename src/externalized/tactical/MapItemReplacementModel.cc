#include "MapItemReplacementModel.h"

static const ItemModel* GetItemFromValue(const rapidjson::Value& val, const ItemSystem* items)
{
	if (val.IsString())
	{
		return items->getItemByName(val.GetString());
	}
	if (val.IsInt())
	{
		return items->getItem(ItemId(val.GetUint()));
	}

	throw std::runtime_error("Value is not a valid item identifier");
}

std::map<ItemId, ItemId> MapItemReplacementModel::deserialize(const rapidjson::Document* doc, const ItemSystem* items)
{
	std::map<ItemId, ItemId> mapping;
	for (auto& m : doc->GetArray())
	{
		const ItemModel* fromItem = GetItemFromValue(m["from"], items);
		const ItemModel* toItem = GetItemFromValue(m["to"], items);

		if (fromItem == NULL || toItem == NULL)
		{
			throw std::runtime_error("Item not found");
		}
		mapping.insert(std::make_pair(ItemId(fromItem->getItemIndex()), ItemId(toItem->getItemIndex())));
	}

	return mapping;
}
