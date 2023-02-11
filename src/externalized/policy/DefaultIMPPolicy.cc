#include "DefaultIMPPolicy.h"

#include "ItemSystem.h"
#include "JsonUtility.h"

#include <string_theory/string>

struct ItemModel;

static void readListOfItems(const JsonValue& value, std::vector<const ItemModel *> &items, const ItemSystem *itemSystem)
{
	std::vector<ST::string> strings;
	JsonUtility::parseListStrings(value, strings);
	for (const ST::string &str : strings)
	{
		items.push_back(itemSystem->getItemByName(str));
	}
}

DefaultIMPPolicy::DefaultIMPPolicy(const JsonValue& json, const ItemSystem *itemSystem)
{
	auto r = json.toObject();

	JsonUtility::parseListStrings(r["activation_codes"], m_activationCodes);

	m_startingLevel = r.getOptionalUInt("starting_level", 1);

	readListOfItems(r["inventory"], m_inventory, itemSystem);

	readListOfItems(r["if_good_shooter"], m_goodShooterItems, itemSystem);
	readListOfItems(r["if_normal_shooter"], m_normalShooterItems, itemSystem);
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
