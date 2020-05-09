#include "DefaultContentManager.h"

#include "game/Directories.h"
#include "game/Strategic/Strategic_Status.h"
#include "game/Tactical/Arms_Dealer.h"
#include "game/Tactical/Items.h"
#include "game/Tactical/Weapons.h"

// XXX: GameRes.h should be integrated to ContentManager
#include "game/GameRes.h"

// XXX
#include "game/GameState.h"

#include "sgp/FileMan.h"
#include "sgp/MemMan.h"
#include "sgp/StrUtils.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "ContentMusic.h"
#include "DealerInventory.h"
#include "JsonObject.h"
#include "JsonUtility.h"
#include "MagazineModel.h"
#include "RustInterface.h"
#include "WeaponModels.h"
#include "policy/DefaultGamePolicy.h"
#include "policy/DefaultIMPPolicy.h"
#include "strategic/BloodCatPlacementsModel.h"
#include "strategic/BloodCatSpawnsModel.h"
#include "strategic/SamSiteModel.h"
#include "strategic/TownModel.h"
#include "strategic/MovementCostsModel.h"
#include "strategic/NpcPlacementModel.h"
#include "tactical/MapItemReplacementModel.h"

#include "Logger.h"

#include <string_theory/format>
#include <string_theory/string>

#include <memory>
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
						const ST::string &configFolder,
						const ST::string &gameResRootPath,
						const ST::string &externalizedDataPath
	)
	:m_gameVersion(gameVersion),
	mNormalGunChoice(ARMY_GUN_LEVELS),
	mExtendedGunChoice(ARMY_GUN_LEVELS),
	m_dealersInventory(NUM_ARMS_DEALERS),
	m_libraryDB(LibraryDB_create())
{
	/*
	 * Searching actual paths to directories 'Data' and 'Data/Tilecache', 'Data/Maps'
	 * On case-sensitive filesystems that might be tricky: if such directories
	 * exist we should use them.  If doesn't exist, then use lowercased names.
	 */

	m_configFolder = configFolder;
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

	m_movementCosts = NULL;
}

/** Get list of game resources. */
std::vector<ST::string> DefaultContentManager::getListOfGameResources() const
{
	std::vector<ST::string> libraries = GetResourceLibraries(m_dataDir);
	return libraries;
}

void DefaultContentManager::initGameResouces(const ST::string &stracciatellaHomeDir, const std::vector<ST::string> &libraries)
{
	for (auto it = libraries.begin(); it != libraries.end(); ++it)
	{
		if (!LibraryDB_push(m_libraryDB.get(), m_dataDir.c_str(), it->c_str()))
		{
			ST::string message = FormattedString(
				"Library '%s' is not found in folder '%s'.\n\nPlease make sure that '%s' contains files of the original game.  You can change this path by editing file '%s/ja2.json'.\n",
				it->c_str(), m_dataDir.c_str(), m_gameResRootPath.c_str(), stracciatellaHomeDir.c_str());
			throw LibraryFileNotFoundException(message);
		}
	}
}

void DefaultContentManager::addExtraResources(const ST::string &baseDir, const ST::string &library)
{
	if (!LibraryDB_push(m_libraryDB.get(), baseDir.c_str(), library.c_str())) {
		RustPointer<char> error(getRustError());
		ST::string message = FormattedString(
			"Library '%s' is not found in folder '%s': %s",
			library.c_str(), baseDir.c_str(), error.get());
		throw LibraryFileNotFoundException(message);
	}
}

DefaultContentManager::~DefaultContentManager()
{
	m_libraryDB.reset(nullptr);

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

	for(const DealerInventory* inv : m_dealersInventory)
	{
		if(inv) delete inv;
	}

	delete m_bobbyRayNewInventory;
	delete m_bobbyRayUsedInventory;
	delete m_impPolicy;
	delete m_gamePolicy;

	for (const ST::string *str : m_newStrings)
	{
		delete str;
	}
	for (const ST::string *str : m_calibreNames)
	{
		delete str;
	}
	for (const ST::string *str : m_calibreNamesBobbyRay)
	{
		delete str;
	}

	m_bloodCatPlacements.clear();
	m_bloodCatSpawns.clear();
	m_samSites.clear();
	m_towns.clear();
	m_npcPlacements.clear();

	delete m_movementCosts;
}

