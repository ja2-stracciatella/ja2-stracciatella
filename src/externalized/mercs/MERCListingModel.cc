#include "Directories.h"
#include "Exceptions.h"
#include "MERCListingModel.h"
#include "Soldier_Control.h"
#include "TranslatableString.h"

#include <cstdint>
#include <memory>
#include <set>
#include <string_theory/format>
#include <utility>

namespace BioStrings {
	constexpr const char* BINARY_STRING_FILE = BINARYDATADIR "/mercbios.edt";
	constexpr uint32_t BINARY_DESCRIPTION_SIZE = 400;
	constexpr uint32_t BINARY_ADDITIONAL_INFORMATION_SIZE = 160;
	constexpr uint32_t BINARY_ITEM_TOTAL_SIZE = BINARY_DESCRIPTION_SIZE + BINARY_ADDITIONAL_INFORMATION_SIZE;
}

MERCListingModel::MERCListingModel(uint8_t index_, uint8_t profileID_, ST::string&& description_, ST::string&& additionalInformation_,
	uint32_t minTotalSpending_, uint32_t minDays_,
	std::vector<SpeckQuote>&& quotes_
	) : index(index_), profileID(profileID_), description(std::move(description_)), additionalInformation(std::move(additionalInformation_)),
	    minTotalSpending(minTotalSpending_), minDays(minDays_),
	    quotes(std::move(quotes_)) {}

bool MERCListingModel::isAvailableAtStart() const
{
	return this->minDays == 0 && this->minTotalSpending == 0;
}

std::vector<SpeckQuote> MERCListingModel::getQuotesByType(SpeckQuoteType type) const
{
	std::vector<SpeckQuote> filtered;
	for (auto const& q : quotes)
	{
		if (q->type == type) filtered.push_back(q);
	}
	return filtered;
}

static SpeckQuoteType SpeckQuoteTypefromString(const ST::string& s)
{
	if (s == "ADVERTISE") return SpeckQuoteType::ADVERTISE;
	if (s == "MERC_DEAD") return SpeckQuoteType::MERC_DEAD;
	if (s == "CROSS_SELL") return SpeckQuoteType::CROSS_SELL;

	throw DataError(ST::format("unknown quote type: {}", s));
}

MERCListingModel* MERCListingModel::deserialize(uint8_t index, const JsonValue& json, const MercSystem* mercSystem, TranslatableString::Loader& stringLoader)
{
	using namespace BioStrings;

	auto reader = json.toObject();
	ST::string profileName = reader.GetString("profile");
	auto profile = mercSystem->getMercProfileInfoByName(profileName);
	if (profile == NULL) {
		throw DataError(ST::format("'{}' does not refer to a valid profile", profile));
	}
	if (profile->mercType != MercType::MERC) {
		throw DataError(ST::format("Profile '{}' does not refer to a M.E.R.C. profile", profile));
	}

	auto jsonQuotes = reader["quotes"];
	std::vector<SpeckQuote> quotes;
	for (const auto& elem : jsonQuotes.toVec())
	{
		auto r = elem.toObject();
		auto quoteType = SpeckQuoteTypefromString(r.GetString("type"));
		ST::string crossSellName = r.getOptionalString("profile");

		uint8_t crossSellID = 0;
		if (quoteType == SpeckQuoteType::CROSS_SELL) {
			if (profileName.empty()) {
				throw DataError(ST::format("Profile '{}' has a CROSS_SELL quote without M.E.R.C. profile", profileName));
			}
			auto crossSellProfile = mercSystem->getMercProfileInfoByName(crossSellName);
			if (crossSellProfile == NULL) {
				throw DataError(ST::format("Profile '{}' has a CROSS_SELL quote '{}' that does not refer to a valid profile", profileName, crossSellName));
			}
			if (crossSellProfile->mercType != MercType::MERC) {
				throw DataError(ST::format("Profile '{}' has a CROSS_SELL quote '{}' that does not refer to a M.E.R.C. profile", profileName, crossSellName));
			}
			crossSellID = crossSellProfile->profileID;
		}

		auto quote = std::make_shared<MERCSpeckQuote>(
			r.GetUInt("quoteID"),
			quoteType,
			crossSellID
		);
		quotes.push_back(quote);
	}

	if ((!reader.has("description") || !reader.has("additionalInformation")) && !reader.has("bioIndex")) {
		SLOGE("M.E.R.C listing {} should have bioIndex set when description or additionalInformation are not set, defaulting to 0", profileName);
	}
	auto bioIndex = static_cast<uint32_t>(reader.getOptionalUInt("bioIndex"));

	return new MERCListingModel(
		index,
		profile->profileID,
		TranslatableString::Utils::resolveOptionalProperty(
			stringLoader,
			reader,
			"description",
			std::make_unique<TranslatableString::Binary>(BINARY_STRING_FILE, BINARY_ITEM_TOTAL_SIZE * bioIndex, BINARY_DESCRIPTION_SIZE)
		),
		TranslatableString::Utils::resolveOptionalProperty(
			stringLoader,
			reader,
			"additionalInformation",
			std::make_unique<TranslatableString::Binary>(BINARY_STRING_FILE, BINARY_ITEM_TOTAL_SIZE * bioIndex + BINARY_DESCRIPTION_SIZE, BINARY_ADDITIONAL_INFORMATION_SIZE)
		),
		reader.getOptionalInt("minTotalSpending"),
		reader.getOptionalInt("minDays"),
		std::move(quotes)
	);
}

void MERCListingModel::validateData(const std::vector<const MERCListingModel*>& models)
{
	std::set<uint8_t> uniqueProfileIDs;
	for (auto m : models)
	{
		if (m->profileID == 0 || m->profileID >= NO_PROFILE)
		{
			throw DataError(ST::format("Invalid profileID '{}'", m->profileID));
		}

		// Check if we have duplicates
		if (uniqueProfileIDs.find(m->profileID) != uniqueProfileIDs.end())
		{
			throw DataError(ST::format("profileID {} has been listed more than once", m->profileID));
		}
		uniqueProfileIDs.insert(m->profileID);
	}

	for (auto m : models)
	{
		for (const auto& quote : m->getQuotesByType(SpeckQuoteType::CROSS_SELL))
		{
			// Check if related merc is set
			if (!quote->relatedMercID)
			{
				throw DataError(ST::format("No related merc ID set for a CROSS_SELL quote ({})", m->profileID));
			}
		}
	}
}
