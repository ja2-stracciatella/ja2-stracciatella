#include "gtest/gtest.h"

#include "FileMan.h"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

#include "externalized/TestUtils.h"

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
#define PS PATH_SEPARATOR_STR

	// find one file with .txt estension
	// result on Linux: "unittests/find-files/lowercase-ext.txt"
	// result on Win:   "unittests/find-files\lowercase-ext.txt"

	std::string testDir = FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files");

	std::vector<std::string> results = FindFilesInDir(testDir, ".txt", false, false);
	ASSERT_EQ(results.size(), 1);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());

	results = FindFilesInDir(FileMan::joinPaths(GetExtraDataDir(), "unittests" PS "find-files"), ".txt", false, false);
	ASSERT_EQ(results.size(), 1);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests" PS "find-files" PS "lowercase-ext.txt").c_str());

	results = FindFilesInDir(testDir, ".TXT", false, false);
	ASSERT_EQ(results.size(), 1);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

	results = FindFilesInDir(testDir, ".TXT", false, true);
	ASSERT_EQ(results.size(), 1);
	EXPECT_STREQ(results[0].c_str(), "uppercase-ext.TXT");

	results = FindFilesInDir(testDir, ".tXt", true, false);
	std::sort(results.begin(), results.end());
	ASSERT_EQ(results.size(), 2);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());
	EXPECT_STREQ(results[1].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

	results = FindFilesInDir(testDir, ".tXt", true, false, true);
	ASSERT_EQ(results.size(), 2);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());
	EXPECT_STREQ(results[1].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

	results = FindFilesInDir(testDir, ".tXt", true, true, true);
	ASSERT_EQ(results.size(), 2);
	EXPECT_STREQ(results[0].c_str(), "lowercase-ext.txt");
	EXPECT_STREQ(results[1].c_str(), "uppercase-ext.TXT");

	results = FindAllFilesInDir(testDir, true);
	ASSERT_EQ(results.size(), 3);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "file-without-extension").c_str());
	EXPECT_STREQ(results[1].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());
	EXPECT_STREQ(results[2].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

#undef PS
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

	EraseDirectory(tmpDir.string().c_str());

	// check that the subdirectory is still there
	ASSERT_EQ(boost::filesystem::is_directory(subDir), true);

	results = FindAllFilesInDir(tmpDir.string(), true);
	ASSERT_EQ(results.size(), 0);
}

TEST(FileManTest, ReadTextFile)
{
	boost::filesystem::path tmpDir = boost::filesystem::temp_directory_path();
	boost::filesystem::path pathA = tmpDir / "foo.txt";

	boost::filesystem::ofstream fileA(pathA);
	fileA << "foo bar baz";
	fileA.close();

	SGPFile* forReading = FileMan::openForReading(pathA.string().c_str());
	std::string content = FileMan::fileReadText(forReading);
	ASSERT_STREQ(content.c_str(), "foo bar baz");
}

TEST(FileManTest, GetFileName)
{
	EXPECT_STREQ(FileMan::getFileName("foo.txt").c_str(),        "foo.txt");
	EXPECT_STREQ(FileMan::getFileName("/a/foo.txt").c_str(),     "foo.txt");
	EXPECT_STREQ(FileMan::getFileName("../a/foo.txt").c_str(),   "foo.txt");

	EXPECT_STREQ(FileMan::getFileNameWithoutExt("foo.txt").c_str(),       "foo");
	EXPECT_STREQ(FileMan::getFileNameWithoutExt("/a/foo.txt").c_str(),    "foo");
	EXPECT_STREQ(FileMan::getFileNameWithoutExt("../a/foo.txt").c_str(),  "foo");
}

#ifdef __WINDOWS__
TEST(FileManTest, GetFileNameWin)
{
	// This tests fail on Linux.
	// Which is might be correct or not correct (depending on your point of view)
	EXPECT_STREQ(FileMan::getFileName("c:\\foo.txt").c_str(),    "foo.txt");
	EXPECT_STREQ(FileMan::getFileName("c:\\b\\foo.txt").c_str(), "foo.txt");

	EXPECT_STREQ(FileMan::getFileNameWithoutExt("c:\\foo.txt").c_str(),     "foo");
	EXPECT_STREQ(FileMan::getFileNameWithoutExt("c:\\b\\foo.txt").c_str(),  "foo");
}
#endif

TEST(FileManTest, ReplaceExtension)
{
	EXPECT_STREQ(FileMan::replaceExtension("foo.txt", "").c_str(),        "foo");
	EXPECT_STREQ(FileMan::replaceExtension("foo.txt", ".").c_str(),       "foo.");
	EXPECT_STREQ(FileMan::replaceExtension("foo.txt", ".bin").c_str(),    "foo.bin");
	EXPECT_STREQ(FileMan::replaceExtension("foo.txt", "bin").c_str(),     "foo.bin");

	EXPECT_STREQ(FileMan::replaceExtension("foo.bar.txt", "").c_str(),        "foo.bar");
	EXPECT_STREQ(FileMan::replaceExtension("foo.bar.txt", ".").c_str(),       "foo.bar.");
	EXPECT_STREQ(FileMan::replaceExtension("foo.bar.txt", ".bin").c_str(),    "foo.bar.bin");
	EXPECT_STREQ(FileMan::replaceExtension("foo.bar.txt", "bin").c_str(),     "foo.bar.bin");

	EXPECT_STREQ(FileMan::replaceExtension("c:/a/foo.txt", "").c_str(),        "c:/a/foo");
	EXPECT_STREQ(FileMan::replaceExtension("c:/a/foo.txt", ".").c_str(),       "c:/a/foo.");
	EXPECT_STREQ(FileMan::replaceExtension("c:/a/foo.txt", ".bin").c_str(),    "c:/a/foo.bin");
	EXPECT_STREQ(FileMan::replaceExtension("c:/a/foo.txt", "bin").c_str(),     "c:/a/foo.bin");

	EXPECT_STREQ(FileMan::replaceExtension("/a/foo.txt", "").c_str(),          "/a/foo");
	EXPECT_STREQ(FileMan::replaceExtension("/a/foo.txt", ".").c_str(),         "/a/foo.");
	EXPECT_STREQ(FileMan::replaceExtension("/a/foo.txt", ".bin").c_str(),      "/a/foo.bin");
	EXPECT_STREQ(FileMan::replaceExtension("/a/foo.txt", "bin").c_str(),       "/a/foo.bin");

	EXPECT_STREQ(FileMan::replaceExtension("c:\\a\\foo.txt", "").c_str(),      "c:\\a\\foo");
	EXPECT_STREQ(FileMan::replaceExtension("c:\\a\\foo.txt", ".").c_str(),     "c:\\a\\foo.");
	EXPECT_STREQ(FileMan::replaceExtension("c:\\a\\foo.txt", ".bin").c_str(),  "c:\\a\\foo.bin");
	EXPECT_STREQ(FileMan::replaceExtension("c:\\a\\foo.txt", "bin").c_str(),   "c:\\a\\foo.bin");
}

TEST(FileManTest, SlashifyPath)
{
	std::string test("foo\\bar\\baz");
	FileMan::slashifyPath(test);
	EXPECT_STREQ(test.c_str(), "foo/bar/baz");
}
