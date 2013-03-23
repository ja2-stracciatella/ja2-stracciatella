#include "gtest/gtest.h"

#include "FileMan.h"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

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


TEST(FileManTest, FindFilesWithBoost)
{
  std::vector<std::string> results = FindFilesInDir("_unittests/find-files", ".txt", false, false);
  ASSERT_EQ(results.size(), 1);
  EXPECT_STREQ(results[0].c_str(), "_unittests/find-files/lowercase-ext.txt");

  results = FindFilesInDir("_unittests/find-files", ".TXT", false, false);
  ASSERT_EQ(results.size(), 1);
  EXPECT_STREQ(results[0].c_str(), "_unittests/find-files/uppercase-ext.TXT");

  results = FindFilesInDir("_unittests/find-files", ".TXT", false, true);
  ASSERT_EQ(results.size(), 1);
  EXPECT_STREQ(results[0].c_str(), "uppercase-ext.TXT");

  results = FindFilesInDir("_unittests/find-files", ".tXt", true, false);
  std::sort(results.begin(), results.end());
  ASSERT_EQ(results.size(), 2);
  EXPECT_STREQ(results[0].c_str(), "_unittests/find-files/lowercase-ext.txt");
  EXPECT_STREQ(results[1].c_str(), "_unittests/find-files/uppercase-ext.TXT");

  results = FindFilesInDir("_unittests/find-files", ".tXt", true, false, true);
  ASSERT_EQ(results.size(), 2);
  EXPECT_STREQ(results[0].c_str(), "_unittests/find-files/lowercase-ext.txt");
  EXPECT_STREQ(results[1].c_str(), "_unittests/find-files/uppercase-ext.TXT");

  results = FindFilesInDir("_unittests/find-files", ".tXt", true, true, true);
  ASSERT_EQ(results.size(), 2);
  EXPECT_STREQ(results[0].c_str(), "lowercase-ext.txt");
  EXPECT_STREQ(results[1].c_str(), "uppercase-ext.TXT");

  results = FindAllFilesInDir("_unittests/find-files", true);
  ASSERT_EQ(results.size(), 3);
  EXPECT_STREQ(results[0].c_str(), "_unittests/find-files/file-without-extension");
  EXPECT_STREQ(results[1].c_str(), "_unittests/find-files/lowercase-ext.txt");
  EXPECT_STREQ(results[2].c_str(), "_unittests/find-files/uppercase-ext.TXT");
}


TEST(FileManTest, RemoveAllFilesInDir)
{
  boost::filesystem::path tmpDir = boost::filesystem::temp_directory_path();
  tmpDir /= boost::filesystem::unique_path();
  boost::filesystem::path subDir = tmpDir / "subdir";
  ASSERT_EQ(boost::filesystem::create_directory(tmpDir), true);
  ASSERT_EQ(boost::filesystem::create_directory(subDir), true);

  boost::filesystem::path pathA = tmpDir / "foo.txt";
  boost::filesystem::path pathB = tmpDir / "bar.txt";

  boost::filesystem::ofstream fileA(pathA);
  boost::filesystem::ofstream fileB(pathB);

  fileA << "foo";
  fileB << "bar";

  fileA.close();
  fileB.close();

  std::vector<std::string> results = FindAllFilesInDir(tmpDir.string(), true);
  ASSERT_EQ(results.size(), 2);

  EraseDirectory(tmpDir.c_str());

  // check that the subdirectory is still there
  ASSERT_EQ(boost::filesystem::is_directory(subDir), true);

  results = FindAllFilesInDir(tmpDir.string(), true);
  ASSERT_EQ(results.size(), 0);
}
