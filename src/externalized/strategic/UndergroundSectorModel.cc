#include "UndergroundSectorModel.h"

#include "JsonUtility.h"
#include "Random.h"

UndergroundSectorModel::UndergroundSectorModel(uint8_t sectorId_, uint8_t sectorZ_, uint8_t adjacentSectors_,
	std::array<uint8_t, NUM_DIF_LEVELS> numTroops_, std::array<uint8_t, NUM_DIF_LEVELS> numElites_, std::array<uint8_t, NUM_DIF_LEVELS> numCreatures_,
	std::array<uint8_t, NUM_DIF_LEVELS> numTroopsVariance_, std::array<uint8_t, NUM_DIF_LEVELS> numElitesVariance_, std::array<uint8_t, NUM_DIF_LEVELS> numCreaturesVariance_)
		: sectorId(sectorId_), sectorZ(sectorZ_), adjacentSectors(adjacentSectors_),
		numTroops(numTroops_), numElites(numElites_), numCreatures(numCreatures_),
		numTroopsVariance(numTroopsVariance_), numElitesVariance(numElitesVariance_), numCreaturesVariance(numCreaturesVariance_) {}

UNDERGROUND_SECTORINFO* UndergroundSectorModel::createUndergroundSectorInfo(uint8_t difficultyLevel) const
{
	UNDERGROUND_SECTORINFO* const u = new UNDERGROUND_SECTORINFO{};

	u->ubSector = SGPSector::FromSectorID(sectorId, sectorZ);
	u->ubAdjacentSectors = adjacentSectors;

	u->ubNumTroops    = numTroops[difficultyLevel - 1]    + Random(numTroopsVariance[difficultyLevel - 1]   );
	u->ubNumElites    = numElites[difficultyLevel - 1]    + Random(numElitesVariance[difficultyLevel - 1]   );
	u->ubNumCreatures = numCreatures[difficultyLevel - 1] + Random(numCreaturesVariance[difficultyLevel - 1]);

	return u;
}

UndergroundSectorModel* UndergroundSectorModel::deserialize(const JsonValue& json)
{
	auto obj = json.toObject();
	uint8_t sectorId = JsonUtility::parseSectorID(obj["sector"]);
	uint8_t sectorZ = obj.GetUInt("sectorLevel");
	if (sectorZ == 0 || sectorZ > 3)
	{
		ST::string err = "Sector level must be between 1 and 3";
		throw std::runtime_error(err.to_std_string());
	}

	uint8_t adjacencyFlag = NO_ADJACENT_SECTOR;
	if (obj.has("adjacentSectors"))
	{
		auto adjacencies = obj["adjacentSectors"].toVec();
		for (auto& el : adjacencies)
		{
			ST::string adj = el.toString();
			if (adj.size() != 1)
			{
				ST::string err = ST::format("'{}' is not a valid adjacency direction.", adj);
				throw std::runtime_error(err.to_std_string());
			}
			switch (adj[0])
			{
			case DIRECTION_NORTH:
				adjacencyFlag |= NORTH_ADJACENT_SECTOR;
				break;
			case DIRECTION_EAST:
				adjacencyFlag |= EAST_ADJACENT_SECTOR;
				break;
			case DIRECTION_SOUTH:
				adjacencyFlag |= SOUTH_ADJACENT_SECTOR;
				break;
			case DIRECTION_WEST:
				adjacencyFlag |= WEST_ADJACENT_SECTOR;
				break;
			default:
				ST::string err = ST::format("{} is not a valid direction.", adj[0]);
				throw std::runtime_error(err.to_std_string());
			}
		}
	}

	return new UndergroundSectorModel(
		sectorId, sectorZ, adjacencyFlag,
		JsonUtility::readIntArrayByDiff(json, "numTroops"),
		JsonUtility::readIntArrayByDiff(json, "numElites"),
		JsonUtility::readIntArrayByDiff(json, "numCreatures"),
		JsonUtility::readIntArrayByDiff(json, "numTroopsVariance"),
		JsonUtility::readIntArrayByDiff(json, "numElitesVariance"),
		JsonUtility::readIntArrayByDiff(json, "numCreaturesVariance")
	);
}

void UndergroundSectorModel::validateData(const std::vector<const UndergroundSectorModel*>& ugSectors)
{
	// check for existence of hard-coded references
	// the list below is based on the occurrences of FindUnderGroundSector in codebase
	SGPSector basementSectors[] = {
		{ 13, 5,  1 },  // Draassen mine
		{ 9,  8,  1 },  // Cambria mine
		{ 14, 10, 1 },  // Alma mine
		{ 4,  8,  2 }   // Grumm mine
	};
	for (const auto& basement : basementSectors)
	{
		uint8_t sectorId = basement.AsByte();
		uint8_t sectorZ = basement.z;
		auto iter = std::find_if(ugSectors.begin(), ugSectors.end(), [sectorId, sectorZ](const UndergroundSectorModel* s) { return (s->sectorId == sectorId && s->sectorZ == sectorZ); });
		if (iter == ugSectors.end())
		{
			ST::string err = ST::format("An underground sector is expected at ({}), but is not defined.", basement);
			throw std::runtime_error(err.to_std_string());
		}
	}
}
