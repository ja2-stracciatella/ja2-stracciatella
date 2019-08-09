#include "DefaultContentManager.h"

#include <memory>
#include <stdexcept>

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
#include "sgp/UTF8String.h"

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

#include "Logger.h"

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

static void LoadEncryptedData(STRING_ENC_TYPE encType, SGPFile* const File, wchar_t* DestString, UINT32 const seek_chars, UINT32 const read_chars)
{
	FileSeek(File, seek_chars * 2, FILE_SEEK_FROM_START);

	UINT16 *Str = MALLOCN(UINT16, read_chars);
	FileRead(File, Str, sizeof(UINT16) * read_chars);

	Str[read_chars - 1] = '\0';
	for (const UINT16* i = Str; *i != '\0'; ++i)
	{
		/* "Decrypt" the ROT-1 "encrypted" data */
		wchar_t c = (*i > 33 ? *i - 1 : *i);

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

		*DestString++ = c;
	}
	*DestString = L'\0';
	MemFree(Str);
}

DefaultContentManager::DefaultContentManager(GameVersion gameVersion,
						const std::string &configFolder,
						const std::string &gameResRootPath,
						const std::string &externalizedDataPath
	)
	:m_gameVersion(gameVersion),
	mNormalGunChoice(ARMY_GUN_LEVELS),
	mExtendedGunChoice(ARMY_GUN_LEVELS),
	m_dealersInventory(NUM_ARMS_DEALERS)
{
	/*
	 * Searching actual paths to directories 'Data' and 'Data/Tilecache', 'Data/Maps'
	 * On case-sensitive filesystems that might be tricky: if such directories
	 * exist we should use them.  If doesn't exist, then use lowercased names.
	 */

	m_configFolder = configFolder;
	m_gameResRootPath = gameResRootPath;
	m_externalizedDataPath = externalizedDataPath;

	m_dataDir = FileMan::joinPaths(gameResRootPath, BASEDATADIR);
	m_tileDir = FileMan::joinPaths(m_dataDir, TILECACHEDIR);


#if CASE_SENSITIVE_FS

	// need to find precise names of the directories

	std::string name;
	if(FileMan::findObjectCaseInsensitive(m_gameResRootPath.c_str(), BASEDATADIR, false, true, name))
	{
		m_dataDir = FileMan::joinPaths(m_gameResRootPath, name);
	}

	if(FileMan::findObjectCaseInsensitive(m_dataDir.c_str(), TILECACHEDIR, false, true, name))
	{
		m_tileDir = FileMan::joinPaths(m_dataDir, name);
	}
#endif

	m_libraryDB = LibraryDB_New();

	m_bobbyRayNewInventory = NULL;
	m_bobbyRayUsedInventory = NULL;
	m_impPolicy = NULL;
	m_gamePolicy = NULL;
}

/** Get list of game resources. */
std::vector<std::string> DefaultContentManager::getListOfGameResources() const
{
	std::vector<std::string> libraries = GetResourceLibraries(m_dataDir);
	return libraries;
}

void DefaultContentManager::initGameResouces(const std::string &stracciatellaHomeDir, const std::vector<std::string> &libraries)
{
	for (auto it = libraries.begin(); it != libraries.end(); ++it)
	{
		if (!LibraryDB_AddLibrary(m_libraryDB, m_dataDir.c_str(), it->c_str()))
		{
			std::string message = FormattedString(
				"Library '%s' is not found in folder '%s'.\n\nPlease make sure that '%s' contains files of the original game.  You can change this path by editing file '%s/ja2.json'.\n",
				it->c_str(), m_dataDir.c_str(), m_gameResRootPath.c_str(), stracciatellaHomeDir.c_str());
			throw LibraryFileNotFoundException(message);
		}
	}
}

