#pragma once

#include "Json.h"

#include <map>
#include <stdint.h>
#include <string_theory/string>

struct AmmoTypeModel
{
	AmmoTypeModel(uint16_t index, ST::string && internalName);

	virtual ~AmmoTypeModel();

	virtual JsonValue serialize() const;

	static AmmoTypeModel* deserialize(const JsonValue &json);

	uint16_t index;
	ST::string internalName;
};

const AmmoTypeModel* getAmmoType(const ST::string& calibreName,
					const std::map<ST::string, const AmmoTypeModel*> &calibreMap);
