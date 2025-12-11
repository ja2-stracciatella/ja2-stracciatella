#pragma once

#include "JA2Types.h"
#include "Types.h"

#include <string_theory/string>

class BinaryProfileData {
	public:
		BinaryProfileData();

		const MERCPROFILESTRUCT* getProfile(ProfileID profileIndex) const;

		static BinaryProfileData deserialize(SGPFile* profilesFile);
		static ST::string itemsFilename();
		static ST::string profilesFilename();

	private:
		std::vector<std::unique_ptr<const MERCPROFILESTRUCT>> profiles;
};
