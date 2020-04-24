#pragma once

#include <string_theory/string>

struct ItemModel;

class ItemSystem
{
public:
	virtual const ItemModel* getItemByName(const ST::string &internalName) const = 0;
};
