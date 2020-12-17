#include "FactParamsModel.h"

#include "Logger.h"
#include <string_theory/format>

FactParamsModel::FactParamsModel(const Fact fact_, const int16_t gridNo_)
	: fact(fact_), gridNo(gridNo_) {}

int16_t FactParamsModel::getGridNo(const int16_t defaultValue) const
{
	if (gridNo <= 0) 
	{
		STLOGW("Falling back to default gridNo for FACT #{}. Param values should always be defined in JSON.", fact);
		return defaultValue;
	}
	return gridNo;
}

FactParamsModel* FactParamsModel::deserialize(const JsonObjectReader& obj)
{
	Fact fact = static_cast<Fact>(obj.GetInt("fact"));
	int16_t gridNo = obj.getOptionalInt("gridNo", 0);
	return new FactParamsModel(fact, gridNo);
}

const FactParamsModel FactParamsModel::empty = { FACT_NONE, 0 };
