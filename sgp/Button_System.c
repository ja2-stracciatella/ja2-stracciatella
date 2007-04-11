// Rewritten mostly by Kris Morness

#include "Button_Sound_Control.h"
#include "Button_System.h"
#include "Debug.h"
#include "Font.h"
#include "Input.h"
#include "MemMan.h"
#include "SGP.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "WordWrap.h"

#ifdef _JA2_RENDER_DIRTY
#	include "Font_Control.h"
#	include "Utilities.h"
#endif


// Moved here from Button System.c by DB 99/01/07
// Names of the default generic button image files.
#ifdef JA2
#define DEFAULT_GENERIC_BUTTON_OFF			"GENBUTN.STI"
#define DEFAULT_GENERIC_BUTTON_ON				"GENBUTN2.STI"
#define DEFAULT_GENERIC_BUTTON_OFF_HI		"GENBUTN3.STI"
#define DEFAULT_GENERIC_BUTTON_ON_HI		"GENBUTN4.STI"
#else
#define DEFAULT_GENERIC_BUTTON_OFF			"Data/Message Box/GENBUTN.STI"
#define DEFAULT_GENERIC_BUTTON_ON				"Data/Message Box/GENBUTN2.STI"
#define DEFAULT_GENERIC_BUTTON_OFF_HI		"Data/Message Box/GENBUTN3.STI"
#define DEFAULT_GENERIC_BUTTON_ON_HI		"Data/Message Box/GENBUTN4.STI"
#endif


#define		MSYS_STARTING_CURSORVAL		0


#define MAX_GENERIC_PICS		40
#define MAX_BUTTON_ICONS		40


#define GUI_BTN_NONE							0
#define GUI_BTN_DUPLICATE_VOBJ		1


static UINT8 str[128];

//Kris:  December 2, 1997
//Special internal debugging utilities that will ensure that you don't attempt to delete
//an already deleted button, or it's images, etc.  It will also ensure that you don't create
//the same button that already exists.
//TO REMOVE ALL DEBUG FUNCTIONALITY:  simply comment out BUTTONSYSTEM_DEBUGGING definition
#if defined(JA2) && defined(_DEBUG)
#	define BUTTONSYSTEM_DEBUGGING
#endif


#ifdef BUTTONSYSTEM_DEBUGGING

BOOLEAN gfIgnoreShutdownAssertions;


//Called immediately before assigning the button to the button list.
static void AssertFailIfIdenticalButtonAttributesFound( GUI_BUTTON *b )
{
	INT32 x;
	GUI_BUTTON *c;
	for( x = 0; x < MAX_BUTTONS; x++ )
	{
		c = ButtonList[ x ];
		if( !c )																													continue;
		if( c->uiFlags									&  BUTTON_DELETION_PENDING		)   continue;
		if( c->UserData[3]							== 0xffffffff									)		continue;
		if( b->Area.PriorityLevel				!= c->Area.PriorityLevel			)		continue;
		if( b->Area.RegionTopLeftX			!= c->Area.RegionTopLeftX			)		continue;
		if( b->Area.RegionTopLeftY			!= c->Area.RegionTopLeftY			)		continue;
		if( b->Area.RegionBottomRightX	!= c->Area.RegionBottomRightX )		continue;
		if( b->Area.RegionBottomRightY	!= c->Area.RegionBottomRightY )		continue;
		if( b->ClickCallback						!= c->ClickCallback						)		continue;
		if( b->MoveCallback							!= c->MoveCallback						)		continue;
		if( b->XLoc											!= c->XLoc										)		continue;
		if( b->YLoc											!= c->YLoc										)		continue;
		//if we get this far, it is reasonably safe to assume that the newly created
		//button already exists.  Placing a break point on the following assert will
		//allow the coder to easily isolate the case!
		sprintf( str, "Attempting to create a button that has already been created (existing buttonID %d).", c->IDNum );
		AssertMsg( 0, str );
	}
}

#endif


//Kris:
//These are the variables used for the anchoring of a particular button.
//When you click on a button, it get's anchored, until you release the mouse button.
//When you move around, you don't want to select other buttons, even when you release
//it.  This follows the Windows 95 convention.
static GUI_BUTTON *gpAnchoredButton;
static GUI_BUTTON *gpPrevAnchoredButton;
static BOOLEAN gfAnchoredState;

static INT8 gbDisabledButtonStyle;
static void DrawShadeOnButton(const GUI_BUTTON *b);
static void DrawHatchOnButton(const GUI_BUTTON *b);

BOOLEAN gfRenderHilights = TRUE;

BUTTON_PICS		ButtonPictures[MAX_BUTTON_PICS];
static INT32 ButtonPicsLoaded;

UINT32 ButtonDestBuffer = FRAME_BUFFER;

GUI_BUTTON *ButtonList[MAX_BUTTONS];


UINT16 GetWidthOfButtonPic( UINT16 usButtonPicID, INT32 iSlot )
{
	return ButtonPictures[ usButtonPicID ].vobj->pETRLEObject[ iSlot ].usWidth;
}

static HVOBJECT GenericButtonOffNormal[MAX_GENERIC_PICS];
static HVOBJECT GenericButtonOffHilite[MAX_GENERIC_PICS];
static HVOBJECT GenericButtonOnNormal[MAX_GENERIC_PICS];
static HVOBJECT GenericButtonOnHilite[MAX_GENERIC_PICS];
UINT16 GenericButtonFillColors[MAX_GENERIC_PICS];

static HVOBJECT GenericButtonIcons[MAX_BUTTON_ICONS];

static BOOLEAN gfDelayButtonDeletion = FALSE;
static BOOLEAN gfPendingButtonDeletion = FALSE;

extern MOUSE_REGION *MSYS_PrevRegion;


// Finds an available slot for loading button pictures
static INT32 FindFreeButtonSlot(void)
{
	int slot;

	// Are there any slots available?
	if(ButtonPicsLoaded >= MAX_BUTTON_PICS)
		return(BUTTON_NO_SLOT);

	// Search for a slot
	for(slot=0;slot<MAX_BUTTON_PICS;slot++)
	{
		if(ButtonPictures[slot].vobj==NULL)
			return(slot);
	}

	return(BUTTON_NO_SLOT);
}


static void InitButtonImage(UINT32 UseSlot, HVOBJECT VObj, UINT32 Flags, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite)
{
	ButtonPictures[UseSlot].vobj = VObj;

	// Init the QuickButton image structure with indexes to use
	ButtonPictures[UseSlot].Grayed    = Grayed;
	ButtonPictures[UseSlot].OffNormal = OffNormal;
	ButtonPictures[UseSlot].OffHilite = OffHilite;
	ButtonPictures[UseSlot].OnNormal  = OnNormal;
	ButtonPictures[UseSlot].OnHilite  = OnHilite;
	ButtonPictures[UseSlot].fFlags    = Flags;

	// Fit the button size to the largest image in the set
	UINT32 MaxWidth  = 0;
	UINT32 MaxHeight = 0;
	if (Grayed != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &VObj->pETRLEObject[Grayed];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OffNormal != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &VObj->pETRLEObject[OffNormal];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OffHilite != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &VObj->pETRLEObject[OffHilite];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OnNormal != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &VObj->pETRLEObject[OnNormal];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	if (OnHilite != BUTTON_NO_IMAGE)
	{
		const ETRLEObject* pTrav = &VObj->pETRLEObject[OnHilite];
		UINT32 ThisHeight = pTrav->usHeight + pTrav->sOffsetY;
		UINT32 ThisWidth  = pTrav->usWidth  + pTrav->sOffsetX;

		if (MaxWidth  < ThisWidth)  MaxWidth  = ThisWidth;
		if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
	}

	// Set the width and height for this image set
	ButtonPictures[UseSlot].MaxHeight = MaxHeight;
	ButtonPictures[UseSlot].MaxWidth  = MaxWidth;

	ButtonPicsLoaded++;
}


// Load images for use with QuickButtons.
INT32 LoadButtonImage(const char *filename, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite)
{
	AssertMsg(filename != NULL, "Attempting to LoadButtonImage() with null filename.");
	AssertMsg(strlen(filename), "Attempting to LoadButtonImage() with empty filename string.");

	if (Grayed    == BUTTON_NO_IMAGE &&
			OffNormal == BUTTON_NO_IMAGE &&
			OffHilite == BUTTON_NO_IMAGE &&
			OnNormal  == BUTTON_NO_IMAGE &&
			OnHilite  == BUTTON_NO_IMAGE)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("No button pictures selected for %s", filename));
		return(-1);
	}

	UINT32 UseSlot = FindFreeButtonSlot();
	if (UseSlot == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Out of button image slots for %s", filename));
		return(-1);
	}

	HVOBJECT VObj = CreateVideoObjectFromFile(filename);
	if (VObj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Couldn't create VOBJECT for %s", filename));
		return(-1);
	}

	InitButtonImage(UseSlot, VObj, GUI_BTN_NONE, Grayed, OffNormal, OffHilite, OnNormal, OnHilite);
	return UseSlot;
}


