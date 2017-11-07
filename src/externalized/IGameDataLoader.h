#pragma once

#include "rapidjson/document.h"

class IGameDataLoader
{
public:
	rapidjson::Document* readJsonDataFile(const char *fileName) const;
};
