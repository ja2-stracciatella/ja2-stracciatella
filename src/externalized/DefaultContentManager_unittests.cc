#ifdef WITH_UNITTESTS

#include "DefaultContentManager.h"
#include "DefaultContentManagerUT.h"
#include "FileMan.h"
#include "TestUtils.h"

#include "gtest/gtest.h"

TEST(TempFiles, createFile)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
	}

	std::vector<ST::string> results = cm->findAllFilesInTempDir(ST::string(""), false, false, true);
	ASSERT_EQ(results.size(), 1u);
	EXPECT_STREQ(results[0].c_str(), "foo.txt");

	delete cm;
}

TEST(TempFiles, writeToFile)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		file->write("hello", 5);
	}

	// open for writing, but don't truncate
	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", false));
		ASSERT_EQ(file->size(), 5u);
	}

	// open with truncate and check that it is empty
	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		ASSERT_EQ(file->size(), 0u);
	}

	delete cm;
}

TEST(TempFiles, writeAndRead)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		file->write("hello", 5);
	}

	{
		char buf[10];
		AutoSGPFile file(cm->openTempFileForReading("foo.txt"));
		file->read(buf, 5);
		buf[5] = 0;
		ASSERT_STREQ(buf, "hello");
	}

	delete cm;
}

TEST(TempFiles, append)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
		file->write("hello", 5);
	}

	{
		AutoSGPFile file(cm->openTempFileForAppend("foo.txt"));
		file->write("hello", 5);
	}

	{
		AutoSGPFile file(cm->openTempFileForReading("foo.txt"));
		ASSERT_EQ(file->size(), 10u);
	}

	delete cm;
}

TEST(TempFiles, deleteFile)
{
	DefaultContentManager * cm = DefaultContentManagerUT::createDefaultCMForTesting();

	{
		AutoSGPFile file(cm->openTempFileForWriting("foo.txt", true));
	}

	std::vector<ST::string> results = cm->findAllFilesInTempDir("", false, false, true);
	ASSERT_EQ(results.size(), 1u);

	cm->deleteTempFile("foo.txt");

	results = cm->findAllFilesInTempDir("", false, false, true);
	ASSERT_EQ(results.size(), 0u);

	delete cm;
}

TEST(ExternalizedData, readAllData)
{
	DefaultContentManager* cm = DefaultContentManagerUT::createDefaultCMForTesting();
	ASSERT_TRUE(cm->loadGameData());
}

TEST(ExternalizedData, readEveryFile)
{
	// Not all files (e.g. translations) are covered by the previous test
	DefaultContentManagerUT* cm = DefaultContentManagerUT::createDefaultCMForTesting();

	ST::string dataPath = ST::format("{}/externalized", GetExtraDataDir());
	std::vector<ST::string> results = FileMan::findFilesInDir(dataPath, "json", true, true, false, true);
	for (ST::string f : results)
	{
		ST::string relativePath = f.substr(dataPath.size() + 1);
		auto json = cm->_readJsonDataFile(relativePath.c_str());
		ASSERT_FALSE(json.get() == NULL);
	}
}
#endif
