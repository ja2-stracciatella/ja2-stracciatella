#include "gtest/gtest.h"

#include "FileMan.h"

TEST(FileManTest, joinPaths)
{
  // std::string joinPaths(const char *first, const char *second);
  {
    std::string result;

    // ~~~ platform-specific separators

    result = FileMan::joinPaths("foo", "bar");
    EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
    result = FileMan::joinPaths(std::string("foo"), std::string("bar"));
    EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
    result = FileMan::joinPaths(std::string("foo"), "bar");
    EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");

    result = FileMan::joinPaths("foo" PATH_SEPARATOR_STR, "bar");
    EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
    result = FileMan::joinPaths(std::string("foo" PATH_SEPARATOR_STR), std::string("bar"));
    EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
    result = FileMan::joinPaths(std::string("foo" PATH_SEPARATOR_STR), "bar");
    EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");

    // // XXX FAILS
    // result = FileMan::joinPaths("foo", PATH_SEPARATOR_STR "bar");
    // EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");

    // // XXX FAILS
    // result = FileMan::joinPaths("foo" PATH_SEPARATOR_STR, PATH_SEPARATOR_STR "bar");
    // EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");

    // ~~~ unix separators
    // ~~~ win separators

    // hmm... what do we really want from the library?
    // should the library consider / to be the separator when it is compiled for windows?
  }

}
