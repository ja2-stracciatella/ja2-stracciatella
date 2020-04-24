#pragma once

#include "game/GameRes.h"

#include "ContentManager.h"
#include "ContentMusic.h"
#include "IGameDataLoader.h"
#include "StringEncodingTypes.h"

#include "rapidjson/document.h"
#include <string_theory/string>

#include <map>
#include <stdexcept>
#include <vector>


struct LibraryDB;

class DefaultContentManager : public ContentManager, public IGameDataLoader
{
public:

	DefaultContentManager(GameVersion gameVersion,
				const ST::string &configFolder,
				const ST::string &gameResRootPath,
				const ST::string &externalizedDataPath);

	virtual ~DefaultContentManager() override;

	/** Get list of game resources. */
	virtual std::vector<ST::string> getListOfGameResources() const;

	/** Initialize game resources. */
	virtual void initGameResouces(const ST::string &stracciatellaHomeDir, const std::vector<ST::string> &libraries);
	virtual void addExtraResources(const ST::string &baseDir, const ST::string &library);

	/** Load the game data. */
	bool loadGameData();

	/** Get map file path. */
	virtual ST::string getMapPath(const ST::string& mapName) const override;

	/** Get radar map resource name. */
	virtual ST::string getRadarMapResourceName(const ST::string &mapName) const override;

	/** Get tileset resource name. */
	virtual ST::string getTilesetResourceName(int number, ST::string fileName) const override;

	/** Get tileset db resource name. */
	virtual ST::string getTilesetDBResName() const override;

	/** Open map for reading. */
	virtual SGPFile* openMapForReading(const ST::string& mapName) const override;

	/** Get directory for storing new map file. */
	virtual ST::string getNewMapFolder() const override;

	/** Get all available maps. */
	virtual std::vector<ST::string> getAllMaps() const override;

	/** Get all available tilecache. */
	virtual std::vector<ST::string> getAllTilecache() const override;

	/* Open a game resource file for reading. */
	virtual SGPFile* openGameResForReading(const char* filename) const override;
	virtual SGPFile* openGameResForReading(const ST::string& filename) const override;

	/** Open temporary file for writing. */
	virtual SGPFile* openTempFileForWriting(const char* filename, bool truncate) const override;

	/** Open temporary file for reading. */
	virtual SGPFile* openTempFileForReading(const char* filename) const override;

	/** Open temporary file for appending. */
	virtual SGPFile* openTempFileForAppend(const char* filename) const override;

	/** Delete temporary file. */
	virtual void deleteTempFile(const char* filename) const override;

	/** Open user's private file (e.g. saved game, settings) for reading. */
	virtual SGPFile* openUserPrivateFileForReading(const ST::string& filename) const override;

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(char const* filename) const override;
	virtual bool doesGameResExists(const ST::string &filename) const override;

	/** Get folder for screenshots. */
	virtual ST::string getScreenshotFolder() const override;

	/** Get folder for video capture. */
	virtual ST::string getVideoCaptureFolder() const override;

	const ST::string& getDataDir() { return m_dataDir; }
	const ST::string& getTileDir() { return m_tileDir; }

	const ST::string& getExternalizedDataDir() { return m_externalizedDataPath; }

	/** Get folder for saved games. */
	virtual ST::string getSavedGamesFolder() const override;

	/** Load encrypted string from game resource file. */
	virtual ST::string loadEncryptedString(const char* fileName, uint32_t seek_chars, uint32_t read_chars) const override;