const DealerInventory* DefaultContentManager::getBobbyRayNewInventory() const
{
	return m_bobbyRayNewInventory;
}

const DealerInventory* DefaultContentManager::getBobbyRayUsedInventory() const
{
	return m_bobbyRayUsedInventory;
}

/** Get map file path. */
ST::string DefaultContentManager::getMapPath(const ST::string& mapName) const
{
	ST::string result = MAPSDIR;
	result += '/';
	result += mapName.c_str();

	SLOGD("map file %s", result.c_str());

	return result;
}

/** Get radar map resource name. */
ST::string DefaultContentManager::getRadarMapResourceName(const ST::string &mapName) const
{
	ST::string result = RADARMAPSDIR;
	result += "/";
	result += mapName;

	SLOGD("map file %s", result.c_str());

	return result;
}

/** Get tileset resource name. */
ST::string DefaultContentManager::getTilesetResourceName(int number, ST::string fileName) const
{
	return FormattedString("%s/%d/%s", TILESETSDIR, number, fileName.c_str());
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
SGPFile* DefaultContentManager::openTempFileForWriting(const char* filename, bool truncate) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	return FileMan::openForWriting(path, truncate);
}

/** Open temporary file for appending. */
SGPFile* DefaultContentManager::openTempFileForAppend(const char* filename) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	return FileMan::openForAppend(path);
}

/* Open temporary file for reading. */
SGPFile* DefaultContentManager::openTempFileForReading(const char* filename) const
{
	ST::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	RustPointer<File> file(File_open(path.c_str(), FILE_OPEN_READ));
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "DefaultContentManager::openTempFileForReading: %s", err.get());
		throw std::runtime_error(buf);
	}
	return FileMan::getSGPFileFromFile(file.release());
}

/** Delete temporary file. */
void DefaultContentManager::deleteTempFile(const char* filename) const
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
SGPFile* DefaultContentManager::openGameResForReading(const char* filename) const
{
	RustPointer<File> file = FileMan::openFileCaseInsensitive(m_externalizedDataPath, filename, FILE_OPEN_READ);
	if (file)
	{
		return FileMan::getSGPFileFromFile(file.release());
	}
	else
	{
		file = FileMan::openFileForReading(filename);
		if (!file)
		{
			// failed to open file in the local directory
			// let's try Data
			file = FileMan::openFileCaseInsensitive(m_dataDir, filename, FILE_OPEN_READ);
			if (!file)
			{
				// failed to open in the data dir
				// let's try libraries

				RustPointer<LibraryFile> libFile(LibraryFile_open(m_libraryDB.get(), filename));
				if (libFile)
				{
					SLOGD("Opened file (from library ): %s", filename);
					SGPFile *file = new SGPFile{};
					file->flags = SGPFILE_NONE;
					file->u.lib = libFile.release();
					return file;
				}
			}
			else
			{
				SLOGD("Opened file (from data dir): %s", filename);
			}
		}
		else
		{
			SLOGD("Opened file (current dir  ): %s", filename);
		}
	}
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "DefaultContentManager::openGameResForReading: %s", err.get());
		throw std::runtime_error(buf);
	}
	return FileMan::getSGPFileFromFile(file.release());
}

/** Open user's private file (e.g. saved game, settings) for reading. */
SGPFile* DefaultContentManager::openUserPrivateFileForReading(const ST::string& filename) const
{
	RustPointer<File> file = FileMan::openFileForReading(filename);
	if (!file)
	{
		RustPointer<char> err(getRustError());
		char buf[128];
		snprintf(buf, sizeof(buf), "DefaultContentManager::openUserPrivateFileForReading: %s", err.get());
		throw std::runtime_error(buf);
	}
	return FileMan::getSGPFileFromFile(file.release());
}

SGPFile* DefaultContentManager::openGameResForReading(const ST::string& filename) const
{
	return openGameResForReading(filename.c_str());
}

/* Checks if a game resource exists. */
bool DefaultContentManager::doesGameResExists(char const* filename) const
{
	if(FileMan::checkFileExistance(m_externalizedDataPath, filename))
	{
		return true;
	}
	else
	{
		RustPointer<File> file(File_open(filename, FILE_OPEN_READ));
		if (!file)
		{
			char path[512];
			snprintf(path, lengthof(path), "%s/%s", m_dataDir.c_str(), filename);
			file.reset(File_open(path, FILE_OPEN_READ));
			if (!file)
			{
				RustPointer<LibraryFile> libFile(LibraryFile_open(m_libraryDB.get(), filename));
				return static_cast<bool>(libFile);
			}
		}

		return true;
	}
}

