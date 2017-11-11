#include "ModPackContentManager.h"

#include "JsonUtility.h"
#include "sgp/FileMan.h"
#include "sgp/UTF8String.h"

#include "slog/slog.h"

#define DEBUG_PRINT_OPENING_FILES (1)

ModPackContentManager::ModPackContentManager(GameVersion gameVersion,
						const std::string &modName,
						const std::string &modResFolder,
						const std::string &configFolder,
						const std::string &gameResRootPath,
						const std::string &externalizedDataPath)
	:DefaultContentManager(gameVersion, configFolder, gameResRootPath, externalizedDataPath)
{
	m_modName = modName;
	m_modResFolder = modResFolder;
}

ModPackContentManager::~ModPackContentManager()
{
}

/* Checks if a game resource exists. */
bool ModPackContentManager::doesGameResExists(char const* fileName) const
{
	if(FileMan::checkFileExistance(m_modResFolder.c_str(), fileName))
	{
		return true;
	}
	else
	{
		return DefaultContentManager::doesGameResExists(fileName);
	}
}

/* Open a game resource file for reading.
 *
 * First trying to open resource in the mod's directory.
 * If not found, use the previous method. */
SGPFile* ModPackContentManager::openGameResForReading(const char* filename) const
{
	int mode;
	const char* fmode = GetFileOpenModeForReading(&mode);

	int d = FileMan::openFileCaseInsensitive(m_modResFolder, filename, mode);
	if (d < 0)
	{
		return DefaultContentManager::openGameResForReading(filename);
	}
	else
	{
		SLOGI(DEBUG_TAG_MODPACK, "opening mod's resource: %s", filename);
		return FileMan::getSGPFileFromFD(d, filename, fmode);
	}
}

SGPFile* ModPackContentManager::openGameResForReading(const std::string& filename) const
{
	return openGameResForReading(filename.c_str());
}

/** Get folder for saved games. */
std::string ModPackContentManager::getSavedGamesFolder() const
{
	std::string folderName = std::string("SavedGames-") + m_modName;
	return FileMan::joinPaths(m_configFolder, folderName);
}

/** Load dialogue quote from file. */
UTF8String* ModPackContentManager::loadDialogQuoteFromFile(const char* filename, int quote_number)
{
	std::string jsonFileName = std::string(filename) + ".json";
	std::map<std::string, std::vector<std::string> >::iterator it = m_dialogQuotesMap.find(jsonFileName);
	if(it != m_dialogQuotesMap.end())
	{
		SLOGD(DEBUG_TAG_MODPACK, "cached quote %d %s", quote_number, jsonFileName.c_str());
		return new UTF8String(it->second[quote_number].c_str());
	}
	else
	{
		if(doesGameResExists(jsonFileName.c_str()))
		{
			AutoSGPFile f(openGameResForReading(jsonFileName));
			std::string jsonQuotes = FileMan::fileReadText(f);
			std::vector<std::string> quotes;
			JsonUtility::parseJsonToListStrings(jsonQuotes.c_str(), quotes);
			m_dialogQuotesMap[jsonFileName] = quotes;
			return new UTF8String(quotes[quote_number].c_str());
		}
		else
		{
			return DefaultContentManager::loadDialogQuoteFromFile(filename, quote_number);
		}
	}
}
