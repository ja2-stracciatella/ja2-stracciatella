#include "TownModel.h"

#include "JsonUtility.h"
#include <utility>

TownModel::TownModel(int8_t townId_, std::vector<uint8_t> sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_)
		: townId(townId_), sectorIDs(std::move(sectorIDs_)), townPoint(townPoint_), isMilitiaTrainingAllowed(isMilitiaTrainingAllowed_) {}

const SGPSector TownModel::getBaseSector() const
{
	SGPSector min(99, 99);
	for ( auto sectorID : sectorIDs ) {
		SGPSector sector(sectorID);
		if (sector < min) min = sector;
	}
	return min;
}

TownModel* TownModel::deserialize(const JsonValue& json)
{
	std::vector<uint8_t> sectorIDs = JsonUtility::parseSectorList(json, "sectors");
	auto obj = json.toObject();

	auto tp = obj["townPoint"].toObject();
	SGPPoint townPoint = SGPPoint();
	townPoint.iX = tp.GetInt("x");
	townPoint.iY = tp.GetInt("y");

	return new TownModel(
		obj.GetInt("townId"),
		sectorIDs,
		townPoint,
		obj.getOptionalBool("isMilitiaTrainingAllowed")
		);
}
