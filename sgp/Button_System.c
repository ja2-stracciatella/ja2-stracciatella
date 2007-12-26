// Rewritten mostly by Kris Morness

#include "Button_Sound_Control.h"
#include "Button_System.h"
#include "Debug.h"
#include "Font.h"
#include "HImage.h"
#include "Input.h"
#include "MemMan.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"
#include "WordWrap.h"

#ifdef _JA2_RENDER_DIRTY
#	include "Font_Control.h"
#endif


// Names of the default generic button image files.
#define DEFAULT_GENERIC_BUTTON_OFF    "GENBUTN.STI"
#define DEFAULT_GENERIC_BUTTON_ON     "GENBUTN2.STI"
#define DEFAULT_GENERIC_BUTTON_OFF_HI "GENBUTN3.STI"
#define DEFAULT_GENERIC_BUTTON_ON_HI  "GENBUTN4.STI"


#define MSYS_STARTING_CURSORVAL 0


#define MAX_GENERIC_PICS 40
#define MAX_BUTTON_ICONS 40


#define GUI_BTN_NONE           0
#define GUI_BTN_DUPLICATE_VOBJ 1


/* Kris:  December 2, 1997
 * Special internal debugging utilities that will ensure that you don't attempt
 * to delete an already deleted button, or it's images, etc.  It will also
 * ensure that you don't create the same button that already exists.
 * TO REMOVE ALL DEBUG FUNCTIONALITY: simply comment out BUTTONSYSTEM_DEBUGGING
 * definition
 */
#if defined JA2 && defined _DEBUG
#	define BUTTONSYSTEM_DEBUGGING
#endif


#ifdef BUTTONSYSTEM_DEBUGGING

BOOLEAN gfIgnoreShutdownAssertions;


// Called immediately before assigning the button to the button list.
static void AssertFailIfIdenticalButtonAttributesFound(const GUI_BUTTON* b)
{
	for (INT32 x = 0; x < MAX_BUTTONS; ++x)
	{
		const GUI_BUTTON* const c = ButtonList[x];
		if (!c)                                                       continue;
		if (c->uiFlags                 &   BUTTON_DELETION_PENDING  ) continue;
		if (c->uiFlags                 &   BUTTON_NO_DUPLICATE      ) continue;
		if (b->Area.PriorityLevel      != c->Area.PriorityLevel     ) continue;
		if (b->Area.RegionTopLeftX     != c->Area.RegionTopLeftX    ) continue;
		if (b->Area.RegionTopLeftY     != c->Area.RegionTopLeftY    ) continue;
		if (b->Area.RegionBottomRightX != c->Area.RegionBottomRightX) continue;
		if (b->Area.RegionBottomRightY != c->Area.RegionBottomRightY) continue;
		if (b->ClickCallback           != c->ClickCallback          ) continue;
		if (b->MoveCallback            != c->MoveCallback           ) continue;
		if (b->XLoc                    != c->XLoc                   ) continue;
		if (b->YLoc                    != c->YLoc                   ) continue;
		/* if we get this far, it is reasonably safe to assume that the newly
		 * created button already exists.  Placing a break point on the following
		 * assert will allow the coder to easily isolate the case!
		 */
		AssertMsg(0, String("Attempting to create a button that has already been created (existing buttonID %d).", c->IDNum));
	}
}

#endif


/* Kris:
 * These are the variables used for the anchoring of a particular button.  When
 * you click on a button, it get's anchored, until you release the mouse button.
 * When you move around, you don't want to select other buttons, even when you
 * release it.  This follows the Windows 95 convention.
 */
static GUI_BUTTON* gpAnchoredButton;
static GUI_BUTTON* gpPrevAnchoredButton;
static BOOLEAN gfAnchoredState;

static INT8 gbDisabledButtonStyle;

BOOLEAN gfRenderHilights = TRUE;

// Struct definition for the QuickButton pictures.
typedef struct BUTTON_PICS
{
	HVOBJECT vobj;      // The Image itself
	INT32    Grayed;    // Index to use for a "Grayed-out" button
	INT32    OffNormal; // Index to use when button is OFF
	INT32    OffHilite; // Index to use when button is OFF w/ hilite on it
	INT32    OnNormal;  // Index to use when button is ON
	INT32    OnHilite;  // Index to use when button is ON w/ hilite on it
	ButtonDimensions max; // width/height of largest image in use
	UINT32   fFlags;    // Special image flags
} BUTTON_PICS;

static BUTTON_PICS ButtonPictures[MAX_BUTTON_PICS];

SGPVSurface* ButtonDestBuffer;

GUI_BUTTON* ButtonList[MAX_BUTTONS];


const ButtonDimensions* GetDimensionsOfButtonPic(UINT16 btn_pic_id)
{
	return &ButtonPictures[btn_pic_id].max;
}


static HVOBJECT GenericButtonOffNormal[MAX_GENERIC_PICS];
static HVOBJECT GenericButtonOffHilite[MAX_GENERIC_PICS];
static HVOBJECT GenericButtonOnNormal[MAX_GENERIC_PICS];
static HVOBJECT GenericButtonOnHilite[MAX_GENERIC_PICS];
static UINT16   GenericButtonFillColors[MAX_GENERIC_PICS];

static HVOBJECT GenericButtonIcons[MAX_BUTTON_ICONS];

static BOOLEAN gfDelayButtonDeletion   = FALSE;
static BOOLEAN gfPendingButtonDeletion = FALSE;

extern MOUSE_REGION* MSYS_PrevRegion;


// Finds an available slot for loading button pictures
static INT32 FindFreeButtonSlot(void)
{
	// Search for a slot
	for (int slot = 0; slot < MAX_BUTTON_PICS; slot++)
	{
		if (ButtonPictures[slot].vobj == NULL) return slot;
	}

	return BUTTON_NO_SLOT;
}


static void InitButtonImage(UINT32 UseSlot, HVOBJECT VObj, UINT32 Flags, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite)
{
	BUTTON_PICS* const pics = &ButtonPictures[UseSlot];

	pics->vobj = VObj;

	// Init the QuickButton image structure with indexes to use
	pics->Grayed    = Grayed;
	pics->OffNormal = OffNormal;
	pics->OffHilite = OffHilite;
	pics->OnNormal  = OnNormal;
	pics->OnHilite  = OnHilite;
	pics->fFlags    = Flags;

	// Fit the button size to the largest image in the set
	UINT32 MaxWidth  = 0;
	UINT32 MaxHeight = 0;
	const ETRLEObject* const Travs = pics->vobj->pETRLEObject;
	if (Grayed != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &Travs[Grayed];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OffNormal != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &Travs[OffNormal];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OffHilite != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &Travs[OffHilite];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OnNormal != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &Travs[OnNormal];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OnHilite != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &Travs[OnHilite];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	// Set the width and height for this image set
	pics->max.w = MaxWidth;
	pics->max.h = MaxHeight;
}


INT32 LoadButtonImage(const char* filename, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite)
{
	AssertMsg(filename != NULL, "Attempting to LoadButtonImage() with null filename.");

	if (Grayed    == BUTTON_NO_IMAGE &&
			OffNormal == BUTTON_NO_IMAGE &&
			OffHilite == BUTTON_NO_IMAGE &&
			OnNormal  == BUTTON_NO_IMAGE &&
			OnHilite  == BUTTON_NO_IMAGE)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("No button pictures selected for %s", filename));
		return BUTTON_NO_SLOT;
	}

	UINT32 UseSlot = FindFreeButtonSlot();
	if (UseSlot == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Out of button image slots for %s", filename));
		return BUTTON_NO_SLOT;
	}

	SGPVObject* const VObj = AddVideoObjectFromFile(filename);
	if (VObj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Couldn't create VOBJECT for %s", filename));
		return BUTTON_NO_SLOT;
	}

	InitButtonImage(UseSlot, VObj, GUI_BTN_NONE, Grayed, OffNormal, OffHilite, OnNormal, OnHilite);
	return UseSlot;
}


