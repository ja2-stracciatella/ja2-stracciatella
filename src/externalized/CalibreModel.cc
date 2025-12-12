#include "CalibreModel.h"
#include "Json.h"
#include "TranslatableString.h"
#include <memory>
#include <string_theory/format>
#include <string_theory/string>
#include <stdexcept>
#include <utility>

const ST::string CalibreModel::DEFAULT_NAME_TRANSLATION_PREFIX = "strings/ammo-calibre";
const ST::string CalibreModel::DEFAULT_BOBBY_RAYS_NAME_TRANSLATION_PREFIX = "strings/ammo-calibre-bobbyray";

JsonValue CalibreModel::serialize() const
{
	JsonObject obj;
	obj.set("index",              index);
	obj.set("internalName",       internalName);
	obj.set("sound",              sound);
	obj.set("burstSound",         burstSound);
	obj.set("silencedSound",      silencedSound);
	obj.set("silencedBurstSound", silencedBurstSound);
	obj.set("showInHelpText",     showInHelpText);
	obj.set("monsterWeapon",      monsterWeapon);
	return obj.toValue();
}

CalibreModel* CalibreModel::deserialize(const JsonValue &json, TranslatableString::Loader& stringLoader)
{
	auto jsonObj = json.toObject();
	auto index = static_cast<uint16_t>(jsonObj.GetInt("index"));
	auto internalName = jsonObj.GetString("internalName");
	auto sound = jsonObj.getOptionalString("sound");
	auto burstSound = jsonObj.getOptionalString("burstSound");
	auto silencedSound = jsonObj.getOptionalString("silencedSound");
	auto silencedBurstSound = jsonObj.getOptionalString("silencedBurstSound");
	auto showInHelpText = jsonObj.GetBool("showInHelpText");
	auto monsterWeapon = jsonObj.GetBool("monsterWeapon");
	std::unique_ptr<TranslatableString::String> translatableName = std::make_unique<TranslatableString::Json>(DEFAULT_NAME_TRANSLATION_PREFIX, index);
	if (jsonObj.has("name")) {
		translatableName = TranslatableString::String::parse(jsonObj.GetValue("name"));
	}
	std::unique_ptr<TranslatableString::String> translatableBobbyRaysName = std::make_unique<TranslatableString::Json>(DEFAULT_BOBBY_RAYS_NAME_TRANSLATION_PREFIX, index);
	if (jsonObj.has("bobbyRaysName")) {
		translatableBobbyRaysName = TranslatableString::String::parse(jsonObj.GetValue("bobbyRaysName"));
	}

	return new CalibreModel(
		index,
		std::move(internalName),
		translatableName->resolve(stringLoader),
		translatableBobbyRaysName->resolve(stringLoader),
		std::move(sound),
		std::move(burstSound),
		std::move(silencedSound),
		std::move(silencedBurstSound),
		showInHelpText,
		monsterWeapon
	);
}

const ST::string& CalibreModel::getName() const
{
	return name;
}

const ST::string& CalibreModel::getBobbyRaysName() const
{
	return name;
}

const CalibreModel* CalibreModel::getNoCalibreObject()
{
	static CalibreModel noCalibre;
	return &noCalibre;
}

const CalibreModel* getCalibre(const ST::string& calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap)
{
	auto it = calibreMap.find(calibreName);
	if(it != calibreMap.end())
	{
		return it->second;
	}

	throw std::runtime_error(ST::format("calibre '{}' is not found", calibreName).to_std_string());
}
