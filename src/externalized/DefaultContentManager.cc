#include "DefaultContentManager.h"

#include "game/Directories.h"
#include "game/Strategic/Strategic_Status.h"
#include "game/Tactical/Items.h"
#include "game/Tactical/Weapons.h"

// XXX: GameRes.h should be integrated to ContentManager
#include "game/GameRes.h"

// XXX
#include "game/GameState.h"

#include "sgp/FileMan.h"
#include "sgp/MemMan.h"

#include "AmmoTypeModel.h"
#include "CacheSectorsModel.h"
#include "CalibreModel.h"
#include "ContentMusic.h"
#include "DealerInventory.h"
#include "DealerModel.h"
#include "JsonObject.h"
#include "JsonUtility.h"
#include "LoadingScreenModel.h"
#include "MagazineModel.h"
#include "RustInterface.h"
#include "ShippingDestinationModel.h"
#include "WeaponModels.h"
#include "army/ArmyCompositionModel.h"
#include "army/GarrisonGroupModel.h"
#include "army/PatrolGroupModel.h"
#include "mercs/MERCListingModel.h"
#include "mercs/MercProfileInfo.h"
#include "mercs/RPCSmallFaceModel.h"
#include "policy/DefaultGamePolicy.h"
#include "policy/DefaultIMPPolicy.h"
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
#include "tactical/MapItemReplacementModel.h"
#include "tactical/NpcActionParamsModel.h"

#include "Logger.h"
#include "Strategic_AI.h"
#include "Soldier_Profile_Type.h"

#include "rapidjson/error/en.h"
#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>

#define BASEDATADIR    "data"

#define MAPSDIR        "maps"
#define RADARMAPSDIR   "radarmaps"
#define TILESETSDIR    "tilesets"

#define PRINT_OPENING_FILES (0)

#define DIALOGUESIZE 240

// XXX: Issue #135
// We need a safe way to create temporary directories - unique and random for every process
// Boost probably provides this functionality
#define NEW_TEMP_DIR "temp"

const MercProfileInfo EMPTY_MERC_PROFILE_INFO;

static ST::string LoadEncryptedData(ST::string& err_msg, STRING_ENC_TYPE encType, SGPFile* File, UINT32 seek_chars, UINT32 read_chars)
{
	FileSeek(File, seek_chars * 2, FILE_SEEK_FROM_START);

	ST::utf16_buffer buf(read_chars, u'\0');
	FileRead(File, buf.data(), sizeof(char16_t) * read_chars);

	buf[read_chars - 1] = u'\0';
	for (char16_t* i = buf.data(); *i != u'\0'; ++i)
	{
		/* "Decrypt" the ROT-1 "encrypted" data */
		char16_t c = (*i > 33 ? *i - 1 : *i);

		if(encType == SE_RUSSIAN)
		{
			/* The Russian data files are incorrectly encoded. The original texts seem to
			 * be encoded in CP1251, but then they were converted from CP1252 (!) to
			 * UTF-16 to store them in the data files. Undo this damage here. */
			if (0xC0 <= c && c <= 0xFF) c += 0x0350;
		}
		else
		{
			if(encType == SE_ENGLISH)
			{
				/* The English data files are incorrectly encoded. The original texts seem
				 * to be encoded in CP437, but then they were converted from CP1252 (!) to
				 * UTF-16 to store them in the data files. Undo this damage here. This
				 * problem only occurs for a few lines by Malice. */
				switch (c)
				{
					case 128: c = 0x00C7; break; // Ç
					case 130: c = 0x00E9; break; // é
					case 135: c = 0x00E7; break; // ç
				}
			}
			else if(encType == SE_POLISH)
			{
				/* The Polish data files are incorrectly encoded. The original texts seem to
				 * be encoded in CP1250, but then they were converted from CP1252 (!) to
				 * UTF-16 to store them in the data files. Undo this damage here.
				 * Also the format code for centering texts differs. */
				switch (c)
				{
					case 143: c = 0x0179; break;
					case 163: c = 0x0141; break;
					case 165: c = 0x0104; break;
					case 175: c = 0x017B; break;
					case 179: c = 0x0142; break;
					case 182: c = 179;    break; // not a char, but a format code (centering)
					case 185: c = 0x0105; break;
					case 191: c = 0x017C; break;
					case 198: c = 0x0106; break;
					case 202: c = 0x0118; break;
					case 209: c = 0x0143; break;
					case 230: c = 0x0107; break;
					case 234: c = 0x0119; break;
					case 241: c = 0x0144; break;
					case 338: c = 0x015A; break;
					case 339: c = 0x015B; break;
					case 376: c = 0x017A; break;
				}
			}

			/* Cyrillic texts (by Ivan Dolvich) in the non-Russian versions are encoded
			 * in some wild manner. Undo this damage here. */
			if (0x044D <= c && c <= 0x0452) // cyrillic A to IE
			{
				c += -0x044D + 0x0410;
			}
			else if (c == 0x0453) // cyrillic IO
			{
				c = 0x0401;
			}
			else if (0x0454 <= c && c <= 0x0467) // cyrillic ZHE to SHCHA
			{
				c += -0x0454 + 0x0416;
			}
			else if (0x0468 <= c && c <= 0x046C) // cyrillic YERU to YA
			{
				c += -0x0468 + 0x042B;
			}
		}

		*i = c;
	}
	return st_checked_buffer_to_string(err_msg, buf);
}