// Uses a previously loaded quick button image for use with QuickButtons.
// The function simply duplicates the vobj!
INT32 UseLoadedButtonImage(INT32 LoadedImg, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite)
{
	if (Grayed    == BUTTON_NO_IMAGE &&
			OffNormal == BUTTON_NO_IMAGE &&
			OffHilite == BUTTON_NO_IMAGE &&
			OnNormal  == BUTTON_NO_IMAGE &&
			OnHilite  == BUTTON_NO_IMAGE)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("No button pictures selected for pre-loaded button image %d",LoadedImg));
		return(-1);
	}

	UINT32 UseSlot = FindFreeButtonSlot();
	if (UseSlot == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Out of button image slots for pre-loaded button image %d",LoadedImg));
		return(-1);
	}

	// Is button image index given valid?
	if (ButtonPictures[LoadedImg].vobj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("Invalid button picture handle given for pre-loaded button image %d",LoadedImg));
		return(-1);
	}

	InitButtonImage(UseSlot, ButtonPictures[LoadedImg].vobj, GUI_BTN_DUPLICATE_VOBJ, Grayed, OffNormal, OffHilite, OnNormal, OnHilite);
	return UseSlot;
}


//Removes a QuickButton image from the system.
void UnloadButtonImage(INT32 Index)
{
	INT32 x;
	BOOLEAN fDone;

	if( Index < 0 || Index >= MAX_BUTTON_PICS )
	{
		sprintf( str, "Attempting to UnloadButtonImage with out of range index %d.", Index );
		AssertMsg( 0, str );
	}

	if( !ButtonPictures[ Index ].vobj )
	{
		#ifdef BUTTONSYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return;
		AssertMsg( 0, "Attempting to UnloadButtonImage that has a null vobj (already deleted).");
	}

	// If this is a duplicated button image, then don't trash the vobject
	if(ButtonPictures[Index].fFlags & GUI_BTN_DUPLICATE_VOBJ)
	{
		ButtonPictures[Index].vobj = NULL;
		ButtonPicsLoaded--;
	}
	else
	{
		// Deleting a non-duplicate, so see if any dups present. if so, then
		// convert one of them to an original!

		fDone = FALSE;
		for( x = 0; x < MAX_BUTTON_PICS && !fDone; x++ )
		{
			if( (x != Index) && (ButtonPictures[x].vobj == ButtonPictures[Index].vobj) )
			{
				if ( ButtonPictures[x].fFlags & GUI_BTN_DUPLICATE_VOBJ )
				{
					// If we got here, then we got a duplicate object of the one we
					// want to delete, so convert it to an original!
					ButtonPictures[x].fFlags &= (~GUI_BTN_DUPLICATE_VOBJ);

					// Now remove this button, but not it's vobject
					ButtonPictures[Index].vobj = NULL;

					fDone = TRUE;
					ButtonPicsLoaded--;
				}
			}
		}
	}

	// If image slot isn't empty, delete the image
	if(ButtonPictures[Index].vobj != NULL)
	{
		DeleteVideoObject(ButtonPictures[Index].vobj);
		ButtonPictures[Index].vobj = NULL;
		ButtonPicsLoaded--;
	}
}


static GUI_BUTTON* GetButton(INT32 BtnID)
{
	AssertMsg(0 <= BtnID && BtnID < MAX_BUTTONS, String("ButtonID %d is out of range.", BtnID));
	GUI_BUTTON* b = ButtonList[BtnID];
	AssertMsg(b != NULL, String("Accessing non-existent button %d.", BtnID));
	return b;
}


// Enables an already created button.
void EnableButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->uiFlags |= BUTTON_ENABLED | BUTTON_DIRTY;
}


// Disables a button. The button remains in the system list, and can be
// reactivated by calling EnableButton.
// Diabled buttons will appear "grayed out" on the screen (unless the
// graphics for such are not available).
void DisableButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->uiFlags &= ~BUTTON_ENABLED;
	b->uiFlags |= BUTTON_DIRTY;
}


// Initializes the button image sub-system. This function is called by
// InitButtonSystem.
static BOOLEAN InitializeButtonImageManager(void)
{
	UINT8 Pix;
	int x;

	// Blank out all QuickButton images
	for(x=0;x<MAX_BUTTON_PICS;x++)
	{
		ButtonPictures[x].vobj=NULL;
		ButtonPictures[x].Grayed = -1;
		ButtonPictures[x].OffNormal = -1;
		ButtonPictures[x].OffHilite = -1;
		ButtonPictures[x].OnNormal = -1;
		ButtonPictures[x].OnHilite = -1;
	}
	ButtonPicsLoaded = 0;

	// Blank out all Generic button data
	for(x=0;x<MAX_GENERIC_PICS;x++)
	{
		GenericButtonOffNormal[x]=NULL;
		GenericButtonOffHilite[x]=NULL;
		GenericButtonOnNormal[x]=NULL;
		GenericButtonOnHilite[x]=NULL;
		GenericButtonFillColors[x]=0;
	}

	// Blank out all icon images
	for(x=0;x<MAX_BUTTON_ICONS;x++)
		GenericButtonIcons[x]=NULL;

	// Load the default generic button images
	GenericButtonOffNormal[0] = CreateVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_OFF);
	if (GenericButtonOffNormal[0] == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Couldn't create VOBJECT for "DEFAULT_GENERIC_BUTTON_OFF);
		return(FALSE);
	}

	GenericButtonOnNormal[0] = CreateVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_ON);
	if (GenericButtonOnNormal[0] == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Couldn't create VOBJECT for "DEFAULT_GENERIC_BUTTON_ON);
		return(FALSE);
	}

	// Load up the off hilite and on hilite images. We won't check for errors because if the file
	// doesn't exists, the system simply ignores that file. These are only here as extra images, they
	// aren't required for operation (only OFF Normal and ON Normal are required).
	GenericButtonOffHilite[0] = CreateVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_OFF_HI);
	GenericButtonOnHilite[0]  = CreateVideoObjectFromFile(DEFAULT_GENERIC_BUTTON_ON_HI);

	Pix=0;
	if(!GetETRLEPixelValue(&Pix,GenericButtonOffNormal[0],8,0,0))
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Couldn't get generic button's background pixel value");
		return(FALSE);
	}

	GenericButtonFillColors[0] = GenericButtonOffNormal[0]->p16BPPPalette[Pix];

	return(TRUE);
}


// Finds the next available slot for button icon images.
static INT16 FindFreeIconSlot(void)
{
	INT16 slot,x;

	slot=BUTTON_NO_SLOT;
	for(x=0;x<MAX_BUTTON_ICONS && slot<0;x++)
	{
		if(GenericButtonIcons[x]==NULL)
			slot=x;
	}

	return(slot);
}


// Loads an image file for use as a button icon.
INT16 LoadGenericButtonIcon(const char* filename)
{
	INT16 ImgSlot;

	AssertMsg(filename != NULL, "Attempting to LoadGenericButtonIcon() with null filename.");

	// Get slot for icon image
	if((ImgSlot=FindFreeIconSlot()) == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "LoadGenericButtonIcon: Out of generic button icon slots");
		return(-1);
	}

	// Load the icon
	GenericButtonIcons[ImgSlot] = CreateVideoObjectFromFile(filename);
	if (GenericButtonIcons[ImgSlot] == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, String("LoadGenericButtonIcon: Couldn't create VOBJECT for %s",filename));
		return(-1);
	}

	// Return the slot number
	return(ImgSlot);
}


// Removes a button icon graphic from the system
BOOLEAN UnloadGenericButtonIcon(INT16 GenImg)
{
	if( GenImg < 0 || GenImg >= MAX_BUTTON_ICONS  )
	{
		sprintf( str, "Attempting to UnloadGenericButtonIcon with out of range index %d.", GenImg );
		AssertMsg( 0, str );
	}

	if( !GenericButtonIcons[ GenImg ] )
	{
		#ifdef BUTTONSYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return FALSE;
		AssertMsg( 0, "Attempting to UnloadGenericButtonIcon that has no icon (already deleted)." );
	}
	// If an icon is present in the slot, remove it.
	DeleteVideoObject(GenericButtonIcons[GenImg]);
	GenericButtonIcons[GenImg]=NULL;
	return TRUE;
}


// Cleans up, and shuts down the button image manager sub-system.
static void ShutdownButtonImageManager(void)
{
	int x;

	#ifdef BUTTONSYSTEM_DEBUGGING
	gfIgnoreShutdownAssertions = TRUE;
	#endif

	// Remove all QuickButton images
	for(x=0;x<MAX_BUTTON_PICS;x++)
		UnloadButtonImage(x);

	// Remove all GenericButton images
	for(x=0;x<MAX_GENERIC_PICS;x++)
	{
		if(GenericButtonOffNormal[x]!=NULL)
		{
			DeleteVideoObject(GenericButtonOffNormal[x]);
			GenericButtonOffNormal[x]=NULL;
		}

		if(GenericButtonOffHilite[x]!=NULL)
		{
			DeleteVideoObject(GenericButtonOffHilite[x]);
			GenericButtonOffHilite[x]=NULL;
		}

		if(GenericButtonOnNormal[x]!=NULL)
		{
			DeleteVideoObject(GenericButtonOnNormal[x]);
			GenericButtonOnNormal[x]=NULL;
		}

		if(GenericButtonOnHilite[x]!=NULL)
		{
			DeleteVideoObject(GenericButtonOnHilite[x]);
			GenericButtonOnHilite[x]=NULL;
		}

		GenericButtonFillColors[x]=0;
	}

	// Remove all button icons
	for(x=0;x<MAX_BUTTON_ICONS;x++)
	{
		if(GenericButtonIcons[x]!=NULL)
			GenericButtonIcons[x]=NULL;
	}
}


