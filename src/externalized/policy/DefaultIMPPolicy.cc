#include "DefaultIMPPolicy.h"

#include "ItemSystem.h"
#include "JsonObject.h"
#include "JsonUtility.h"

#include <string_theory/string>

struct ItemModel;

static void readListOfItems(rapidjson::Value &value, std::vector<const ItemModel *> &items, const ItemSystem *itemSystem)
{
	std::vector<ST::string> strings;
	JsonUtility::parseListStrings(value, strings);
	for (const ST::string &str : strings)
	{
		items.push_back(itemSystem->getItemByName(str));
	}
}

DefaultIMPPolicy::DefaultIMPPolicy(rapidjson::Document *json, const ItemSystem *itemSystem)
{
	JsonUtility::parseListStrings((*json)["activation_codes"], m_activationCodes);

	JsonObjectReader r(*json);
	m_startingLevel = r.getOptionalUInt("starting_level", 1);

	readListOfItems((*json)["inventory"], m_inventory, itemSystem);

	readListOfItems((*json)["if_good_shooter"], m_goodShooterItems, itemSystem);
	readListOfItems((*json)["if_normal_shooter"], m_normalShooterItems, itemSystem);
}

bool DefaultIMPPolicy::isCodeAccepted(const ST::string& code) const
{
	for (auto& s : m_activationCodes)
	{
		if (s == code) return true;
	}
	return false;
}

uint8_t DefaultIMPPolicy::getStartingLevel() const
{
	return m_startingLevel;
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
