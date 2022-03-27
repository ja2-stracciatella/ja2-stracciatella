#ifndef __WORLD_ITEMS
#define __WORLD_ITEMS

#include "Debug.h"
#include "Item_Types.h"

#include <vector>


#define WORLD_ITEM_DONTRENDER				0x0001
#define WORLD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO		0x0002
#define WORLD_ITEM_ARMED_BOMB				0x0040
#define WORLD_ITEM_SCIFI_ONLY				0x0080
#define WORLD_ITEM_REALISTIC_ONLY			0x0100
#define WORLD_ITEM_REACHABLE				0x0200
#define WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT	0x0400

struct WORLDITEM
{
	BOOLEAN    fExists;
	INT16      sGridNo;
	UINT8      ubLevel;
	OBJECTTYPE o;
	UINT16     usFlags;
	INT8       bRenderZHeightAboveLevel;

	INT8       bVisible;

	//This is the chance associated with an item or a trap not-existing in the world.  The reason why
	//this is reversed (10 meaning item has 90% chance of appearing, is because the order that the map
	//is saved, we don't know if the version is older or not until after the items are loaded and added.
	//Because this value is zero in the saved maps, we can't change it to 100, hence the reversal method.
	//This check is only performed the first time a map is loaded.  Later, it is entirely skipped.
	UINT8      ubNonExistChance;
};


// items in currently loaded sector
extern std::vector<WORLDITEM> gWorldItems;

static inline WORLDITEM& GetWorldItem(size_t const idx)
{
	Assert(idx < gWorldItems.size());
	return gWorldItems[idx];
}

#define BASE_FOR_EACH_WORLD_ITEM(type, iter) \
	for (type& iter : gWorldItems) \
		if (!iter.fExists) continue; else
#define FOR_EACH_WORLD_ITEM( iter) BASE_FOR_EACH_WORLD_ITEM(      WORLDITEM, iter)
#define CFOR_EACH_WORLD_ITEM(iter) BASE_FOR_EACH_WORLD_ITEM(const WORLDITEM, iter)

INT32 AddItemToWorld(INT16 sGridNo, const OBJECTTYPE* pObject, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT8 bVisible);
void RemoveItemFromWorld( INT32 iItemIndex );
INT32 FindWorldItem( UINT16 usItem );

void LoadWorldItemsFromMap(HWFILE);

void SaveWorldItemsToMap( HWFILE fp );

void TrashWorldItems(void);

struct WORLDBOMB
{
	BOOLEAN fExists;
	INT32   iItemIndex;
};

extern std::vector<WORLDBOMB> gWorldBombs;

#define BASE_FOR_EACH_WORLD_BOMB(type, iter) \
	for (type& iter : gWorldBombs) \
		if (!iter.fExists) continue; else
#define FOR_EACH_WORLD_BOMB( iter) BASE_FOR_EACH_WORLD_BOMB(      WORLDBOMB, iter)
#define CFOR_EACH_WORLD_BOMB(iter) BASE_FOR_EACH_WORLD_BOMB(const WORLDBOMB, iter)

extern void FindPanicBombsAndTriggers( void );
extern INT32 FindWorldItemForBombInGridNo( INT16 sGridNo, INT8 bLevel);

void RefreshWorldItemsIntoItemPools(const std::vector<WORLDITEM>& items);

#endif
