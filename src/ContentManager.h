#pragma once

class ContentManager
{
public:
  virtual ~ContentManager() {};

  /** Get map file path. */
  virtual std::string getMapPath(const char *mapName) const = 0;
  virtual std::string getMapPath(const wchar_t *mapName) const = 0;

  /** Get directory for storing new map file. */
  virtual std::string getNewMapFolder() const = 0;

  /** Get all available maps. */
  virtual std::vector<std::string> getAllMaps() const = 0;

  /* /\** */
  /*  * Get location of the game executable file. */
  /*  *\/ */
  /* virtual std::string getExeLocation() = 0; */
};
