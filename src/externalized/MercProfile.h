#pragma once

#include <stdint.h>

struct MERCPROFILESTRUCT;

class MercProfile
{
public:
	MercProfile(uint16_t profileNum);

	uint16_t getNum() const;

	bool isRecruited() const;
	bool isForcedNPCQuote() const;

protected:
	const uint16_t m_profileNum;
	MERCPROFILESTRUCT *m_profile;
};
