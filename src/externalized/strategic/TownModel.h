#pragma once

#include "Json.h"
#include "TranslatableString.h"
#include "Types.h"
#include <string_theory/st_string.h>
#include <vector>

class TownModel
{
public:
	TownModel(int8_t townId_, ST::string&& internalName_, ST::string&& name, ST::string&& nameLocative, std::vector<uint8_t>&& sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_ );

	// Returns the top-left corner of the town on map. It may or may not belong to the town.
	SGPSector getBaseSector() const;
	static TownModel* deserialize(const JsonValue& obj, TranslatableString::Loader& stringLoader);

	int8_t townId;
	ST::string internalName;
	ST::string name;
	ST::string nameLocative;
	std::vector<uint8_t> sectorIDs;
	SGPPoint townPoint;
	bool isMilitiaTrainingAllowed;
private:
	static const ST::string NAME_TRANSLATION_PREFIX;
	static const ST::string NAME_LOCATIVE_TRANSLATION_PREFIX;
};