INT32 UseLoadedButtonImage(INT32 LoadedImg, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite)
{
	if (Grayed    == BUTTON_NO_IMAGE &&
			OffNormal == BUTTON_NO_IMAGE &&
			OffHilite == BUTTON_NO_IMAGE &&
			OnNormal  == BUTTON_NO_IMAGE &&
			OnHilite  == BUTTON_NO_IMAGE)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("No button pictures selected for pre-loaded button image %d", LoadedImg));
		return -1;
	}

	UINT32 UseSlot = FindFreeButtonSlot();
	if (UseSlot == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Out of button image slots for pre-loaded button image %d", LoadedImg));
		return -1;
	}

	// Is button image index given valid?
	const HVOBJECT vobj = ButtonPictures[LoadedImg].vobj;
	if (vobj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Invalid button picture handle given for pre-loaded button image %d", LoadedImg));
		return -1;
	}

	InitButtonImage(UseSlot, vobj, GUI_BTN_DUPLICATE_VOBJ, Grayed, OffNormal, OffHilite, OnNormal, OnHilite);
	return UseSlot;
}


void UnloadButtonImage(INT32 Index)
{
	AssertMsg(0 <= Index && Index < MAX_BUTTON_PICS, String("Attempting to UnloadButtonImage with out of range index %d.", Index));

	BUTTON_PICS* const pics = &ButtonPictures[Index];

	if (pics->vobj == NULL)
	{
#if defined BUTTONSYSTEM_DEBUGGING
		if (gfIgnoreShutdownAssertions)
#endif
			return;
		AssertMsg(0, "Attempting to UnloadButtonImage that has a null vobj (already deleted).");
	}

	// If this is a duplicated button image, then don't trash the vobject
	if (!(pics->fFlags & GUI_BTN_DUPLICATE_VOBJ))
	{
		/* Deleting a non-duplicate, so see if any dups present. if so, then convert
		 * one of them to an original!
		 */
		for (INT32 x = 0; x < MAX_BUTTON_PICS; ++x)
		{
			if (x == Index) continue;
			BUTTON_PICS* const other = &ButtonPictures[x];
			if (other->vobj != pics->vobj) continue;
			if (!(other->fFlags & GUI_BTN_DUPLICATE_VOBJ)) continue;

			/* If we got here, then we got a duplicate object of the one we want to
			 * delete, so convert it to an original!
			 */
			other->fFlags &= ~GUI_BTN_DUPLICATE_VOBJ;

			// Now remove this button, but not its vobject
			goto remove_pic;
		}

		DeleteVideoObject(pics->vobj);
	}

remove_pic:
	pics->vobj = NULL;
}


static GUI_BUTTON* GetButton(INT32 BtnID)
{
	CHECKN(0 <= BtnID && BtnID < MAX_BUTTONS); // XXX HACK000C
	AssertMsg(0 <= BtnID && BtnID < MAX_BUTTONS, String("ButtonID %d is out of range.", BtnID));
	GUI_BUTTON* b = ButtonList[BtnID];
	CHECKN(b != NULL); // XXX HACK000C
	AssertMsg(b != NULL, String("Accessing non-existent button %d.", BtnID));
	return b;
}


void EnableButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	b->uiFlags |= BUTTON_ENABLED | BUTTON_DIRTY;
}


void DisableButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	b->uiFlags &= ~BUTTON_ENABLED;
	b->uiFlags |= BUTTON_DIRTY;
}


/* Initializes the button image sub-system. This function is called by
 * InitButtonSystem.
 */
static BOOLEAN InitializeButtonImageManager(void)
{
	// Blank out all QuickButton images
	for (int x = 0; x < MAX_BUTTON_PICS; ++x)
	{
		BUTTON_PICS* const pics = &ButtonPictures[x];
		pics->vobj      = NULL;
		pics->Grayed    = -1;
		pics->OffNormal = -1;
		pics->OffHilite = -1;
		pics->OnNormal  = -1;
		pics->OnHilite  = -1;
	}

	// Blank out all Generic button data
	for (int x = 0; x < MAX_GENERIC_PICS; ++x)
	{
		GenericButtonOffNormal[x]  = NULL;
		GenericButtonOffHilite[x]  = NULL;
		GenericButtonOnNormal[x]   = NULL;
		GenericButtonOnHilite[x]   = NULL;
		GenericButtonFillColors[x] = 0;
	}

	// Blank out all icon images
	for (int x = 0; x < MAX_BUTTON_ICONS; ++x)
		GenericButtonIcons[x] = NULL;

	// Load the default generic button images
	GenericButtonOffNormal[0] = AddVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_OFF);
	if (GenericButtonOffNormal[0] == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Couldn't create VOBJECT for "DEFAULT_GENERIC_BUTTON_OFF);
		return FALSE;
	}

	GenericButtonOnNormal[0] = AddVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_ON);
	if (GenericButtonOnNormal[0] == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Couldn't create VOBJECT for "DEFAULT_GENERIC_BUTTON_ON);
		return FALSE;
	}

	/* Load up the off hilite and on hilite images. We won't check for errors
	 * because if the file doesn't exists, the system simply ignores that file.
	 * These are only here as extra images, they aren't required for operation
	 * (only OFF Normal and ON Normal are required).
	 */
	GenericButtonOffHilite[0] = AddVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_OFF_HI);
	GenericButtonOnHilite[0]  = AddVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_ON_HI);

	UINT8 Pix = 0;
	if (!GetETRLEPixelValue(&Pix, GenericButtonOffNormal[0], 8, 0, 0))
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Couldn't get generic button's background pixel value");
		return FALSE;
	}

	GenericButtonFillColors[0] = GenericButtonOffNormal[0]->p16BPPPalette[Pix];

	return TRUE;
}


// Finds the next available slot for button icon images.
static INT16 FindFreeIconSlot(void)
{
	for (INT16 x = 0; x < MAX_BUTTON_ICONS; ++x)
	{
		if (GenericButtonIcons[x] == NULL) return x;
	}
	return BUTTON_NO_SLOT;
}


INT16 LoadGenericButtonIcon(const char* filename)
{
	AssertMsg(filename != NULL, "Attempting to LoadGenericButtonIcon() with null filename.");

	// Get slot for icon image
	INT16 ImgSlot = FindFreeIconSlot();
	if (ImgSlot == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "LoadGenericButtonIcon: Out of generic button icon slots");
		return BUTTON_NO_SLOT;
	}

	// Load the icon
	GenericButtonIcons[ImgSlot] = AddVideoObjectFromFile(filename);
	if (GenericButtonIcons[ImgSlot] == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("LoadGenericButtonIcon: Couldn't create VOBJECT for %s", filename));
		return BUTTON_NO_SLOT;
	}

	// Return the slot number
	return ImgSlot;
}


BOOLEAN UnloadGenericButtonIcon(INT16 GenImg)
{
	AssertMsg(0 <= GenImg && GenImg < MAX_BUTTON_ICONS, String("Attempting to UnloadGenericButtonIcon with out of range index %d.", GenImg));

	if (!GenericButtonIcons[GenImg])
	{
#if defined BUTTONSYSTEM_DEBUGGING
		if (gfIgnoreShutdownAssertions)
#endif
			return FALSE;
		AssertMsg(0, "Attempting to UnloadGenericButtonIcon that has no icon (already deleted).");
	}
	// If an icon is present in the slot, remove it.
	DeleteVideoObject(GenericButtonIcons[GenImg]);
	GenericButtonIcons[GenImg] = NULL;
	return TRUE;
}


// Cleans up, and shuts down the button image manager sub-system.
static void ShutdownButtonImageManager(void)
{
#if defined BUTTONSYSTEM_DEBUGGING
	gfIgnoreShutdownAssertions = TRUE;
#endif

	// Remove all QuickButton images
	for (int x = 0; x < MAX_BUTTON_PICS; ++x)
	{
		UnloadButtonImage(x);
	}

	// Remove all GenericButton images
	for (int x = 0; x <MAX_GENERIC_PICS; ++x)
	{
		if (GenericButtonOffNormal[x] != NULL)
		{
			DeleteVideoObject(GenericButtonOffNormal[x]);
			GenericButtonOffNormal[x] = NULL;
		}

		if (GenericButtonOffHilite[x]!=NULL)
		{
			DeleteVideoObject(GenericButtonOffHilite[x]);
			GenericButtonOffHilite[x] = NULL;
		}

		if (GenericButtonOnNormal[x] != NULL)
		{
			DeleteVideoObject(GenericButtonOnNormal[x]);
			GenericButtonOnNormal[x] = NULL;
		}

		if (GenericButtonOnHilite[x] != NULL)
		{
			DeleteVideoObject(GenericButtonOnHilite[x]);
			GenericButtonOnHilite[x] = NULL;
		}

		GenericButtonFillColors[x] = 0;
	}

	// Remove all button icons
	for (int x = 0; x < MAX_BUTTON_ICONS; ++x)
	{
		if (GenericButtonIcons[x] != NULL)
			GenericButtonIcons[x] = NULL;
	}
}


