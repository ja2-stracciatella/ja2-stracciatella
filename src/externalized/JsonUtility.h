#pragma once

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
}
