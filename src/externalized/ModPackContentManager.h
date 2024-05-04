#pragma once

#include "DefaultContentManager.h"
#include <stdint.h>
#include <map>
#include <vector>
#include <string_theory/string>


class ModPackContentManager : public DefaultContentManager
{
public:
	ModPackContentManager(RustPointer<EngineOptions> engineOptions);

	~ModPackContentManager() override = default;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const ST::string& filename, int quote_number) override;

	IEDT::uptr openEDT(std::string_view filename, IEDT::column_list columns) const override;

protected:
	void logConfiguration() const override;

	// list of enabled mods
	std::vector<ST::string> m_modNames;

	std::map<ST::string, std::vector<ST::string> > m_dialogQuotesMap;

	// locate the directory of the mod and add to VFS
	void loadMod(const ST::string modName);
};
