#pragma once

#include "JA2Types.h"
#include "LoadSaveMercProfile.h"
#include "Types.h"

#include <string_theory/string>

#include <map>

class BinaryData {
	public:
		BinaryData();

		const ST::string& getItemShortName(uint32_t itemIndex) const;
		const ST::string& getItemName(uint32_t itemIndex) const;
		const ST::string& getItemDescription(uint32_t itemIndex) const;

		const MERCPROFILESTRUCT* getProfile(ProfileID profileIndex) const;

		static BinaryData deserialize(SGPFile* itemsFile, SGPFile* profilesFile);
		static ST::string itemsFilename();
		static ST::string profilesFilename();

	private:
		struct VanillaItem {
			ST::string shortName;
			ST::string name;
			ST::string description;
		};

		std::map<uint32_t, VanillaItem> items;
		std::vector<std::unique_ptr<const MERCPROFILESTRUCT>> profiles;
};
