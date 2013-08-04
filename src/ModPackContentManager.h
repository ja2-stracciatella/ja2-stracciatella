#pragma once

#include <string>
#include <vector>

#include "DefaultContentManager.h"

class ModPackContentManager : public DefaultContentManager
{
public:
  ModPackContentManager(const std::string &modName,
                        const std::string &modResFolder,
                        const std::string &configFolder, const std::string &configPath,
                        const std::string &gameResRootPath);

  virtual SGPFile* openGameResForReading(const char* filename) const;
  virtual SGPFile* openGameResForReading(const std::string& filename) const;

protected:
  std::string m_modName;
  std::string m_modResFolder;
};
