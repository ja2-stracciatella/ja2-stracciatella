#include "TownModel.h"

#include "JsonObject.h"
#include "JsonUtility.h"

TownModel::TownModel(int8_t townId_, std::vector<uint8_t> sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_)
		: townId(townId_), sectorIDs(sectorIDs_), townPoint(townPoint_), isMilitiaTrainingAllowed(isMilitiaTrainingAllowed_) {}

const uint8_t TownModel::getBaseSector() const
{
	int8_t minX = 99, minY = 99;
	for ( auto sectorID : sectorIDs ) {
		int8_t x = SECTORX(sectorID), y = SECTORY(sectorID);
		if ( x < minX ) minX = x;
		if ( y < minY ) minY = y;
	}
	return SECTOR( minX, minY );
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
