#pragma once

#include <string_theory/string>

#include <map>
#include <vector>

#include "DefaultContentManager.h"

class ModPackContentManager : public DefaultContentManager
{
public:
	ModPackContentManager(GameVersion gameVersion,
				const std::vector<ST::string> &modNames,
				const std::vector<ST::string> &modResFolders,
				const ST::string &configFolder,
				const ST::string &gameResRootPath,
				const ST::string &externalizedDataPath);

	virtual ~ModPackContentManager() override;

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(char const* fileName) const override;

	virtual SGPFile* openGameResForReading(const char* filename) const override;
	virtual SGPFile* openGameResForReading(const ST::string& filename) const override;

	/** Get folder for saved games. */
	virtual ST::string getSavedGamesFolder() const override;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const char* filename, int quote_number) override;

protected:
	std::vector<ST::string> m_modNames;
	std::vector<ST::string> m_modResFolders;
	std::map<ST::string, std::vector<ST::string> > m_dialogQuotesMap;
};
