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

  /** Get radar map resource name. */
  virtual std::string getRadarMapResourceName(const std::string &mapName) const;

  /** Open map for reading. */
  virtual SGPFile* openMapForReading(const std::string& mapName) const;
  virtual SGPFile* openMapForReading(const wchar_t *mapName) const;

  /** Get directory for storing new map file. */
  virtual std::string getNewMapFolder() const;

  /** Get all available maps. */
  virtual std::vector<std::string> getAllMaps() const;

  /** Get all available tilecache. */
  virtual std::vector<std::string> getAllTilecache() const;

  virtual void initGameResources(const std::string &configFolder, const std::string &configPath,
                                 const std::string &gameResRootPath, std::string &dataDir, std::string &tileDir);

  /* XXX: is openForReadingSmart to generic? */
  virtual SGPFile* openForReadingSmart(const char* filename, bool useSmartLookup) const;
  virtual SGPFile* openForReadingSmart(const std::string& filename, bool useSmartLookup) const;

  /* Checks if a game resource exists. */
  virtual bool doesGameResExists(char const* filename) const;
  virtual bool doesGameResExists(const std::string &filename) const;

  /** Get folder for screenshots. */
  virtual std::string getScreenshotFolder() const;

  /** Get folder for video capture. */
  virtual std::string getVideoCaptureFolder() const;

protected:
  std::string m_dataDir;
  std::string m_tileDir;
  std::string m_configFolder;
  std::string m_gameResRootPath;
};

class LibraryFileNotFoundException : public std::runtime_error
{
public:
  LibraryFileNotFoundException(const std::string& what_arg)
    :std::runtime_error(what_arg)
  {
  }
};
