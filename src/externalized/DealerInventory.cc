#include "DealerInventory.h"
#include "ItemModel.h"
#include "ItemSystem.h"


DealerInventory::DealerInventory(const JsonValue& json, const ItemSystem *itemSystem)
{
	auto obj = json.toObject();
	for (auto& it : obj.keys())
	{
		const ItemModel *item = itemSystem->getItemByName(it);
		int count = obj.GetInt(it.c_str());
		m_inventory.insert(std::make_pair(item, count));
	}
}

bool DealerInventory::hasItem(const ItemModel *item) const
{
	std::map<const ItemModel*, int>::const_iterator it = m_inventory.find(item);
	return it != m_inventory.end();
}

int DealerInventory::getMaxItemAmount(const ItemModel *item) const
{
	std::map<const ItemModel*, int>::const_iterator it = m_inventory.find(item);
	return (it == m_inventory.end()) ? 0 : it->second;
}