BOOLEAN InitButtonSystem(void)
{
#if defined BUTTONSYSTEM_DEBUGGING
	gfIgnoreShutdownAssertions = FALSE;
#endif

	ButtonDestBuffer = FRAME_BUFFER;

	// Clear out button list
	for (INT32 x = 0; x < MAX_BUTTONS; ++x)
	{
		ButtonList[x] = NULL;
	}

	// Initialize the button image manager sub-system
	if (!InitializeButtonImageManager())
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Failed button image manager init\n");
		return FALSE;
	}

	return TRUE;
}


void ShutdownButtonSystem(void)
{
	// Kill off all buttons in the system
	for (int x = 0; x < MAX_BUTTONS; ++x)
	{
		if (ButtonList[x] != NULL) RemoveButton(x);
	}
	ShutdownButtonImageManager();
}


static void RemoveButtonsMarkedForDeletion(void)
{
	for (INT32 i = 0; i < MAX_BUTTONS; ++i)
	{
		if (ButtonList[i] && ButtonList[i]->uiFlags & BUTTON_DELETION_PENDING)
		{
			RemoveButton(i);
		}
	}
}


void RemoveButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C

	/* If we happen to be in the middle of a callback, and attempt to delete a
	 * button, like deleting a node during list processing, then we delay it till
	 * after the callback is completed.
	 */
	if (gfDelayButtonDeletion)
	{
		b->uiFlags |= BUTTON_DELETION_PENDING;
		gfPendingButtonDeletion = TRUE;
		return;
	}

	//Kris:
	if (b->uiFlags & BUTTON_SELFDELETE_IMAGE)
	{
		/* Checkboxes and simple create buttons have their own graphics associated
		 * with them, and it is handled internally.  We delete it here.  This
		 * provides the advantage of less micromanagement, but with the
		 * disadvantage of wasting more memory if you have lots of buttons using the
		 * same graphics.
		 */
		UnloadButtonImage(b->ImageNum);
	}

	MSYS_RemoveRegion(&b->Area);

	// Get rid of the text string
	if (b->string != NULL) MemFree(b->string);

	if (b == gpAnchoredButton)     gpAnchoredButton     = NULL;
	if (b == gpPrevAnchoredButton) gpPrevAnchoredButton = NULL;

	MemFree(b);
	ButtonList[iButtonID] = NULL;
}


// Finds the next available button slot.
static INT32 GetNextButtonNumber(void)
{
	for (INT32 x = 0; x < MAX_BUTTONS; x++)
	{
		if (ButtonList[x] == NULL) return x;
	}
	return BUTTON_NO_SLOT;
}


static void QuickButtonCallbackMButn(MOUSE_REGION* reg, INT32 reason);
static void QuickButtonCallbackMMove(MOUSE_REGION* reg, INT32 reason);


static GUI_BUTTON* AllocateButton(UINT32 ImageNum, UINT32 Flags, INT16 Left, INT16 Top, INT16 Width, INT16 Height, INT8 Priority, GUI_CALLBACK Click, GUI_CALLBACK Move)
{
	AssertMsg(Left >= 0 && Top >= 0 && Width >= 0 && Height >= 0, String("Attempting to create button with invalid coordinates %dx%d+%dx%d", Left, Top, Width, Height));

	INT32 BtnID = GetNextButtonNumber();
	if (BtnID == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "No more button slots");
		return NULL;
	}

	GUI_BUTTON* b = MemAlloc(sizeof(*b));
	if (b == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Cannot allocte memory for button struct");
		return NULL;
	}

	b->IDNum                   = BtnID;
	b->ImageNum                = ImageNum;
	b->ClickCallback           = Click;
	b->MoveCallback            = Move;
	b->uiFlags                 = BUTTON_DIRTY | BUTTON_ENABLED | Flags;
	b->uiOldFlags              = 0;
	b->XLoc                    = Left;
	b->YLoc                    = Top;
	b->User.Data               = 0;
	b->bDisabledStyle          = DISABLED_STYLE_DEFAULT;
	b->string                  = NULL;
	b->usFont                  = 0;
	b->sForeColor              = 0;
	b->sShadowColor            = -1;
	b->sForeColorDown          = -1;
	b->sShadowColorDown        = -1;
	b->sForeColorHilited       = -1;
	b->sShadowColorHilited     = -1;
	b->bJustification          = BUTTON_TEXT_CENTER;
	b->bTextXOffset            = -1;
	b->bTextYOffset            = -1;
	b->bTextXSubOffSet         = 0;
	b->bTextYSubOffSet         = 0;
	b->fShiftText              = TRUE;
	b->sWrappedWidth           = -1;
	b->icon                    = NO_VOBJECT;
	b->usIconIndex             = -1;
	b->bIconXOffset            = -1;
	b->bIconYOffset            = -1;
	b->fShiftImage             = TRUE;
	b->ubToggleButtonActivated = FALSE;

	memset(&b->Area, 0, sizeof(b->Area));
	MSYS_DefineRegion(
		&b->Area,
		Left,
		Top,
		Left + Width,
		Top  + Height,
		Priority,
		MSYS_STARTING_CURSORVAL,
		QuickButtonCallbackMMove,
		QuickButtonCallbackMButn
	);

	MSYS_SetRegionUserData(&b->Area, 0, BtnID);

#ifdef BUTTONSYSTEM_DEBUGGING
	AssertFailIfIdenticalButtonAttributesFound(b);
#endif

	ButtonList[BtnID] = b;

	SpecifyButtonSoundScheme(BtnID, BUTTON_SOUND_SCHEME_GENERIC);

	return b;
}


static void CopyButtonText(GUI_BUTTON* b, const wchar_t* text)
{
	if (text == NULL || text[0] == L'\0') return;

	wchar_t* Buf = MemAlloc((wcslen(text) + 1) * sizeof(*Buf));
	AssertMsg(Buf != NULL, "Out of memory error:  Couldn't allocate string in CopyButtonText.");
	wcscpy(Buf, text);
	b->string = Buf;
}


static void DefaultMoveCallback(GUI_BUTTON* btn, INT32 reason);


