#include "DefaultContentManagerUT.h"

#include "GameRes.h"

#include "src/DefaultContentManager.h"

/** Create DefaultContentManager for usage in unit testing. */
DefaultContentManager * createDefaultCMForTesting()
{
  std::string configFolderPath = "_unittests";
  std::string gameResRootPath = "_unittests";
  std::string externalizedDataPath = "externalized";

  DefaultContentManager *cm;

  cm = new DefaultContentManager(GV_ENGLISH,
                                 configFolderPath,
                                 gameResRootPath, externalizedDataPath);

  // we don't load game resources
  // bacause we don't need them at the moment

  return cm;
}
