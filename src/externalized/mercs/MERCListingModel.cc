#include "MERCListingModel.h"
#include "JsonObject.h"
#include "Soldier_Control.h"
#include <set>
#include <string_theory/format>

MERCListingModel::MERCListingModel(uint8_t index_, uint8_t profileID_, 
	uint32_t minTotalSpending_, uint32_t minDays_, 
	std::vector<SpeckQuote> quotes_
	) : index(index_), profileID(profileID_),
	    minTotalSpending(minTotalSpending_), minDays(minDays_),
	    quotes(quotes_) {}

bool MERCListingModel::isAvailableAtStart() const
{
	return this->minDays == 0 && this->minTotalSpending == 0;
}

const std::vector<SpeckQuote> MERCListingModel::getQuotesByType(SpeckQuoteType type) const
{
	std::vector<SpeckQuote> filtered;
	for (auto q : quotes)
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

	throw std::runtime_error("unsupported quote type: " + s);
}

MERCListingModel* MERCListingModel::deserialize(uint8_t index, const rapidjson::Value& json)
{
	std::vector<SpeckQuote> quotes;
	for (auto& elem : json["quotes"].GetArray())
	{
		JsonObjectReader r(elem);
		auto quote = std::make_shared<MERCSpeckQuote>(
			r.GetUInt("quoteID"),
			SpeckQuoteTypefromString(r.GetString("type")),
			static_cast<uint8_t>(r.getOptionalInt("profileID"))
		);
		quotes.push_back(quote);
	}

	JsonObjectReader r(json);
	return new MERCListingModel(
		index,
		r.GetUInt("profileID"),
		r.getOptionalInt("minTotalSpending"), 
		r.getOptionalInt("minDays"),
		quotes
	);

}

void MERCListingModel::validateData(std::vector<const MERCListingModel*> models)
{
	std::set<uint8_t> uniqueProfileIDs;
	for (auto m : models)
	{
		if (m->profileID == 0 || m->profileID >= NO_PROFILE)
		{
			ST::string err = ST::format("Invalid profileID '{}'", m->profileID);
			throw std::runtime_error(err.to_std_string());
		}

		// Check if we have duplicates
		if (uniqueProfileIDs.find(m->profileID) != uniqueProfileIDs.end())
		{
			ST::string err = ST::format("profileID {} has been listed more than once", m->profileID);
			throw std::runtime_error(err.to_std_string());
		}
		uniqueProfileIDs.insert(m->profileID);
	}

	for (auto m : models)
	{
		for (auto quote : m->getQuotesByType(SpeckQuoteType::CROSS_SELL))
		{
			// Check if related merc is set
			if (!quote->relatedMercID)
			{
				ST::string err = ST::format("No related merc ID set for a CROSS_SELL quote ({})", m->profileID);
				throw std::runtime_error(err.to_std_string());
			}
		}
	}
}