// Initializes the GUI button system for use. Must be called before using
// any other button functions.
BOOLEAN InitButtonSystem(void)
{
	INT32 x;

	#ifdef BUTTONSYSTEM_DEBUGGING
	gfIgnoreShutdownAssertions = FALSE;
	#endif

	// Clear out button list
	for(x=0;x<MAX_BUTTONS;x++)
	{
		ButtonList[x]=NULL;
	}

	// Initialize the button image manager sub-system
	if (!InitializeButtonImageManager())
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0,"Failed button image manager init\n");
		return(FALSE);
	}

	return(TRUE);
}


// Shuts down and cleans up the GUI button system. Must be called before
// exiting the program. Button functions should not be used after calling
// this function.
void ShutdownButtonSystem(void)
{
	int x;

	// Kill off all buttons in the system
	for(x=0;x<MAX_BUTTONS;x++)
	{
		if(ButtonList[x]!=NULL)
			RemoveButton(x);
	}
	ShutdownButtonImageManager();
}


static void RemoveButtonsMarkedForDeletion(void)
{
	INT32 i;
	for( i = 0; i < MAX_BUTTONS; i++ )
	{
		if( ButtonList[ i ] && ButtonList[ i ]->uiFlags & BUTTON_DELETION_PENDING )
		{
			RemoveButton( i );
		}
	}
}


// Removes a button from the system's list. All memory associated with the
// button is released.
void RemoveButton(INT32 iButtonID)
{
	GUI_BUTTON* b = GetButton(iButtonID);

	//If we happen to be in the middle of a callback, and attempt to delete a button,
	//like deleting a node during list processing, then we delay it till after the callback
	//is completed.
	if( gfDelayButtonDeletion )
	{
		b->uiFlags |= BUTTON_DELETION_PENDING;
		gfPendingButtonDeletion = TRUE;
		return;
	}

	//Kris:
	if( b->uiFlags & BUTTON_SELFDELETE_IMAGE )
	{ //checkboxes and simple create buttons have their own graphics associated with them,
		//and it is handled internally.  We delete it here.  This provides the advantage of less
		//micromanagement, but with the disadvantage of wasting more memory if you have lots of
		//buttons using the same graphics.
		UnloadButtonImage( b->ImageNum );
	}

	// ...kill it!!!
	MSYS_RemoveRegion(&b->Area);

	// Get rid of the text string
	if (b->string != NULL)
		MemFree( b->string );

	if( b == gpAnchoredButton )
		gpAnchoredButton = NULL;
	if( b == gpPrevAnchoredButton )
		gpPrevAnchoredButton = NULL;

	MemFree(b);
	ButtonList[ iButtonID ] = NULL;
}


// Finds the next available button slot.
static INT32 GetNextButtonNumber(void)
{
	INT32 x;

	for(x=0;x<MAX_BUTTONS;x++)
	{
		if(ButtonList[x] == NULL)
			return(x);
	}

	return(BUTTON_NO_SLOT);
}


static void QuickButtonCallbackMButn(MOUSE_REGION* reg, INT32 reason);
static void QuickButtonCallbackMMove(MOUSE_REGION* reg, INT32 reason);


static GUI_BUTTON* AllocateButton(UINT32 ImageNum, UINT32 Flags, INT16 Left, INT16 Top, INT16 Width, INT16 Height, INT8 Priority, GUI_CALLBACK Click, GUI_CALLBACK Move)
{
	if (Left < 0 || Top < 0 || Width < 0 || Height < 0)
	{
		AssertMsg(0, String("Attempting to create button with invalid coordinates %dx%d+%dx%d", Left, Top, Width, Height));
	}

	INT32 BtnID = GetNextButtonNumber();
	if (BtnID == BUTTON_NO_SLOT)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0, "No more button slots");
		return NULL;
	}

	GUI_BUTTON* b = MemAlloc(sizeof(*b));
	if (b == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0, "Cannor allocte memory for button struct");
		return NULL;
	}

	ButtonList[BtnID] = b;

	b->IDNum                   = BtnID;
	b->ImageNum                = ImageNum;
	b->ClickCallback           = Click;
	b->MoveCallback            = Move;
	b->uiFlags                 = BUTTON_DIRTY | BUTTON_ENABLED | Flags;
	b->uiOldFlags              = 0;
	b->XLoc                    = Left;
	b->YLoc                    = Top;
	for (UINT32 i = 0; i < lengthof(b->UserData); i++) b->UserData[i] = 0;
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
	b->iIconID                 = -1;
	b->usIconIndex             = -1;
	b->bIconXOffset            = -1;
	b->bIconYOffset            = -1;
	b->fShiftImage             = TRUE;
	b->ubToggleButtonOldState  = 0;
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

	SpecifyButtonSoundScheme(BtnID, BUTTON_SOUND_SCHEME_GENERIC);

	return b;
}


static void CopyButtonText(GUI_BUTTON* b, const wchar_t* text)
{
	if (text == NULL || text[0] == L'\0') return;

	wchar_t* Buf = MemAlloc((wcslen(text) + 1) * sizeof(*Buf));
	AssertMsg(Buf != NULL, "Out of memory error:  Couldn't allocate string in CreateTextButton.");
	wcscpy(Buf, text);
	b->string = Buf;
}


// Creates an Iconic type button.
INT32 CreateIconButton(INT16 Icon, INT16 IconIndex, INT16 GenImg, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT32 Type, INT16 Priority, GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback)
{
	if (GenImg < -1 || GenImg >= MAX_GENERIC_PICS)
	{
		sprintf(str, "Attempting to %s with out of range iconID %d.", __func__, GenImg);
		AssertMsg(0, str);
	}

	// if button size is too small, adjust it.
	if (w < 4) w = 4;
	if (h < 3) h = 3;

	GUI_BUTTON* b = AllocateButton(GenImg < 0 ? 0 : GenImg, (Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE)) | BUTTON_GENERIC, xloc, yloc, w, h, Priority, ClickCallback, MoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	b->iIconID     = Icon;
	b->usIconIndex = IconIndex;

	return b->IDNum;
}


// Creates a generic button with text on it.
INT32 CreateTextButton(const wchar_t *string, UINT32 uiFont, INT16 sForeColor, INT16 sShadowColor, INT16 GenImg, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT32 Type, INT16 Priority, GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback)
{
	if (GenImg < -1 || GenImg >= MAX_GENERIC_PICS)
	{
		sprintf(str, "Attempting to %s with out of range iconID %d.", __func__, GenImg);
		AssertMsg(0, str);
	}

	// if button size is too small, adjust it.
	if (w < 4) w = 4;
	if (h < 3) h = 3;

	GUI_BUTTON* b = AllocateButton(GenImg < 0 ? 0 : GenImg, (Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE)) | BUTTON_GENERIC, xloc, yloc, w, h, Priority, ClickCallback, MoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	CopyButtonText(b, string);
	b->usFont       = uiFont;
	b->sForeColor   = sForeColor;
	b->sShadowColor = sShadowColor;

	return b->IDNum;
}


// Creates a button like HotSpot. HotSpots have no graphics associated with
// them.
INT32 CreateHotSpot(INT16 xloc, INT16 yloc, INT16 Width, INT16 Height,INT16 Priority,GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback)
{
	GUI_BUTTON* b = AllocateButton(0xFFFFFFFF, BUTTON_HOT_SPOT, xloc, yloc, Width, Height, Priority, ClickCallback, MoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	return b->IDNum;
}


// Addition Oct15/97, Carter
// will simply set the cursor for the mouse region the button occupies
BOOLEAN SetButtonCursor(INT32 iBtnId, UINT16 crsr)
{
  GUI_BUTTON *b;
  b = ButtonList[iBtnId];
	if (!b)
		return FALSE;
  b->Area.Cursor = crsr;
	return TRUE;
}


// Creates a QuickButton. QuickButtons only have graphics associated with
// them. They cannot be re-sized, nor can the graphic be changed.
INT32 QuickCreateButton(UINT32 Image,INT16 xloc,INT16 yloc,INT32 Type,INT16 Priority,GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback)
{
	if( Image < 0 || Image >= MAX_BUTTON_PICS )
	{
		sprintf( str, "Attempting to QuickCreateButton with out of range ImageID %d.", Image );
		AssertMsg( 0, str );
	}

	// Is there a QuickButton image in the given image slot?
	const BUTTON_PICS* BtnPic = &ButtonPictures[Image];
	if (BtnPic->vobj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0,"QuickCreateButton: Invalid button image number");
		return(-1);
	}

	GUI_BUTTON* b = AllocateButton(Image, (Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE)) | BUTTON_QUICK, xloc, yloc, BtnPic->MaxWidth, BtnPic->MaxHeight, Priority, ClickCallback, MoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	return b->IDNum;
}


