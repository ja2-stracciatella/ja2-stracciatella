#include "DefaultContentManager.h"

#include "FileMan.h"
#include "UTF8String.h"

#include "slog/slog.h"
#define TAG "DefaultCM"

#define MAPSDIR        "maps"

/** Get map file path. */
std::string DefaultContentManager::getMapPath(const char *mapName) const
{
  std::string result = MAPSDIR;
  result += "/";
  result += mapName;

  SLOGD(TAG, "map file %s", result.c_str());

  return result;
}

std::string DefaultContentManager::getMapPath(const wchar_t *mapName) const
{
  SLOGW(TAG, "converting wchar to char");

  // This will not work for non-latin names.
  // But it is just a hack to make the code compile.
  // XXX: This method should be removed altogether

  UTF8String str(mapName);
  return getMapPath(str.getUTF8());
}

/** Open map for reading. */
SGPFile* DefaultContentManager::openMapForReading(const std::string& mapName) const
{
  return FileMan::openForReadingSmart(getMapPath(mapName.c_str()), true);
}

SGPFile* DefaultContentManager::openMapForReading(const wchar_t *mapName) const
{
  return FileMan::openForReadingSmart(getMapPath(mapName), true);
}

/** Get directory for storing new map file. */
std::string DefaultContentManager::getNewMapFolder() const
{
  return FileMan::joinPaths(FileMan::getDataDirPath(), MAPSDIR);
}

/** Get all available maps. */
std::vector<std::string> DefaultContentManager::getAllMaps() const
{
  return FindFilesInDir(MAPSDIR, ".dat", true, true, true);
}
