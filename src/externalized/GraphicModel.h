#pragma once

#include "Json.h"

#include <string_theory/string>

class GraphicModel {
	public:
		GraphicModel(const ST::string path_, const uint16_t subImageIndex);

		const ST::string& getPath() const noexcept { return path; };
		uint16_t getSubImageIndex() const noexcept { return subImageIndex; };

		static GraphicModel deserialize(const JsonValue &json);
		JsonValue serialize() const;
	private:
		ST::string path;
		uint16_t subImageIndex;
};
