#include "Directories.h"
#include "Font.h"
#include "Overhead_Types.h"
#include "Types.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "EditorDefines.h"
#include "Editor_Taskbar_Utils.h"
#include "EditorMercs.h"
#include "EditorItems.h"
#include "Item_Statistics.h"
#include "Text_Input.h"
#include "Action_Items.h"
#include "Item_Types.h"
#include "Video.h"
#include "Weapons.h"
#include "Items.h"
#include "EditScreen.h"
#include "Random.h"
#include "Handle_Items.h"
#include "World_Items.h"
#include "PopupMenu.h"
#include "Pits.h"


GUIButtonRef giBothCheckboxButton;
GUIButtonRef giRealisticCheckboxButton;
GUIButtonRef giSciFiCheckboxButton;
GUIButtonRef giAlarmTriggerButton;
GUIButtonRef giOwnershipGroupButton;

const wchar_t* gszActionItemDesc[NUM_ACTIONITEMS] =
{
	L"Klaxon Mine",
	L"Flare Mine",
	L"Teargas Explosion",
	L"Stun Explosion",
	L"Smoke Explosion",
	L"Mustard Gas",
	L"Land Mine",
	L"Open Door",
	L"Close Door",
	L"3x3 Hidden Pit",
	L"5x5 Hidden Pit",
	L"Small Explosion",
	L"Medium Explosion",
	L"Large Explosion",
	L"Toggle Door",
	L"Toggle Action1s",
	L"Toggle Action2s",
	L"Toggle Action3s",
	L"Toggle Action4s",
	L"Enter Brothel",
	L"Exit Brothel",
	L"Kingpin Alarm",
	L"Sex with Prostitute",
	L"Reveal Room",
	L"Local Alarm",
	L"Global Alarm",
	L"Klaxon Sound",
	L"Unlock door",
	L"Toggle lock",
	L"Untrap door",
	L"Tog pressure items",
	L"Museum alarm",
	L"Bloodcat alarm",
	L"Big teargas",
};

wchar_t const* GetActionItemName(OBJECTTYPE const* const pItem)
{
	if( !pItem || pItem->usItem != ACTION_ITEM )
		return NULL;
	if( pItem->bActionValue != ACTION_ITEM_BLOW_UP )
	{
		switch( pItem->bActionValue )
		{
			case ACTION_ITEM_OPEN_DOOR:								return gszActionItemDesc[ ACTIONITEM_OPEN ];
			case ACTION_ITEM_CLOSE_DOOR:							return gszActionItemDesc[ ACTIONITEM_CLOSE ];
			case ACTION_ITEM_SMALL_PIT:								return gszActionItemDesc[ ACTIONITEM_SMPIT ];
			case ACTION_ITEM_LARGE_PIT:								return gszActionItemDesc[ ACTIONITEM_LGPIT ];
			case ACTION_ITEM_TOGGLE_DOOR:							return gszActionItemDesc[ ACTIONITEM_TOGGLE_DOOR ];
			case ACTION_ITEM_TOGGLE_ACTION1:					return gszActionItemDesc[ ACTIONITEM_TOGGLE_ACTION1 ];
			case ACTION_ITEM_TOGGLE_ACTION2:					return gszActionItemDesc[ ACTIONITEM_TOGGLE_ACTION2 ];
			case ACTION_ITEM_TOGGLE_ACTION3:					return gszActionItemDesc[ ACTIONITEM_TOGGLE_ACTION3 ];
			case ACTION_ITEM_TOGGLE_ACTION4:					return gszActionItemDesc[ ACTIONITEM_TOGGLE_ACTION4 ];
			case ACTION_ITEM_ENTER_BROTHEL:						return gszActionItemDesc[ ACTIONITEM_ENTER_BROTHEL ];
			case ACTION_ITEM_EXIT_BROTHEL:						return gszActionItemDesc[ ACTIONITEM_EXIT_BROTHEL ];
			case ACTION_ITEM_KINGPIN_ALARM:						return gszActionItemDesc[ ACTIONITEM_KINGPIN_ALARM ];
			case ACTION_ITEM_SEX:											return gszActionItemDesc[ ACTIONITEM_SEX ];
			case ACTION_ITEM_REVEAL_ROOM:							return gszActionItemDesc[ ACTIONITEM_REVEAL_ROOM ];
			case ACTION_ITEM_LOCAL_ALARM:							return gszActionItemDesc[ ACTIONITEM_LOCAL_ALARM ];
			case ACTION_ITEM_GLOBAL_ALARM:						return gszActionItemDesc[ ACTIONITEM_GLOBAL_ALARM ];
			case ACTION_ITEM_KLAXON:									return gszActionItemDesc[ ACTIONITEM_KLAXON ];
			case ACTION_ITEM_UNLOCK_DOOR:							return gszActionItemDesc[ ACTIONITEM_UNLOCK_DOOR ];
			case ACTION_ITEM_TOGGLE_LOCK:							return gszActionItemDesc[ ACTIONITEM_TOGGLE_LOCK ];
			case ACTION_ITEM_UNTRAP_DOOR:							return gszActionItemDesc[ ACTIONITEM_UNTRAP_DOOR ];
			case ACTION_ITEM_TOGGLE_PRESSURE_ITEMS:		return gszActionItemDesc[ ACTIONITEM_TOGGLE_PRESSURE_ITEMS ];
			case ACTION_ITEM_MUSEUM_ALARM:						return gszActionItemDesc[ ACTIONITEM_MUSEUM_ALARM ];
			case ACTION_ITEM_BLOODCAT_ALARM:					return gszActionItemDesc[ ACTIONITEM_BLOODCAT_ALARM ];
			default:																	return NULL;
		}
	}
	else switch( pItem->usBombItem )
	{
		case STUN_GRENADE:			return gszActionItemDesc[ ACTIONITEM_STUN ];
		case SMOKE_GRENADE:			return gszActionItemDesc[ ACTIONITEM_SMOKE ];
		case TEARGAS_GRENADE:		return gszActionItemDesc[ ACTIONITEM_TEARGAS ];
		case MUSTARD_GRENADE:		return gszActionItemDesc[ ACTIONITEM_MUSTARD ];
		case HAND_GRENADE:			return gszActionItemDesc[ ACTIONITEM_SMALL ];
		case TNT:								return gszActionItemDesc[ ACTIONITEM_MEDIUM ];
		case C4:								return gszActionItemDesc[ ACTIONITEM_LARGE ];
		case MINE:							return gszActionItemDesc[ ACTIONITEM_MINE ];
		case TRIP_FLARE:				return gszActionItemDesc[ ACTIONITEM_FLARE ];
		case TRIP_KLAXON:				return gszActionItemDesc[ ACTIONITEM_TRIP_KLAXON ];
		case BIG_TEAR_GAS:			return gszActionItemDesc[ ACTIONITEM_BIG_TEAR_GAS ];
		default:								return NULL;
	}
}