bool DefaultContentManager::doesGameResExists(const ST::string &filename) const
{
	return doesGameResExists(filename.c_str());
}

ST::string DefaultContentManager::getScreenshotFolder() const
{
	return m_configFolder;
}

ST::string DefaultContentManager::getVideoCaptureFolder() const
{
	return m_configFolder;
}

/** Get folder for saved games. */
ST::string DefaultContentManager::getSavedGamesFolder() const
{
	return FileMan::joinPaths(m_configFolder, "SavedGames");
}

/** Load encrypted string from game resource file. */
ST::string DefaultContentManager::loadEncryptedString(const char* fileName, uint32_t seek_chars, uint32_t read_chars) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	ST::string err_msg;
	ST::string str = LoadEncryptedData(err_msg, getStringEncType(), File, seek_chars, read_chars);
	if (!err_msg.empty())
	{
		SLOGW("DefaultContentManager::loadEncryptedString '%s' %u %u: %s", fileName, seek_chars, read_chars, err_msg.c_str());
	}
	return str;
}

ST::string DefaultContentManager::loadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars) const
{
	ST::string err_msg;
	ST::string str = LoadEncryptedData(err_msg, getStringEncType(), File, seek_chars, read_chars);
	if (!err_msg.empty())
	{
		SLOGW("DefaultContentManager::loadEncryptedString ? %u %u: %s", seek_chars, read_chars, err_msg.c_str());
	}
	return str;
}


/** Load dialogue quote from file. */
ST::string* DefaultContentManager::loadDialogQuoteFromFile(const char* fileName, int quote_number)
{
	AutoSGPFile File(openGameResForReading(fileName));
	ST::string err_msg;
	ST::string quote = LoadEncryptedData(err_msg, getStringEncType(), File, quote_number * DIALOGUESIZE, DIALOGUESIZE);
	if (!err_msg.empty())
	{
		SLOGW("DefaultContentManager::loadDialogQuoteFromFile '%s' %d: %s", fileName, quote_number, err_msg.c_str());
	}
	return new ST::string(quote);
}

/** Load all dialogue quotes for a character. */
void DefaultContentManager::loadAllDialogQuotes(STRING_ENC_TYPE encType, const char* fileName, std::vector<ST::string*> &quotes) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	uint32_t fileSize = FileGetSize(File);
	uint32_t numQuotes = fileSize / DIALOGUESIZE / 2;
	// SLOGI("%d quotes in dialog %s", numQuotes, fileName);
	for(uint32_t i = 0; i < numQuotes; i++)
	{
		ST::string err;
		ST::string quote = LoadEncryptedData(err, encType, File, i * DIALOGUESIZE, DIALOGUESIZE);
		if (!err.empty())
		{
			SLOGW("DefaultContentManager::loadAllDialogQuotes '%s' %d: %s", fileName, i, err.c_str());
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
		SLOGE("weapon '%s' is not found", internalName.c_str());
		throw std::runtime_error(FormattedString("weapon '%s' is not found", internalName.c_str()).to_std_string());
	}
	return it->second;//m_weaponMap[internalName];
}

const MagazineModel* DefaultContentManager::getMagazineByName(const ST::string &internalName)
{
	if(m_magazineMap.find(internalName) == m_magazineMap.end())
	{
		SLOGE("magazine '%s' is not found", internalName.c_str());
		throw std::runtime_error(FormattedString("magazine '%s' is not found", internalName.c_str()).to_std_string());
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
	AutoSGPFile f(openGameResForReading("weapons.json"));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document document;
	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse weapons.json");
		return false;
	}
	else
	{
		if(document.IsArray()) {
			const rapidjson::Value& a = document;
			for (rapidjson::SizeType i = 0; i < a.Size(); i++)
			{
				JsonObjectReader obj(a[i]);
				WeaponModel *w = WeaponModel::deserialize(obj, m_calibreMap);
				SLOGD("Loaded weapon %d %s", w->getItemIndex(), w->getInternalName().c_str());

				if((w->getItemIndex() < 0) || (w->getItemIndex() > MAX_WEAPONS))
				{
					SLOGE("Weapon index must be in the interval 0 - %d", MAX_WEAPONS);
					return false;
				}

				m_items[w->getItemIndex()] = w;
				m_weaponMap.insert(std::make_pair(w->getInternalName(), w));
			}
		}
	}

	return true;
}

