#include "NpcPlacementModel.h"
#include "Campaign_Types.h"

NpcPlacementModel::NpcPlacementModel(uint8_t profileId_, std::vector<uint8_t> sectorIds_)
	:profileId(profileId_), sectorIds(sectorIds_) {}

NpcPlacementModel* NpcPlacementModel::deserialize(const rapidjson::Value& element)
{
	std::vector<uint8_t> sectorIds;
	for (auto& sector : element["sectors"].GetArray())
	{
		auto sectorString = sector.GetString();
		if (!IS_VALID_SECTOR_SHORT_STRING(sectorString))
		{
			throw std::runtime_error("Invalid sector string");
		}
		sectorIds.push_back(SECTOR_FROM_SECTOR_SHORT_STRING(sectorString));
	}
	return new NpcPlacementModel(
		element["profileId"].GetInt(),
		sectorIds
	);
}
