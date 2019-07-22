#pragma once

#include <map>
#include <string>
#include <vector>

#include "DefaultContentManager.h"

class ModPackContentManager : public DefaultContentManager
{
public:
	ModPackContentManager(GameVersion gameVersion,
				const std::vector<std::string> &modNames,
				const std::vector<std::string> &modResFolders,
				const std::string &configFolder,
				const std::string &gameResRootPath,
				const std::string &externalizedDataPath);

	virtual ~ModPackContentManager();

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(char const* fileName) const;

	virtual SGPFile* openGameResForReading(const char* filename) const;
	virtual SGPFile* openGameResForReading(const std::string& filename) const;

	/** Get folder for saved games. */
	std::string getSavedGamesFolder() const;

	/** Load dialogue quote from file. */
	virtual UTF8String* loadDialogQuoteFromFile(const char* filename, int quote_number);

protected:
	std::vector<std::string> m_modNames;
	std::vector<std::string> m_modResFolders;
	std::map<std::string, std::vector<std::string> > m_dialogQuotesMap;
};
