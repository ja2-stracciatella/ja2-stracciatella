#pragma once

#include "ItemModel.h"
#include "ItemStrings.h"

#include <string_theory/string>

#include <map>
#include <stdint.h>

class JsonObject;
class JsonObjectReader;
struct AmmoTypeModel;
struct CalibreModel;

struct MagazineModel : ItemModel
{
	MagazineModel(ItemId itemIndex,
			ST::string internalName,
			ST::string shortName,
			ST::string name,
			ST::string description,
			uint32_t  itemClass,
			const CalibreModel *calibre,
			uint16_t capacity,
			const AmmoTypeModel *ammoType,
			bool dontUseAsDefaultMagazine
	);

	virtual const MagazineModel* asAmmo() const   { return this; }

	virtual void serializeTo(JsonObject &obj) const;

	static MagazineModel* deserialize(JsonObjectReader &obj,
						const std::map<ST::string, const CalibreModel*> &calibreMap,
						const std::map<ST::string, const AmmoTypeModel*> &ammoTypeMap,
						const VanillaItemStrings& vanillaItemStrings);


	/** Get standard replacement ammo name. */
	virtual const ST::string & getStandardReplacement() const;

	ST::string standardReplacement;
	const CalibreModel *calibre;
	const uint16_t capacity;
	const AmmoTypeModel *ammoType;
	const bool dontUseAsDefaultMagazine;

};
