#include "DefaultContentManager.h"

#include "Build/GameRes.h"
#include "sgp/FileMan.h"
#include "sgp/LibraryDataBase.h"
#include "sgp/MemMan.h"
#include "sgp/UTF8String.h"

#include "slog/slog.h"
#define TAG "DefaultCM"

#define MAPSDIR        "maps"

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
  return FileMan::joinPaths(FileMan::getDataDirPath(), MAPSDIR);
}

/** Get all available maps. */
std::vector<std::string> DefaultContentManager::getAllMaps() const
{
  return FindFilesInDir(MAPSDIR, ".dat", true, true, true);
}

void DefaultContentManager::initGameResources()
{
  m_dataDir = FileMan::getDataDirPath();
  InitGameResources(m_dataDir);
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

