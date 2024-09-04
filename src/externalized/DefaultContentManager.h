#pragma once

/* This header is somewhat private, unless you are constructing an instance
 * of this class or inheriting from it there should not be any reason to
 * include this file instead of ContentManager.h.
 */
#include "game/GameRes.h"

#include "ContentManager.h"
#include "ContentMusic.h"
#include "IEDT.h"
#include "IGameDataLoader.h"
#include "ItemStrings.h"
#include "RustInterface.h"
#include "StringEncodingTypes.h"

#include <string_theory/string>

#include <map>
#include <memory>
#include <string_view>
#include <vector>

class DefaultContentManager : public ContentManager, public IGameDataLoader
{
public:
	DefaultContentManager(RustPointer<EngineOptions> engineOptions);

	virtual ~DefaultContentManager() override;

	void logConfiguration() const override;

	/** Load the game data. */
	virtual bool loadGameData();

	/** Get map file path. */
	virtual ST::string getMapPath(const ST::string& mapName) const override;

	/** Get radar map resource name. */
	virtual ST::string getRadarMapResourceName(const ST::string &mapName) const override;

	/** Get tileset resource name. */
	virtual ST::string getTilesetResourceName(int number, const ST::string& fileName) const override;

	/** Get tileset db resource name. */
	virtual ST::string getTilesetDBResName() const override;

	/** Open map for reading. */
	virtual SGPFile* openMapForReading(const ST::string& mapName) const override;

	/** Get all available tilecache. */
	virtual std::vector<ST::string> getAllTilecache() const override;

	/** User private file (e.g. settings) */
	virtual DirFs* userPrivateFiles() const override;

	/** Save game files */
	virtual DirFs* saveGameFiles() const override;

	/** Temp files */
	virtual DirFs* tempFiles() const override;

	/* Open a game resource file for reading. */
	virtual SGPFile* openGameResForReading(ST::string filename) const override;

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(const ST::string& filename) const override;

	/** Load encrypted string from game resource file. */
	virtual ST::string loadEncryptedString(const ST::string& fileName, uint32_t seek_chars, uint32_t read_chars) const override;

	/** Load dialogue quote from file. */
	ST::string loadDialogQuoteFromFile(const ST::string& filename, unsigned quote_number) override;

	/** Load all dialogue quotes for a character. */
	void loadAllDialogQuotes(STRING_ENC_TYPE encType, const ST::string& filename, std::vector<ST::string*> &quotes) const;

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

	virtual const SmokeEffectModel* getSmokeEffect(SmokeEffectID id) const override;

	virtual const ExplosionAnimationModel* getExplosionAnimation(uint8_t id) override;
	virtual const ExplosiveModel* getExplosive(uint16_t index) override;
	virtual const ExplosiveModel* getExplosiveByName(const ST::string &name) override;

	virtual ItemRange getItems() const override;
	virtual const ItemModel* getItem(uint16_t index) const override;
	virtual const ItemModel* getItem(uint16_t itemIndex, ItemSystem::nothrow_t const&) const noexcept override;
	virtual const ItemModel* getItemByName(const ST::string &internalName) const override;
	virtual const ItemModel* getKeyItemForKeyId(uint16_t usKeyItem) const override;
	virtual std::vector<ST::string> getAllSmallInventoryGraphicPaths() const override;
	virtual const std::map<uint16_t, uint16_t>& getMapItemReplacements() const override;

	virtual const std::vector<std::vector<const WeaponModel*> > & getNormalGunChoice() const override;
	virtual const std::vector<std::vector<const WeaponModel*> > & getExtendedGunChoice() const override;
	virtual const std::vector<GARRISON_GROUP>& getGarrisonGroups() const override;
	virtual const std::vector<PATROL_GROUP>& getPatrolGroups() const override;
	virtual const std::vector<ARMY_COMPOSITION>& getArmyCompositions() const override;

	virtual const DealerInventory* getDealerInventory(ArmsDealerID dealerId) const override;
	virtual const DealerInventory* getBobbyRayNewInventory() const override;
	virtual const DealerInventory* getBobbyRayUsedInventory() const override;

	virtual const DealerModel* getDealer(ArmsDealerID dealerID) const override;
	virtual const std::vector<const DealerModel*> & getDealers() const override;

	virtual const std::vector<const ShippingDestinationModel*>& getShippingDestinations() const override;
	virtual const ShippingDestinationModel* getShippingDestination(uint8_t locationId) const override;
	virtual const ShippingDestinationModel* getPrimaryShippingDestination() const override;
	virtual const ST::string* getShippingDestinationName(uint8_t index) const override;

	virtual const NpcActionParamsModel* getNpcActionParams(uint16_t actionCode) const override;
	virtual const FactParamsModel* getFactParams(Fact fact) const override;

	virtual const ST::string* getMusicForMode(MusicMode mode) const override;

	virtual const GamePolicy* getGamePolicy() const override;
	virtual const IMPPolicy* getIMPPolicy() const override;
	virtual const StrategicAIPolicy* getStrategicAIPolicy() const override;

	virtual const ST::string* getNewString(size_t stringId) const override;
	virtual const ST::string& getLandTypeString(size_t index) const override;

