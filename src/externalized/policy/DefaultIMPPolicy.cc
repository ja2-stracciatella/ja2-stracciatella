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

static bool readIsMale(const JsonObject& json)
{
	return json.GetString("gender") == "MALE";
}


// an [x, y] pair, the same shape the merc profile and RPC face offsets use
static void readOffset(const JsonValue& value, uint16_t& x, uint16_t& y)
{
	auto const xy = value.toVec();
	x = xy[0].toUInt();
	y = xy[1].toUInt();
}


static void readVoices(const JsonValue& value, std::vector<IMPVoice>& voices)
{
	for (auto& entry : value.toVec())
	{
		auto obj = entry.toObject();
		IMPVoice voice;
		voice.profile = obj.GetUInt("profile");
		voice.isMale  = readIsMale(obj);
		voices.push_back(std::move(voice));
	}
}


static void readPortraits(const JsonValue& value, std::vector<IMPPortrait>& portraits)
{
	for (auto& entry : value.toVec())
	{
		auto obj = entry.toObject();
		IMPPortrait portrait;
		portrait.face    = obj.GetUInt("face");
		portrait.isMale  = readIsMale(obj);
		portrait.bigBody = obj.getOptionalBool("big_body");
		portrait.skin    = obj.GetString("skin");
		portrait.hair    = obj.GetString("hair");
		readOffset(obj["eyes"],  portrait.eyesX,  portrait.eyesY);
		readOffset(obj["mouth"], portrait.mouthX, portrait.mouthY);
		portraits.push_back(std::move(portrait));
	}
}


DefaultIMPPolicy::DefaultIMPPolicy(const JsonValue& json, const ItemSystem *itemSystem)
{
	auto r = json.toObject();

	JsonUtility::parseListStrings(r["activation_codes"], m_activationCodes);

	m_startingLevel = r.getOptionalUInt("starting_level", 1);

	readVoices(r["voices"], m_voices);
	readPortraits(r["portraits"], m_portraits);

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

const std::vector<IMPVoice>& DefaultIMPPolicy::getVoices() const
{
	return m_voices;
}

const std::vector<IMPPortrait>& DefaultIMPPolicy::getPortraits() const
{
	return m_portraits;
}
