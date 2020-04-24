#include "DealerInventory.h"

#include <stdexcept>

#include "sgp/StrUtils.h"

#include "ItemModel.h"
#include "ItemSystem.h"

DealerInventory::DealerInventory(rapidjson::Document *json, const ItemSystem *itemSystem)
{
	for(rapidjson::Document::MemberIterator it = json->MemberBegin(); it != json->MemberEnd(); it++)
	{
		if(!it->value.IsInt())
		{
			throw std::runtime_error(FormattedString("Property '%s' should have integer value", it->name.GetString()).to_std_string());
		}
		const ItemModel *item = itemSystem->getItemByName(it->name.GetString());
		int count = it->value.GetInt();
		// printf("%s: %d\n", item->getInternalName().c_str(), count);
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
