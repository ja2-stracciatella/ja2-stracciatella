#include "DefaultContentManagerUT.h"

#include "DefaultContentManager.h"
#include "ItemStrings.h"
#include "FileMan.h"
#include "TestUtils.h"
#include <utility>

DefaultContentManagerUT::DefaultContentManagerUT(RustPointer<EngineOptions> engineOptions)
	: DefaultContentManager(std::move(engineOptions))
{
}

DefaultContentManagerUT* DefaultContentManagerUT::createDefaultCMForTesting()
{
	RustPointer<EngineOptions> engineOptions(EngineOptions_default());
	ST::string extraDataDir = GetExtraDataDir();
	ST::string gameResRootPath = FileMan::joinPaths(extraDataDir, "unittests");

	EngineOptions_setVanillaGameDir(engineOptions.get(), gameResRootPath.c_str());

	return new DefaultContentManagerUT(std::move(engineOptions));
}

bool DefaultContentManagerUT::loadGameData()
{
	return DefaultContentManager::loadGameData(BinaryData{});
}