DefaultContentManager::DefaultContentManager(GameVersion gameVersion,
						const ST::string &userHomeDir,
						const ST::string &gameResRootPath,
						const ST::string &externalizedDataPath
	)
	:m_gameVersion(gameVersion),
	mNormalGunChoice(ARMY_GUN_LEVELS),
	mExtendedGunChoice(ARMY_GUN_LEVELS),
	m_vfs(Vfs_create())
{
	/*
	 * Searching actual paths to directories 'Data' and 'Data/Tilecache', 'Data/Maps'
	 * On case-sensitive filesystems that might be tricky: if such directories
	 * exist we should use them.  If doesn't exist, then use lowercased names.
	 */

	m_userHomeDir = userHomeDir;
	m_gameResRootPath = gameResRootPath;
	m_externalizedDataPath = externalizedDataPath;

	RustPointer<char> path{Fs_resolveExistingComponents(BASEDATADIR, m_gameResRootPath.c_str(), true)};
	m_dataDir = path.get();

	path.reset(Fs_resolveExistingComponents(TILECACHEDIR, m_dataDir.c_str(), true));
	m_tileDir = path.get();

	m_bobbyRayNewInventory = NULL;
	m_bobbyRayUsedInventory = NULL;
	m_impPolicy = NULL;
	m_gamePolicy = NULL;

	m_cacheSectors = NULL;
	m_movementCosts = NULL;
	m_loadingScreenModel = NULL;
	m_samSitesAirControl = NULL;
}

void DefaultContentManager::init(EngineOptions* engine_options)
{
	auto succeeded = Vfs_init_from_engine_options(m_vfs.get(), engine_options);
	if (!succeeded) {
		RustPointer<char> err{ getRustError() };
		auto error = ST::format("Failed to build virtual file system (VFS): {}", err.get());
		SLOGE(error);
		throw std::runtime_error(error.c_str());
	}
}

template <class T> 
void deleteElements(std::vector<const T*> vec)
{
	for (auto elem : vec)
	{
		delete elem;
	}
	vec.clear();
}

template <typename K, class V>
void deleteElements(std::map<K, const V*> map)
{
	for (auto& kv : map)
	{
		delete kv.second;
	}
	map.clear();
}

DefaultContentManager::~DefaultContentManager()
{
	for (const ItemModel* item : m_items)
	{
		delete item;
	}
	m_items.clear();
	m_magazineMap.clear();
	m_magazines.clear();
	m_weaponMap.clear();
	m_itemMap.clear();
	m_mapItemReplacements.clear();

	m_armyCompositions.clear();
	m_garrisonGroups.clear();
	m_patrolGroups.clear();

	for (const CalibreModel* calibre : m_calibres)
	{
		delete calibre;
	}
	m_calibres.clear();
	m_calibreMap.clear();

	for (const AmmoTypeModel* ammoType : m_ammoTypes)
	{
		delete ammoType;
	}
	m_ammoTypes.clear();
	m_ammoTypeMap.clear();

	deleteElements(m_dealersInventory);
	delete m_bobbyRayNewInventory;
	delete m_bobbyRayUsedInventory;

	deleteElements(m_dealers);

	delete m_impPolicy;
	delete m_gamePolicy;
	delete m_loadingScreenModel;

	deleteElements(m_newStrings);
	deleteElements(m_landTypeStrings);
	for (const ST::string *str : m_calibreNames)
	{
		delete str;
	}
	for (const ST::string *str : m_calibreNamesBobbyRay)
	{
		delete str;
	}

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
	deleteElements(m_shippingDestinationNames);
	deleteElements(m_towns);
	deleteElements(m_townNames);
	deleteElements(m_townNameLocatives);
	deleteElements(m_undergroundSectors);
	deleteElements(m_rpcSmallFaces);
	deleteElements(m_MERCListings);
	deleteElements(m_mercProfileInfo);
	deleteElements(m_mercProfiles);

	m_sectorLandTypes.clear();

	delete m_cacheSectors;
	delete m_movementCosts;
	delete m_samSitesAirControl;
}

const DealerInventory* DefaultContentManager::getBobbyRayNewInventory() const
{
	return m_bobbyRayNewInventory;
}

const DealerInventory* DefaultContentManager::getBobbyRayUsedInventory() const
{
	return m_bobbyRayUsedInventory;
}

