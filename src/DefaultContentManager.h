#pragma once

#include <string>
#include <vector>

#include "ContentManager.h"

class DefaultContentManager : public ContentManager
{
public:

  /** Get map file path. */
  virtual std::string getMapPath(const char *mapName) const;
  virtual std::string getMapPath(const wchar_t *mapName) const;

  /** Open map for reading. */
  virtual SGPFile* openMapForReading(const std::string& mapName) const;
  virtual SGPFile* openMapForReading(const wchar_t *mapName) const;

  /** Get directory for storing new map file. */
  virtual std::string getNewMapFolder() const;

  /** Get all available maps. */
  virtual std::vector<std::string> getAllMaps() const;
};
