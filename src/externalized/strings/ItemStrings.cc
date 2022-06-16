#include "ItemStrings.h"
#include "EncryptedString.h"
#include "Directories.h"

#define VANILLA_MAX_ITEMS    351
#define SIZE_ITEM_NAME        80
#define SIZE_SHORT_ITEM_NAME  80
#define SIZE_ITEM_INFO       240

VanillaItemStrings::VanillaItemStrings() {}

const ST::string& VanillaItemStrings::getShortName(uint32_t itemIndex) const {
	return items.at(itemIndex).shortName;
}

const ST::string& VanillaItemStrings::getName(uint32_t itemIndex) const {
	return items.at(itemIndex).name;
}

const ST::string& VanillaItemStrings::getDesciption(uint32_t itemIndex) const {
	return items.at(itemIndex).description;
};

ST::string VanillaItemStrings::filename() {
	return BINARYDATADIR "/itemdesc.edt";
}

VanillaItemStrings VanillaItemStrings::deserialize(SGPFile* file) {
	auto itemStrings = VanillaItemStrings();
	uint16_t index = 0;

	while (true) {
		try {
			uint32_t seek = (SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * index;

			auto shortName = LoadEncryptedString(file, seek, SIZE_SHORT_ITEM_NAME);
			auto name = LoadEncryptedString(file, seek + SIZE_SHORT_ITEM_NAME, SIZE_ITEM_NAME);
			auto description = LoadEncryptedString(file, seek + SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME, SIZE_ITEM_INFO);

			itemStrings.items.emplace(std::make_pair(index, VanillaItem{
				shortName: shortName,
				name: name,
				description: description
			}));

			index++;
		} catch (const std::runtime_error& error) {
			if (index <= VANILLA_MAX_ITEMS - 1) {
				SLOGE("Could not read itemdesc.edt to completion: {}", error.what());
			}
			break;
		}
	}

	return itemStrings;
}
