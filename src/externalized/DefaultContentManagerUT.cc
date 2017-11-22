#include "DefaultContentManagerUT.h"

#include "GameRes.h"
#include "sgp/FileMan.h"
#include "sgp/PathTools.h"

#include "externalized/DefaultContentManager.h"
#include "externalized/TestUtils.h"

/** Create DefaultContentManager for usage in unit testing. */
DefaultContentManager * createDefaultCMForTesting()
{
	std::string extraDataDir = GetExtraDataDir();
	std::string configFolderPath = PathTools::joinPaths(extraDataDir, "unittests");
	std::string gameResRootPath = PathTools::joinPaths(extraDataDir, "unittests");
	std::string externalizedDataPath = PathTools::joinPaths(extraDataDir, "externalized");

	DefaultContentManager *cm;

	cm = new DefaultContentManager(GV_ENGLISH,
					configFolderPath,
					gameResRootPath, externalizedDataPath);

	// we don't load game resources
	// bacause we don't need them at the moment

	return cm;
}
