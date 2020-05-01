#include "CalibreModel.h"

#include "game/Utils/Text.h"

#include "JsonObject.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>

#include <stdexcept>

CalibreModel::CalibreModel(uint16_t index_,
				const char* internalName_,
				const char* burstSoundString_,
				bool showInHelpText_,
				bool monsterWeapon_,
				int silencerSound_)
	:index(index_), internalName(internalName_),
	burstSoundString(burstSoundString_),
	showInHelpText(showInHelpText_),
	monsterWeapon(monsterWeapon_),
	silencerSound(silencerSound_)
{
}

CalibreModel::~CalibreModel() {}

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
	const char *internalName = obj.GetString("internalName");
	const char *burstSoundString = obj.GetString("burstSoundString");
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


const CalibreModel* getCalibre(const char *calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap)
{
	std::map<ST::string, const CalibreModel*>::const_iterator it = calibreMap.find(calibreName);
	if(it != calibreMap.end())
	{
		return it->second;
	}

	throw std::runtime_error(ST::format("calibre '{}' is not found", calibreName).to_std_string());
}