INT32 CreateIconButton(INT16 Icon, INT16 IconIndex, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	// if button size is too small, adjust it.
	if (w < 4) w = 4;
	if (h < 3) h = 3;

	GUI_BUTTON* b = AllocateButton(0, BUTTON_GENERIC, xloc, yloc, w, h, Priority, ClickCallback, DefaultMoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	b->icon        = GenericButtonIcons[Icon];
	b->usIconIndex = IconIndex;

	return b->IDNum;
}


INT32 CreateTextButton(const wchar_t *string, UINT32 uiFont, INT16 sForeColor, INT16 sShadowColor, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	// if button size is too small, adjust it.
	if (w < 4) w = 4;
	if (h < 3) h = 3;

	GUI_BUTTON* b = AllocateButton(0, BUTTON_GENERIC, xloc, yloc, w, h, Priority, ClickCallback, DefaultMoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	CopyButtonText(b, string);
	b->usFont       = uiFont;
	b->sForeColor   = sForeColor;
	b->sShadowColor = sShadowColor;

	return b->IDNum;
}


INT32 CreateHotSpot(INT16 xloc, INT16 yloc, INT16 Width, INT16 Height, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	GUI_BUTTON* b = AllocateButton(0xFFFFFFFF, BUTTON_HOT_SPOT, xloc, yloc, Width, Height, Priority, ClickCallback, DefaultMoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	return b->IDNum;
}


BOOLEAN SetButtonCursor(INT32 iBtnId, UINT16 crsr)
{
	GUI_BUTTON* b = GetButton(iBtnId);
	CHECKF(b != NULL); // XXX HACK000C
  b->Area.Cursor = crsr;
	return TRUE;
}


static INT32 QuickCreateButtonInternal(UINT32 Image, INT16 xloc, INT16 yloc, INT32 Type, INT16 Priority, GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback)
{
	AssertMsg(0 <= Image && Image < MAX_BUTTON_PICS, String("Attempting to QuickCreateButton with out of range ImageID %d.", Image));

	// Is there a QuickButton image in the given image slot?
	const BUTTON_PICS* const BtnPic = &ButtonPictures[Image];
	if (BtnPic->vobj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "QuickCreateButton: Invalid button image number");
		return BUTTON_NO_SLOT;
	}

	GUI_BUTTON* const b = AllocateButton(Image, (Type & (BUTTON_CHECKBOX | BUTTON_NEWTOGGLE)) | BUTTON_QUICK, xloc, yloc, BtnPic->max.w, BtnPic->max.h, Priority, ClickCallback, MoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	return b->IDNum;
}


INT32 QuickCreateButton(UINT32 image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click)
{
	return QuickCreateButtonInternal(image, x, y, BUTTON_TOGGLE, priority, DefaultMoveCallback, click);
}


INT32 QuickCreateButtonNoMove(UINT32 image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click)
{
	return QuickCreateButtonInternal(image, x, y, BUTTON_TOGGLE, priority, MSYS_NO_CALLBACK, click);
}


INT32 QuickCreateButtonToggle(UINT32 image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click)
{
	return QuickCreateButtonInternal(image, x, y, BUTTON_NEWTOGGLE, priority, MSYS_NO_CALLBACK, click);
}


INT32 QuickCreateButtonImg(const char* gfx, INT32 grayed, INT32 off_normal, INT32 off_hilite, INT32 on_normal, INT32 on_hilite, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click)
{
	INT32 img = LoadButtonImage(gfx, grayed, off_normal, off_hilite, on_normal, on_hilite);
	INT32 btn = QuickCreateButton(img, x, y, priority, click);
	ButtonList[btn]->uiFlags |= BUTTON_SELFDELETE_IMAGE;
	return btn;
}


INT32 CreateEasyButton(INT32 x, INT32 y, const char* filename, GUI_CALLBACK ClickCallback)
{
	return CreateSimpleButton(x, y, filename, MSYS_PRIORITY_NORMAL, ClickCallback);
}


INT32 CreateSimpleButton(INT32 x, INT32 y, const char* filename, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	AssertMsg(filename != NULL, "Attempting to CreateSimpleButton with null filename.");

	INT32 ButPic = LoadButtonImage(filename, -1, 1, 2, 3, 4);
	if (ButPic == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Can't load button image");
		return BUTTON_NO_SLOT;
	}

	INT32 ButNum = QuickCreateButton(ButPic, x, y, Priority, ClickCallback);
	AssertMsg(ButNum != BUTTON_NO_SLOT, "Failed to CreateSimpleButton.");

	ButtonList[ButNum]->uiFlags |= BUTTON_SELFDELETE_IMAGE;

	SpecifyDisabledButtonStyle(ButNum, DISABLED_STYLE_SHADED);

	return ButNum;
}


INT32 CreateIconAndTextButton(INT32 Image, const wchar_t* string, UINT32 uiFont, INT16 sForeColor, INT16 sShadowColor, INT16 sForeColorDown, INT16 sShadowColorDown, INT16 xloc, INT16 yloc, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	const INT32 id = QuickCreateButton(Image, xloc, yloc, Priority, ClickCallback);
	if (id != BUTTON_NO_SLOT)
	{
		GUI_BUTTON* const b = GetButton(id);
		CopyButtonText(b, string);
		b->usFont           = uiFont;
		b->sForeColor       = sForeColor;
		b->sShadowColor     = sShadowColor;
		b->sForeColorDown   = sForeColorDown;
		b->sShadowColorDown = sShadowColorDown;
	}

	return id;
}


INT32 CreateLabel(const wchar_t* text, UINT32 font, INT16 forecolor, INT16 shadowcolor, INT16 x, INT16 y, INT16 w, INT16 h, INT16 priority)
{
	INT32 btn = CreateTextButton(text, font, forecolor, shadowcolor, x, y, w, h, priority, NULL);
	SpecifyDisabledButtonStyle(btn, DISABLED_STYLE_NONE);
	DisableButton(btn);
	return btn;
}


void SpecifyButtonText(INT32 iButtonID, const wchar_t* string)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C

	//free the previous strings memory if applicable
	if (b->string) MemFree(b->string);
	b->string = NULL;

	CopyButtonText(b, string);
	b->uiFlags |= BUTTON_DIRTY;
}


void SpecifyButtonDownTextColors(INT32 iButtonID, INT16 sForeColorDown, INT16 sShadowColorDown)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	b->sForeColorDown   = sForeColorDown;
	b->sShadowColorDown = sShadowColorDown;
	b->uiFlags |= BUTTON_DIRTY;
}


void SpecifyButtonHilitedTextColors(INT32 iButtonID, INT16 sForeColorHilited, INT16 sShadowColorHilited)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	b->sForeColorHilited   = sForeColorHilited;
	b->sShadowColorHilited = sShadowColorHilited;
	b->uiFlags |= BUTTON_DIRTY;
}


void SpecifyButtonTextJustification(INT32 iButtonID, INT8 bJustification)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	Assert(bJustification >= BUTTON_TEXT_LEFT && bJustification <= BUTTON_TEXT_RIGHT);
	b->bJustification = bJustification;
	b->uiFlags |= BUTTON_DIRTY;
}


void SpecifyGeneralButtonTextAttributes(INT32 iButtonID, const wchar_t* string, INT32 uiFont, INT16 sForeColor, INT16 sShadowColor)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	//Copy over information
	SpecifyButtonText(iButtonID, string);
	b->usFont = uiFont;
	b->sForeColor = sForeColor;
	b->sShadowColor = sShadowColor;
	b->uiFlags |= BUTTON_DIRTY;
}


void SpecifyButtonTextOffsets(INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	//Copy over information
	b->bTextXOffset = bTextXOffset;
	b->bTextYOffset = bTextYOffset;
	b->fShiftText = fShiftText;
}


void SpecifyButtonTextSubOffsets(INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	//Copy over information
	b->bTextXSubOffSet = bTextXOffset;
	b->bTextYSubOffSet = bTextYOffset;
	b->fShiftText = fShiftText;
}


void SpecifyButtonTextWrappedWidth(INT32 iButtonID, INT16 sWrappedWidth)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	b->sWrappedWidth = sWrappedWidth;
}


void SpecifyDisabledButtonStyle(INT32 iButtonID, INT8 bStyle)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C

	Assert(bStyle >= DISABLED_STYLE_NONE && bStyle <= DISABLED_STYLE_SHADED);

	b->bDisabledStyle = bStyle;
}


BOOLEAN SpecifyButtonIcon(const INT32 iButtonID, const SGPVObject* const icon, const UINT16 usVideoObjectIndex, const INT8 bXOffset, const INT8 bYOffset, const BOOLEAN fShiftImage)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKF(b != NULL); // XXX HACK000C

	b->icon        = icon;
	b->usIconIndex = usVideoObjectIndex;

	if (icon == NO_VOBJECT) return FALSE;

	b->bIconXOffset = bXOffset;
	b->bIconYOffset = bYOffset;
	b->fShiftImage  = TRUE;

	b->uiFlags |= BUTTON_DIRTY;

	return TRUE;
}


void AllowDisabledButtonFastHelp(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C
	b->Area.uiFlags |= MSYS_ALLOW_DISABLED_FASTHELP;
}


void SetButtonFastHelpText(INT32 iButton, const wchar_t* Text)
{
	GUI_BUTTON* b = GetButton(iButton);
	CHECKV(b != NULL); // XXX HACK000C
	SetRegionFastHelpText(&b->Area, Text);
}


/* Dispatches all button callbacks for mouse movement. This function gets
 * called by the Mouse System. *DO NOT CALL DIRECTLY*
 */
