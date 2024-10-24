#include "Debug.h"
#include "ExplosionAnimationModel.h"
#include "Exceptions.h"

#include <string_theory/format>

ExplosionAnimationModel::ExplosionAnimationModel(uint8_t id, ST::string&& name, uint8_t blastSpeed, uint8_t damageKeyframe, ST::string&& graphics, std::vector<ST::string>&& sounds, uint8_t transparentKeyframe) {
	this->id = id;
	this->name = std::move(name);
	this->blastSpeed = blastSpeed;
	this->damageKeyframe = damageKeyframe;
	this->graphics = std::move(graphics);
	this->sounds = std::move(sounds);
	this->transparentKeyframe = transparentKeyframe;
	this->waterAnimation = nullptr;
}

ExplosionAnimationModel* ExplosionAnimationModel::deserialize(const JsonValue &json) {
	auto obj = json.toObject();
	uint8_t id = obj.GetUInt("id");
	ST::string name = obj.GetString("name");
	uint8_t blastSpeed = obj.GetUInt("blastSpeed");
	uint8_t damageKeyframe = obj.GetUInt("damageKeyframe");
	ST::string graphics = obj.GetString("graphics");
	uint8_t transparentKeyframe = obj.GetUInt("transparentKeyframe");

	std::vector<ST::string> sounds = {};
	auto soundsVec = obj.GetValue("sounds");
	for (const auto& sound : soundsVec.toVec()) {
		sounds.push_back(sound.toString());
	}

	return new ExplosionAnimationModel(id, std::move(name), blastSpeed, damageKeyframe, std::move(graphics), std::move(sounds), transparentKeyframe);
}

std::vector<const ExplosionAnimationModel*> ExplosionAnimationModel::deserializeAll(const JsonValue &json) {
	auto jsonVec = json.toVec();
	std::vector<ExplosionAnimationModel*> explosionAnimations;

	for (auto& element : jsonVec) {
		auto model = ExplosionAnimationModel::deserialize(element);
		SLOGD("Loaded explosion animation {} {}", static_cast<uint16_t>(model->getID()), model->getName());

		explosionAnimations.push_back(model);
	}

	size_t index = 0;
	for (auto& element : jsonVec) {
		auto obj = element.toObject();
		auto waterAnimation = obj.getOptionalString("waterAnimation");
		auto explosionAnimation = explosionAnimations[index];

		if (!waterAnimation.empty()) {
			auto it = std::find_if(explosionAnimations.begin(), explosionAnimations.end(), [&waterAnimation](const ExplosionAnimationModel* item) -> bool {
				return item->getName() == waterAnimation;
			});

			if (it == explosionAnimations.end()) {
				throw DataError(ST::format("Could not find waterAnimation: '{}' for explosion animation '{}'", waterAnimation, explosionAnimation->getName()));
			}

			explosionAnimations[index]->setWaterAnimation(*it);

			index++;
		}
	}

	return std::vector<const ExplosionAnimationModel*>{ explosionAnimations.begin(), explosionAnimations.end() };
}

uint8_t ExplosionAnimationModel::getID() const {
	return id;
}

const ST::string& ExplosionAnimationModel::getName() const {
	return name;
}

uint8_t ExplosionAnimationModel::getBlastSpeed() const {
	return blastSpeed;
}

uint8_t ExplosionAnimationModel::getDamageKeyframe() const {
	return damageKeyframe;
}

uint8_t ExplosionAnimationModel::getTransparentKeyframe() const {
	return transparentKeyframe;
}

const ST::string& ExplosionAnimationModel::getGraphics() const {
	return graphics;
}

const std::vector<ST::string>& ExplosionAnimationModel::getSounds() const {
	return sounds;
}

const ExplosionAnimationModel* ExplosionAnimationModel::getWaterAnimation() const {
	return waterAnimation;
}

void ExplosionAnimationModel::setWaterAnimation(const ExplosionAnimationModel* waterAnimation) {
	this->waterAnimation = waterAnimation;
}
