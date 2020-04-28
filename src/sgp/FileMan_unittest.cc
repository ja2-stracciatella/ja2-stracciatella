#include "gtest/gtest.h"

#include "FileMan.h"

#include "externalized/TestUtils.h"

TEST(FileManTest, joinPaths)
{
	// ST::string joinPaths(const char *first, const char *second);
	{
		ST::string result;

		// ~~~ platform-specific separators

		result = FileMan::joinPaths("foo", "bar");
		EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
		result = FileMan::joinPaths(ST::string("foo"), ST::string("bar"));
		EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
		result = FileMan::joinPaths(ST::string("foo"), "bar");
		EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");

		result = FileMan::joinPaths("foo" PATH_SEPARATOR_STR, "bar");
		EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
		result = FileMan::joinPaths(ST::string("foo" PATH_SEPARATOR_STR), ST::string("bar"));
		EXPECT_STREQ(result.c_str(), "foo" PATH_SEPARATOR_STR "bar");
		result = FileMan::joinPaths(ST::string("foo" PATH_SEPARATOR_STR), "bar");
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


TEST(FileManTest, FindFilesInDir)
{
#define PS PATH_SEPARATOR_STR

	// find one file with .txt estension
	// result on Linux: "unittests/find-files/lowercase-ext.txt"
	// result on Win:   "unittests/find-files\lowercase-ext.txt"

	ST::string testDir = FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files");

	std::vector<ST::string> results = FindFilesInDir(testDir, "txt", false, false);
	ASSERT_EQ(results.size(), 1u);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());

	results = FindFilesInDir(FileMan::joinPaths(GetExtraDataDir(), "unittests" PS "find-files"), "txt", false, false);
	ASSERT_EQ(results.size(), 1u);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests" PS "find-files" PS "lowercase-ext.txt").c_str());

	results = FindFilesInDir(testDir, "TXT", false, false);
	ASSERT_EQ(results.size(), 1u);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

	results = FindFilesInDir(testDir, "TXT", false, true);
	ASSERT_EQ(results.size(), 1u);
	EXPECT_STREQ(results[0].c_str(), "uppercase-ext.TXT");

	results = FindFilesInDir(testDir, "tXt", true, false);
	std::sort(results.begin(), results.end());
	ASSERT_EQ(results.size(), 2u);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());
	EXPECT_STREQ(results[1].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

	results = FindFilesInDir(testDir, "tXt", true, false, true);
	ASSERT_EQ(results.size(), 2u);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());
	EXPECT_STREQ(results[1].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

	results = FindFilesInDir(testDir, "tXt", true, true, true);
	ASSERT_EQ(results.size(), 2u);
	EXPECT_STREQ(results[0].c_str(), "lowercase-ext.txt");
	EXPECT_STREQ(results[1].c_str(), "uppercase-ext.TXT");

	results = FindAllFilesInDir(testDir, true);
	ASSERT_EQ(results.size(), 3u);
	EXPECT_STREQ(results[0].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "file-without-extension").c_str());
	EXPECT_STREQ(results[1].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "lowercase-ext.txt").c_str());
	EXPECT_STREQ(results[2].c_str(), FileMan::joinPaths(GetExtraDataDir(), "unittests/find-files" PS "uppercase-ext.TXT").c_str());

#undef PS
}


TEST(FileManTest, RemoveAllFilesInDir)
{
	RustPointer<TempDir> tempDir(TempDir_create());
	ASSERT_NE(tempDir.get(), nullptr);
	RustPointer<char> tempPath(TempDir_path(tempDir.get()));
	ASSERT_NE(tempPath.get(), nullptr);
	ST::string subDir = FileMan::joinPaths(tempPath.get(), "subdir");
	ASSERT_EQ(Fs_createDir(subDir.c_str()), true);

	ST::string pathA = FileMan::joinPaths(tempPath.get(), "foo.txt");
	ST::string pathB = FileMan::joinPaths(tempPath.get(), "bar.txt");

	SGPFile* fileA = FileMan::openForWriting(pathA.c_str());
	ASSERT_NE(fileA, nullptr);
	SGPFile* fileB = FileMan::openForWriting(pathB.c_str());
	ASSERT_NE(fileB, nullptr);

	FileWrite(fileA, "foo", 3);
	FileWrite(fileB, "bar", 3);

	FileClose(fileA);
	FileClose(fileB);

	std::vector<ST::string> results = FindAllFilesInDir(tempPath.get(), true);
	ASSERT_EQ(results.size(), 2u);

	EraseDirectory(tempPath.get());

	// check that the subdirectory is still there
	ASSERT_EQ(Fs_isDir(subDir.c_str()), true);

	results = FindAllFilesInDir(tempPath.get(), true);
	ASSERT_EQ(results.size(), 0u);
}

TEST(FileManTest, ReadTextFile)
{
	RustPointer<TempDir> tempDir(TempDir_create());
	ASSERT_NE(tempDir.get(), nullptr);
	RustPointer<char> tempPath(TempDir_path(tempDir.get()));
	ASSERT_NE(tempPath.get(), nullptr);
	ST::string pathA = FileMan::joinPaths(tempPath.get(), "foo.txt");

	SGPFile* fileA = FileMan::openForWriting(pathA.c_str());
	ASSERT_NE(fileA, nullptr);
	FileWrite(fileA, "foo bar baz", 11);
	FileClose(fileA);

	SGPFile* forReading = FileMan::openForReading(pathA.c_str());
	ASSERT_NE(forReading, nullptr);
	ST::string content = FileMan::fileReadText(forReading);
	ASSERT_STREQ(content.c_str(), "foo bar baz");
	FileClose(forReading);
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
	ST::string test("foo\\bar\\baz");
	FileMan::slashifyPath(test);
	EXPECT_STREQ(test.c_str(), "foo/bar/baz");
}

TEST(FileManTest, FreeSpace)
{
	EXPECT_NE(GetFreeSpaceOnHardDriveWhereGameIsRunningFrom(), 0u);
}