DefaultContentManager::~DefaultContentManager()
{
	if(m_libraryDB)
	{
		LibraryDB_Delete(m_libraryDB);
		m_libraryDB = nullptr;
	}

	for (const ItemModel* item : m_items)
	{
		delete item;
	}
	m_items.clear();
	m_magazineMap.clear();
	m_magazines.clear();
	m_weaponMap.clear();
	m_itemMap.clear();

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

	for (const UTF8String *str : m_newStrings)
	{
		delete str;
	}
	for (const UTF8String *str : m_calibreNames)
	{
		delete str;
	}
	for (const UTF8String *str : m_calibreNamesBobbyRay)
	{
		delete str;
	}
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
std::string DefaultContentManager::getMapPath(const char *mapName) const
{
	std::string result = MAPSDIR;
	result += "/";
	result += mapName;

	SLOGD("map file %s", result.c_str());

	return result;
}

/** Get radar map resource name. */
std::string DefaultContentManager::getRadarMapResourceName(const std::string &mapName) const
{
	std::string result = RADARMAPSDIR;
	result += "/";
	result += mapName;

	SLOGD("map file %s", result.c_str());

	return result;
}

/** Get tileset resource name. */
std::string DefaultContentManager::getTilesetResourceName(int number, std::string fileName) const
{
	return FormattedString("%s/%d/%s", TILESETSDIR, number, fileName.c_str());
}


/** Get tileset db resource name. */
std::string DefaultContentManager::getTilesetDBResName() const
{
	return BINARYDATADIR "/ja2set.dat";
}

std::string DefaultContentManager::getMapPath(const wchar_t *mapName) const
{
	SLOGW("converting wchar to char");

	// This will not work for non-latin names.
	// But it is just a hack to make the code compile.
	// XXX: This method should be removed altogether

	UTF8String str(mapName);
	return getMapPath(str.getUTF8());
}

/** Open map for reading. */
SGPFile* DefaultContentManager::openMapForReading(const std::string& mapName) const
{
	return openGameResForReading(getMapPath(mapName.c_str()));
}

SGPFile* DefaultContentManager::openMapForReading(const wchar_t *mapName) const
{
	return openGameResForReading(getMapPath(mapName));
}

/** Get directory for storing new map file. */
std::string DefaultContentManager::getNewMapFolder() const
{
	return FileMan::joinPaths(m_dataDir, MAPSDIR);
}

/** Get all available maps. */
std::vector<std::string> DefaultContentManager::getAllMaps() const
{
	return FindFilesInDir(MAPSDIR, ".dat", true, true, true);
}

/** Get all available tilecache. */
std::vector<std::string> DefaultContentManager::getAllTilecache() const
{
	return FindFilesInDir(m_tileDir, ".jsd", true, false);
}

/** Open temporary file for writing. */
SGPFile* DefaultContentManager::openTempFileForWriting(const char* filename, bool truncate) const
{
	std::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	return FileMan::openForWriting(path.c_str(), truncate);
}

/** Open temporary file for appending. */
SGPFile* DefaultContentManager::openTempFileForAppend(const char* filename) const
{
	std::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
	return FileMan::openForAppend(path.c_str());
}

/* Open temporary file for reading. */
SGPFile* DefaultContentManager::openTempFileForReading(const char* filename) const
{
	std::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);

	int         mode;
	const char* fmode = GetFileOpenModeForReading(&mode);

	int d = FileMan::openFileForReading(path.c_str(), mode);
	return FileMan::getSGPFileFromFD(d, path.c_str(), fmode);
}

