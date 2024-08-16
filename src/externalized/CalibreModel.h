#pragma once

#include "Json.h"

#include <map>
#include <stdint.h>
#include <string_theory/string>

struct CalibreModel
{
	static constexpr uint16_t NOAMMO = 0;

	[[nodiscard]] const ST::string* getName() const;

	[[nodiscard]] JsonValue serialize() const;
	[[nodiscard]] static CalibreModel* deserialize(const JsonValue &json);

	[[nodiscard]] static const CalibreModel* getNoCalibreObject();

	uint16_t index{ NOAMMO };
	ST::string internalName{ "NO_CALIBRE" };
	ST::string sound;
	ST::string burstSound;
	ST::string silencedSound;
	ST::string silencedBurstSound;
	bool showInHelpText{ false };
	bool monsterWeapon{ false };

private:
	CalibreModel() = default;
	CalibreModel(JsonObject jsonObj);
};

const CalibreModel* getCalibre(const ST::string& calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap);
