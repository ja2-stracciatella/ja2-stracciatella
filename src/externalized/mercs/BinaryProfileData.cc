#include "BinaryProfileData.h"
#include "Directories.h"
#include "GameRes.h"
#include "LoadSaveMercProfile.h"
#include "Tactical_Save.h"

#define VANILLA_MAX_ITEMS    351
#define SIZE_ITEM_NAME        80
#define SIZE_SHORT_ITEM_NAME  80
#define SIZE_ITEM_INFO       240

BinaryProfileData::BinaryProfileData() {}

const MERCPROFILESTRUCT* BinaryProfileData::getProfile(ProfileID profileIndex) const
{
	// This is to avoid failures due to prof.dat unavailability during unit-testing
	if (profiles.empty()) {
		static const MERCPROFILESTRUCT DEFAULT_PROFILE{};
		return &DEFAULT_PROFILE;
	}

	return profiles[profileIndex].get();
}

ST::string BinaryProfileData::profilesFilename() {
	return BINARYDATADIR "/prof.dat";
}

BinaryProfileData BinaryProfileData::deserialize(SGPFile* profilesFile) {
	auto binData = BinaryProfileData();

	binData.profiles.resize(NUM_PROFILES);
	bool const isCorrectlyEncoded = !(isRussianVersion() || isRussianGoldVersion());
	for (ProfileID profileID = 0; profileID != NUM_PROFILES; ++profileID) {
		BYTE data[MERC_PROFILE_SIZE];
		JA2EncryptedFileRead(profilesFile, data, sizeof(data));
		auto prof = std::make_unique<MERCPROFILESTRUCT>();
		UINT32 checksum;
		ExtractMercProfile(data, *prof, false, &checksum, isCorrectlyEncoded);
		// not checking the checksum
		binData.profiles[profileID] = std::move(prof);
	}

	return binData;
}
