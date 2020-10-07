#pragma once
#include "MercProfile.h"
#include "MERCSpeckQuote.h"
#include <memory>
#include <rapidjson/document.h>
#include <vector>

typedef std::shared_ptr<MERCSpeckQuote> SpeckQuote;

class MERCListingModel
{
public:
	MERCListingModel(uint8_t index_, uint8_t profileID_, uint8_t bioIndex_, uint32_t minTotalSpending_, uint32_t mindays, std::vector<SpeckQuote> quotes_);
	const uint8_t index;

	// If we are coming from M.E.R.C., we should always use the GetProfileIDFromMERCListing 
	// instead, due to the hard-coded LARRY logic
	const uint8_t profileID;
	const uint8_t bioIndex;

	const uint32_t minTotalSpending;
	const uint32_t minDays;

	bool isAvailableAtStart() const;
	std::vector<SpeckQuote> getQuotesByType(SpeckQuoteType type) const;

	static MERCListingModel* deserialize(uint8_t index, const rapidjson::Value& json);
	static void validateData(const std::vector<const MERCListingModel*>&);
private:
	const std::vector<SpeckQuote> quotes;
};