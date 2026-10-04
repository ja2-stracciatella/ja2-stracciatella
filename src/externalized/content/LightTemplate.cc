#include "LightTemplate.h"

std::unique_ptr<LightTemplate> LightTemplate::deserialize(const JsonValue& json)
{
	auto jTmpl = json.toObject();

	auto jRays = jTmpl["rays"].toVec();
	std::vector<uint16_t> rays;
	rays.reserve(jRays.size());
	std::transform(jRays.begin(), jRays.end(),
					std::back_inserter(rays),
					[](JsonValue& jVal) { return static_cast<uint16_t>(jVal.toUInt()); });

	auto jLights = jTmpl["lights"].toVec();
	std::vector<LIGHT_NODE> lights;
	lights.reserve(jLights.size());
	std::transform(jLights.begin(), jLights.end(),
					std::back_inserter(lights),
					[](JsonValue& jVal) {
						auto jLight = jVal.toVec();
						LIGHT_NODE lightNode;
						lightNode.iDX = static_cast<int16_t>(jLight[0].toInt());
						lightNode.iDY = static_cast<int16_t>(jLight[1].toInt());
						lightNode.uiFlags = static_cast<uint16_t>(jLight[2].toUInt());
						lightNode.ubLight = static_cast<uint8_t>(jLight[3].toUInt());
						return lightNode;
	});

	auto lightTmpl = std::make_unique<LightTemplate>();
	lightTmpl.get()->name = jTmpl.GetString("name");
	lightTmpl.get()->rays = std::move(rays);
	lightTmpl.get()->lights = std::move(lights);

	return lightTmpl;
}
