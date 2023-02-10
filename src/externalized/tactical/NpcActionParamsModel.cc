#include "NpcActionParamsModel.h"

#include "Logger.h"
#include <string_theory/format>

NpcActionParamsModel::NpcActionParamsModel(const uint16_t actionCode_, const int16_t gridNo_, const int32_t amount_)
	: actionCode(actionCode_), gridNo(gridNo_), amount(amount_) {}

int16_t NpcActionParamsModel::getGridNo(const int16_t defaultValue) const
{
	if (gridNo <= 0)
	{
		SLOGW("Falling back to default gridNo for NPC_ACTION #{}. Param values should always be defined in JSON.", actionCode);
		return defaultValue;
	}
	return gridNo;
}

int32_t NpcActionParamsModel::getAmount(const int32_t defaultValue) const
{
	if (amount == 0)
	{
		SLOGW("Falling back to default amount for NPC_ACTION #{}. Param values should always be defined in JSON.", actionCode);
		return defaultValue;
	}
	return amount;
}

NpcActionParamsModel* NpcActionParamsModel::deserialize(const JsonValue& json)
{
	auto obj = json.toObject();
	return new NpcActionParamsModel(
		static_cast<uint16_t>(obj.GetInt("actionCode")),
		static_cast<int16_t>(obj.getOptionalInt("gridNo", 0)),
		static_cast<int32_t>(obj.getOptionalInt("amount", 0))
	);
}

const NpcActionParamsModel NpcActionParamsModel::empty = { 0, 0, 0 };

