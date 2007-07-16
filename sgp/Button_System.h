// by Kris Morness (originally created by Bret Rowden)

#ifndef BUTTON_SYSTEM_H
#define BUTTON_SYSTEM_H

#include "VObject.h"
#include "MouseSystem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTON_TEXT_LEFT		-1
#define BUTTON_TEXT_CENTER	0
#define BUTTON_TEXT_RIGHT		1

#define TEXT_LJUSTIFIED		BUTTON_TEXT_LEFT
#define TEXT_CJUSTIFIED		BUTTON_TEXT_CENTER
#define TEXT_RJUSTIFIED		BUTTON_TEXT_RIGHT


// Some GUI_BUTTON system defines
#define BUTTON_USE_DEFAULT		-1
#define BUTTON_NO_CALLBACK		NULL
#define BUTTON_NO_IMAGE				-1
#define BUTTON_NO_SLOT				-1

//effects how the button is rendered.
#define BUTTON_TYPES						( BUTTON_QUICK | BUTTON_GENERIC | BUTTON_HOT_SPOT | BUTTON_CHECKBOX )
//effects how the button is processed
#define BUTTON_TYPE_MASK (BUTTON_NO_TOGGLE | BUTTON_CHECKBOX)

//button flags
#define BUTTON_TOGGLE										0x00000000
#define BUTTON_QUICK										0x00000000
#define BUTTON_ENABLED									0x00000001
#define BUTTON_CLICKED_ON								0x00000002
#define BUTTON_NO_TOGGLE								0x00000004
#define BUTTON_GENERIC									0x00000020
#define BUTTON_HOT_SPOT									0x00000040
#define BUTTON_SELFDELETE_IMAGE					0x00000080
#define BUTTON_DELETION_PENDING					0x00000100
#define BUTTON_DIRTY										0x00000400
#define BUTTON_CHECKBOX									0x00001000
#define BUTTON_NEWTOGGLE								0x00002000
#define BUTTON_FORCE_UNDIRTY						0x00004000 // no matter what happens this buttons does NOT get marked dirty
#define BUTTON_NO_DUPLICATE             0x80000000 // Exclude button from duplicate check

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
	UINT32				uiFlags;					// Button state flags etc.( 32-bit )
	UINT32				uiOldFlags;				// Old flags from previous render loop
	INT16					XLoc;							// Coordinates where button is on the screen
	INT16					YLoc;
	INT32         UserData;         // Place holder for user data etc.
	//Button disabled style
	INT8					bDisabledStyle;
	//For buttons with text
	wchar_t				*string;					//the string
	UINT16				usFont;						//font for text
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

	UINT8					ubSoundSchemeID;
} GUI_BUTTON;


#define MAX_BUTTONS					400

extern GUI_BUTTON			*ButtonList[MAX_BUTTONS];									// Button System's Main Button List

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


BOOLEAN InitButtonSystem(void);
void ShutdownButtonSystem(void);

// Now used by Wizardry -- DB
void SetButtonFastHelpText(INT32 iButton, const wchar_t *Text);

#ifdef _JA2_RENDER_DIRTY
void RenderButtonsFastHelp(void);

#define RenderButtonsFastHelp()			RenderFastHelp()

#endif

INT16 LoadGenericButtonIcon(const char* filename);
BOOLEAN UnloadGenericButtonIcon(INT16 GenImg);
INT32 LoadButtonImage(const char *filename, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
INT32 UseLoadedButtonImage(INT32 LoadedImg, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);
void UnloadButtonImage(INT32 Index);

void EnableButton(INT32 iButtonID);
void DisableButton(INT32 iButtonID);
void RemoveButton(INT32 iButtonID );
void HideButton( INT32 iButtonID );
void ShowButton( INT32 iButton );

void RenderButtons(void);
BOOLEAN DrawButton(INT32 iButtonID);

extern BOOLEAN gfRenderHilights;

//Providing you have allocated your own image, this is a somewhat simplified function.
INT32 QuickCreateButton(UINT32 Image, INT16 xloc, INT16 yloc, INT32 Type,INT16 Priority,GUI_CALLBACK MoveCallback,GUI_CALLBACK ClickCallback);

//A hybrid of QuickCreateButton.  Takes a lot less parameters, but makes more assumptions.  It self manages the
//loading, and deleting of the image.  The size of the image determines the size of the button.  It also uses
//the default move callback which emulates Win95.  Finally, it sets the priority to normal.  The function you
//choose also determines the type of button (toggle, notoggle, or newtoggle)
INT32 CreateEasyNoToggleButton(INT32 x, INT32 y, const char* filename, GUI_CALLBACK ClickCallback);
INT32 CreateEasyToggleButton(INT32 x, INT32 y, const char* filename, GUI_CALLBACK ClickCallback);
//Same as above, but accepts priority specification.
INT32 CreateSimpleButton(INT32 x, INT32 y, const char* filename, INT32 Type, INT16 Priority, GUI_CALLBACK ClickCallback);

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

void SpecifyButtonText( INT32 iButtonID, const wchar_t *string );
void SpecifyButtonFont( INT32 iButtonID, UINT32 uiFont );
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

enum //for use with SpecifyDisabledButtonStyle
{
	DISABLED_STYLE_NONE,		//for dummy buttons, panels, etc.  Always displays normal state.
	DISABLED_STYLE_DEFAULT,	//if button has text then shade, else hatch
	DISABLED_STYLE_HATCHED,	//always hatches the disabled button
	DISABLED_STYLE_SHADED		//always shades the disabled button 25% darker
};
void SpecifyDisabledButtonStyle( INT32 iButtonID, INT8 bStyle );


//Note:  Text is always on top
//If fShiftImage is true, then the image will shift down one pixel and right one pixel
//just like the text does.
BOOLEAN SpecifyButtonIcon( INT32 iButtonID, INT32 iVideoObjectID, UINT16 usVideoObjectIndex,
													 INT8 bXOffset, INT8 bYOffset, BOOLEAN fShiftImage );


BOOLEAN SetButtonCursor(INT32 iBtnId, UINT16 crsr);
void MSYS_SetBtnUserData(INT32 iButtonNum, INT32 userdata);
INT32 MSYS_GetBtnUserData(const GUI_BUTTON* b);
void MarkAButtonDirty( INT32 iButtonNum ); // will mark only selected button dirty
void MarkButtonsDirty(void);// Function to mark buttons dirty ( all will redraw at next RenderButtons )
void UnMarkButtonDirty( INT32 iButtonIndex );  // unmark button
void UnmarkButtonsDirty( void ); // unmark ALL the buttoms on the screen dirty
void ForceButtonUnDirty( INT32 iButtonIndex ); // forces button undirty no matter the reason, only lasts one frame

void BtnGenericMouseMoveButtonCallback(GUI_BUTTON *btn,INT32 reason);
#define DEFAULT_MOVE_CALLBACK		BtnGenericMouseMoveButtonCallback


void DrawCheckBoxButtonOn( INT32 iButtonID );

void DrawCheckBoxButtonOff( INT32 iButtonID );

extern UINT16 GetWidthOfButtonPic( UINT16 usButtonPicID, INT32 iSlot );

#ifdef __cplusplus
}
#endif

#endif
