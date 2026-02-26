#include "DefaultContentManagerUT.h"

#include "BinaryProfileData.h"
#include "DefaultContentManager.h"
#include "FileMan.h"
#include "TestUtils.h"
#include "TranslatableString.h"
#include <utility>


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
	auto loader = TranslatableString::Unittests::TestLoader();
	return DefaultContentManager::loadGameData(loader, BinaryProfileData());
}