struct AttachmentInfo
{
	AttachmentInfo(UINT16 const a, wchar_t const* const l) : attachment(a), label(l), attached(false) {}

	UINT16         const attachment;
	wchar_t const* const label;
	GUIButtonRef         button;
	bool                 attached;
};

static AttachmentInfo g_weapon_attachment[] =
{
	AttachmentInfo(SILENCER,        L"SILENCER"),
	AttachmentInfo(SNIPERSCOPE,     L"SNIPERSCOPE"),
	AttachmentInfo(LASERSCOPE,      L"LASERSCOPE"),
	AttachmentInfo(BIPOD,           L"BIPOD"),
	AttachmentInfo(DUCKBILL,        L"DUCKBILL"),
	AttachmentInfo(UNDER_GLAUNCHER, L"G-LAUNCHER")
};

static AttachmentInfo g_ceramic_attachment(CERAMIC_PLATES, L"CERAMIC PLATES");

static AttachmentInfo g_detonator_attachment(DETONATOR, L"DETONATOR");

GUIButtonRef guiActionItemButton;
static void ActionItemCallback(GUI_BUTTON* btn, INT32 reason);
INT8 gbActionItemIndex = ACTIONITEM_MEDIUM;
INT8 gbDefaultBombTrapLevel = 9;

enum
{
	EDITING_NOTHING,
	EDITING_NOT_YET_IMPLEMENTED,
	EDITING_DROPPABLE,
	EDITING_GUNS,
	EDITING_AMMO,
	EDITING_ARMOUR,
	EDITING_EQUIPMENT,
	EDITING_EXPLOSIVES,
	EDITING_MONEY,
	EDITING_ACTIONITEMS,
	EDITING_TRIGGERS,
	EDITING_KEYS,
	EDITING_OWNERSHIP,
};

INT8 gbEditingMode = EDITING_NOTHING;

OBJECTTYPE	*gpItem = NULL;
BOOLEAN			gfShowItemStatsPanel;
INT16				gsItemGridNo;

ITEM_POOL		*gpEditingItemPool = NULL;


void ShowItemStatsPanel()
{
	ShowEditorButtons( FIRST_ITEMSTATS_BUTTON, LAST_ITEMSTATS_BUTTON );
	if( iCurrentTaskbar == TASK_MERCS || !gpItem )
		HideEditorButton( ITEMSTATS_HIDDEN_BTN );
	gfShowItemStatsPanel = TRUE;
}

void HideItemStatsPanel()
{
	HideEditorButtons( FIRST_ITEMSTATS_BUTTON, LAST_ITEMSTATS_BUTTON );
	SpecifyItemToEdit( NULL, -1 );
	gfShowItemStatsPanel = FALSE;
}


static void ExtractAndUpdateActionItemsGUI(void);
static void ExtractAndUpdateAmmoGUI(void);
static void ExtractAndUpdateArmourGUI(void);
static void ExtractAndUpdateEquipGUI(void);
static void ExtractAndUpdateExplosivesGUI(void);
static void ExtractAndUpdateGunGUI(void);
static void ExtractAndUpdateKeysGUI(void);
static void ExtractAndUpdateMoneyGUI(void);
static void ExtractAndUpdateOwnershipGUI(void);
static void ExtractAndUpdateTriggersGUI(void);


void ExecuteItemStatsCmd( UINT8 ubAction )
{
	switch( ubAction )
	{
		case ITEMSTATS_APPLY:
			if( gpItem && gpItem->usItem == ACTION_ITEM )
			{
				ExtractAndUpdateActionItemsGUI();
			}
			else if( gpItem && gpItem->usItem == SWITCH )
			{
				ExtractAndUpdateTriggersGUI();
			}
			else if( gpItem && gpItem->usItem == OWNERSHIP )
			{
				ExtractAndUpdateOwnershipGUI();
			}
			else switch( gbEditingMode )
			{
				case EDITING_GUNS:				ExtractAndUpdateGunGUI();					break;
				case EDITING_AMMO:				ExtractAndUpdateAmmoGUI();				break;
				case EDITING_ARMOUR:			ExtractAndUpdateArmourGUI();			break;
				case EDITING_EQUIPMENT:		ExtractAndUpdateEquipGUI();				break;
				case EDITING_EXPLOSIVES:	ExtractAndUpdateExplosivesGUI();	break;
				case EDITING_MONEY:				ExtractAndUpdateMoneyGUI();				break;
				case EDITING_KEYS:				ExtractAndUpdateKeysGUI();				break;
			}
			SetActiveField( 0 );
			gfRenderTaskbar = TRUE;
			break;
		case ITEMSTATS_CANCEL:
			SpecifyItemToEdit( gpItem, gsItemGridNo );
			SetActiveField( 0 );
			break;
		case ITEMSTATS_DEFAULT:
			break;
		case ITEMSTATS_DELETE:
			DeleteSelectedItem();
			break;
		case ITEMSTATS_SHOW:
			ShowSelectedItem();
			break;
		case ITEMSTATS_HIDE:
			HideSelectedItem();
			break;
	}
}


