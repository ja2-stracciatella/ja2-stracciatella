#include "CacheSectorsModel.h"
#include "JsonUtility.h"
#include "Random.h"
#include <utility>

CacheSectorsModel::CacheSectorsModel(std::vector<uint8_t> sectors_,
	std::array<uint8_t, NUM_DIF_LEVELS> numTroops_,
	std::array<uint8_t, NUM_DIF_LEVELS> numTroopsVariance_)
	: sectors(std::move(sectors_)), numTroops(numTroops_), numTroopsVariance(numTroopsVariance_) {}

const CacheSectorsModel* CacheSectorsModel::deserialize(const rapidjson::Document& doc)
{
	auto sectorIDs = JsonUtility::parseSectorList(doc, "sectors");
	return new CacheSectorsModel(
		sectorIDs,
		JsonUtility::readIntArrayByDiff(doc, "numTroops"),
		JsonUtility::readIntArrayByDiff(doc, "numTroopsVariance")
		);
}

int16_t CacheSectorsModel::pickSector() const
{
	return sectors.empty() ? -1 : sectors[Random(sectors.size())];
}

uint8_t CacheSectorsModel::getNumTroops(uint8_t difficultyLevel) const
{
	if (difficultyLevel < 1 || difficultyLevel > NUM_DIF_LEVELS)
	{
		ST::string err = ST::format("invalid difficulty level {}", difficultyLevel);
		throw std::runtime_error(err.to_std_string());
	}
	return numTroops[difficultyLevel - 1] + Random(numTroopsVariance[difficultyLevel - 1]);
}