static void QuickButtonCallbackMMove(MOUSE_REGION* reg, INT32 reason)
{
	Assert(reg != NULL);

	INT32 iButtonID = MSYS_GetRegionUserData(reg, 0);
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C

	if (b->uiFlags & BUTTON_ENABLED &&
			reason & (MSYS_CALLBACK_REASON_LOST_MOUSE | MSYS_CALLBACK_REASON_GAIN_MOUSE))
	{
		b->uiFlags |= BUTTON_DIRTY;
	}

	// Mouse moved on the button, so reset it's timer to maximum.
	if (b->Area.uiFlags & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		//check for sound playing stuff
		if (b->ubSoundSchemeID)
		{
			if (&b->Area == MSYS_PrevRegion && !gpAnchoredButton)
			{
				INT32 snd = (b->uiFlags & BUTTON_ENABLED ? BUTTON_SOUND_MOVED_ONTO : BUTTON_SOUND_DISABLED_MOVED_ONTO);
				PlayButtonSound(b, snd);
			}
		}
	}
	else
	{
		//Check if we should play a sound
		if (b->ubSoundSchemeID)
		{
			if (b->uiFlags & BUTTON_ENABLED)
			{
				if (&b->Area == MSYS_PrevRegion && !gpAnchoredButton)
				{
					PlayButtonSound(b, BUTTON_SOUND_MOVED_OFF_OF);
				}
			}
			else
			{
				PlayButtonSound(b, BUTTON_SOUND_DISABLED_MOVED_OFF_OF);
			}
		}
	}

	// ATE: New stuff for toggle buttons that work with new Win95 paradigm
	if (b->uiFlags & BUTTON_NEWTOGGLE &&
			reason & MSYS_CALLBACK_REASON_LOST_MOUSE &&
			b->ubToggleButtonActivated)
	{
		b->uiFlags ^= BUTTON_CLICKED_ON;
		b->ubToggleButtonActivated = FALSE;
	}

	/* If this button is enabled and there is a callback function associated with
	 * it, call the callback function.
	 */
	if (b->uiFlags & BUTTON_ENABLED && b->MoveCallback != NULL)
		b->MoveCallback(b, reason);
}


/* Dispatches all button callbacks for button presses. This function is called
 * by the Mouse System. *DO NOT CALL DIRECTLY*
 */
static void QuickButtonCallbackMButn(MOUSE_REGION* reg, INT32 reason)
{
	Assert(reg != NULL);

	INT32 iButtonID = MSYS_GetRegionUserData(reg, 0);
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C

	BOOLEAN MouseBtnDown = (reason & (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_RBUTTON_DWN)) != 0;
	BOOLEAN StateBefore  = (b->uiFlags & BUTTON_CLICKED_ON) != 0;

	// ATE: New stuff for toggle buttons that work with new Win95 paridigm
	if (b->uiFlags & BUTTON_NEWTOGGLE && b->uiFlags & BUTTON_ENABLED)
	{
		if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
		{
			if (!b->ubToggleButtonActivated)
			{
				b->uiFlags ^= BUTTON_CLICKED_ON;
				b->ubToggleButtonActivated = TRUE;
			}
		}
		else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			b->ubToggleButtonActivated = FALSE;
		}
	}

	BOOLEAN StateAfter;

	/* Kris:
	 * Set the anchored button incase the user moves mouse off region while still
	 * holding down the button, but only if the button is up.  In Win95, buttons
	 * that are already down, and anchored never change state, unless you release
	 * the mouse in the button area.
	 */
	if (b->MoveCallback == DefaultMoveCallback && b->uiFlags & BUTTON_ENABLED)
	{
		if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
		{
			gpAnchoredButton = b;
			gfAnchoredState = StateBefore;
			b->uiFlags |= BUTTON_CLICKED_ON;
		}
		else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			b->uiFlags &= ~BUTTON_CLICKED_ON;
		}
	}
	else if (b->uiFlags & BUTTON_CHECKBOX)
	{
		if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
		{
			/* The check box button gets anchored, though it doesn't actually use the
			 * anchoring move callback.  The effect is different, we don't want to
			 * toggle the button state, but we do want to anchor this button so that
			 * we don't effect any other buttons while we move the mouse around in
			 * anchor mode.
			 */
			gpAnchoredButton = b;
			gfAnchoredState = StateBefore;

			/* Trick the before state of the button to be different so the sound will
			 * play properly as checkbox buttons are processed differently.
			 */
			StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? FALSE : TRUE;
			StateAfter = !StateBefore;
		}
		else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			b->uiFlags ^= BUTTON_CLICKED_ON; //toggle the checkbox state upon release inside button area.
			/* Trick the before state of the button to be different so the sound will
			 * play properly as checkbox buttons are processed differently.
			 */
			StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? FALSE : TRUE;
			StateAfter = !StateBefore;
		}
	}

	// Should we play a sound if clicked on while disabled?
	if (b->ubSoundSchemeID && !(b->uiFlags & BUTTON_ENABLED) && MouseBtnDown)
	{
		PlayButtonSound(b, BUTTON_SOUND_DISABLED_CLICK);
	}

	// If this button is disabled, and no callbacks allowed when disabled callback
	if (!(b->uiFlags & BUTTON_ENABLED)) return;

	// If there is a callback function with this button, call it
	if (b->ClickCallback != NULL)
	{
		/* Kris:  January 6, 1998
		 * Added these checks to avoid a case where it was possible to process a
		 * leftbuttonup message when the button wasn't anchored, and should have
		 * been.
		 */
		gfDelayButtonDeletion = TRUE;
		if (!(reason & MSYS_CALLBACK_REASON_LBUTTON_UP) ||
				b->MoveCallback != DefaultMoveCallback ||
				gpPrevAnchoredButton == b)
		{
			b->ClickCallback(b, reason);
		}
		gfDelayButtonDeletion = FALSE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// Otherwise, do default action with this button.
		b->uiFlags ^= BUTTON_CLICKED_ON;
	}

	if (b->uiFlags & BUTTON_CHECKBOX)
	{
		StateAfter = (b->uiFlags & BUTTON_CLICKED_ON) != 0;
	}

	// Play sounds for this enabled button (disabled sounds have already been done)
	if (b->ubSoundSchemeID && b->uiFlags & BUTTON_ENABLED)
	{
		if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			if (b->ubSoundSchemeID && StateBefore && !StateAfter)
			{
				PlayButtonSound(b, BUTTON_SOUND_CLICKED_OFF);
			}
		}
		else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
		{
			if (b->ubSoundSchemeID && !StateBefore && StateAfter)
			{
				PlayButtonSound(b, BUTTON_SOUND_CLICKED_ON);
			}
		}
	}

#if defined JA2
	if (StateBefore != StateAfter)
	{
		InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
	}
#endif

	if (gfPendingButtonDeletion) RemoveButtonsMarkedForDeletion();
}


static void DrawButtonFromPtr(GUI_BUTTON* b);


void RenderButtons(void)
{
	SaveFontSettings();
	for (INT32 iButtonID = 0; iButtonID < MAX_BUTTONS; ++iButtonID)
	{
		// If the button exists, and it's not owned by another object, draw it
		// Kris:  and make sure that the button isn't hidden.
		GUI_BUTTON* b = ButtonList[iButtonID];
		if (b == NULL || !(b->Area.uiFlags & MSYS_REGION_ENABLED)) continue;

		if ((b->uiFlags ^ b->uiOldFlags) & (BUTTON_CLICKED_ON | BUTTON_ENABLED))
		{
			// Something is different, set dirty!
			b->uiFlags |= BUTTON_DIRTY;
		}

		// Set old flags
		b->uiOldFlags = b->uiFlags;

		if (b->uiFlags & BUTTON_FORCE_UNDIRTY)
		{
			b->uiFlags &= ~BUTTON_DIRTY;
			b->uiFlags &= ~BUTTON_FORCE_UNDIRTY;
		}

		// Check if we need to update!
		if (b->uiFlags & BUTTON_DIRTY)
		{
			// Turn off dirty flag
			b->uiFlags &= ~BUTTON_DIRTY;
			DrawButtonFromPtr(b);

#if defined JA2
			InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
#endif
		}
	}

	RestoreFontSettings();
}


void MarkAButtonDirty(INT32 iButtonNum)
{
  // surgical dirtying -> marks a user specified button dirty, without dirty the whole lot of them
	GUI_BUTTON* b = GetButton(iButtonNum);
	CHECKV(b != NULL); // XXX HACK000C
	b->uiFlags |= BUTTON_DIRTY;
}


void MarkButtonsDirty(void)
{
	for (INT32 x = 0; x < MAX_BUTTONS; ++x)
	{
		if (ButtonList[x]) ButtonList[x]->uiFlags |= BUTTON_DIRTY;
	}
}


