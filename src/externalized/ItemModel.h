#pragma once

#include <string>

#include "game/Tactical/Item_Types.h"

class JsonObject;
class JsonObjectReader;
struct MagazineModel;
struct WeaponModel;

struct ItemModel
{
	ItemModel(
		uint16_t itemIndex,
		const char* internalName,
		uint32_t usItemClass,
		uint8_t classIndex=0,
		ItemCursor cursor=INVALIDCURS);

	ItemModel(
		uint16_t   itemIndex,
		const char* internalName,
		uint32_t   usItemClass,
		uint8_t    ubClassIndex,
		ItemCursor ubCursor,
		uint8_t    ubGraphicType,
		uint8_t    ubGraphicNum,
		uint8_t    ubWeight,
		uint8_t    ubPerPocket,
		uint16_t   usPrice,
		uint8_t    ubCoolness,
		int8_t     bReliability,
		int8_t     bRepairEase,
		uint16_t   fFlags);

	// This could be default in C++11
	virtual ~ItemModel();

	const virtual std::string& getInternalName() const;

	virtual uint16_t        getItemIndex() const;
	virtual uint32_t        getItemClass() const;
	virtual uint8_t         getClassIndex() const;
	virtual ItemCursor      getCursor() const;
	virtual uint8_t         getGraphicType() const;
	virtual uint8_t         getGraphicNum() const;
	virtual uint8_t         getWeight() const;
	virtual uint8_t         getPerPocket() const;
	virtual uint16_t        getPrice() const;
	virtual uint8_t         getCoolness() const;
	virtual int8_t          getReliability() const;
	virtual int8_t          getRepairEase() const;
	virtual uint16_t        getFlags() const;

	virtual bool isAmmo() const;
	virtual bool isArmour() const;
	virtual bool isBlade() const;
	virtual bool isBomb() const;
	virtual bool isExplosive() const;
	virtual bool isFace() const;
	virtual bool isGrenade() const;
	virtual bool isGun() const;
	virtual bool isKey() const;
	virtual bool isKit() const;
	virtual bool isLauncher() const;
	virtual bool isMedkit() const;
	virtual bool isMisc() const;
	virtual bool isMoney() const;
	virtual bool isPunch() const;
	virtual bool isTentacles() const;
	virtual bool isThrowingKnife() const;
	virtual bool isThrown() const;
	virtual bool isWeapon() const;

	virtual bool isTwoHanded() const;
	virtual bool isInBigGunList() const;

	virtual const WeaponModel* asWeapon() const   { return NULL; }
	virtual const MagazineModel* asAmmo() const   { return NULL; }

	/** Check if the given attachment can be attached to the item. */
	virtual bool canBeAttached(uint16_t attachment) const;

protected:
	uint16_t   itemIndex;
	std::string internalName;
	uint32_t   usItemClass;
	uint8_t    ubClassIndex;
	ItemCursor ubCursor;
	uint8_t    ubGraphicType;
	uint8_t    ubGraphicNum;
	uint8_t    ubWeight; //2 units per kilogram; roughly 1 unit per pound
	uint8_t    ubPerPocket;
	uint16_t   usPrice;
	uint8_t    ubCoolness;
	int8_t     bReliability;
	int8_t     bRepairEase;
	uint16_t   fFlags;

	void serializeFlags(JsonObject &obj) const;
	uint32_t deserializeFlags(JsonObjectReader &obj) const;
};
