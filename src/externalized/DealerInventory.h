#pragma once

#include "rapidjson/document.h"

#include <map>

struct ItemModel;
class ItemSystem;

/**
 * Dealer's inventory.
 *
 * It is a list of pairs (item: amount).
 * If amount > 0, the dealer has this amount of items for sell.
 * If amount == 0, the dealer only buys the given item, but not sell.
 *
 * If item is not present in the inventory, the dealer doesn't buy nor sell the item.
 */
class DealerInventory
{
public:
	DealerInventory(rapidjson::Document *json, const ItemSystem *itemSystem);

	/**
	 * Check if item in the inventory.
	 */
	bool hasItem(const ItemModel *item) const;

	/**
	 * Return amount of the given item in the inventory or zero if the item is not present.
	 */
	int getMaxItemAmount(const ItemModel *item) const;

protected:
	std::map<const ItemModel*, int> m_inventory;
};
