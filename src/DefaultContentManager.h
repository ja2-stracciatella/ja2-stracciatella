#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "ContentManager.h"
#include "StringEncodingTypes.h"

class LibraryDB;

class DefaultContentManager : public ContentManager
{
public:

  DefaultContentManager(const std::string &configFolder, const std::string &configPath,
                        const std::string &gameResRootPath);

  ~DefaultContentManager();

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

  const std::string& getDataDir() { return m_dataDir; }
  const std::string& getTileDir() { return m_tileDir; }

  /** Get folder for saved games. */
  virtual std::string getSavedGamesFolder() const;

  /** Load encrypted string from game resource file. */
  virtual void loadEncryptedString(const char *fileName, wchar_t* DestString, uint32_t seek_chars, uint32_t read_chars) const;

  virtual void loadEncryptedString(SGPFile* const File, wchar_t* DestString, uint32_t const seek_chars, uint32_t const read_chars) const;

  /** Load dialogue quote from file. */
  virtual UTF8String* loadDialogQuoteFromFile(const char* filename, int quote_number) const;

  /** Load all dialogue quotes for a character. */
  void loadAllDialogQuotes(STRING_ENC_TYPE encType, const char* filename, std::vector<UTF8String*> &quotes) const;

protected:
  std::string m_dataDir;
  std::string m_tileDir;
  std::string m_configFolder;
  std::string m_gameResRootPath;

  LibraryDB *m_libraryDB;
};

class LibraryFileNotFoundException : public std::runtime_error
{
public:
  LibraryFileNotFoundException(const std::string& what_arg)
    :std::runtime_error(what_arg)
  {
  }
};
