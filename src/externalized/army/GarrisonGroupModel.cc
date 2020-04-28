#include "GarrisonGroupModel.h"
#include "Campaign_Types.h"

GARRISON_GROUP GarrisonGroupModel::deserialize(JsonObjectReader& obj, std::map<std::string, uint8_t> armyCompMapping)
{
	uint8_t sectorId = SECTOR_FROM_SECTOR_SHORT_STRING(obj.GetString("sector"));
	uint8_t compositionId = armyCompMapping.at(std::string(obj.GetString("composition")));
	return { sectorId, compositionId, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
}

void GarrisonGroupModel::validateData(std::vector<GARRISON_GROUP> garrisonGroups)
{
	if (garrisonGroups.size() > SAVED_GARRISON_GROUPS)
	{
		SLOGE(ST::format("There cannot be more than {} Garrison Groups", SAVED_GARRISON_GROUPS));
		throw std::runtime_error("Too many Garrison Groups");
	}
}
