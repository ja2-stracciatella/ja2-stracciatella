#include "TraversibilityMapping.h"

// mapping from traversibility string name to enum
TraversibilityMap TraversibilityMapping::deserialize(const JsonValue& root)
{
	auto obj = root.toObject();
	auto keys = obj.keys();
	TraversibilityMap mapToEnum;
	for (auto& key : keys)
	{
		mapToEnum.insert(std::make_pair(key, obj.GetInt(key.c_str())));
	}
	return mapToEnum;
}
