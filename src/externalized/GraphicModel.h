#pragma once

#include "Json.h"

#include <string_theory/string>

class GraphicModel {
	public:
		GraphicModel(const ST::string path_, const uint8_t subImageIndex);

		const ST::string& getPath() const;
		uint8_t getSubImageIndex() const;

		static GraphicModel deserialize(const JsonValue &json);
		JsonValue serialize() const;
	private:
		ST::string path;
		uint8_t subImageIndex;
};
