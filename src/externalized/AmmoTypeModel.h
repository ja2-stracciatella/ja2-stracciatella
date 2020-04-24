#pragma once

#include <string_theory/string>

#include <map>
#include <stdint.h>


class JsonObject;
class JsonObjectReader;

struct AmmoTypeModel
{
	AmmoTypeModel(uint16_t index, const char* internalName);

	// This could be default in C++11
	virtual ~AmmoTypeModel();

	virtual void serializeTo(JsonObject &obj) const;

	static AmmoTypeModel* deserialize(JsonObjectReader &obj);

	uint16_t index;
	ST::string internalName;
};

const AmmoTypeModel* getAmmoType(const char *calibreName,
					const std::map<ST::string, const AmmoTypeModel*> &calibreMap);
