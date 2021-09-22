#include "CalibreModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "JsonObject.h"
#include "Text.h"
#include <string_theory/format>
#include <string_theory/string>
#include <stdexcept>
#include <utility>

CalibreModel::CalibreModel(uint16_t index_,
				ST::string internalName_,
				ST::string burstSoundString_,
				bool showInHelpText_,
				bool monsterWeapon_,
				int silencerSound_)
	:index(index_), internalName(std::move(internalName_)),
	burstSoundString(std::move(burstSoundString_)),
	showInHelpText(showInHelpText_),
	monsterWeapon(monsterWeapon_),
	silencerSound(silencerSound_)
{
}

CalibreModel::~CalibreModel() = default;

void CalibreModel::serializeTo(JsonObject &obj) const
{
	obj.AddMember("index",                index);
	obj.AddMember("internalName",         internalName.c_str());
	obj.AddMember("burstSoundString",     burstSoundString.c_str());
	obj.AddMember("showInHelpText",       showInHelpText);
	obj.AddMember("monsterWeapon",        monsterWeapon);
	obj.AddMember("silencerSound",        silencerSound);
}

CalibreModel* CalibreModel::deserialize(JsonObjectReader &obj)
{
	int index = obj.GetInt("index");
	ST::string internalName = obj.GetString("internalName");
	ST::string burstSoundString = obj.GetString("burstSoundString");
	return new CalibreModel(index, internalName, burstSoundString,
				obj.GetBool("showInHelpText"),
				obj.GetBool("monsterWeapon"),
				obj.GetInt("silencerSound")
);
}

const ST::string* CalibreModel::getName() const
{
	return GCM->getCalibreName(index);
}

const CalibreModel* CalibreModel::getNoCalibreObject()
{
	static CalibreModel noCalibre(NOAMMO, "NO CALIBRE", "", false, false, -1);
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
