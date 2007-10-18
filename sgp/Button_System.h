// by Kris Morness (originally created by Bret Rowden)

#ifndef BUTTON_SYSTEM_H
#define BUTTON_SYSTEM_H

#include "VObject.h"
#include "MouseSystem.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_BUTTONS     400
#define MAX_BUTTON_PICS 256


#define BUTTON_TEXT_LEFT   -1
#define BUTTON_TEXT_CENTER  0
#define BUTTON_TEXT_RIGHT   1


// Some GUI_BUTTON system defines
#define BUTTON_NO_IMAGE    -1
#define BUTTON_NO_SLOT     -1

//effects how the button is rendered.
#define BUTTON_TYPES (BUTTON_QUICK | BUTTON_GENERIC | BUTTON_HOT_SPOT | BUTTON_CHECKBOX)

//button flags
#define BUTTON_TOGGLE           0x00000000
#define BUTTON_QUICK            0x00000000
#define BUTTON_ENABLED          0x00000001
#define BUTTON_CLICKED_ON       0x00000002
#define BUTTON_GENERIC          0x00000020
#define BUTTON_HOT_SPOT         0x00000040
#define BUTTON_SELFDELETE_IMAGE 0x00000080
#define BUTTON_DELETION_PENDING 0x00000100
#define BUTTON_DIRTY            0x00000400
#define BUTTON_CHECKBOX         0x00001000
#define BUTTON_NEWTOGGLE        0x00002000
#define BUTTON_FORCE_UNDIRTY    0x00004000 // no matter what happens this buttons does NOT get marked dirty
#define BUTTON_NO_DUPLICATE     0x80000000 // Exclude button from duplicate check

#define BUTTON_SOUND_NONE                  0x00
#define BUTTON_SOUND_CLICKED_ON            0x01
#define BUTTON_SOUND_CLICKED_OFF           0x02
#define BUTTON_SOUND_MOVED_ONTO            0x04
#define BUTTON_SOUND_MOVED_OFF_OF          0x08
#define BUTTON_SOUND_DISABLED_CLICK        0x10
#define BUTTON_SOUND_DISABLED_MOVED_ONTO   0x20
#define BUTTON_SOUND_DISABLED_MOVED_OFF_OF 0x40


extern UINT32 ButtonDestBuffer;

typedef struct GUI_BUTTON GUI_BUTTON;

// GUI_BUTTON callback function type
typedef void (*GUI_CALLBACK)(struct GUI_BUTTON*, INT32);

// GUI_BUTTON structure definitions.
struct GUI_BUTTON
{
	INT32        IDNum;         // ID Number, contains it's own button number
	UINT32       ImageNum;      // Image number to use (see DOCs for details)
	MOUSE_REGION Area;          // Mouse System's mouse region to use for this button
	GUI_CALLBACK ClickCallback; // Button Callback when button is clicked
	GUI_CALLBACK MoveCallback;  // Button Callback when mouse moved on this region
	UINT32       uiFlags;       // Button state flags etc.( 32-bit )
	UINT32       uiOldFlags;    // Old flags from previous render loop
	INT16        XLoc;          // Coordinates where button is on the screen
	INT16        YLoc;
	union                       // Place holder for user data etc.
	{
		INT32 Data;
		void* Ptr;
	} User;
	INT8         bDisabledStyle; // Button disabled style

	// For buttons with text
	wchar_t*     string;              // the string
	UINT16       usFont;              // font for text
	INT16        sForeColor;          // text colors if there is text
	INT16        sShadowColor;
	INT16        sForeColorDown;      // text colors when button is down (optional)
	INT16        sShadowColorDown;
	INT16        sForeColorHilited;   // text colors when button is down (optional)
	INT16        sShadowColorHilited;
	INT8         bJustification;      // BUTTON_TEXT_LEFT, BUTTON_TEXT_CENTER, BUTTON_TEXT_RIGHT
	INT8         bTextXOffset;
	INT8         bTextYOffset;
	INT8         bTextXSubOffSet;
	INT8         bTextYSubOffSet;
	BOOLEAN      fShiftText;
	INT16        sWrappedWidth;

