#include "TraversibilityMapping.h"

// mapping from traversibility string name to enum
TraversibilityMap TraversibilityMapping::deserialize(const rapidjson::Document& root)
{
	TraversibilityMap mapToEnum;
	for (auto& iter : root.GetObject())
	{
		mapToEnum.insert(std::make_pair(iter.name.GetString(), iter.value.GetInt()));
	}
	return mapToEnum;
}
