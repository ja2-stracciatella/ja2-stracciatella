#include "gtest/gtest.h"

#include "SGPStrings.h"

TEST(SGPStringsTest, ReplacePath)
{
  char buf[256];

  // replacing extension
  {
    ReplacePath(buf, 256, NULL, "foo.txt", ".bin");
    EXPECT_STREQ(buf, "foo.bin");

    ReplacePath(buf, 256, NULL, "c:\\dir\\foo.txt", ".bin");
    EXPECT_STREQ(buf, "c:\\dir\\foo.bin");

    ReplacePath(buf, 256, NULL, "/home/user/foo.txt", ".bin");
    EXPECT_STREQ(buf, "/home/user/foo.bin");
  }

  // replacing directory and extension
  {
    // // FAILS without trailing slash
    // ReplacePath(buf, 256, "d:\\another", "c:\\dir\\foo.txt", ".bin");
    // EXPECT_STREQ(buf, "d:\\another\\foo.bin");

    ReplacePath(buf, 256, "d:\\another\\", "c:\\dir\\foo.txt", ".bin");
    EXPECT_STREQ(buf, "d:\\another\\foo.bin");

    ReplacePath(buf, 256, "/home/user/", "c:\\dir\\foo.txt", ".bin");
    EXPECT_STREQ(buf, "/home/user/foo.bin");
  }
}