const DealerModel* DefaultContentManager::getDealer(uint8_t dealerID) const
{
	return m_dealers[dealerID];
}

const std::vector<const DealerModel*> DefaultContentManager::getDealers() const
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
	throw std::runtime_error("Bobby Ray primary destination is not defined");
}

const ST::string* DefaultContentManager::getShippingDestinationName(uint8_t index) const
{
	return m_shippingDestinationNames[index];
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
	ST::string result = MAPSDIR;
	result += '/';
	result += mapName.c_str();

	SLOGD(ST::format("map file {}", result));

	return result;
}

/** Get radar map resource name. */
ST::string DefaultContentManager::getRadarMapResourceName(const ST::string &mapName) const
{
	ST::string result = RADARMAPSDIR;
	result += "/";
	result += mapName;

	SLOGD(ST::format("map file {}", result));

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

/** Get directory for storing new map file. */
ST::string DefaultContentManager::getNewMapFolder() const
{
	return FileMan::joinPaths(m_dataDir, MAPSDIR);
}

/** Get all available maps. */
std::vector<ST::string> DefaultContentManager::getAllMaps() const
{
	return FindFilesInDir(MAPSDIR, "dat", true, true, true);
}

/** Get all available tilecache. */
std::vector<ST::string> DefaultContentManager::getAllTilecache() const
{
	return FindFilesInDir(m_tileDir, "jsd", true, false);
}

/** Open temporary file for writing. */
SGPFile* DefaultContentManager::openTempFileForWriting(const ST::string& filename, bool truncate) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	return FileMan::openForWriting(path, truncate);
}

/** Open temporary file for appending. */
SGPFile* DefaultContentManager::openTempFileForAppend(const ST::string& filename) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	return FileMan::openForAppend(path);
}

/* Open temporary file for reading. */
SGPFile* DefaultContentManager::openTempFileForReading(const ST::string& filename) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	RustPointer<File> file(File_open(path.c_str(), FILE_OPEN_READ));
	if (!file)
	{
		RustPointer<char> err(getRustError());
		ST::string buf = ST::format("DefaultContentManager::openTempFileForReading: {}", err.get());
		throw std::runtime_error(buf.to_std_string());
	}
	return FileMan::getSGPFileFromFile(file.release());
}

/** Delete temporary file. */
void DefaultContentManager::deleteTempFile(const ST::string& filename) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	FileDelete(path);
}

/* Open a game resource file for reading.
 *
 * First trying to open the file normally. It will work if the path is absolute
 * and the file is found or path is relative to the current directory (game
 * settings directory) and file is present.
 * If file is not found, try to find it relatively to 'Data' directory.
 * If file is not found, try to find the file in libraries located in 'Data' directory; */
SGPFile* DefaultContentManager::openGameResForReading(const ST::string& filename) const
{
	{
		RustPointer<File> file = FileMan::openFileForReading(filename);
		if (file)
		{
			SLOGD(ST::format("Opened file (current dir): '{}'", filename));
			return FileMan::getSGPFileFromFile(file.release());
		}
	}

	RustPointer<VfsFile> vfile(VfsFile_open(m_vfs.get(), filename.c_str()));
	if (!vfile)
	{
		RustPointer<char> err{getRustError()};
		throw std::runtime_error(ST::format("openGameResForReading: {}", err.get()).to_std_string());
	}
	SLOGD(ST::format("Opened file (vfs): '{}'", filename));
	SGPFile *file = new SGPFile{};
	file->flags = SGPFILE_NONE;
	file->u.vfile = vfile.release();
	return file;
}

/** Open user's private file (e.g. saved game, settings) for reading. */
SGPFile* DefaultContentManager::openUserPrivateFileForReading(const ST::string& filename) const
{
	RustPointer<File> file = FileMan::openFileForReading(filename);
	if (!file)
	{
		RustPointer<char> err(getRustError());
		ST::string buf = ST::format("DefaultContentManager::openUserPrivateFileForReading: {}", err.get());
		throw std::runtime_error(buf.to_std_string());
	}
	return FileMan::getSGPFileFromFile(file.release());
}

/* Checks if a game resource exists. */
bool DefaultContentManager::doesGameResExists(const ST::string& filename) const
{
	if(FileMan::checkFileExistance(m_externalizedDataPath, filename))
	{
		return true;
	}
	else
	{
		RustPointer<File> file(File_open(filename.c_str(), FILE_OPEN_READ));
		if (!file)
		{
			ST::string path = ST::format("{}/{}", m_dataDir, filename);
			file.reset(File_open(path.c_str(), FILE_OPEN_READ));
			if (!file)
			{
				RustPointer<VfsFile> vfile(VfsFile_open(m_vfs.get(), filename.c_str()));
				return static_cast<bool>(vfile);
			}
		}

		return true;
	}
}