bool DefaultContentManager::loadMagazines()
{
	AutoSGPFile f(openGameResForReading("magazines.json"));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document document;
	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse magazines.json");
		return false;
	}
	else
	{
		if(document.IsArray()) {
			const rapidjson::Value& a = document;
			for (rapidjson::SizeType i = 0; i < a.Size(); i++)
			{
				JsonObjectReader obj(a[i]);
				MagazineModel *mag = MagazineModel::deserialize(obj, m_calibreMap, m_ammoTypeMap);
				SLOGD("Loaded magazine %d %s", mag->getItemIndex(), mag->getInternalName().c_str());

				if((mag->getItemIndex() < FIRST_AMMO) || (mag->getItemIndex() > LAST_AMMO))
				{
					SLOGE("Magazine item index must be in the interval %d - %d", FIRST_AMMO, LAST_AMMO);
					return false;
				}

				m_magazines.push_back(mag);
				m_items[mag->getItemIndex()] = mag;
				m_magazineMap.insert(std::make_pair(mag->getInternalName(), mag));
			}
		}
	}

	return true;
}

bool DefaultContentManager::loadCalibres()
{
	AutoSGPFile f(openGameResForReading("calibres.json"));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document document;
	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse calibres.json");
		return false;
	}
	else
	{
		if(document.IsArray()) {
			const rapidjson::Value& a = document;
			for (rapidjson::SizeType i = 0; i < a.Size(); i++)
			{
				JsonObjectReader obj(a[i]);
				CalibreModel *calibre = CalibreModel::deserialize(obj);
				SLOGD("Loaded calibre %d %s", calibre->index, calibre->internalName.c_str());

				if(m_calibres.size() <= calibre->index)
				{
					m_calibres.resize(calibre->index + 1);
				}

				m_calibres[calibre->index] = calibre;
			}
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
	AutoSGPFile f(openGameResForReading("ammo_types.json"));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document document;
	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse ammo_types.json");
		return false;
	}
	else
	{
		if(document.IsArray()) {
			const rapidjson::Value& a = document;
			for (rapidjson::SizeType i = 0; i < a.Size(); i++)
			{
				JsonObjectReader obj(a[i]);
				AmmoTypeModel *ammoType = AmmoTypeModel::deserialize(obj);
				SLOGD("Loaded ammo type %d %s", ammoType->index, ammoType->internalName.c_str());

				if(m_ammoTypes.size() <= ammoType->index)
				{
					m_ammoTypes.resize(ammoType->index + 1);
				}

				m_ammoTypes[ammoType->index] = ammoType;
			}
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
		SLOGD("Loaded music %s", str.c_str());
	}

	m_musicMap[mode] = musicModeList;

	return true;
}

bool DefaultContentManager::loadMusic()
{
	AutoSGPFile f(openGameResForReading("music.json"));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document document;
	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError()) {
		SLOGE("Failed to parse music.json");
		return false;
	}
	if(!document.IsObject()) {
		SLOGE("music.json has wrong structure");
		return false;
	}

	SLOGD("Loading main_menu music");
	loadMusicModeList(MUSIC_MAIN_MENU, document["main_menu"]);
	SLOGD("Loading main_menu music");
	loadMusicModeList(MUSIC_LAPTOP, document["laptop"]);
	SLOGD("Loading tactical music");
	loadMusicModeList(MUSIC_TACTICAL_NOTHING, document["tactical"]);
	SLOGD("Loading tactical_enemypresent music");
	loadMusicModeList(MUSIC_TACTICAL_ENEMYPRESENT, document["tactical_enemypresent"]);
	SLOGD("Loading tactical_battle music");
	loadMusicModeList(MUSIC_TACTICAL_BATTLE, document["tactical_battle"]);
	SLOGD("Loading tactical_creature music");
	loadMusicModeList(MUSIC_TACTICAL_CREATURE_NOTHING, document["tactical_creature"]);
	SLOGD("Loading tactical_creature_enemypresent music");
	loadMusicModeList(MUSIC_TACTICAL_CREATURE_ENEMYPRESENT, document["tactical_creature_enemypresent"]);
	SLOGD("Loading tactical_creature_battle music");
	loadMusicModeList(MUSIC_TACTICAL_CREATURE_BATTLE, document["tactical_creature_battle"]);
	SLOGD("Loading tactical_victory music");
	loadMusicModeList(MUSIC_TACTICAL_VICTORY, document["tactical_victory"]);
	SLOGD("Loading tactical_defeat music");
	loadMusicModeList(MUSIC_TACTICAL_DEFEAT, document["tactical_defeat"]);

	return true;
}

