#pragma once

#include "StrategicAIPolicy.h"
#include "rapidjson/document.h"


class DefaultStrategicAIPolicy : public StrategicAIPolicy
{
public:
	DefaultStrategicAIPolicy(rapidjson::Document *json);
};
