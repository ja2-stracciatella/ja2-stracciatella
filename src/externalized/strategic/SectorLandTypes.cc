#include "SectorLandTypes.h"
#include "Campaign_Types.h"
#include "JsonObject.h"

std::map<SectorKey, LandType> SectorLandTypes::deserialize(const rapidjson::Document& document, const TraversibilityMap& travMap)
{
	std::map<SectorKey, LandType> landTypes;

	for (const auto &el : document.GetArray())
	{
		JsonObjectReader reader(el);
		auto sector = reader.GetString("sector");
		if (!IS_VALID_SECTOR_SHORT_STRING(sector))
		{
			ST::string err = ST::format("{} is not a valid sector string", sector);
			throw std::runtime_error(err.to_std_string());
		}
		const uint8_t sectorLevel = reader.getOptionalUInt("sectorLevel");
		const SectorKey sectorKey(SECTOR_FROM_SECTOR_SHORT_STRING(sector), sectorLevel);

		const ST::string &landTypeString = reader.GetString("landType");
		const uint8_t landType = travMap.at(landTypeString);

		landTypes[sectorKey] = landType;
	}

	return landTypes;
}