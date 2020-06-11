#ifdef WITH_UNITTESTS

#include "DefaultContentManager.h"
#include "DefaultContentManagerUT.h"
#include "FileMan.h"
#include "TestUtils.h"

#include "gtest/gtest.h"

#ifdef __cpp_lib_filesystem
#include <filesystem>
namespace fs = std::filesystem;
#else
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem> // remove when we are on C++ 17
namespace fs = std::experimental::filesystem;
#endif

#define TMPDIR "temp"

#define PS PATH_SEPARATOR_STR

TEST(TempFiles, createFile)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	Fs_removeDirAll(TMPDIR);
	FileMan::createDir(TMPDIR);

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
	}

	std::vector<ST::string> results = FindFilesInDir(TMPDIR, "txt", false, false);
	ASSERT_EQ(results.size(), 1u);
	EXPECT_STREQ(results[0].c_str(), TMPDIR PS "foo.txt");

	Fs_removeDirAll(TMPDIR);
	delete cm;
}

TEST(TempFiles, writeToFile)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	Fs_removeDirAll(TMPDIR);
	FileMan::createDir(TMPDIR);

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		FileWrite(file, "hello", 5);
	}

	// open for writing, but don't truncate
	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", false));
		ASSERT_EQ(FileGetSize(file), 5u);
	}

	// open with truncate and check that it is empty
	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		ASSERT_EQ(FileGetSize(file), 0u);
	}

	// // void FileRead(SGPFile* const f, void* const pDest, size_t const uiBytesToRead)

	Fs_removeDirAll(TMPDIR);
	delete cm;
}

TEST(TempFiles, writeAndRead)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	Fs_removeDirAll(TMPDIR);
	FileMan::createDir(TMPDIR);

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		FileWrite(file, "hello", 5);
	}

	{
		char buf[10];
		AutoSGPFile file(cm->openTempFileForReading("foo.txt"));
		FileRead(file, buf, 5);
		buf[5] = 0;
		ASSERT_STREQ(buf, "hello");
	}

	Fs_removeDirAll(TMPDIR);
	delete cm;
}

TEST(TempFiles, append)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	Fs_removeDirAll(TMPDIR);
	FileMan::createDir(TMPDIR);

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		FileWrite(file, "hello", 5);
	}

	{
		AutoSGPFile file(cm->openTempFileForAppend("foo.txt"));
		FileWrite(file, "hello", 5);
	}

	{
		AutoSGPFile file(cm->openTempFileForReading("foo.txt"));
		ASSERT_EQ(FileGetSize(file), 10u);
	}

	Fs_removeDirAll(TMPDIR);
	delete cm;
}

TEST(TempFiles, deleteFile)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();
	Fs_removeDirAll(TMPDIR);
	FileMan::createDir(TMPDIR);

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
	}

	std::vector<ST::string> results = FindFilesInDir(TMPDIR, "txt", false, false);
	ASSERT_EQ(results.size(), 1u);

	cm->deleteTempFile("foo.txt");

	results = FindFilesInDir(TMPDIR, "txt", false, false);
	ASSERT_EQ(results.size(), 0u);

	Fs_removeDirAll(TMPDIR);
	delete cm;
}

TEST(ExternalizedData, readAllData)
{
	DefaultContentManager* cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());
}

std::vector<ST::string> recursivelyListAllFiles(ST::string dir, std::string suffix)
{
	std::vector<ST::string> files;
	for (auto& p : fs::recursive_directory_iterator(dir.to_std_string()))
	{
		ST::string path = ST::string(p.path());
		if (path.ends_with(suffix))
		{
			files.push_back(path);
		}
	}
	return files;
}

TEST(ExternalizedData, readEveryFile)
{
	// Not all files (e.g. translations) are covered by the previous test
	DefaultContentManagerUT* cm = DefaultContentManagerUT::createDefaultCMForTesting();

	ST::string dataPath = ST::format("{}/externalized", GetExtraDataDir());
	std::vector<ST::string> results = recursivelyListAllFiles(dataPath, ".json");
	for (ST::string f : results)
	{
		auto json = cm->_readJsonDataFile(f.c_str());
		ASSERT_FALSE(json == NULL);
	}
}
#endif
