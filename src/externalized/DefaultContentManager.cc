#include "DefaultContentManager.h"

#include "Exceptions.h"
#include "game/Directories.h"
#include "game/Strategic/Strategic_Status.h"

// XXX: GameRes.h should be integrated to ContentManager
#include "game/GameRes.h"

#include "sgp/FileMan.h"

#include "AmmoTypeModel.h"
#include "CacheSectorsModel.h"
#include "CalibreModel.h"
#include "ContentMusic.h"
#include "DealerInventory.h"
#include "DealerModel.h"
#include "JsonUtility.h"
#include "LoadingScreenModel.h"
#include "MagazineModel.h"
#include "RustInterface.h"
#include "ShippingDestinationModel.h"
#include "VehicleModel.h"
#include "WeaponModels.h"
#include "army/ArmyCompositionModel.h"
#include "army/GarrisonGroupModel.h"
#include "army/PatrolGroupModel.h"
#include "mercs/MERCListingModel.h"
#include "MercProfile.h"
#include "mercs/MercProfileInfo.h"
#include "mercs/RPCSmallFaceModel.h"
#include "policy/DefaultGamePolicy.h"
#include "policy/DefaultIMPPolicy.h"
#include "policy/DefaultStrategicAIPolicy.h"
#include "strategic/BloodCatPlacementsModel.h"
#include "strategic/BloodCatSpawnsModel.h"
#include "strategic/CreatureLairModel.h"
#include "strategic/FactParamsModel.h"
#include "strategic/MineModel.h"
#include "strategic/SamSiteModel.h"
#include "strategic/SamSiteAirControlModel.h"
#include "strategic/SectorLandTypes.h"
#include "strategic/StrategicMapSecretModel.h"
#include "strategic/TownModel.h"
#include "strategic/TraversibilityMapping.h"
#include "strategic/MovementCostsModel.h"
#include "strategic/NpcPlacementModel.h"
#include "strategic/UndergroundSectorModel.h"
#include "strings/EncryptedString.h"
#include "strings/Localization.h"
#include "tactical/MapItemReplacementModel.h"
#include "tactical/NpcActionParamsModel.h"

#include "Logger.h"
#include "Strategic_AI.h"

#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>
#include <utility>

#define BASEDATADIR    "data"

#define MAPSDIR        "maps"
#define RADARMAPSDIR   "radarmaps"
#define TILESETSDIR    "tilesets"

#define DIALOGUESIZE 240

const MercProfileInfo EMPTY_MERC_PROFILE_INFO;

DefaultContentManager::DefaultContentManager(RustPointer<EngineOptions> engineOptions)
	:m_schemaManager(SchemaManager_create()),
	mNormalGunChoice(ARMY_GUN_LEVELS),
	mExtendedGunChoice(ARMY_GUN_LEVELS),
	m_vfs(Vfs_create())
{
	m_engineOptions = std::move(engineOptions);
	m_modManager.reset(ModManager_create(m_engineOptions.get()));
	if (m_modManager.get() == NULL) {
		RustPointer<char> err{ getRustError() };
		throw std::runtime_error(err.get());
	}

	RustPointer<char> vanillaGameDir{EngineOptions_getVanillaGameDir(m_engineOptions.get())};

	RustPointer<char> stracciatellaHome{EngineOptions_getStracciatellaHome()};
	RustPointer<char> saveGameDir{EngineOptions_getSaveGameDir(m_engineOptions.get())};

	m_userPrivateFiles = std::make_unique<DirFs>(stracciatellaHome.get());
	m_saveGameFiles = std::make_unique<DirFs>(saveGameDir.get());

	m_gameVersion = EngineOptions_getResourceVersion(m_engineOptions.get());

	// Initialize temp dir
	RustPointer<TempDir> tempDir(TempDir_create());
	if (tempDir.get() == NULL) {
		RustPointer<char> err{ getRustError() };
		auto error = ST::format("Failed to create temporary directory: {}", err.get());
		throw std::runtime_error(error.c_str());
	}
	m_tempDir = std::move(tempDir);
	RustPointer<char> tempDirPath(TempDir_path(m_tempDir.get()));
	m_tempFiles = std::make_unique<DirFs>(tempDirPath.get());

	// Initialize VFS
	auto succeeded = Vfs_init(m_vfs.get(), m_engineOptions.get(), m_modManager.get());
	if (!succeeded) {
		RustPointer<char> err{ getRustError() };
		auto error = ST::format("Failed to build virtual file system (VFS): {}", err.get());
		throw std::runtime_error(error.c_str());
	}
}

void DefaultContentManager::logConfiguration() const {
	RustPointer<char> vanillaGameDir{EngineOptions_getVanillaGameDir(m_engineOptions.get())};
	RustPointer<char> assetsDir{EngineOptions_getAssetsDir(m_engineOptions.get())};

	SLOGI("JA2 Home Dir:                  '{}'", m_userPrivateFiles.get()->basePath());
	SLOGI("Root game resources directory: '{}'", vanillaGameDir.get());
	SLOGI("Extra data directory:          '{}'", assetsDir.get());
	SLOGI("Saved games directory:         '{}'", m_saveGameFiles.get()->basePath());
	SLOGI("Temporary directory:           '{}'", m_tempFiles.get()->basePath());
}

template <class T>
void deleteElements(std::vector<const T*> & vec)
{
	for (auto elem : vec)
	{
		delete elem;
	}
}

template <typename K, typename V>
void deleteElements(std::map<K, const V*> & map)
{
	for (auto& kv : map)
	{
		delete kv.second;
	}
}

