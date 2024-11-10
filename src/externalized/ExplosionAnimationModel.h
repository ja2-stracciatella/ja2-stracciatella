#pragma once

#include "Json.h"

#include <string_theory/string>

class ExplosionAnimationModel {
	public:
		ExplosionAnimationModel(uint8_t id, ST::string&& name, uint8_t blastSpeed, uint8_t damageKeyframe, ST::string&& graphics, std::vector<ST::string>&& sounds, uint8_t transparentKeyframe);

		uint8_t getID() const;
		const ST::string& getName() const;
		uint8_t getBlastSpeed() const;
		uint8_t getDamageKeyframe() const;
		const ST::string& getGraphics() const;
		const std::vector<ST::string>& getSounds() const;
		uint8_t getTransparentKeyframe() const;
		const ExplosionAnimationModel* getWaterAnimation() const;

		static ExplosionAnimationModel* deserialize(const JsonValue &json);
		static std::vector<const ExplosionAnimationModel*> deserializeAll(const JsonValue &json);
	private:
		uint8_t id;
		ST::string name;
		uint8_t blastSpeed;
		uint8_t damageKeyframe;
		ST::string graphics;
		std::vector<ST::string> sounds;
		uint8_t transparentKeyframe;

		// This needs to be set after all explosion animations have been loaded, as it's a cross-reference to a different explosion animation
		const ExplosionAnimationModel* waterAnimation;
		void setWaterAnimation(const ExplosionAnimationModel* animation);
};
