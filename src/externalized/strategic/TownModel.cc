#include "TownModel.h"

#include "JsonObject.h"
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

TownModel* TownModel::deserialize(const rapidjson::Value& obj)
{
	std::vector<uint8_t> sectorIDs = JsonUtility::parseSectorList(obj, "sectors");

	Assert( obj.HasMember("townPoint") && obj["townPoint"].IsObject() );
	auto tp = obj["townPoint"].GetObject();
	SGPPoint townPoint = SGPPoint();
	townPoint.iX = tp["x"].GetInt();
	townPoint.iY = tp["y"].GetInt();

	JsonObjectReader reader(obj);
	return new TownModel(
		reader.GetInt("townId"),
		sectorIDs,
		townPoint,
		reader.getOptionalBool("isMilitiaTrainingAllowed")
		);
}
