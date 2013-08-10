#pragma once

#include <vector>
#include <string>

namespace JsonUtility
{
  /** Write list of strings to file. */
  bool writeToFile(const char *name, const std::vector<std::string> &strings);
}
