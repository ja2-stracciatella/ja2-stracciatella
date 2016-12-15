#include "DefaultContentManagerUT.h"

#include "game/GameRes.h"
#include "sgp/FileMan.h"

#include "DefaultContentManager.h"
#include "TestUtils.h"

/** Create DefaultContentManager for usage in unit testing. */
DefaultContentManager * createDefaultCMForTesting()
{
  std::string extraDataDir = GetExtraDataDir();
  std::string configFolderPath = FileMan::joinPaths(extraDataDir, "unittests");
  std::string gameResRootPath = FileMan::joinPaths(extraDataDir, "unittests");
  std::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

  DefaultContentManager *cm;

  cm = new DefaultContentManager(GV_ENGLISH,
                                 configFolderPath,
                                 gameResRootPath, externalizedDataPath);

  // we don't load game resources
  // bacause we don't need them at the moment

  return cm;
}
