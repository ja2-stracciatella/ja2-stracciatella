#ifndef __INTERFACE_ITEMS_H
#define __INTERFACE_ITEMS_H

#include "Button_System.h"
#include "Interface.h"
#include "MouseSystem.h"
#include "Soldier_Control.h"

#include "UILayout.h"

#include <string_theory/string>
#include <utility>


struct ItemModel;


// DEFINES FOR ITEM SLOT SIZES IN PIXELS
#define BIG_INV_SLOT_WIDTH	61
#define BIG_INV_SLOT_HEIGHT	22
#define SM_INV_SLOT_WIDTH	30
#define SM_INV_SLOT_HEIGHT	23
#define VEST_INV_SLOT_WIDTH	43
#define VEST_INV_SLOT_HEIGHT	24
#define LEGS_INV_SLOT_WIDTH	43
#define LEGS_INV_SLOT_HEIGHT	24
#define HEAD_INV_SLOT_WIDTH	43
#define HEAD_INV_SLOT_HEIGHT	24


// Itempickup stuff
void InitializeItemPickupMenu(SOLDIERTYPE* pSoldier, INT16 sGridNo, ITEM_POOL* pItemPool, INT8 bZLevel);
void RenderItemPickupMenu(void);
void RemoveItemPickupMenu(void);
void SetItemPickupMenuDirty( BOOLEAN fDirtyLevel );
BOOLEAN HandleItemPickupMenu(void);


// FUNCTIONS FOR INTERFACEING WITH ITEM PANEL STUFF
void InitInvSlotInterface(INV_REGION_DESC const* pRegionDesc, INV_REGION_DESC const* pCamoRegion, MOUSE_CALLBACK INVMoveCallback, MOUSE_CALLBACK INVClickCallback, MOUSE_CALLBACK INVMoveCamoCallback, MOUSE_CALLBACK INVClickCamoCallback);
void ShutdownInvSlotInterface();
void HandleRenderInvSlots(SOLDIERTYPE const&, DirtyLevel);
void HandleNewlyAddedItems(SOLDIERTYPE&, DirtyLevel*);
void RenderInvBodyPanel(const SOLDIERTYPE* pSoldier, INT16 sX, INT16 sY);
void DisableInvRegions( BOOLEAN fDisable );

void DegradeNewlyAddedItems(void);
void CheckForAnyNewlyAddedItems( SOLDIERTYPE *pSoldier );


BOOLEAN HandleCompatibleAmmoUI(const SOLDIERTYPE* pSoldier, INT8 bInvPos, BOOLEAN fOn);


// THIS FUNCTION IS CALLED TO RENDER AN ITEM.
// uiBuffer - The Dest Video Surface - can only be FRAME_BUFFER or guiSAVEBUFFER
// pSoldier - used for determining whether burst mode needs display
// pObject  - Usually taken from pSoldier->inv[HANDPOS]
// sX, sY, Width, Height, - Will Center it in the Width
// fDirtyLevel  if == DIRTYLEVEL2 will render everything
//              if == DIRTYLEVEL1 will render bullets and status only
//
//  Last parameter used mainly for when mouse is over item
void INVRenderItem(SGPVSurface* uiBuffer, SOLDIERTYPE const* pSoldier, OBJECTTYPE const&, INT16 sX, INT16 sY, INT16 sWidth, INT16 sHeight, DirtyLevel, UINT8 ubStatusIndex, UINT32 sOutlineColor);


extern BOOLEAN gfInItemDescBox;

BOOLEAN InItemDescriptionBox(void);
void InitItemDescriptionBox(SOLDIERTYPE* pSoldier, UINT8 ubPosition, INT16 sX, INT16 sY, UINT8 ubStatusIndex);
void InternalInitItemDescriptionBox(OBJECTTYPE* pObject, INT16 sX, INT16 sY, UINT8 ubStatusIndex, SOLDIERTYPE* pSoldier);
void InitKeyItemDescriptionBox(SOLDIERTYPE* pSoldier, UINT8 ubPosition, INT16 sX, INT16 sY);
void RenderItemDescriptionBox(void);
void HandleItemDescriptionBox(DirtyLevel*);
void DeleteItemDescriptionBox(void);


