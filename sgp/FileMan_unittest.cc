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


TEST(FileManTest, FindFiles)
{
  {
    SGP::FindFiles find("_unittests/find-files/*.txt");
    std::vector<std::string> results;
    for (;;)
    {
      const char * fileName = find.Next();
      if (fileName == NULL) break;
      results.push_back(std::string(fileName));
    }
#if CASE_SENSITIVE_FS
    EXPECT_EQ(results.size(), 1);
    EXPECT_STREQ(results[0].c_str(), "lowercase-ext.txt");
#else
    EXPECT_EQ(results.size(), 2);
    EXPECT_STREQ(results[0].c_str(), "lowercase-ext.txt");
    EXPECT_STREQ(results[1].c_str(), "uppercase-ext.TXT");
#endif
  }
}
