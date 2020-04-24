#include "AmmoTypeModel.h"

#include <stdexcept>

#include "sgp/StrUtils.h"

#include "JsonObject.h"

AmmoTypeModel::AmmoTypeModel(uint16_t index_,
				const char* internalName_)
	:index(index_), internalName(internalName_)
{
}

// This could be default in C++11
AmmoTypeModel::~AmmoTypeModel() {}

void AmmoTypeModel::serializeTo(JsonObject &obj) const
{
	obj.AddMember("index",                index);
	obj.AddMember("internalName",         internalName.c_str());
}

AmmoTypeModel* AmmoTypeModel::deserialize(JsonObjectReader &obj)
{
	int index = obj.GetInt("index");
	const char *internalName = obj.GetString("internalName");
	return new AmmoTypeModel(index, internalName);
}


const AmmoTypeModel* getAmmoType(const char *ammoTypeName,
					const std::map<ST::string, const AmmoTypeModel*> &ammoTypeMap)
{
	std::map<ST::string, const AmmoTypeModel*>::const_iterator it = ammoTypeMap.find(ammoTypeName);
	if(it != ammoTypeMap.end())
	{
		return it->second;
	}

	throw std::runtime_error(FormattedString("ammoType '%s' is not found", ammoTypeName).to_std_string());
}
