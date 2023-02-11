#pragma once

#include "StrategicAIPolicy.h"
#include "Json.h"


class DefaultStrategicAIPolicy : public StrategicAIPolicy
{
public:
	DefaultStrategicAIPolicy(const JsonValue& json);
};