BOOLEAN InItemStackPopup(void);
void    InitItemStackPopup(SOLDIERTYPE* pSoldier, UINT8 ubPosition, INT16 sInvX, INT16 sInvY, INT16 sInvWidth, INT16 sInvHeight);
void RenderItemStackPopup( BOOLEAN fFullRender );


// keyring handlers
void InitKeyRingPopup(SOLDIERTYPE* pSoldier, INT16 sInvX, INT16 sInvY, INT16 sInvWidth, INT16 sInvHeight);
void RenderKeyRingPopup( BOOLEAN fFullRender );
void InitKeyRingInterface( MOUSE_CALLBACK KeyRingClickCallback );
void InitMapKeyRingInterface( MOUSE_CALLBACK KeyRingClickCallback );
void DeleteKeyRingPopup(void);


void ShutdownKeyRingInterface( void );
BOOLEAN InKeyRingPopup( void );
void BeginKeyRingItemPointer( SOLDIERTYPE *pSoldier, UINT8 ubKeyRingPosition );


extern OBJECTTYPE*  gpItemPointer;
extern OBJECTTYPE   gItemPointer;
extern SOLDIERTYPE* gpItemPointerSoldier;
extern BOOLEAN      gfItemPointerDifferentThanDefault;


void BeginItemPointer( SOLDIERTYPE *pSoldier, UINT8 ubHandPos );
void InternalBeginItemPointer( SOLDIERTYPE *pSoldier, OBJECTTYPE *pObject, INT8 bHandPos );
void EndItemPointer(void);
void DrawItemFreeCursor(void);
void DrawItemTileCursor(void);
BOOLEAN HandleItemPointerClick( UINT16 usMapPos );
UINT32 GetAttachmentHintColor(const OBJECTTYPE* pObj);
std::pair<const SGPVObject*, UINT8> GetSmallInventoryGraphicForItem(const ItemModel *item);
std::pair<SGPVObject*, UINT8> GetBigInventoryGraphicForItem(const ItemModel *item);
UINT16            GetTileGraphicForItem(const ItemModel *item);

ST::string GetHelpTextForItem(const OBJECTTYPE& obj);

void CancelItemPointer(void);

void LoadItemCursorFromSavedGame(HWFILE);
void SaveItemCursorToSavedGame(HWFILE);

// handle compatable items for merc and map inventory
BOOLEAN HandleCompatibleAmmoUIForMapScreen(const SOLDIERTYPE* pSoldier, INT32 bInvPos, BOOLEAN fOn, BOOLEAN fFromMerc);
BOOLEAN HandleCompatibleAmmoUIForMapInventory( SOLDIERTYPE *pSoldier, INT32 bInvPos, INT32 iStartSlotNumber, BOOLEAN fOn, BOOLEAN fFromMerc  );
void ResetCompatibleItemArray();

void CycleItemDescriptionItem(void);

void UpdateItemHatches(void);

extern BOOLEAN      gfInKeyRingPopup;
extern BOOLEAN      gfInItemPickupMenu;
extern SOLDIERTYPE* gpItemPopupSoldier;
extern INT8         gbCompatibleApplyItem;
extern INT8         gbInvalidPlacementSlot[NUM_INV_SLOTS];
extern MOUSE_REGION gInvDesc;
extern BOOLEAN      gfAddingMoneyToMercFromPlayersAccount;
extern MOUSE_REGION gItemDescAttachmentRegions[MAX_ATTACHMENTS];
extern INT8         gbItemPointerSrcSlot;
extern BOOLEAN      gfDontChargeAPsToPickup;
extern GUIButtonRef giMapInvDescButton;

void    HandleAnyMercInSquadHasCompatibleStuff(const OBJECTTYPE* pObject);
BOOLEAN InternalHandleCompatibleAmmoUI(const SOLDIERTYPE* pSoldier, const OBJECTTYPE* pTestObject, BOOLEAN fOn);

void SetMouseCursorFromItem(UINT16 item_idx);
void SetMouseCursorFromCurrentItem();

void SetItemPointer(OBJECTTYPE*, SOLDIERTYPE*);

void LoadInterfaceItemsGraphics();
void DeleteInterfaceItemsGraphics();

#endif