	virtual const std::vector<const BloodCatPlacementsModel*>& getBloodCatPlacements() const override;
	virtual const std::vector<const BloodCatSpawnsModel*>& getBloodCatSpawns() const override;
	virtual const BloodCatSpawnsModel* getBloodCatSpawnsOfSector(uint8_t sectorId) const override;
	virtual const std::vector<const CreatureLairModel*>& getCreatureLairs() const override;
	virtual const CreatureLairModel* getCreatureLair(uint8_t lairId) const override;
	virtual const CreatureLairModel* getCreatureLairByMineId(uint8_t mineId) const override;
	virtual const MineModel* getMineForSector(const SGPSector& sector) const override;
	virtual const MineModel* getMine(uint8_t mineId) const override;
	virtual const std::vector<const MineModel*>& getMines() const override;
	virtual const std::vector<const SamSiteModel*>& getSamSites() const override;
	virtual       int8_t findSamIDBySector(uint8_t sectorId) const override;
	virtual const SamSiteModel* findSamSiteBySector(uint8_t sectorId) const override;
	virtual       int8_t getControllingSamSite(uint8_t sectorId) const override;
	virtual const TownModel* getTown(int8_t townId) const  override;
	virtual const std::map<int8_t, const TownModel*>& getTowns() const override;
	virtual const ST::string getTownName(uint8_t townId) const override;
	virtual const ST::string getTownLocative(uint8_t townId) const override;
	virtual const std::vector <const UndergroundSectorModel*>& getUndergroundSectors() const override;
	virtual const MovementCostsModel* getMovementCosts() const override;
	virtual       int16_t getSectorLandType(uint8_t sectorID, uint8_t sectorLevel) const override;
	virtual const std::vector<const StrategicMapSecretModel*>& getMapSecrets() const override;
	virtual const CacheSectorsModel* getCacheSectors() const override;

	virtual const std::map<uint8_t, const NpcPlacementModel*>& listNpcPlacements() const override;
	virtual const NpcPlacementModel* getNpcPlacement(uint8_t profileId) const override;
	virtual const RPCSmallFaceModel* getRPCSmallFaceOffsets(uint8_t profileID) const override;
	virtual const std::vector<const MERCListingModel*>& getMERCListings() const override;
	virtual const std::vector<const MercProfile*>& listMercProfiles() const override;
	virtual const VehicleModel* getVehicle(uint8_t vehicleID) const override;

	const NPCQuoteInfo* getScriptRecords(uint8_t profileId) const override;
	const NPCQuoteInfo* getScriptRecords(uint8_t profileId, uint8_t meanwhileId) const override;

	virtual const LoadingScreen* getLoadingScreenForSector(uint8_t sectorId, uint8_t sectorLevel, bool isNight) const override;
	virtual const LoadingScreen* getLoadingScreen(uint8_t index) const override;

	virtual const std::map<UINT32, UINT16>* getTranslationTable() const override;

	JsonValue readJsonDataFile(const ST::string& fileName) const;

	/* Gets the enabled mods and their version strings */
	virtual const std::vector<std::pair<ST::string, ST::string>> getEnabledMods() const override;

	/* Opens an EDT file. The columns parameter describes the layout of the data
	   inside the EDT file. */
	IEDT::uptr openEDT(std::string_view filename, IEDT::column_list columns) const override;

protected:
	RustPointer<EngineOptions> m_engineOptions;
	RustPointer<ModManager> m_modManager;
	RustPointer<SchemaManager> m_schemaManager;

	RustPointer<TempDir> m_tempDir;
	std::unique_ptr<DirFs> m_tempFiles;

	std::unique_ptr<DirFs> m_userPrivateFiles;
	std::unique_ptr<DirFs> m_saveGameFiles;

	GameVersion m_gameVersion;

	std::vector<ST::string> m_newStrings;
	std::vector<ST::string> m_landTypeStrings;

	std::vector<const ItemModel*> m_items;
	std::vector<const MagazineModel*> m_magazines;

	std::vector<const CalibreModel*> m_calibres;
	std::vector<ST::string> m_calibreNames;
	std::vector<ST::string> m_calibreNamesBobbyRay;

	std::vector<AmmoTypeModel const *> m_ammoTypes;

	std::map<std::pair<uint8_t, uint8_t>, std::unique_ptr<NPCQuoteInfo const []>> m_scriptRecordsMeanwhiles;
	                          std::vector<std::unique_ptr<NPCQuoteInfo const []>> m_scriptRecords;
							              std::unique_ptr<NPCQuoteInfo const []>  m_scriptRecordsRecruited;

	/** Mapping of calibre names to objects. */
	std::map<ST::string, const AmmoTypeModel*> m_ammoTypeMap;
	std::map<ST::string, const CalibreModel*> m_calibreMap;
	std::map<ST::string, const MagazineModel*> m_magazineMap;
	std::map<ST::string, const WeaponModel*> m_weaponMap;
	std::map<ST::string, const ExplosiveModel*> m_explosiveMap;
	std::map<ST::string, const ItemModel*> m_itemMap;
	std::map<uint16_t, uint16_t> m_mapItemReplacements;
	std::multimap<MusicMode, const ST::string> m_musicMap;

