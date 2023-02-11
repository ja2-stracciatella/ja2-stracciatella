#include "MapItemReplacementModel.h"

static const ItemModel* GetItemFromValue(const JsonValue& val, const ItemSystem* items)
{
	if (val.isString()) {
		auto str = val.toString();
		return items->getItemByName(str);
	}
	if (val.isInt()) {
		auto id = val.toInt();
		return items->getItem(id);
	}
	throw std::runtime_error("value should be internal name or item id");
}

std::map<uint16_t, uint16_t> MapItemReplacementModel::deserialize(const JsonValue& json, const ItemSystem* items)
{
	std::map<uint16_t, uint16_t> mapping;
	for (auto& item : json.toVec())
	{
		auto m = item.toObject();
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