ST::string DefaultContentManager::getScreenshotFolder() const
{
	return m_userHomeDir;
}

ST::string DefaultContentManager::getVideoCaptureFolder() const
{
	return m_userHomeDir;
}

/** Get folder for saved games. */
ST::string DefaultContentManager::getSavedGamesFolder() const
{
	return FileMan::joinPaths(m_userHomeDir, "SavedGames");
}

/** Load encrypted string from game resource file. */
ST::string DefaultContentManager::loadEncryptedString(const ST::string& fileName, uint32_t seek_chars, uint32_t read_chars) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	ST::string err_msg;
	ST::string str = LoadEncryptedData(err_msg, getStringEncType(), File, seek_chars, read_chars);
	if (!err_msg.empty())
	{
		SLOGW(ST::format("DefaultContentManager::loadEncryptedString '{}' {} {}: {}", fileName, seek_chars, read_chars, err_msg));
	}
	return str;
}

ST::string DefaultContentManager::loadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars) const
{
	ST::string err_msg;
	ST::string str = LoadEncryptedData(err_msg, getStringEncType(), File, seek_chars, read_chars);
	if (!err_msg.empty())
	{
		SLOGW(ST::format("DefaultContentManager::loadEncryptedString ? {} {}: {}", seek_chars, read_chars, err_msg));
	}
	return str;
}


/** Load dialogue quote from file. */
ST::string* DefaultContentManager::loadDialogQuoteFromFile(const ST::string& fileName, int quote_number)
{
	AutoSGPFile File(openGameResForReading(fileName));
	ST::string err_msg;
	ST::string quote = LoadEncryptedData(err_msg, getStringEncType(), File, quote_number * DIALOGUESIZE, DIALOGUESIZE);
	if (!err_msg.empty())
	{
		SLOGW(ST::format("DefaultContentManager::loadDialogQuoteFromFile '{}' {}: {}", fileName, quote_number, err_msg));
	}
	return new ST::string(quote);
}

/** Load all dialogue quotes for a character. */
void DefaultContentManager::loadAllDialogQuotes(STRING_ENC_TYPE encType, const ST::string& fileName, std::vector<ST::string*> &quotes) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	uint32_t fileSize = FileGetSize(File);
	uint32_t numQuotes = fileSize / DIALOGUESIZE / 2;

	for(uint32_t i = 0; i < numQuotes; i++)
	{
		ST::string err;
		ST::string quote = LoadEncryptedData(err, encType, File, i * DIALOGUESIZE, DIALOGUESIZE);
		if (!err.empty())
		{
			SLOGW(ST::format("DefaultContentManager::loadAllDialogQuotes '{}' {}: {}", fileName, i, err));
		}
		quotes.push_back(new ST::string(quote));
	}
}

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
		SLOGE(ST::format("weapon '{}' is not found", internalName));
		throw std::runtime_error(ST::format("weapon '{}' is not found", internalName).to_std_string());
	}
	return it->second;//m_weaponMap[internalName];
}

