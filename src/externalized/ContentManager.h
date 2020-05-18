#pragma once

/* XXX */
#include "ItemModel.h"
#include "ContentMusic.h"

#include "Facts.h"
#include "ItemSystem.h"

#include <string_theory/string>

#include <map>
#include <stdint.h>
#include <vector>


class BloodCatPlacementsModel;
class BloodCatSpawnsModel;
class DealerInventory;
class FactParamsModel;
class GamePolicy;
class IMPPolicy;
class MovementCostsModel;
class NpcActionParamsModel;
class NpcPlacementModel;
class SamSiteModel;
class TownModel;
class UndergroundSectorModel;
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
	virtual ST::string getMapPath(const ST::string& mapName) const = 0;

	/** Get radar map resource name. */
	virtual ST::string getRadarMapResourceName(const ST::string &mapName) const = 0;

	/** Get tileset resource name. */
	virtual ST::string getTilesetResourceName(int number, ST::string fileName) const = 0;

	/** Get tileset db resource name. */
	virtual ST::string getTilesetDBResName() const = 0;

	/** Get directory for storing new map file. */
	virtual ST::string getNewMapFolder() const = 0;

	/** Get all available maps. */
	virtual std::vector<ST::string> getAllMaps() const = 0;

	/** Get all available tilecache. */
	virtual std::vector<ST::string> getAllTilecache() const = 0;

	/** Open map for reading. */
	virtual SGPFile* openMapForReading(const ST::string& mapName) const = 0;

	/** Open user's private file (e.g. saved game, settings) for reading. */
	virtual SGPFile* openUserPrivateFileForReading(const ST::string& filename) const = 0;

	/* Open a game resource file for reading. */
	virtual SGPFile* openGameResForReading(const char* filename) const = 0;
	virtual SGPFile* openGameResForReading(const ST::string& filename) const = 0;

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(char const* filename) const = 0;
	virtual bool doesGameResExists(const ST::string &filename) const = 0;

	/** Get folder for screenshots. */
	virtual ST::string getScreenshotFolder() const = 0;

	/** Get folder for video capture. */
	virtual ST::string getVideoCaptureFolder() const = 0;

	/** Get folder for saved games. */
	virtual ST::string getSavedGamesFolder() const = 0;

	/** Load encrypted string from game resource file. */
	virtual ST::string loadEncryptedString(const char* fileName, uint32_t seek_chars, uint32_t read_chars) const = 0;

	virtual ST::string loadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars) const = 0;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const char* filename, int quote_number) = 0;

	/** Get weapons with the give index. */
	virtual const WeaponModel* getWeapon(uint16_t index) = 0;
	virtual const WeaponModel* getWeaponByName(const ST::string &internalName) = 0;

	virtual const MagazineModel* getMagazineByName(const ST::string &internalName) = 0;
	virtual const MagazineModel* getMagazineByItemIndex(uint16_t itemIndex) = 0;
	virtual const std::vector<const MagazineModel*>& getMagazines() const = 0;

	virtual const CalibreModel* getCalibre(uint8_t index) = 0;
	virtual const ST::string* getCalibreName(uint8_t index) const = 0;
	virtual const ST::string* getCalibreNameForBobbyRay(uint8_t index) const = 0;

	virtual const AmmoTypeModel* getAmmoType(uint8_t index) = 0;

	virtual const ItemModel* getItem(uint16_t index) const = 0;
	virtual const std::map<uint16_t, uint16_t> getMapItemReplacements() const = 0;

	virtual const std::vector<std::vector<const WeaponModel*> > & getNormalGunChoice() const = 0;
	virtual const std::vector<std::vector<const WeaponModel*> > & getExtendedGunChoice() const = 0;

	virtual const DealerInventory* getDealerInventory(int dealerId) const = 0;
	virtual const DealerInventory* getBobbyRayNewInventory() const = 0;
	virtual const DealerInventory* getBobbyRayUsedInventory() const = 0;

	virtual const ST::string* getMusicForMode(MusicMode mode) const = 0;

	virtual const GamePolicy* getGamePolicy() const = 0;
	virtual const IMPPolicy* getIMPPolicy() const = 0;

	virtual const std::vector<const BloodCatPlacementsModel*> & getBloodCatPlacements() const = 0;
	virtual const std::vector<const BloodCatSpawnsModel*> & getBloodCatSpawns() const = 0;
	virtual const BloodCatSpawnsModel* getBloodCatSpawnsOfSector(uint8_t sectorId) const = 0;
	virtual const std::vector<const SamSiteModel*>& getSamSites() const = 0;
	virtual const int8_t findSamIDBySector(uint8_t sectorId) const = 0;
	virtual const SamSiteModel* findSamSiteBySector(uint8_t sectorId) const = 0;
	virtual const TownModel* getTown(int8_t townId) const = 0;
	virtual const std::map<int8_t, const TownModel*>& getTowns() const = 0;
	virtual const ST::string getTownName(uint8_t townId) const = 0;
	virtual const ST::string getTownLocative(uint8_t townId) const = 0;
	virtual const std::vector <const UndergroundSectorModel*> & getUndergroundSectors() const = 0;

	virtual const MovementCostsModel* getMovementCosts() const = 0;
	virtual const NpcPlacementModel* getNpcPlacement(uint8_t profileId) const = 0;
	
	/* Params for the given NPC_ACTION if found, or return an empty instance */
	virtual const NpcActionParamsModel* getNpcActionParams(uint16_t actionCode) const = 0;

	/* Params for the given NPC_ACTION if found, or return an empty instance */
	virtual const FactParamsModel* getFactParams(Fact fact) const = 0;

	virtual const ST::string* getNewString(size_t stringId) const = 0;

	/** Open temporary file for writing. */
	virtual SGPFile* openTempFileForWriting(const char* filename, bool truncate) const = 0;

	/** Open temporary file for reading. */
	virtual SGPFile* openTempFileForReading(const char* filename) const = 0;

	/** Open temporary file for appending. */
	virtual SGPFile* openTempFileForAppend(const char* filename) const = 0;

	/** Delete temporary file. */
	virtual void deleteTempFile(const char* filename) const = 0;
};
