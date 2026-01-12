#include "DefaultIMPPolicy.h"

#include "ItemSystem.h"
#include "JsonUtility.h"

#include <string_theory/string>

struct ItemModel;

static void readListOfItems(const JsonValue& value, std::vector<IMPStartingItemSet>& sets, const ItemSystem* itemSystem)
{
	for (auto& set : value.toVec()) {
		IMPStartingItemSet itemSet;
		auto setObject = set.toObject();
		if (setObject.has("slot")) {
			itemSet.slot = Internals::getInventorySlotEnumFromString(setObject.GetString("slot"));
		}

		std::vector<ST::string> strings;
		JsonUtility::parseListStrings(setObject["items"], strings);
		for (const ST::string &str : strings)
		{
			itemSet.items.push_back(itemSystem->getItemByName(str));
		}

		auto PreserveType = [](const JsonValue& conVal)
		{
			Condition con;
			if (conVal.isUInt()) {
				con.emplace<uint8_t>(conVal.toUInt());
			} else {
				con.emplace<std::string>(conVal.toString().to_std_string());
			}
			return con;
		};
		if (!setObject.has("conditions")) {
			sets.push_back(std::move(itemSet));
			continue;
		}

		AssertMsg(setObject["conditions"].toVec().size() % 2 == 0, "IMP starting item condition list does not have an even number of values!");
		std::vector<Condition> conditions;
		Condition con1, con2;
		int count = 0;
		for (auto& con : setObject["conditions"].toVec()) {
			if (count % 2 == 0) {
				con1 = PreserveType(con);
				++count;
				continue;
			} else {
				con2 = PreserveType(con);
				++count;
			}

			itemSet.conditions.emplace_back(con1, con2);
		}

		sets.push_back(std::move(itemSet));
	}
}

DefaultIMPPolicy::DefaultIMPPolicy(const JsonValue& json, const ItemSystem *itemSystem)
{
	auto r = json.toObject();

	JsonUtility::parseListStrings(r["activation_codes"], m_activationCodes);

	m_startingLevel = r.getOptionalUInt("starting_level", 1);

	readListOfItems(r["inventory"], m_inventory, itemSystem);
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

const std::vector<IMPStartingItemSet>& DefaultIMPPolicy::getInventory() const
{
	return m_inventory;
}
