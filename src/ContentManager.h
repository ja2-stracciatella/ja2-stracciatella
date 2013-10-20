#pragma once

#include <stdint.h>
#include <string>
#include <vector>

/* XXX */
#include "ItemModel.h"

#include "ItemSystem.h"

class DealerInventory;
class GamePolicy;
class IMPPolicy;
class UTF8String;
struct AmmoTypeModel;
struct CalibreModel;
struct MagazineModel;
struct SGPFile;
struct WeaponModel;

class ContentManager : public ItemSystem
{
public:
  virtual ~ContentManager() {};

  /** Get map file path. */
  virtual std::string getMapPath(const char *mapName) const = 0;
  virtual std::string getMapPath(const wchar_t *mapName) const = 0;

  /** Get radar map resource name. */
  virtual std::string getRadarMapResourceName(const std::string &mapName) const = 0;

  /** Get tileset resource name. */
  virtual std::string getTilesetResourceName(int number, std::string fileName) const = 0;

  /** Get tileset db resource name. */
  virtual std::string getTilesetDBResName() const = 0;

  /** Get directory for storing new map file. */
  virtual std::string getNewMapFolder() const = 0;

  /** Get all available maps. */
  virtual std::vector<std::string> getAllMaps() const = 0;

  /** Get all available tilecache. */
  virtual std::vector<std::string> getAllTilecache() const = 0;

  /** Open map for reading. */
  virtual SGPFile* openMapForReading(const std::string& mapName) const = 0;
  virtual SGPFile* openMapForReading(const wchar_t *mapName) const = 0;

  /** Open user's private file (e.g. saved game, settings) for reading. */
  virtual SGPFile* openUserPrivateFileForReading(const std::string& filename) const = 0;

  /* Open a game resource file for reading. */
  virtual SGPFile* openGameResForReading(const char* filename) const = 0;
  virtual SGPFile* openGameResForReading(const std::string& filename) const = 0;

  /* Checks if a game resource exists. */
  virtual bool doesGameResExists(char const* filename) const = 0;
  virtual bool doesGameResExists(const std::string &filename) const = 0;

  /** Get folder for screenshots. */
  virtual std::string getScreenshotFolder() const = 0;

  /** Get folder for video capture. */
  virtual std::string getVideoCaptureFolder() const = 0;

  /** Get folder for saved games. */
  virtual std::string getSavedGamesFolder() const = 0;

  /** Load encrypted string from game resource file. */
  virtual void loadEncryptedString(const char *fileName, wchar_t* DestString, uint32_t seek_chars, uint32_t read_chars) const = 0;

  virtual void loadEncryptedString(SGPFile* const File, wchar_t* DestString, uint32_t const seek_chars, uint32_t const read_chars) const = 0;

  /** Load dialogue quote from file. */
  virtual UTF8String* loadDialogQuoteFromFile(const char* filename, int quote_number) = 0;

  /** Get weapons with the give index. */
  virtual const WeaponModel* getWeapon(uint16_t index) = 0;
  virtual const WeaponModel* getWeaponByName(const std::string &internalName) = 0;

  virtual const MagazineModel* getMagazineByName(const std::string &internalName) = 0;
  virtual const MagazineModel* getMagazineByItemIndex(uint16_t itemIndex) = 0;
  virtual const std::vector<const MagazineModel*>& getMagazines() const = 0;

  virtual const CalibreModel* getCalibre(uint8_t index) = 0;
  virtual const UTF8String* getCalibreName(uint8_t index) const = 0;
  virtual const UTF8String* getCalibreNameForBobbyRay(uint8_t index) const = 0;

  virtual const AmmoTypeModel* getAmmoType(uint8_t index) = 0;

  virtual const ItemModel* getItem(uint16_t index) const = 0;

  virtual const std::vector<std::vector<const WeaponModel*> > & getNormalGunChoice() const = 0;
  virtual const std::vector<std::vector<const WeaponModel*> > & getExtendedGunChoice() const = 0;

  virtual const DealerInventory* getDealerInventory(int dealerId) const = 0;
  virtual const DealerInventory* getBobbyRayNewInventory() const = 0;
  virtual const DealerInventory* getBobbyRayUsedInventory() const = 0;

  virtual const GamePolicy* getGamePolicy() const = 0;
  virtual const IMPPolicy* getIMPPolicy() const = 0;

  /* /\** */
  /*  * get location of the game executable file. */
  /*  *\/ */
  /* virtual std::string getExeLocation() = 0; */
};
