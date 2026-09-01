#include "TownModel.h"

#include "JsonUtility.h"
#include "TranslatableString.h"
#include <algorithm>
#include <cstdint>

const ST::string TownModel::NAME_TRANSLATION_PREFIX = "strings/strategic-map-town-names";
const ST::string TownModel::NAME_LOCATIVE_TRANSLATION_PREFIX = "strings/strategic-map-town-name-locatives";

TownModel::TownModel(int8_t townId_, ST::string&& internalName_, ST::string&& name_, ST::string&& nameLocative_, std::vector<uint8_t>&& sectorIDs_, uint8_t militiaMapBaseSectorID_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_)
	: townId(townId_), internalName(std::move(internalName_)), name(std::move(name_)), nameLocative(std::move(nameLocative_)), sectorIDs(std::move(sectorIDs_)), militiaMapBaseSectorID(militiaMapBaseSectorID_), townPoint(townPoint_), isMilitiaTrainingAllowed(isMilitiaTrainingAllowed_) {}

SGPSector TownModel::getBaseSector() const
{
	SGPSector min(99, 99);
	for ( auto sectorID : sectorIDs ) {
		SGPSector sector(sectorID);
		if (sector < min) min = sector;
	}
	return min;
}

SGPSector TownModel::getMilitiaMapBaseSector() const
{
	return SGPSector(militiaMapBaseSectorID);
}

TownModel* TownModel::deserialize(const JsonValue& json, TranslatableString::Loader& stringLoader)
{
	std::vector<uint8_t> sectorIDs = JsonUtility::parseSectorList(json, "sectors");
	auto obj = json.toObject();
	auto townId = static_cast<int8_t>(obj.GetInt("townId"));

	/* The militia panel draws a fixed 3x3 cutout per town, so where that grid starts is a property
	 * of the artwork rather than of the sector list.  Fall back to the town's own top left sector. */
	uint8_t militiaMapBaseSectorID = obj.has("militiaMapBaseSector") ?
		JsonUtility::parseSectorID(obj["militiaMapBaseSector"]) :
		(sectorIDs.empty() ? 0 : *std::min_element(sectorIDs.begin(), sectorIDs.end()));

	auto tp = obj["townPoint"].toObject();
	SGPPoint townPoint = SGPPoint();
	townPoint.iX = tp.GetInt("x");
	townPoint.iY = tp.GetInt("y");

	return new TownModel(
		obj.GetInt("townId"),
		obj.getOptionalString("internalName"),
		TranslatableString::Utils::resolveOptionalProperty(stringLoader, obj, "name", std::make_unique<TranslatableString::Json>(NAME_TRANSLATION_PREFIX, townId)),
		TranslatableString::Utils::resolveOptionalProperty(stringLoader, obj, "nameLocative", std::make_unique<TranslatableString::Json>(NAME_LOCATIVE_TRANSLATION_PREFIX, townId)),
		std::move(sectorIDs),
		militiaMapBaseSectorID,
		townPoint,
		obj.getOptionalBool("isMilitiaTrainingAllowed")
		);
}
