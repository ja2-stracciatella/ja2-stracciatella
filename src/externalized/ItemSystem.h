#pragma once

#include <string_theory/string>

struct ItemModel;

class ItemSystem
{
public:
	virtual const ItemModel* getItemByName(const ST::string &internalName) const = 0;
	virtual const ItemModel* getItem(uint16_t itemIndex) const = 0;
};
