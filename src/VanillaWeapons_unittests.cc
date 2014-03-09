#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

#include "src/DefaultContentManager.h"
#include "src/DefaultContentManagerUT.h"

TEST(Items, weaponsLoading)
{
  DefaultContentManager * cm = createDefaultCMForTesting();
  ASSERT_TRUE(cm != NULL);
  ASSERT_TRUE(cm->loadGameData());
  EXPECT_TRUE(cm->getWeaponByName("MP5K") != NULL);
  EXPECT_TRUE(cm->getWeapon(9 /* MP5K */) != NULL);
  EXPECT_EQ(cm->getWeaponByName("MP5K"), cm->getWeapon(9 /* MP5K */));
  delete cm;
}

#endif
