#include "DefaultContentManagerUT.h"

#include "GameRes.h"
#include "sgp/FileMan.h"

#include "externalized/DefaultContentManager.h"
#include "externalized/TestUtils.h"

class DefaultContentManagerUT : public DefaultContentManager
{
public:
	DefaultContentManagerUT(GameVersion gameVersion,
				const ST::string& configFolder,
				const ST::string& gameResRootPath,
				const ST::string& externalizedDataPath)
		: DefaultContentManager(gameVersion, configFolder, gameResRootPath, externalizedDataPath)
	{}
	virtual void init()
	{
		// externalized data is used in the tests
		if (!Vfs_addDir(m_vfs.get(), VFS_ORDER_STRACCIATELLA, m_externalizedDataPath.c_str()))
		{
			RustPointer<char> err{getRustError()};
			SLOGE(ST::format("DefaultContentManagerUT::init '{}': {}", m_externalizedDataPath, err.get()));
			throw std::runtime_error("Failed to add stracciatella dir");
		}
	}
};

/** Create DefaultContentManager for usage in unit testing. */
DefaultContentManager * createDefaultCMForTesting()
{
	ST::string extraDataDir = GetExtraDataDir();
	ST::string configFolderPath = FileMan::joinPaths(extraDataDir, "unittests");
	ST::string gameResRootPath = FileMan::joinPaths(extraDataDir, "unittests");
	ST::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

	DefaultContentManager* cm = new DefaultContentManagerUT(GameVersion::ENGLISH,
					configFolderPath,
					gameResRootPath, externalizedDataPath);
	cm->init();

	return cm;
}
