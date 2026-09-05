#include "gtest/gtest.h"

#include "DirFs.h"
#include "Exceptions.h"
#include "FileMan.h"

#include <memory>

namespace
{
	void writeFile(DirFs& dirFs, const ST::string& path, const ST::string& contents)
	{
		AutoSGPFile file{dirFs.openForWriting(path)};
		file->write(contents.c_str(), contents.size());
	}

	ST::string readFile(DirFs& dirFs, const ST::string& path)
	{
		AutoSGPFile file{dirFs.openForReading(path)};
		return file->readStringToEnd();
	}
}

TEST(DirFsTest, writeReadAndDelete)
{
	auto dirFs = DirFs::createTemp();

	EXPECT_FALSE(dirFs->exists("save.sav"));
	writeFile(*dirFs, "save.sav", "savegame");

	EXPECT_TRUE(dirFs->exists("save.sav"));
	EXPECT_TRUE(dirFs->isFile("save.sav"));
	EXPECT_FALSE(dirFs->isDir("save.sav"));
	EXPECT_EQ(readFile(*dirFs, "save.sav"), "savegame");

	dirFs->deleteFile("save.sav");
	EXPECT_FALSE(dirFs->exists("save.sav"));
}

TEST(DirFsTest, temporaryDirectoryIsRemovedWithTheDirFs)
{
	ST::string path;
	{
		auto dirFs = DirFs::createTemp();
		path = dirFs->basePath();
		writeFile(*dirFs, "scratch.dat", "scratch");
		EXPECT_TRUE(FileMan::isDir(path));
	}

	EXPECT_FALSE(FileMan::exists(path));
}

TEST(DirFsTest, pathsAreCaseInsensitive)
{
	auto dirFs = DirFs::createTemp();
	dirFs->createDir("SavedGames");
	writeFile(*dirFs, "SavedGames/SaveGame01.sav", "savegame");

	EXPECT_TRUE(dirFs->exists("savedgames/savegame01.sav"));
	EXPECT_EQ(readFile(*dirFs, "SAVEDGAMES/SAVEGAME01.SAV"), "savegame");
	// Resolving keeps the case that is actually on disk.
	EXPECT_EQ(dirFs->resolveExistingComponents("savedgames/savegame01.sav"), "SavedGames/SaveGame01.sav");
	// Components that do not exist are kept as they were passed in.
	EXPECT_EQ(dirFs->resolveExistingComponents("savedgames/Missing.sav"), "SavedGames/Missing.sav");
}

TEST(DirFsTest, createDirCreatesParents)
{
	auto dirFs = DirFs::createTemp();

	dirFs->createDir("saves/backup");
	EXPECT_TRUE(dirFs->isDir("saves"));
	EXPECT_TRUE(dirFs->isDir("saves/backup"));

	// Creating an existing directory does nothing instead of failing.
	EXPECT_NO_THROW(dirFs->createDir("saves/backup"));
}

TEST(DirFsTest, listsFilesAndDirs)
{
	auto dirFs = DirFs::createTemp();
	dirFs->createDir("saves");
	writeFile(*dirFs, "saves/SaveGame01.sav", "1");
	writeFile(*dirFs, "saves/SaveGame02.SAV", "2");
	writeFile(*dirFs, "saves/notes.txt", "3");

	std::vector<ST::string> names = dirFs->findAllFilesInDir("saves", false, false, true);
	ASSERT_EQ(names.size(), 3u);
	// Listings fold the case, like every other lookup in the VFS, so they sort folded too.
	EXPECT_EQ(names[0], "notes.txt");
	EXPECT_EQ(names[1], "savegame01.sav");
	EXPECT_EQ(names[2], "savegame02.sav");

	// The extension filter is case insensitive as well, so it matches SaveGame02.SAV.
	std::vector<ST::string> saves = dirFs->findFilesInDir("saves", "sav", true, true);
	EXPECT_EQ(saves.size(), 2u);

	std::vector<ST::string> dirs = dirFs->findAllDirsInDir("", false, false, true);
	ASSERT_EQ(dirs.size(), 1u);
	EXPECT_EQ(dirs[0], "saves");
}

TEST(DirFsTest, moveFileWithinDirFs)
{
	auto dirFs = DirFs::createTemp();
	dirFs->createDir("backup");
	writeFile(*dirFs, "one.sav", "savegame");

	dirFs->moveFile("one.sav", "backup/one.sav");

	EXPECT_FALSE(dirFs->exists("one.sav"));
	EXPECT_EQ(readFile(*dirFs, "backup/one.sav"), "savegame");
}

TEST(DirFsTest, moveFileBetweenDirFs)
{
	// This is what saving a game does: write to the temp directory, then move the finished file
	// into the save game directory.
	auto tempFiles = DirFs::createTemp();
	auto saveGameFiles = DirFs::createTemp();
	writeFile(*tempFiles, "temp.sav", "savegame");

	DirFs::moveFileBetween(*tempFiles, "temp.sav", *saveGameFiles, "SaveGame01.sav");

	EXPECT_FALSE(tempFiles->exists("temp.sav"));
	EXPECT_EQ(readFile(*saveGameFiles, "SaveGame01.sav"), "savegame");
}

TEST(DirFsTest, eraseDirKeepsSubdirectories)
{
	auto dirFs = DirFs::createTemp();
	dirFs->createDir("saves/backup");
	writeFile(*dirFs, "saves/one.sav", "1");
	writeFile(*dirFs, "saves/backup/two.sav", "2");

	dirFs->eraseDir("saves");

	EXPECT_FALSE(dirFs->exists("saves/one.sav"));
	EXPECT_TRUE(dirFs->isDir("saves/backup"));
	EXPECT_TRUE(dirFs->exists("saves/backup/two.sav"));
}

TEST(DirFsTest, reportsLastModifiedTime)
{
	auto dirFs = DirFs::createTemp();
	writeFile(*dirFs, "one.sav", "savegame");

	// Just has to be a plausible unix timestamp, the value itself is up to the filesystem.
	EXPECT_GT(dirFs->getLastModifiedTime("one.sav"), 0.0);
	EXPECT_THROW(dirFs->getLastModifiedTime("missing.sav"), IoException);
}

TEST(DirFsTest, appendsToExistingFile)
{
	auto dirFs = DirFs::createTemp();
	writeFile(*dirFs, "log.txt", "one");
	{
		AutoSGPFile file{dirFs->openForAppend("log.txt")};
		file->write("two", 3);
	}

	EXPECT_EQ(readFile(*dirFs, "log.txt"), "onetwo");
}

TEST(DirFsTest, missingDirectoryFailsOnUse)
{
	// Creating a DirFs never touches the filesystem, so a directory that is not there only shows
	// up once something is actually read or written.
	DirFs dirFs{FileMan::joinPaths(FileMan::joinPaths("nonexistent", "directory"), "ja2")};

	EXPECT_FALSE(dirFs.exists("one.sav"));
	EXPECT_FALSE(dirFs.isDir(""));
	EXPECT_THROW(dirFs.openForReading("one.sav"), IoException);
}
