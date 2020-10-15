#include "NpcPlacementModel.h"
#include "Campaign_Types.h"
#include "Random.h"
#include <utility>

NpcPlacementModel::NpcPlacementModel(uint8_t profileId_, std::vector<uint8_t> sectorIds_,
		bool isPlacedAtStart_, bool useAlternateMap_, bool isSciFiOnly_)
	:profileId(profileId_),  sectorIds(std::move(sectorIds_)),
	isPlacedAtStart(isPlacedAtStart_), useAlternateMap(useAlternateMap_), isSciFiOnly(isSciFiOnly_) {}

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

	JsonObjectReader r(element);
	return new NpcPlacementModel(
		r.GetUInt("profileId"),
		sectorIds,
		r.GetBool("placedAtStart"),
		r.getOptionalBool("useAlternateMap"),
		r.getOptionalBool("sciFiOnly")
	);
}

int16_t NpcPlacementModel::pickPlacementSector() const
{
	if (!sectorIds.empty())
	{
		uint8_t sector = sectorIds[Random(sectorIds.size())];
		return sector;
	}
	return -1;
}
