#pragma once

#include <vector>

#include "externalized/ItemModel.h"

class ItemSystem;

class IMPPolicy
{
public:
  virtual const std::vector<const ItemModel *> & getInventory() const = 0;
  virtual const std::vector<const ItemModel *> & getGoodShooterItems() const = 0;
  virtual const std::vector<const ItemModel *> & getNormalShooterItems() const = 0;
  // this could be defaulted in C++11
  virtual ~IMPPolicy() {}
};