static void RemoveActionItemsGUI(void);
static void RemoveAmmoGUI(void);
static void RemoveArmourGUI(void);
static void RemoveEquipGUI(void);
static void RemoveExplosivesGUI(void);
static void RemoveGameTypeFlags(void);
static void RemoveGunGUI(void);
static void RemoveKeysGUI(void);
static void RemoveMoneyGUI(void);
static void RemoveOwnershipGUI(void);
static void RemoveTriggersGUI(void);


static void RemoveItemGUI(void)
{
	if( !gpItem )
		return;
	if( TextInputMode() )
		KillTextInputMode();
	HideEditorButton( ITEMSTATS_HIDDEN_BTN );
	RemoveGameTypeFlags();
	if( gpItem && gpItem->usItem == ACTION_ITEM )
	{
		RemoveActionItemsGUI();
	}
	else if( gpItem && gpItem->usItem == SWITCH )
	{
		RemoveTriggersGUI();
	}
	else if( gpItem && gpItem->usItem == OWNERSHIP )
	{
		RemoveOwnershipGUI();
	}
	else switch( gbEditingMode )
	{
		case EDITING_GUNS:				RemoveGunGUI();					break;
		case EDITING_AMMO:				RemoveAmmoGUI();				break;
		case EDITING_ARMOUR:			RemoveArmourGUI();			break;
		case EDITING_EQUIPMENT:		RemoveEquipGUI();				break;
		case EDITING_EXPLOSIVES:	RemoveExplosivesGUI();	break;
		case EDITING_MONEY:				RemoveMoneyGUI();				break;
		case EDITING_KEYS:				RemoveKeysGUI();				break;
	}
}


static void SetupActionItemsGUI(void);
static void SetupAmmoGUI(void);
static void SetupArmourGUI(void);
static void SetupEquipGUI(void);
static void SetupExplosivesGUI(void);
static void SetupGameTypeFlags(void);
static void SetupGunGUI();
static void SetupKeysGUI(void);
static void SetupMoneyGUI(void);
static void SetupOwnershipGUI(void);
static void SetupTriggersGUI(void);


void SpecifyItemToEdit( OBJECTTYPE *pItem, INT32 iMapIndex )
{
	//Set the global item pointer to point to the new item
	if( gpItem == pItem )
		return;
	if( gpItem != pItem )
	{
		ExecuteItemStatsCmd( ITEMSTATS_APPLY );
		RemoveItemGUI();
		gpItem = pItem;
		if( gpItemPool )
		{
			gpEditingItemPool = gpItemPool;
		}
		gsItemGridNo = (INT16)iMapIndex;
	}
	else
		RemoveItemGUI();

	gfRenderTaskbar = TRUE;

	if( !gpItem )
	{
		//Hide all edit features.
		gbEditingMode = EDITING_NOTHING;
		gsItemGridNo = -1;
		return;
	}

	//Setup the text mode
	InitTextInputModeWithScheme( DEFAULT_SCHEME );

	if( iCurrentTaskbar == TASK_ITEMS )
		ShowEditorButton( ITEMSTATS_HIDDEN_BTN );

	//Determine the type of item so that we can determine the dynamic editing mode.
	AddUserInputField( NULL );

	SetupGameTypeFlags();

	if( Item[ gpItem->usItem ].usItemClass == IC_MONEY )
	{
		gbEditingMode = EDITING_MONEY;
		SetupMoneyGUI();
	}
	else if( gpItem->usItem == ACTION_ITEM )
	{
		gbEditingMode = EDITING_ACTIONITEMS;
		SetupActionItemsGUI();
		HideEditorButton( ITEMSTATS_HIDDEN_BTN );
	}
	else if( gpItem->usItem == SWITCH )
	{
		gbEditingMode = EDITING_TRIGGERS;
		SetupTriggersGUI();
		HideEditorButton( ITEMSTATS_HIDDEN_BTN );
	}
	else if( gpItem->usItem == OWNERSHIP )
	{
		gbEditingMode = EDITING_OWNERSHIP;
		SetupOwnershipGUI();
		HideEditorButton( ITEMSTATS_HIDDEN_BTN );
	}
	else switch( Item[ gpItem->usItem ].usItemClass )
	{
		case IC_GUN:
			gbEditingMode = EDITING_GUNS;
			SetupGunGUI();
			break;
		case IC_AMMO:
			gbEditingMode = EDITING_AMMO;
			SetupAmmoGUI();
			break;
		case IC_ARMOUR:
			gbEditingMode = EDITING_ARMOUR;
			SetupArmourGUI();
			break;
		case IC_MEDKIT:
		case IC_KIT:
		case IC_MISC:
		case IC_FACE:
		case IC_BLADE:
		case IC_LAUNCHER:
		case IC_THROWING_KNIFE:
		case IC_MONEY:
			gbEditingMode = EDITING_EQUIPMENT;
			SetupEquipGUI();
			break;
		case IC_THROWN:
		case IC_GRENADE:
		case IC_EXPLOSV:
		case IC_BOMB:
			gbEditingMode = EDITING_EXPLOSIVES;
			SetupExplosivesGUI();
			break;
		case IC_KEY:
			gbEditingMode = EDITING_KEYS;
			SetupKeysGUI();
			break;
		case IC_PUNCH:
			if ( gpItem->usItem != NOTHING)
			{
				gbEditingMode = EDITING_EQUIPMENT;
				SetupEquipGUI();
				break;
			}
			// else fall through and act as nothing
		case IC_NONE:
			gbEditingMode = EDITING_NOTHING;
			if( !(gpItem->fFlags & OBJECT_UNDROPPABLE) )
				gbEditingMode = EDITING_DROPPABLE;
			break;
		default:
			gbEditingMode = EDITING_NOT_YET_IMPLEMENTED;
			break;
	}
	if( gpItemPool )
	{
		if (GetWorldItem(gpItemPool->iItemIndex).bVisible == INVISIBLE)
		{
			UnclickEditorButton( ITEMSTATS_HIDDEN_BTN );
			ShowSelectedItem();
		}
		else
		{
			ClickEditorButton( ITEMSTATS_HIDDEN_BTN );
			HideSelectedItem();
		}
	}
}

