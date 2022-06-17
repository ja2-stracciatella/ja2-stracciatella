#pragma once

#include "Types.h"

#include <string_theory/string>

#include <map>

class VanillaItemStrings {
	public:
		VanillaItemStrings();

		const ST::string& getShortName(uint32_t itemIndex) const;
		const ST::string& getName(uint32_t itemIndex) const;
		const ST::string& getDescription(uint32_t itemIndex) const;

		static VanillaItemStrings deserialize(SGPFile* file);
		static ST::string filename();

	private:
		struct VanillaItem {
			ST::string shortName;
			ST::string name;
			ST::string description;
		};

		std::map<uint32_t, VanillaItem> items;
};
