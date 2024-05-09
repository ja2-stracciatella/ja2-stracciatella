#include "ModPackContentManager.h"

#include "JsonUtility.h"

#include "Logger.h"
#include <utility>

ModPackContentManager::ModPackContentManager(RustPointer<EngineOptions> engineOptions)
	:DefaultContentManager(std::move(engineOptions))
{
	uint32_t nMods = EngineOptions_getModsLength(m_engineOptions.get());
	std::vector<ST::string> modNames;
	for (uint32_t i = 0; i < nMods; ++i)
	{
		RustPointer<char> modName(EngineOptions_getMod(m_engineOptions.get(), i));
		modNames.emplace_back(modName.get());
	}
	m_modNames = modNames;
}

void ModPackContentManager::logConfiguration() const {
	ST::string joinedModList;
    for(const auto &s : m_modNames) {
        if(!joinedModList.empty())
            joinedModList += ", ";
        joinedModList += s;
    }
	DefaultContentManager::logConfiguration();
	SLOGI("Enabled mods                    '{}'", joinedModList);
}

/** Load dialogue quote from file. */
ST::string ModPackContentManager::loadDialogQuoteFromFile(const ST::string& filename, unsigned quote_number)
{
	ST::string jsonFileName = ST::string(filename) + ".json";
	std::map<ST::string, std::vector<ST::string> >::iterator it = m_dialogQuotesMap.find(jsonFileName);
	if(it != m_dialogQuotesMap.end())
	{
		SLOGD("cached quote {} {}", quote_number, jsonFileName);
		return it->second[quote_number];
	}
	else
	{
		if(doesGameResExists(jsonFileName))
		{
			AutoSGPFile f(openGameResForReading(jsonFileName));
			ST::string jsonQuotes = f->readStringToEnd();
			std::vector<ST::string> quotes;
			JsonUtility::parseJsonToListStrings(jsonQuotes.c_str(), quotes);
			m_dialogQuotesMap[jsonFileName] = quotes;
			return quotes[quote_number];
		}
		else
		{
			return DefaultContentManager::loadDialogQuoteFromFile(filename, quote_number);
		}
	}
}