	// For buttons with icons (don't confuse this with quickbuttons which have up to 5 states)
	INT32        iIconID;
	INT16        usIconIndex;
	INT8         bIconXOffset; // -1 means horizontally centered
	INT8         bIconYOffset; // -1 means vertically centered
	BOOLEAN      fShiftImage;  // if true, icon is shifted +1,+1 when button state is down.

	UINT8        ubToggleButtonActivated;

	UINT8        ubSoundSchemeID;
};


extern GUI_BUTTON* ButtonList[MAX_BUTTONS]; // Button System's Main Button List


/* Initializes the GUI button system for use. Must be called before using any
 * other button functions.
 */
BOOLEAN InitButtonSystem(void);

/* Shuts down and cleans up the GUI button system. Must be called before exiting
 * the program.  Button functions should not be used after calling this
 * function.
 */
void ShutdownButtonSystem(void);

// Set the text that will be displayed as the FastHelp
void SetButtonFastHelpText(INT32 iButton, const wchar_t* Text);

#if defined _JA2_RENDER_DIRTY

void RenderButtonsFastHelp(void);
#	define RenderButtonsFastHelp() RenderFastHelp()

#endif

// Loads an image file for use as a button icon.
INT16 LoadGenericButtonIcon(const char* filename);

// Removes a button icon graphic from the system
BOOLEAN UnloadGenericButtonIcon(INT16 GenImg);

// Load images for use with QuickButtons.
INT32 LoadButtonImage(const char* filename, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);

/* Uses a previously loaded quick button image for use with QuickButtons.  The
 * function simply duplicates the vobj!
 */
INT32 UseLoadedButtonImage(INT32 LoadedImg, INT32 Grayed, INT32 OffNormal, INT32 OffHilite, INT32 OnNormal, INT32 OnHilite);

// Removes a QuickButton image from the system.
void UnloadButtonImage(INT32 Index);

// Enables an already created button.
void EnableButton(INT32 iButtonID);

/* Disables a button. The button remains in the system list, and can be
 * reactivated by calling EnableButton.  Diabled buttons will appear "grayed
 * out" on the screen (unless the graphics for such are not available).
 */
void DisableButton(INT32 iButtonID);

/* Removes a button from the system's list. All memory associated with the
 * button is released.
 */
void RemoveButton(INT32 iButtonID);

void HideButton(INT32 iButtonID);
void ShowButton(INT32 iButton);

void RenderButtons(void);

// Draws a single button on the screen.
BOOLEAN DrawButton(INT32 iButtonID);

extern BOOLEAN gfRenderHilights;

/* Creates a QuickButton. QuickButtons only have graphics associated with them.
 * They cannot be re-sized, nor can the graphic be changed.  Providing you have
 * allocated your own image, this is a somewhat simplified function.
 */
INT32 QuickCreateButton(UINT32 image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);
INT32 QuickCreateButtonNoMove(UINT32 image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);
INT32 QuickCreateButtonToggle(UINT32 image, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);

INT32 QuickCreateButtonImg(const char* gfx, INT32 grayed, INT32 off_normal, INT32 off_hilite, INT32 on_normal, INT32 on_hilite, INT16 x, INT16 y, INT16 priority, GUI_CALLBACK click);

/* A hybrid of QuickCreateButton.  Takes a lot less parameters, but makes more
 * assumptions.  It self manages the loading, and deleting of the image.  The
 * size of the image determines the size of the button.  It also uses the
 * default move callback which emulates Win95.  Finally, it sets the priority to
 * normal.
 */
INT32 CreateEasyButton(INT32 x, INT32 y, const char* filename, GUI_CALLBACK ClickCallback);
// Same as above, but accepts priority specification.
INT32 CreateSimpleButton(INT32 x, INT32 y, const char* filename, INT16 Priority, GUI_CALLBACK ClickCallback);

INT32 CreateCheckBoxButton(INT16 x, INT16 y, const char* filename, INT16 Priority, GUI_CALLBACK ClickCallback);

// Creates an Iconic type button.
INT32 CreateIconButton(INT16 Icon, INT16 IconIndex, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT16 Priority, GUI_CALLBACK ClickCallback);