//A hybrid of QuickCreateButton.  Takes a lot less parameters, but makes more assumptions.  It self manages the
//loading, and deleting of the image.  The size of the image determines the size of the button.  It also uses
//the default move callback which emulates Win95.  Finally, it sets the priority to normal.  The function you
//choose also determines the type of button (toggle, notoggle, or newtoggle)
INT32 CreateEasyNoToggleButton(INT32 x, INT32 y, const char* filename, GUI_CALLBACK ClickCallback)
{
	return CreateSimpleButton( x, y, filename, BUTTON_NO_TOGGLE, MSYS_PRIORITY_NORMAL, ClickCallback );
}


INT32 CreateEasyToggleButton(INT32 x, INT32 y, const char* filename, GUI_CALLBACK ClickCallback)
{
	return CreateSimpleButton( x, y, filename, BUTTON_TOGGLE, MSYS_PRIORITY_NORMAL, ClickCallback );
}


//Same as above, but accepts priority specification.
INT32 CreateSimpleButton(INT32 x, INT32 y, const char* filename, INT32 Type, INT16 Priority, GUI_CALLBACK ClickCallback)
{
	INT32 ButPic,ButNum;

	AssertMsg(filename != NULL && strlen(filename) != 0, "Attempting to CreateSimpleButton with null filename.");

	if( ( ButPic = LoadButtonImage( filename, -1, 1, 2, 3, 4 ) ) == -1)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0,"Can't load button image");
		return( -1 );
	}

	ButNum = (INT16)QuickCreateButton( ButPic, (INT16)x,(INT16)y, Type, Priority, DEFAULT_MOVE_CALLBACK, ClickCallback);

	AssertMsg( ButNum != -1, "Failed to CreateSimpleButton." );

	ButtonList[ ButNum ]->uiFlags |= BUTTON_SELFDELETE_IMAGE;

	SpecifyDisabledButtonStyle( ButNum, DISABLED_STYLE_SHADED );

	return( ButNum );
}


INT32 CreateIconAndTextButton( INT32 Image, const wchar_t *string, UINT32 uiFont,
															 INT16 sForeColor, INT16 sShadowColor,
															 INT16 sForeColorDown, INT16 sShadowColorDown,
															 INT8 bJustification,
															 INT16 xloc, INT16 yloc, INT32 Type, INT16 Priority,
															 GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback)
{
	if( Image < 0 || Image >= MAX_BUTTON_PICS )
	{
		sprintf( str, "Attemting to CreateIconAndTextButton with out of range ImageID %d.", Image );
		AssertMsg( 0, str );
	}

	// Is there a QuickButton image in the given image slot?
	const BUTTON_PICS* BtnPic = &ButtonPictures[Image];
	if (BtnPic->vobj == NULL)
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0,"QuickCreateButton: Invalid button image number");
		return(-1);
	}

	GUI_BUTTON* b = AllocateButton(Image, (Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE)) | BUTTON_QUICK, xloc, yloc, BtnPic->MaxWidth, BtnPic->MaxHeight, Priority, ClickCallback, MoveCallback);
	if (b == NULL) return BUTTON_NO_SLOT;

	CopyButtonText(b, string);
	b->bJustification   = bJustification;
	b->usFont           = uiFont;
	b->sForeColor       = sForeColor;
	b->sShadowColor     = sShadowColor;
	b->sForeColorDown   = sForeColorDown;
	b->sShadowColorDown = sShadowColorDown;

	return b->IDNum;
}


//New functions
void SpecifyButtonText( INT32 iButtonID, const wchar_t *string )
{
	GUI_BUTTON* b = GetButton(iButtonID);

	//free the previous strings memory if applicable
	if( b->string )
		MemFree( b->string );
	b->string = NULL;

	CopyButtonText(b, string);
	b->uiFlags |= BUTTON_DIRTY;
}


void SpecifyButtonFont( INT32 iButtonID, UINT32 uiFont )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->usFont = (UINT16)uiFont;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyButtonUpTextColors( INT32 iButtonID, INT16 sForeColor, INT16 sShadowColor )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->sForeColor = sForeColor;
	b->sShadowColor = sShadowColor;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyButtonDownTextColors( INT32 iButtonID, INT16 sForeColorDown, INT16 sShadowColorDown )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->sForeColorDown = sForeColorDown;
	b->sShadowColorDown = sShadowColorDown;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyButtonHilitedTextColors( INT32 iButtonID, INT16 sForeColorHilited, INT16 sShadowColorHilited )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->sForeColorHilited = sForeColorHilited;
	b->sShadowColorHilited = sShadowColorHilited;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyButtonTextJustification( INT32 iButtonID, INT8 bJustification )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	//Range check:  if invalid, then set it to center justified.
	if( bJustification < BUTTON_TEXT_LEFT || bJustification > BUTTON_TEXT_RIGHT )
		bJustification = BUTTON_TEXT_CENTER;
	b->bJustification = bJustification;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyFullButtonTextAttributes( INT32 iButtonID, const wchar_t *string, INT32 uiFont,
																			INT16 sForeColor, INT16 sShadowColor,
																			INT16 sForeColorDown, INT16 sShadowColorDown, INT8 bJustification )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	//Copy over information
	SpecifyButtonText( iButtonID, string );
	b->usFont = (UINT16)uiFont;
	b->sForeColor = sForeColor;
	b->sShadowColor = sShadowColor;
	b->sForeColorDown = sForeColorDown;
	b->sShadowColorDown = sShadowColorDown;
	//Range check:  if invalid, then set it to center justified.
	if( bJustification < BUTTON_TEXT_LEFT || bJustification > BUTTON_TEXT_RIGHT )
		bJustification = BUTTON_TEXT_CENTER;
	b->bJustification = bJustification;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyGeneralButtonTextAttributes( INT32 iButtonID, const wchar_t *string, INT32 uiFont,
																			INT16 sForeColor, INT16 sShadowColor )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	//Copy over information
	SpecifyButtonText( iButtonID, string );
	b->usFont = (UINT16)uiFont;
	b->sForeColor = sForeColor;
	b->sShadowColor = sShadowColor;
	b->uiFlags |= BUTTON_DIRTY ;
}


void SpecifyButtonTextOffsets( INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	//Copy over information
	b->bTextXOffset = bTextXOffset;
	b->bTextYOffset = bTextYOffset;
	b->fShiftText = fShiftText;
}


void SpecifyButtonTextSubOffsets( INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	//Copy over information
	b->bTextXSubOffSet = bTextXOffset;
	b->bTextYSubOffSet = bTextYOffset;
	b->fShiftText = fShiftText;
}


void SpecifyButtonTextWrappedWidth(INT32 iButtonID, INT16 sWrappedWidth)
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->sWrappedWidth = sWrappedWidth;
}


void SpecifyDisabledButtonStyle( INT32 iButtonID, INT8 bStyle )
{
	GUI_BUTTON* b = GetButton(iButtonID);

	Assert( bStyle >= DISABLED_STYLE_NONE && bStyle <= DISABLED_STYLE_SHADED );

	b->bDisabledStyle = bStyle;
}


//Note:  Text is always on top
//If fShiftImage is true, then the image will shift down one pixel and right one pixel
//just like the text does.
BOOLEAN SpecifyButtonIcon( INT32 iButtonID, INT32 iVideoObjectID, UINT16 usVideoObjectIndex,
													 INT8 bXOffset, INT8 bYOffset, BOOLEAN fShiftImage )
{
	GUI_BUTTON* b = GetButton(iButtonID);

	b->iIconID = iVideoObjectID;
	b->usIconIndex = usVideoObjectIndex;

	if( b->iIconID == -1 )
		return FALSE;

	b->bIconXOffset = bXOffset;
	b->bIconYOffset = bYOffset;
	b->fShiftImage = TRUE;

	b->uiFlags |= BUTTON_DIRTY;

	return TRUE;
}


void AllowDisabledButtonFastHelp( INT32 iButtonID, BOOLEAN fAllow )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	b->Area.uiFlags |= MSYS_ALLOW_DISABLED_FASTHELP;
}


// Set the text that will be displayed as the FastHelp
void SetButtonFastHelpText(INT32 iButton, const wchar_t *Text)
{
	GUI_BUTTON* b = GetButton(iButton);
	SetRegionFastHelpText( &b->Area, Text );
}


