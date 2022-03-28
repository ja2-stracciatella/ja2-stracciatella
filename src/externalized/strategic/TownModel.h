#pragma once

#include "Campaign_Types.h"
#include "JsonObject.h"

#include <vector>

class TownModel
{
public:
	TownModel(int8_t townId_, std::vector<uint8_t> sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_ );

	// Returns the top-left corner of the town on map. It may or may not belong to the town.
	const SGPSector getBaseSector() const;
	static TownModel* deserialize(const rapidjson::Value& obj);

	int8_t townId;
	std::vector<uint8_t> sectorIDs;
	SGPPoint townPoint;
	bool isMilitiaTrainingAllowed;
};

