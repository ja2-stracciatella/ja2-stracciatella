#pragma once

#include <vector>

#include "ItemModel.h"

class ItemSystem;

class IMPPolicy
{
public:
	virtual uint8_t getStartingLevel() const = 0;
	virtual const std::vector<const ItemModel *> & getInventory() const = 0;
	virtual const std::vector<const ItemModel *> & getGoodShooterItems() const = 0;
	virtual const std::vector<const ItemModel *> & getNormalShooterItems() const = 0;
	// this could be defaulted in C++11
	virtual ~IMPPolicy() {}
};
