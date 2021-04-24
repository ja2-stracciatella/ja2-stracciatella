#include "DefaultContentManagerUT.h"

#include "sgp/FileMan.h"
#include "externalized/TestUtils.h"

DefaultContentManagerUT::DefaultContentManagerUT(RustPointer<EngineOptions> engineOptions)
	: DefaultContentManager(move(engineOptions))
{}

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

	DefaultContentManagerUT* cm = new DefaultContentManagerUT(move(engineOptions));

	return cm;
}