// Dispatches all button callbacks for mouse movement. This function gets
// called by the Mouse System. *DO NOT CALL DIRECTLY*
static void QuickButtonCallbackMMove(MOUSE_REGION *reg, INT32 reason)
{
	Assert(reg != NULL);

	INT32 iButtonID = MSYS_GetRegionUserData(reg, 0);
	GUI_BUTTON* b = GetButton(iButtonID);

	if( b->uiFlags & BUTTON_ENABLED &&
		  reason & (MSYS_CALLBACK_REASON_LOST_MOUSE | MSYS_CALLBACK_REASON_GAIN_MOUSE) )
	{
		b->uiFlags |= BUTTON_DIRTY;
	}

	// Mouse moved on the button, so reset it's timer to maximum.
	if( b->Area.uiFlags & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		//check for sound playing stuff
		if( b->ubSoundSchemeID )
		{
			if( &b->Area == MSYS_PrevRegion && !gpAnchoredButton )
			{
				if( b->uiFlags & BUTTON_ENABLED )
				{
					PlayButtonSound( iButtonID, BUTTON_SOUND_MOVED_ONTO );
				}
				else
				{
					PlayButtonSound( iButtonID, BUTTON_SOUND_DISABLED_MOVED_ONTO );
				}
			}
		}
	}
	else
	{
		//Check if we should play a sound
		if( b->ubSoundSchemeID )
		{
			if( b->uiFlags & BUTTON_ENABLED )
			{
				if( &b->Area == MSYS_PrevRegion && !gpAnchoredButton )
				{
					PlayButtonSound( iButtonID, BUTTON_SOUND_MOVED_OFF_OF );
				}
			}
			else
			{
				PlayButtonSound( iButtonID, BUTTON_SOUND_DISABLED_MOVED_OFF_OF );
			}
		}
	}

	// ATE: New stuff for toggle buttons that work with new Win95 paridigm
	if ( ( b->uiFlags & BUTTON_NEWTOGGLE ) )
	{
		if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
		{
			if ( b->ubToggleButtonActivated )
			{
				b->ubToggleButtonActivated = FALSE;

				if ( !b->ubToggleButtonOldState )
				{
					b->uiFlags &= (~BUTTON_CLICKED_ON );
				}
				else
				{
					b->uiFlags |= BUTTON_CLICKED_ON;
				}
			}
		}
	}

	// If this button is enabled and there is a callback function associated with it,
	// call the callback function.
	if (b->uiFlags & BUTTON_ENABLED && b->MoveCallback != BUTTON_NO_CALLBACK)
		b->MoveCallback(b, reason);
}


// Dispatches all button callbacks for button presses. This function is
// called by the Mouse System. *DO NOT CALL DIRECTLY*
static void QuickButtonCallbackMButn(MOUSE_REGION *reg, INT32 reason)
{
	BOOLEAN MouseBtnDown;
	BOOLEAN StateBefore,StateAfter;


	Assert(reg != NULL);

	INT32 iButtonID = MSYS_GetRegionUserData(reg, 0);
	GUI_BUTTON* b = GetButton(iButtonID);

	if( reason & (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_RBUTTON_DWN) )
		MouseBtnDown = TRUE;
	else
		MouseBtnDown = FALSE;

	StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? (TRUE) : (FALSE);

	// ATE: New stuff for toggle buttons that work with new Win95 paridigm
	if( b->uiFlags & BUTTON_NEWTOGGLE && b->uiFlags & BUTTON_ENABLED )
	{
		if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
		{
			if ( !b->ubToggleButtonActivated )
			{
				if ( !(b->uiFlags & BUTTON_CLICKED_ON ) )
				{
					b->ubToggleButtonOldState = FALSE;
					b->uiFlags |= BUTTON_CLICKED_ON;
				}
				else
				{
					b->ubToggleButtonOldState = TRUE;
					b->uiFlags &= (~BUTTON_CLICKED_ON);
				}
				b->ubToggleButtonActivated = TRUE;
			}
		}
		else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			b->ubToggleButtonActivated = FALSE;
		}
	}


	//Kris:
	//Set the anchored button incase the user moves mouse off region while still holding
	//down the button, but only if the button is up.  In Win95, buttons that are already
	//down, and anchored never change state, unless you release the mouse in the button area.

	if( b->MoveCallback == DEFAULT_MOVE_CALLBACK && b->uiFlags & BUTTON_ENABLED )
	{
		if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
		{
			gpAnchoredButton =  b;
			gfAnchoredState = StateBefore;
			b->uiFlags |= BUTTON_CLICKED_ON;
		}
		else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			b->uiFlags &= (~BUTTON_CLICKED_ON);
		}
	}
	else if( b->uiFlags & BUTTON_CHECKBOX )
	{
		if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
		{	//the check box button gets anchored, though it doesn't actually use the anchoring move callback.
			//The effect is different, we don't want to toggle the button state, but we do want to anchor this
			//button so that we don't effect any other buttons while we move the mouse around in anchor mode.
			gpAnchoredButton = b;
			gfAnchoredState = StateBefore;

			//Trick the before state of the button to be different so the sound will play properly as checkbox buttons
			//are processed differently.
			StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? FALSE : TRUE;
			StateAfter = !StateBefore;
		}
		else if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			b->uiFlags ^= BUTTON_CLICKED_ON; //toggle the checkbox state upon release inside button area.
			//Trick the before state of the button to be different so the sound will play properly as checkbox buttons
			//are processed differently.
			StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? FALSE : TRUE;
			StateAfter = !StateBefore;
		}
	}

	// Should we play a sound if clicked on while disabled?
	if( b->ubSoundSchemeID && !(b->uiFlags & BUTTON_ENABLED) && MouseBtnDown )
	{
		PlayButtonSound( iButtonID, BUTTON_SOUND_DISABLED_CLICK );
	}

	// If this button is disabled, and no callbacks allowed when disabled
	// callback
	if (!(b->uiFlags & BUTTON_ENABLED)) return;

	// If there is a callback function with this button, call it
	if (b->ClickCallback != BUTTON_NO_CALLBACK)
	{
		//Kris:  January 6, 1998
		//Added these checks to avoid a case where it was possible to process a leftbuttonup message when
		//the button wasn't anchored, and should have been.
		gfDelayButtonDeletion = TRUE;
		if( !(reason & MSYS_CALLBACK_REASON_LBUTTON_UP) || b->MoveCallback != DEFAULT_MOVE_CALLBACK ||
				b->MoveCallback == DEFAULT_MOVE_CALLBACK && gpPrevAnchoredButton == b )
			b->ClickCallback(b, reason);
		gfDelayButtonDeletion = FALSE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// Otherwise, do default action with this button.
		b->uiFlags^=BUTTON_CLICKED_ON;
	}

	if( b->uiFlags & BUTTON_CHECKBOX )
	{
		StateAfter = (b->uiFlags & BUTTON_CLICKED_ON) ? (TRUE) : (FALSE);
	}

	// Play sounds for this enabled button (disabled sounds have already been done)
	if( b->ubSoundSchemeID && b->uiFlags & BUTTON_ENABLED )
	{
		if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			if( b->ubSoundSchemeID && StateBefore && !StateAfter )
			{
				PlayButtonSound( iButtonID, BUTTON_SOUND_CLICKED_OFF );
			}
		}
		else if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
		{
			if( b->ubSoundSchemeID && !StateBefore && StateAfter)
			{
				PlayButtonSound( iButtonID, BUTTON_SOUND_CLICKED_ON );
			}
		}
	}

	if( StateBefore != StateAfter )
	{
#ifdef JA2
		InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
#endif
	}

	if( gfPendingButtonDeletion )
	{
		RemoveButtonsMarkedForDeletion();
	}
}


static void DrawButtonFromPtr(GUI_BUTTON *b);


void RenderButtons(void)
{
	INT32			iButtonID;
	BOOLEAN		fOldButtonDown, fOldEnabled;
	GUI_BUTTON *b;

	SaveFontSettings();
	for(iButtonID=0;iButtonID<MAX_BUTTONS;iButtonID++)
	{
		// If the button exists, and it's not owned by another object, draw it
		//Kris:  and make sure that the button isn't hidden.
		b = ButtonList[iButtonID];
		if( b && b->Area.uiFlags & MSYS_REGION_ENABLED )
		{
			// Check for buttonchanged status
			fOldButtonDown = (BOOLEAN)(b->uiFlags & BUTTON_CLICKED_ON);

			if ( fOldButtonDown != ( b->uiOldFlags & BUTTON_CLICKED_ON ) )
			{
				//Something is different, set dirty!
				b->uiFlags |= BUTTON_DIRTY;
			}

			// Check for button dirty flags
			fOldEnabled = (BOOLEAN)(b->uiFlags & BUTTON_ENABLED);

			if ( fOldEnabled != ( b->uiOldFlags & BUTTON_ENABLED ) )
			{
				//Something is different, set dirty!
				b->uiFlags |= BUTTON_DIRTY;
			}

			// Set old flags
			b->uiOldFlags = b->uiFlags;

			if ( b->uiFlags & BUTTON_FORCE_UNDIRTY )
			{
				b->uiFlags &= ~( BUTTON_DIRTY );
				b->uiFlags &= ~( BUTTON_FORCE_UNDIRTY );
			}

			// Check if we need to update!
			if ( b->uiFlags & BUTTON_DIRTY )
			{
				// Turn off dirty flag
				b->uiFlags &= (~BUTTON_DIRTY);
				DrawButtonFromPtr(b);

#ifdef JA2
				InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
//#else
//				InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY, FALSE);
#endif

			}
		}
	}

	RestoreFontSettings();
}


void MarkAButtonDirty( INT32 iButtonNum )
{
  // surgical dirtying -> marks a user specified button dirty, without dirty the whole lot of them

  // If the button exists, and it's not owned by another object, draw it
		if( ButtonList[ iButtonNum ] )
		{
			// Turn on dirty flag
			ButtonList[ iButtonNum ]->uiFlags |= BUTTON_DIRTY;
		}
}


void MarkButtonsDirty( void )
{
	INT32 x;
	for(x=0;x<MAX_BUTTONS;x++)
	{
		// If the button exists, and it's not owned by another object, draw it
		if( ButtonList[x] )
		{
			// Turn on dirty flag
			ButtonList[x]->uiFlags |= BUTTON_DIRTY;
		}
	}
}


