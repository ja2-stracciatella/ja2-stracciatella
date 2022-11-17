#include "DefaultContentManagerUT.h"

#include "sgp/FileMan.h"
#include "externalized/TestUtils.h"
#include <utility>

DefaultContentManagerUT::DefaultContentManagerUT(RustPointer<EngineOptions> engineOptions)
	: DefaultContentManager(std::move(engineOptions))
{
}

std::unique_ptr<rapidjson::Document> DefaultContentManagerUT::_readJsonDataFile(const char* fileName) const
{
	return DefaultContentManager::readJsonDataFile(fileName);
}

DefaultContentManagerUT* DefaultContentManagerUT::createDefaultCMForTesting()
{
	RustPointer<EngineOptions> engineOptions(EngineOptions_default());
	ST::string extraDataDir = GetExtraDataDir();
	ST::string gameResRootPath = FileMan::joinPaths(extraDataDir, "unittests");

	EngineOptions_setVanillaGameDir(engineOptions.get(), gameResRootPath.c_str());

	DefaultContentManagerUT* cm = new DefaultContentManagerUT(std::move(engineOptions));

	return cm;
}
