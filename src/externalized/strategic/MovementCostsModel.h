#pragma once

#include "Json.h"
#include "TraversibilityMapping.h"
#include "Types.h"

#include <vector>

typedef std::vector<std::vector<uint8_t>> IntIntVector;

class MovementCostsModel
{
public:
	MovementCostsModel(IntIntVector traverseWE_, IntIntVector traverseNS_, IntIntVector traverseThrough_, IntIntVector travelRatings_);

	uint8_t getTraversibilityWestEast(const SGPSector& sSector) const;
	uint8_t getTraversibilityNorthSouth(const SGPSector& sSector) const;
	uint8_t getTraversibilityThrough(const SGPSector& sSector) const;
	uint8_t getTravelRating(const SGPSector& sSector) const;
	static MovementCostsModel* deserialize(const JsonValue& root, const TraversibilityMap& mapping);

protected:
	IntIntVector traverseWE;
	IntIntVector traverseNS;
	IntIntVector traverseThrough;
	IntIntVector travelRatings;
};

