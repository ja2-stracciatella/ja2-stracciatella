#include "BloodCatPlacementsModel.h"


BloodCatPlacementsModel::BloodCatPlacementsModel(uint8_t sectorId_, int8_t bloodCatPlacements_)
		:sectorId(sectorId_), bloodCatPlacements(bloodCatPlacements_)
{}

BloodCatPlacementsModel* BloodCatPlacementsModel::deserialize(JsonObjectReader & obj)
{
	uint8_t sectorId = SECTOR_FROM_SECTOR_SHORT_STRING(obj.GetString("sector"));
	return new BloodCatPlacementsModel(	
		sectorId,
		obj.GetInt("bloodCatPlacements")
		);
}
