#pragma once

class IGameDataLoader
{
public:
	JsonValue readJsonDataFile(const char *fileName) const;
};
