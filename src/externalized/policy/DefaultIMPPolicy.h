#pragma once

#include "IMPPolicy.h"
#include "externalized/ItemModel.h"

#include "rapidjson/document.h"

class ItemSystem;

class DefaultIMPPolicy : public IMPPolicy
{
public:
  DefaultIMPPolicy(rapidjson::Document *json, const ItemSystem *itemSystem);

  virtual const std::vector<const ItemModel *> & getInventory() const;
  virtual const std::vector<const ItemModel *> & getGoodShooterItems() const;
  virtual const std::vector<const ItemModel *> & getNormalShooterItems() const;

protected:
  std::vector<const ItemModel *> m_inventory;
  std::vector<const ItemModel *> m_goodShooterItems;
  std::vector<const ItemModel *> m_normalShooterItems;
};
