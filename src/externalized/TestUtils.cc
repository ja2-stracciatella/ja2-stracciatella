#include "TestUtils.h"

#include "externalized/DefaultContentManager.h"


SGPFile* OpenTestResourceForReading(const char *filePath)
{
	ST::string extraDataDir = GetExtraDataDir();
	return FileMan::openForReading(FileMan::joinPaths(extraDataDir, filePath));
}

ST::string GetExtraDataDir()
{
	RustPointer<char> extraDataDir(Env_assetsDir());
	return ST::string(extraDataDir.get());
}
