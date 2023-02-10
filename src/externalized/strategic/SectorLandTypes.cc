#include "SectorLandTypes.h"
#include "Campaign_Types.h"

std::map<SGPSector, LandType> SectorLandTypes::deserialize(const JsonValue& json, const TraversibilityMap& travMap)
{
	std::map<SGPSector, LandType> landTypes;

	for (const auto &el : json.toVec())
	{
		auto reader = el.toObject();
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
