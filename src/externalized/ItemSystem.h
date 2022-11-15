#pragma once

#include <string_theory/string>
#include <vector>
#include <map>

struct ItemModel;

class ItemSystem
{
public:
	// Returns an item by internal name
	virtual const ItemModel* getItemByName(const ST::string &internalName) const = 0;

	// Returns an item by numeric id
	virtual const ItemModel* getItem(ItemId itemIndex) const = 0;

	// Returns all paths to small inventory images
	virtual std::vector<ST::string> getAllSmallInventoryGraphicPaths() const = 0;

	// Returns item replacements for maps
	virtual const std::map<ItemId, ItemId> getMapItemReplacements() const = 0;

	// Returns a key for
	virtual const ItemModel* getKeyItemForKeyId(uint16_t usKeyItem) const = 0;
};
