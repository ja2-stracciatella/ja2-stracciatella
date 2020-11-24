#pragma once

#include <string_theory/string>

#include <map>
#include <stdexcept>
#include <stdint.h>

class JsonObject;
class JsonObjectReader;

#define NOAMMO (0)

struct CalibreModel
{
	CalibreModel(uint16_t index,
			ST::string internalName,
			ST::string burstSoundString,
			bool showInHelpText,
			bool monsterWeapon,
			int silencerSound
	);

	// This could be default in C++11
	virtual ~CalibreModel();

	const ST::string* getName() const;

	virtual void serializeTo(JsonObject &obj) const;
	static CalibreModel* deserialize(JsonObjectReader &obj);

	static const CalibreModel* getNoCalibreObject();

	uint16_t index;
	ST::string internalName;
	ST::string burstSoundString;
	bool showInHelpText;
	bool monsterWeapon;
	int silencerSound;
};

const CalibreModel* getCalibre(const ST::string& calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap);
