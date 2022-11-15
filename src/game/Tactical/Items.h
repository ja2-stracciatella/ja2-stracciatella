#ifndef ITEMS_H
#define ITEMS_H

#include <vector>

#include "Item_Types.h"
#include "JA2Types.h"
#include "Weapons.h"

struct CalibreModel;
struct ItemModel;
struct WeaponModel;

void DamageObj(OBJECTTYPE* pObj, INT8 bAmount);

extern UINT8 SlotToPocket[7];

BOOLEAN WeaponInHand(const SOLDIERTYPE* pSoldier);

INT8 FindObj(const SOLDIERTYPE* pSoldier, ItemId usItem);
INT8 FindAmmo(const SOLDIERTYPE*, const CalibreModel *, UINT8 ubMagSize, INT8 bExcludeSlot);

INT8 FindAttachment(const OBJECTTYPE* pObj, ItemId usItem);
INT8 FindObjClass(const SOLDIERTYPE* s, UINT32 usItemClass);
extern INT8 FindAIUsableObjClass( const SOLDIERTYPE * pSoldier, 	UINT32 usItemClass );
extern INT8 FindAIUsableObjClassWithin( const SOLDIERTYPE * pSoldier, 	UINT32 usItemClass, INT8 bLower, INT8 bUpper );
extern INT8 FindEmptySlotWithin( const SOLDIERTYPE * pSoldier, INT8 bLower, INT8 bUpper );
extern INT8 FindExactObj( const SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj );
INT8 FindObjInObjRange(const SOLDIERTYPE* s, ItemId usItem1, ItemId usItem2);
extern INT8 FindLaunchable( const SOLDIERTYPE * pSoldier, ItemId usWeapon );
extern INT8 FindGLGrenade( const SOLDIERTYPE * pSoldier );
extern INT8 FindThrowableGrenade( const SOLDIERTYPE * pSoldier );
extern INT8 FindUsableObj( const SOLDIERTYPE * pSoldier, ItemId usItem );

extern void DeleteObj(OBJECTTYPE * pObj );
extern void SwapObjs( OBJECTTYPE * pObj1, OBJECTTYPE * pObj2 );

extern void RemoveObjFrom( OBJECTTYPE * pObj, UINT8 ubRemoveIndex );
//Returns true if swapped, false if added to end of stack
extern BOOLEAN PlaceObjectAtObjectIndex( OBJECTTYPE * pSourceObj, OBJECTTYPE * pTargetObj, UINT8 ubIndex );
extern void GetObjFrom( OBJECTTYPE * pObj, UINT8 ubGetIndex, OBJECTTYPE * pDest );

bool AttachObject(SOLDIERTYPE* const s, OBJECTTYPE* const pTargetObj, OBJECTTYPE* const pAttachment, UINT8 const ubIndexInStack = 0);
extern BOOLEAN RemoveAttachment( OBJECTTYPE * pObj, INT8 bAttachPos, OBJECTTYPE * pNewObj );

UINT8	CalculateObjectWeight(const OBJECTTYPE* pObject);
UINT32 CalculateCarriedWeight(const SOLDIERTYPE* pSoldier);

extern UINT16 TotalPoints(const OBJECTTYPE*);
UINT16 UseKitPoints(OBJECTTYPE&, UINT16 points, SOLDIERTYPE const&);

extern BOOLEAN EmptyWeaponMagazine( OBJECTTYPE * pWeapon, OBJECTTYPE *pAmmo );
void CreateItem(ItemId usItem, INT8 bStatus, OBJECTTYPE*);
void CreateItems(ItemId usItem, INT8 bStatus, UINT8 ubNumber, OBJECTTYPE*);
void CreateMoney(UINT32 uiMoney, OBJECTTYPE*);
ItemId DefaultMagazine(ItemId gun);
ItemId RandomMagazine( ItemId usItem, UINT8 ubPercentStandard );
extern BOOLEAN ReloadGun( SOLDIERTYPE * pSoldier, OBJECTTYPE * pGun, OBJECTTYPE * pAmmo );

UINT8 ItemSlotLimit( ItemId usItem, INT8 bSlot );

// Function to put an item in a soldier profile
// It's very primitive, just finds an empty place!
BOOLEAN PlaceObjectInSoldierProfile( UINT8 ubProfile, OBJECTTYPE *pObject );
BOOLEAN RemoveObjectFromSoldierProfile( UINT8 ubProfile, ItemId usItem );
INT8 FindObjectInSoldierProfile(MERCPROFILESTRUCT const&, ItemId item_id);

void SetMoneyInSoldierProfile( UINT8 ubProfile, UINT32 uiMoney );

void CheckEquipmentForDamage( SOLDIERTYPE *pSoldier, INT32 iDamage );
BOOLEAN ArmBomb( OBJECTTYPE * pObj, INT8 bSetting );

// NOTE TO ANDREW:
//
// The following functions expect that pObj points to the object
// "in the cursor", which should have memory allocated for it already
BOOLEAN PlaceObject( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj );

