//*****************************************************************************************************
//	Button System.h
//
//	by Kris Morness (originally created by Bret Rowden)
//*****************************************************************************************************

#ifndef __BUTTON_SYSTEM_H
#define __BUTTON_SYSTEM_H

#include "VObject.h"
#include "MouseSystem.h"
#include "Button_Sound_Control.h"

#ifdef __cplusplus
extern "C" {
#endif

// Moved here from Button System.c by DB 99/01/07
// Names of the default generic button image files.
#ifdef JA2
#define DEFAULT_GENERIC_BUTTON_OFF			"GENBUTN.STI"
#define DEFAULT_GENERIC_BUTTON_ON				"GENBUTN2.STI"
#define DEFAULT_GENERIC_BUTTON_OFF_HI		"GENBUTN3.STI"
#define DEFAULT_GENERIC_BUTTON_ON_HI		"GENBUTN4.STI"
#else
#define DEFAULT_GENERIC_BUTTON_OFF			"Data\\Message Box\\GENBUTN.STI"
#define DEFAULT_GENERIC_BUTTON_ON				"Data\\Message Box\\GENBUTN2.STI"
#define DEFAULT_GENERIC_BUTTON_OFF_HI		"Data\\Message Box\\GENBUTN3.STI"
#define DEFAULT_GENERIC_BUTTON_ON_HI		"Data\\Message Box\\GENBUTN4.STI"
#endif

#define BUTTON_TEXT_LEFT		-1
#define BUTTON_TEXT_CENTER	0
#define BUTTON_TEXT_RIGHT		1

#define TEXT_LJUSTIFIED		BUTTON_TEXT_LEFT
#define TEXT_CJUSTIFIED		BUTTON_TEXT_CENTER
#define TEXT_RJUSTIFIED		BUTTON_TEXT_RIGHT


// Some GUI_BUTTON system defines
#define BUTTON_USE_DEFAULT		-1
#define BUTTON_NO_FILENAME		NULL
#define BUTTON_NO_CALLBACK		NULL
#define BUTTON_NO_IMAGE				-1
#define BUTTON_NO_SLOT				-1

#define BUTTON_INIT						1
#define BUTTON_WAS_CLICKED		2

//effects how the button is rendered.
#define BUTTON_TYPES						( BUTTON_QUICK | BUTTON_GENERIC | BUTTON_HOT_SPOT | BUTTON_CHECKBOX )
//effects how the button is processed
#define BUTTON_TYPE_MASK				(BUTTON_NO_TOGGLE| BUTTON_ALLOW_DISABLED_CALLBACK | BUTTON_CHECKBOX | BUTTON_IGNORE_CLICKS )

//button flags
#define BUTTON_TOGGLE										0x00000000
#define BUTTON_QUICK										0x00000000
#define BUTTON_ENABLED									0x00000001
#define BUTTON_CLICKED_ON								0x00000002
#define BUTTON_NO_TOGGLE								0x00000004
#define BUTTON_CLICK_CALLBACK						0x00000008
#define BUTTON_MOVE_CALLBACK						0x00000010
#define BUTTON_GENERIC									0x00000020
#define BUTTON_HOT_SPOT									0x00000040
#define BUTTON_SELFDELETE_IMAGE					0x00000080
#define BUTTON_DELETION_PENDING					0x00000100
#define BUTTON_ALLOW_DISABLED_CALLBACK	0x00000200
#define BUTTON_DIRTY										0x00000400
#define BUTTON_SAVEBACKGROUND						0x00000800
#define BUTTON_CHECKBOX									0x00001000
#define BUTTON_NEWTOGGLE								0x00002000
#define BUTTON_FORCE_UNDIRTY						0x00004000 // no matter what happens this buttons does NOT get marked dirty
#define BUTTON_IGNORE_CLICKS						0x00008000 // Ignore any clicks on this button
#define BUTTON_DISABLED_CALLBACK				0x80000000

#define BUTTON_SOUND_NONE										0x00
#define BUTTON_SOUND_CLICKED_ON							0x01
#define BUTTON_SOUND_CLICKED_OFF						0x02
#define BUTTON_SOUND_MOVED_ONTO							0x04
#define BUTTON_SOUND_MOVED_OFF_OF						0x08
#define BUTTON_SOUND_DISABLED_CLICK					0x10
#define BUTTON_SOUND_DISABLED_MOVED_ONTO		0x20
#define BUTTON_SOUND_DISABLED_MOVED_OFF_OF	0x40
#define BUTTON_SOUND_ALREADY_PLAYED					0X80

#define BUTTON_SOUND_ALL_EVENTS							0xff


// Internal use!
#define	GUI_SND_CLK_ON									BUTTON_SOUND_CLICKED_ON
#define	GUI_SND_CLK_OFF									BUTTON_SOUND_CLICKED_OFF
#define	GUI_SND_MOV_ON									BUTTON_SOUND_MOVED_ONTO
#define	GUI_SND_MOV_OFF									BUTTON_SOUND_MOVED_OFF_OF
#define	GUI_SND_DCLK										BUTTON_SOUND_DISABLED_CLICK
#define	GUI_SND_DMOV										BUTTON_SOUND_DISABLED_MOVED_ONTO

extern UINT32 ButtonDestBuffer;

struct _GUI_BUTTON;

// GUI_BUTTON callback function type
typedef void (*GUI_CALLBACK)(struct _GUI_BUTTON *,INT32);

// GUI_BUTTON structure definitions.
typedef struct _GUI_BUTTON {
	INT32					IDNum;						// ID Number, contains it's own button number
	UINT32				ImageNum;					// Image number to use (see DOCs for details)
	MOUSE_REGION	Area;							// Mouse System's mouse region to use for this button
	GUI_CALLBACK	ClickCallback;		// Button Callback when button is clicked
	GUI_CALLBACK	MoveCallback;			// Button Callback when mouse moved on this region
	INT16					Cursor;						// Cursor to use for this button
	UINT32				uiFlags;					// Button state flags etc.( 32-bit )
	UINT32				uiOldFlags;				// Old flags from previous render loop
	INT16					XLoc;							// Coordinates where button is on the screen
	INT16					YLoc;
	INT32					UserData[4];			// Place holder for user data etc.
	INT16					Group;						// Group this button belongs to (see DOCs)
	INT8					bDefaultStatus;
	//Button disabled style
	INT8					bDisabledStyle;
	//For buttons with text
	wchar_t				*string;					//the string
	UINT16				usFont;						//font for text
	BOOLEAN				fMultiColor;			//font is a multi-color font
	INT16					sForeColor;				//text colors if there is text
	INT16					sShadowColor;
	INT16					sForeColorDown;		//text colors when button is down (optional)
	INT16					sShadowColorDown;
	INT16					sForeColorHilited;		//text colors when button is down (optional)
	INT16					sShadowColorHilited;
	INT8					bJustification;		// BUTTON_TEXT_LEFT, BUTTON_TEXT_CENTER, BUTTON_TEXT_RIGHT
	INT8					bTextXOffset;
	INT8					bTextYOffset;
	INT8					bTextXSubOffSet;
	INT8					bTextYSubOffSet;
	BOOLEAN				fShiftText;
	INT16					sWrappedWidth;
	//For buttons with icons (don't confuse this with quickbuttons which have up to 5 states )
	INT32					iIconID;
	INT16					usIconIndex;
	INT8					bIconXOffset; //-1 means horizontally centered
	INT8					bIconYOffset; //-1 means vertically centered
	BOOLEAN				fShiftImage;  //if true, icon is shifted +1,+1 when button state is down.

	UINT8					ubToggleButtonOldState;		// Varibles for new toggle buttons that work
	UINT8					ubToggleButtonActivated;

	INT32					BackRect;					// Handle to a Background Rectangle
	UINT8					ubSoundSchemeID;
} GUI_BUTTON;


#define MAX_BUTTONS					400

extern GUI_BUTTON			*ButtonList[MAX_BUTTONS];									// Button System's Main Button List

#define GetButtonPtr(x)	(((x>=0) && (x<MAX_BUTTONS))? ButtonList[x] : NULL)

// Struct definition for the QuickButton pictures.
typedef struct {
	HVOBJECT	vobj;						// The Image itself
	INT32			Grayed;					// Index to use for a "Grayed-out" button
	INT32			OffNormal;			// Index to use when button is OFF
	INT32			OffHilite;			// Index to use when button is OFF w/ hilite on it
	INT32			OnNormal;				// Index to use when button is ON
	INT32			OnHilite;				// Index to use when button is ON w/ hilite on it
	UINT32		MaxWidth;				// Width of largest image in use
 	UINT32		MaxHeight;			// Height of largest image in use
	UINT32		fFlags;					// Special image flags
} BUTTON_PICS;

#define MAX_BUTTON_PICS			256

extern BUTTON_PICS		ButtonPictures[MAX_BUTTON_PICS];


// Function protos for button system
BOOLEAN InitializeButtonImageManager(INT32 DefaultBuffer, INT32 DefaultPitch, INT32 DefaultBPP);
void ShutdownButtonImageManager(void);
BOOLEAN InitButtonSystem(void);
void ShutdownButtonSystem(void);

INT16 FindFreeIconSlot(void);
INT32 FindFreeButtonSlot(void);
INT16 FindFreeGenericSlot(void);
INT16 FindFreeIconSlot(void);
INT32 GetNextButtonNumber(void);

// Now used by Wizardry -- DB
void SetButtonFastHelpText(INT32 iButton, const wchar_t *Text);

#ifdef _JA2_RENDER_DIRTY
void SetBtnHelpEndCallback( INT32 iButton, MOUSE_HELPTEXT_DONE_CALLBACK CallbackFxn );
//void DisplayFastHelp(GUI_BUTTON *b);
void RenderButtonsFastHelp(void);

#define RenderButtonsFastHelp()			RenderFastHelp()

BOOLEAN	SetButtonSavedRect( INT32 iButton );
void		FreeButtonSavedRect( INT32 iButton );

#endif

INT16 LoadGenericButtonIcon(UINT8 *filename);
BOOLEAN UnloadGenericButtonIcon(INT16 GenImg);
INT32 LoadButtonImage(const char *filename, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
INT32 UseLoadedButtonImage(INT32 LoadedImg, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
INT32 UseVObjAsButtonImage(HVOBJECT hVObject, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
void UnloadButtonImage(INT32 Index);
INT16 LoadGenericButtonImages(UINT8 *GrayName,UINT8 *OffNormName,UINT8 *OffHiliteName,UINT8 *OnNormName,UINT8 *OnHiliteName,UINT8 *BkGrndName,INT16 Index,INT16 OffsetX, INT16 OffsetY);
BOOLEAN UnloadGenericButtonImage(INT16 GenImg);

BOOLEAN SetButtonDestBuffer(UINT32 DestBuffer);

BOOLEAN EnableButton(INT32 iButtonID);
BOOLEAN DisableButton(INT32 iButtonID);
void RemoveButton(INT32 iButtonID );
void HideButton( INT32 iButtonID );
void ShowButton( INT32 iButton );

void RenderButtons(void);
BOOLEAN DrawButton(INT32 iButtonID);
void DrawButtonFromPtr(GUI_BUTTON *b);

//Base button types
void DrawGenericButton(GUI_BUTTON *b);
void DrawQuickButton(GUI_BUTTON *b);
void DrawCheckBoxButton( GUI_BUTTON *b );
//Additional layers on buttons that can exist in any combination on generic or quick buttons
//To do so, use the new specify functions below.
void DrawIconOnButton(GUI_BUTTON *b);
void DrawTextOnButton(GUI_BUTTON *b);

extern BOOLEAN gfRenderHilights;
#define EnableHilightsAndHelpText()			gfRenderHilights = TRUE;
#define DisableHilightsAndHelpText()		gfRenderHilights = FALSE;

//Providing you have allocated your own image, this is a somewhat simplified function.
INT32 QuickCreateButton(UINT32 Image, INT16 xloc, INT16 yloc, INT32 Type,INT16 Priority,GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback);

//A hybrid of QuickCreateButton.  Takes a lot less parameters, but makes more assumptions.  It self manages the
//loading, and deleting of the image.  The size of the image determines the size of the button.  It also uses
//the default move callback which emulates Win95.  Finally, it sets the priority to normal.  The function you
//choose also determines the type of button (toggle, notoggle, or newtoggle)
INT32 CreateEasyNoToggleButton ( INT32 x, INT32 y, UINT8 *filename, GUI_CALLBACK ClickCallback );
INT32 CreateEasyToggleButton   ( INT32 x, INT32 y, UINT8 *filename, GUI_CALLBACK ClickCallback );
INT32 CreateEasyNewToggleButton( INT32 x, INT32 y, UINT8 *filename, GUI_CALLBACK ClickCallback );
//Same as above, but accepts specify toggle type
INT32 CreateEasyButton( INT32 x, INT32 y, UINT8 *filename, INT32 Type, GUI_CALLBACK ClickCallback);
//Same as above, but accepts priority specification.
INT32 CreateSimpleButton( INT32 x, INT32 y, UINT8 *filename, INT32 Type, INT16 Priority, GUI_CALLBACK ClickCallback );

INT32 CreateCheckBoxButton( INT16 x, INT16 y, const char *filename, INT16 Priority, GUI_CALLBACK ClickCallback );
INT32 CreateIconButton(INT16 Icon,INT16 IconIndex,INT16 GenImg,INT16 xloc,INT16 yloc,INT16 w,INT16 h,INT32 Type,INT16 Priority,GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback);
INT32 CreateHotSpot(INT16 xloc, INT16 yloc, INT16 Width, INT16 Height,INT16 Priority,GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback);

INT32 CreateTextButton(const wchar_t *string, UINT32 uiFont, INT16 sForeColor, INT16 sShadowColor, INT16 GenImg, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT32 Type, INT16 Priority,GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback);
INT32 CreateIconAndTextButton( INT32 Image, const wchar_t *string, UINT32 uiFont,
															 INT16 sForeColor, INT16 sShadowColor,
															 INT16 sForeColorDown, INT16 sShadowColorDown,
															 INT8 bJustification,
															 INT16 xloc, INT16 yloc, INT32 Type, INT16 Priority,
															 GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback);

//New functions
void SpecifyButtonText( INT32 iButtonID, const wchar_t *string );
void SpecifyButtonFont( INT32 iButtonID, UINT32 uiFont );
void SpecifyButtonMultiColorFont(INT32 iButtonID, BOOLEAN fMultiColor);
void SpecifyButtonUpTextColors( INT32 iButtonID, INT16 sForeColor, INT16 sShadowColor );
void SpecifyButtonDownTextColors( INT32 iButtonID, INT16 sForeColorDown, INT16 sShadowColorDown );
void SpecifyButtonHilitedTextColors( INT32 iButtonID, INT16 sForeColorHilited, INT16 sShadowColorHilited );
void SpecifyButtonTextJustification( INT32 iButtonID, INT8 bJustification );
void SpecifyGeneralButtonTextAttributes( INT32 iButtonID, const wchar_t *string, INT32 uiFont,
																			INT16 sForeColor, INT16 sShadowColor );
void SpecifyFullButtonTextAttributes( INT32 iButtonID, const wchar_t *string, INT32 uiFont,
																		  INT16 sForeColor, INT16 sShadowColor,
																			INT16 sForeColorDown, INT16 sShadowColorDown, INT8 bJustification );
void SpecifyButtonTextOffsets( INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText );
void SpecifyButtonTextSubOffsets( INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText );
void SpecifyButtonTextWrappedWidth(INT32 iButtonID, INT16 sWrappedWidth);

void SpecifyButtonSoundScheme( INT32 iButtonID, INT8 bSoundScheme );
void PlayButtonSound( INT32 iButtonID, INT32 iSoundType );

void AllowDisabledButtonFastHelp( INT32 iButtonID, BOOLEAN fAllow );

enum{
	DEFAULT_STATUS_NONE,
	DEFAULT_STATUS_DARKBORDER,			//shades the borders 2 pixels deep
	DEFAULT_STATUS_DOTTEDINTERIOR,	//draws the familiar dotted line in the interior portion of the button.
	DEFAULT_STATUS_WINDOWS95,				//both DARKBORDER and DOTTEDINTERIOR
};
void GiveButtonDefaultStatus( INT32 iButtonID, INT32 iDefaultStatus );
void RemoveButtonDefaultStatus( INT32 iButtonID );

enum //for use with SpecifyDisabledButtonStyle
{
	DISABLED_STYLE_NONE,		//for dummy buttons, panels, etc.  Always displays normal state.
	DISABLED_STYLE_DEFAULT,	//if button has text then shade, else hatch
	DISABLED_STYLE_HATCHED,	//always hatches the disabled button
	DISABLED_STYLE_SHADED		//always shades the disabled button 25% darker
};
void SpecifyDisabledButtonStyle( INT32 iButtonID, INT8 bStyle );


void RemoveTextFromButton( INT32 iButtonID );
void RemoveIconFromButton( INT32 iButtonID );

//Note:  Text is always on top
//If fShiftImage is true, then the image will shift down one pixel and right one pixel
//just like the text does.
BOOLEAN SpecifyButtonIcon( INT32 iButtonID, INT32 iVideoObjectID, UINT16 usVideoObjectIndex,
													 INT8 bXOffset, INT8 bYOffset, BOOLEAN fShiftImage );



void SetButtonPosition(INT32 iButtonID,INT16 x, INT16 y);
void ResizeButton(INT32 iButtonID,INT16 w, INT16 h);


void QuickButtonCallbackMMove(MOUSE_REGION *reg,INT32 reason);
void QuickButtonCallbackMButn(MOUSE_REGION *reg,INT32 reason);

BOOLEAN SetButtonCursor(INT32 iBtnId, UINT16 crsr);
void MSYS_SetBtnUserData(INT32 iButtonNum,INT32 index,INT32 userdata);
INT32 MSYS_GetBtnUserData(GUI_BUTTON *b,INT32 index);
void MarkAButtonDirty( INT32 iButtonNum ); // will mark only selected button dirty
void MarkButtonsDirty(void);// Function to mark buttons dirty ( all will redraw at next RenderButtons )
void PausedMarkButtonsDirty( void ); // mark buttons dirty for button render the frame after the next
void UnMarkButtonDirty( INT32 iButtonIndex );  // unmark button
void UnmarkButtonsDirty( void ); // unmark ALL the buttoms on the screen dirty
void ForceButtonUnDirty( INT32 iButtonIndex ); // forces button undirty no matter the reason, only lasts one frame

// DB 98-05-05
BOOLEAN GetButtonArea(INT32 iButtonID, SGPRect *pRect);
// DB 99-01-13
INT32 GetButtonWidth(INT32 iButtonID);
INT32 GetButtonHeight(INT32 iButtonID);

// DB 99-08-27
INT32 GetButtonX(INT32 iButtonID);
INT32 GetButtonY(INT32 iButtonID);

void BtnGenericMouseMoveButtonCallback(GUI_BUTTON *btn,INT32 reason);
#define DEFAULT_MOVE_CALLBACK		BtnGenericMouseMoveButtonCallback


void DrawCheckBoxButtonOn( INT32 iButtonID );

void DrawCheckBoxButtonOff( INT32 iButtonID );

extern UINT16 GetWidthOfButtonPic( UINT16 usButtonPicID, INT32 iSlot );

#ifdef __cplusplus
}
#endif

#endif