DefaultContentManager::~DefaultContentManager()
{
	// Deconstruction of vectors containing non-pointer types
	// is left to the compiler, no need to clear() them.

	deleteElements(m_items);
	deleteElements(m_calibres);
	deleteElements(m_ammoTypes);
	deleteElements(m_dealersInventory);
	deleteElements(m_dealers);
	deleteElements(m_bloodCatPlacements);
	deleteElements(m_bloodCatSpawns);
	deleteElements(m_creatureLairs);
	deleteElements(m_factParams);
	deleteElements(m_mines);
	deleteElements(m_npcActionParams);
	deleteElements(m_npcPlacements);
	deleteElements(m_samSites);
	deleteElements(m_mapSecrets);
	deleteElements(m_shippingDestinations);
	deleteElements(m_towns);
	deleteElements(m_undergroundSectors);
	deleteElements(m_rpcSmallFaces);
	deleteElements(m_MERCListings);
	deleteElements(m_mercProfileInfo);
	deleteElements(m_mercProfiles);
	deleteElements(m_vehicles);
}

const DealerInventory* DefaultContentManager::getBobbyRayNewInventory() const
{
	return m_bobbyRayNewInventory.get();
}

const DealerInventory* DefaultContentManager::getBobbyRayUsedInventory() const
{
	return m_bobbyRayUsedInventory.get();
}

const DealerModel* DefaultContentManager::getDealer(ArmsDealerID dealerID) const
{
	auto dealerIndex = static_cast<std::size_t>(dealerID);
	if (dealerIndex >= m_dealers.size()) {
		ST::string err = ST::format("failed to get dealer: invalid dealer ID: {}", dealerID);
		throw std::runtime_error(err.to_std_string());
	}
	return m_dealers[dealerID];
}

const std::vector<const DealerModel*>& DefaultContentManager::getDealers() const
{
	return m_dealers;
}

const std::vector<const ShippingDestinationModel*>& DefaultContentManager::getShippingDestinations() const
{
	return m_shippingDestinations;
}

const ShippingDestinationModel* DefaultContentManager::getShippingDestination(uint8_t locationId) const
{
	return m_shippingDestinations[locationId];
}

const ShippingDestinationModel* DefaultContentManager::getPrimaryShippingDestination() const
{
	for (auto dest : m_shippingDestinations)
	{
		if (dest->isPrimary)
		{
			return dest;
		}
	}
	throw DataError("Bobby Ray primary destination is not defined");
}

const ST::string* DefaultContentManager::getShippingDestinationName(uint8_t index) const
{
	return &m_shippingDestinationNames[index];
}

const NpcActionParamsModel* DefaultContentManager::getNpcActionParams(uint16_t actionCode) const
{
	auto it = m_npcActionParams.find(actionCode);
	if (it != m_npcActionParams.end())
	{
		return it->second;
	}
	return &NpcActionParamsModel::empty;
}

const FactParamsModel* DefaultContentManager::getFactParams(Fact fact) const
{
	auto it = m_factParams.find(fact);
	if (it != m_factParams.end())
	{
		return it->second;
	}
	return &FactParamsModel::empty;
}

/** Get map file path. */
ST::string DefaultContentManager::getMapPath(const ST::string& mapName) const
{
	ST::string result = MAPSDIR "/" + mapName;

	SLOGD("map file {}", result);

	return result;
}

/** Get radar map resource name. */
ST::string DefaultContentManager::getRadarMapResourceName(const ST::string &mapName) const
{
	ST::string result = RADARMAPSDIR "/" + mapName;

	SLOGD("map file {}", result);

	return result;
}

/** Get tileset resource name. */
ST::string DefaultContentManager::getTilesetResourceName(int number, const ST::string& fileName) const
{
	return ST::format("{}/{}/{}", TILESETSDIR, number, fileName);
}


/** Get tileset db resource name. */
ST::string DefaultContentManager::getTilesetDBResName() const
{
	return BINARYDATADIR "/ja2set.dat";
}

/** Open map for reading. */
SGPFile* DefaultContentManager::openMapForReading(const ST::string& mapName) const
{
	return openGameResForReading(getMapPath(mapName));
}

/** Get all available tilecache. */
std::vector<ST::string> DefaultContentManager::getAllTilecache() const
{
	RustPointer<VecCString> vec(Vfs_readDir(m_vfs.get(), TILECACHEDIR, "jsd"));
	if (vec.get() == NULL) {
		throw std::runtime_error(ST::format("DefaultContentManager::getAllTilecache: {}", getRustError()).c_str());
	}
	auto len = VecCString_len(vec.get());
	std::vector<ST::string> paths;
	for (size_t i = 0; i < len; i++)
	{
		RustPointer<char> path{VecCString_get(vec.get(), i)};
		paths.emplace_back(FileMan::joinPaths(TILECACHEDIR, path.get()));
	}
	return paths;
}

/* Open a game resource file for reading.
 *
 * First trying to open the file normally. It will work if the path is absolute
 * and the file is found or path is relative to the current directory (game
 * settings directory) and file is present.
 * If file is not found, try to find it relatively to 'Data' directory.
 * If file is not found, try to find the file in libraries located in 'Data' directory; */
SGPFile* DefaultContentManager::openGameResForReading(ST::string filename) const
{
	RustPointer<VFile> vfile(VfsFile_open(m_vfs.get(), filename.c_str()));
	if (!vfile)
	{
		RustPointer<char> err{getRustError()};
		throw std::runtime_error(ST::format("openGameResForReading: {}", err.get()).to_std_string());
	}
	SLOGD("Opened resource file from VFS: '{}'", filename);
	return new SGPFile(vfile.release(), std::move(filename));
}

/* Checks if a game resource exists. */
bool DefaultContentManager::doesGameResExists(const ST::string& filename) const
{
	RustPointer<VFile> vfile(VfsFile_open(m_vfs.get(), filename.c_str()));
	return static_cast<bool>(vfile.get());
}

DirFs *DefaultContentManager::tempFiles() const
{
	return m_tempFiles.get();
}

DirFs *DefaultContentManager::userPrivateFiles() const
{
	return m_userPrivateFiles.get();
}

