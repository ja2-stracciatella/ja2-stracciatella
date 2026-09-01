#pragma once

#include "Json.h"
#include "TranslatableString.h"
#include "Types.h"
#include <string_theory/st_string.h>
#include <vector>

class TownModel
{
public:
	TownModel(int8_t townId_, ST::string&& internalName_, ST::string&& name, ST::string&& nameLocative, std::vector<uint8_t>&& sectorIDs_, uint8_t militiaMapBaseSectorID_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_ );

	// Returns the top-left corner of the town on map. It may or may not belong to the town.
	SGPSector getBaseSector() const;
	/* Returns the top-left corner of the 3x3 grid the militia panel draws for this town.  It has to
	 * line up with the town's cutout in interface/militiamaps.sti, which for some towns sits a row
	 * or a column outside the town itself, so it is not the same as getBaseSector(). */
	SGPSector getMilitiaMapBaseSector() const;
	static TownModel* deserialize(const JsonValue& obj, TranslatableString::Loader& stringLoader);

	int8_t townId;
	ST::string internalName;
	ST::string name;
	ST::string nameLocative;
	std::vector<uint8_t> sectorIDs;
	uint8_t militiaMapBaseSectorID;
	SGPPoint townPoint;
	bool isMilitiaTrainingAllowed;
private:
	static const ST::string NAME_TRANSLATION_PREFIX;
	static const ST::string NAME_LOCATIVE_TRANSLATION_PREFIX;
};
