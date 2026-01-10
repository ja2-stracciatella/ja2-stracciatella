#pragma once

#include "stdint.h"
#include <string_theory/string>
#include <vector>
#include <map>

class ItemModel;
class ItemsContainer;

class ItemSystem
{
public:
	// Similar to std::nothrow and nothrow_t, defined here so we do not have
	// to include <new> almost everywhere.
	struct nothrow_t{ explicit nothrow_t() = default; };
	static inline nothrow_t const nothrow{};

	// Returns a range that can be iterated over, without the NOTHING item
	virtual const ItemsContainer& getItems() const = 0;

	// Returns an item by internal name
	virtual const ItemModel* getItemByName(const ST::string &internalName) const = 0;

	// Returns an item by numeric id
	// Throws an exception if itemIndex is invalid
	virtual const ItemModel* getItem(uint16_t itemIndex) const = 0;

	// Returns an item by numeric id
	// Returns nullptr if itemIndex is invalid
	virtual const ItemModel* getItem(uint16_t itemIndex, nothrow_t const&) const noexcept = 0;

	// Returns all paths to small inventory images
	virtual std::vector<ST::string> getAllSmallInventoryGraphicPaths() const = 0;

	// Returns item replacements for maps
	virtual const std::map<uint16_t, uint16_t>& getMapItemReplacements() const = 0;

	// Returns a key for
	virtual const ItemModel* getKeyItemForKeyId(uint16_t usKeyItem) const = 0;
};