void UnMarkButtonDirty(INT32 iButtonIndex)
{
	GUI_BUTTON* b = GetButton(iButtonIndex);
	CHECKV(b != NULL); // XXX HACK000C
	b->uiFlags &= ~BUTTON_DIRTY;
}


void UnmarkButtonsDirty(void)
{
	for (INT32 x = 0; x < MAX_BUTTONS; ++x)
	{
		if (ButtonList[x]) UnMarkButtonDirty(x);
	}
}


void ForceButtonUnDirty(INT32 iButtonIndex)
{
	GUI_BUTTON* b = GetButton(iButtonIndex);
	CHECKV(b != NULL); // XXX HACK000C
	b->uiFlags &= ~BUTTON_DIRTY;
	b->uiFlags |= BUTTON_FORCE_UNDIRTY;
}


BOOLEAN DrawButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKF(b != NULL); // XXX HACK000C
	if (b->string != NULL) SaveFontSettings();
	if (b->Area.uiFlags & MSYS_REGION_ENABLED) DrawButtonFromPtr(b);
	if (b->string != NULL) RestoreFontSettings();
	return TRUE;
}


static void DrawCheckBoxButton(const GUI_BUTTON* b);
static void DrawGenericButton( const GUI_BUTTON* b);
static void DrawHatchOnButton( const GUI_BUTTON* b);
static void DrawIconOnButton(  const GUI_BUTTON* b);
static void DrawQuickButton(   const GUI_BUTTON* b);
static void DrawShadeOnButton( const GUI_BUTTON* b);
static void DrawTextOnButton(  const GUI_BUTTON* b);


// Given a pointer to a GUI_BUTTON structure, draws the button on the screen.
static void DrawButtonFromPtr(GUI_BUTTON* b)
{
	// Draw the appropriate button according to button type
	gbDisabledButtonStyle = DISABLED_STYLE_NONE;
	switch (b->uiFlags & BUTTON_TYPES)
	{
		case BUTTON_QUICK:    DrawQuickButton(b);    break;
		case BUTTON_GENERIC:  DrawGenericButton(b);  break;
		case BUTTON_CHECKBOX: DrawCheckBoxButton(b); break;

		case BUTTON_HOT_SPOT:
			return; // hotspots don't have text, but if you want to, change this to a break!
	}
	if (b->icon != NO_VOBJECT) DrawIconOnButton(b);
	if (b->string)             DrawTextOnButton(b);
	/* If the button is disabled, and a style has been calculated, then draw the
	 * style last.
	 */
	switch (gbDisabledButtonStyle)
	{
		case DISABLED_STYLE_HATCHED: DrawHatchOnButton(b); break;
		case DISABLED_STYLE_SHADED:  DrawShadeOnButton(b); break;
	}
}


// Draws a QuickButton type button on the screen.
static void DrawQuickButton(const GUI_BUTTON* b)
{
	const BUTTON_PICS* const pics = &ButtonPictures[b->ImageNum];

	INT32 UseImage = 0;
	if (b->uiFlags & BUTTON_ENABLED)
	{
		if (b->uiFlags & BUTTON_CLICKED_ON)
		{
			// Is the mouse over this area, and we have a hilite image?
			if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA &&
					gfRenderHilights &&
					pics->OnHilite != -1)
			{
				UseImage = pics->OnHilite;
			}
			else if (pics->OnNormal != -1)
			{
				UseImage = pics->OnNormal;
			}
		}
		else
		{
			// Is the mouse over the button, and do we have hilite image?
			if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA &&
					gfRenderHilights &&
					pics->OffHilite != -1)
			{
				UseImage = pics->OffHilite;
			}
			else if (pics->OffNormal != -1)
			{
				UseImage = pics->OffNormal;
			}
		}
	}
	else if (pics->Grayed != -1)
	{
		// Button is diabled so use the "Grayed-out" image
		UseImage = pics->Grayed;
	}
	else
	{
		UseImage = pics->OffNormal;
		switch (b->bDisabledStyle)
		{
			case DISABLED_STYLE_DEFAULT:
				gbDisabledButtonStyle = b->string ? DISABLED_STYLE_SHADED : DISABLED_STYLE_HATCHED;
				break;

			case DISABLED_STYLE_HATCHED:
			case DISABLED_STYLE_SHADED:
				gbDisabledButtonStyle = b->bDisabledStyle;
				break;
		}
	}

	BltVideoObject(ButtonDestBuffer, pics->vobj, UseImage, b->XLoc, b->YLoc);
}


static void DrawHatchOnButton(const GUI_BUTTON* b)
{
	SGPRect ClipRect;
	ClipRect.iLeft   = b->Area.RegionTopLeftX;
	ClipRect.iRight  = b->Area.RegionBottomRightX - 1;
	ClipRect.iTop    = b->Area.RegionTopLeftY;
	ClipRect.iBottom = b->Area.RegionBottomRightY - 1;
	UINT32 uiDestPitchBYTES;
	UINT16* pDestBuf = (UINT16*)LockVideoSurface(ButtonDestBuffer, &uiDestPitchBYTES);
	Blt16BPPBufferHatchRect(pDestBuf, uiDestPitchBYTES, &ClipRect);
	UnLockVideoSurface(ButtonDestBuffer);
}


static void DrawShadeOnButton(const GUI_BUTTON* b)
{
	const MOUSE_REGION* r = &b->Area;
	ShadowVideoSurfaceRect(ButtonDestBuffer, r->RegionTopLeftX, r->RegionTopLeftY, r->RegionBottomRightX, r->RegionBottomRightY);
}


void DrawCheckBoxButtonOnOff(INT32 iButtonID, BOOLEAN on)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	CHECKV(b != NULL); // XXX HACK000C

	BOOLEAN fLeftButtonState = gfLeftButtonState;

	gfLeftButtonState = on;
	b->Area.uiFlags |= MSYS_MOUSE_IN_AREA;
	DrawButton(iButtonID);

	gfLeftButtonState = fLeftButtonState;
}


static void DrawCheckBoxButton(const GUI_BUTTON *b)
{
	const BUTTON_PICS* const pics = &ButtonPictures[b->ImageNum];

	INT32 UseImage = 0;
	if (b->uiFlags & BUTTON_ENABLED)
	{
		if (b->uiFlags & BUTTON_CLICKED_ON)
		{
			// Is the mouse over this area, and we have a hilite image?
			if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA &&
					gfRenderHilights &&
					gfLeftButtonState &&
					pics->OnHilite != -1)
			{
				UseImage = pics->OnHilite;
			}
			else if (pics->OnNormal != -1)
			{
				UseImage = pics->OnNormal;
			}
		}
		else
		{
			// Is the mouse over the button, and do we have hilite image?
			if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA &&
					gfRenderHilights &&
					gfLeftButtonState &&
					pics->OffHilite != -1)
			{
				UseImage = pics->OffHilite;
			}
			else if (pics->OffNormal != -1)
			{
				UseImage = pics->OffNormal;
			}
		}
	}
	else if (pics->Grayed != -1)
	{
		// Button is disabled so use the "Grayed-out" image
		UseImage = pics->Grayed;
	}
	else //use the disabled style
	{
		if (b->uiFlags & BUTTON_CLICKED_ON)
		{
			UseImage = pics->OnHilite;
		}
		else
		{
			UseImage = pics->OffHilite;
		}
		switch (b->bDisabledStyle)
		{
			case DISABLED_STYLE_DEFAULT:
				gbDisabledButtonStyle = DISABLED_STYLE_HATCHED;
				break;

			case DISABLED_STYLE_HATCHED:
			case DISABLED_STYLE_SHADED:
				gbDisabledButtonStyle = b->bDisabledStyle;
				break;
		}
	}

	BltVideoObject(ButtonDestBuffer, pics->vobj, UseImage, b->XLoc, b->YLoc);
}


