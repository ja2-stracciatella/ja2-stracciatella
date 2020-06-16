#include "DefaultContentManagerUT.h"

#include "sgp/FileMan.h"
#include "externalized/TestUtils.h"

DefaultContentManagerUT::DefaultContentManagerUT(GameVersion gameVersion,
			const ST::string& configFolder,
			const ST::string& gameResRootPath,
			const ST::string& externalizedDataPath)
	: DefaultContentManager(gameVersion, configFolder, gameResRootPath, externalizedDataPath)
{}

void DefaultContentManagerUT::init()
{
	// externalized data is used in the tests
	if (!Vfs_addDir(m_vfs.get(), VFS_ORDER_STRACCIATELLA, m_externalizedDataPath.c_str()))
	{
		RustPointer<char> err{getRustError()};
		SLOGE(ST::format("DefaultContentManagerUT::init '{}': {}", m_externalizedDataPath, err.get()));
		throw std::runtime_error("Failed to add stracciatella dir");
	}
}

rapidjson::Document* DefaultContentManagerUT::_readJsonDataFile(const char* fileName) const
{
	return DefaultContentManager::readJsonDataFile(fileName);
}

DefaultContentManagerUT* DefaultContentManagerUT::createDefaultCMForTesting()
{
	ST::string extraDataDir = GetExtraDataDir();
	ST::string configFolderPath = FileMan::joinPaths(extraDataDir, "unittests");
	ST::string gameResRootPath = FileMan::joinPaths(extraDataDir, "unittests");
	ST::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

	DefaultContentManagerUT* cm = new DefaultContentManagerUT(GameVersion::ENGLISH,
					configFolderPath,
					gameResRootPath, externalizedDataPath);
	cm->init();

	return cm;
}
