#include "UndergroundSectorModel.h"

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

	u->ubSectorX = SECTORX(sectorId);
	u->ubSectorY = SECTORY(sectorId);
	u->ubSectorZ = sectorZ;
	u->ubAdjacentSectors = adjacentSectors;

	u->ubNumTroops    = numTroops[difficultyLevel - 1]    + Random(numTroopsVariance[difficultyLevel - 1]   );
	u->ubNumElites    = numElites[difficultyLevel - 1]    + Random(numElitesVariance[difficultyLevel - 1]   );
	u->ubNumCreatures = numCreatures[difficultyLevel - 1] + Random(numCreaturesVariance[difficultyLevel - 1]);

	return u;
}

std::array<uint8_t, NUM_DIF_LEVELS> readIntArray(const rapidjson::Value& obj, const char* fieldName)
{
	std::array<uint8_t, NUM_DIF_LEVELS> vals = {};
	if (!obj.HasMember(fieldName))
	{
		return vals;
	}
	auto arr = obj[fieldName].GetArray();
	if (arr.Size() != NUM_DIF_LEVELS)
	{
		ST::string err = ST::format("The number of values in {} is not same as NUM_DIF_LEVELS({})", fieldName, NUM_DIF_LEVELS);
		throw std::runtime_error(err.to_std_string());
	}
	for (auto i = 0; i < NUM_DIF_LEVELS; i++)
	{
		vals[i] = static_cast<uint8_t>(arr[i].GetUint());
	}

	return vals;
}

UndergroundSectorModel* UndergroundSectorModel::deserialize(const rapidjson::Value& obj)
{
	auto sectorString = obj["sector"].GetString();
	if (!IS_VALID_SECTOR_SHORT_STRING(sectorString))
	{
		ST::string err = ST::format("'{}' is not valid sector", sectorString);
		throw std::runtime_error(err.to_std_string());
	}
	uint8_t sectorId = SECTOR_FROM_SECTOR_SHORT_STRING(sectorString);
	uint8_t sectorZ = obj["sectorLevel"].GetUint();
	if (sectorZ == 0 || sectorZ > 3)
	{
		ST::string err = "Sector level must be between 1 and 3";
		throw std::runtime_error(err.to_std_string());
	}

	uint8_t adjacencyFlag = NO_ADJACENT_SECTOR;
	if (obj.HasMember("adjacentSectors") && obj["adjacentSectors"].IsArray())
	{
		auto adjacencies = obj["adjacentSectors"].GetArray();
		for (auto& el : adjacencies)
		{
			const char* adj = el.GetString();
			if (strlen(adj) != 1)
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
		readIntArray(obj, "numTroops"), readIntArray(obj, "numElites"), readIntArray(obj, "numCreatures"),
		readIntArray(obj, "numTroopsVariance"), readIntArray(obj, "numElitesVariance"), readIntArray(obj, "numCreaturesVariance")
	);
}

void UndergroundSectorModel::validateData(const std::vector<const UndergroundSectorModel*> ugSectors)
{
	// check for existence of hard-coded references
	// the list below is based on the occurrences of FindUnderGroundSector in codebase
	std::array<uint8_t, 3> basementSectors[] = {
		{ 13, 5,  1 },  // Draassen mine
		{ 9,  8,  1 },  // Cambria mine
		{ 14, 10, 1 },  // Alma mine
		{ 4,  8,  2 }   // Grumm mine
	};
	for (auto basementLoc : basementSectors)
	{
		uint8_t sectorId = SECTOR(basementLoc[0], basementLoc[1]);
		uint8_t sectorZ = basementLoc[2];
		auto iter = std::find_if(ugSectors.begin(), ugSectors.end(), [sectorId, sectorZ](const UndergroundSectorModel* s) { return (s->sectorId == sectorId && s->sectorZ == sectorZ); });
		if (iter == ugSectors.end())
		{
			ST::string err = ST::format("An underground sector is expected at ({},{},{}), but is not defined.", basementLoc[0], basementLoc[1], basementLoc[2]);
			throw std::runtime_error(err.to_std_string());
		}
	}
}