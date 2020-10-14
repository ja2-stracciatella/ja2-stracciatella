#pragma once

#include <map>
#include <rapidjson/document.h>
#include <string_theory/string>

typedef std::map<ST::string, uint8_t> TraversibilityMap;

namespace TraversibilityMapping
{
	TraversibilityMap deserialize(const rapidjson::Document& root);
}