//called from the taskbar renderer.
void UpdateItemStatsPanel()
{
	SetFontAttributes(FONT10ARIAL, FONT_GRAY2);
	if( gpItem && iCurrentTaskbar == TASK_ITEMS &&
			gbEditingMode != EDITING_TRIGGERS && gbEditingMode != EDITING_ACTIONITEMS )
	{
		MPrint(500, 366, L"Toggle hide flag");
	}
	SetFontForeground( FONT_YELLOW );
	switch( gbEditingMode )
	{
		case EDITING_NOTHING:
			if( iCurrentTaskbar == TASK_ITEMS )
				MPrint(520, 400, L"No item selected.");
			else
			{
				MPrint(500, 390, L"Slot available for");
				MPrint(500, 400, L"random generation.");
			}
			return;
		case EDITING_KEYS:
			if( !gpEditingItemPool )
			{
				MPrint(500, 400, L"Keys not editable.");
				return;
			}
			break;
		case EDITING_OWNERSHIP:
			MPrint(512, 384, L"ProfileID of owner");
			return;
		case EDITING_NOT_YET_IMPLEMENTED:
			MPrint(500, 400, L"Item class not implemented.");
			return;
		case EDITING_DROPPABLE:
			MPrint(500, 400, L"Slot locked as empty.");
			return;
		case EDITING_GUNS:
			MPrint(512, 384, L"Status");
			MPrint(512, 404, L"Rounds");
			MPrint(512, 424, L"Trap Level");
			break;
		case EDITING_AMMO:
			MPrint(512, 384, L"Quantity");
			MPrint(512, 404, L"Trap Level");
			break;
		case EDITING_ARMOUR:
		case EDITING_EQUIPMENT:
			MPrint(512, 384, L"Status");
			MPrint(512, 404, L"Trap Level");
			break;
		case EDITING_EXPLOSIVES:
			MPrint(512, 380, L"Status");
			MPrint(512, 404, L"Quantity");
			MPrint(512, 424, L"Trap Level");
			break;
		case EDITING_MONEY:
			MPrint(532, 384, L"Dollars");
			break;
		case EDITING_ACTIONITEMS:
			MPrint(512, 369, L"Status");
			MPrint(512, 389, L"Trap Level");
			break;
		case EDITING_TRIGGERS:
			MPrint(512, 369, L"Trap Level");
			MPrint(512, 389, L"Tolerance");
			if (gpEditingItemPool && gpItem->bFrequency >= PANIC_FREQUENCY_3)
				MPrint(500, 407, L"Alarm Trigger");
			break;
	}
	if( gpEditingItemPool )
	{
		const INT32 iPercent = 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance;
		if( iPercent == 100 )
			SetFontForeground( FONT_YELLOW );
		else if( iPercent >= 50 )
			SetFontForeground( FONT_ORANGE );
		else
			SetFontForeground( FONT_RED );
		MPrint(512, 444, L"Exist Chance");
		MPrint(587, 366, L"B");
		MPrint(609, 366, L"R");
		MPrint(630, 366, L"S");
	}
	InvalidateRegion( 477, 362, 161, 97 );
}


static void RealisticOnlyCheckboxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		giRealisticCheckboxButton->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
		giSciFiCheckboxButton->uiFlags     &= ~BUTTON_CLICKED_ON;
		giSciFiCheckboxButton->uiFlags     |= BUTTON_DIRTY;
		giBothCheckboxButton->uiFlags      &= ~BUTTON_CLICKED_ON;
		giBothCheckboxButton->uiFlags      |= BUTTON_DIRTY;
		WORLDITEM& wi = GetWorldItem(gpEditingItemPool->iItemIndex);
		wi.usFlags &= ~(WORLD_ITEM_REALISTIC_ONLY | WORLD_ITEM_SCIFI_ONLY);
		wi.usFlags |= WORLD_ITEM_REALISTIC_ONLY;
	}
}


static void SciFiOnlyCheckboxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		giRealisticCheckboxButton->uiFlags &= ~BUTTON_CLICKED_ON;
		giRealisticCheckboxButton->uiFlags |= BUTTON_DIRTY;
		giSciFiCheckboxButton->uiFlags     |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
		giBothCheckboxButton->uiFlags      &= ~BUTTON_CLICKED_ON;
		giBothCheckboxButton->uiFlags      |= BUTTON_DIRTY;
		WORLDITEM& wi = GetWorldItem(gpEditingItemPool->iItemIndex);
		wi.usFlags &= ~(WORLD_ITEM_REALISTIC_ONLY | WORLD_ITEM_SCIFI_ONLY);
		wi.usFlags |= WORLD_ITEM_SCIFI_ONLY;
	}
}


static void BothModesCheckboxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		giRealisticCheckboxButton->uiFlags &= ~BUTTON_CLICKED_ON;
		giRealisticCheckboxButton->uiFlags |= BUTTON_DIRTY;
		giSciFiCheckboxButton->uiFlags     &= ~BUTTON_CLICKED_ON;
		giSciFiCheckboxButton->uiFlags     |= BUTTON_DIRTY;
		giBothCheckboxButton->uiFlags      |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
		WORLDITEM& wi = GetWorldItem(gpEditingItemPool->iItemIndex);
		wi.usFlags &= ~(WORLD_ITEM_REALISTIC_ONLY | WORLD_ITEM_SCIFI_ONLY);
	}
}


static GUIButtonRef MakeRadioButton(INT16 const x, GUI_CALLBACK const click, wchar_t const* const help)
{
	GUIButtonRef const b = CreateCheckBoxButton(x, 365, EDITORDIR "/radiobutton.sti", MSYS_PRIORITY_NORMAL, click);
	b->SetFastHelpText(help);
	return b;
}


static void SetupGameTypeFlags(void)
{
	if( gpEditingItemPool )
	{
		giBothCheckboxButton      = MakeRadioButton(573, BothModesCheckboxCallback,     L"Item appears in both Sci-Fi and Realistic modes. (|B)");
		giRealisticCheckboxButton = MakeRadioButton(595, RealisticOnlyCheckboxCallback, L"Item appears in |Realistic mode only.");
		giSciFiCheckboxButton     = MakeRadioButton(616, SciFiOnlyCheckboxCallback,     L"Item appears in |Sci-Fi mode only.");

		GUIButtonRef     btn;
		WORLDITEM& wi = GetWorldItem(gpEditingItemPool->iItemIndex);
		switch (wi.usFlags & (WORLD_ITEM_REALISTIC_ONLY | WORLD_ITEM_SCIFI_ONLY))
		{
			case WORLD_ITEM_REALISTIC_ONLY: btn = giRealisticCheckboxButton; break;
			case WORLD_ITEM_SCIFI_ONLY:     btn = giSciFiCheckboxButton;     break;
			default:                        btn = giBothCheckboxButton;      break;
		}
		btn->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
	}
}


static void RemoveGameTypeFlags(void)
{
	if (giBothCheckboxButton)      RemoveButton(giBothCheckboxButton);
	if (giRealisticCheckboxButton) RemoveButton(giRealisticCheckboxButton);
	if (giSciFiCheckboxButton)     RemoveButton(giSciFiCheckboxButton);
}


static bool MakeAttachmentButton(AttachmentInfo& a, INT16 const x, INT16 const y, INT16 const w, GUI_CALLBACK const click)
{
	if (!ValidAttachment(a.attachment, gpItem->usItem)) return false;

	GUIButtonRef const btn = CreateTextButton(a.label, SMALLCOMPFONT, FONT_YELLOW, FONT_BLACK, x, y, w, 12, MSYS_PRIORITY_NORMAL, click);
	btn->SetUserPtr(&a);
	a.button   = btn;
	a.attached = FindAttachment(gpItem, a.attachment) != -1;
	if (a.attached) btn->uiFlags |= BUTTON_CLICKED_ON;
	return true;
}


static void ReEvaluateAttachmentStatii(void);
static void ToggleWeaponAttachment(GUI_BUTTON* btn, INT32 reason);


static void SetupGunGUI()
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->bGunStatus);
	AddTextInputField(485, 380, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
	swprintf(str, lengthof(str), L"%d", gpItem->ubGunShotsLeft);
	AddTextInputField(485, 400, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField(485, 420, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT);
	if (gpEditingItemPool)
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField(485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
	}

	/* Attachments are a dynamic part of guns. None, some, or all attachments
	 * could be available for a particular weapon. Show only the ones that we can
	 * apply to this gun. */
	INT16 y = 383;
	FOR_EACH(AttachmentInfo, i, g_weapon_attachment)
	{
		if (MakeAttachmentButton(*i, 570, y, 60, ToggleWeaponAttachment)) y += 14;
	}

	ReEvaluateAttachmentStatii();
}


static void RemoveGunGUI(void)
{
	FOR_EACH(AttachmentInfo, i, g_weapon_attachment)
	{
		GUIButtonRef& b = i->button;
		if (b) RemoveButton(b);
	}
}


static void ExtractAndUpdateGunGUI(void)
{
	INT32 i;
	//Update the gun status
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = 20 + Random( 81 );
	else
		i = MIN( i, 100 );
	gpItem->bGunStatus = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	//Update the ammo
	i = GetNumericStrictValueFromField( 2 );
	if( i == -1 )
		i = Random( 1 + Weapon[ gpItem->usItem ].ubMagSize );
	else
		i = MIN( i, Weapon[ gpItem->usItem ].ubMagSize );
	gpItem->ubGunShotsLeft = (UINT8)i;
	SetInputFieldStringWithNumericStrictValue( 2, i );
	//Update the trap level
	i = GetNumericStrictValueFromField( 3 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 3, i );
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 4 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 4, giDefaultExistChance );
	}
}


