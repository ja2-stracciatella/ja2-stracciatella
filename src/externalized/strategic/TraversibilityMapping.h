#pragma once

#include "Json.h"

#include <map>
#include <string_theory/string>

typedef std::map<ST::string, uint8_t> TraversibilityMap;

namespace TraversibilityMapping
{
	TraversibilityMap deserialize(const JsonValue& root);
}
