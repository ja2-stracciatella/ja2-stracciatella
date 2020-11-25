#pragma once

#include "IMPPolicy.h"
#include "ItemModel.h"

#include "rapidjson/document.h"
#include <string_theory/string>
#include <vector>

class ItemSystem;

class DefaultIMPPolicy : public IMPPolicy
{
public:
	DefaultIMPPolicy(rapidjson::Document *json, const ItemSystem *itemSystem);

	virtual bool isCodeAccepted(const ST::string& code) const;
	virtual uint8_t getStartingLevel() const;
	virtual const std::vector<const ItemModel *> & getInventory() const;
	virtual const std::vector<const ItemModel *> & getGoodShooterItems() const;
	virtual const std::vector<const ItemModel *> & getNormalShooterItems() const;

protected:
	uint8_t m_startingLevel;
	std::vector<ST::string> m_activationCodes;
	std::vector<const ItemModel *> m_inventory;
	std::vector<const ItemModel *> m_goodShooterItems;
	std::vector<const ItemModel *> m_normalShooterItems;
};
