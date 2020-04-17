#pragma once

#include "NpcPlacementModel.h"
#include "Campaign_Types.h"

NpcPlacementModel::NpcPlacementModel(uint8_t profileId_, std::vector<uint8_t> sectorIds_)
	:profileId(profileId_), sectorIds(sectorIds_) {}

NpcPlacementModel* NpcPlacementModel::deserialize(const rapidjson::Value& element)
{
	std::vector<uint8_t> sectorIds;
	for (auto& sector : element["sectors"].GetArray())
	{
		sectorIds.push_back(SECTOR_FROM_SECTOR_SHORT_STRING(sector.GetString()));
	}
	return new NpcPlacementModel(
		element["profileId"].GetInt(),
		sectorIds
	);
}