void UnMarkButtonDirty( INT32 iButtonIndex )
{
  if ( ButtonList[ iButtonIndex ] )
  {
	  ButtonList[ iButtonIndex ]->uiFlags &= ~( BUTTON_DIRTY );
  }
}


void UnmarkButtonsDirty( void )
{
	INT32 x;
	for(x=0;x<MAX_BUTTONS;x++)
	{
		// If the button exists, and it's not owned by another object, draw it
		if( ButtonList[x] )
		{
			UnMarkButtonDirty( x );
		}
	}
}


void ForceButtonUnDirty( INT32 iButtonIndex )
{
	GUI_BUTTON* b = GetButton(iButtonIndex);
	b->uiFlags &= ~BUTTON_DIRTY;
	b->uiFlags |= BUTTON_FORCE_UNDIRTY;
}


// Draws a single button on the screen.
BOOLEAN DrawButton(INT32 iButtonID )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	if (b->string != NULL) SaveFontSettings();
	if (b->Area.uiFlags & MSYS_REGION_ENABLED) DrawButtonFromPtr(b);
	if (b->string != NULL) RestoreFontSettings();
	return TRUE;
}


static void DrawCheckBoxButton(const GUI_BUTTON *b);
static void DrawGenericButton(const GUI_BUTTON *b);
static void DrawIconOnButton(const GUI_BUTTON *b);
static void DrawQuickButton(const GUI_BUTTON *b);
static void DrawTextOnButton(const GUI_BUTTON *b);


// Given a pointer to a GUI_BUTTON structure, draws the button on the screen.
static void DrawButtonFromPtr(GUI_BUTTON *b)
{
	Assert( b );
	// Draw the appropriate button according to button type
	gbDisabledButtonStyle = DISABLED_STYLE_NONE;
	switch(b->uiFlags & BUTTON_TYPES)
	{
		case BUTTON_QUICK:
			DrawQuickButton(b);
			break;
		case BUTTON_GENERIC:
			DrawGenericButton(b);
			break;
		case BUTTON_HOT_SPOT:
			if(b->uiFlags & BUTTON_NO_TOGGLE)
				b->uiFlags &= (~BUTTON_CLICKED_ON);
			return;  //hotspots don't have text, but if you want to, change this to a break!
		case BUTTON_CHECKBOX:
			DrawCheckBoxButton(b);
			break;
	}
	//If button has an icon, overlay it on current button.
	if( b->iIconID != -1 )
		DrawIconOnButton( b );
	//If button has text, draw it now
	if( b->string )
		DrawTextOnButton( b );
	//If the button is disabled, and a style has been calculated, then
	//draw the style last.
	switch (gbDisabledButtonStyle)
	{
		case DISABLED_STYLE_HATCHED: DrawHatchOnButton(b); break;
		case DISABLED_STYLE_SHADED:  DrawShadeOnButton(b); break;
	}
}


// Draws a QuickButton type button on the screen.
static void DrawQuickButton(const GUI_BUTTON *b)
{
	INT32 UseImage;
	UseImage=0;
	// Is button Enabled, or diabled but no "Grayed" image associated with this QuickButton?
	if(b->uiFlags & BUTTON_ENABLED )
	{
		// Is the button's state ON?
		if(b->uiFlags & BUTTON_CLICKED_ON)
		{
			// Is the mouse over this area, and we have a hilite image?
			if((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && gfRenderHilights &&
					(ButtonPictures[b->ImageNum].OnHilite != -1))
				UseImage = ButtonPictures[b->ImageNum].OnHilite;			// Use On-Hilite image
			else if(ButtonPictures[b->ImageNum].OnNormal != -1)
				UseImage = ButtonPictures[b->ImageNum].OnNormal;			// Use On-Normal image
		}
		else
		{
			// Is the mouse over the button, and do we have hilite image?
			if((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && gfRenderHilights &&
					(ButtonPictures[b->ImageNum].OffHilite != -1))
				UseImage = ButtonPictures[b->ImageNum].OffHilite;			// Use Off-Hilite image
			else if(ButtonPictures[b->ImageNum].OffNormal != -1)
				UseImage = ButtonPictures[b->ImageNum].OffNormal;			// Use Off-Normal image
		}
	}
	else if( ButtonPictures[b->ImageNum].Grayed != -1)
	{	// Button is diabled so use the "Grayed-out" image
		UseImage = ButtonPictures[b->ImageNum].Grayed;
	}
	else
	{
		UseImage = ButtonPictures[b->ImageNum].OffNormal;
		switch( b->bDisabledStyle )
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

	// Display the button image
	BltVideoObject(ButtonDestBuffer, ButtonPictures[b->ImageNum].vobj, (UINT16)UseImage, b->XLoc, b->YLoc);
}


static void DrawHatchOnButton(const GUI_BUTTON *b)
{
	UINT8	 *pDestBuf;
	UINT32 uiDestPitchBYTES;
	SGPRect ClipRect;
	ClipRect.iLeft = b->Area.RegionTopLeftX;
	ClipRect.iRight = b->Area.RegionBottomRightX - 1;
	ClipRect.iTop = b->Area.RegionTopLeftY;
	ClipRect.iBottom = b->Area.RegionBottomRightY - 1;
	pDestBuf = LockVideoSurface( ButtonDestBuffer, &uiDestPitchBYTES );
	Blt16BPPBufferHatchRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect );
	UnLockVideoSurface( ButtonDestBuffer );
}


static void DrawShadeOnButton(const GUI_BUTTON *b)
{
	UINT8 *pDestBuf;
	UINT32 uiDestPitchBYTES;
	SGPRect ClipRect;
	ClipRect.iLeft = b->Area.RegionTopLeftX;
	ClipRect.iRight = b->Area.RegionBottomRightX-1;
	ClipRect.iTop = b->Area.RegionTopLeftY;
	ClipRect.iBottom = b->Area.RegionBottomRightY-1;
	pDestBuf = LockVideoSurface( ButtonDestBuffer, &uiDestPitchBYTES );
	Blt16BPPBufferShadowRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect );
	UnLockVideoSurface( ButtonDestBuffer );
}


void DrawCheckBoxButtonOn( INT32 iButtonID )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	BOOLEAN fLeftButtonState = gfLeftButtonState;

	gfLeftButtonState = TRUE;
	b->Area.uiFlags |= MSYS_MOUSE_IN_AREA;

	DrawButton( iButtonID );

	gfLeftButtonState = fLeftButtonState;
}


void DrawCheckBoxButtonOff( INT32 iButtonID )
{
	GUI_BUTTON* b = GetButton(iButtonID);
	BOOLEAN fLeftButtonState = gfLeftButtonState;

	gfLeftButtonState = FALSE;
	b->Area.uiFlags |= MSYS_MOUSE_IN_AREA;

	DrawButton( iButtonID );

	gfLeftButtonState = fLeftButtonState;
}