static void DrawIconOnButton(const GUI_BUTTON* b)
{
	if (b->icon == NO_VOBJECT) return;

	// Get width and height of button area
	INT32 width  = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
	INT32 height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

	// Compute viewable area (inside borders)
	SGPRect NewClip;
	NewClip.iLeft   = b->XLoc + 3;
	NewClip.iRight  = b->XLoc + width - 3;
	NewClip.iTop    = b->YLoc + 2;
	NewClip.iBottom = b->YLoc + height - 2;

	// Get Icon's blit start coordinates
	INT32 IconX = NewClip.iLeft;
	INT32 IconY = NewClip.iTop;

	// Get current clip area
	SGPRect OldClip;
	GetClippingRect(&OldClip);

	// Clip button's viewable area coords to screen
	if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

	// Is button right off the right side of the screen?
	if (NewClip.iLeft > OldClip.iRight) return;

	if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

	// Is button completely off the left side of the screen?
	if (NewClip.iRight < OldClip.iLeft) return;

	if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

	// Are we right off the bottom of the screen?
	if (NewClip.iTop > OldClip.iBottom) return;

	if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

	// Are we off the top?
	if (NewClip.iBottom < OldClip.iTop) return;

	// Did we clip the viewable area out of existance?
	if (NewClip.iRight <= NewClip.iLeft || NewClip.iBottom <= NewClip.iTop) return;

	// Get the width and height of the icon itself
	const SGPVObject*  const hvObject = b->icon;
	const ETRLEObject* const pTrav    = &hvObject->pETRLEObject[b->usIconIndex];

	/* Compute coordinates for centering the icon on the button or use the offset
	 * system.
	 */
	INT32 xp;
	if (b->bIconXOffset == -1)
	{
		const INT32 IconW = pTrav->usWidth  + pTrav->sOffsetX;
		xp = IconX + (width - 6 - IconW) / 2;
	}
	else
	{
		xp = b->Area.RegionTopLeftX + b->bIconXOffset;
	}

	INT32 yp;
	if (b->bIconYOffset == -1)
	{
		const INT32 IconH = pTrav->usHeight + pTrav->sOffsetY;
		yp = IconY + (height - 4 - IconH) / 2;
	}
	else
	{
		yp = b->Area.RegionTopLeftY + b->bIconYOffset;
	}

	/* Was the button clicked on? if so, move the image slightly for the illusion
	 * that the image moved into the screen.
	 */
	if (b->uiFlags & BUTTON_CLICKED_ON && b->fShiftImage)
	{
		xp++;
		yp++;
	}

	// Set the clipping rectangle to the viewable area of the button
	SetClippingRect(&NewClip);

	BltVideoObject(ButtonDestBuffer, hvObject, b->usIconIndex, xp, yp);

	// Restore previous clip region
	SetClippingRect(&OldClip);
}


// If a button has text attached to it, then it'll draw it last.
static void DrawTextOnButton(const GUI_BUTTON* b)
{
	// If this button actually has a string to print
	if (b->string == NULL) return;

	// Get the width and height of this button
	const INT32 width  = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
	const INT32 height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

	// Compute the viewable area on this button
	SGPRect NewClip;
	NewClip.iLeft   = b->XLoc + 3;
	NewClip.iRight  = b->XLoc + width - 3;
	NewClip.iTop    = b->YLoc + 2;
	NewClip.iBottom = b->YLoc + height - 2;

	// Get the starting coordinates to print
	const INT32 TextX = NewClip.iLeft;
	const INT32 TextY = NewClip.iTop;

	// Get the current clipping area
	SGPRect OldClip;
	GetClippingRect(&OldClip);

	// Clip the button's viewable area to the screen
	if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

	// Are we off hte right side?
	if (NewClip.iLeft > OldClip.iRight) return;

	if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

	// Are we off the left side?
	if (NewClip.iRight < OldClip.iLeft) return;

	if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

	// Are we off the bottom of the screen?
	if (NewClip.iTop > OldClip.iBottom) return;

	if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

	// Are we off the top?
	if (NewClip.iBottom < OldClip.iTop) return;

	// Did we clip the viewable area out of existance?
	if (NewClip.iRight <= NewClip.iLeft || NewClip.iBottom <= NewClip.iTop) return;

	// Set the font printing settings to the buttons viewable area
	SetFontDestBuffer(ButtonDestBuffer, NewClip.iLeft, NewClip.iTop, NewClip.iRight, NewClip.iBottom);

	// Compute the coordinates to center the text
	INT32 yp;
	if (b->bTextYOffset == -1)
	{
		yp = TextY + (height - GetFontHeight(b->usFont)) / 2 - 1;
	}
	else
	{
		yp = b->Area.RegionTopLeftY + b->bTextYOffset;
	}

	INT32 xp;
	if (b->bTextXOffset == -1)
	{
		switch (b->bJustification)
		{
			case BUTTON_TEXT_LEFT:   xp = TextX + 3; break;
			case BUTTON_TEXT_RIGHT:  xp = NewClip.iRight - StringPixLength(b->string, b->usFont) - 3; break;
			default:
			case BUTTON_TEXT_CENTER: xp = TextX + (width - 6 - StringPixLength(b->string, b->usFont)) / 2; break;
		}
	}
	else
	{
		xp = b->Area.RegionTopLeftX + b->bTextXOffset;
	}

	// Set the printing font to the button text font
	SetFont(b->usFont);

	// print the text
	SetFontBackground(FONT_MCOLOR_BLACK);

	//Override the colors if necessary.
	INT16 sForeColor;
	if (b->uiFlags & BUTTON_ENABLED && b->Area.uiFlags & MSYS_MOUSE_IN_AREA && b->sForeColorHilited != -1)
	{
		sForeColor = b->sForeColorHilited;
	}
	else if (b->uiFlags & BUTTON_CLICKED_ON && b->sForeColorDown != -1)
	{
		sForeColor = b->sForeColorDown;
	}
	else
	{
		sForeColor = b->sForeColor;
	}
	SetFontForeground(sForeColor);

	if (b->uiFlags & BUTTON_ENABLED && b->Area.uiFlags & MSYS_MOUSE_IN_AREA && b->sShadowColorHilited != -1)
	{
		SetFontShadow(b->sShadowColorHilited);
	}
	else if (b->uiFlags & BUTTON_CLICKED_ON && b->sShadowColorDown != -1)
	{
		SetFontShadow(b->sShadowColorDown);
	}
	else if (b->sShadowColor != -1)
	{
		SetFontShadow(b->sShadowColor);
	}

	if (b->uiFlags & BUTTON_CLICKED_ON && b->fShiftText)
	{
		/* Was the button clicked on? if so, move the text slightly for the illusion
		 * that the text moved into the screen. */
		xp++;
		yp++;
	}

#if defined JA2
	if (b->sWrappedWidth != -1)
	{
		UINT8 bJustified = 0;
		switch (b->bJustification)
		{
			case BUTTON_TEXT_LEFT:    bJustified = LEFT_JUSTIFIED;    break;
			case BUTTON_TEXT_RIGHT:   bJustified = RIGHT_JUSTIFIED;   break;
			case BUTTON_TEXT_CENTER:  bJustified = CENTER_JUSTIFIED;  break;
			default:                  Assert(0);                      break;
		}
		if (b->bTextXOffset == -1)
		{
			/* Kris:
			 * There needs to be recalculation of the start positions based on the
			 * justification and the width specified wrapped width.  I was drawing a
			 * double lined word on the right side of the button to find it drawing
			 * way over to the left.  I've added the necessary code for the right and
			 * center justification.
			 */
			yp = b->Area.RegionTopLeftY + 2;

			switch (b->bJustification)
			{
				case BUTTON_TEXT_RIGHT:
					xp = b->Area.RegionBottomRightX - 3 - b->sWrappedWidth;
					if (b->fShiftText && b->uiFlags & BUTTON_CLICKED_ON)
					{
						xp++;
						yp++;
					}
					break;

				case BUTTON_TEXT_CENTER:
					xp = b->Area.RegionTopLeftX + 3 + b->sWrappedWidth / 2;
					if (b->fShiftText && b->uiFlags & BUTTON_CLICKED_ON)
					{
						xp++;
						yp++;
					}
					break;
			}
		}
		yp += b->bTextYSubOffSet;
		xp += b->bTextXSubOffSet;
		DisplayWrappedString(xp, yp, b->sWrappedWidth, 1, b->usFont, sForeColor, b->string, FONT_MCOLOR_BLACK, bJustified);
	}
	else
	{
		yp += b->bTextYSubOffSet;
		xp += b->bTextXSubOffSet;
		mprintf(xp, yp, b->string);
	}
#else
	mprintf(xp, yp, b->string);
#endif
	// Restore the old text printing settings
}


/* This function is called by the DrawIconicButton and DrawTextButton routines
 * to draw the borders and background of the buttons.
 */
