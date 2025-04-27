#include "Directories.h"
#include "EncryptedString.h"
#include "GameRes.h"
#include "ItemStrings.h"
#include "Tactical_Save.h"

#define VANILLA_MAX_ITEMS    351
#define SIZE_ITEM_NAME        80
#define SIZE_SHORT_ITEM_NAME  80
#define SIZE_ITEM_INFO       240

BinaryData::BinaryData() {}

// Needed to be able to return a reference to an empty string which is not local
const ST::string EMPTY_STRING = "";

const ST::string& BinaryData::getItemShortName(uint32_t itemIndex) const {
	auto found = items.find(itemIndex);
	if (found == items.end()) {
		return EMPTY_STRING;
	}
	return (*found).second.shortName;
}

const ST::string& BinaryData::getItemName(uint32_t itemIndex) const {
	auto found = items.find(itemIndex);
	if (found == items.end()) {
		return EMPTY_STRING;
	}
	return (*found).second.name;
}

const ST::string& BinaryData::getItemDescription(uint32_t itemIndex) const {
	auto found = items.find(itemIndex);
	if (found == items.end()) {
		return EMPTY_STRING;
	}
	return (*found).second.description;
};

const MERCPROFILESTRUCT* BinaryData::getProfile(ProfileID profileIndex) const
{
	// This is to avoid failures due to prof.dat unavailability during unit-testing
	if (profiles.size() == 0) {
		static const MERCPROFILESTRUCT* DEFAULT_PROFILE = new MERCPROFILESTRUCT();
		return DEFAULT_PROFILE;
	}

	return profiles[profileIndex].get();
}

ST::string BinaryData::itemsFilename() {
	return BINARYDATADIR "/itemdesc.edt";
}

ST::string BinaryData::profilesFilename() {
	return BINARYDATADIR "/prof.dat";
}

BinaryData BinaryData::deserialize(SGPFile* itemsFile, SGPFile* profilesFile) {
	auto binData = BinaryData();
	uint16_t index = 0;

	while (true) {
		try {
			uint32_t seek = (SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * index;

			auto shortName = LoadEncryptedString(itemsFile, seek, SIZE_SHORT_ITEM_NAME);
			auto name = LoadEncryptedString(itemsFile, seek + SIZE_SHORT_ITEM_NAME, SIZE_ITEM_NAME);
			auto description = LoadEncryptedString(itemsFile, seek + SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME, SIZE_ITEM_INFO);

			binData.items.emplace(std::make_pair(index, VanillaItem{ std::move(shortName), std::move(name), std::move(description) }));

			index++;
		}
		catch (const std::runtime_error& error) {
			if (index <= VANILLA_MAX_ITEMS - 1) {
				SLOGE("Could not read itemdesc.edt to completion: {}", error.what());
			}
			break;
		}
	}

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
