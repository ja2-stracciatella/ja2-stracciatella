#pragma once

#include <vector>
#include <string>

#include "rapidjson/document.h"

namespace JsonUtility
{
  /** Write list of strings to file. */
  bool writeToFile(const char *name, const std::vector<std::string> &strings);

  /** Parse json to a list of strings. */
  bool parseJsonToListStrings(const char* jsonData, std::vector<std::string> &strings);

  /** Parse value as list of strings. */
  bool parseListStrings(const rapidjson::Value &value, std::vector<std::string> &strings);
}