static void DrawCheckBoxButton(const GUI_BUTTON *b)
{
	INT32 UseImage;

	UseImage=0;
	// Is button Enabled, or diabled but no "Grayed" image associated with this QuickButton?
	if( b->uiFlags & BUTTON_ENABLED )
	{
		// Is the button's state ON?
		if(b->uiFlags & BUTTON_CLICKED_ON)
		{
			// Is the mouse over this area, and we have a hilite image?
			if( b->Area.uiFlags & MSYS_MOUSE_IN_AREA && gfRenderHilights &&
					gfLeftButtonState &&
					ButtonPictures[b->ImageNum].OnHilite != -1 )
				UseImage = ButtonPictures[b->ImageNum].OnHilite;			// Use On-Hilite image
			else if(ButtonPictures[b->ImageNum].OnNormal != -1)
				UseImage = ButtonPictures[b->ImageNum].OnNormal;			// Use On-Normal image
		}
		else
		{
			// Is the mouse over the button, and do we have hilite image?
			if( b->Area.uiFlags & MSYS_MOUSE_IN_AREA && gfRenderHilights &&
				  gfLeftButtonState &&
					ButtonPictures[b->ImageNum].OffHilite != -1 )
				UseImage = ButtonPictures[b->ImageNum].OffHilite;			// Use Off-Hilite image
			else if(ButtonPictures[b->ImageNum].OffNormal != -1)
				UseImage = ButtonPictures[b->ImageNum].OffNormal;			// Use Off-Normal image
		}
	}
	else if( ButtonPictures[b->ImageNum].Grayed != -1 )
	{	// Button is disabled so use the "Grayed-out" image
		UseImage = ButtonPictures[b->ImageNum].Grayed;
	}
	else //use the disabled style
	{
		if( b->uiFlags & BUTTON_CLICKED_ON )
			UseImage = ButtonPictures[b->ImageNum].OnHilite;
		else
			UseImage = ButtonPictures[b->ImageNum].OffHilite;
		switch( b->bDisabledStyle )
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

	// Display the button image
	BltVideoObject(ButtonDestBuffer, ButtonPictures[b->ImageNum].vobj, (UINT16)UseImage, b->XLoc, b->YLoc);
}


static void DrawIconOnButton(const GUI_BUTTON *b)
{
	INT32 xp,yp,width,height,IconX,IconY;
	INT32 IconW,IconH;
	SGPRect NewClip,OldClip;
	ETRLEObject		*pTrav;
	HVOBJECT	hvObject;

	// If there's an actual icon on this button, try to show it.
	if(b->iIconID >= 0)
	{
		// Get width and height of button area
		width = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
		height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

		// Compute viewable area (inside borders)
		NewClip.iLeft = b->XLoc + 3;
		NewClip.iRight = b->XLoc + width - 3;
		NewClip.iTop = b->YLoc + 2;
		NewClip.iBottom = b->YLoc + height - 2;

		// Get Icon's blit start coordinates
		IconX = NewClip.iLeft;
		IconY = NewClip.iTop;

		// Get current clip area
		GetClippingRect(&OldClip);

		// Clip button's viewable area coords to screen
		if(NewClip.iLeft < OldClip.iLeft)
			NewClip.iLeft = OldClip.iLeft;

		// Is button right off the right side of the screen?
		if(NewClip.iLeft > OldClip.iRight)
			return;

		if(NewClip.iRight > OldClip.iRight)
			NewClip.iRight = OldClip.iRight;

		// Is button completely off the left side of the screen?
		if(NewClip.iRight < OldClip.iLeft)
			return;

		if(NewClip.iTop < OldClip.iTop)
			NewClip.iTop = OldClip.iTop;

		// Are we right off the bottom of the screen?
		if(NewClip.iTop > OldClip.iBottom)
			return;

		if(NewClip.iBottom > OldClip.iBottom)
			NewClip.iBottom = OldClip.iBottom;

		// Are we off the top?
		if(NewClip.iBottom < OldClip.iTop)
			return;

		// Did we clip the viewable area out of existance?
		if((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop))
			return;

		// Get the width and height of the icon itself
		if( b->uiFlags & BUTTON_GENERIC )
			pTrav = &(GenericButtonIcons[b->iIconID]->pETRLEObject[b->usIconIndex]);
		else
		{
			hvObject = GetVideoObject(b->iIconID);
			pTrav = &(hvObject->pETRLEObject[b->usIconIndex] );
		}
		IconH = (UINT32)(pTrav->usHeight+pTrav->sOffsetY);
		IconW = (UINT32)(pTrav->usWidth+pTrav->sOffsetX);

		// Compute coordinates for centering the icon on the button or
		// use the offset system.
		if( b->bIconXOffset == -1 )
			xp = (((width-6)-IconW) / 2) + IconX;
		else
			xp = b->Area.RegionTopLeftX + b->bIconXOffset;
		if( b->bIconYOffset == -1 )
			yp = (((height-4)-IconH) / 2) + IconY;
		else
			yp = b->Area.RegionTopLeftY + b->bIconYOffset;

		// Was the button clicked on? if so, move the image slightly for the illusion
		// that the image moved into the screen.
		if( b->uiFlags & BUTTON_CLICKED_ON && b->fShiftImage )
		{
			xp++;
			yp++;
		}

		// Set the clipping rectangle to the viewable area of the button
		SetClippingRect(&NewClip);
		// Blit the icon
		if( b->uiFlags & BUTTON_GENERIC )
			BltVideoObject(ButtonDestBuffer,GenericButtonIcons[b->iIconID], b->usIconIndex, (INT16)xp, (INT16)yp);
		else
			BltVideoObject(ButtonDestBuffer, hvObject, b->usIconIndex, (INT16)xp, (INT16)yp);
		// Restore previous clip region
		SetClippingRect(&OldClip);
	}
}


//If a button has text attached to it, then it'll draw it last.
static void DrawTextOnButton(const GUI_BUTTON *b)
{
	INT32 xp,yp,width,height,TextX,TextY;
	SGPRect NewClip,OldClip;
	INT16	sForeColor;

	// If this button actually has a string to print
	if( b->string )
	{
		// Get the width and height of this button
		width = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
		height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

		// Compute the viewable area on this button
		NewClip.iLeft = b->XLoc + 3;
		NewClip.iRight = b->XLoc + width - 3;
		NewClip.iTop = b->YLoc + 2;
		NewClip.iBottom = b->YLoc + height - 2;

		// Get the starting coordinates to print
		TextX = NewClip.iLeft;
		TextY = NewClip.iTop;

		// Get the current clipping area
		GetClippingRect(&OldClip);

		// Clip the button's viewable area to the screen
		if(NewClip.iLeft < OldClip.iLeft)
			NewClip.iLeft = OldClip.iLeft;

		// Are we off hte right side?
		if(NewClip.iLeft > OldClip.iRight)
			return;

		if(NewClip.iRight > OldClip.iRight)
			NewClip.iRight = OldClip.iRight;

		// Are we off the left side?
		if(NewClip.iRight < OldClip.iLeft)
			return;

		if(NewClip.iTop < OldClip.iTop)
			NewClip.iTop = OldClip.iTop;

		// Are we off the bottom of the screen?
		if(NewClip.iTop > OldClip.iBottom)
			return;

		if(NewClip.iBottom > OldClip.iBottom)
			NewClip.iBottom = OldClip.iBottom;

		// Are we off the top?
		if(NewClip.iBottom < OldClip.iTop)
			return;

		// Did we clip the viewable area out of existance?
		if((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop))
			return;

		// Set the font printing settings to the buttons viewable area
		SetFontDestBuffer(ButtonDestBuffer, NewClip.iLeft,
											NewClip.iTop, NewClip.iRight,
											NewClip.iBottom, FALSE);

		// Compute the coordinates to center the text
		if( b->bTextYOffset == -1 )
			yp = (((height) - GetFontHeight(b->usFont)) / 2) + TextY - 1;
		else
			yp = b->Area.RegionTopLeftY + b->bTextYOffset;
		if( b->bTextXOffset == -1 )
		{
			switch( b->bJustification )
			{
				case BUTTON_TEXT_LEFT:
					xp = TextX + 3;
					break;
				case BUTTON_TEXT_RIGHT:
					xp = NewClip.iRight - StringPixLength(b->string, b->usFont) - 3;
					break;
				case BUTTON_TEXT_CENTER:
				default:
					xp = (((width-6)-StringPixLength(b->string,b->usFont)) / 2) + TextX;
					break;
			}
		}
		else
			xp = b->Area.RegionTopLeftX + b->bTextXOffset;

		// Set the printing font to the button text font
		SetFont(b->usFont);

		// print the text
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( (UINT8)b->sForeColor );
		sForeColor = b->sForeColor;
		if( b->sShadowColor != -1 )
			SetFontShadow( (UINT8)b->sShadowColor );
		//Override the colors if necessary.
		if( b->uiFlags & BUTTON_ENABLED && b->Area.uiFlags & MSYS_MOUSE_IN_AREA && b->sForeColorHilited != -1 )
		{
			SetFontForeground( (UINT8)b->sForeColorHilited );
			sForeColor = b->sForeColorHilited;
		}
		else if( b->uiFlags & BUTTON_CLICKED_ON && b->sForeColorDown != -1 )
		{
			SetFontForeground( (UINT8)b->sForeColorDown );
			sForeColor = b->sForeColorDown;
		}
		if( b->uiFlags & BUTTON_ENABLED && b->Area.uiFlags & MSYS_MOUSE_IN_AREA && b->sShadowColorHilited != -1 )
		{
			SetFontShadow( (UINT8)b->sShadowColorHilited );
		}
		else if( b->uiFlags & BUTTON_CLICKED_ON && b->sShadowColorDown != -1 )
		{
			SetFontShadow( (UINT8)b->sShadowColorDown );
		}
		if( b->uiFlags & BUTTON_CLICKED_ON && b->fShiftText )
		{	// Was the button clicked on? if so, move the text slightly for the illusion
			// that the text moved into the screen.
			xp++;
			yp++;
		}
#ifdef JA2
		if( b->sWrappedWidth != -1 )
		{
			UINT8 bJustified=0;
			switch( b->bJustification )
			{
				case BUTTON_TEXT_LEFT:		bJustified = LEFT_JUSTIFIED;		break;
				case BUTTON_TEXT_RIGHT:		bJustified = RIGHT_JUSTIFIED;		break;
				case BUTTON_TEXT_CENTER:	bJustified = CENTER_JUSTIFIED;	break;
				default:									Assert( 0 );										break;
			}
			if( b->bTextXOffset == -1 )
			{
				//Kris:
				//There needs to be recalculation of the start positions based on the
				//justification and the width specified wrapped width.  I was drawing a
				//double lined word on the right side of the button to find it drawing way
				//over to the left.  I've added the necessary code for the right and center
				//justification.
				yp = b->Area.RegionTopLeftY + 2;

				switch( b->bJustification )
				{
					case BUTTON_TEXT_RIGHT:
						xp = b->Area.RegionBottomRightX - 3 - b->sWrappedWidth;

						if( b->fShiftText && b->uiFlags & BUTTON_CLICKED_ON )
							xp++, yp++;
						break;
					case BUTTON_TEXT_CENTER:
						xp = b->Area.RegionTopLeftX + 3 + b->sWrappedWidth/2;

						if( b->fShiftText && b->uiFlags & BUTTON_CLICKED_ON )
							xp++, yp++;
						break;
				}
			}
			yp+= b->bTextYSubOffSet;
			xp+= b->bTextXSubOffSet;
			DisplayWrappedString((UINT16)xp, (UINT16)yp, b->sWrappedWidth, 1, b->usFont, (UINT8)sForeColor, b->string, FONT_MCOLOR_BLACK, FALSE, bJustified);
		}
		else
		{
			yp+= b->bTextYSubOffSet;
			xp+= b->bTextXSubOffSet;
			mprintf(xp, yp, b->string);
		}
#else
		mprintf(xp, yp, b->string);
#endif
		// Restore the old text printing settings
	}
}


// This function is called by the DrawIconicButton and DrawTextButton
// routines to draw the borders and background of the buttons.
static void DrawGenericButton(const GUI_BUTTON *b)
{
	INT32 NumChunksWide,NumChunksHigh,cx,cy,width,height,hremain,wremain;
	INT32 q, ImgNum;
	INT32 iBorderHeight, iBorderWidth;
	HVOBJECT BPic;
	UINT32			uiDestPitchBYTES;
	UINT8				*pDestBuf;
	SGPRect			ClipRect;
	ETRLEObject *pTrav;

	// Select the graphics to use depending on the current state of the button
	if( b->uiFlags & BUTTON_ENABLED )
	{
		if (!(b->uiFlags & BUTTON_ENABLED))
			BPic = GenericButtonOffNormal[b->ImageNum];
		else if(b->uiFlags & BUTTON_CLICKED_ON)
		{
			if((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && (GenericButtonOnHilite[b->ImageNum]!=NULL) && gfRenderHilights )
				BPic = GenericButtonOnHilite[b->ImageNum];
			else
				BPic = GenericButtonOnNormal[b->ImageNum];
		}
		else
		{
			if((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && (GenericButtonOffHilite[b->ImageNum]!=NULL) && gfRenderHilights )
				BPic = GenericButtonOffHilite[b->ImageNum];
			else
				BPic = GenericButtonOffNormal[b->ImageNum];
		}
	}
	else
	{
		BPic = GenericButtonOffNormal[ b->ImageNum ];
		switch( b->bDisabledStyle )
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

	iBorderWidth=3;
	iBorderHeight=2;
	pTrav=NULL;

// DB - Added this to support more flexible sizing of border images
// The 3x2 size was a bit limiting. JA2 should default to the original
// size, unchanged

#ifndef JA2
	pTrav = &(BPic->pETRLEObject[0] );
	iBorderHeight = (INT32)pTrav->usHeight;
	iBorderWidth = (INT32)pTrav->usWidth;
#endif

	// Compute the number of button "chunks" needed to be blitted
	width = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
	height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;
	NumChunksWide = width / iBorderWidth;
	NumChunksHigh = height / iBorderHeight;
	hremain = height % iBorderHeight;
	wremain = width % iBorderWidth;

	cx = (b->XLoc + ((NumChunksWide-1)*iBorderWidth) + wremain);
	cy = (b->YLoc + ((NumChunksHigh-1)*iBorderHeight) + hremain);

	// Fill the button's area with the button's background color
	ColorFillVideoSurfaceArea(ButtonDestBuffer,b->Area.RegionTopLeftX,
																						 b->Area.RegionTopLeftY,
																						 b->Area.RegionBottomRightX,
																						 b->Area.RegionBottomRightY,
																						 GenericButtonFillColors[b->ImageNum]);

	// Lock the dest buffer
	pDestBuf = LockVideoSurface( ButtonDestBuffer, &uiDestPitchBYTES );

	GetClippingRect(&ClipRect);

	// Draw the button's borders and corners (horizontally)
	for(q=0;q<NumChunksWide;q++)
	{
		if(q==0)
			ImgNum=0;
		else
			ImgNum=1;

		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, (INT32)(b->XLoc + q * iBorderWidth), (INT32)b->YLoc, (UINT16)ImgNum, &ClipRect);

		if(q==0)
			ImgNum=5;
		else
			ImgNum=6;

		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, (INT32)(b->XLoc + q * iBorderWidth), cy, (UINT16)ImgNum, &ClipRect);
	}
	// Blit the right side corners
	Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, cx, (INT32)b->YLoc, 2, &ClipRect);
	Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, cx, cy, 7, &ClipRect);
	// Draw the vertical members of the button's borders
	NumChunksHigh--;

	if(hremain!=0)
	{
		q=NumChunksHigh;
		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, (INT32)b->XLoc, (INT32)(b->YLoc + q * iBorderHeight - iBorderHeight + hremain), 3, &ClipRect);
		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, cx, (INT32)(b->YLoc + q * iBorderHeight - iBorderHeight + hremain), 4, &ClipRect);
	}

	for(q=1;q<NumChunksHigh;q++)
	{
		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, (INT32)b->XLoc, (INT32)(b->YLoc + q * iBorderHeight), 3, &ClipRect);
		Blt8BPPDataTo16BPPBufferTransparentClip((UINT16*)pDestBuf, uiDestPitchBYTES, BPic, cx, (INT32)(b->YLoc + q * iBorderHeight), 4, &ClipRect);
	}

	// Unlock buffer
	UnLockVideoSurface( ButtonDestBuffer );
}


