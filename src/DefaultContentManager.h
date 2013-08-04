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

  /** Get tileset resource name. */
  virtual std::string getTilesetResourceName(int number, std::string fileName) const;

  /** Get tileset db resource name. */
  virtual std::string getTilesetDBResName() const;

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

  /* Open a game resource file for reading. */
  virtual SGPFile* openGameResForReading(const char* filename) const;
  virtual SGPFile* openGameResForReading(const std::string& filename) const;

  /** Open user's private file (e.g. saved game, settings) for reading. */
  virtual SGPFile* openUserPrivateFileForReading(const std::string& filename) const;

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
