#ifndef __EDITORITEMS_H
#define __EDITORITEMS_H

#include "EditorDefines.h"


struct EditorItemsInfo
{
	BOOLEAN fGameInit;	//Used for initializing save variables the first time.
	BOOLEAN fActive;		//currently active
	ItemId *pusItemIndex;		//a dynamic array of Item indices
	SGPVSurface* uiBuffer;
	ToolbarMode  uiItemType; //Weapons, ammo, armour, explosives, equipment
	INT16   sNumItems;	//total number of items in the current class of item.
	INT16		sSelItemIndex;	//currently selected item index.
	INT16   sHilitedItemIndex;
	INT16   sScrollIndex;	//current scroll index (0 is far left, 1 is next tile to the right, ...)
	INT16   sSaveSelWeaponsIndex, sSaveSelAmmoIndex, sSaveSelArmourIndex, sSaveSelExplosivesIndex,
					sSaveSelEquipment1Index, sSaveSelEquipment2Index, sSaveSelEquipment3Index,
					sSaveSelTriggersIndex, sSaveSelKeysIndex;
	INT16   sSaveWeaponsScrollIndex, sSaveAmmoScrollIndex, sSaveArmourScrollIndex, sSaveExplosivesScrollIndex,
					sSaveEquipment1ScrollIndex, sSaveEquipment2ScrollIndex, sSaveEquipment3ScrollIndex,
					sSaveTriggersScrollIndex, sSaveKeysScrollIndex;
	INT16   sNumWeapons, sNumAmmo, sNumArmour, sNumExplosives, sNumEquipment1, sNumEquipment2, sNumEquipment3,
					sNumTriggers, sNumKeys;
};

extern EditorItemsInfo eInfo;

void EntryInitEditorItemsInfo(void);
void InitEditorItemsInfo(ToolbarMode uiItemType);
void RenderEditorItemsInfo(void);
void ClearEditorItemsInfo(void);
void DisplayItemStatistics(void);
void DetermineItemsScrolling(void);

//User actions
void AddSelectedItemToWorld( INT16 sGridNo );
void HandleRightClickOnItem( INT16 sGridNo );
void DeleteSelectedItem(void);
void ShowSelectedItem(void);
void HideSelectedItem(void);
void SelectNextItemPool(void);
void SelectNextItemInPool(void);
void SelectPrevItemInPool(void);

void KillItemPoolList(void);
void BuildItemPoolList(void);

void HandleItemsPanel( UINT16 usScreenX, UINT16 usScreenY, INT8 bEvent );

extern INT32 giDefaultExistChance;

extern ITEM_POOL* gpItemPool;

#endif
