#include "DefaultContentManagerUT.h"

#include "GameRes.h"
#include "src/sgp/FileMan.h"

#include "src/externalized/DefaultContentManager.h"
#include "src/externalized/TestUtils.h"

/** Create DefaultContentManager for usage in unit testing. */
DefaultContentManager * createDefaultCMForTesting()
{
  std::string extraDataDir = GetExtraDataDir();
  std::string configFolderPath = FileMan::joinPaths(extraDataDir, "_unittests");
  std::string gameResRootPath = FileMan::joinPaths(extraDataDir, "_unittests");
  std::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

  DefaultContentManager *cm;

  cm = new DefaultContentManager(GV_ENGLISH,
                                 configFolderPath,
                                 gameResRootPath, externalizedDataPath);

  // we don't load game resources
  // bacause we don't need them at the moment

  return cm;
}
