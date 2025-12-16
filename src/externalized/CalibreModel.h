#pragma once

#include "Containers.h"
#include "Json.h"
#include "TranslatableString.h"

#include <cstdint>
#include <map>
#include <stdint.h>
#include <string_theory/st_string.h>
#include <string_theory/string>

struct CalibreModel : public Containers::NamedEntity<uint16_t>
{
	CalibreModel() = delete;
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
	) : index(index_), internalName(std::move(internalName_)), name(std::move(name_)), bobbyRaysName(std::move(bobbyRaysName_)), sound(std::move(sound_)), burstSound(std::move(burstSound_)), silencedSound(std::move(silencedSound_)), silencedBurstSound(std::move(silencedBurstSound_)), showInHelpText(showInHelpText_), monsterWeapon(monsterWeapon_) {}

	static constexpr uint16_t NOAMMO = 0;
	static constexpr const char* ENTITY_NAME = "Calibre";
	virtual uint16_t getId() const override {
		return index;
	};
	virtual const ST::string& getInternalName() const override {
		return internalName;
	};

	[[nodiscard]] const ST::string& getName() const;
	[[nodiscard]] const ST::string& getBobbyRaysName() const;

	[[nodiscard]] JsonValue serialize() const;
	[[nodiscard]] static std::unique_ptr<CalibreModel> deserialize(const JsonValue &json, TranslatableString::Loader& stringLoader);

	uint16_t index;
	ST::string internalName;
	ST::string name;
	ST::string bobbyRaysName;
	ST::string sound;
	ST::string burstSound;
	ST::string silencedSound;
	ST::string silencedBurstSound;
	bool showInHelpText{ false };
	bool monsterWeapon{ false };

private:
	static const ST::string DEFAULT_NAME_TRANSLATION_PREFIX;
	static const ST::string DEFAULT_BOBBY_RAYS_NAME_TRANSLATION_PREFIX;
};

const CalibreModel* getCalibre(const ST::string& calibreName,
				const std::map<ST::string, const CalibreModel*> &calibreMap);
