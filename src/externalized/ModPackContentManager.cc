#include "ModPackContentManager.h"

#include "JsonUtility.h"
#include "sgp/FileMan.h"

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

/* Checks if a game resource exists. */
bool ModPackContentManager::doesGameResExists(char const* fileName) const
{
	for (const auto& folder : m_modResFolders)
	{
		if (FileMan::checkFileExistance(folder.c_str(), fileName))
		{
			return true;
		}
	}
	return DefaultContentManager::doesGameResExists(fileName);
}

/* Open a game resource file for reading.
 *
 * First trying to open resource in the mod's directory.
 * If not found, use the previous method. */
SGPFile* ModPackContentManager::openGameResForReading(const char* filename) const
{
	for (const auto& folder : m_modResFolders)
	{
		RustPointer<File> file = FileMan::openFileCaseInsensitive(folder, filename, FILE_OPEN_READ);
		if (file) {
			SLOGI("opening mod's resource: %s", filename);
			return FileMan::getSGPFileFromFile(file.release());
		}
	}
	return DefaultContentManager::openGameResForReading(filename);
}

SGPFile* ModPackContentManager::openGameResForReading(const ST::string& filename) const
{
	return openGameResForReading(filename.c_str());
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
