#pragma once

enum class SpeckQuoteType
{
	ADVERTISE,
	MERC_DEAD,
	CROSS_SELL
};

struct MERCSpeckQuote
{
	MERCSpeckQuote(uint8_t q, SpeckQuoteType t, uint8_t r) : quoteID(q), type(t), relatedMercID(r) {}

	const uint8_t quoteID;
	const SpeckQuoteType type;
	const uint8_t relatedMercID;
};
