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
				ST::string sound_,
				ST::string burstSound_,
				ST::string silencedSound_,
				ST::string silencedBurstSound_,
				bool showInHelpText_,
				bool monsterWeapon_)
	:index(index_), internalName(std::move(internalName_)),
	sound(sound_),
	burstSound(burstSound_),
	silencedSound(silencedSound_),
	silencedBurstSound(silencedBurstSound_),
	showInHelpText(showInHelpText_),
	monsterWeapon(monsterWeapon_)
{
}

CalibreModel::~CalibreModel() = default;

void CalibreModel::serializeTo(JsonObject &obj) const
{
	obj.AddMember("index",			index);
	obj.AddMember("internalName",	internalName.c_str());
	obj.AddMember("sound",			sound.c_str());
	obj.AddMember("burstSound",		burstSound.c_str());
	obj.AddMember("silencedSound",		silencedSound.c_str());
	obj.AddMember("silencedBurstSound",	silencedBurstSound.c_str());
	obj.AddMember("showInHelpText",       showInHelpText);
	obj.AddMember("monsterWeapon",        monsterWeapon);
}

CalibreModel* CalibreModel::deserialize(JsonObjectReader &obj)
{
	int index = obj.GetInt("index");
	ST::string internalName = obj.GetString("internalName");
	ST::string sound = obj.getOptionalString("sound");
	ST::string burstSound = obj.getOptionalString("burstSound");
	ST::string silencedSound = obj.getOptionalString("silencedSound");
	ST::string silencedBurstSound = obj.getOptionalString("silencedBurstSound");
	return new CalibreModel(index, internalName, sound, burstSound, silencedSound, silencedBurstSound,
				obj.GetBool("showInHelpText"),
				obj.GetBool("monsterWeapon")
);
}

const ST::string* CalibreModel::getName() const
{
	return GCM->getCalibreName(index);
}

const CalibreModel* CalibreModel::getNoCalibreObject()
{
	static CalibreModel noCalibre(NOAMMO, "NO CALIBRE", "", "", "", "", false, false);
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
