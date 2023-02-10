#pragma once

#include "Json.h"
#include "TraversibilityMapping.h"
#include "Types.h"

#include <map>

typedef uint8_t LandType;

namespace SectorLandTypes
{
	std::map<SGPSector, LandType> deserialize(const JsonValue& document, const TraversibilityMap &travMap);
};