/** Delete temporary file. */
void DefaultContentManager::deleteTempFile(const char* filename) const
{
	std::string path = FileMan::joinPaths(NEW_TEMP_DIR, filename);
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
	int         mode;
	const char* fmode = GetFileOpenModeForReading(&mode);

	int d = FileMan::openFileCaseInsensitive(m_externalizedDataPath, filename, mode);
	if (d >= 0)
	{
		return FileMan::getSGPFileFromFD(d, filename, fmode);
	}
	else
	{
		d = FileMan::openFileForReading(filename, mode);
		if (d < 0)
		{
			// failed to open file in the local directory
			// let's try Data
			d = FileMan::openFileCaseInsensitive(m_dataDir, filename, mode);
			if (d < 0)
			{
				// failed to open in the data dir
				// let's try libraries

				LibraryFile* libFile = LibraryFile_Open(m_libraryDB, filename);
				if (libFile)
				{
					SLOGD("Opened file (from library ): %s", filename);
					SGPFile *file = MALLOCZ(SGPFile);
					file->flags = SGPFILE_NONE;
					file->u.lib = libFile;
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

	return FileMan::getSGPFileFromFD(d, filename, fmode);
}

/** Open user's private file (e.g. saved game, settings) for reading. */
SGPFile* DefaultContentManager::openUserPrivateFileForReading(const std::string& filename) const
{
	int         mode;
	const char* fmode = GetFileOpenModeForReading(&mode);

	int d = FileMan::openFileForReading(filename.c_str(), mode);
	return FileMan::getSGPFileFromFD(d, filename.c_str(), fmode);
}

SGPFile* DefaultContentManager::openGameResForReading(const std::string& filename) const
{
	return openGameResForReading(filename.c_str());
}

/* Checks if a game resource exists. */
bool DefaultContentManager::doesGameResExists(char const* filename) const
{
	if(FileMan::checkFileExistance(m_externalizedDataPath.c_str(), filename))
	{
		return true;
	}
	else
	{
		FILE* file = fopen(filename, "rb");
		if (!file)
		{
			char path[512];
			snprintf(path, lengthof(path), "%s/%s", m_dataDir.c_str(), filename);
			file = fopen(path, "rb");
			if (!file)
			{
				LibraryFile* libFile = LibraryFile_Open(m_libraryDB, filename);
				if (libFile)
				{
					LibraryFile_Close(libFile);
					return true;
				}
				return false;
			}
		}

		fclose(file);
		return true;
	}
}

bool DefaultContentManager::doesGameResExists(const std::string &filename) const
{
	return doesGameResExists(filename.c_str());
}

std::string DefaultContentManager::getScreenshotFolder() const
{
	return m_configFolder;
}

std::string DefaultContentManager::getVideoCaptureFolder() const
{
	return m_configFolder;
}

/** Get folder for saved games. */
std::string DefaultContentManager::getSavedGamesFolder() const
{
	return FileMan::joinPaths(m_configFolder, "SavedGames");
}

/** Load encrypted string from game resource file. */
void DefaultContentManager::loadEncryptedString(const char *fileName, wchar_t* DestString, uint32_t seek_chars, uint32_t read_chars) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	loadEncryptedString(File, DestString, seek_chars, read_chars);
}

void DefaultContentManager::loadEncryptedString(SGPFile* const File, wchar_t* DestString, uint32_t const seek_chars, uint32_t const read_chars) const
{
	LoadEncryptedData(getStringEncType(), File, DestString, seek_chars, read_chars);
}

/** Load dialogue quote from file. */
UTF8String* DefaultContentManager::loadDialogQuoteFromFile(const char* fileName, int quote_number)
{
	AutoSGPFile File(openGameResForReading(fileName));

	wchar_t quote[DIALOGUESIZE];
	LoadEncryptedData(getStringEncType(), File, quote, quote_number * DIALOGUESIZE, DIALOGUESIZE);
	return new UTF8String(quote);
}

/** Load all dialogue quotes for a character. */
void DefaultContentManager::loadAllDialogQuotes(STRING_ENC_TYPE encType, const char* fileName, std::vector<UTF8String*> &quotes) const
{
	AutoSGPFile File(openGameResForReading(fileName));
	uint32_t fileSize = FileGetSize(File);
	uint32_t numQuotes = fileSize / DIALOGUESIZE / 2;
	// SLOGI("%d quotes in dialog %s", numQuotes, fileName);
	for(int i = 0; i < numQuotes; i++)
	{
		wchar_t quote[DIALOGUESIZE];
		LoadEncryptedData(encType, File, quote, i * DIALOGUESIZE, DIALOGUESIZE);
		quotes.push_back(new UTF8String(quote));
	}
}

/** Get weapons with the give index. */
const WeaponModel* DefaultContentManager::getWeapon(uint16_t itemIndex)
{
	return getItem(itemIndex)->asWeapon();
}

const WeaponModel* DefaultContentManager::getWeaponByName(const std::string &internalName)
{
	std::map<std::string, const WeaponModel*>::const_iterator it = m_weaponMap.find(internalName);
	if(it == m_weaponMap.end())
	{
		SLOGE("weapon '%s' is not found", internalName.c_str());
		throw std::runtime_error(FormattedString("weapon '%s' is not found", internalName.c_str()));
	}
	return it->second;//m_weaponMap[internalName];
}

const MagazineModel* DefaultContentManager::getMagazineByName(const std::string &internalName)
{
	if(m_magazineMap.find(internalName) == m_magazineMap.end())
	{
		SLOGE("magazine '%s' is not found", internalName.c_str());
		throw std::runtime_error(FormattedString("magazine '%s' is not found", internalName.c_str()));
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

const UTF8String* DefaultContentManager::getCalibreName(uint8_t index) const
{
	return m_calibreNames[index];
}

const UTF8String* DefaultContentManager::getCalibreNameForBobbyRay(uint8_t index) const
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
	std::string jsonData = FileMan::fileReadText(f);

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
	std::string jsonData = FileMan::fileReadText(f);

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
	std::string jsonData = FileMan::fileReadText(f);

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
		m_calibreMap.insert(std::make_pair(std::string(calibre->internalName), calibre));
	}

	return true;
}

bool DefaultContentManager::loadAmmoTypes()
{
	AutoSGPFile f(openGameResForReading("ammo_types.json"));
	std::string jsonData = FileMan::fileReadText(f);

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
		m_ammoTypeMap.insert(std::make_pair(std::string(ammoType->internalName), ammoType));
	}

	return true;
}

bool DefaultContentManager::loadMusicModeList(const MusicMode mode, rapidjson::Value &array)
{
	std::vector<const UTF8String*>* musicModeList = new std::vector<const UTF8String*>();

	std::vector<std::string> utf8_encoded;
	JsonUtility::parseListStrings(array, utf8_encoded);
	for (const std::string &str : utf8_encoded)
	{
		musicModeList->push_back(new UTF8String(str.c_str()));
		SLOGD("Loaded music %s", str.c_str());
	}

	m_musicMap[mode] = musicModeList;

	return true;
}

bool DefaultContentManager::loadMusic()
{
	AutoSGPFile f(openGameResForReading("music.json"));
	std::string jsonData = FileMan::fileReadText(f);

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
	std::string jsonData = FileMan::fileReadText(f);

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
			std::vector<std::string> weaponNames;
			if(JsonUtility::parseListStrings(a[i], weaponNames))
			{
				for (const std::string &weapon : weaponNames)
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

void DefaultContentManager::loadStringRes(const char *name, std::vector<const UTF8String*> &strings) const
{
	std::string fullName(name);

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
		throw std::runtime_error(FormattedString("unknown game version %d", m_gameVersion));
	}
	}

	fullName += ".json";
	std::shared_ptr<rapidjson::Document> json(readJsonDataFile(fullName.c_str()));
	std::vector<std::string> utf8_encoded;
	JsonUtility::parseListStrings(*json, utf8_encoded);
	for (const std::string &str : utf8_encoded)
	{
		strings.push_back(new UTF8String(str.c_str()));
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

	loadAllDealersInventory();

	std::shared_ptr<rapidjson::Document> game_json(readJsonDataFile("game.json"));
	m_gamePolicy = new DefaultGamePolicy(game_json.get());

	std::shared_ptr<rapidjson::Document> imp_json(readJsonDataFile("imp.json"));
	m_impPolicy = new DefaultIMPPolicy(imp_json.get(), this);

	loadStringRes("strings/ammo-calibre", m_calibreNames);
	loadStringRes("strings/ammo-calibre-bobbyray", m_calibreNamesBobbyRay);

	loadStringRes("strings/new-strings", m_newStrings);

	return result;
}

rapidjson::Document* DefaultContentManager::readJsonDataFile(const char *fileName) const
{
	AutoSGPFile f(openGameResForReading(fileName));
	std::string jsonData = FileMan::fileReadText(f);

	rapidjson::Document *document = new rapidjson::Document();
	if (document->Parse<rapidjson::kParseCommentsFlag>(jsonData.c_str()).HasParseError())
	{
		SLOGE("Failed to parse '%s'", fileName);
		delete document;
		throw std::runtime_error(FormattedString("Failed to parse '%s'", fileName));
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
	return m_items[itemIndex];
}

const ItemModel* DefaultContentManager::getItemByName(const std::string &internalName) const
{
	std::map<std::string, const ItemModel*>::const_iterator it = m_itemMap.find(internalName);
	if(it == m_itemMap.end())
	{
		SLOGE("item '%s' is not found", internalName.c_str());
		throw std::runtime_error(FormattedString("item '%s' is not found", internalName.c_str()));
	}
	return it->second;
}

const DealerInventory* DefaultContentManager::getDealerInventory(int dealerId) const
{
	return m_dealersInventory[dealerId];
}

const UTF8String* DefaultContentManager::getMusicForMode(MusicMode mode) const {
	const uint32_t index = Random((uint32_t)m_musicMap.find(mode)->second->size());
	const UTF8String* chosen = m_musicMap.find(mode)->second->at(index);

	SLOGD("Choosing music index %d of %d for: '%s'", index, m_musicMap.find(mode)->second->size(), chosen->getUTF8());
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

const UTF8String* DefaultContentManager::getNewString(int stringId) const
{
	if(stringId >= m_newStrings.size())
	{
		SLOGE("new string %d is not found", stringId);
		throw std::runtime_error(FormattedString("new string %d is not found", stringId));
	}
	else
	{
		return m_newStrings[stringId];
	}
}
