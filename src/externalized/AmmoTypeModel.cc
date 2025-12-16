#include "AmmoTypeModel.h"

#include <stdint.h>
#include <utility>
#include <string_theory/format>

AmmoTypeModel::AmmoTypeModel(uint16_t index_, ST::string && internalName_)
	:index(index_), internalName(std::move(internalName_))
{
}

JsonValue AmmoTypeModel::serialize() const
{
	auto obj = JsonObject();
	obj.set("index",                index);
	obj.set("internalName",         internalName);
	return obj.toValue();
}

std::unique_ptr<AmmoTypeModel> AmmoTypeModel::deserialize(const JsonValue &json)
{
	auto obj = json.toObject();
	int index = obj.GetInt("index");
	return std::make_unique<AmmoTypeModel>(index, obj.GetString("internalName"));
}
