#include "TestUtils.h"

#include "externalized/DefaultContentManager.h"
#include "sgp/PathTools.h"

SGPFile* OpenTestResourceForReading(const char *filePath)
{
	std::string extraDataDir = GetExtraDataDir();
	return FileMan::openForReading(PathTools::joinPaths(extraDataDir, filePath));
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
