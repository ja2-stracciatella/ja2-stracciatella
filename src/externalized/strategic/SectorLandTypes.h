#pragma once
#include "TraversibilityMapping.h"
#include <map>
#include <tuple>
#include <rapidjson/document.h>

typedef std::tuple<uint8_t, uint8_t> SectorKey;
typedef uint8_t LandType;

namespace SectorLandTypes
{
	std::map<SectorKey, LandType> deserialize(const rapidjson::Document& document, const TraversibilityMap &travMap);
};
