#pragma once

#include "Json.h"
#include "MercSystem.h"
#include "TranslatableString.h"
#include <string_theory/st_string.h>

class AIMListingModel
{
public:
	AIMListingModel(uint8_t index_, uint8_t profileID_, ST::string&& description_, ST::string&& additionalInformation_);
	const uint8_t index;
	const uint8_t profileID;
	const ST::string description;
	const ST::string additionalInformation;

	static AIMListingModel* deserialize(uint8_t index, const JsonValue& json, const MercSystem* mercSystem, TranslatableString::Loader& stringLoader);
	static void validateData(const std::vector<const AIMListingModel*>&);
};
