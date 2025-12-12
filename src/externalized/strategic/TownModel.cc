#include "TownModel.h"

#include "JsonUtility.h"
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

	std::unique_ptr<TranslatableString::String> translatableName = std::make_unique<TranslatableString::Json>(NAME_TRANSLATION_PREFIX, townId);
	if (obj.has("name")) {
		translatableName = TranslatableString::String::parse(obj.GetValue("name"));
	}
	std::unique_ptr<TranslatableString::String> translatableLocative = std::make_unique<TranslatableString::Json>(NAME_LOCATIVE_TRANSLATION_PREFIX, townId);
	if (obj.has("locative")) {
		translatableLocative = TranslatableString::String::parse(obj.GetValue("locative"));
	}

	return new TownModel(
		obj.GetInt("townId"),
		obj.getOptionalString("internalName"),
		translatableName->resolve(stringLoader),
		translatableLocative->resolve(stringLoader),
		std::move(sectorIDs),
		townPoint,
		obj.getOptionalBool("isMilitiaTrainingAllowed")
		);
}
