#include "TestUtils.h"

#include "externalized/DefaultContentManager.h"


SGPFile* OpenTestResourceForReading(const char *filePath)
{
	std::string extraDataDir = GetExtraDataDir();
	return FileMan::openForReading(FileMan::joinPaths(extraDataDir, filePath));
}

std::string GetExtraDataDir()
{
	std::string extraDataDir = EXTRA_DATA_DIR;
	if(extraDataDir.empty())
	{
		extraDataDir = ".";
	}
	return extraDataDir;
}
