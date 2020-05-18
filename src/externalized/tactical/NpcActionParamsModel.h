#pragma once

#include "JsonObject.h"

class NpcActionParamsModel
{
public:
	NpcActionParamsModel(const uint16_t actionCode_, const int16_t gridNo_, const int32_t amount_);
	
	int16_t getGridNo(const int16_t defaultValue) const;
	int32_t getAmount(const int32_t defaultValue) const;
	
	static NpcActionParamsModel* deserialize(const JsonObjectReader& obj);
	static const NpcActionParamsModel empty; // an empty instance that always return given defaults

	const uint16_t actionCode;

protected:
	const int16_t gridNo;
	const int32_t amount;
};
