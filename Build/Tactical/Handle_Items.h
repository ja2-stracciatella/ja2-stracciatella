#ifndef __HANDLE_ITEMS_H
#define __HANDLE_ITEMS_H

#include "JA2Types.h"
#include "World_Items.h"
#include "Structure.h"


#define ITEM_HANDLE_OK													1
#define ITEM_HANDLE_RELOADING										-1
#define ITEM_HANDLE_UNCONSCIOUS									-2
#define ITEM_HANDLE_NOAPS												-3
#define ITEM_HANDLE_NOAMMO											-4
#define ITEM_HANDLE_CANNOT_GETTO_LOCATION				-5
#define ITEM_HANDLE_BROKEN											-6
#define ITEM_HANDLE_NOROOM											-7
#define ITEM_HANDLE_REFUSAL											-8

// Define for code to try and pickup all items....
#define ITEM_PICKUP_ACTION_ALL									32000
#define ITEM_PICKUP_SELECTION										31000

#define ITEM_IGNORE_Z_LEVEL											-1

// visibility defines
#define ANY_VISIBILITY_VALUE  -10
#define HIDDEN_ITEM -4
#define BURIED -3
#define HIDDEN_IN_OBJECT -2
#define INVISIBLE -1
#define VISIBLE 1

#define	ITEM_LOCATOR_LOCKED 0x02



// MACRO FOR DEFINING OF ITEM IS VISIBLE
#define ITEMPOOL_VISIBLE( pItemPool )			( ( pItemPool->bVisible >= 1 ) || (gTacticalStatus.uiFlags&SHOW_ALL_ITEMS) )


struct ITEM_POOL
{
	struct ITEM_POOL* pNext;

	INT32				iItemIndex;
	INT8				bVisible;
	INT8				bFlashColor;
	INT16				sGridNo;
	UINT8				ubLevel;
	UINT16			usFlags;
	INT8				bRenderZHeightAboveLevel;
	LEVELNODE*  pLevelNode;
};


INT32 HandleItem( SOLDIERTYPE *pSoldier, UINT16 usGridNo, INT8 bLevel, UINT16 usHandItem, BOOLEAN fFromUI );
void SoldierPickupItem( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel );
void HandleSoldierPickupItem( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel );
void HandleFlashingItems(void);

BOOLEAN SoldierDropItem( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj );

void HandleSoldierThrowItem( SOLDIERTYPE *pSoldier, INT16 sGridNo );
SOLDIERTYPE* VerifyGiveItem(SOLDIERTYPE* pSoldier);
void SoldierGiveItemFromAnimation( SOLDIERTYPE *pSoldier );
void SoldierGiveItem( SOLDIERTYPE *pSoldier, SOLDIERTYPE *pTargetSoldier, OBJECTTYPE *pObject, INT8 bInvPos );


void NotifySoldiersToLookforItems(void);
void AllSoldiersLookforItems( BOOLEAN RevealRoofsAndItems );


void SoldierGetItemFromWorld( SOLDIERTYPE *pSoldier, INT32 iItemIndex, INT16 sGridNo, INT8 bZLevel, BOOLEAN *pfSelectionList );

INT32 AddItemToPool(INT16 sGridNo, OBJECTTYPE *pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel);
INT32 InternalAddItemToPool(INT16* psGridNo, OBJECTTYPE* pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel);

INT16 AdjustGridNoForItemPlacement( SOLDIERTYPE *pSoldier, INT16 sGridNo );
ITEM_POOL* GetItemPool(UINT16 usMapPos, UINT8 ubLevel);
BOOLEAN DrawItemPoolList(const ITEM_POOL* pItemPool, INT16 sGridNo, INT8 bZLevel, INT16 sXPos, INT16 sYPos);
void RemoveItemFromPool(INT16 grid_no, INT32 item_index, UINT8 level);
BOOLEAN MoveItemPools( INT16 sStartPos, INT16 sEndPos );

BOOLEAN SetItemPoolVisibilityOn( ITEM_POOL *pItemPool, INT8 bAllGreaterThan, BOOLEAN fSetLocator );

void SetItemPoolVisibilityHidden(ITEM_POOL* pItemPool);

void RenderTopmostFlashingItems(void);

void RemoveAllUnburiedItems( INT16 sGridNo, UINT8 ubLevel );


BOOLEAN DoesItemPoolContainAnyHiddenItems(const ITEM_POOL* pItemPool);


void HandleSoldierDropBomb( SOLDIERTYPE *pSoldier, INT16 sGridNo );
void HandleSoldierUseRemote( SOLDIERTYPE *pSoldier, INT16 sGridNo );

BOOLEAN ItemPoolOKForDisplay(const ITEM_POOL* pItemPool, INT8 bZLevel);

void SoldierHandleDropItem( SOLDIERTYPE *pSoldier );

INT8 GetZLevelOfItemPoolGivenStructure( INT16 sGridNo, UINT8 ubLevel, STRUCTURE *pStructure );

INT8 GetLargestZLevelOfItemPool(const ITEM_POOL* pItemPool);

BOOLEAN NearbyGroundSeemsWrong( SOLDIERTYPE * pSoldier, INT16 sGridNo, BOOLEAN fCheckAroundGridno, INT16 * psProblemGridNo );
void MineSpottedDialogueCallBack( void );

extern INT16 gsBoobyTrapGridNo;
extern SOLDIERTYPE * gpBoobyTrapSoldier;
void RemoveBlueFlag( INT16 sGridNo, INT8 bLevel  );

// check if item is booby trapped
BOOLEAN ContinuePastBoobyTrapInMapScreen( OBJECTTYPE *pObject, SOLDIERTYPE *pSoldier );

void RefreshItemPools( WORLDITEM* pItemList, INT32 iNumberOfItems );

BOOLEAN ItemTypeExistsAtLocation( INT16 sGridNo, UINT16 usItem, UINT8 ubLevel, INT32 * piItemIndex );

INT16 FindNearestAvailableGridNoForItem( INT16 sSweetGridNo, INT8 ubRadius );

BOOLEAN CanPlayerUseRocketRifle( SOLDIERTYPE *pSoldier, BOOLEAN fDisplay );

void MakeNPCGrumpyForMinorOffense(SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pOffendingSoldier);

BOOLEAN AnyItemsVisibleOnLevel(const ITEM_POOL* pItemPool, INT8 bZLevel);

BOOLEAN RemoveFlashItemSlot(const ITEM_POOL* pItemPool);

void ToggleItemGlow(BOOLEAN fOn);

BOOLEAN HandleCheckForBadChangeToGetThrough(SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pTargetSoldier, INT16 sTargetGridNo, INT8 bLevel);

#endif
