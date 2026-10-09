#pragma once

#include "Containers.h"
#include "Json.h"

#include <stdint.h>

// stucture of node in linked list for lights
struct LIGHT_NODE
{
	int16_t  iDX;
	int16_t  iDY;
	uint16_t uiFlags;
	uint8_t  ubLight;
};

struct LightTemplate : public Containers::NamedEntity<uint8_t>
{
	LightTemplate() : LightTemplate(0, ST::string{}, std::vector<uint16_t>{}, std::vector<LIGHT_NODE>{}) {}
	LightTemplate(
		uint8_t      index_,
		ST::string&& internalName_,
		std::vector<uint16_t>&&   rays_,
		std::vector<LIGHT_NODE>&& lights_
	) : index(index_), internalName(std::move(internalName_)), rays(std::move(rays_)), lights(std::move(lights_)) {}

	static constexpr const char* ENTITY_NAME = "LightTemplate";
	virtual uint8_t getId() const override {
		return index;
	};
	virtual const ST::string& getInternalName() const override {
		return internalName;
	};

	uint8_t    index;
	ST::string internalName;
	std::vector<uint16_t>   rays;
	std::vector<LIGHT_NODE> lights;

	static std::unique_ptr<LightTemplate> deserialize(const JsonValue& json);
};

class LightTemplatesContainer : public Containers::Named<uint8_t, LightTemplate> {
	
};
