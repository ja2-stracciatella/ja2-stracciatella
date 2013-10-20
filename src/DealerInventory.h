#pragma once

#include <map>
#include <string>

#include "rapidjson/document.h"

struct ItemModel;
class ItemSystem;

class DealerInventory
{
public:
  DealerInventory(rapidjson::Document *json, const ItemSystem *itemSystem);

  int getMaxItemAmount(const ItemModel *item) const;

protected:
  std::map<const ItemModel*, int> m_inventory;
};
