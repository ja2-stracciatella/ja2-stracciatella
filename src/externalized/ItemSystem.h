#pragma once

#include <string>

struct ItemModel;

class ItemSystem
{
public:
	virtual const ItemModel* getItemByName(const std::string &internalName) const = 0;
};
