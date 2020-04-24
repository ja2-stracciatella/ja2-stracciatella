#include "DefaultContentManagerUT.h"

#include "GameRes.h"
#include "sgp/FileMan.h"

#include "externalized/DefaultContentManager.h"
#include "externalized/TestUtils.h"

/** Create DefaultContentManager for usage in unit testing. */
DefaultContentManager * createDefaultCMForTesting()
{
	ST::string extraDataDir = GetExtraDataDir();
	ST::string configFolderPath = FileMan::joinPaths(extraDataDir, "unittests");
	ST::string gameResRootPath = FileMan::joinPaths(extraDataDir, "unittests");
	ST::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

	DefaultContentManager *cm;

	cm = new DefaultContentManager(GameVersion::ENGLISH,
					configFolderPath,
					gameResRootPath, externalizedDataPath);

	// we don't load game resources
	// bacause we don't need them at the moment

	return cm;
}
