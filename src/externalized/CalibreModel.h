#pragma once

#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string_theory/string>

class JsonObject;
class JsonObjectReader;

#define NOAMMO (0)

struct CalibreModel
{
	CalibreModel(uint16_t index,
			ST::string internalName,
			ST::string sound,
			ST::string burstSound,
			ST::string silencedSound,
			ST::string silencedBurstSound,
			bool showInHelpText,
			bool monsterWeapon
	);

	// This could be default in C++11
	virtual ~CalibreModel();

	const ST::string* getName() const;

	virtual void serializeTo(JsonObject &obj) const;
	static CalibreModel* deserialize(JsonObjectReader &obj);

	static const CalibreModel* getNoCalibreObject();

	uint16_t index;
	ST::string internalName;
	ST::string sound;
	ST::string burstSound;
	ST::string silencedSound;
	ST::string silencedBurstSound;
	bool showInHelpText;
	bool monsterWeapon;
};

const CalibreModel* getCalibre(const ST::string& calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap);