INT32 CreateCheckBoxButton( INT16 x, INT16 y, const char *filename, INT16 Priority, GUI_CALLBACK ClickCallback )
{
	GUI_BUTTON *b;
	INT32 ButPic, iButtonID;
	Assert( filename != NULL );
	Assert( strlen(filename) );
	if( ( ButPic = LoadButtonImage(filename,-1,0,1,2,3) ) == -1 )
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0,"CreateCheckBoxButton: Can't load button image");
		return( -1 );
	}
	iButtonID = (INT16)QuickCreateButton(
									(UINT32)ButPic, x, y, BUTTON_CHECKBOX, Priority,
									MSYS_NO_CALLBACK, ClickCallback );
	if( iButtonID == - 1 )
	{
		DebugMsg(TOPIC_BUTTON_HANDLER,DBG_LEVEL_0,"CreateCheckBoxButton: Can't create button");
		return( -1 );
	}

	//change the flags so that it isn't a quick button anymore
	b = ButtonList[ iButtonID ];
	b->uiFlags &= ( ~BUTTON_QUICK );
	b->uiFlags |= ( BUTTON_CHECKBOX | BUTTON_SELFDELETE_IMAGE);

	return( iButtonID );
}


// Added Oct17, 97 Carter - kind of mindless, but might as well have it
void MSYS_SetBtnUserData(INT32 iButtonNum,INT32 index,INT32 userdata)
{
  GUI_BUTTON *b;
	b=ButtonList[iButtonNum];
	if(index < 0 || index > 3)
		return;
	b->UserData[index]=userdata;
}


INT32 MSYS_GetBtnUserData(GUI_BUTTON *b,INT32 index)
{
	if(index < 0 || index > 3)
		return(0);

	return(b->UserData[index]);
}


//Generic Button Movement Callback to reset the mouse button if the mouse is no longer
//in the button region.
void BtnGenericMouseMoveButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	//If the button isn't the anchored button, then we don't want to modify the button state.
	if( btn != gpAnchoredButton )
		return;
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		if( !gfAnchoredState )
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );
			if(	btn->ubSoundSchemeID )
			{
				PlayButtonSound( btn->IDNum, BUTTON_SOUND_CLICKED_OFF );
			}
		}
		#ifdef JA2
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		#endif
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON ;
		if( btn->ubSoundSchemeID )
		{
			PlayButtonSound( btn->IDNum, BUTTON_SOUND_CLICKED_ON );
		}
		#ifdef JA2
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		#endif
	}
}


void ReleaseAnchorMode()
{
  if( !gpAnchoredButton )
		return;

	if(	gusMouseXPos < gpAnchoredButton->Area.RegionTopLeftX ||
		  gusMouseXPos > gpAnchoredButton->Area.RegionBottomRightX ||
			gusMouseYPos < gpAnchoredButton->Area.RegionTopLeftY ||
			gusMouseYPos > gpAnchoredButton->Area.RegionBottomRightY )
	{
		//released outside button area, so restore previous button state.
		if( gfAnchoredState )
			gpAnchoredButton->uiFlags |= BUTTON_CLICKED_ON;
		else
			gpAnchoredButton->uiFlags &= ( ~BUTTON_CLICKED_ON );
			#ifdef JA2
			InvalidateRegion(gpAnchoredButton->Area.RegionTopLeftX, gpAnchoredButton->Area.RegionTopLeftY, gpAnchoredButton->Area.RegionBottomRightX, gpAnchoredButton->Area.RegionBottomRightY);
			#endif
	}
	gpPrevAnchoredButton = gpAnchoredButton;
	gpAnchoredButton = NULL;
}


//Kris:
//Yet new logical additions to the winbart library.
void HideButton( INT32 iButtonNum )
{
	GUI_BUTTON* b = GetButton(iButtonNum);
	b->Area.uiFlags &= (~MSYS_REGION_ENABLED);
	b->uiFlags |= BUTTON_DIRTY;
	#ifdef JA2
		InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
	#endif
}

void ShowButton( INT32 iButtonNum )
{
	GUI_BUTTON* b = GetButton(iButtonNum);
	b->Area.uiFlags |= MSYS_REGION_ENABLED;
	b->uiFlags |= BUTTON_DIRTY;
	#ifdef JA2
		InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX, b->Area.RegionBottomRightY);
	#endif
}