static void DrawGenericButton(const GUI_BUTTON* b)
{
	// Select the graphics to use depending on the current state of the button
	HVOBJECT BPic;
	if (!(b->uiFlags & BUTTON_ENABLED))
	{
		BPic = GenericButtonOffNormal[b->ImageNum];
		switch (b->bDisabledStyle)
		{
			case DISABLED_STYLE_DEFAULT:
				gbDisabledButtonStyle = b->string ? DISABLED_STYLE_SHADED : DISABLED_STYLE_HATCHED;
				break;

			case DISABLED_STYLE_HATCHED:
			case DISABLED_STYLE_SHADED:
				gbDisabledButtonStyle = b->bDisabledStyle;
				break;
		}
	}
	else if (b->uiFlags & BUTTON_CLICKED_ON)
	{
		if  (b->Area.uiFlags & MSYS_MOUSE_IN_AREA && GenericButtonOnHilite[b->ImageNum] != NULL && gfRenderHilights)
		{
			BPic = GenericButtonOnHilite[b->ImageNum];
		}
		else
		{
			BPic = GenericButtonOnNormal[b->ImageNum];
		}
	}
	else
	{
		if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA && GenericButtonOffHilite[b->ImageNum] != NULL && gfRenderHilights)
		{
			BPic = GenericButtonOffHilite[b->ImageNum];
		}
		else
		{
			BPic = GenericButtonOffNormal[b->ImageNum];
		}
	}

#if defined JA2
	const INT32 iBorderWidth  = 3;
	const INT32 iBorderHeight = 2;
#else
	/* DB - Added this to support more flexible sizing of border images.  The 3x2
	 * size was a bit limiting. JA2 should default to the original size, unchanged
	 */
	const ETRLEObject* const pTrav = &BPic->pETRLEObject[0];
	const INT32 iBorderHeight = pTrav->usHeight;
	const INT32 iBorderWidth  = pTrav->usWidth;
#endif

	// Compute the number of button "chunks" needed to be blitted
	const INT32 width         = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
	const INT32 height        = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;
	const INT32 NumChunksWide = width  / iBorderWidth;
	INT32       NumChunksHigh = height / iBorderHeight;
	const INT32 hremain       = height % iBorderHeight;
	const INT32 wremain       = width  % iBorderWidth;

	INT32 cx = b->XLoc + (NumChunksWide - 1) * iBorderWidth  + wremain;
	INT32 cy = b->YLoc + (NumChunksHigh - 1) * iBorderHeight + hremain;

	// Fill the button's area with the button's background color
	ColorFillVideoSurfaceArea(ButtonDestBuffer, b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY, GenericButtonFillColors[b->ImageNum]);

	UINT32  uiDestPitchBYTES;
	UINT16* pDestBuf = (UINT16*)LockVideoSurface(ButtonDestBuffer, &uiDestPitchBYTES);

	SGPRect ClipRect;
	GetClippingRect(&ClipRect);

	// Draw the button's borders and corners (horizontally)
	for (INT32 q = 0; q < NumChunksWide; q++)
	{
		const INT32 ImgNum = (q == 0 ? 0 : 1);
		const INT32 x = b->XLoc + q * iBorderWidth;
		Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, x,  b->YLoc, ImgNum,     &ClipRect);
		Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, x,  cy,      ImgNum + 5, &ClipRect);
	}
	// Blit the right side corners
	Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, cx, b->YLoc, 2, &ClipRect);
	Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, cx, cy,      7, &ClipRect);
	// Draw the vertical members of the button's borders
	NumChunksHigh--;

	if (hremain != 0)
	{
		const INT32 y = b->YLoc + NumChunksHigh * iBorderHeight - iBorderHeight + hremain;
		Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, b->XLoc, y, 3, &ClipRect);
		Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, cx,      y, 4, &ClipRect);
	}

	for (INT32 q = 1; q < NumChunksHigh; q++)
	{
		const INT32 y = b->YLoc + q * iBorderHeight;
		Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, b->XLoc, y, 3, &ClipRect);
		Blt8BPPDataTo16BPPBufferTransparentClip(pDestBuf, uiDestPitchBYTES, BPic, cx,      y, 4, &ClipRect);
	}

	UnLockVideoSurface(ButtonDestBuffer);
}


INT32 CreateCheckBoxButton(INT16 x, INT16 y, const char* filename, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	Assert(filename != NULL);
	INT32 ButPic = LoadButtonImage(filename, -1, 0, 1, 2, 3);
	if (ButPic == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateCheckBoxButton: Can't load button image");
		return BUTTON_NO_SLOT;
	}

	INT32 iButtonID = QuickCreateButtonInternal(ButPic, x, y, BUTTON_CHECKBOX, Priority, MSYS_NO_CALLBACK, ClickCallback);
	if (iButtonID == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateCheckBoxButton: Can't create button");
		return BUTTON_NO_SLOT;
	}

	//change the flags so that it isn't a quick button anymore
	GUI_BUTTON* b = ButtonList[iButtonID];
	b->uiFlags &= ~BUTTON_QUICK;
	b->uiFlags |= BUTTON_CHECKBOX | BUTTON_SELFDELETE_IMAGE;

	return iButtonID;
}


void MSYS_SetBtnUserData(INT32 iButtonNum, INT32 userdata)
{
	GUI_BUTTON* b = GetButton(iButtonNum);
	CHECKV(b != NULL); // XXX HACK000C
	b->User.Data = userdata;
}


INT32 MSYS_GetBtnUserData(const GUI_BUTTON* b)
{
	return b->User.Data;
}


/* Generic Button Movement Callback to reset the mouse button if the mouse is no
 * longer in the button region.
 */
static void DefaultMoveCallback(GUI_BUTTON* btn, INT32 reason)
{
	// If the button isn't the anchored button, then we don't want to modify the button state.
	if (btn != gpAnchoredButton) return;

	if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		if (!gfAnchoredState)
		{
			btn->uiFlags &= ~BUTTON_CLICKED_ON;
			if (btn->ubSoundSchemeID)
			{
				PlayButtonSound(btn, BUTTON_SOUND_CLICKED_OFF);
			}
		}
#if defined JA2
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
#endif
	}
	else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		if (btn->ubSoundSchemeID)
		{
			PlayButtonSound(btn, BUTTON_SOUND_CLICKED_ON);
		}
#if defined JA2
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
#endif
	}
}


void ReleaseAnchorMode(void)
{
  if (!gpAnchoredButton) return;

	if (gusMouseXPos < gpAnchoredButton->Area.RegionTopLeftX     ||
			gusMouseXPos > gpAnchoredButton->Area.RegionBottomRightX ||
			gusMouseYPos < gpAnchoredButton->Area.RegionTopLeftY     ||
			gusMouseYPos > gpAnchoredButton->Area.RegionBottomRightY)
	{
		//released outside button area, so restore previous button state.
		if (gfAnchoredState)
		{
			gpAnchoredButton->uiFlags |= BUTTON_CLICKED_ON;
		}
		else
		{
			gpAnchoredButton->uiFlags &= ~BUTTON_CLICKED_ON;
		}
#if defined JA2
		InvalidateRegion(gpAnchoredButton->Area.RegionTopLeftX, gpAnchoredButton->Area.RegionTopLeftY, gpAnchoredButton->Area.RegionBottomRightX, gpAnchoredButton->Area.RegionBottomRightY);
#endif
	}
	gpPrevAnchoredButton = gpAnchoredButton;
	gpAnchoredButton = NULL;
}


void HideButton(INT32 iButtonNum)
{
	GUI_BUTTON* b = GetButton(iButtonNum);
	CHECKV(b != NULL); // XXX HACK000C
	b->Area.uiFlags &= ~MSYS_REGION_ENABLED;
	b->uiFlags |= BUTTON_DIRTY;
#if defined JA2
	InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
#endif
}


void ShowButton(INT32 iButtonNum)
{
	GUI_BUTTON* b = GetButton(iButtonNum);
	CHECKV(b != NULL); // XXX HACK000C
	b->Area.uiFlags |= MSYS_REGION_ENABLED;
	b->uiFlags |= BUTTON_DIRTY;
#if defined JA2
	InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
#endif
}


UINT16 GetGenericButtonFillColor(void)
{
	return GenericButtonFillColors[0];
}
