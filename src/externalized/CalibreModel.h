#pragma once

#include "Json.h"
#include "TranslatableString.h"

#include <cstdint>
#include <map>
#include <stdint.h>
#include <string_theory/st_string.h>
#include <string_theory/string>

struct CalibreModel
{
	static constexpr uint16_t NOAMMO = 0;

	[[nodiscard]] const ST::string& getName() const;
	[[nodiscard]] const ST::string& getBobbyRaysName() const;

	[[nodiscard]] JsonValue serialize() const;
	[[nodiscard]] static CalibreModel* deserialize(const JsonValue &json, TranslatableString::Loader& stringLoader);

	[[nodiscard]] static const CalibreModel* getNoCalibreObject();

	uint16_t index{ NOAMMO };
	ST::string internalName{ "NO_CALIBRE" };
	ST::string name;
	ST::string bobbyRaysName;
	ST::string sound;
	ST::string burstSound;
	ST::string silencedSound;
	ST::string silencedBurstSound;
	bool showInHelpText{ false };
	bool monsterWeapon{ false };

private:
	CalibreModel() = default;
	CalibreModel(
		uint16_t index_,
		ST::string&& internalName_,
		ST::string&& name_,
		ST::string&& bobbyRaysName_,
		ST::string&& sound_,
		ST::string&& burstSound_,
		ST::string&& silencedSound_,
		ST::string&& silencedBurstSound_,
		bool showInHelpText_,
		bool monsterWeapon_
	) : index(index_), internalName(internalName_), name(name_), bobbyRaysName(bobbyRaysName_), sound(sound_), burstSound(burstSound_), silencedSound(silencedSound_), silencedBurstSound(silencedBurstSound_), showInHelpText(showInHelpText_), monsterWeapon(monsterWeapon_) {}

	static const ST::string DEFAULT_NAME_TRANSLATION_PREFIX;
	static const ST::string DEFAULT_BOBBY_RAYS_NAME_TRANSLATION_PREFIX;
};

const CalibreModel* getCalibre(const ST::string& calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap);
