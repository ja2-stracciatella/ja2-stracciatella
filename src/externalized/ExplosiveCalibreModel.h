#pragma once

#include "Json.h"

class ExplosiveCalibreModel {
	public:
		ExplosiveCalibreModel(uint16_t id, ST::string&& name);

		uint16_t getID() const;
		const ST::string& getName() const;

		static ExplosiveCalibreModel* deserialize(uint16_t id, const JsonValue &json);

	private:
		uint16_t id;
		ST::string name;
};
