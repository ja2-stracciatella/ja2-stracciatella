#pragma once

#include "Containers.h"
#include "Json.h"
#include "MercSystem.h"
#include "TranslatableString.h"
#include <cstdint>
#include <string_theory/st_string.h>

class AIMListingModel : public Containers::Entity<uint8_t>
{
public:
	AIMListingModel(uint8_t profileID_, ST::string&& description_, ST::string&& additionalInformation_);

	static constexpr const char* ENTITY_NAME = "AIMListing";
	virtual uint8_t getId() const override {
		return profileID;
	};

	const uint8_t profileID;
	const ST::string description;
	const ST::string additionalInformation;

	static std::unique_ptr<AIMListingModel> deserialize(const JsonValue& json, const MercSystem* mercSystem, TranslatableString::Loader& stringLoader);
};