DirFs *DefaultContentManager::saveGameFiles() const
{
	return m_saveGameFiles.get();
}

/** Load encrypted string from game resource file. */
ST::string DefaultContentManager::loadEncryptedString(const ST::string& fileName, uint32_t seek_chars, uint32_t read_chars) const
{
	AutoSGPFile file(openGameResForReading(fileName));
	return LoadEncryptedString(file, seek_chars, read_chars);
}

/** Load dialogue quote from file. */
ST::string DefaultContentManager::loadDialogQuoteFromFile(const ST::string& fileName, unsigned quote_number)
{
	// Using the qualified name because we do not want a virtual function call here.
	return DefaultContentManager::openEDT(fileName.view(), { DIALOGUESIZE })->at(quote_number);
}

#if 0
/* This function is only used when someone wants to extract all quotes to JSON,
   see the commented out code in SGP.cc */
/** Load all dialogue quotes for a character. */
void DefaultContentManager::loadAllDialogQuotes(STRING_ENC_TYPE encType, const ST::string& fileName, std::vector<ST::string*> &quotes) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	uint32_t fileSize = File->size();
	uint32_t numQuotes = fileSize / DIALOGUESIZE / 2;

	for(uint32_t i = 0; i < numQuotes; i++)
	{
		ST::string err;
		ST::string quote = LoadEncryptedData(err, encType, File, i * DIALOGUESIZE, DIALOGUESIZE);
		if (!err.empty())
		{
			SLOGW("DefaultContentManager::loadAllDialogQuotes '{}' {}: {}", fileName, i, err);
		}
		quotes.push_back(new ST::string{std::move(quote)});
	}
}
#endif

/** Get weapons with the give index. */
const WeaponModel* DefaultContentManager::getWeapon(uint16_t itemIndex)
{
	return getItem(itemIndex)->asWeapon();
}

const WeaponModel* DefaultContentManager::getWeaponByName(const ST::string &internalName)
{
	std::map<ST::string, const WeaponModel*>::const_iterator it = m_weaponMap.find(internalName);
	if(it == m_weaponMap.end())
	{
		SLOGE("weapon '{}' is not found", internalName);
		throw std::runtime_error(ST::format("weapon '{}' is not found", internalName).to_std_string());
	}
	return it->second;//m_weaponMap[internalName];
}

const MagazineModel* DefaultContentManager::getMagazineByName(const ST::string &internalName)
{
	if(m_magazineMap.find(internalName) == m_magazineMap.end())
	{
		SLOGE("magazine '{}' is not found", internalName);
		throw std::runtime_error(ST::format("magazine '{}' is not found", internalName).to_std_string());
	}
	return m_magazineMap[internalName];
}

const MagazineModel* DefaultContentManager::getMagazineByItemIndex(uint16_t itemIndex)
{
	return getItem(itemIndex)->asAmmo();
}

const std::vector<const MagazineModel*>& DefaultContentManager::getMagazines() const
{
	return m_magazines;
}

const CalibreModel* DefaultContentManager::getCalibre(uint8_t index)
{
	return m_calibres[index];
}

const ST::string* DefaultContentManager::getCalibreName(uint8_t index) const
{
	return &m_calibreNames[index];
}

const ST::string* DefaultContentManager::getCalibreNameForBobbyRay(uint8_t index) const
{
	return &m_calibreNamesBobbyRay[index];
}

const AmmoTypeModel* DefaultContentManager::getAmmoType(uint8_t index)
{
	return m_ammoTypes[index];
}

bool DefaultContentManager::loadWeapons(const VanillaItemStrings& vanillaItemStrings)
{
	auto json = readJsonDataFileWithSchema("weapons.json");
	for (auto& element : json.toVec()) {
		WeaponModel *w = WeaponModel::deserialize(element, m_calibreMap, vanillaItemStrings);
		SLOGD("Loaded weapon {} {}", w->getItemIndex(), w->getInternalName());

		if (w->getItemIndex() > MAX_WEAPONS)
		{
			SLOGE("Weapon index must be in the interval 0 - {}", MAX_WEAPONS);
			return false;
		}

		m_items[w->getItemIndex()] = w;
		m_weaponMap.insert(std::make_pair(w->getInternalName(), w));
	}

	return true;
}

bool DefaultContentManager::loadItems(const VanillaItemStrings& vanillaItemStrings)
{
	auto json = readJsonDataFileWithSchema("items.json");
	for (auto& el : json.toVec())
	{
		auto* item = ItemModel::deserialize(el, vanillaItemStrings);
		if (item->getItemIndex() <= MAX_WEAPONS || item->getItemIndex() > MAXITEMS)
		{
			ST::string err = ST::format("Item index must be in the interval {} - {}", MAX_WEAPONS+1, MAXITEMS);
			throw DataError(err);
		}

		m_items[item->getItemIndex()] = item;
	}

	return true;
}

bool DefaultContentManager::loadMagazines(const VanillaItemStrings& vanillaItemStrings)
{
	auto json = readJsonDataFileWithSchema("magazines.json");
	for (auto& element : json.toVec())
	{
		MagazineModel *mag = MagazineModel::deserialize(element, m_calibreMap, m_ammoTypeMap, vanillaItemStrings);
		SLOGD("Loaded magazine {} {}", mag->getItemIndex(), mag->getInternalName());

		m_magazines.push_back(mag);
		m_items[mag->getItemIndex()] = mag;
		m_magazineMap.insert(std::make_pair(mag->getInternalName(), mag));
	}

	return true;
}

bool DefaultContentManager::loadCalibres()
{
	auto json = readJsonDataFileWithSchema("calibres.json");
	for (auto& element : json.toVec()) {
		CalibreModel *calibre = CalibreModel::deserialize(element);
		SLOGD("Loaded calibre {} {}", calibre->index, calibre->internalName);

		if(m_calibres.size() <= calibre->index)
		{
			m_calibres.resize(calibre->index + 1);
		}

		m_calibres[calibre->index] = calibre;
	}
	for (const CalibreModel* calibre : m_calibres)
	{
		m_calibreMap.emplace(calibre->internalName, calibre);
	}

	return true;
}

