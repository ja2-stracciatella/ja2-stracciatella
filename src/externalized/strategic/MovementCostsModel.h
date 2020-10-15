#pragma once

#include "JsonObject.h"
#include "TraversibilityMapping.h"
#include <vector>

typedef std::vector<std::vector<uint8_t>> IntIntVector;

class MovementCostsModel
{
public:
	MovementCostsModel(IntIntVector traverseWE_, IntIntVector traverseNS_, IntIntVector traverseThrough_, IntIntVector travelRatings_);

	const uint8_t getTraversibilityWestEast(uint8_t x, uint8_t y) const;
	const uint8_t getTraversibilityNorthSouth(uint8_t x, uint8_t y) const;
	const uint8_t getTraversibilityThrough(uint8_t x, uint8_t y) const;
	const uint8_t getTravelRating(uint8_t x, uint8_t y) const;
	static MovementCostsModel* deserialize(const rapidjson::Document& root, const TraversibilityMap& mapping);

protected:
	IntIntVector traverseWE;
	IntIntVector traverseNS;
	IntIntVector traverseThrough;
	IntIntVector travelRatings;
};

