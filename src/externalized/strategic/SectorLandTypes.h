#pragma once
#include "TraversibilityMapping.h"
#include "Types.h"
#include <map>
#include <rapidjson/document.h>

typedef uint8_t LandType;

namespace SectorLandTypes
{
	std::map<SGPSector, LandType> deserialize(const rapidjson::Document& document, const TraversibilityMap &travMap);
};