bool DefaultContentManager::loadAmmoTypes()
{
	auto json = readJsonDataFileWithSchema("ammo-types.json");
	for (auto& element : json.toVec()) {
		AmmoTypeModel *ammoType = AmmoTypeModel::deserialize(element);
		SLOGD("Loaded ammo type {} {}", ammoType->index, ammoType->internalName);

		if(m_ammoTypes.size() <= ammoType->index)
		{
			m_ammoTypes.resize(ammoType->index + 1);
		}

		m_ammoTypes[ammoType->index] = ammoType;
	}

	for (const AmmoTypeModel* ammoType : m_ammoTypes)
	{
		m_ammoTypeMap.emplace(ammoType->internalName, ammoType);
	}

	return true;
}

bool DefaultContentManager::loadMusicModeList(const MusicMode mode, const JsonValue& array)
{
	std::vector<ST::string> utf8_encoded;
	JsonUtility::parseListStrings(array, utf8_encoded);
	for (const ST::string &str : utf8_encoded)
	{
		m_musicMap.insert(std::make_pair(mode, str));
		SLOGD("Loaded music {}", str);
	}

	return true;
}

bool DefaultContentManager::loadMusic()
{
	auto document = readJsonDataFileWithSchema("music.json");
	auto obj = document.toObject();

	SLOGD("Loading main_menu music");
	loadMusicModeList(MUSIC_MAIN_MENU, obj["main_menu"]);
	SLOGD("Loading main_menu music");
	loadMusicModeList(MUSIC_LAPTOP, obj["laptop"]);
	SLOGD("Loading tactical music");
	loadMusicModeList(MUSIC_TACTICAL_NOTHING, obj["tactical"]);
	SLOGD("Loading tactical_enemypresent music");
	loadMusicModeList(MUSIC_TACTICAL_ENEMYPRESENT, obj["tactical_enemypresent"]);
	SLOGD("Loading tactical_battle music");
	loadMusicModeList(MUSIC_TACTICAL_BATTLE, obj["tactical_battle"]);
	SLOGD("Loading tactical_creature music");
	loadMusicModeList(MUSIC_TACTICAL_CREATURE_NOTHING, obj["tactical_creature"]);
	SLOGD("Loading tactical_creature_enemypresent music");
	loadMusicModeList(MUSIC_TACTICAL_CREATURE_ENEMYPRESENT, obj["tactical_creature_enemypresent"]);
	SLOGD("Loading tactical_creature_battle music");
	loadMusicModeList(MUSIC_TACTICAL_CREATURE_BATTLE, obj["tactical_creature_battle"]);
	SLOGD("Loading tactical_victory music");
	loadMusicModeList(MUSIC_TACTICAL_VICTORY, obj["tactical_victory"]);
	SLOGD("Loading tactical_defeat music");
	loadMusicModeList(MUSIC_TACTICAL_DEFEAT, obj["tactical_defeat"]);

	return true;
}

void DefaultContentManager::readWeaponTable(
	const ST::string& fileName,
	std::vector<std::vector<const WeaponModel*> > & weaponTable)
{
	auto document = readJsonDataFileWithSchema(fileName);

	size_t i = 0;
	for (auto &element : document.toVec()) {
		std::vector<ST::string> weaponNames;
		JsonUtility::parseListStrings(element, weaponNames);
		for (const ST::string &weapon : weaponNames)
		{
			weaponTable[i].push_back(getWeaponByName(weapon));
		}
		++i;
	}
}

const std::vector<std::vector<const WeaponModel*> > & DefaultContentManager::getNormalGunChoice() const
{
	return mNormalGunChoice;
}

const std::vector<std::vector<const WeaponModel*> > & DefaultContentManager::getExtendedGunChoice() const
{
	return mExtendedGunChoice;
}

const std::vector<GARRISON_GROUP>& DefaultContentManager::getGarrisonGroups() const
{
	return m_garrisonGroups;
}

const std::vector<PATROL_GROUP>& DefaultContentManager::getPatrolGroups() const
{
	return m_patrolGroups;
}

const std::vector<ARMY_COMPOSITION>& DefaultContentManager::getArmyCompositions() const
{
	return m_armyCompositions;
}

bool DefaultContentManager::loadArmyData()
{
	readWeaponTable("army-gun-choice-normal.json", mNormalGunChoice);
	readWeaponTable("army-gun-choice-extended.json", mExtendedGunChoice);

	auto jsonAC = readJsonDataFileWithSchema("army-compositions.json");
	auto armyCompModels = ArmyCompositionModel::deserialize(jsonAC);
	ArmyCompositionModel::validateData(armyCompModels);

	std::map<ST::string, uint8_t> mapping;
	for (auto& armyComp : armyCompModels)
	{
		mapping[armyComp->name] = armyComp->compositionId;
		m_armyCompositions.push_back(armyComp->toArmyComposition());
	}
	deleteElements(armyCompModels);
	armyCompModels.clear();

	auto jsonGG = readJsonDataFileWithSchema("army-garrison-groups.json");
	for (auto& element : jsonGG.toVec())
	{
		m_garrisonGroups.push_back(
			GarrisonGroupModel::deserialize(element, mapping)
		);
	}
	GarrisonGroupModel::validateData(m_garrisonGroups);

	auto jsonPG = readJsonDataFileWithSchema("army-patrol-groups.json");

	for (auto& element : jsonPG.toVec())
	{
		m_patrolGroups.push_back(
			PatrolGroupModel::deserialize(element)
		);
	}
	PatrolGroupModel::validateData(m_patrolGroups);

	return true;
}

