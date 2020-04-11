#include "TownModel.h"

#include "JsonObject.h"

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
	Assert( obj.HasMember("sectors") && obj["sectors"].IsArray() );
	std::vector<uint8_t> sectorIDs;
	for ( auto& s : obj["sectors"].GetArray() ) 
	{
		sectorIDs.push_back( SECTOR_FROM_COORDINATES( s.GetString() ) );
	}

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
