#include "BloodCatPlacementsModel.h"
#include "JsonUtility.h"

BloodCatPlacementsModel::BloodCatPlacementsModel(uint8_t sectorId_, int8_t bloodCatPlacements_)
		:sectorId(sectorId_), bloodCatPlacements(bloodCatPlacements_)
{}

BloodCatPlacementsModel* BloodCatPlacementsModel::deserialize(const JsonValue& json)
{
	auto obj = json.toObject();
	uint8_t sectorId = JsonUtility::parseSectorID(obj["sector"]);
	return new BloodCatPlacementsModel(
		sectorId,
		obj.GetInt("bloodCatPlacements")
		);
}
