#include "AmmoTypeModel.h"
#include "JsonObject.h"
#include <stdint.h>
#include <stdexcept>
#include <utility>
#include <string_theory/format>

AmmoTypeModel::AmmoTypeModel(uint16_t index_, ST::string && internalName_)
	:index(index_), internalName(std::move(internalName_))
{
}

AmmoTypeModel::~AmmoTypeModel() = default;

void AmmoTypeModel::serializeTo(JsonObject &obj) const
{
	obj.AddMember("index",                index);
	obj.AddMember("internalName",         internalName);
}

AmmoTypeModel* AmmoTypeModel::deserialize(JsonObjectReader &obj)
{
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
