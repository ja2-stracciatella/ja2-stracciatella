#pragma once

#include "IEncodingCorrector.h"

/** Russian encoding corrector. */
class RussianEncodingCorrector : public IEncodingCorrector
{
public:
	/** Fix one UTF-16 code point. */
	virtual uint16_t fix(uint16_t codePoint) const;
};
