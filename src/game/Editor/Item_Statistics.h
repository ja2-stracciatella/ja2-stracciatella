#ifndef __ITEM_STATISTICS_H
#define __ITEM_STATISTICS_H

#include "game/Tactical/Item_Types.h"

//Handles the dynamic changing of text input fields and button modes depending on the currently edited
//item.  Both the merc's inventory panel, and the items tab use the same code to accomplish this.

//Set if we are editing items from the items tab.  Otherwise, it is assumed that we are
//editing items from the merc's inventory panel.
extern BOOLEAN gfItemEditingMode;

//Set if we need to update the panel.
extern BOOLEAN gfRenderItemStatsPanel;

void SpecifyItemToEdit( OBJECTTYPE *pItem, INT32 iMapIndex );

void ShowItemStatsPanel(void);
void HideItemStatsPanel(void);

//called from the taskbar renderer.
void UpdateItemStatsPanel(void);

enum
{
	ITEMSTATS_APPLY,
	ITEMSTATS_CANCEL,
	ITEMSTATS_DEFAULT,
	ITEMSTATS_DELETE,
	ITEMSTATS_HIDE,
	ITEMSTATS_SHOW,
};
void ExecuteItemStatsCmd( UINT8 ubAction );

extern OBJECTTYPE *gpItem;
extern INT16			gsItemGridNo;

//enumerations for all of the different action items.  Used by the popup menu for
//changing the type of action item.  When modified, an equivalent text array must be
//changed as well.
enum
{
	ACTIONITEM_TRIP_KLAXON,
	ACTIONITEM_FLARE,
	ACTIONITEM_TEARGAS,
	ACTIONITEM_STUN,
	ACTIONITEM_SMOKE,
	ACTIONITEM_MUSTARD,
	ACTIONITEM_MINE,
	ACTIONITEM_OPEN,
	ACTIONITEM_CLOSE,
	ACTIONITEM_SMPIT,
	ACTIONITEM_LGPIT,
	ACTIONITEM_SMALL,		//grenade
	ACTIONITEM_MEDIUM,	//TNT
	ACTIONITEM_LARGE,		//C4
	ACTIONITEM_TOGGLE_DOOR,
	ACTIONITEM_TOGGLE_ACTION1,
	ACTIONITEM_TOGGLE_ACTION2,
	ACTIONITEM_TOGGLE_ACTION3,
	ACTIONITEM_TOGGLE_ACTION4,
	ACTIONITEM_ENTER_BROTHEL,
	ACTIONITEM_EXIT_BROTHEL,
	ACTIONITEM_KINGPIN_ALARM,
	ACTIONITEM_SEX,
	ACTIONITEM_REVEAL_ROOM,
	ACTIONITEM_LOCAL_ALARM,
	ACTIONITEM_GLOBAL_ALARM,
	ACTIONITEM_KLAXON,
	ACTIONITEM_UNLOCK_DOOR,
	ACTIONITEM_TOGGLE_LOCK,
	ACTIONITEM_UNTRAP_DOOR,
	ACTIONITEM_TOGGLE_PRESSURE_ITEMS,
	ACTIONITEM_MUSEUM_ALARM,
	ACTIONITEM_BLOODCAT_ALARM,
	ACTIONITEM_BIG_TEAR_GAS,
	NUM_ACTIONITEMS
};
extern const wchar_t* gszActionItemDesc[NUM_ACTIONITEMS];
//Returns a pointer to one of the above string array.
extern wchar_t const* GetActionItemName(OBJECTTYPE const*);
//Called by the popup menu, when a selection is made.
extern void UpdateActionItem( INT8 bActionItemIndex );
//Changes an action item into the type specified by the ACTIONITEM enumeration.
extern void ChangeActionItem( OBJECTTYPE *pItem, INT8 bActionItemIndex );
extern INT8 gbActionItemIndex;
extern INT8 gbDefaultBombTrapLevel;

extern void SetOwnershipGroup( UINT8 ubNewGroup );

#endif
