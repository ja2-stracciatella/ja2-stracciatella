#pragma once

#include "Containers.h"
#include "Json.h"

#include <stdint.h>
#include <string_theory/string>

struct AmmoTypeModel : public Containers::NamedEntity<uint16_t>
{
	AmmoTypeModel() = delete;
	AmmoTypeModel(uint16_t index, ST::string && internalName);

	static constexpr const char* ENTITY_NAME = "AmmoType";
	virtual uint16_t getId() const override {
		return index;
	};
	virtual const ST::string& getInternalName() const override {
		return internalName;
	};

	virtual JsonValue serialize() const;

	static std::unique_ptr<AmmoTypeModel> deserialize(const JsonValue &json);

	uint16_t index;
	ST::string internalName;
};