static void SetupAmmoGUI(void)
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->ubNumberOfObjects);
	AddTextInputField( 485, 380, 25, 15, MSYS_PRIORITY_NORMAL, str, 1, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField( 485, 400, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}
}


static void RemoveAmmoGUI(void)
{
	//nothing to remove
}


static void ExtractAndUpdateAmmoGUI(void)
{
	INT32 i;
	//Update the number of clips
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = 1 + Random( Item[ gpItem->usItem ].ubPerPocket );
	else
		i = MAX( 1, MIN( i, Item[ gpItem->usItem ].ubPerPocket ) );
	gpItem->ubNumberOfObjects = (UINT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	CreateItems( gpItem->usItem, 100, gpItem->ubNumberOfObjects, gpItem );
	//Update the trap level
	i = GetNumericStrictValueFromField( 2 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 2, i );
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 3 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 3, giDefaultExistChance );
	}
}


static void ToggleAttachment(AttachmentInfo& a)
{
	OBJECTTYPE& o = *gpItem;
	OBJECTTYPE  temp;
	a.attached = !a.attached;
	if (a.attached)
	{
		a.button->uiFlags |= BUTTON_CLICKED_ON;
		CreateItem(a.attachment, o.bStatus[0], &temp);
		AttachObject(0, &o, &temp);
	}
	else
	{
		a.button->uiFlags &= ~BUTTON_CLICKED_ON;
		INT16 const slot = FindAttachment(&o, a.attachment);
		if (slot != -1) RemoveAttachment(&o, slot, &temp);
	}
}


static void ToggleItemAttachment(GUI_BUTTON* const btn, INT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ToggleAttachment(*btn->GetUserPtr<AttachmentInfo>());
	}
}


static void SetupArmourGUI(void)
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->bStatus[0]);
	AddTextInputField( 485, 380, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField( 485, 400, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}

	MakeAttachmentButton(g_ceramic_attachment, 558, 375, 72, ToggleItemAttachment);
}


static void RemoveArmourGUI(void)
{
	if (g_ceramic_attachment.button) RemoveButton(g_ceramic_attachment.button);
}


static void ExtractAndUpdateArmourGUI(void)
{
	INT32 i;
	//Update the armour status
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = 20 + Random( 81 );
	else
		i = MIN( i, 100 );
	gpItem->bStatus[0] = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	//Update the trap level
	i = GetNumericStrictValueFromField( 2 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 2, i );
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 3 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 3, giDefaultExistChance );
	}
}


static void SetupEquipGUI(void)
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->bStatus[0]);
	AddTextInputField( 485, 380, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField( 485, 400, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}
}


static void RemoveEquipGUI(void)
{
	//nothing to remove
}


static void ExtractAndUpdateEquipGUI(void)
{
	INT32 i;
	//Update the equipment status
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = 20 + Random( 81 );
	else
		i = MIN( i, 100 );
	gpItem->bStatus[0] = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	//Update the trap level
	i = GetNumericStrictValueFromField( 2 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 2, i );
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 3 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 3, giDefaultExistChance );
	}
}


static void SetupExplosivesGUI(void)
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->bStatus[0]);
	AddTextInputField( 485, 380, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gpItem->ubNumberOfObjects);
	AddTextInputField( 485, 400, 25, 15, MSYS_PRIORITY_NORMAL, str, 1, INPUTTYPE_NUMERICSTRICT );
	if( Item[ gpItem->usItem ].ubPerPocket == 1 )
	{
		DisableTextField( 2 );
	}
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField( 485, 420, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}

	MakeAttachmentButton(g_detonator_attachment, 570, 375, 60, ToggleItemAttachment);
}


static void RemoveExplosivesGUI(void)
{
	if (g_detonator_attachment.button) RemoveButton(g_detonator_attachment.button);
}


static void ExtractAndUpdateExplosivesGUI(void)
{
	INT32 i;
	//Update the explosives status
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = 20 + Random( 81 );
	else
		i = MIN( i, 100 );
	gpItem->bStatus[0] = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	//Update the quantity
	if( Item[ gpItem->usItem ].ubPerPocket > 1 )
	{
		i = GetNumericStrictValueFromField( 2 );
		if( i == -1 )
			i = 1 + Random( Item[ gpItem->usItem ].ubPerPocket );
		else
			i = MAX( 1, MIN( i, Item[ gpItem->usItem ].ubPerPocket ) );
		gpItem->ubNumberOfObjects = (UINT8)i;
		SetInputFieldStringWithNumericStrictValue( 2, i );
		CreateItems( gpItem->usItem, gpItem->bStatus[0], gpItem->ubNumberOfObjects, gpItem );
	}
	//Update the trap level
	i = GetNumericStrictValueFromField( 3 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 3, i );
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 4 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 4, giDefaultExistChance );
	}
}


static void SetupMoneyGUI(void)
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->uiMoneyAmount);
	AddTextInputField( 485, 380, 45, 15, MSYS_PRIORITY_NORMAL, str, 5, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}
}


static void ExtractAndUpdateMoneyGUI(void)
{
	INT32 i;
	//Update the amount of cash
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = Random( 20000 );
	else
		i = MAX( 1, MIN( i, 20000 ) );
	gpItem->uiMoneyAmount = i;
	gpItem->bStatus[0] = 100;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 2 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 2, giDefaultExistChance );
	}
}


static void RemoveMoneyGUI(void)
{
}


static void OwnershipGroupButtonCallback(GUI_BUTTON* btn, INT32 reason);


