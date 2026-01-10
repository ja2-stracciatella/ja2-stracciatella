#pragma once

#include "IMPPolicy.h"
#include "ItemModel.h"
#include "Json.h"

#include <string_theory/string>
#include <vector>

class ItemSystem;

class DefaultIMPPolicy : public IMPPolicy
{
public:
	DefaultIMPPolicy(const JsonValue& json, const ItemSystem *itemSystem);

	virtual bool isCodeAccepted(const ST::string& code) const;
	virtual uint8_t getStartingLevel() const;
	virtual const std::vector<IMPStartingItemSet>& getInventory() const;

protected:
	uint8_t m_startingLevel;
	std::vector<ST::string> m_activationCodes;
	std::vector<IMPStartingItemSet> m_inventory;
};