bool DefaultContentManager::readWeaponTable(
	const char *fileName,
	std::vector<std::vector<const WeaponModel*> > & weaponTable)
{
	AutoSGPFile f(openGameResForReading(fileName));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document document;
	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse %s", fileName);
		return false;
	}

	if(document.IsArray())
	{
		const rapidjson::Value& a = document;
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

bool DefaultContentManager::loadArmyGunChoice()
{
	return readWeaponTable("army-gun-choice-normal.json", mNormalGunChoice)
		&& readWeaponTable("army-gun-choice-extended.json", mExtendedGunChoice);
}

void DefaultContentManager::loadStringRes(const char *name, std::vector<const ST::string*> &strings) const
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
		throw std::runtime_error(FormattedString("unknown game version %d", m_gameVersion).to_std_string());
	}
	}

	fullName += ".json";
	std::shared_ptr<rapidjson::Document> json(readJsonDataFile(fullName.c_str()));
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
		&& loadArmyGunChoice()
		&& loadMusic();

	for (const ItemModel *item : m_items)
	{
		m_itemMap.insert(std::make_pair(item->getInternalName(), item));
	}

	std::shared_ptr<rapidjson::Document> replacement_json(readJsonDataFile("tactical-map-item-replacements.json"));
	m_mapItemReplacements = MapItemReplacementModel::deserialize(replacement_json.get(), this);

	loadAllDealersInventory();

	std::shared_ptr<rapidjson::Document> game_json(readJsonDataFile("game.json"));
	m_gamePolicy = new DefaultGamePolicy(game_json.get());

	std::shared_ptr<rapidjson::Document> imp_json(readJsonDataFile("imp.json"));
	m_impPolicy = new DefaultIMPPolicy(imp_json.get(), this);

	loadStringRes("strings/ammo-calibre", m_calibreNames);
	loadStringRes("strings/ammo-calibre-bobbyray", m_calibreNamesBobbyRay);

	loadStringRes("strings/new-strings", m_newStrings);

	loadStrategicLayerData();

	return result;
}

rapidjson::Document* DefaultContentManager::readJsonDataFile(const char *fileName) const
{
	AutoSGPFile f(openGameResForReading(fileName));
	ST::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document *document = new rapidjson::Document();
	if (document->Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse '%s'", fileName);
		delete document;
		throw std::runtime_error(FormattedString("Failed to parse '%s'", fileName).to_std_string());
	}

	return document;
}

const DealerInventory * DefaultContentManager::loadDealerInventory(const char *fileName)
{
	std::shared_ptr<rapidjson::Document> json(readJsonDataFile(fileName));
	return new DealerInventory(json.get(), this);
}

