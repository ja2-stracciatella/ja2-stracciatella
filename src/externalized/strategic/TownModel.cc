#include "TownModel.h"

#include "JsonUtility.h"

TownModel::TownModel(int8_t townId_, ST::string&& internalName_, std::vector<uint8_t>&& sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_)
	: townId(townId_), internalName(std::move(internalName_)), sectorIDs(std::move(sectorIDs_)), townPoint(townPoint_), isMilitiaTrainingAllowed(isMilitiaTrainingAllowed_) {}

SGPSector TownModel::getBaseSector() const
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
		obj.GetString("internalName"),
		std::move(sectorIDs),
		townPoint,
		obj.getOptionalBool("isMilitiaTrainingAllowed")
		);
}