void DefaultContentManager::loadStringRes(const ST::string& name, std::vector<ST::string> &strings) const
{
	ST::string const fullName = name + L10n::GetSuffix(m_gameVersion, true) + ".json";

	auto json = readJsonDataFileWithSchema(fullName);
	std::vector<ST::string> utf8_encoded;
	JsonUtility::parseListStrings(json, utf8_encoded);
	for (const ST::string &str : utf8_encoded)
	{
		strings.push_back(str);
	}
}

/** Load the game data. */
bool DefaultContentManager::loadGameData()
{
	VanillaItemStrings vanillaItemStrings = {};
	try {
		AutoSGPFile f(openGameResForReading(VanillaItemStrings::filename()));
		vanillaItemStrings = VanillaItemStrings::deserialize(f);
	} catch (const std::runtime_error& err) {
		SLOGE("Could not read vanilla item strings from {}: {}", VanillaItemStrings::filename(), err.what());
	}

	m_items.resize(MAXITEMS);
	bool result = loadItems(vanillaItemStrings)
		&& loadCalibres()
		&& loadAmmoTypes()
		&& loadMagazines(vanillaItemStrings)
		&& loadWeapons(vanillaItemStrings)
		&& loadArmyData()
		&& loadMusic();

	for (const ItemModel *item : m_items)
	{
		m_itemMap.insert(std::make_pair(item->getInternalName(), item));
	}

	auto replacement_json = readJsonDataFileWithSchema("tactical-map-item-replacements.json");

	m_mapItemReplacements = MapItemReplacementModel::deserialize(replacement_json, this);

	loadMercsData();
	loadAllDealersAndInventory();

	auto game_json = readJsonDataFileWithSchema("game.json");

	m_gamePolicy = std::make_unique<DefaultGamePolicy>(game_json);

	auto imp_json = readJsonDataFileWithSchema("imp.json");
	m_impPolicy = std::make_unique<DefaultIMPPolicy>(imp_json, this);

	auto sai_json = readJsonDataFileWithSchema("strategic-ai-policy.json");
	m_strategicAIPolicy = std::make_unique<DefaultStrategicAIPolicy>(sai_json);

	loadStringRes("strings/shipping-destinations", m_shippingDestinationNames);

	auto shippingDestJson = readJsonDataFileWithSchema("shipping-destinations.json");
	for (auto& element : shippingDestJson.toVec())
	{
		m_shippingDestinations.push_back(ShippingDestinationModel::deserialize(element));
	}
	ShippingDestinationModel::validateData(m_shippingDestinations, m_shippingDestinationNames);

	auto loadScreensList = readJsonDataFileWithSchema("loading-screens.json");
	auto loadScreensMapping = readJsonDataFileWithSchema("loading-screens-mapping.json");

	m_loadingScreenModel.reset(LoadingScreenModel::deserialize(loadScreensList, loadScreensMapping));
	m_loadingScreenModel->validateData(this);

	loadStringRes("strings/ammo-calibre", m_calibreNames);
	loadStringRes("strings/ammo-calibre-bobbyray", m_calibreNamesBobbyRay);

	loadStringRes("strings/new-strings", m_newStrings);
	loadStringRes("strings/strategic-map-land-types", m_landTypeStrings);

	loadStrategicLayerData();
	loadTacticalLayerData();
	loadVehicles();

	loadTranslationTable();

	std::unique_ptr<SGPFile> const translation { openGameResForReading(ST::format(
		"strings/translation{}.json", L10n::GetSuffix(m_gameVersion, false)))};
	g_langRes = std::make_unique<L10n::L10n_t>(translation.get());

	return result;
}

JsonValue DefaultContentManager::readJsonFromString(const ST::string& jsonData, const ST::string& label) const
{
	auto r = JsonValue::deserialize(jsonData);
	return r;
}

JsonValue DefaultContentManager::readJsonDataFile(const ST::string& fileName) const
{
	AutoSGPFile f(openGameResForReading(fileName));
	ST::string jsonData = f->readStringToEnd();

	JsonValue v(0);
	try {
		v = readJsonFromString(jsonData, fileName);
	} catch (const std::runtime_error &ex) {
		throw std::runtime_error(ST::format("failed to read json file {}: {}", fileName, ex.what()).c_str());
	}

	return v;
}

JsonValue DefaultContentManager::readJsonDataFileWithSchema(const ST::string& jsonPath) const
{
	auto value = readJsonDataFile(jsonPath);
	RustPointer<VecCString> errors(SchemaManager_validateValueForPath(m_schemaManager.get(), jsonPath.c_str(), value.get()));
	if (errors) {
		auto numErrors = VecCString_len(errors.get());
		for (uintptr_t i = 0; i < numErrors; i++) {
			RustPointer<char> error(VecCString_get(errors.get(), i));
			SLOGE("{}", error.get());
		}
		throw DataError(ST::format("JSON schema validation error(s) occurred when validating JSON file `{}`", jsonPath));
	}
	return value;
}

const DealerInventory * DefaultContentManager::loadDealerInventory(const ST::string& fileName)
{
	return new DealerInventory(readJsonDataFileWithSchema(fileName), this);
}

bool DefaultContentManager::loadAllDealersAndInventory()
{
	auto json = readJsonDataFileWithSchema("dealers.json");
	int index = 0;
	for (auto& element : json.toVec())
	{
		m_dealers.push_back(DealerModel::deserialize(element, this, index++));
	}
	DealerModel::validateData(m_dealers, this);

	m_dealersInventory = std::vector<const DealerInventory*>(m_dealers.size());
	for (auto dealer : m_dealers)
	{
		ST::string filename = dealer->getInventoryDataFileName(this);
		m_dealersInventory[dealer->dealerID] = loadDealerInventory(filename);
	}
	m_bobbyRayNewInventory .reset(loadDealerInventory("bobby-ray-inventory-new.json"));
	m_bobbyRayUsedInventory.reset(loadDealerInventory("bobby-ray-inventory-used.json"));
	return true;
}

