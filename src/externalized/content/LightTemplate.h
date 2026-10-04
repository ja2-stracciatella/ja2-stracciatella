#pragma once

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

struct LightTemplate
{
	std::vector<LIGHT_NODE> lights;
	std::vector<uint16_t>   rays;
	ST::string name;

	static std::unique_ptr<LightTemplate> deserialize(const JsonValue& json);
};
