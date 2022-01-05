#pragma once

#include "MercProfileInfo.h"
#include <string_theory/string>

struct ItemModel;

class MercSystem
{
public:
	/**
	 * @param profileID
	 * @return pointer to a MercProfileInfo. Never returns null. A pointer to the empty instance is returned if not defined in JSON.
	 */
	virtual const MercProfileInfo* getMercProfileInfo(uint8_t const profileID) const = 0;
	/**
	 * @param profile
	 * @return pointer to a MercProfileInfo. Returns null if not defined in JSON
	 */
	virtual const MercProfileInfo* getMercProfileInfoByName(const ST::string &internalName) const = 0;
};
