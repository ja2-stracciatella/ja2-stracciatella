#pragma once

#include "Facts.h"
#include "Json.h"

class FactParamsModel
{
public:
	FactParamsModel(const Fact fact_, const int16_t gridNo_);

	int16_t getGridNo(const int16_t defaultValue) const;

	static FactParamsModel* deserialize(const JsonValue& json);
	static const FactParamsModel empty; // an empty instance that always return given defaults

	const Fact fact;

protected:
	const int16_t gridNo;
};
