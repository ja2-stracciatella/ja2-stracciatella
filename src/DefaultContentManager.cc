#include "DefaultContentManager.h"

#include <stdexcept>

#include "Build/Directories.h"
#include "Build/GameRes.h"

// XXX
#include "Build/GameState.h"

#include "sgp/FileMan.h"
#include "sgp/LibraryDataBase.h"
#include "sgp/MemMan.h"
#include "sgp/StrUtils.h"
#include "sgp/UTF8String.h"

#include "slog/slog.h"
#define TAG "DefaultCM"

#define BASEDATADIR    "data"

#define MAPSDIR        "maps"
#define RADARMAPSDIR   "radarmaps"
#define TILESETSDIR    "tilesets"

#define DEBUG_PRINT_OPENING_FILES               (1)             /**< Flag telling to print names of the opening files. */

/** Get map file path. */
std::string DefaultContentManager::getMapPath(const char *mapName) const
{
  std::string result = MAPSDIR;
  result += "/";
  result += mapName;

  SLOGD(TAG, "map file %s", result.c_str());

  return result;
}

/** Get radar map resource name. */
std::string DefaultContentManager::getRadarMapResourceName(const std::string &mapName) const
{
  std::string result = RADARMAPSDIR;
  result += "/";
  result += mapName;

  SLOGD(TAG, "map file %s", result.c_str());

  return result;
}

/** Get tileset resource name. */
std::string DefaultContentManager::getTilesetResourceName(int number, std::string fileName) const
{
  return FormattedString("%s/%d/%s", TILESETSDIR, number, fileName.c_str());
}


/** Get tileset db resource name. */
std::string DefaultContentManager::getTilesetDBResName() const
{
  return BINARYDATADIR "/ja2set.dat";
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
  return openForReadingSmart(getMapPath(mapName.c_str()), true);
}

SGPFile* DefaultContentManager::openMapForReading(const wchar_t *mapName) const
{
  return openForReadingSmart(getMapPath(mapName), true);
}

/** Get directory for storing new map file. */
std::string DefaultContentManager::getNewMapFolder() const
{
  return FileMan::joinPaths(m_dataDir, MAPSDIR);
}

/** Get all available maps. */
std::vector<std::string> DefaultContentManager::getAllMaps() const
{
  return FindFilesInDir(MAPSDIR, ".dat", true, true, true);
}

/** Get all available tilecache. */
std::vector<std::string> DefaultContentManager::getAllTilecache() const
{
  return FindFilesInDir(m_tileDir, ".jsd", true, false);
}

void DefaultContentManager::initGameResources(const std::string &configFolder, const std::string &configPath, const std::string &gameResRootPath, std::string &dataDir, std::string &tileDir)
{
  /** Find actual paths to directories 'Data' and 'Data/Tilecache', 'Data/Maps'
   * On case-sensitive filesystems that might be tricky: if such directories
   * exist we should use them.  If doesn't exist, then use lowercased names.
   */

  m_configFolder = configFolder;
  m_gameResRootPath = gameResRootPath;

  dataDir = FileMan::joinPaths(gameResRootPath, BASEDATADIR);
  tileDir = FileMan::joinPaths(dataDir, TILECACHEDIR);

#if CASE_SENSITIVE_FS

  // need to find precise names of the directories

  std::string name;
  if(findObjectCaseInsensitive(s_gameResRootPath.c_str(), BASEDATADIR, false, true, name))
  {
    dataDir = FileMan::joinPaths(s_gameResRootPath, name);
  }

  if(findObjectCaseInsensitive(dataDir.c_str(), TILECACHEDIR, false, true, name))
  {
    tileDir = FileMan::joinPaths(dataDir, name);
  }
#endif

  m_dataDir = dataDir;
  m_tileDir = tileDir;
  std::vector<std::string> libraries = GetResourceLibraries(m_dataDir);

  // XXX
  if(GameState::getInstance()->isEditorMode())
  {
    libraries.push_back("editor.slf");
  }

  const char *failedLib = InitializeFileDatabase(dataDir, libraries);
  if(failedLib)
  {
    std::string message = FormattedString(
      "Library '%s' is not found in folder '%s'.\n\nPlease make sure that '%s' contains files of the original game.  You can change this path by editing file '%s'.\n",
      failedLib, m_dataDir.c_str(), m_gameResRootPath.c_str(), configPath.c_str());
    throw LibraryFileNotFoundException(message);
  }
}

/** Open file for reading only.
 * When using the smart lookup:
 *  - first try to open file normally.
 *    It will work if the path is absolute and the file is found or path is relative to the current directory
 *    and file is present;
 *  - if file is not found, try to find the file relatively to 'Data' directory;
 *  - if file is not found, try to find the file in libraries located in 'Data' directory; */
SGPFile* DefaultContentManager::openForReadingSmart(const char* filename, bool useSmartLookup) const
{
  int         mode;
  const char* fmode = GetFileOpenModeForReading(&mode);

  int d;

  {
    d = FileMan::openFileForReading(filename, mode);
    if ((d < 0) && useSmartLookup)
    {
      // failed to open file in the local directory
      // let's try Data
      d = FileMan::openFileCaseInsensitive(m_dataDir, filename, mode);
      if (d < 0)
      {
        LibraryFile libFile;
        memset(&libFile, 0, sizeof(libFile));

        // failed to open in the data dir
        // let's try libraries

        // XXX: need to optimize this
        // XXX: the whole LibraryDataBase thing requires refactoring
        std::string _filename(filename);
        FileMan::slashifyPath(_filename);
        if (FindFileInTheLibrarry(_filename, &libFile))
        {
#if DEBUG_PRINT_OPENING_FILES
          SLOGD(TAG, "Opened file (from library ): %s", filename);
#endif
          SGPFile *file = MALLOCZ(SGPFile);
          file->flags = SGPFILE_NONE;
          file->u.lib = libFile;
          return file;
        }
      }
      else
      {
#if DEBUG_PRINT_OPENING_FILES
        SLOGD(TAG, "Opened file (from data dir): %s", filename);
#endif
      }
    }
    else
    {
#if DEBUG_PRINT_OPENING_FILES
      SLOGD(TAG, "Opened file (current dir  ): %s", filename);
#endif
    }
  }

  return FileMan::getSGPFileFromFD(d, filename, fmode);
}

/** Open user's private file (e.g. saved game, settings) for reading. */
SGPFile* DefaultContentManager::openUserPrivateFileForReading(const std::string& filename) const
{
  int         mode;
  const char* fmode = GetFileOpenModeForReading(&mode);

  int d = FileMan::openFileForReading(filename.c_str(), mode);
  return FileMan::getSGPFileFromFD(d, filename.c_str(), fmode);
}

SGPFile* DefaultContentManager::openForReadingSmart(const std::string& filename, bool useSmartLookup) const
{
  return openForReadingSmart(filename.c_str(), useSmartLookup);
}

/* Checks if a game resource exists. */
bool DefaultContentManager::doesGameResExists(char const* filename) const
{
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		char path[512];
		snprintf(path, lengthof(path), "%s/%s", m_dataDir.c_str(), filename);
		file = fopen(path, "rb");
		if (!file) return CheckIfFileExistInLibrary(filename);
	}

	fclose(file);
	return true;
}

bool DefaultContentManager::doesGameResExists(const std::string &filename) const
{
  return doesGameResExists(filename.c_str());
}

std::string DefaultContentManager::getScreenshotFolder() const
{
  return m_configFolder;
}

std::string DefaultContentManager::getVideoCaptureFolder() const
{
  return m_configFolder;
}
