#include "GarrisonGroupModel.h"
#include "Campaign_Types.h"
#include "JsonUtility.h"

GARRISON_GROUP GarrisonGroupModel::deserialize(const JsonValue& json, const std::map<ST::string, uint8_t>& armyCompMapping)
{
	auto obj = json.toObject();
	uint8_t sectorId = JsonUtility::parseSectorID(obj["sector"]);
	uint8_t compositionId = armyCompMapping.at(obj.GetString("composition"));

	GARRISON_GROUP group{};
	group.ubSectorID = sectorId;
	group.ubComposition = compositionId;
	return group;
}

void GarrisonGroupModel::validateData(const std::vector<GARRISON_GROUP>& garrisonGroups)
{
	if (garrisonGroups.size() > SAVED_GARRISON_GROUPS)
	{
		SLOGE("There cannot be more than {} Garrison Groups", SAVED_GARRISON_GROUPS);
		throw std::runtime_error("Too many Garrison Groups");
	}
}
