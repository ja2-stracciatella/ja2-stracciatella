#include "MercProfile.h"

#include "game/Tactical/Soldier_Profile.h"
#include "game/Tactical/Soldier_Profile_Type.h"

MercProfile::MercProfile(uint16_t profileNum)
	:m_profileNum(profileNum),
	m_profile(&gMercProfiles[profileNum])
{
}

uint16_t MercProfile::getNum() const
{
	return m_profileNum;
}


bool MercProfile::isRecruited() const
{
	return m_profile->ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED;
}

bool MercProfile::isForcedNPCQuote() const
{
	return m_profile->ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE;
}
