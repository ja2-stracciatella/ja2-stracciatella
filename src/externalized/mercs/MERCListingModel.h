#pragma once

#include "MercSystem.h"
#include "MERCSpeckQuote.h"
#include "Json.h"
#include "TranslatableString.h"

#include <memory>
#include <vector>

typedef std::shared_ptr<MERCSpeckQuote> SpeckQuote;

class MERCListingModel
{
public:
	MERCListingModel(uint8_t index_, uint8_t profileID_, ST::string&& description_, ST::string&& additionalInformation_, uint32_t minTotalSpending_, uint32_t mindays, std::vector<SpeckQuote>&& quotes_);
	const uint8_t index;

	// If we are coming from M.E.R.C., we should always use the GetProfileIDFromMERCListing
	// instead, due to the hard-coded LARRY logic
	const uint8_t profileID;
	const ST::string description;
	const ST::string additionalInformation;

	const uint32_t minTotalSpending;
	const uint32_t minDays;

	bool isAvailableAtStart() const;
	std::vector<SpeckQuote> getQuotesByType(SpeckQuoteType type) const;

	static MERCListingModel* deserialize(uint8_t index, const JsonValue& json, const MercSystem* mercSystem, TranslatableString::Loader& stringLoader);
	static void validateData(const std::vector<const MERCListingModel*>&);
private:
	const std::vector<SpeckQuote> quotes;
};
