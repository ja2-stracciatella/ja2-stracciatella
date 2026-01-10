#pragma once

#include <string>
#include <variant>
#include <vector>

#include "enums.h"
#include "ItemModel.h"
#include "Random.h"
#include "Soldier_Profile_Type.h"

class IMPItemCondition;
class IMPStartingItemSet;
class ItemSystem;
using Condition = std::variant<uint8_t, std::string>;

class IMPPolicy
{
public:
	virtual bool isCodeAccepted(const ST::string& code) const = 0;
	virtual uint8_t getStartingLevel() const = 0;
	virtual const std::vector<IMPStartingItemSet>& getInventory() const = 0;
	virtual ~IMPPolicy() = default;
};

class IMPItemCondition
{
	Condition one, two;
public:
	IMPItemCondition(Condition& a, Condition& b) : one(std::move(a)), two(std::move(b)) {};
	bool Evaluate(const MERCPROFILESTRUCT& p) const
	{
		// we don't know in what order the check and value are, but it carries meaning
		std::string check;
		uint8_t value;
		bool reverse = false;
		if (std::holds_alternative<uint8_t>(one)) { // int, string
			value = std::get<uint8_t>(one);
			check = std::get<std::string>(two);
			reverse = true;
		} else if (std::holds_alternative<uint8_t>(two)) { // string, int
			value = std::get<uint8_t>(two);
			check = std::get<std::string>(one);
		} else { // string, string
			check = std::get<std::string>(one);
			std::string check2 = std::get<std::string>(two);
			if (check == "RND100") {
				value = PreRandom(100);
				check = check2;
			} else if (check2 == "RND100") {
				value = PreRandom(100);
			} else if (check == "SKILL") { // skill presence check
				uint8_t checkVal = Internals::getSkillTraitEnumFromString(check2);
				return HasSkillTrait(p, static_cast<SkillTrait>(checkVal));
			} else {
				SLOGE("Skipping items due to bad I.M.P. starting gear condition in imp.json: {} {}", check, value);
				return false;
			}
		}

		// convert check to merc's actual attribute value
		Attributes attrIdx = Internals::getAttributeEnumFromString(check);
		if (attrIdx == Attributes::NUM_ATTRIBUTES) return false;

		uint8_t checkVal = static_cast<uint8_t>(Attribute(p, attrIdx + 1)); // +1 is to skip the extra LifeMax entry
		return reverse ? (value >= checkVal) : (checkVal >= value);
	}
};

struct IMPStartingItemSet
{
	std::vector<IMPItemCondition> conditions;
	std::optional<uint8_t> slot;
	std::vector<const ItemModel *> items;

	bool Evaluate(const MERCPROFILESTRUCT& p) const
	{
		for (const auto& condition : conditions) {
			if (!condition.Evaluate(p)) return false;
		}
		return true;
	}
};
