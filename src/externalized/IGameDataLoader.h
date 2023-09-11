#pragma once

#include "Json.h"

class IGameDataLoader
{
public:
	JsonValue readJsonDataFile(const char *fileName) const;
};
