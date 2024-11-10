#pragma once

#include "Json.h"
#include "Types.h"
#include <vector>

class TownModel
{
public:
	TownModel(int8_t townId_, ST::string&& internalName_, std::vector<uint8_t>&& sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_ );

	// Returns the top-left corner of the town on map. It may or may not belong to the town.
	SGPSector getBaseSector() const;
	static TownModel* deserialize(const JsonValue& obj);

	int8_t townId;
	ST::string internalName;
	std::vector<uint8_t> sectorIDs;
	SGPPoint townPoint;
	bool isMilitiaTrainingAllowed;
};
