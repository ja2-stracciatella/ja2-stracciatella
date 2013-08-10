#include "ModPackContentManager.h"

#include "sgp/FileMan.h"
// #include "sgp/LibraryDataBase.h"
// #include "sgp/MemMan.h"
// #include "sgp/StrUtils.h"
// #include "sgp/UTF8String.h"

#include "slog/slog.h"
#define TAG "ModPackCM"

#define DEBUG_PRINT_OPENING_FILES (1)

ModPackContentManager::ModPackContentManager(const std::string &modName,
                                             const std::string &modResFolder,
                                             const std::string &configFolder, const std::string &configPath,
                                             const std::string &gameResRootPath)
  :DefaultContentManager(configFolder, configPath, gameResRootPath)
{
  m_modName = modName;
  m_modResFolder = modResFolder;
}

/* Open a game resource file for reading.
 *
 * First trying to open resource in the mod's directory.
 * If not found, use the previous method. */
SGPFile* ModPackContentManager::openGameResForReading(const char* filename) const
{
  int mode;
  const char* fmode = GetFileOpenModeForReading(&mode);

  int d = FileMan::openFileCaseInsensitive(m_modResFolder, filename, mode);
  if (d < 0)
  {
    return DefaultContentManager::openGameResForReading(filename);
  }
  else
  {
    SLOGI(TAG, "opening mod's resource: %s", filename);
    return FileMan::getSGPFileFromFD(d, filename, fmode);
  }
}

SGPFile* ModPackContentManager::openGameResForReading(const std::string& filename) const
{
  return openGameResForReading(filename.c_str());
}

/** Get folder for saved games. */
std::string ModPackContentManager::getSavedGamesFolder() const
{
  std::string folderName = std::string("SavedGames-") + m_modName;
  return FileMan::joinPaths(m_configFolder, folderName);
}