/* Creates a button like HotSpot. HotSpots have no graphics associated with
 * them.
 */
INT32 CreateHotSpot(INT16 xloc, INT16 yloc, INT16 Width, INT16 Height, INT16 Priority, GUI_CALLBACK ClickCallback);

// Creates a generic button with text on it.
INT32 CreateTextButton(const wchar_t* string, UINT32 uiFont, INT16 sForeColor, INT16 sShadowColor, INT16 xloc, INT16 yloc, INT16 w, INT16 h, INT16 Priority, GUI_CALLBACK ClickCallback);

INT32 CreateIconAndTextButton(INT32 Image, const wchar_t* string, UINT32 uiFont, INT16 sForeColor, INT16 sShadowColor, INT16 sForeColorDown, INT16 sShadowColorDown, INT16 xloc, INT16 yloc, INT16 Priority, GUI_CALLBACK ClickCallback);

/* This is technically not a clickable button, but just a label with text. It is
 * implemented as button */
INT32 CreateLabel(const wchar_t* text, UINT32 font, INT16 forecolor, INT16 shadowcolor, INT16 x, INT16 y, INT16 w, INT16 h, INT16 priority);

void SpecifyButtonText(INT32 iButtonID, const wchar_t* string);
void SpecifyButtonDownTextColors(INT32 iButtonID, INT16 sForeColorDown, INT16 sShadowColorDown);
void SpecifyButtonHilitedTextColors(INT32 iButtonID, INT16 sForeColorHilited, INT16 sShadowColorHilited);
void SpecifyButtonTextJustification(INT32 iButtonID, INT8 bJustification);
void SpecifyGeneralButtonTextAttributes(INT32 iButtonID, const wchar_t* string, INT32 uiFont, INT16 sForeColor, INT16 sShadowColor);
void SpecifyButtonTextOffsets(INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText);
void SpecifyButtonTextSubOffsets(INT32 iButtonID, INT8 bTextXOffset, INT8 bTextYOffset, BOOLEAN fShiftText);
void SpecifyButtonTextWrappedWidth(INT32 iButtonID, INT16 sWrappedWidth);

void AllowDisabledButtonFastHelp(INT32 iButtonID);

enum // for use with SpecifyDisabledButtonStyle
{
	DISABLED_STYLE_NONE,    // for dummy buttons, panels, etc.  Always displays normal state.
	DISABLED_STYLE_DEFAULT, // if button has text then shade, else hatch
	DISABLED_STYLE_HATCHED, // always hatches the disabled button
	DISABLED_STYLE_SHADED   // always shades the disabled button 25% darker
};
void SpecifyDisabledButtonStyle(INT32 iButtonID, INT8 bStyle);

/* Note:  Text is always on top
 * If fShiftImage is true, then the image will shift down one pixel and right
 * one pixel just like the text does.
 */
BOOLEAN SpecifyButtonIcon(INT32 iButtonID, INT32 iVideoObjectID, UINT16 usVideoObjectIndex, INT8 bXOffset, INT8 bYOffset, BOOLEAN fShiftImage);

// will simply set the cursor for the mouse region the button occupies
BOOLEAN SetButtonCursor(INT32 iBtnId, UINT16 crsr);

void MSYS_SetBtnUserData(INT32 iButtonNum, INT32 userdata);
INT32 MSYS_GetBtnUserData(const GUI_BUTTON* b);
void MarkAButtonDirty(INT32 iButtonNum); // will mark only selected button dirty
void MarkButtonsDirty(void);// Function to mark buttons dirty ( all will redraw at next RenderButtons )
void UnMarkButtonDirty(INT32 iButtonIndex);  // unmark button
void UnmarkButtonsDirty(void); // unmark ALL the buttoms on the screen dirty
void ForceButtonUnDirty(INT32 iButtonIndex); // forces button undirty no matter the reason, only lasts one frame


void DrawCheckBoxButtonOnOff(INT32 iButtonID, BOOLEAN on);

extern UINT16 GetWidthOfButtonPic(UINT16 usButtonPicID);

UINT16 GetGenericButtonFillColor(void);

#ifdef __cplusplus
}
#endif

#endif
