#pragma once
#include "ContentManager.h"
#include "TranslatableString.h"
#include "JA2Types.h"

enum class MercType : int8_t;
class MercProfileInfo;
struct MERCPROFILESTRUCT;

/**
 * This class ia a thin wrapper around MERCPROFILESTRUCT and MercProfileInfo.
 *
 * ContentManager holds a copy of the all MercProfiles, but since this class only contains
 * pointers and index to the actual data, it is also OK to construct a new temporary
 * instance from ProfileID or MERCPROFILESTRUCT.
 */
class MercProfile
{
public:
	explicit MercProfile(ProfileID profileID);
	explicit MercProfile(const MercProfileInfo *info);
	MercProfile(MERCPROFILESTRUCT& p);

	// returns ProfileID which is the index to the gMercProfiles array
	ProfileID getID() const;

	// Returns if the RPC is currently recruited into our team. Always returns false if character is not RPC
	bool isRecruited() const;
	bool isForcedNPCQuote() const;

	// Can this characters be hired by the player on AIM or MERC or IMP (excludes RPCs)?
	bool isPlayerMerc() const;

	bool isAIMMerc() const;
	bool isMERCMerc() const;
	bool isIMPMerc() const;

	// Can the character be recruited (when conditions are met)?
	bool isRPC() const;

	// Is it an NPC character that can never be recruited (i.e. excluding RPCs)?
	bool isNPC() const;

	bool isNPCorRPC() const;
	bool isVehicle() const;

	// returns a reference to the MercProfileInfo which holds some
	// supplementary read-only data. It is preferable to use functions
	// in this class than using using MercProfileInfo directly.
	const MercProfileInfo& getInfo() const;

	// returns a reference MERCPROFILESTRUCT, which holds data from data
	// files (PROF.dat) and some other modifiable fields.
	MERCPROFILESTRUCT& getStruct() const;

	// allow implicit conversion to MERCPROFILESTRUCT. Same as getStruct()
	operator MERCPROFILESTRUCT&() const;

	JsonValue serializeStruct(const ContentManager* contentManager) const;
	JsonValue serializeStructRelations(const ContentManager* contentManager) const;

	// Extract merc profile from the json data.
	static std::unique_ptr<MERCPROFILESTRUCT> deserializeStruct(const MERCPROFILESTRUCT* binaryProf, TranslatableString::Loader& stringLoader, const JsonObject& json, const ContentManager* contentManager);
	// Extract relations tables for merc profiles from the json data.
	static void deserializeStructRelations(const MERCPROFILESTRUCT* binaryProf, MERCPROFILESTRUCT* prof, const JsonObject& json, const ContentManager* contentManager);

protected:
	const ProfileID m_profileID;
	MERCPROFILESTRUCT *m_profile;
};
