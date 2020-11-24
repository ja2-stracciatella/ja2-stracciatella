#pragma once

#include "GameSettings.h"
#include "rapidjson/document.h"
#include <string_theory/string>

#include <vector>


namespace JsonUtility
{
	/** Write list of strings to file. */
	bool writeToFile(const char *name, const std::vector<ST::string> &strings);

	/** Parse json to a list of strings. */
	bool parseJsonToListStrings(const char* jsonData, std::vector<ST::string> &strings);

	/** Parse value as list of strings. */
	bool parseListStrings(const rapidjson::Value &value, std::vector<ST::string> &strings);

	/** Parse a sector string to sector ID */
	uint8_t parseSectorID(const ST::string& sectorShortString);

	/** Parse a given string field to sector ID */
	uint8_t parseSectorID(const rapidjson::Value& json, const char* fieldName);

	/** Parse a given string array field to list of sector IDs */
	std::vector<uint8_t> parseSectorList(const rapidjson::Value& json, const char* fieldName);

	/** Parse a given integer array field into a list keyed by game difficulty */
	std::array<uint8_t, NUM_DIF_LEVELS> readIntArrayByDiff(const rapidjson::Value& obj, const char* fieldName);
}