// Send fNewItem to true to set off new item glow in inv panel
BOOLEAN AutoPlaceObject( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN fNewItem );
BOOLEAN RemoveObjectFromSlot( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj );

// Swap keys in keyring slot and keys in pocket
void SwapKeysToSlot(SOLDIERTYPE&, INT8 key_ring_pos, OBJECTTYPE&);

// create a keyobject
void CreateKeyObject(OBJECTTYPE*, UINT8 ubNumberOfKeys, UINT8 ubKeyIdValue);
BOOLEAN DeleteKeyObject( OBJECTTYPE * pObj );
void    AllocateObject(OBJECTTYPE** pObj);

// remove one or more keys from a *KEYRING* slot
BOOLEAN RemoveKeysFromSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, UINT8 ubNumberOfKeys ,OBJECTTYPE * pObj );

// add the keys to an inventory slot
UINT8 AddKeysToSlot(SOLDIERTYPE&, INT8 key_ring_pos, OBJECTTYPE const& key);



//Kris:  December 9, 1997
//I need a bunch of validation functions for ammo, attachments, etc., so I'll be adding them here.
//Chris, maybe you might find these useful, or add your own.  I don't really know what I'm doing yet,
//so feel free to correct me...

//Simple check to see if the item has any attachments
bool ItemHasAttachments(OBJECTTYPE const&);

//Determine if this item can receive this attachment.  This is different, in that it may
//be possible to have this attachment on this item, but may already have an attachment on
//it which doesn't work simultaneously with the new attachment (like a silencer and duckbill).
BOOLEAN ValidItemAttachment(const OBJECTTYPE* pObj, ItemId usAttachment, BOOLEAN fAttemptingAttachment);

//Determines if it is possible to equip this weapon with this ammo.
bool ValidAmmoType( ItemId usItem, ItemId usAmmoType );

// Determine if it is possible to add this attachment to the item
bool ValidAttachment(ItemId attachment, ItemId item);

BOOLEAN ValidLaunchable( ItemId usLaunchable, ItemId usItem );
ItemId GetLauncherFromLaunchable( ItemId usLaunchable );

BOOLEAN ValidMerge( ItemId usMerge, ItemId usItem );


// Is the item passed a medical kit?
BOOLEAN IsMedicalKitItem(const OBJECTTYPE* pObject);

BOOLEAN AutoReload( SOLDIERTYPE * pSoldier );
INT8 FindAmmoToReload( const SOLDIERTYPE * pSoldier, INT8 bWeaponIn, INT8 bExcludeSlot );

void SwapHandItems( SOLDIERTYPE * pSoldier );

INT8 FindAttachmentByClass(OBJECTTYPE const*, UINT32 uiItemClass);
void RemoveObjs( OBJECTTYPE * pObj, UINT8 ubNumberToRemove );

void WaterDamage(SOLDIERTYPE&);

INT8 FindObjWithin( SOLDIERTYPE * pSoldier, ItemId usItem, INT8 bLower, INT8 bUpper );

BOOLEAN ApplyCamo(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, BOOLEAN* pfGoodAPs);

BOOLEAN ItemIsLegal( ItemId usItemIndex );
ItemId FindReplacementMagazine(const CalibreModel * calibre, UINT8 const mag_size, UINT8 const ammo_type);
ItemId FindReplacementMagazineIfNecessary(const WeaponModel *old_gun, ItemId const old_ammo_id, const WeaponModel *new_gun);

BOOLEAN DamageItemOnGround(OBJECTTYPE* pObject, INT16 sGridNo, INT8 bLevel, INT32 iDamage, SOLDIERTYPE* owner);

BOOLEAN ApplyCanteen( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs );
BOOLEAN ApplyElixir( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs );

BOOLEAN CompatibleFaceItem( ItemId usItem1, ItemId usItem2 );

UINT32 MoneySlotLimit( INT8 bSlot );

void CheckEquipmentForFragileItemDamage( SOLDIERTYPE *pSoldier, INT32 iDamage );

// Range of Xray device
#define XRAY_RANGE	40
// Seconds that Xray lasts
#define XRAY_TIME	5

extern void ActivateXRayDevice( SOLDIERTYPE * pSoldier );
extern void TurnOffXRayEffects( SOLDIERTYPE * pSoldier );
INT8 FindLaunchableAttachment(const OBJECTTYPE* pObj, ItemId usWeapon);

BOOLEAN CanItemFitInPosition(SOLDIERTYPE* s, OBJECTTYPE* pObj, INT8 bPos, BOOLEAN fDoingPlacement);

void SetNewItem(SOLDIERTYPE* pSoldier, UINT8 ubInvPos, BOOLEAN fNewItem);
void CleanUpStack(OBJECTTYPE* pObj, OBJECTTYPE* pCursorObj);
void StackObjs(OBJECTTYPE* pSourceObj, OBJECTTYPE* pTargetObj, UINT8 ubNumberToCopy);
bool ItemIsCool(OBJECTTYPE const&);

bool HasObjectImprint(OBJECTTYPE const&);

#endif
