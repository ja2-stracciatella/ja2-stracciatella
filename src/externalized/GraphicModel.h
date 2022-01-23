#pragma once

#include "JsonObject.h"

#include <string_theory/string>

class GraphicModel {
	public:
		GraphicModel(const ST::string path_, const uint8_t subImageIndex);

		const ST::string& getPath() const;
		uint8_t getSubImageIndex() const;

		static GraphicModel deserialize(JsonObjectReader &obj);
		JsonObject serialize(rapidjson::Document::AllocatorType& allocator) const;
	private:
		ST::string path;
		uint8_t subImageIndex;
};
