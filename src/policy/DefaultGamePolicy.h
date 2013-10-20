#pragma once

#include "GamePolicy.h"

#include "rapidjson/document.h"

class DefaultGamePolicy : public GamePolicy
{
public:
  DefaultGamePolicy(rapidjson::Document *json);
};