	std::vector<const SmokeEffectModel*> m_smokeEffects;
	std::vector<const ExplosionAnimationModel*> m_explosionAnimations;
	std::vector<const ExplosiveCalibreModel*> m_explosiveCalibres;

	std::vector<std::vector<const WeaponModel*> > mNormalGunChoice;
	std::vector<std::vector<const WeaponModel*> > mExtendedGunChoice;
	std::vector<GARRISON_GROUP> m_garrisonGroups;
	std::vector<PATROL_GROUP> m_patrolGroups;
	std::vector<ARMY_COMPOSITION> m_armyCompositions;

	std::vector<const DealerInventory*> m_dealersInventory;
	std::unique_ptr<DealerInventory const> m_bobbyRayNewInventory;
	std::unique_ptr<DealerInventory const> m_bobbyRayUsedInventory;

	std::vector<const DealerModel*> m_dealers;

	std::vector<const ShippingDestinationModel*> m_shippingDestinations;
	std::vector<ST::string> m_shippingDestinationNames;

	std::map<Fact, const FactParamsModel*> m_factParams;
	std::map<uint16_t, const NpcActionParamsModel*> m_npcActionParams;
	std::map<uint8_t, const NpcPlacementModel*> m_npcPlacements;

	std::unique_ptr<IMPPolicy const> m_impPolicy;
	std::unique_ptr<GamePolicy const> m_gamePolicy;
	std::unique_ptr<StrategicAIPolicy const> m_strategicAIPolicy;

	std::unique_ptr<CacheSectorsModel const> m_cacheSectors;
	std::unique_ptr<LoadingScreenModel const> m_loadingScreenModel;
	std::unique_ptr<MovementCostsModel const> m_movementCosts;
	std::map<SGPSector, uint8_t> m_sectorLandTypes;

	std::vector<const BloodCatPlacementsModel*> m_bloodCatPlacements;
	std::vector<const BloodCatSpawnsModel*> m_bloodCatSpawns;
	std::vector<const CreatureLairModel*> m_creatureLairs;
	std::vector<const MineModel*> m_mines;
	std::vector<const SamSiteModel*> m_samSites;
	std::unique_ptr<SamSiteAirControlModel const> m_samSitesAirControl;
	std::vector<const StrategicMapSecretModel*> m_mapSecrets;
	std::map<int8_t, const TownModel*> m_towns;
	std::vector<ST::string> m_townNames;
	std::vector<ST::string> m_townNameLocatives;
	std::vector<const UndergroundSectorModel*> m_undergroundSectors;

	std::map<uint8_t, const RPCSmallFaceModel*> m_rpcSmallFaces;
	std::vector<const MERCListingModel*> m_MERCListings;
	std::vector<const VehicleModel*> m_vehicles;

	// List of pre-constructed MercProfile objects; indices of elements are arbitrary (unlike gMercProfiles) and not guaranteed to follow any order
	std::vector<const MercProfile*> m_mercProfiles;
	std::map<uint8_t, const MercProfileInfo*> m_mercProfileInfo;
	std::map<UINT32, UINT16> m_translationTable;

	RustPointer<Vfs> m_vfs;

	bool loadGameData(const VanillaItemStrings& vanillaItemStrings);
	bool loadWeapons(const VanillaItemStrings& vanillaItemStrings);
	bool loadSmokeEffects();
	bool loadExplosionAnimations();
	bool loadExplosives(const VanillaItemStrings& vanillaItemStrings, const std::vector<const ExplosionAnimationModel*>& animations);
	bool loadItems(const VanillaItemStrings& vanillaItemStrings);
	bool loadMagazines(const VanillaItemStrings& vanillaItemStrings);
	bool loadExplosiveCalibres();
	bool loadCalibres();
	bool loadAmmoTypes();
	bool loadArmyData();
	bool loadMusicModeList(MusicMode mode, const JsonValue& array);
	bool loadMusic();

	const DealerInventory * loadDealerInventory(const ST::string& fileName);
	bool loadAllDealersAndInventory();
	void loadStringRes(const ST::string& name, std::vector<ST::string> &strings) const;

	void readWeaponTable(
		const ST::string& fileName,
		std::vector<std::vector<const WeaponModel*> > & weaponTable);

	bool loadStrategicLayerData();
	bool loadTacticalLayerData();
	bool loadMercsData();
	void loadVehicles();
	void loadTranslationTable();
	void loadAllScriptRecords();

	JsonValue readJsonDataFileWithSchema(const ST::string& jsonPath) const;


	/**
	 * @param profileID
	 * @return pointer to a MercProfileInfo. Never returns null. A pointer to the empty instance is returned if not defined in JSON.
	 */
	const MercProfileInfo* getMercProfileInfo(uint8_t profileID) const override;
	/**
	 * @param profile
	 * @return pointer to a MercProfileInfo. Returns null if not defined in JSON
	 */
	const MercProfileInfo* getMercProfileInfoByName(const ST::string& profile) const override;
};
