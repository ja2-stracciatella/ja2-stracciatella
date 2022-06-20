#include "SectorLandTypes.h"
#include "Campaign_Types.h"
#include "JsonObject.h"

std::map<SGPSector, LandType> SectorLandTypes::deserialize(const rapidjson::Document& document, const TraversibilityMap& travMap)
{
	std::map<SGPSector, LandType> landTypes;

	for (const auto &el : document.GetArray())
	{
		JsonObjectReader reader(el);
		auto sector = reader.GetString("sector");
		if (!SGPSector().IsValid(sector))
		{
			ST::string err = ST::format("{} is not a valid sector string", sector);
			throw std::runtime_error(err.to_std_string());
		}
		const uint8_t sectorLevel = reader.getOptionalUInt("sectorLevel");
		const SGPSector sectorKey = SGPSector::FromShortString(sector, sectorLevel);

		const ST::string &landTypeString = reader.GetString("landType");
		const uint8_t landType = travMap.at(landTypeString);

		landTypes.emplace(std::make_pair(sectorKey, landType));
	}

	return landTypes;
}