const MagazineModel* DefaultContentManager::getMagazineByName(const ST::string &internalName)
{
	if(m_magazineMap.find(internalName) == m_magazineMap.end())
	{
		SLOGE(ST::format("magazine '{}' is not found", internalName));
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
	return m_calibreNames[index];
}

const ST::string* DefaultContentManager::getCalibreNameForBobbyRay(uint8_t index) const
{
	return m_calibreNamesBobbyRay[index];
}

const AmmoTypeModel* DefaultContentManager::getAmmoType(uint8_t index)
{
	return m_ammoTypes[index];
}

bool DefaultContentManager::loadWeapons()
{
	auto document = readJsonDataFile("weapons.json");
	if (document->IsArray()) 
	{
		const rapidjson::Value& a = document->GetArray();
		for (rapidjson::SizeType i = 0; i < a.Size(); i++)
		{
			JsonObjectReader obj(a[i]);
			WeaponModel *w = WeaponModel::deserialize(obj, m_calibreMap);
			SLOGD(ST::format("Loaded weapon {} {}", w->getItemIndex(), w->getInternalName()));

			if (w->getItemIndex() > MAX_WEAPONS)
			{
				SLOGE(ST::format("Weapon index must be in the interval 0 - {}", MAX_WEAPONS));
				return false;
			}

			m_items[w->getItemIndex()] = w;
			m_weaponMap.insert(std::make_pair(w->getInternalName(), w));
		}
	}

	return true;
}

bool DefaultContentManager::loadMagazines()
{
	auto document = readJsonDataFile("magazines.json");
	if(document->IsArray()) 
	{
		const rapidjson::Value& a = document->GetArray();
		for (rapidjson::SizeType i = 0; i < a.Size(); i++)
		{
			JsonObjectReader obj(a[i]);
			MagazineModel *mag = MagazineModel::deserialize(obj, m_calibreMap, m_ammoTypeMap);
			SLOGD(ST::format("Loaded magazine {} {}", mag->getItemIndex(), mag->getInternalName()));

			if((mag->getItemIndex() < FIRST_AMMO) || (mag->getItemIndex() > LAST_AMMO))
			{
				SLOGE(ST::format("Magazine item index must be in the interval {} - {}", FIRST_AMMO, LAST_AMMO));
				return false;
			}

			m_magazines.push_back(mag);
			m_items[mag->getItemIndex()] = mag;
			m_magazineMap.insert(std::make_pair(mag->getInternalName(), mag));
		}
	}
	
	return true;
}

bool DefaultContentManager::loadCalibres()
{
	auto document = readJsonDataFile("calibres.json");
	if (document->IsArray()) {
		const rapidjson::Value& a = document->GetArray();
		for (rapidjson::SizeType i = 0; i < a.Size(); i++)
		{
			JsonObjectReader obj(a[i]);
			CalibreModel *calibre = CalibreModel::deserialize(obj);
			SLOGD(ST::format("Loaded calibre {} {}", calibre->index, calibre->internalName));

			if(m_calibres.size() <= calibre->index)
			{
				m_calibres.resize(calibre->index + 1);
			}

			m_calibres[calibre->index] = calibre;
		}
	}

	for (const CalibreModel* calibre : m_calibres)
	{
		m_calibreMap.insert(std::make_pair(ST::string(calibre->internalName), calibre));
	}

	return true;
}

bool DefaultContentManager::loadAmmoTypes()
{
	auto document = readJsonDataFile("ammo_types.json");
	if(document->IsArray()) {
		const rapidjson::Value& a = document->GetArray();
		for (rapidjson::SizeType i = 0; i < a.Size(); i++)
		{
			JsonObjectReader obj(a[i]);
			AmmoTypeModel *ammoType = AmmoTypeModel::deserialize(obj);
			SLOGD(ST::format("Loaded ammo type {} {}", ammoType->index, ammoType->internalName));

			if(m_ammoTypes.size() <= ammoType->index)
			{
				m_ammoTypes.resize(ammoType->index + 1);
			}

			m_ammoTypes[ammoType->index] = ammoType;
		}
	}

	for (const AmmoTypeModel* ammoType : m_ammoTypes)
	{
		m_ammoTypeMap.insert(std::make_pair(ST::string(ammoType->internalName), ammoType));
	}

	return true;
}

bool DefaultContentManager::loadMusicModeList(const MusicMode mode, rapidjson::Value &array)
{
	std::vector<const ST::string*>* musicModeList = new std::vector<const ST::string*>();

	std::vector<ST::string> utf8_encoded;
	JsonUtility::parseListStrings(array, utf8_encoded);
	for (const ST::string &str : utf8_encoded)
	{
		musicModeList->push_back(new ST::string(str));
		SLOGD(ST::format("Loaded music {}", str));
	}

	m_musicMap[mode] = musicModeList;

	return true;
}

bool DefaultContentManager::loadMusic()
{
	auto document = readJsonDataFile("music.json");
	if(!document->IsObject()) {
		SLOGE("music.json has wrong structure");
		return false;
	}

	auto obj = document->GetObject();
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

bool DefaultContentManager::readWeaponTable(
	const ST::string& fileName,
	std::vector<std::vector<const WeaponModel*> > & weaponTable)
{
	auto document = readJsonDataFile(fileName);
	if(document->IsArray())
	{
		const rapidjson::Value& a = document->GetArray();
		for (rapidjson::SizeType i = 0; i < a.Size(); i++)
		{
			std::vector<ST::string> weaponNames;
			if(JsonUtility::parseListStrings(a[i], weaponNames))
			{
				for (const ST::string &weapon : weaponNames)
				{
					weaponTable[i].push_back(getWeaponByName(weapon));
				}
			}
		}
	}

	return true;
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

	auto jsonAC = readJsonDataFile("army-compositions.json");
	auto armyCompModels = ArmyCompositionModel::deserialize(*jsonAC);
	ArmyCompositionModel::validateData(armyCompModels);

	std::map<ST::string, uint8_t> mapping;
	for (auto& armyComp : armyCompModels)
	{
		mapping[armyComp->name] = armyComp->compositionId;
		m_armyCompositions.push_back(armyComp->toArmyComposition());
	}
	deleteElements(armyCompModels);
	armyCompModels.clear();

	auto jsonGG = readJsonDataFile("army-garrison-groups.json");
	for (auto& element : jsonGG->GetArray())
	{
		auto obj = JsonObjectReader(element);
		m_garrisonGroups.push_back(
			GarrisonGroupModel::deserialize(obj, mapping)
		);
	}
	GarrisonGroupModel::validateData(m_garrisonGroups);

	auto jsonPG = readJsonDataFile("army-patrol-groups.json");
	for (auto& element : jsonPG->GetArray())
	{
		m_patrolGroups.push_back(
			PatrolGroupModel::deserialize(element)
		);
	}
	PatrolGroupModel::validateData(m_patrolGroups);

	return true;
}

void DefaultContentManager::loadStringRes(const ST::string& name, std::vector<const ST::string*> &strings) const
{
	ST::string fullName(name);

	switch(m_gameVersion)
	{
	case GameVersion::DUTCH:        fullName += "-dut";   break;
	case GameVersion::ENGLISH:      fullName += "-eng";   break;
	case GameVersion::FRENCH:       fullName += "-fr";    break;
	case GameVersion::GERMAN:       fullName += "-ger";   break;
	case GameVersion::ITALIAN:      fullName += "-it";    break;
	case GameVersion::POLISH:       fullName += "-pl";    break;
	case GameVersion::RUSSIAN:
	case GameVersion::RUSSIAN_GOLD: fullName += "-rus";   break;
	default:
	{
		throw std::runtime_error(ST::format("unknown game version {}", static_cast<int>(m_gameVersion)).to_std_string());
	}
	}

	fullName += ".json";
	auto json = readJsonDataFile(fullName.c_str());
	std::vector<ST::string> utf8_encoded;
	JsonUtility::parseListStrings(*json, utf8_encoded);
	for (const ST::string &str : utf8_encoded)
	{
		strings.push_back(new ST::string(str));
	}
}

/** Load the game data. */
bool DefaultContentManager::loadGameData()
{
	createAllHardcodedItemModels(m_items);

	bool result = loadCalibres()
		&& loadAmmoTypes()
		&& loadMagazines()
		&& loadWeapons()
		&& loadArmyData()
		&& loadMusic();

	for (const ItemModel *item : m_items)
	{
		m_itemMap.insert(std::make_pair(item->getInternalName(), item));
	}

	auto replacement_json = readJsonDataFile("tactical-map-item-replacements.json");
	m_mapItemReplacements = MapItemReplacementModel::deserialize(replacement_json.get(), this);

	loadAllDealersAndInventory();

	auto game_json = readJsonDataFile("game.json");
	m_gamePolicy = new DefaultGamePolicy(game_json.get());

	auto imp_json = readJsonDataFile("imp.json");
	m_impPolicy = new DefaultIMPPolicy(imp_json.get(), this);

	loadStringRes("strings/shipping-destinations", m_shippingDestinationNames);

	auto shippingDestJson = readJsonDataFile("shipping-destinations.json");
	for (auto& element : shippingDestJson->GetArray())
	{
		auto r = JsonObjectReader(element);
		m_shippingDestinations.push_back(ShippingDestinationModel::deserialize(r));
	}
	ShippingDestinationModel::validateData(m_shippingDestinations, m_shippingDestinationNames);

	auto loadScreensList = readJsonDataFile("loading-screens.json");
	auto loadScreensMapping = readJsonDataFile("loading-screens-mapping.json");
	m_loadingScreenModel = LoadingScreenModel::deserialize(*loadScreensList, *loadScreensMapping);
	m_loadingScreenModel->validateData(this);

	loadStringRes("strings/ammo-calibre", m_calibreNames);
	loadStringRes("strings/ammo-calibre-bobbyray", m_calibreNamesBobbyRay);

	loadStringRes("strings/new-strings", m_newStrings);
	loadStringRes("strings/strategic-map-land-types", m_landTypeStrings);

	loadStrategicLayerData();
	loadTacticalLayerData();
	loadMercsData();

	return result;
}

std::unique_ptr<rapidjson::Document> DefaultContentManager::readJsonDataFile(const ST::string& fileName) const
{
	AutoSGPFile f(openGameResForReading(fileName));
	ST::string jsonData = FileMan::fileReadText(f);

	auto document = std::make_unique<rapidjson::Document>();
	if (document->Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		ST::string errorMessage = ST::format("Failed to parse {} (at location {}) {} ",
			fileName,
			document->GetErrorOffset(),
			rapidjson::GetParseError_En(document->GetParseError())
		);
		throw std::runtime_error(errorMessage.to_std_string());
	}

	return document;
}

const DealerInventory * DefaultContentManager::loadDealerInventory(const ST::string& fileName)
{
	return new DealerInventory(readJsonDataFile(fileName).get(), this);
}

bool DefaultContentManager::loadAllDealersAndInventory()
{
	auto json = readJsonDataFile("dealers.json");
	int index = 0;
	for (auto& element : json->GetArray())
	{
		m_dealers.push_back(DealerModel::deserialize(element, index++));
	}
	DealerModel::validateData(m_dealers);
	
	m_dealersInventory = std::vector<const DealerInventory*>(m_dealers.size());
	for (auto dealer : m_dealers)
	{
		ST::string filename = dealer->getInventoryDataFileName();
		m_dealersInventory[dealer->dealerID] = loadDealerInventory(filename.c_str());
	}
	m_bobbyRayNewInventory                        = loadDealerInventory("bobby-ray-inventory-new.json");
	m_bobbyRayUsedInventory                       = loadDealerInventory("bobby-ray-inventory-used.json");
	return true;
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
		SLOGE(ST::format("item '{}' is not found", internalName));
		throw std::runtime_error(ST::format("item '{}' is not found", internalName).to_std_string());
	}
	return it->second;
}

const std::map<uint16_t, uint16_t> DefaultContentManager::getMapItemReplacements() const
{
	return m_mapItemReplacements;
}

const DealerInventory* DefaultContentManager::getDealerInventory(int dealerId) const
{
	return m_dealersInventory[dealerId];
}

const ST::string* DefaultContentManager::getMusicForMode(MusicMode mode) const {
	const uint32_t index = Random((uint32_t)m_musicMap.find(mode)->second->size());
	const ST::string* chosen = m_musicMap.find(mode)->second->at(index);

	SLOGD(ST::format("Choosing music index {} of {} for: '{}'", index, m_musicMap.find(mode)->second->size(), chosen));
	return chosen;
}

const IMPPolicy* DefaultContentManager::getIMPPolicy() const
{
	return m_impPolicy;
}

const GamePolicy* DefaultContentManager::getGamePolicy() const
{
	return m_gamePolicy;
}

const ST::string* DefaultContentManager::getNewString(size_t stringId) const
{
	if(stringId >= m_newStrings.size())
	{
		ST::string message = ST::format("new string {} is not found", stringId);
		SLOGE(message.c_str());
		throw std::runtime_error(message.c_str());
	}
	else
	{
		return m_newStrings[stringId];
	}
}

const ST::string& DefaultContentManager::getLandTypeString(size_t index) const
{
	return *m_landTypeStrings.at(index);
}

bool DefaultContentManager::loadStrategicLayerData() 
{
	auto json = readJsonDataFile("strategic-bloodcat-placements.json");
	for (auto& element : json->GetArray()) {
		auto obj = JsonObjectReader(element);
		m_bloodCatPlacements.push_back(
			BloodCatPlacementsModel::deserialize(obj)
		);
	}

	json = readJsonDataFile("strategic-bloodcat-spawns.json");
	for (auto& element : json->GetArray()) 
	{
		auto obj = JsonObjectReader(element);
		m_bloodCatSpawns.push_back(
			BloodCatSpawnsModel::deserialize(obj)
		);
	}

	json = readJsonDataFile("strategic-map-creature-lairs.json");
	for (auto& element : json->GetArray())
	{
		m_creatureLairs.push_back(
			CreatureLairModel::deserialize(element)
		);
	}

	json = readJsonDataFile("strategic-fact-params.json");
	for (auto& element : json->GetArray())
	{
		auto params = FactParamsModel::deserialize(element);
		m_factParams[params->fact] = params;
	}

	json = readJsonDataFile("strategic-mines.json");
	auto arr = json->GetArray();
	for (rapidjson::SizeType i = 0; i < arr.Size(); i++)
	{
		m_mines.push_back(
			MineModel::deserialize(i, arr[i].GetObject())
		);
	}
	MineModel::validateData(m_mines);

	json = readJsonDataFile("strategic-map-sam-sites.json");
	for (auto& element : json->GetArray())
	{
		auto samSite = SamSiteModel::deserialize(element);
		m_samSites.push_back(samSite);
	}
	SamSiteModel::validateData(m_samSites);

	json = readJsonDataFile("strategic-map-sam-sites-air-control.json");
	m_samSitesAirControl = SamSiteAirControlModel::deserialize(*json);
	SamSiteAirControlModel::validateData(m_samSitesAirControl, m_samSites.size());

	json = readJsonDataFile("strategic-map-towns.json");
	for (auto& element : json->GetArray()) 
	{
		auto town = TownModel::deserialize(element);
		m_towns.insert(std::make_pair(town->townId, town));
	}
	
	loadStringRes("strings/strategic-map-town-names", m_townNames);
	loadStringRes("strings/strategic-map-town-name-locatives", m_townNameLocatives);

	json = readJsonDataFile("strategic-map-underground-sectors.json");
	for (auto& element : json->GetArray())
	{
		auto ugSector = UndergroundSectorModel::deserialize(element);
		m_undergroundSectors.push_back(ugSector);
	}
	UndergroundSectorModel::validateData(m_undergroundSectors);

	json = readJsonDataFile("strategic-map-traversibility-ratings.json");
	auto travRatingMap = TraversibilityMapping::deserialize(*json);

	json = readJsonDataFile("strategic-map-movement-costs.json");
	m_movementCosts = MovementCostsModel::deserialize(*json, travRatingMap);

	json = readJsonDataFile("strategic-map-sectors-descriptions.json");
	m_sectorLandTypes = SectorLandTypes::deserialize(*json, travRatingMap);

	json = readJsonDataFile("strategic-map-secrets.json");
	for (auto& element : json->GetArray())
	{
		auto secret = StrategicMapSecretModel::deserialize(element, travRatingMap);
		m_mapSecrets.push_back(secret);
	}
	StrategicMapSecretModel::validateData(m_mapSecrets, m_samSites);

	json = readJsonDataFile("strategic-map-npc-placements.json");
	for (auto& element : json->GetArray())
	{
		auto placement = NpcPlacementModel::deserialize(element);
		m_npcPlacements.insert(std::make_pair(placement->profileId, placement));
	}

	CreatureLairModel::validateData(m_creatureLairs, m_undergroundSectors, m_mines.size());

	json = readJsonDataFile("strategic-map-cache-sectors.json");
	m_cacheSectors = CacheSectorsModel::deserialize(*json);

	return true;
}

bool DefaultContentManager::loadTacticalLayerData() 
{
	auto json = readJsonDataFile("tactical-npc-action-params.json");
	for (auto& element : json->GetArray())
	{
		auto params = NpcActionParamsModel::deserialize(element);
		m_npcActionParams[params->actionCode] = params;
	}

	return true;
}

bool DefaultContentManager::loadMercsData()
{
	auto json = readJsonDataFile("mercs-rpc-small-faces.json");
	for (auto& element : json->GetArray())
	{
		auto face = RPCSmallFaceModel::deserialize(element);
		m_rpcSmallFaces[face->ubProfileID] = face;
	}

	json = readJsonDataFile("mercs-MERC-listings.json");
	int i = 0;
	for (auto& element : json->GetArray())
	{
		auto item = MERCListingModel::deserialize(i++, element);
		m_MERCListings.push_back(item);
	}
	MERCListingModel::validateData(m_MERCListings);

	MercProfileInfo::load = [=](uint8_t p) { return this->getMercProfileInfo(p); };
	json = readJsonDataFile("mercs-profile-info.json");
	for (auto& element : json->GetArray())
	{
		auto profileInfo = MercProfileInfo::deserialize(element);
		ProfileID profileID = profileInfo->profileID;
		m_mercProfileInfo[profileID] = profileInfo;
		m_mercProfiles.push_back(new MercProfile(profileID));
	}
	MercProfileInfo::validateData(m_mercProfileInfo);

	return true;
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

const MineModel* DefaultContentManager::getMineForSector(uint8_t sectorX, uint8_t sectorY, uint8_t sectorZ) const
{
	auto sectorId = SECTOR(sectorX, sectorY);
	for (auto m : m_mines)
	{
		if (sectorZ == 0 && sectorId == m->entranceSector) return m;
		for (auto s : m->mineSectors)
		{
			if (s[0] == sectorId && s[1] == sectorZ) return m;
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

const int8_t DefaultContentManager::findSamIDBySector(uint8_t sectorId) const
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

const int8_t DefaultContentManager::getControllingSamSite(uint8_t sectorId) const
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
		SLOGD(ST::format("Town name not defined for index {}", townId));
		return ST::null;
	}
	return *m_townNames[townId];
}

const ST::string DefaultContentManager::getTownLocative(uint8_t townId) const
{
	if (townId >= m_townNameLocatives.size()) {
		SLOGD(ST::format("Town name locative not defined for index {}", townId));
		return ST::null;
	}
	return *m_townNameLocatives[townId];
}

const std::vector<const UndergroundSectorModel*>& DefaultContentManager::getUndergroundSectors() const
{
	return m_undergroundSectors;
}

const MovementCostsModel* DefaultContentManager::getMovementCosts() const
{
	return m_movementCosts;
}

int16_t DefaultContentManager::getSectorLandType(uint8_t const sectorID, uint8_t const sectorLevel) const
{
	SectorKey key(sectorID, sectorLevel);
	if (m_sectorLandTypes.find(key) == m_sectorLandTypes.end())
	{
		return -1;
	}
	return m_sectorLandTypes.at(key);
}

const CacheSectorsModel* DefaultContentManager::getCacheSectors() const
{
	return m_cacheSectors;
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
	
	SLOGD(ST::format("MercProfileInfo is not defined at {}", profileID));
	return &EMPTY_MERC_PROFILE_INFO;
}

const std::vector<const MercProfile*>& DefaultContentManager::listMercProfiles() const
{
	return m_mercProfiles;
}

const LoadingScreen* DefaultContentManager::getLoadingScreenForSector(uint8_t sectorId, uint8_t sectorLevel, bool isNight) const
{
	return m_loadingScreenModel->getScreenForSector(sectorId, sectorLevel, isNight);
}

const LoadingScreen* DefaultContentManager::getLoadingScreen(uint8_t index) const
{
	return m_loadingScreenModel->getByIndex(index);
}
