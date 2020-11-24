#pragma once

#include "GameRes.h"
#include "externalized/DefaultContentManager.h"

struct EngineOptions;

class DefaultContentManagerUT : public DefaultContentManager
{
public:
	DefaultContentManagerUT(GameVersion gameVersion, const ST::string& configFolder, const ST::string& gameResRootPath, const ST::string& externalizedDataPath);
	
	virtual void init(EngineOptions*);

	// expose this method to unit tests
	std::unique_ptr<rapidjson::Document> _readJsonDataFile(const char* fileName) const;

	/** Create DefaultContentManager for usage in unit testing. */
	static DefaultContentManagerUT* createDefaultCMForTesting();
};


