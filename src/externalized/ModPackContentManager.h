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

	virtual ~ModPackContentManager() override;

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(char const* fileName) const override;

	virtual SGPFile* openGameResForReading(const char* filename) const override;
	virtual SGPFile* openGameResForReading(const std::string& filename) const override;

	/** Get folder for saved games. */
	virtual std::string getSavedGamesFolder() const override;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const char* filename, int quote_number) override;

protected:
	std::vector<std::string> m_modNames;
	std::vector<std::string> m_modResFolders;
	std::map<std::string, std::vector<std::string> > m_dialogQuotesMap;
};
