#pragma once

#include "Json.h"

enum class SmokeEffectID {
	NOTHING,
	SMOKE,
	TEARGAS,
	MUSTARDGAS,
	CREATUREGAS
};

class SmokeEffectModel {
	public:
		SmokeEffectModel(
			SmokeEffectID index,
			ST::string name,
			ST::string graphics,
			ST::string dissipatingGraphics,
			ST::string staticGraphics,
			uint8_t damage,
			uint8_t breathDamage,
			uint8_t lostVisibilityPerTile,
			uint8_t maxVisibility,
			uint8_t maxVisibilityWhenAffected,
			bool ignoresGasMask,
			bool affectsRobot,
			bool affectsMonsters
		);
		static SmokeEffectModel* deserialize(uint16_t index, const JsonValue& value);

		SmokeEffectID getID() const;
		const ST::string& getName() const;
		const ST::string& getGraphics() const;
		const ST::string& getDissipatingGraphics() const;
		const ST::string& getStaticGraphics() const;
		uint8_t getDamage() const;
		uint8_t getBreathDamage() const;
		uint8_t getLostVisibilityPerTile() const;
		uint8_t getMaxVisibility() const;
		uint8_t getMaxVisibilityWhenAffected() const;
		bool dealsAnyDamage() const;
		bool getIgnoresGasMask() const;
		bool getAffectsRobot() const;
		bool getAffectsMonsters() const;
	private:
		SmokeEffectID id;
		ST::string name;
		ST::string graphics;
		ST::string dissipatingGraphics;
		ST::string staticGraphics;
		uint8_t damage;
		uint8_t breathDamage;
		uint8_t lostVisibilityPerTile;
		uint8_t maxVisibility;
		uint8_t maxVisibilityWhenAffected;
		bool ignoresGasMask;
		bool affectsRobot;
		bool affectsMonsters;
};
