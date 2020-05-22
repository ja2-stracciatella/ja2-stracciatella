#include "GarrisonGroupModel.h"
#include "Campaign_Types.h"

GARRISON_GROUP GarrisonGroupModel::deserialize(JsonObjectReader& obj, std::map<std::string, uint8_t> armyCompMapping)
{
	auto sector = obj.GetString("sector");
	if (!IS_VALID_SECTOR_SHORT_STRING(sector))
	{
		throw std::runtime_error("Not a valid sector string");
	}
	uint8_t sectorId = SECTOR_FROM_SECTOR_SHORT_STRING(sector);
	uint8_t compositionId = armyCompMapping.at(std::string(obj.GetString("composition")));

	GARRISON_GROUP group{};
	group.ubSectorID = sectorId;
	group.ubComposition = compositionId;
	return group;
}

void GarrisonGroupModel::validateData(std::vector<GARRISON_GROUP> garrisonGroups)
{
	if (garrisonGroups.size() > SAVED_GARRISON_GROUPS)
	{
		SLOGE(ST::format("There cannot be more than {} Garrison Groups", SAVED_GARRISON_GROUPS));
		throw std::runtime_error("Too many Garrison Groups");
	}
}