bool DefaultContentManager::loadAllDealersInventory()
{
	m_dealersInventory[ARMS_DEALER_TONY]          = loadDealerInventory("dealer-inventory-tony.json");
	m_dealersInventory[ARMS_DEALER_FRANK]         = loadDealerInventory("dealer-inventory-frank.json");
	m_dealersInventory[ARMS_DEALER_MICKY]         = loadDealerInventory("dealer-inventory-micky.json");
	m_dealersInventory[ARMS_DEALER_ARNIE]         = loadDealerInventory("dealer-inventory-arnie.json");
	m_dealersInventory[ARMS_DEALER_PERKO]         = loadDealerInventory("dealer-inventory-perko.json");
	m_dealersInventory[ARMS_DEALER_KEITH]         = loadDealerInventory("dealer-inventory-keith.json");
	m_dealersInventory[ARMS_DEALER_BAR_BRO_1]     = loadDealerInventory("dealer-inventory-herve-santos.json");
	m_dealersInventory[ARMS_DEALER_BAR_BRO_2]     = loadDealerInventory("dealer-inventory-peter-santos.json");
	m_dealersInventory[ARMS_DEALER_BAR_BRO_3]     = loadDealerInventory("dealer-inventory-alberto-santos.json");
	m_dealersInventory[ARMS_DEALER_BAR_BRO_4]     = loadDealerInventory("dealer-inventory-carlo-santos.json");
	m_dealersInventory[ARMS_DEALER_JAKE]          = loadDealerInventory("dealer-inventory-jake.json");
	m_dealersInventory[ARMS_DEALER_FRANZ]         = loadDealerInventory("dealer-inventory-franz.json");
	m_dealersInventory[ARMS_DEALER_HOWARD]        = loadDealerInventory("dealer-inventory-howard.json");
	m_dealersInventory[ARMS_DEALER_SAM]           = loadDealerInventory("dealer-inventory-sam.json");
	m_dealersInventory[ARMS_DEALER_FREDO]         = loadDealerInventory("dealer-inventory-fredo.json");
	m_dealersInventory[ARMS_DEALER_GABBY]         = loadDealerInventory("dealer-inventory-gabby.json");
	m_dealersInventory[ARMS_DEALER_DEVIN]         = loadDealerInventory("dealer-inventory-devin.json");
	m_dealersInventory[ARMS_DEALER_ELGIN]         = loadDealerInventory("dealer-inventory-elgin.json");
	m_dealersInventory[ARMS_DEALER_MANNY]         = loadDealerInventory("dealer-inventory-manny.json");
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
		SLOGE("item '%s' is not found", internalName.c_str());
		throw std::runtime_error(FormattedString("item '%s' is not found", internalName.c_str()).to_std_string());
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

	SLOGD("Choosing music index %d of %d for: '%s'", index, m_musicMap.find(mode)->second->size(), chosen->c_str());
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


bool DefaultContentManager::loadStrategicLayerData() {
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

	json = readJsonDataFile("strategic-map-sam-sites.json");
	for (auto& element : json->GetArray())
	{
		auto samSite = SamSiteModel::deserialize(element);
		m_samSites.push_back(samSite);
	}
	SamSiteModel::validateData(m_samSites);
	delete json;

	json = readJsonDataFile("strategic-map-towns.json");
	for (auto& element : json->GetArray()) 
	{
		auto town = TownModel::deserialize(element);
		m_towns.insert(std::make_pair(town->townId, town));
	}
	
	loadStringRes("strings/strategic-map-town-names", m_townNames);
	loadStringRes("strings/strategic-map-town-name-locatives", m_townNameLocatives);

	json = readJsonDataFile("strategic-map-movement-costs.json");
	m_movementCosts = MovementCostsModel::deserialize(*json);

	json = readJsonDataFile("strategic-map-npc-placements.json");
	for (auto& element : json->GetArray())
	{
		auto placement = NpcPlacementModel::deserialize(element);
		m_npcPlacements.insert(std::make_pair(placement->profileId, placement));
	}

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
			return i;
		}
	}
	return -1;
}

const SamSiteModel* DefaultContentManager::findSamSiteBySector(uint8_t sectorId) const
{
	auto i = findSamIDBySector(sectorId);
	return (i > -1) ? m_samSites[i] : NULL;
}

const std::map<int8_t, const TownModel*>& DefaultContentManager::getTowns() const
{
	return m_towns;
}

const ST::string DefaultContentManager::getTownName(uint8_t townId) const
{
	if (townId >= m_townNames.size()) {
		SLOGD("Town name not defined for index %d", townId);
		return ST::null;
	}
	return *m_townNames[townId];
}

const ST::string DefaultContentManager::getTownLocative(uint8_t townId) const
{
	if (townId >= m_townNameLocatives.size()) {
		SLOGD("Town name locative not defined for index %d", townId);
		return ST::null;
	}
	return *m_townNameLocatives[townId];
}

const MovementCostsModel* DefaultContentManager::getMovementCosts() const
{
	return m_movementCosts;
}

const NpcPlacementModel* DefaultContentManager::getNpcPlacement(uint8_t profileId) const
{
	return m_npcPlacements.at(profileId);
}
