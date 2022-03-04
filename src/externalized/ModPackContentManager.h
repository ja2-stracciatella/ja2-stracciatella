#pragma once

#include <string_theory/string>

#include <map>
#include <vector>

#include "DefaultContentManager.h"

class ModPackContentManager : public DefaultContentManager
{
public:
	ModPackContentManager(RustPointer<EngineOptions> engineOptions);

	virtual ~ModPackContentManager() override;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const ST::string& filename, int quote_number) override;

protected:
	void logConfiguration() const override;

	// list of enabled mods
	std::vector<ST::string> m_modNames;

	std::map<ST::string, std::vector<ST::string> > m_dialogQuotesMap;

	// locate the directory of the mod and add to VFS
	void loadMod(const ST::string modName);
};
