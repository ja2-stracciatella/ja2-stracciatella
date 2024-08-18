#include "CalibreModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "Json.h"
#include <string_theory/format>
#include <string_theory/string>
#include <stdexcept>
#include <utility>


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

CalibreModel* CalibreModel::deserialize(const JsonValue &json)
{
	return new CalibreModel{ json.toObject() };
}

CalibreModel::CalibreModel(JsonObject const jsonObj) :
	index{ static_cast<uint16_t>(jsonObj.GetInt("index")) },
	internalName{ jsonObj.GetString("internalName") },
	sound{ jsonObj.getOptionalString("sound") },
	burstSound{ jsonObj.getOptionalString("burstSound") },
	silencedSound{ jsonObj.getOptionalString("silencedSound") },
	silencedBurstSound{ jsonObj.getOptionalString("silencedBurstSound") },
	showInHelpText{ jsonObj.GetBool("showInHelpText") },
	monsterWeapon{ jsonObj.GetBool("monsterWeapon") }
{
}

const ST::string* CalibreModel::getName() const
{
	return GCM->getCalibreName(index);
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
