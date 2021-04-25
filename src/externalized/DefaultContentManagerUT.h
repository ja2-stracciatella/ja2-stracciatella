#pragma once

#include "GameRes.h"
#include "externalized/DefaultContentManager.h"

struct EngineOptions;

class DefaultContentManagerUT : public DefaultContentManager
{
public:
	DefaultContentManagerUT(RustPointer<EngineOptions> engineOptions);

	// expose this method to unit tests
	std::unique_ptr<rapidjson::Document> _readJsonDataFile(const char* fileName) const;

	/** Create DefaultContentManager for usage in unit testing. */
	static DefaultContentManagerUT* createDefaultCMForTesting();
};


