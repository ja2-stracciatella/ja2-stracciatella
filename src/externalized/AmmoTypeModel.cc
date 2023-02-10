#include "AmmoTypeModel.h"

#include <stdint.h>
#include <stdexcept>
#include <utility>
#include <string_theory/format>

AmmoTypeModel::AmmoTypeModel(uint16_t index_, ST::string && internalName_)
	:index(index_), internalName(std::move(internalName_))
{
}

AmmoTypeModel::~AmmoTypeModel() = default;

JsonValue AmmoTypeModel::serialize() const
{
	auto obj = JsonObject();
	obj.set("index",                index);
	obj.set("internalName",         internalName);
	return obj.toValue();
}

AmmoTypeModel* AmmoTypeModel::deserialize(const JsonValue &json)
{
	auto obj = json.toObject();
	int index = obj.GetInt("index");
	return new AmmoTypeModel(index, obj.GetString("internalName"));
}


const AmmoTypeModel* getAmmoType(const ST::string& ammoTypeName,
					const std::map<ST::string, const AmmoTypeModel*> &ammoTypeMap)
{
	auto it = ammoTypeMap.find(ammoTypeName);
	if(it != ammoTypeMap.end())
	{
		return it->second;
	}

	throw std::runtime_error(ST::format("ammoType '{}' is not found", ammoTypeName).to_std_string());
}