	virtual ST::string loadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars) const override;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const char* filename, int quote_number) override;

	/** Load all dialogue quotes for a character. */
	void loadAllDialogQuotes(STRING_ENC_TYPE encType, const char* filename, std::vector<ST::string*> &quotes) const;

	/** Get weapons with the give index. */
	virtual const WeaponModel* getWeapon(uint16_t index) override;
	virtual const WeaponModel* getWeaponByName(const ST::string &internalName) override;

	virtual const MagazineModel* getMagazineByName(const ST::string &internalName) override;
	virtual const MagazineModel* getMagazineByItemIndex(uint16_t itemIndex) override;
	virtual const std::vector<const MagazineModel*>& getMagazines() const override;

	virtual const CalibreModel* getCalibre(uint8_t index) override;
	virtual const ST::string* getCalibreName(uint8_t index) const override;
	virtual const ST::string* getCalibreNameForBobbyRay(uint8_t index) const override;

	virtual const AmmoTypeModel* getAmmoType(uint8_t index) override;

	virtual const ItemModel* getItem(uint16_t index) const override;
	virtual const ItemModel* getItemByName(const ST::string &internalName) const override;

	virtual const std::vector<std::vector<const WeaponModel*> > & getNormalGunChoice() const override;
	virtual const std::vector<std::vector<const WeaponModel*> > & getExtendedGunChoice() const override;

	virtual const DealerInventory* getDealerInventory(int dealerId) const override;
	virtual const DealerInventory* getBobbyRayNewInventory() const override;
	virtual const DealerInventory* getBobbyRayUsedInventory() const override;

	virtual const ST::string* getMusicForMode(MusicMode mode) const override;

	virtual const GamePolicy* getGamePolicy() const override;
	virtual const IMPPolicy* getIMPPolicy() const override;

	virtual const ST::string* getNewString(size_t stringId) const override;

	virtual const std::vector<const BloodCatPlacementsModel*>& getBloodCatPlacements() const override;
	virtual const std::vector<const BloodCatSpawnsModel*>& getBloodCatSpawns() const override;
	virtual const BloodCatSpawnsModel* getBloodCatSpawnsOfSector(uint8_t sectorId) const override;
	virtual const TownModel* getTown(int8_t townId) const  override;
	virtual const std::map<int8_t, const TownModel*>& getTowns() const override;
	virtual const ST::string getTownName(uint8_t townId) const override;
	virtual const ST::string getTownLocative(uint8_t townId) const override;
	virtual const MovementCostsModel* getMovementCosts() const override;
	virtual const NpcPlacementModel* getNpcPlacement(uint8_t profileId) const override;

protected:
	ST::string m_dataDir;
	ST::string m_tileDir;
	ST::string m_configFolder;
	ST::string m_gameResRootPath;
	ST::string m_externalizedDataPath;

	const GameVersion m_gameVersion;

	std::vector<const ST::string*> m_newStrings;

	std::vector<const ItemModel*> m_items;
	std::vector<const MagazineModel*> m_magazines;

	std::vector<const CalibreModel*> m_calibres;
	std::vector<const ST::string*> m_calibreNames;
	std::vector<const ST::string*> m_calibreNamesBobbyRay;

	std::vector<AmmoTypeModel*> m_ammoTypes;

	/** Mapping of calibre names to objects. */
	std::map<ST::string, const AmmoTypeModel*> m_ammoTypeMap;
	std::map<ST::string, const CalibreModel*> m_calibreMap;
	std::map<ST::string, const MagazineModel*> m_magazineMap;
	std::map<ST::string, const WeaponModel*> m_weaponMap;
	std::map<ST::string, const ItemModel*> m_itemMap;
	std::map<MusicMode, const std::vector<const ST::string*>*> m_musicMap;

	std::vector<std::vector<const WeaponModel*> > mNormalGunChoice;
	std::vector<std::vector<const WeaponModel*> > mExtendedGunChoice;

	std::vector<const DealerInventory*> m_dealersInventory;
	const DealerInventory *m_bobbyRayNewInventory;
	const DealerInventory *m_bobbyRayUsedInventory;
	const IMPPolicy *m_impPolicy;
	const GamePolicy *m_gamePolicy;

	std::vector<const BloodCatPlacementsModel*> m_bloodCatPlacements;
	std::vector<const BloodCatSpawnsModel*> m_bloodCatSpawns;
	std::map<int8_t, const TownModel*> m_towns;
	std::vector<const ST::string*> m_townNames;
	std::vector<const ST::string*> m_townNameLocatives;
	const MovementCostsModel *m_movementCosts;
	std::map<uint8_t, const NpcPlacementModel*> m_npcPlacements;

	RustPointer<LibraryDB> m_libraryDB;

	bool loadWeapons();
	bool loadMagazines();
	bool loadCalibres();
	bool loadAmmoTypes();
	bool loadArmyGunChoice();
	bool loadMusicModeList(MusicMode mode, rapidjson::Value &array);
	bool loadMusic();

	const DealerInventory * loadDealerInventory(const char *fileName);
	bool loadAllDealersInventory();
	void loadStringRes(const char *name, std::vector<const ST::string*> &strings) const;

	bool readWeaponTable(
		const char *fileName,
		std::vector<std::vector<const WeaponModel*> > & weaponTable);

	bool loadStrategicLayerData();

	rapidjson::Document* readJsonDataFile(const char *fileName) const;
};

class LibraryFileNotFoundException : public std::runtime_error
{
public:
	LibraryFileNotFoundException(const ST::string& what_arg)
		:std::runtime_error(what_arg.to_std_string())
	{
	}
};