static void SetupOwnershipGUI(void)
{
	wchar_t str[20];
	swprintf(str, lengthof(str), L"%d", gpItem->ubOwnerProfile);
	AddTextInputField( 485, 380, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	giOwnershipGroupButton = CreateTextButton(gszCivGroupNames[gpItem->ubOwnerCivGroup], SMALLCOMPFONT, FONT_YELLOW, FONT_BLACK, 485, 415, 80, 25, MSYS_PRIORITY_NORMAL, OwnershipGroupButtonCallback);
}


static void OwnershipGroupButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		InitPopupMenu(btn, OWNERSHIPGROUP_POPUP, DIR_UPLEFT);
	}
}

void SetOwnershipGroup( UINT8 ubNewGroup )
{
	gpItem->ubOwnerCivGroup = ubNewGroup;
	giOwnershipGroupButton->SpecifyText(gszCivGroupNames[ubNewGroup]);
}


static void ExtractAndUpdateOwnershipGUI(void)
{
	INT32 i;
	//Update the amount of cash
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = Random( 0 );
	else
		i = MAX( 0, MIN( i, 255 ) );
	gpItem->ubOwnerProfile = (UINT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
}


static void RemoveOwnershipGUI(void)
{
	if (giOwnershipGroupButton) RemoveButton(giOwnershipGroupButton);
}


static void SetupKeysGUI(void)
{
	wchar_t str[20];
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}
}


static void ExtractAndUpdateKeysGUI(void)
{
	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 1 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 1, giDefaultExistChance );
	}
}


static void RemoveKeysGUI(void)
{
}


static void SetupActionItemsGUI(void)
{
	wchar_t str[4];
	const wchar_t* pStr;
	swprintf(str, lengthof(str), L"%d", gpItem->bStatus[0]);
	AddTextInputField( 485, 365, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField( 485, 385, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	}
	pStr = GetActionItemName( gpItem );
	guiActionItemButton = CreateTextButton(pStr, FONT10ARIAL, FONT_YELLOW, FONT_BLACK, 510, 410, 100, 20, MSYS_PRIORITY_NORMAL, ActionItemCallback);
}


static void ExtractAndUpdateActionItemsGUI(void)
{
	INT32 i;
	//Update the equipment status
	i = GetNumericStrictValueFromField( 1 );
	if( i == -1 )
		i = 20 + Random( 81 );
	else
		i = MIN( i, 100 );
	gpItem->bStatus[0] = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );
	//Update the trap level
	i = GetNumericStrictValueFromField( 2 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	if( i != gpItem->bTrap )
		gbDefaultBombTrapLevel = (INT8)i;
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 2, i );

	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 3 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 3, giDefaultExistChance );
	}
}


static void RemoveActionItemsGUI(void)
{
	if (guiActionItemButton) RemoveButton(guiActionItemButton);
}


static void AlarmTriggerCheckboxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->Clicked())
			gpItem->fFlags |= OBJECT_ALARM_TRIGGER;
		else
			gpItem->fFlags &= ~OBJECT_ALARM_TRIGGER;
	}
}


static void SetupTriggersGUI(void)
{
	wchar_t str[4];
	swprintf(str, lengthof(str), L"%d", gpItem->bTrap);
	AddTextInputField( 485, 365, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gpItem->ubTolerance);
	AddTextInputField( 485, 385, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	if( gpEditingItemPool )
	{
		swprintf(str, lengthof(str), L"%d", 100 - GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance);
		AddTextInputField( 485, 440, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
		if (gpItem->bFrequency >= PANIC_FREQUENCY_3)
		{
			giAlarmTriggerButton = CreateCheckBoxButton(485, 405, EDITORDIR "/smcheckbox.sti", MSYS_PRIORITY_NORMAL, AlarmTriggerCheckboxCallback);
			giAlarmTriggerButton->SetFastHelpText(L"If the panic trigger is an alarm trigger,\nenemies won't attempt to use it if they\nare already aware of your presence.");
			if( gpItem->fFlags & OBJECT_ALARM_TRIGGER )
				giAlarmTriggerButton->uiFlags |= BUTTON_CLICKED_ON;
		}
	}
}


static void ExtractAndUpdateTriggersGUI(void)
{
	INT32 i;
	//Update the trap level
	i = GetNumericStrictValueFromField( 1 );
	i = ( i == -1 ) ? 0 : MIN( i, 20 );
	gpItem->bTrap = (INT8)i;
	SetInputFieldStringWithNumericStrictValue( 1, i );

	i = GetNumericStrictValueFromField( 2 );
	i = ( i == -1 ) ? 0 : MAX( 0, MIN( i, 99 ) );
	gpItem->ubTolerance = (UINT8)i;
	SetInputFieldStringWithNumericStrictValue( 2, i );

	if( gpEditingItemPool )
	{
		giDefaultExistChance = GetNumericStrictValueFromField( 3 );
		giDefaultExistChance = ( giDefaultExistChance == -1 ) ? 100 : MAX( 1, MIN( giDefaultExistChance, 100 ) );
		GetWorldItem(gpEditingItemPool->iItemIndex).ubNonExistChance = 100 - giDefaultExistChance;
		SetInputFieldStringWithNumericStrictValue( 3, giDefaultExistChance );
	}
}


static void RemoveTriggersGUI(void)
{
	if (gpEditingItemPool && gpItem->bFrequency >= PANIC_FREQUENCY_3)
	{
		if (giAlarmTriggerButton) RemoveButton(giAlarmTriggerButton);
	}
}


static void ToggleWeaponAttachment(GUI_BUTTON* const btn, INT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ToggleAttachment(*btn->GetUserPtr<AttachmentInfo>());
		ReEvaluateAttachmentStatii();
	}
}


static void ActionItemCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		InitPopupMenu( guiActionItemButton, ACTIONITEM_POPUP, DIR_UPLEFT );
	}
}