ItemRange DefaultContentManager::getItems() const
{
	return ItemRange(m_items.begin() + 1, m_items.end());
}

const ItemModel* DefaultContentManager::getItem(uint16_t itemIndex) const
{
	if(itemIndex >= m_items.size())
	{
		return nullptr;
	}
	return m_items[itemIndex];
}

const ItemModel* DefaultContentManager::getItemByName(const ST::string &internalName) const
{
	std::map<ST::string, const ItemModel*>::const_iterator it = m_itemMap.find(internalName);
	if(it == m_itemMap.end())
	{
		SLOGE("item '{}' is not found", internalName);
		throw std::runtime_error(ST::format("item '{}' is not found", internalName).to_std_string());
	}
	return it->second;
}

const ItemModel* DefaultContentManager::getKeyItemForKeyId(uint16_t usKeyItem) const
{
	for (auto item : m_items) {
		if (item->getItemClass() == IC_KEY && item->getClassIndex() == usKeyItem) {
			return item;
		}
	}
	return NULL;
}

std::vector<ST::string> DefaultContentManager::getAllSmallInventoryGraphicPaths() const
{
	std::vector<ST::string> v = {};

	for (auto item : m_items) {
		auto& path = item->getInventoryGraphicSmall().getPath();
		auto existing = std::find(v.begin(), v.end(), path);
		if (existing == v.end()) {
			v.push_back(path);
		}
	}

	return v;
}

const std::map<uint16_t, uint16_t>& DefaultContentManager::getMapItemReplacements() const
{
	return m_mapItemReplacements;
}

const DealerInventory* DefaultContentManager::getDealerInventory(ArmsDealerID dealerId) const
{
	return m_dealersInventory[dealerId];
}

const ST::string* DefaultContentManager::getMusicForMode(MusicMode mode) const {
	uint32_t const count = static_cast<uint32_t>(m_musicMap.count(mode));
	uint32_t const index = Random(count);
	auto lower = m_musicMap.lower_bound(mode);
	std::advance(lower, index);

	SLOGD("Choosing music index {} of {} : '{}'", index, count, lower->second);
	return &lower->second;
}

const IMPPolicy* DefaultContentManager::getIMPPolicy() const
{
	return m_impPolicy.get();
}

const GamePolicy* DefaultContentManager::getGamePolicy() const
{
	return m_gamePolicy.get();
}

const StrategicAIPolicy* DefaultContentManager::getStrategicAIPolicy() const
{
	return m_strategicAIPolicy.get();
}

const ST::string* DefaultContentManager::getNewString(size_t stringId) const
{
	if(stringId >= m_newStrings.size())
	{
		ST::string message = ST::format("new string {} is not found", stringId);
		SLOGE("{}", message);
		throw std::runtime_error(message.c_str());
	}
	else
	{
		return &m_newStrings[stringId];
	}
}

const ST::string& DefaultContentManager::getLandTypeString(size_t index) const
{
	return m_landTypeStrings.at(index);
}

bool DefaultContentManager::loadStrategicLayerData()
{
	auto json = readJsonDataFileWithSchema("strategic-bloodcat-placements.json");
	for (auto& element : json.toVec()) {
		m_bloodCatPlacements.push_back(
			BloodCatPlacementsModel::deserialize(element)
		);
	}

	json = readJsonDataFileWithSchema("strategic-bloodcat-spawns.json");
	for (auto& element : json.toVec())
	{
		m_bloodCatSpawns.push_back(
			BloodCatSpawnsModel::deserialize(element)
		);
	}

	json = readJsonDataFileWithSchema("strategic-map-creature-lairs.json");
	for (auto& element : json.toVec())
	{
		m_creatureLairs.push_back(
			CreatureLairModel::deserialize(element)
		);
	}

	json = readJsonDataFileWithSchema("strategic-fact-params.json");
	for (auto& element : json.toVec())
	{
		auto params = FactParamsModel::deserialize(element);
		m_factParams[params->fact] = params;
	}

	json = readJsonDataFileWithSchema("strategic-mines.json");

	uint8_t i = 0;
	for (auto& element : json.toVec())
	{
		m_mines.push_back(
			MineModel::deserialize(i, element)
		);
		i++;
	}
	MineModel::validateData(m_mines);

	json = readJsonDataFileWithSchema("strategic-map-sam-sites.json");
	for (auto& element : json.toVec())
	{
		auto samSite = SamSiteModel::deserialize(element);
		m_samSites.push_back(samSite);
	}
	SamSiteModel::validateData(m_samSites);

	json = readJsonDataFileWithSchema("strategic-map-sam-sites-air-control.json");
	m_samSitesAirControl.reset(SamSiteAirControlModel::deserialize(json));
	SamSiteAirControlModel::validateData(m_samSitesAirControl.get(), m_samSites.size());

	json = readJsonDataFileWithSchema("strategic-map-towns.json");
	for (auto& element : json.toVec())
	{
		auto town = TownModel::deserialize(element);
		m_towns.insert(std::make_pair(town->townId, town));
	}

	loadStringRes("strings/strategic-map-town-names", m_townNames);
	loadStringRes("strings/strategic-map-town-name-locatives", m_townNameLocatives);

	json = readJsonDataFileWithSchema("strategic-map-underground-sectors.json");
	for (auto& element : json.toVec())
	{
		auto ugSector = UndergroundSectorModel::deserialize(element);
		m_undergroundSectors.push_back(ugSector);
	}
	UndergroundSectorModel::validateData(m_undergroundSectors);

	json = readJsonDataFileWithSchema("strategic-map-traversibility-ratings.json");
	auto travRatingMap = TraversibilityMapping::deserialize(json);

	json = readJsonDataFileWithSchema("strategic-map-movement-costs.json");
	m_movementCosts.reset(MovementCostsModel::deserialize(json, travRatingMap));

	json = readJsonDataFileWithSchema("strategic-map-sectors-descriptions.json");
	m_sectorLandTypes = SectorLandTypes::deserialize(json, travRatingMap);

	json = readJsonDataFileWithSchema("strategic-map-secrets.json");
	for (auto& element : json.toVec())
	{
		auto secret = StrategicMapSecretModel::deserialize(element, travRatingMap);
		m_mapSecrets.push_back(secret);
	}
	StrategicMapSecretModel::validateData(m_mapSecrets, m_samSites);

	json = readJsonDataFileWithSchema("strategic-map-npc-placements.json");
	for (auto& element : json.toVec())
	{
		auto placement = NpcPlacementModel::deserialize(element, this);
		m_npcPlacements.insert(std::make_pair(placement->profileId, placement));
	}

	CreatureLairModel::validateData(m_creatureLairs, m_undergroundSectors, m_mines.size());

	json = readJsonDataFileWithSchema("strategic-map-cache-sectors.json");
	m_cacheSectors.reset(CacheSectorsModel::deserialize(json));

	return true;
}

