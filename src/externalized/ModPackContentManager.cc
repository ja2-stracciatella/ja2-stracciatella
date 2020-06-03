#include "ModPackContentManager.h"

#include "JsonUtility.h"
#include "sgp/FileMan.h"

#include "Debug.h"
#include "Logger.h"

#define DEBUG_PRINT_OPENING_FILES (1)

ModPackContentManager::ModPackContentManager(GameVersion gameVersion,
						const std::vector<ST::string> &modNames,
						const std::vector<ST::string> &modResFolders,
						const ST::string &configFolder,
						const ST::string &gameResRootPath,
						const ST::string &externalizedDataPath)
	:DefaultContentManager(gameVersion, configFolder, gameResRootPath, externalizedDataPath)
{
	m_modNames = modNames;
	m_modResFolders = modResFolders;
}

ModPackContentManager::~ModPackContentManager()
{
}

void ModPackContentManager::init()
{
	for (const ST::string& dir : m_modResFolders)
	{
		SLOGI(ST::format("Vfs with mod dir ({}) '{}'", VFS_ORDER_MOD, dir));
		if (!Vfs_addDir(m_vfs.get(), VFS_ORDER_MOD, dir.c_str()))
		{
			RustPointer<char> err{getRustError()};
			SLOGE(ST::format("ModPackContentManager::init '{}': {}", dir, err.get()));
			throw std::runtime_error("Failed to add mod dir");
		}
	}
	DefaultContentManager::init();
}

/** Get folder for saved games. */
ST::string ModPackContentManager::getSavedGamesFolder() const
{
	ST::string folderName("SavedGames");
	for (const auto& name : m_modNames)
	{
		folderName += '-';
		folderName += name;
	}
	return FileMan::joinPaths(m_configFolder, folderName);
}

/** Load dialogue quote from file. */
ST::string* ModPackContentManager::loadDialogQuoteFromFile(const char* filename, int quote_number)
{
	ST::string jsonFileName = ST::string(filename) + ".json";
	std::map<ST::string, std::vector<ST::string> >::iterator it = m_dialogQuotesMap.find(jsonFileName);
	if(it != m_dialogQuotesMap.end())
	{
		SLOGD("cached quote %d %s", quote_number, jsonFileName.c_str());
		return new ST::string(it->second[quote_number].c_str());
	}
	else
	{
		if(doesGameResExists(jsonFileName.c_str()))
		{
			AutoSGPFile f(openGameResForReading(jsonFileName));
			ST::string jsonQuotes = FileMan::fileReadText(f);
			std::vector<ST::string> quotes;
			JsonUtility::parseJsonToListStrings(jsonQuotes.c_str(), quotes);
			m_dialogQuotesMap[jsonFileName] = quotes;
			return new ST::string(quotes[quote_number].c_str());
		}
		else
		{
			return DefaultContentManager::loadDialogQuoteFromFile(filename, quote_number);
		}
	}
}
