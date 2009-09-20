#ifndef __WORLD_ITEMS
#define __WORLD_ITEMS

#include "Debug.h"
#include "Item_Types.h"


#define	WORLD_ITEM_DONTRENDER												0x0001
#define	WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO				0x0002
#define WORLD_ITEM_ARMED_BOMB												0x0040
#define WORLD_ITEM_SCIFI_ONLY												0x0080
#define WORLD_ITEM_REALISTIC_ONLY										0x0100
#define WORLD_ITEM_REACHABLE												0x0200
#define WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT		0x0400

struct WORLDITEM
{
	BOOLEAN				fExists;
	INT16					sGridNo;
	UINT8					ubLevel;
	OBJECTTYPE		o;
	UINT16				usFlags;
	INT8					bRenderZHeightAboveLevel;

	INT8					bVisible;

	//This is the chance associated with an item or a trap not-existing in the world.  The reason why
	//this is reversed (10 meaning item has 90% chance of appearing, is because the order that the map
	//is saved, we don't know if the version is older or not until after the items are loaded and added.
	//Because this value is zero in the saved maps, we can't change it to 100, hence the reversal method.
	//This check is only performed the first time a map is loaded.  Later, it is entirely skipped.
	UINT8					ubNonExistChance;
};
CASSERT(sizeof(WORLDITEM) == 52)


extern WORLDITEM		*gWorldItems;

// number of items in currently loaded sector
extern UINT32 guiNumWorldItems;

static inline WORLDITEM& GetWorldItem(size_t const idx)
{
	Assert(idx < guiNumWorldItems);
	return gWorldItems[idx];
}

#define BASE_FOR_EACH_WORLD_ITEM(type, iter)                     \
	for (type*       iter        = gWorldItems,                    \
	         * const end__##iter = gWorldItems + guiNumWorldItems; \
	     iter != end__##iter;                                      \
	     ++iter)                                                   \
		if (!iter->fExists) continue; else
#define FOR_EACH_WORLD_ITEM( iter) BASE_FOR_EACH_WORLD_ITEM(      WORLDITEM, iter)
#define CFOR_EACH_WORLD_ITEM(iter) BASE_FOR_EACH_WORLD_ITEM(const WORLDITEM, iter)

INT32 AddItemToWorld(INT16 sGridNo, const OBJECTTYPE* pObject, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel, INT8 bVisible);
void RemoveItemFromWorld( INT32 iItemIndex );
INT32 FindWorldItem( UINT16 usItem );

void LoadWorldItemsFromMap(HWFILE);

#ifdef JA2EDITOR
void SaveWorldItemsToMap( HWFILE fp );
#endif

void TrashWorldItems(void);

struct WORLDBOMB
{
	BOOLEAN				fExists;
	INT32					iItemIndex;
};

extern WORLDBOMB * gWorldBombs;
extern UINT32 guiNumWorldBombs;

#define BASE_FOR_ALL_WORLD_BOMBS(type, iter)                     \
	for (type*       iter        = gWorldBombs,                    \
	         * const end__##iter = gWorldBombs + guiNumWorldBombs; \
	     iter != end__##iter;                                      \
	     ++iter)                                                   \
		if (!iter->fExists) continue; else
#define FOR_ALL_WORLD_BOMBS( iter) BASE_FOR_ALL_WORLD_BOMBS(      WORLDBOMB, iter)
#define CFOR_ALL_WORLD_BOMBS(iter) BASE_FOR_ALL_WORLD_BOMBS(const WORLDBOMB, iter)

extern void FindPanicBombsAndTriggers( void );
extern INT32 FindWorldItemForBombInGridNo( INT16 sGridNo, INT8 bLevel);

void RefreshWorldItemsIntoItemPools(const WORLDITEM* pItemList, INT32 iNumberOfItems);

#endif
