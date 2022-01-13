#include "Exceptions.h"
#include "JsonUtility.h"
#include "NpcPlacementModel.h"
#include "Random.h"

#include <utility>

NpcPlacementModel::NpcPlacementModel(uint8_t profileId_, std::vector<uint8_t> sectorIds_,
		bool isPlacedAtStart_, bool useAlternateMap_, bool isSciFiOnly_)
	:profileId(profileId_),  sectorIds(std::move(sectorIds_)),
	isPlacedAtStart(isPlacedAtStart_), useAlternateMap(useAlternateMap_), isSciFiOnly(isSciFiOnly_) {}

NpcPlacementModel* NpcPlacementModel::deserialize(const rapidjson::Value& element, const MercSystem* mercSystem)
{
	std::vector<uint8_t> sectorIds = JsonUtility::parseSectorList(element, "sectors");
	ST::string profile = element["profile"].GetString();
	auto mercProfile = mercSystem->getMercProfileInfoByName(profile);
	if (mercProfile == NULL) {
		throw DataError(ST::format("`{}` does not refer to a valid profile.", profile));
	}

	JsonObjectReader r(element);
	return new NpcPlacementModel(
		mercProfile->profileID,
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
