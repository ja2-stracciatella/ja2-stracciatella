#include "MercProfile.h"
#include "MercProfileInfo.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"

MercProfile::MercProfile(ProfileID profileID)
	: m_profileID(profileID), m_profile(&gMercProfiles[profileID])
{
	if (m_profileID >= lengthof(gMercProfiles))
	{
		ST::string err = ST::format("invalid m_profileID '{}'", m_profileID);
		throw std::runtime_error(err.to_std_string());
	}
}

MercProfile::MercProfile(const MercProfileInfo* info)
	: MercProfile(info->profileID)
{
}

MercProfile::MercProfile(MERCPROFILESTRUCT& p)
	: MercProfile(static_cast<ProfileID>(&p - gMercProfiles))
{
}

ProfileID MercProfile::getID() const
{
	return m_profileID;
}

bool MercProfile::isRecruited() const
{
	return m_profile->ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED;
}

bool MercProfile::isForcedNPCQuote() const
{
	return m_profile->ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE;
}

const MercProfileInfo& MercProfile::getInfo() const
{
	return *(MercProfileInfo::load(m_profileID));
}

bool MercProfile::isAIMMerc() const
{
	return getInfo().mercType == MercType::AIM;
}

bool MercProfile::isMERCMerc() const
{
	return getInfo().mercType == MercType::MERC;
}

bool MercProfile::isIMPMerc() const
{
	return getInfo().mercType == MercType::IMP;
}

bool MercProfile::isRPC() const
{
	return getInfo().mercType == MercType::RPC;
}
bool MercProfile::isNPC() const
{
	return getInfo().mercType == MercType::NPC;
}

bool MercProfile::isNPCorRPC() const
{
	return isNPC() || isRPC();
}

bool MercProfile::isVehicle() const
{
	return getInfo().mercType == MercType::VEHICLE;
}

bool MercProfile::isPlayerMerc() const
{
	return isAIMMerc() || isMERCMerc() || isIMPMerc();
}

MERCPROFILESTRUCT &MercProfile::getStruct() const
{
	return *m_profile;
}

MercProfile::operator MERCPROFILESTRUCT&() const
{
	return *m_profile;
}