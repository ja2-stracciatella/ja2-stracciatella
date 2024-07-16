#pragma once

#include "externalized/DefaultContentManager.h"

struct EngineOptions;

class DefaultContentManagerUT : public DefaultContentManager
{
	DefaultContentManagerUT(RustPointer<EngineOptions> engineOptions);

public:
	/** Create DefaultContentManager for usage in unit testing. */
	static DefaultContentManagerUT* createDefaultCMForTesting();

	bool loadGameData() override;
};
