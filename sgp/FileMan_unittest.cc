#include "gtest/gtest.h"

#include "FileMan.h"

TEST(FileManTest, joinPaths)
{
  // void joinPaths(const char *first, const char *second, char *outputBuf, int outputBufSize);
  {
    char buf[256];

    // ~~~ unix separators

    FileMan::joinPaths("foo", "bar", buf, 256);
    EXPECT_STREQ(buf, "foo/bar");

    FileMan::joinPaths("foo/", "bar", buf, 256);
    EXPECT_STREQ(buf, "foo/bar");

    // // XXX FAILS
    // FileMan::joinPaths("foo", "/bar", buf, 256);
    // EXPECT_STREQ(buf, "foo/bar");

    // // XXX FAILS
    // FileMan::joinPaths("foo/", "/bar", buf, 256);
    // EXPECT_STREQ(buf, "foo/bar");

    // ~~~ win separators

    // hmm... what do we really want from the library?

  }

  // std::string joinPaths(const char *first, const char *second);
  {
    std::string result;

    // ~~~ unix separators

    result = FileMan::joinPaths("foo", "bar");
    EXPECT_STREQ(result.c_str(), "foo/bar");

    result = FileMan::joinPaths("foo/", "bar");
    EXPECT_STREQ(result.c_str(), "foo/bar");

    // // XXX FAILS
    // result = FileMan::joinPaths("foo", "/bar");
    // EXPECT_STREQ(result.c_str(), "foo/bar");

    // // XXX FAILS
    // result = FileMan::joinPaths("foo/", "/bar");
    // EXPECT_STREQ(result.c_str(), "foo/bar");

    // ~~~ win separators

    // hmm... what do we really want from the library?

  }

}