void ChangeActionItem( OBJECTTYPE *pItem, INT8 bActionItemIndex )
{
	pItem->usItem = ACTION_ITEM;
	pItem->bActionValue = ACTION_ITEM_BLOW_UP;
	switch( bActionItemIndex )
	{
		case ACTIONITEM_TRIP_KLAXON:
			pItem->usBombItem = TRIP_KLAXON;
			break;
		case ACTIONITEM_FLARE:
			pItem->usBombItem = TRIP_FLARE;
			break;
		case ACTIONITEM_TEARGAS:
			pItem->usBombItem = TEARGAS_GRENADE;
			break;
		case ACTIONITEM_STUN:
			pItem->usBombItem = STUN_GRENADE;
			break;
		case ACTIONITEM_SMOKE:
			pItem->usBombItem = SMOKE_GRENADE;
			break;
		case ACTIONITEM_MUSTARD:
			pItem->usBombItem = MUSTARD_GRENADE;
			break;
		case ACTIONITEM_MINE:
			pItem->usBombItem = MINE;
			break;
		case ACTIONITEM_OPEN:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_OPEN_DOOR;
			break;
		case ACTIONITEM_CLOSE:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_CLOSE_DOOR;
			break;
		case ACTIONITEM_UNLOCK_DOOR:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_UNLOCK_DOOR;
			break;
		case ACTIONITEM_TOGGLE_LOCK:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_LOCK;
			break;
		case ACTIONITEM_UNTRAP_DOOR:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_UNTRAP_DOOR;
			break;
		case ACTIONITEM_TOGGLE_PRESSURE_ITEMS:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_PRESSURE_ITEMS;
			break;
		case ACTIONITEM_SMPIT:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_SMALL_PIT;
			break;
		case ACTIONITEM_LGPIT:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_LARGE_PIT;
			break;
		case ACTIONITEM_TOGGLE_DOOR:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_DOOR;
			break;
		case ACTIONITEM_TOGGLE_ACTION1:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_ACTION1;
			break;
		case ACTIONITEM_TOGGLE_ACTION2:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_ACTION2;
			break;
		case ACTIONITEM_TOGGLE_ACTION3:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_ACTION3;
			break;
		case ACTIONITEM_TOGGLE_ACTION4:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_TOGGLE_ACTION4;
			break;
		case ACTIONITEM_ENTER_BROTHEL:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_ENTER_BROTHEL;
			break;
		case ACTIONITEM_EXIT_BROTHEL:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_EXIT_BROTHEL;
			break;
		case ACTIONITEM_KINGPIN_ALARM:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_KINGPIN_ALARM;
			break;
		case ACTIONITEM_SEX:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_SEX;
			break;
		case ACTIONITEM_REVEAL_ROOM:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_REVEAL_ROOM;
			break;
		case ACTIONITEM_LOCAL_ALARM:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_LOCAL_ALARM;
			break;
		case ACTIONITEM_GLOBAL_ALARM:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_GLOBAL_ALARM;
			break;
		case ACTIONITEM_KLAXON:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_KLAXON;
			break;
		case ACTIONITEM_SMALL:
			pItem->usBombItem = HAND_GRENADE;
			break;
		case ACTIONITEM_MEDIUM:
			pItem->usBombItem = TNT;
			break;
		case ACTIONITEM_LARGE:
			pItem->usBombItem = C4;
			break;
		case ACTIONITEM_MUSEUM_ALARM:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_MUSEUM_ALARM;
			break;
		case ACTIONITEM_BLOODCAT_ALARM:
			pItem->usBombItem = NOTHING;
			pItem->bActionValue = ACTION_ITEM_BLOODCAT_ALARM;
			break;
		case ACTIONITEM_BIG_TEAR_GAS:
			pItem->usBombItem = BIG_TEAR_GAS;
			break;

	}
}

void UpdateActionItem( INT8 bActionItemIndex )
{
	gbActionItemIndex = bActionItemIndex; //used for future new actionitems as the default.

	if( !gpItemPool || !gpItem )
		return;

	//If the previous item was a pit, remove it before changing it
	if( gpItem->usItem == ACTION_ITEM )
	{
		WORLDITEM const& wi = GetWorldItem(gpItemPool->iItemIndex);
		switch (gpItem->bActionValue)
		{
			case ACTION_ITEM_SMALL_PIT: Remove3X3Pit(wi.sGridNo); break;
			case ACTION_ITEM_LARGE_PIT: Remove5X5Pit(wi.sGridNo); break;
		}
	}

	ChangeActionItem( gpItem, gbActionItemIndex );
	guiActionItemButton->SpecifyText(GetActionItemName(gpItem));

	//If the new item is a pit, add it so we can see how it looks.
	if( gpItem->usItem == ACTION_ITEM )
	{
		WORLDITEM const& wi = GetWorldItem(gpItemPool->iItemIndex);
		switch (gpItem->bActionValue)
		{
			case ACTION_ITEM_SMALL_PIT: Add3X3Pit(wi.sGridNo); break;
			case ACTION_ITEM_LARGE_PIT: Add5X5Pit(wi.sGridNo); break;
		}
	}
}


static void ReEvaluateAttachmentStatii(void)
{
	FOR_EACH(AttachmentInfo, i, g_weapon_attachment)
	{
		AttachmentInfo const& a = *i;
		if (!a.button) continue;
		GUI_BUTTON& b = *a.button;
		if (b.Clicked()) continue;
		EnableButton(&b, ValidItemAttachment(gpItem, a.attachment, TRUE));
	}
}

