#pragma once

#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>

#include "sgp/StrUtils.h"

class JsonObject;
class JsonObjectReader;

#define NOAMMO (0)

struct CalibreModel
{
	CalibreModel(uint16_t index,
			const char* internalName,
			const char* burstSoundString,
			bool showInHelpText,
			bool monsterWeapon,
			int silencerSound
	);

	// This could be default in C++11
	virtual ~CalibreModel();

	const wchar_t* getName() const;

	virtual void serializeTo(JsonObject &obj) const;
	static CalibreModel* deserialize(JsonObjectReader &obj);

	static const CalibreModel* getNoCalibreObject();

	uint16_t index;
	std::string internalName;
	std::string burstSoundString;
	bool showInHelpText;
	bool monsterWeapon;
	int silencerSound;
};

const CalibreModel* getCalibre(const char *calibreName,
				const std::map<std::string, const CalibreModel*> &calibreMap);
