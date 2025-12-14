#include "TownModel.h"

#include "JsonUtility.h"
#include "TranslatableString.h"
#include <cstdint>

const ST::string TownModel::NAME_TRANSLATION_PREFIX = "strings/strategic-map-town-names";
const ST::string TownModel::NAME_LOCATIVE_TRANSLATION_PREFIX = "strings/strategic-map-town-name-locatives";

TownModel::TownModel(int8_t townId_, ST::string&& internalName_, ST::string&& name_, ST::string&& nameLocative_, std::vector<uint8_t>&& sectorIDs_, SGPPoint townPoint_, bool isMilitiaTrainingAllowed_)
	: townId(townId_), internalName(std::move(internalName_)), name(std::move(name_)), nameLocative(std::move(nameLocative_)), sectorIDs(std::move(sectorIDs_)), townPoint(townPoint_), isMilitiaTrainingAllowed(isMilitiaTrainingAllowed_) {}

SGPSector TownModel::getBaseSector() const
{
	SGPSector min(99, 99);
	for ( auto sectorID : sectorIDs ) {
		SGPSector sector(sectorID);
		if (sector < min) min = sector;
	}
	return min;
}

TownModel* TownModel::deserialize(const JsonValue& json, TranslatableString::Loader& stringLoader)
{
	std::vector<uint8_t> sectorIDs = JsonUtility::parseSectorList(json, "sectors");
	auto obj = json.toObject();
	auto townId = static_cast<int8_t>(obj.GetInt("townId"));

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
		townPoint,
		obj.getOptionalBool("isMilitiaTrainingAllowed")
		);
}
