#include "TestUtils.h"

#include "externalized/DefaultContentManager.h"


SGPFile* OpenTestResourceForReading(const char *filePath)
{
	ST::string extraDataDir = GetExtraDataDir();
	return FileMan::openForReading(FileMan::joinPaths(extraDataDir, filePath));
}

ST::string GetExtraDataDir()
{
	ST::string extraDataDir = EXTRA_DATA_DIR;
	if(extraDataDir.empty())
	{
		extraDataDir = ".";
	}
	return extraDataDir;
}
