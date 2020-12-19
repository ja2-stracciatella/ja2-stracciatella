#include "GarrisonGroupModel.h"
#include "Campaign_Types.h"
#include "JsonUtility.h"

GARRISON_GROUP GarrisonGroupModel::deserialize(JsonObjectReader& obj, const std::map<ST::string, uint8_t>& armyCompMapping)
{
	auto sector = obj.GetString("sector");
	uint8_t sectorId = JsonUtility::parseSectorID(sector);
	uint8_t compositionId = armyCompMapping.at(std::string(obj.GetString("composition")));

	GARRISON_GROUP group{};
	group.ubSectorID = sectorId;
	group.ubComposition = compositionId;
	return group;
}

void GarrisonGroupModel::validateData(const std::vector<GARRISON_GROUP>& garrisonGroups)
{
	if (garrisonGroups.size() > SAVED_GARRISON_GROUPS)
	{
		STLOGE("There cannot be more than {} Garrison Groups", SAVED_GARRISON_GROUPS);
		throw std::runtime_error("Too many Garrison Groups");
	}
}
