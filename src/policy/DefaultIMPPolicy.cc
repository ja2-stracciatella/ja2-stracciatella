#include "DefaultIMPPolicy.h"

#include <string>

#include "boost/foreach.hpp"

#include "ItemSystem.h"
#include "JsonUtility.h"

struct ItemModel;

static void readListOfItems(rapidjson::Value &value, std::vector<const ItemModel *> &items, const ItemSystem *itemSystem)
{
  std::vector<std::string> strings;
  JsonUtility::parseListStrings(value, strings);
  BOOST_FOREACH(const std::string &str, strings)
  {
    items.push_back(itemSystem->getItemByName(str));
  }
}

DefaultIMPPolicy::DefaultIMPPolicy(rapidjson::Document *json, const ItemSystem *itemSystem)
{
  readListOfItems((*json)["inventory"], m_inventory, itemSystem);

  readListOfItems((*json)["if_good_shooter"], m_goodShooterItems, itemSystem);
  readListOfItems((*json)["if_normal_shooter"], m_normalShooterItems, itemSystem);
}

const std::vector<const ItemModel *> & DefaultIMPPolicy::getInventory() const
{
  return m_inventory;
}

const std::vector<const ItemModel *> & DefaultIMPPolicy::getGoodShooterItems() const
{
  return m_goodShooterItems;
}

const std::vector<const ItemModel *> & DefaultIMPPolicy::getNormalShooterItems() const
{
  return m_normalShooterItems;
}
