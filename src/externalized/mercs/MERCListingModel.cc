#include "Exceptions.h"
#include "JsonObject.h"
#include "MERCListingModel.h"
#include "Soldier_Control.h"

#include <set>
#include <string_theory/format>
#include <utility>

MERCListingModel::MERCListingModel(uint8_t index_, uint8_t profileID_, uint8_t bioIndex_,
	uint32_t minTotalSpending_, uint32_t minDays_,
	std::vector<SpeckQuote> quotes_
	) : index(index_), profileID(profileID_), bioIndex(bioIndex_),
	    minTotalSpending(minTotalSpending_), minDays(minDays_),
	    quotes(std::move(quotes_)) {}

bool MERCListingModel::isAvailableAtStart() const
{
	return this->minDays == 0 && this->minTotalSpending == 0;
}

std::vector<SpeckQuote> MERCListingModel::getQuotesByType(SpeckQuoteType type) const
{
	std::vector<SpeckQuote> filtered;
	for (const auto& q : quotes)
	{
		if (q->type == type) filtered.push_back(q);
	}
	return filtered;
}

static SpeckQuoteType SpeckQuoteTypefromString(std::string s)
{
	if (s == "ADVERTISE") return SpeckQuoteType::ADVERTISE;
	if (s == "MERC_DEAD") return SpeckQuoteType::MERC_DEAD;
	if (s == "CROSS_SELL") return SpeckQuoteType::CROSS_SELL;

	throw DataError(ST::format("unknown quote type: {}", s));
}

MERCListingModel* MERCListingModel::deserialize(uint8_t index, const rapidjson::Value& json, const MercSystem* mercSystem)
{
	ST::string profileName = json["profile"].GetString();
	auto profile = mercSystem->getMercProfileInfoByName(profileName);
	if (profile == NULL) {
		throw DataError(ST::format("'{}' does not refer to a valid profile", profile));
	}
	if (profile->mercType != MercType::MERC) {
		throw DataError(ST::format("Profile '{}' does not refer to a M.E.R.C. profile", profile));
	}

	std::vector<SpeckQuote> quotes;
	for (auto& elem : json["quotes"].GetArray())
	{
		JsonObjectReader r(elem);
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

	JsonObjectReader r(json);
	return new MERCListingModel(
		index,
		profile->profileID,
		r.GetUInt("bioIndex"),
		r.getOptionalInt("minTotalSpending"),
		r.getOptionalInt("minDays"),
		quotes
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
