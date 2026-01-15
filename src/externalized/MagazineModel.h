#pragma once

#include "Containers.h"
#include "ItemModel.h"
#include "TranslatableString.h"
#include "Views.h"

#include <cstdint>
#include <string_theory/string>

class JsonObject;
class JsonObject;
struct AmmoTypeModel;
struct CalibreModel;

struct MagazineModel : public ItemModel
{
	static constexpr const char* ENTITY_NAME = "Magazine";

	MagazineModel(uint16_t itemIndex,
			ST::string&& internalName,
			ST::string&& shortName,
			ST::string&& name,
			ST::string&& description,
			ST::string&& bobbyRaysName,
			ST::string&& bobbyRaysDescription,
			uint32_t  itemClass,
			const CalibreModel *calibre,
			uint16_t capacity,
			const AmmoTypeModel *ammoType,
			bool dontUseAsDefaultMagazine
	);

	virtual const MagazineModel* asAmmo() const   { return this; }

	virtual JsonValue serialize() const;

	static std::unique_ptr<MagazineModel> deserialize(const JsonValue &json,
						const Containers::Named<uint16_t, CalibreModel>& calibres,
						const Containers::Named<uint16_t, AmmoTypeModel>& ammoTypes,
						TranslatableString::Loader& stringLoader);


	/** Get standard replacement ammo name. */
	virtual const ST::string & getStandardReplacement() const;

	ST::string standardReplacement;
	const CalibreModel *calibre;
	const uint16_t capacity;
	const AmmoTypeModel *ammoType;
	const bool dontUseAsDefaultMagazine;

};

class MagazinesContainer : public Containers::Views::Named<uint16_t, MagazineModel, ItemModel> {
	public:
		MagazinesContainer() = default;
		MagazinesContainer(const ItemsContainer& items) :
			Containers::Views::Named<uint16_t, MagazineModel, ItemModel>(items.begin(), items.end(), [](const ItemModel* entity) { return entity->asAmmo(); }) {}
};