bool DefaultContentManager::loadTacticalLayerData()
{
	auto json = readJsonDataFileWithSchema("tactical-npc-action-params.json");
	for (auto& element : json.toVec())
	{
		auto params = NpcActionParamsModel::deserialize(element);
		m_npcActionParams[params->actionCode] = params;
	}

	return true;
}

bool DefaultContentManager::loadMercsData()
{
	MercProfileInfo::load = [this](uint8_t p) { return this->getMercProfileInfo(p); };
	auto json = readJsonDataFileWithSchema("mercs-profile-info.json");
	for (auto& element : json.toVec())
	{
		auto profileInfo = MercProfileInfo::deserialize(element);
		ProfileID profileID = profileInfo->profileID;
		m_mercProfileInfo[profileID] = profileInfo;
		m_mercProfiles.push_back(new MercProfile(profileID));
	}
	MercProfileInfo::validateData(m_mercProfileInfo);

	json = readJsonDataFileWithSchema("mercs-rpc-small-faces.json");

	for (auto& element : json.toVec())
	{
		auto face = RPCSmallFaceModel::deserialize(element, this);
		m_rpcSmallFaces[face->ubProfileID] = face;
	}


	json = readJsonDataFileWithSchema("mercs-MERC-listings.json");
	int i = 0;
	for (auto& element : json.toVec())
	{
		auto item = MERCListingModel::deserialize(i++, element, this);
		m_MERCListings.push_back(item);
	}
	MERCListingModel::validateData(m_MERCListings);

	return true;
}

void DefaultContentManager::loadVehicles()
{
	auto json = readJsonDataFileWithSchema("vehicles.json");
	for (auto& element : json.toVec())
	{
		auto vehicleTypeInfo = VehicleModel::deserialize(element, this, this);
		m_vehicles.push_back(vehicleTypeInfo);
	}
	VehicleModel::validateData(m_vehicles);
}

void DefaultContentManager::loadTranslationTable()
{
	m_translationTable.clear();
	ST::string name = "translation_tables/translation-table";
	ST::string suffix;
	switch (m_gameVersion)
	{
	case GameVersion::FRENCH:
		suffix = "fr";
		break;

	case GameVersion::RUSSIAN:
		suffix = "rus";
		break;

	case GameVersion::RUSSIAN_GOLD:
		suffix = "rus-gold";
		break;

	case GameVersion::SIMPLIFIED_CHINESE:
		suffix = "chs";
		break;
	default:
		suffix = "eng";
		break;
	}

	auto fullName = ST::format("{}-{}.json", name, suffix);

	auto json = readJsonDataFileWithSchema(fullName);
	auto obj = json.toObject();
	for (auto& c : obj.keys())
	{
		auto fixedC = c.to_utf32();
		if (fixedC.size() != 1) {
			throw DataError(ST::format("Translation table entry needs to be a single character string was {}", fixedC.size()));
		}
		m_translationTable[fixedC[0]] = obj.GetUInt(c.c_str());
	}
}

const std::vector<const BloodCatPlacementsModel*>& DefaultContentManager::getBloodCatPlacements() const
{
	return m_bloodCatPlacements;
}

const std::vector<const BloodCatSpawnsModel*>& DefaultContentManager::getBloodCatSpawns() const
{
	return m_bloodCatSpawns;
}

const BloodCatSpawnsModel* DefaultContentManager::getBloodCatSpawnsOfSector(uint8_t sectorId) const
{
	for ( auto spawns : m_bloodCatSpawns )
	{
		if ( spawns->sectorId == sectorId )
		{
			return spawns;
		}
	}
	return NULL;
}

const std::vector<const CreatureLairModel*>& DefaultContentManager::getCreatureLairs() const
{
	return m_creatureLairs;
}

const CreatureLairModel* DefaultContentManager::getCreatureLair(uint8_t lairId) const
{
	for (auto lair : m_creatureLairs)
	{
		if (lair->lairId == lairId)
		{
			return lair;
		}
	}
	return NULL;
}

const CreatureLairModel* DefaultContentManager::getCreatureLairByMineId(uint8_t mineId) const
{
	for (auto lair : m_creatureLairs)
	{
		if (lair->associatedMineId == mineId)
		{
			return lair;
		}
	}
	return NULL;
}

const MineModel* DefaultContentManager::getMineForSector(const SGPSector& sector) const
{
	auto sectorId = sector.AsByte();
	for (auto m : m_mines)
	{
		if (sector.z == 0 && sectorId == m->entranceSector) return m;
		for (auto const& s : m->mineSectors)
		{
			if (s[0] == sectorId && s[1] == sector.z) return m;
		}
	}

	return NULL;
}

const MineModel* DefaultContentManager::getMine(uint8_t mineId) const
{
	return m_mines[mineId];
}

const std::vector<const MineModel*>& DefaultContentManager::getMines() const
{
	return m_mines;
}

const TownModel* DefaultContentManager::getTown(int8_t townId) const
{
	auto iter = m_towns.find(townId);
	return (iter != m_towns.end()) ? iter->second : NULL;
}

const std::vector<const SamSiteModel*>& DefaultContentManager::getSamSites() const
{
	return m_samSites;
}

int8_t DefaultContentManager::findSamIDBySector(uint8_t sectorId) const
{
	for (size_t i = 0; i < m_samSites.size(); i++)
	{
		if (m_samSites[i]->sectorId == sectorId)
		{
			return static_cast<int8_t>(i);
		}
	}
	return -1;
}

const SamSiteModel* DefaultContentManager::findSamSiteBySector(uint8_t sectorId) const
{
	auto i = findSamIDBySector(sectorId);
	return (i > -1) ? m_samSites[i] : NULL;
}

int8_t DefaultContentManager::getControllingSamSite(uint8_t sectorId) const
{
	return m_samSitesAirControl->getControllingSamSiteID(sectorId);
}

const std::map<int8_t, const TownModel*>& DefaultContentManager::getTowns() const
{
	return m_towns;
}

const ST::string DefaultContentManager::getTownName(uint8_t townId) const
{
	if (townId >= m_townNames.size()) {
		SLOGD("Town name not defined for index {}", townId);
		return {};
	}
	return m_townNames[townId];
}

const ST::string DefaultContentManager::getTownLocative(uint8_t townId) const
{
	if (townId >= m_townNameLocatives.size()) {
		SLOGD("Town name locative not defined for index {}", townId);
		return {};
	}
	return m_townNameLocatives[townId];
}

const std::vector<const UndergroundSectorModel*>& DefaultContentManager::getUndergroundSectors() const
{
	return m_undergroundSectors;
}

const MovementCostsModel* DefaultContentManager::getMovementCosts() const
{
	return m_movementCosts.get();
}

int16_t DefaultContentManager::getSectorLandType(uint8_t const sectorID, uint8_t const sectorLevel) const
{
	SGPSector key = SGPSector::FromSectorID(sectorID, sectorLevel);
	auto result = m_sectorLandTypes.find(key);
	if (result == m_sectorLandTypes.end())
	{
		return -1;
	}
	return result->second;
}

const CacheSectorsModel* DefaultContentManager::getCacheSectors() const
{
	return m_cacheSectors.get();
}

const std::vector<const StrategicMapSecretModel*>& DefaultContentManager::getMapSecrets() const
{
	return m_mapSecrets;
}

const NpcPlacementModel* DefaultContentManager::getNpcPlacement(uint8_t profileId) const
{
	return m_npcPlacements.at(profileId);
}

const std::map<uint8_t, const NpcPlacementModel*>& DefaultContentManager::listNpcPlacements() const
{
	return m_npcPlacements;
}

const RPCSmallFaceModel* DefaultContentManager::getRPCSmallFaceOffsets(uint8_t profileID) const
{
	if (m_rpcSmallFaces.find(profileID) == m_rpcSmallFaces.end())
	{
		return NULL;
	}
	return m_rpcSmallFaces.at(profileID);
}

const std::vector<const MERCListingModel*>& DefaultContentManager::getMERCListings() const
{
	return m_MERCListings;
}

const MercProfileInfo* DefaultContentManager::getMercProfileInfo(uint8_t const profileID) const
{
	if (m_mercProfileInfo.find(profileID) != m_mercProfileInfo.end())
	{
		return m_mercProfileInfo.at(profileID);
	}

	SLOGD("MercProfileInfo is not defined at {}", profileID);
	return &EMPTY_MERC_PROFILE_INFO;
}

const MercProfileInfo* DefaultContentManager::getMercProfileInfoByName(const ST::string& name) const
{
	for (auto i = m_mercProfileInfo.begin(); i != m_mercProfileInfo.end(); i++) {
		if (i->second->internalName == name) {
			return i->second;
		}
	}

	SLOGW("MercProfileInfo is not defined for {}", name);
	return NULL;
}

const std::vector<const MercProfile*>& DefaultContentManager::listMercProfiles() const
{
	return m_mercProfiles;
}

const VehicleModel* DefaultContentManager::getVehicle(uint8_t const vehicleID) const
{
	if (vehicleID > m_vehicles.size())
	{
		ST::string error = ST::format("Vehicle #{} is not defined", vehicleID);
		throw std::out_of_range(error.to_std_string());
	}
	return m_vehicles[vehicleID];
}

const LoadingScreen* DefaultContentManager::getLoadingScreenForSector(uint8_t sectorId, uint8_t sectorLevel, bool isNight) const
{
	return m_loadingScreenModel->getScreenForSector(sectorId, sectorLevel, isNight);
}

const LoadingScreen* DefaultContentManager::getLoadingScreen(uint8_t index) const
{
	return m_loadingScreenModel->getByIndex(index);
}

const std::map<UINT32, UINT16>* DefaultContentManager::getTranslationTable() const
{
	return &m_translationTable;
}

const std::vector<std::pair<ST::string, ST::string>> DefaultContentManager::getEnabledMods() const
{
	std::vector<std::pair<ST::string, ST::string>> mods;
	auto nmods = EngineOptions_getModsLength(this->m_engineOptions.get());
	for (UINT32 i = 0; i < nmods; i++) {
		RustPointer<char> modId(EngineOptions_getMod(this->m_engineOptions.get(), i));
		RustPointer<Mod> mod(ModManager_getAvailableModById(this->m_modManager.get(), modId.get()));
		RustPointer<char> modVersion(Mod_getVersionString(mod.get()));

		mods.push_back(std::pair(modId.get(), modVersion.get()));
	}

	return mods;
}
